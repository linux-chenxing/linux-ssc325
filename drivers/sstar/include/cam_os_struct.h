/*
* cam_os_struct.h - Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: giggs.huang <giggs.huang@sigmastar.com.tw>
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/


#ifndef __CAM_OS_STRUCT_H__
#define __CAM_OS_STRUCT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(__KERNEL__)
#define CAM_OS_LINUX_KERNEL
#endif

#ifdef CAM_OS_RTK

#include "sys_MsWrapper_cus_os_sem.h"

#define CAM_OS_MUTEX_SIZE       ((sizeof(Ms_Mutex_t)+sizeof(u32)-1)/sizeof(u32)+1)
#define CAM_OS_TSEM_SIZE        ((sizeof(Ms_DynSemaphor_t)+sizeof(u32)-1)/sizeof(u32)+1)
#define CAM_OS_RWTSEM_SIZE      ((sizeof(Ms_Mutex_t)+sizeof(Ms_DynSemaphor_t)+sizeof(u32)+sizeof(u32)-1)/sizeof(u32)+1)
#define CAM_OS_TCOND_SIZE       ((sizeof(Ms_DynSemaphor_t)+sizeof(u32)-1)/sizeof(u32)+1)
#define CAM_OS_SPINLOCK_SIZE    ((sizeof(unsigned long)-1)/sizeof(u32)+1)
#define CAM_OS_TIMER_SIZE       ((sizeof(MsTimerId_e)+sizeof(void*)+sizeof(void*)-1)/sizeof(u32)+1)
#define CAM_OS_MEMCACHE_SIZE    ((sizeof(u8)+sizeof(u32)-1)/sizeof(u32)+1)
#define CAM_OS_IDR_SIZE         ((sizeof(void **)+sizeof(unsigned long *)+sizeof(unsigned long)-1)/sizeof(u32)+1)

#elif defined(CAM_OS_LINUX_KERNEL)

#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/rwsem.h>
#include <linux/completion.h>
#include <linux/spinlock_types.h>
#include <linux/timer.h>
#include <linux/scatterlist.h>
#include <linux/slab_def.h>
#include <linux/idr.h>

#define CAM_OS_MUTEX_SIZE       ((sizeof(struct mutex)+sizeof(u32)-1)/sizeof(u32)+1)
#define CAM_OS_TSEM_SIZE        ((sizeof(struct semaphore)+sizeof(u32)-1)/sizeof(u32)+1)
#define CAM_OS_RWTSEM_SIZE      ((sizeof(struct rw_semaphore)+sizeof(u32)-1)/sizeof(u32)+1)
#define CAM_OS_TCOND_SIZE       ((sizeof(struct completion)+sizeof(u32)-1)/sizeof(u32)+1)
#define CAM_OS_SPINLOCK_SIZE    ((sizeof(spinlock_t)+sizeof(u32)+sizeof(unsigned long)-1)/sizeof(u32)+1)
#define CAM_OS_TIMER_SIZE       ((sizeof(struct timer_list)-1)/sizeof(u32)+1)
#define CAM_OS_MEMCACHE_SIZE    ((sizeof(struct kmem_cache)-1)/sizeof(u32)+1)
#define CAM_OS_IDR_SIZE         ((sizeof(struct idr)-1)/sizeof(u32)+1)

#else

#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define CAM_OS_MUTEX_SIZE       ((sizeof(pthread_mutex_t)+sizeof(u32)-1)/sizeof(u32)+1)
#define CAM_OS_TSEM_SIZE        ((sizeof(sem_t)+sizeof(u32)-1)/sizeof(u32)+1)
#define CAM_OS_RWTSEM_SIZE      ((sizeof(pthread_rwlock_t)+sizeof(u32)-1)/sizeof(u32)+1)
#define CAM_OS_TCOND_SIZE       ((sizeof(pthread_mutex_t)+sizeof(pthread_cond_t)+sizeof(u32)+sizeof(u32)-1)/sizeof(u32)+1)
#define CAM_OS_SPINLOCK_SIZE    ((sizeof(pthread_spinlock_t)+sizeof(u32)-1)/sizeof(u32)+1)
#define CAM_OS_TIMER_SIZE       ((sizeof(timer_t)-1)/sizeof(u32)+1)
#define CAM_OS_MEMCACHE_SIZE    0
#define CAM_OS_IDR_SIZE         ((sizeof(void **)+sizeof(unsigned long *)+sizeof(unsigned long)-1)/sizeof(u32)+1)

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__CAM_OS_STRUCT_H__
