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


#ifndef __CAM_OS_WRAPPER_TEST_H__
#define __CAM_OS_WRAPPER_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef CAM_OS_RTK
s32 CamOsWrapperTest(CLI_t *pCli, char *p);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__CAM_OS_WRAPPER_TEST_H__
