/*
* mdrv_types.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: karl.xiao <karl.xiao@sigmastar.com.tw>
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
#ifndef __MDRV_TYPE_H__
#define __MDRV_TYPE_H__

#define    U8     unsigned char             // 1 byte
#define    U16    unsigned short            // 2 byte
#define    U32    unsigned int              // 4 byte
#define    U64    unsigned long long        // 8 byte

#define    S8     signed char               // 1 byte
#define    S16    signed short              // 2 byte
#define    S32    signed int                // 4 byte
#define    S64    signed long long          // 4 byte

#define    B16    unsigned short            // 2 byte
#define    BOOL   unsigned int              // 4 byte

#define BOOLEAN BOOL
#define WORD    U16
#define BYTE    U8

#if !defined(TRUE) && !defined(FALSE)
/// definition for TRUE
#define TRUE                           1
/// definition for FALSE
#define FALSE                          0
#endif

#ifndef true
/// definition for true
#define true                           1
/// definition for false
#define false                          0
#endif


#if !defined(TRUE) && !defined(FALSE)
/// definition for TRUE
#define TRUE                           1
/// definition for FALSE
#define FALSE                          0
#endif


#if !defined(ENABLE) && !defined(DISABLE)
/// definition for ENABLE
#define ENABLE                         1
/// definition for DISABLE
#define DISABLE                        0
#endif


#if !defined(ON) && !defined(OFF)
/// definition for ON
#define ON                             1
/// definition for OFF
#define OFF                            0
#endif

/// @name BIT#
/// definition of one bit mask
/// @{
#if !defined(BIT0) && !defined(BIT1)
#define BIT0                           0x00000001
#define BIT1                           0x00000002
#define BIT2                           0x00000004
#define BIT3                           0x00000008
#define BIT4                           0x00000010
#define BIT5                           0x00000020
#define BIT6                           0x00000040
#define BIT7                           0x00000080
#define BIT8                           0x00000100
#define BIT9                           0x00000200
#define BIT10                          0x00000400
#define BIT11                          0x00000800
#define BIT12                          0x00001000
#define BIT13                          0x00002000
#define BIT14                          0x00004000
#define BIT15                          0x00008000
#define BIT16                          0x00010000
#define BIT17                          0x00020000
#define BIT18                          0x00040000
#define BIT19                          0x00080000
#define BIT20                          0x00100000
#define BIT21                          0x00200000
#define BIT22                          0x00400000
#define BIT23                          0x00800000
#define BIT24                          0x01000000
#define BIT25                          0x02000000
#define BIT26                          0x04000000
#define BIT27                          0x08000000
#define BIT28                          0x10000000
#define BIT29                          0x20000000
#define BIT30                          0x40000000
#define BIT31                          0x80000000
#endif
/// @}
#endif //

