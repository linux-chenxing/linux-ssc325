/*
* drv_dualos.c- Sigmastar
*
* Copyright (c) [2019~2020] SigmaStar Technology.
*
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License version 2 for more details.
*
*/
/*
 * drv_dualos.c
 *   ipc rsq with LH/RTOS
 */
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/ioport.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
#include <linux/ctype.h>
#include <linux/module.h>
#include <asm/compiler.h>
#include <asm/io.h>
#include "linux/arm-smccc.h"
#include "ms_platform.h"
#include "registers.h"
#include "drv_dualos.h"
#include "interos_call.h"
#include "rsq.h"
#include "sw_sem.h"
#include "lock.h"

/* proc */
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>
#include <linux/list.h>

static int disable_rtos = 0;
module_param(disable_rtos, int, 0644);
MODULE_PARM_DESC(disable_rtos, "Disable RTOS IPC");

static struct file *file_open(const char *path, int flags, int rights);
static void file_close(struct file *file);
static int file_write(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size);
static int file_read(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size);
static int file_sync(struct file *file);
/* */
struct sstream {
	reservoir_t		*rvr;
	struct resource	*res;
	unsigned int	id;
	void			*private;
};

#define	SHARE_SIZE		(0x1000)
static rtkinfo_t 		*_rtklin;
static struct sstream	_sstr[8] = { { 0 } };

static struct resource	*_rtkres;
struct semaphore        _interos_call_req = __SEMAPHORE_INITIALIZER(_interos_call_req, 0);
static int              _interos_call_pid;
#ifdef CONFIG_SS_AMP
struct semaphore        _interos_call_mbox = __SEMAPHORE_INITIALIZER(_interos_call_mbox, 1);
struct semaphore        _interos_call_resp = __SEMAPHORE_INITIALIZER(_interos_call_resp, 0);
static int              _signal_1st_using_mbox = 1;
#endif
#if ENABLE_NBLK_CALL
struct semaphore        _interos_nblk_call_req = __SEMAPHORE_INITIALIZER(_interos_nblk_call_req, 0);
static int              _interos_nblk_call_pid;
#endif

struct rsqcb {
	struct list_head	list;
	int	(*rsqproc)(void *param, void *buf, int size, slot_t* slot);
	void			*private;
};

LIST_HEAD(_rsqcblst);
struct rsqcb	rec_rsq;
struct rsqrec {
	struct rsqcb	cb;
	char			file[32];
	reservoir_t		*rvr;
	unsigned int	frms;
	struct file		*fp;
	long long		off;
};

#if defined(CONFIG_LH_RTOS) && defined(CONFIG_SMP)
static reroute_smc_info_t _reroute_smc_info = {
    .mbox_sem = __SEMAPHORE_INITIALIZER(_reroute_smc_info.mbox_sem, 1),
    .resp_sem = __SEMAPHORE_INITIALIZER(_reroute_smc_info.resp_sem, 0)
};
#endif

typedef struct {
    u32 id;
    u32 (*func)(u32 arg0, u32 arg1, u32 arg2, u32 arg3);
#ifdef __ADV_HYP_PROF__
    u64 count;
    char name[SYSCALL_NAME_LEN_MAX];
#endif
} stInterosSyscallEntry;

#define SYSCALL_ENTRY_MAX   64
static stInterosSyscallEntry gInterosSyscallEntry[SYSCALL_ENTRY_MAX] = {{0}};
static CamOsMutex_t gSyscallEntryLock;

