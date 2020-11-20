/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

 Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
Sigmastar Technology Corp. and be kept in strict confidence
(Sigmastar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of Sigmastar Confidential
Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/


#ifndef __CAM_OS_UTIL_STRING_H__
#define __CAM_OS_UTIL_STRING_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(__KERNEL__)
#include "linux/kernel.h"
#include "linux/string.h"
#include "linux/sort.h"
#else
#include "string.h"
#include "stdlib.h"
#endif

#if defined(__KERNEL__)
#define atoi(s)             simple_strtol(s, NULL, 10)
#define qsort(b,n,s,c)      sort(b,n,s,c,NULL)
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifndef KERN_SOH
#define KERN_SOH        "\001"          /* ASCII Start Of Header */
#endif

#ifndef KERN_EMERG
#define KERN_EMERG      KERN_SOH "0"    /* system is unusable */
#endif

#ifndef KERN_ALERT
#define KERN_ALERT      KERN_SOH "1"    /* action must be taken immediately */
#endif

#ifndef KERN_CRIT
#define KERN_CRIT       KERN_SOH "2"    /* critical conditions */
#endif

#ifndef KERN_ERR
#define KERN_ERR        KERN_SOH "3"    /* error conditions */
#endif

#ifndef KERN_WARNING
#define KERN_WARNING    KERN_SOH "4"    /* warning conditions */
#endif

#ifndef KERN_NOTICE
#define KERN_NOTICE     KERN_SOH "5"    /* normal but significant condition */
#endif

#ifndef KERN_INFO
#define KERN_INFO       KERN_SOH "6"    /* informational */
#endif

#ifndef KERN_DEBUG
#define KERN_DEBUG      KERN_SOH "7"    /* debug-level messages */
#endif

#endif //__CAM_OS_UTIL_STRING_H__
