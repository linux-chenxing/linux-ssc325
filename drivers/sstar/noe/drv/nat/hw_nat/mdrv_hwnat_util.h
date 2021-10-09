/*
* mdrv_hwnat_util.h- Sigmastar
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
#ifndef _MDRV_HWNAT_UTIL_H_
#define _MDRV_HWNAT_UTIL_H_

//-------------------------------------------------------------------------------------------------
//  Include files
//-------------------------------------------------------------------------------------------------

#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>



uint8_t *MDrv_HWNAT_Util_Ip_To_Str(uint32_t Ip);
unsigned int MDrv_HWNAT_Util_Str_To_Ip(char *str);
void MDrv_HWNAT_Util_Calc_Tcphdr(struct foe_entry *entry, struct iphdr *iph, struct tcphdr *th);
void MDrv_HWNAT_Util_Calc_Udphdr(struct foe_entry *entry, struct iphdr *iph, struct udphdr *uh);
void MDrv_HWNAT_Util_Calc_Iphdr(struct foe_entry *entry, struct iphdr *iph);
void MDrv_HWNAT_Util_Memcpy(void *dest, void *src, u32 n);
void MDrv_HWNAT_CalIpRange(uint32_t StartIp, uint32_t EndIp, uint8_t * M, uint8_t * E);
unsigned int MDrv_HWNAT_Str2Ip(char *str);



#endif