static int c_logshow(struct seq_file *m, void *v)
{
	reservoir_t		*rv;
	unsigned int	rs;
	char			msg[256];
	slot_t			slot;

	rv = m->private;
	if (!rv) {
		seq_printf(m, "not available\n");
		return 0;
	}
/* for debug
	seq_printf(m, "Reservoir - %s(%p) size %dK\n", rv->name, rv, rv->size >> 10);
	seq_printf(m, "\tCurrent available frame NO.\t: %d~%d\n", rv->ns.rdno, rv->slotno);
	seq_printf(m, "\treset id                   \t: %d~%d\n", rv->ss.reset_id, rv->ns.reset_id);
	seq_printf(m, "\tDrop Count/Frms\t: %u/%u\n", rv->dropcnt, rv->dropfrms);
	seq_printf(m, "\tReservoir Reset\t: %u\n", rv->resetcnt);
	seq_printf(m, "\tHot sync\t: %u\n", rv->synccnt);
*/
    do {
        DUALOS_LOCK_INIT;
        DUALOS_LOCK(SW_SEM_RESERVE_LOCK_ID);
        rs = rcvr_de_rsqslot(rv, msg, sizeof(msg), (slot_t*)&slot);
        DUALOS_UNLOCK(SW_SEM_RESERVE_LOCK_ID);
        if (!rs) break;
        seq_printf(m, msg);
    } while (m->size >= (m->count + sizeof(msg)));

    seq_printf(m, "\n");

    return 0;
}

static void *c_start(struct seq_file *m, loff_t *pos)
{
	return *pos < 1 ? (void *)1 : NULL;
}

static void *c_next(struct seq_file *m, void *v, loff_t *pos)
{
	++*pos;
	return NULL;
}

static void c_stop(struct seq_file *m, void *v)
{
}

const struct seq_operations rsqlog_op = {
	.start	= c_start,
	.next	= c_next,
	.stop	= c_stop,
	.show	= c_logshow
};

static int rsqinfo_open(struct inode *inode, struct file *file)
{
	int					res = -ENOMEM;
	int					i;
	struct seq_file		*seq;

    res = seq_open(file, NULL);
	if (res)
		return res;
	seq = file->private_data;
	for (i = 0; i < sizeof(_sstr) / sizeof(struct sstream); i++) {
		if (!_sstr[i].rvr) continue;
		if (!strncmp(file->f_path.dentry->d_iname,
					 _sstr[i].rvr->name, sizeof(_sstr[i].rvr->name))) {
			seq->private = _sstr[i].rvr;
			break;
		}
	}
	if (i == (sizeof(_sstr) / sizeof(struct sstream)))
		return -ENOMEM;
	seq->op = _sstr[i].private;
	return res;
}

static ssize_t rsqinfo_write(struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
	char	code[32];
	reservoir_t	*rv;

	rv = (reservoir_t*)((struct seq_file *)file->private_data)->private;
	if (!rv) return len;
	if (copy_from_user(code, buf, 32))
		return -EFAULT;
	if (strncmp(rv->name, "log", 3) == 0) {
		signal_rtos(INTEROS_SC_L2R_RTK_LOG, 0, 0, 0);
	}

	return len;
}

#define	TTM(s) (((s) + 3000) / 6000)
static u64	_spent = 0, _lifet = 0;
static u64 _spent_hyp = 0, _spent_sc = 0;
static int c_show_rtk(struct seq_file *m, void *v)
{
	rtkinfo_t		*rtk;
	int		s;
	u64		cs, cl;
    int     sh, sc;
	u64     ch, cc;

	rtk = m->private;
	if (!rtk) {
		seq_printf(m, "not avaliable\n");
		return 0;
	}

#ifdef CONFIG_SS_AMP
    /* in AMP, use an IPC signal to trigger CPU statistics update */
    signal_rtos(INTEROS_SC_L2R_AMP_RTK_CPU_USAGE, 0, 0, 0);
#endif

	/* reset cpu usage after present */
    cs = rtk->spent;
    cl = rtk->lifet;
    ch = rtk->spent_hyp;
    cc = rtk->spent_sc;

	s = (int)div64_u64((cs - _spent) * 1000, (cl - _lifet));
	if (ch && cc)   // It means __ADV_HYP_PROF__ enabled in rtos
	{
	    sh = (int)div64_u64((ch - _spent_hyp) * 1000, (cl - _lifet));
	    sc = (int)div64_u64((cc - _spent_sc) * 1000, (cl - _lifet));
	}

	seq_printf(m, "RTOS: %s\n", rtk->version);

	if (ch && cc)   // It means __ADV_HYP_PROF__ enabled in rtos
	{
	    seq_printf(m, "\tcpu usage(hyp/sc/rtos): %u.%u%% (%llu/%llu)\n", s / 10, s % 10, cs - _spent, cl - _lifet);
	    seq_printf(m, "\tcpu usage(hyp): %u.%u%% (%llu/%llu)\n", sh / 10, sh % 10, ch - _spent_hyp, cl - _lifet);
	    seq_printf(m, "\tcpu usage(sc): %u.%u%% (%llu/%llu)\n", sc / 10, sc % 10, cc - _spent_sc, cl - _lifet);
	}
	else
	{
	    seq_printf(m, "\tcpu usage: %u.%u%% (%llu/%llu)\n", s / 10, s % 10, cs - _spent, cl - _lifet);
	}
	seq_printf(m, "\tttff(isp): %u ms\n", TTM(rtk->ttff_isp));
	seq_printf(m, "\tttff(scl): %u ms\n", TTM(rtk->ttff_scl));
	seq_printf(m, "\tttff(mfe): %u ms\n", TTM(rtk->ttff_mfe));
	seq_printf(m, "\tload ns  : %u ms\n", TTM(rtk->ldns_ts));
	seq_printf(m, "\tfiq count: %u\n", rtk->fiq_cnt);
	seq_printf(m, "\tsyscall  : %llu\n", rtk->syscall_cnt);
	_spent = cs; _lifet = cl;
	if (ch && cc)   // It means __ADV_HYP_PROF__ enabled in rtk
	{
	    _spent_hyp = ch;
	    _spent_sc = cc;
	}

	return 0;
}

