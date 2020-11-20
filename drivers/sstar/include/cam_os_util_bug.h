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


#ifndef __CAM_OS_UTIL_BUG_H__
#define __CAM_OS_UTIL_BUG_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define CAM_OS_BUG() do { \
	CamOsPrintf("BUG: failure at %s:%d/%s()!\n", __FILE__, __LINE__, __func__); \
	CamOsPanic("BUG!"); \
} while (0)

#define CAM_OS_BUG_ON(condition) do { if (unlikely(condition)) CAM_OS_BUG(); } while (0)

#define CAM_OS_MAX_ERRNO   4096
#define CAM_OS_IS_ERR_VALUE(x) CAM_OS_UNLIKELY((x) >= (unsigned long)-CAM_OS_MAX_ERRNO)

#define CAM_OS_ERR_PTR(x) (void *)(x)

#define CAM_OS_PTR_ERR(x) (long)(x)

#define CAM_OS_IS_ERR(x) CAM_OS_IS_ERR_VALUE((unsigned long)(x))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__CAM_OS_UTIL_BUG_H__
