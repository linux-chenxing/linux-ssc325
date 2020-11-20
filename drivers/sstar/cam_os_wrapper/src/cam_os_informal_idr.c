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


///////////////////////////////////////////////////////////////////////////////
/// @file      cam_os_informal_idr.c
/// @brief     Cam OS Informal IDR Source File for Linux User Space and RTK.
///            It's Not A Standard IDR Algorithm.
///////////////////////////////////////////////////////////////////////////////

#if defined(CAM_OS_RTK) || defined(CAM_OS_LINUX_USER)
#include <stdio.h>
#include <string.h>
#include "cam_os_wrapper.h"
#include "cam_os_util_bitmap.h"

#define IDR_ENTRY_NUM   0x10000

typedef struct
{
    void **ppEntryPtr;
    unsigned long *pBitmap;
} CamOsInformalIdr_t, *pCamOsInformalIdr;

CamOsRet_e _CamOsIdrInit(CamOsIdr_t *ptIdr)
{
    CamOsRet_e eRet = CAM_OS_OK;
    CamOsInformalIdr_t *pInformalIdr = (CamOsInformalIdr_t *)ptIdr;

    if (pInformalIdr)
    {
        if (NULL == (pInformalIdr->ppEntryPtr = CamOsMemAlloc(sizeof(void *)*IDR_ENTRY_NUM)))
            eRet = CAM_OS_ALLOCMEM_FAIL;

        if (NULL == (pInformalIdr->pBitmap = CamOsMemAlloc(sizeof(unsigned long)*CAM_OS_BITS_TO_LONGS(IDR_ENTRY_NUM))))
        {
            CamOsMemRelease(pInformalIdr->ppEntryPtr);
            pInformalIdr->ppEntryPtr = NULL;
            eRet = CAM_OS_ALLOCMEM_FAIL;
        }

        memset(pInformalIdr->ppEntryPtr, 0, sizeof(void *)*IDR_ENTRY_NUM);
        memset(pInformalIdr->pBitmap, 0, sizeof(unsigned long)*CAM_OS_BITS_TO_LONGS(IDR_ENTRY_NUM));
    }
    else
        eRet = CAM_OS_PARAM_ERR;

    return eRet;
}

void _CamOsIdrDestroy(CamOsIdr_t *ptIdr)
{
    CamOsInformalIdr_t *pInformalIdr = (CamOsInformalIdr_t *)ptIdr;

    if (pInformalIdr)
    {
        if (pInformalIdr->ppEntryPtr)
            CamOsMemRelease(pInformalIdr->ppEntryPtr);

        if (pInformalIdr->pBitmap)
            CamOsMemRelease(pInformalIdr->pBitmap);
    }
}

s32 _CamOsIdrAlloc(CamOsIdr_t *ptIdr, void *pDataPtr, s32 nStart, s32 nEnd)
{
    CamOsInformalIdr_t *pInformalIdr = (CamOsInformalIdr_t *)ptIdr;
    s32 nEmptyID=-1;

    if (pInformalIdr && pDataPtr && pInformalIdr->ppEntryPtr)
    {
        if (nEnd < nStart || nEnd == 0)
            nEnd = IDR_ENTRY_NUM - 1;

        nEmptyID = CAM_OS_FIND_NEXT_ZERO_BIT(pInformalIdr->pBitmap, IDR_ENTRY_NUM, nStart);

        if (nEmptyID < nStart || nEmptyID > nEnd)
            nEmptyID = -1;
        else
        {
            pInformalIdr->ppEntryPtr[nEmptyID] = pDataPtr;
            CAM_OS_SET_BIT(nEmptyID, pInformalIdr->pBitmap);
        }
    }

    return nEmptyID;
}

void _CamOsIdrRemove(CamOsIdr_t *ptIdr, s32 nId)
{
    CamOsInformalIdr_t *pInformalIdr = (CamOsInformalIdr_t *)ptIdr;

    if (pInformalIdr && pInformalIdr->ppEntryPtr)
    {
        pInformalIdr->ppEntryPtr[nId] = NULL;
        CAM_OS_CLEAR_BIT(nId, pInformalIdr->pBitmap);
    }
}

void *_CamOsIdrFind(CamOsIdr_t *ptIdr, s32 nId)
{
    CamOsInformalIdr_t *pInformalIdr = (CamOsInformalIdr_t *)ptIdr;

    if (pInformalIdr && pInformalIdr->ppEntryPtr)
    {
        if (CAM_OS_TEST_BIT(nId, pInformalIdr->pBitmap))
        {
            return pInformalIdr->ppEntryPtr[nId];
        }
    }

    return NULL;
}
#endif