static ssize_t rtkinfo_write(struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
    char code[128] = {0};
    rtkinfo_t *rtk;
    if(len > sizeof(code))
    {
        printk(KERN_ERR "command len is to long!\n");
        return len;
    }

    rtk = (rtkinfo_t*)((struct seq_file *)file->private_data)->private;
    if (!rtk)
    {
        return len;
    }
    if (copy_from_user(code, buf, len))
    {
        return -EFAULT;
    }
    // rtkinfo simple command parser
    // cli -
    if (!strncmp(code, "cli", 3))
    {
        char	*ptr;
        // strip return and space in tail and head.
        ptr = strrchr(code, 0x0a);
        if (ptr)
        {
            *ptr = '\0';
        }
        for (ptr = code + 4; *ptr == ' ' || *ptr == '\t'; ptr++);
        // copy command to share buffer and send to S
        strncpy(rtk->sbox, ptr, 127);
        signal_rtos(INTEROS_SC_L2R_RTK_CLI, (u32)rtk, (u32)rtk->diff, 0);
    } 
    else if (!strncmp(code, "assert", 6)) 
    {
        signal_rtos(INTEROS_SC_L2R_RTK_ASSERT, 0, 0, 0);
    }
    return len;
}

const struct seq_operations rtkinfo_op = {
	.start	= c_start,
	.next	= c_next,
	.stop	= c_stop,
	.show	= c_show_rtk
};

static int rtkinfo_open(struct inode *inode, struct file *file)
{
	int					res = -ENOMEM;
	struct seq_file		*seq;

    res = seq_open(file, &rtkinfo_op);
	if (res)
		return res;
	seq = file->private_data;
	seq->private = (void*)_rtklin;
    return res;
}

static const struct file_operations proc_rsqinfo_operations = {
    .open       = rsqinfo_open,
    .read       = seq_read,
	.write		= rsqinfo_write,
    .llseek     = seq_lseek,
    .release    = seq_release,
};

static const struct file_operations proc_rtkinfo_operations = {
    .open       = rtkinfo_open,
    .read       = seq_read,
	.write		= rtkinfo_write,
    .llseek     = seq_lseek,
    .release    = seq_release,
};

void handle_interos_call_req(void)
{
    up(&_interos_call_req);
}

