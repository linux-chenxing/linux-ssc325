/*
* cam_os_struct.h- Sigmastar
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


#ifndef __CAM_OS_STRUCT_H__
#define __CAM_OS_STRUCT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "cam_os_util.h"

#if defined(__KERNEL__)
#define CAM_OS_LINUX_KERNEL
#endif

#ifdef CAM_OS_RTK

#include "sys_MsWrapper_cus_os_sem.h"
#include "sys_MsWrapper_cus_os_timer.h"

#define CAM_OS_MUTEX_SIZE       CAM_OS_ALIGN_UP(sizeof(u32) + sizeof(Ms_Mutex_t), 4) / sizeof(u32)
#define CAM_OS_TSEM_SIZE        CAM_OS_ALIGN_UP(sizeof(u32) + sizeof(Ms_DynSemaphor_t), 4) / sizeof(u32)
#define CAM_OS_RWTSEM_SIZE      CAM_OS_ALIGN_UP(sizeof(u32) + sizeof(u32) + sizeof(Ms_Mutex_t) + sizeof(Ms_DynSemaphor_t), 4) / sizeof(u32)
#define CAM_OS_TCOND_SIZE       CAM_OS_ALIGN_UP(sizeof(u32) + sizeof(Ms_DynSemaphor_t), 4) / sizeof(u32)
#define CAM_OS_SPINLOCK_SIZE    CAM_OS_ALIGN_UP(sizeof(u32), 4) / sizeof(u32)
#define CAM_OS_TIMER_SIZE       CAM_OS_ALIGN_UP(sizeof(MsTimerExt_t), 4) / sizeof(u32)
#define CAM_OS_MEMCACHE_SIZE    CAM_OS_ALIGN_UP(sizeof(u8) + sizeof(u32), 4) / sizeof(u32)
#define CAM_OS_IDR_SIZE         CAM_OS_ALIGN_UP(sizeof(void**) + sizeof(u32*) + sizeof(u32), 4) / sizeof(u32)
#define CAM_OS_CPUMASK_SIZE     CAM_OS_ALIGN_UP(sizeof(u32), 4) / sizeof(u32)

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

#define CAM_OS_MUTEX_SIZE       CAM_OS_ALIGN_UP(sizeof(u32) + sizeof(struct mutex), 4) / sizeof(u32)
#define CAM_OS_TSEM_SIZE        CAM_OS_ALIGN_UP(sizeof(u32) + sizeof(struct semaphore), 4) / sizeof(u32)
#define CAM_OS_RWTSEM_SIZE      CAM_OS_ALIGN_UP(sizeof(u32) + sizeof(struct rw_semaphore), 4) / sizeof(u32)
#define CAM_OS_TCOND_SIZE       CAM_OS_ALIGN_UP(sizeof(u32) + sizeof(struct completion), 4) / sizeof(u32)
#define CAM_OS_SPINLOCK_SIZE    CAM_OS_ALIGN_UP(sizeof(u32) + sizeof(spinlock_t) + sizeof(unsigned long), 4) / sizeof(u32)
#define CAM_OS_TIMER_SIZE       CAM_OS_ALIGN_UP(sizeof(struct timer_list), 4) / sizeof(u32)
#define CAM_OS_MEMCACHE_SIZE    CAM_OS_ALIGN_UP(sizeof(struct kmem_cache*) + 16, 4) / sizeof(u32)
#define CAM_OS_IDR_SIZE         CAM_OS_ALIGN_UP(sizeof(struct idr), 4) / sizeof(u32)
#define CAM_OS_CPUMASK_SIZE     CAM_OS_ALIGN_UP(sizeof(struct cpumask), 4) / sizeof(u32)

#else

#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define CAM_OS_MUTEX_SIZE       CAM_OS_ALIGN_UP(sizeof(u32) + sizeof(pthread_mutex_t), 4) / sizeof(u32)
#define CAM_OS_TSEM_SIZE        CAM_OS_ALIGN_UP(sizeof(u32) + sizeof(sem_t), 4) / sizeof(u32)
#define CAM_OS_RWTSEM_SIZE      CAM_OS_ALIGN_UP(sizeof(u32) + sizeof(pthread_rwlock_t), 4) / sizeof(u32)
#define CAM_OS_TCOND_SIZE       CAM_OS_ALIGN_UP(sizeof(u32) + sizeof(pthread_mutex_t) + sizeof(pthread_cond_t) + sizeof(u32), 4) / sizeof(u32)
#define CAM_OS_SPINLOCK_SIZE    CAM_OS_ALIGN_UP(sizeof(u32) + sizeof(pthread_spinlock_t), 4) / sizeof(u32)
#define CAM_OS_TIMER_SIZE       CAM_OS_ALIGN_UP(sizeof(timer_t), 4) / sizeof(u32)
#define CAM_OS_MEMCACHE_SIZE    0
#define CAM_OS_IDR_SIZE         CAM_OS_ALIGN_UP(sizeof(void **) + sizeof(unsigned long *) + sizeof(unsigned long), 4) / sizeof(u32)
#define CAM_OS_CPUMASK_SIZE     0

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__CAM_OS_STRUCT_H__
