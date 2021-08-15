////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006-2007 MStar Semiconductor, Inc.
// All rights reserved.
//
// Unless otherwise stipulated in writing, any and all information contained
// herein regardless in any format shall remain the sole proprietary of
// MStar Semiconductor Inc. and be kept in strict confidence
// (“MStar Confidential Information”) by the recipien
// Any unauthorized act including without limitation unauthorized disclosure,
// copying, use, reproduction, sale, distribution, modification, disassembling,
// reverse engineering and compiling of the contents of MStar Confidential
// Information is unlawful and strictly prohibited. MStar hereby reserves the
// rights to any and all damages, losses, costs and expenses resulting therefrom.
//
////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
///
/// @file   MDRV_NOE_ETHTOOL.h
/// @brief  NOE Driver
/// @author MStar Semiconductor Inc.
///
///////////////////////////////////////////////////////////////////////////////////////////////////




#ifndef _MDRV_NOE_ETHTOOL_H_
#define _MDRV_NOE_ETHTOOL_H_

unsigned char MDrv_NOE_ETHTOOL_Get_Phy_Address(void);
u32 MDrv_NOE_ETHTOOL_Get_Link(struct net_device *dev);
int MDrv_NOE_ETHTOOL_Get_Settings(struct net_device *dev, struct ethtool_cmd *cmd);
int MDrv_NOE_ETHTOOL_Set_Settings(struct net_device *dev, struct ethtool_cmd *cmd);
void MDrv_NOE_ETHTOOL_Init(struct net_device *dev);
u32 MDrv_NOE_ETHTOOL_Virt_Get_Link(struct net_device *dev);
int MDrv_NOE_ETHTOOL_Virt_Set_Settings(struct net_device *dev, struct ethtool_cmd *cmd);
int MDrv_NOE_ETHTOOL_Virt_Get_Settings(struct net_device *dev, struct ethtool_cmd *cmd);
void MDrv_NOE_ETHTOOL_Virt_Init(struct net_device *dev);

#endif /* _MDRV_NOE_ETHTOOL_H_ */