unsigned long do_interos_call(u32 arg0, u32 arg1, u32 arg2, u32 arg3)
{
    u32 ret = 0;
    u32 i;

#ifdef CONFIG_SS_SWTOE
    if (arg0 == INTEROS_SC_L2R_SWTOE)
    {
        extern int drv_swtoe_cb_hdl(u32 arg0, u32 arg1, u32 arg2, u32 arg3);
        drv_swtoe_cb_hdl(type, arg1, arg2, arg3);
        return ret;
    }
#endif

    CamOsMutexLock(&gSyscallEntryLock);
    for (i=0; i<SYSCALL_ENTRY_MAX; i++)
    {
        if (gInterosSyscallEntry[i].id == arg0 && gInterosSyscallEntry[i].func)
        {
            ret = gInterosSyscallEntry[i].func(arg0, arg1, arg2, arg3);
#ifdef __ADV_HYP_PROF__
            gInterosSyscallEntry[i].count++;
#endif
        }
    }
    CamOsMutexUnlock(&gSyscallEntryLock);

    return ret;
}

#ifdef CONFIG_SS_AMP
void handle_interos_call_resp(void)
{
    up(&_interos_call_resp);
}

static int interos_call_receiver(void *unused)
{
	struct task_struct	*tsk = current;
	u32 arg0, arg1, arg2, arg3, ret;
    int signal_1st_using_mbox;

    interos_call_mbox_args_t *ptr_mbox_args;
    interos_call_args_t *ptr_args;
    struct arm_smccc_res res;

	/* Setup a clean context for our children to inherit. */
	set_task_comm(tsk, "interos_call_receiver");

	while (1) {
		down(&_interos_call_req);
        signal_1st_using_mbox = _signal_1st_using_mbox;

        if (signal_1st_using_mbox)
        {
            ptr_mbox_args = (interos_call_mbox_args_t *)(BASE_REG_MAILBOX_PA+BK_REG(0x50)+IO_OFFSET);
            arg0 = (ptr_mbox_args->arg0_h << 16) + ptr_mbox_args->arg0_l;
            arg1 = (ptr_mbox_args->arg1_h << 16) + ptr_mbox_args->arg1_l;
            arg2 = (ptr_mbox_args->arg2_h << 16) + ptr_mbox_args->arg2_l;
            arg3 = (ptr_mbox_args->arg3_h << 16) + ptr_mbox_args->arg3_l;
        }
        else
        {
            ptr_args = (interos_call_args_t *)((u32)_rtklin + INTEROS_CALL_SHMEM_OFFSET_RX);
            arg0 = ptr_args->arg0;
            arg1 = ptr_args->arg1;
            arg2 = ptr_args->arg2;
            arg3 = ptr_args->arg3;
        }

        /* process here */
        ret = do_interos_call(arg0, arg1, arg2, arg3);
        if (signal_1st_using_mbox)
        {
            ptr_mbox_args->ret_l = ret & 0xFFFF;
            ptr_mbox_args->ret_h = ret >> 16;
        }
        else
        {
            ptr_args->ret = ret;
        }
        arm_smccc_smc(INTEROS_SC_L2R_CALL, TARGET_BITS_CORE1, NSATT_BITS_GROUP0, SGIINTID_BITS_09, 0, 0, 0, 0, &res);
	}
	return 0;
}
#else
static int interos_call_receiver(void *unused)
{
    static int init_sched = 1;
    struct task_struct  *tsk = current;
    u32 ret;

    reservoir_t     *rv;
    unsigned int    rs;
    char            msg[32];
    slot_t          slot;
    u32             arg0, arg1, arg2, arg3;
    int             i;

    for (i = 0; i < 8; i++)
    {
        if (_sstr[i].rvr != NULL && strncmp(_sstr[i].rvr->name, "blk_call", 9) == 0)
        {
            rv = _sstr[i].rvr;
            break;
        }
    }

    if (rv == NULL)
    {
        printk(KERN_ERR "Error!! interos call rsq\n");
        *(int *)0 = 0;
    }

    /* Setup a clean context for our children to inherit. */
    set_task_comm(tsk, "interos_call_receiver");

    while (1) {
        down(&_interos_call_req);
        /* Linux kernel disable FIQ until start scheduling. */
        /* Enable FIQ at first time. */
        if (init_sched == 1)
        {
            local_fiq_enable();
            init_sched = 0;
        }

        do {
            rs = rcvr_de_rsqslot(rv, msg, sizeof(msg), (slot_t*)&slot);
            if (!rs) break;
            arg0 = *((u32 *)msg);
            arg1 = *((u32 *)(msg + 4));
            arg2 = *((u32 *)(msg + 8));
            arg3 = *((u32 *)(msg + 12));

            /* process here */
            ret = do_interos_call(arg0, arg1, arg2, arg3);

            // Ack RTOS block signal finish
            signal_rtos(INTEROS_SC_L2R_ACK_RTK_SIG_BLK, ret, 0, 0);
        } while (1);
    }
    return 0;
}
#endif

#if ENABLE_NBLK_CALL
void handle_interos_nblk_call_req(void)
{
    up(&_interos_nblk_call_req);
}

static int interos_nblk_call_receiver(void *unused)
{
    static int init_sched = 1;
    struct task_struct	*tsk = current;

    reservoir_t     *rv;
    unsigned int    rs;
    char            msg[32];
    slot_t          slot;
    u32             arg0, arg1, arg2, arg3;
    int             i;

    for (i = 0; i < 8; i++)
    {
        if (_sstr[i].rvr != NULL && strncmp(_sstr[i].rvr->name, "nblk_call", 9) == 0)
        {
            rv = _sstr[i].rvr;
            break;
        }
    }

    if (rv == NULL)
    {
        printk(KERN_ERR "Error!! interos nblk call rsq\n");
        *(int *)0 = 0;
    }

    /* Setup a clean context for our children to inherit. */
    set_task_comm(tsk, "interos_nblk_call_receiver");

    while (1) {
        down(&_interos_nblk_call_req);
        /* Linux kernel disable FIQ until start scheduling. */
        /* Enable FIQ at first time. */
        if (init_sched == 1)
        {
            local_fiq_enable();
            init_sched = 0;
        }

        do {
            rs = rcvr_de_rsqslot(rv, msg, sizeof(msg), (slot_t*)&slot);
            if (!rs) break;
            arg0 = *((u32 *)msg);
            arg1 = *((u32 *)(msg + 4));
            arg2 = *((u32 *)(msg + 8));
            arg3 = *((u32 *)(msg + 12));

            /* process here */
            do_interos_call(arg0, arg1, arg2, arg3);
	    } while (1);
	}
	return 0;
}
#endif

static int __init epoch_init(void)
{
#ifndef CONFIG_DISABLE_DUALOS_NODE
    char				proc[32];
#endif
    long				share;
    struct rlink_head	*rl;

    if (disable_rtos)
    {
        return 0;
    }

#ifndef CONFIG_DISABLE_DUALOS_NODE
    proc_mkdir("dualos", NULL);
#endif

	/* get RTOS info */
    share  = (long)signal_rtos(INTEROS_SC_L2R_HANDSHAKE, (u32)0, (u32)0, (u32)0);
#ifdef CONFIG_SS_AMP
    _signal_1st_using_mbox = 0;
#endif
    _rtkres = request_mem_region(share, SHARE_SIZE, "dualos");
    _rtklin = (rtkinfo_t*)ioremap(_rtkres->start, resource_size(_rtkres));

#if defined(CONFIG_SS_AMP) || (defined(CONFIG_LH_RTOS) && defined(CONFIG_SMP))
    intercore_sem_init((u32)_rtklin + SHARE_SIZE - SW_SEM_LOCK_TOTAL_NUM * sizeof(intercore_sem_t));
#endif

    if (INTEROS_CALL_SHMEM_PARAM_SIZE < sizeof(interos_call_args_t)) {
        printk(KERN_ERR "Error!! interos shmem param address\n");
        *(int *)0 = 0;
    }

    if (!_rtklin							||
        _rtklin->size  != sizeof(*_rtklin)	||
        _rtklin->verid != RSQ_VERSION_ID) {
#ifndef CONFIG_DISABLE_DUALOS_NODE
        proc_create("dualos/version_not_match", 0, NULL, &proc_rtkinfo_operations);
#endif
        printk(KERN_ERR "Error!! RTOS version not match\n");
        _rtklin = NULL;
        return 0;
    }
#ifndef CONFIG_DISABLE_DUALOS_NODE
    snprintf(proc, sizeof(proc), "dualos/%s", _rtklin->name);
    proc_create(proc, 0, NULL, &proc_rtkinfo_operations);
#endif
	/* */
    for (rl = &(_rtklin->root); rl->nphys && rl->nsize;) {
        struct sstream      sst;

        sst.res = request_mem_region((long)rl->nphys, rl->nsize, "rsq");
        sst.rvr = (reservoir_t*)ioremap(sst.res->start, resource_size(sst.res));
        rename_region(sst.res, sst.rvr->name);
        init_rsq_rcvr(sst.rvr);
#ifndef CONFIG_DISABLE_DUALOS_NODE
        snprintf(proc, sizeof(proc), "dualos/%s", sst.rvr->name);
        proc_create(proc, 0, NULL, &proc_rsqinfo_operations);
#endif
        sst.id = sst.rvr->iid;
        if (strncmp(sst.rvr->name, "log", 3) == 0)
            sst.private = (void*)&rsqlog_op;  // set log read op
        _sstr[sst.id] = sst;
        rl = &(sst.rvr->link);
    }
	_rtklin->diff = (unsigned int)share - (unsigned int)_rtklin;	// offset for address transfer

    /* create a thread to get interos call */
    _interos_call_pid = kernel_thread(interos_call_receiver, NULL, CLONE_FS | CLONE_FILES);

#if ENABLE_NBLK_CALL
    _interos_nblk_call_pid = kernel_thread(interos_nblk_call_receiver, NULL, CLONE_FS | CLONE_FILES);
#endif

    /* for debug */
    signal_rtos(INTEROS_SC_L2R_RSQ_INIT, (u32)_rtklin, _rtklin->diff, 0);
    return 0;
}

/*
 * kerenl file I/O
 */
struct file *file_open(const char *path, int flags, int rights)
{
    struct file *filp = NULL;
    mm_segment_t oldfs;
    int err = 0;

    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flags, rights);
    set_fs(oldfs);
    if (IS_ERR(filp)) {
        err = PTR_ERR(filp);
        return NULL;
    }
    return filp;
}

void file_close(struct file *file)
{
    filp_close(file, NULL);
}

int file_read(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size)
{
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_read(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}

int file_write(struct file *file, unsigned long long offset, unsigned char *data, unsigned int size)
{
    mm_segment_t oldfs;
    int ret;

    oldfs = get_fs();
    set_fs(get_ds());

    ret = vfs_write(file, data, size, &offset);

    set_fs(oldfs);
    return ret;
}

int file_sync(struct file *file)
{
    vfs_fsync(file, 0);
    return 0;
}

rtkinfo_t* get_rtkinfo(void)
{
    return _rtklin;
}

#define GICD_SGIR			0x0F00
#define GICD_BASE           0xF4001000
#define GICD_WRITEL(a,v)    (*(volatile unsigned int *)(u32)(GICD_BASE + a) = (v))

static void hal_send_SGI(int cpu, int no)
{
	GICD_WRITEL(GICD_SGIR, (1 << (cpu + 16)) | (1 << 15) | no);
}

/* call to RTOS */
#ifdef CONFIG_SS_AMP
unsigned long signal_rtos(u32 type, u32 arg1, u32 arg2, u32 arg3)
{
    interos_call_mbox_args_t *ptr_mbox_args;
    interos_call_args_t *ptr_args;
    struct arm_smccc_res res;
    u32 ret;
    int signal_1st_using_mbox;

    if (disable_rtos)
    {
        return -1;
    }

	down(&_interos_call_mbox);

    signal_1st_using_mbox = _signal_1st_using_mbox;

    if (signal_1st_using_mbox)
    {
        ptr_mbox_args = (interos_call_mbox_args_t *)(BASE_REG_MAILBOX_PA+BK_REG(0x60)+IO_OFFSET);
        ptr_mbox_args->arg0_l = type & 0xFFFF;
        ptr_mbox_args->arg0_h = type >> 16;
        ptr_mbox_args->arg1_l = arg1 & 0xFFFF;
        ptr_mbox_args->arg1_h = arg1 >> 16;
        ptr_mbox_args->arg2_l = arg2 & 0xFFFF;
        ptr_mbox_args->arg2_h = arg2 >> 16;
        ptr_mbox_args->arg3_l = arg3 & 0xFFFF;
        ptr_mbox_args->arg3_h = arg3 >> 16;
        ptr_mbox_args->ret_l = 0;
        ptr_mbox_args->ret_h = 0;
    }
    else
    {
        ptr_args = (interos_call_args_t *)((u32)_rtklin + INTEROS_CALL_SHMEM_OFFSET_TX);
        ptr_args->arg0 = type;
        ptr_args->arg1 = arg1;
        ptr_args->arg2 = arg2;
        ptr_args->arg3 = arg3;
        ptr_args->ret = 0;
    }

  	arm_smccc_smc(INTEROS_SC_L2R_CALL, TARGET_BITS_CORE1, NSATT_BITS_GROUP0, SGIINTID_BITS_10, 0, 0, 0, 0, &res);

	down(&_interos_call_resp);

    if (signal_1st_using_mbox)
    {
        ret = (ptr_mbox_args->ret_h << 16) + ptr_mbox_args->ret_l;
    }
    else
    {
        ret = ptr_args->ret;
    }

	up(&_interos_call_mbox);

    return ret;
}
#endif
#ifdef CONFIG_LH_RTOS
#ifndef CONFIG_SMP
unsigned long signal_rtos(u32 type, u32 arg1, u32 arg2, u32 arg3)
{
	struct arm_smccc_res res;

    if (disable_rtos)
    {
        return -1;
    }

	arm_smccc_smc(type, arg1, arg2, arg3,
		      0, 0, 0, 0, &res);

	return res.a0;
}
#else
#include <linux/cpumask.h>
unsigned long signal_rtos(u32 type, u32 arg1, u32 arg2, u32 arg3)
{
    struct arm_smccc_res res;
    int cpu, cpu_v;
    long ret;
    struct cpumask old_mask, new_mask;

    if (disable_rtos)
    {
        return -1;
    }

    down(&(_reroute_smc_info.mbox_sem));

    _reroute_smc_info.type = type;
    _reroute_smc_info.arg1 = arg1;
    _reroute_smc_info.arg2 = arg2;
    _reroute_smc_info.arg3 = arg3;

    cpu = get_cpu();
    ret = sched_getaffinity(0, &old_mask);
	if (ret != 0)
    {
        printk(KERN_CRIT "get affinity error at %s %d, ret:%ld\n",
            __FUNCTION__, __LINE__, ret);
    }
    cpumask_clear(&new_mask);
    cpumask_set_cpu(cpu, &new_mask);
    ret = sched_setaffinity(0, &new_mask);
    if (ret != 0)
    {
        printk(KERN_CRIT "set affinity error at %s %d, ret:%ld\n",
            __FUNCTION__, __LINE__, ret);
    }
    put_cpu();

    if (cpu != 0)
    {
        cpu_v = get_cpu();
        if (cpu_v == 0)
        {
            printk(KERN_CRIT "%s %d: cpu_v:%d\n", __FUNCTION__, __LINE__, cpu_v);
            asm("b .");
        }
        put_cpu();

        hal_send_SGI(0, IPI_NR_REROUTE_SMC);
        down(&(_reroute_smc_info.resp_sem));
        res.a0 = _reroute_smc_info.ret;
    }
    else
    {
        cpu_v = get_cpu();
        if (cpu_v != 0)
        {
            printk(KERN_CRIT "%s %d: cpu_v:%d\n", __FUNCTION__, __LINE__, cpu_v);
            asm("b .");
        }
        put_cpu();

    	arm_smccc_smc(_reroute_smc_info.type,
                    _reroute_smc_info.arg1,
                    _reroute_smc_info.arg2,
                    _reroute_smc_info.arg3,
                    0, 0, 0, 0, &res);
    }

    ret = sched_setaffinity(0, &old_mask);
    if (ret != 0)
    {
        printk(KERN_CRIT "set affinity error at %s %d, ret=%ld\n", __FUNCTION__, __LINE__, ret);
    }
    up(&(_reroute_smc_info.mbox_sem));

	return res.a0;
}

void handle_reroute_smc(void)
{
	struct arm_smccc_res res;

	arm_smccc_smc(_reroute_smc_info.type,
                _reroute_smc_info.arg1,
                _reroute_smc_info.arg2,
                _reroute_smc_info.arg3,
		        0, 0, 0, 0, &res);
    _reroute_smc_info.ret = res.a0;
    up(&(_reroute_smc_info.resp_sem));
}
#endif
#endif

s32 CamInterOsSignalReg(u32 id, void *func, const char *name)
{
    u32 i;
    s32 ret = -1;

    CamOsMutexLock(&gSyscallEntryLock);
    for (i=0; i<SYSCALL_ENTRY_MAX; i++)
    {
        if (!gInterosSyscallEntry[i].id && !gInterosSyscallEntry[i].func)
        {
            gInterosSyscallEntry[i].id = id;
            gInterosSyscallEntry[i].func = func;
#ifdef __ADV_HYP_PROF__
            gInterosSyscallEntry[i].count = 0;
            snprintf(gInterosSyscallEntry[i].name, SYSCALL_NAME_LEN_MAX, "%s", name);
#endif
            ret = 0;
            break;
        }
    }

    CamOsMutexUnlock(&gSyscallEntryLock);
    return ret;
}

s32 CamInterOsSignalDereg(u32 id, void *func)
{
    u32 i;
    s32 ret = -1;

    CamOsMutexLock(&gSyscallEntryLock);
    for (i=0; i<SYSCALL_ENTRY_MAX; i++)
    {
        if (gInterosSyscallEntry[i].id == id && gInterosSyscallEntry[i].func == func)
        {
            gInterosSyscallEntry[i].id = 0;
            gInterosSyscallEntry[i].func = NULL;
#ifdef __ADV_HYP_PROF__
            gInterosSyscallEntry[i].count = 0;
            memset(gInterosSyscallEntry[i].name, 0, SYSCALL_NAME_LEN_MAX);
#endif
            ret = 0;
            break;
        }
    }

    CamOsMutexUnlock(&gSyscallEntryLock);
    return ret;
}

unsigned int CamInterOsSignal(unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3)
{
    return signal_rtos(arg0, arg1, arg2, arg3);
}

void CamInterOsSignalAsync(unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3)
{
    printk(KERN_ERR "CamInterOsSignalAsync not supported in linux\n");
    return;
}

unsigned int CamInterOsRequestLock(void)
{
    return intercore_sem_lock_request();
}

void CamInterOsFreeLock(unsigned int sem_id)
{
    if (sem_id)
        intercore_sem_lock_free(sem_id);

    return;
}

void CamInterOsLock(unsigned int *lock, unsigned int sem_id)
{
#if defined(CONFIG_SS_AMP)  // Multi-core AMP
    unsigned long cpu_sr = 0;
    local_irq_save(cpu_sr);
    *lock = cpu_sr;
    intercore_sem_lock(sem_id);
#elif defined(CONFIG_LH_RTOS)
#ifdef CONFIG_SMP   // Multi-core SMPLH
    unsigned long cpu_sr = 0;
    if (get_cpu() == 0) {
        local_fiq_disable();
    }
    local_irq_save(cpu_sr);
    *lock = cpu_sr;
    intercore_sem_lock(sem_id);
#else   // Single-core LH
    local_fiq_disable();
#endif
#endif
}

void CamInterOsUnlock(unsigned int *lock, unsigned int sem_id)
{
#if defined(CONFIG_SS_AMP)  // Multi-core AMP
    unsigned long cpu_sr = 0;
    intercore_sem_unlock(sem_id);
    cpu_sr = *lock;
    local_irq_restore(cpu_sr);
#elif defined(CONFIG_LH_RTOS)
#ifdef CONFIG_SMP   // Multi-core SMPLH
    unsigned long cpu_sr = 0;
    intercore_sem_unlock(sem_id);
    cpu_sr = *lock;
    local_irq_restore(cpu_sr);
    if (smp_processor_id() == 0) {
        local_fiq_enable();
        put_cpu();
    }
#else   // Single-core LH
    local_fiq_enable();
#endif
#endif
}

fs_initcall(epoch_init);
