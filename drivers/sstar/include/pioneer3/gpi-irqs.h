/*
* gpi-irqs.h- Sigmastar
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
/*------------------------------------------------------------------------------

------------------------------------------------------------------------------*/


#define GPI_FIQ_START                   0
#define INT_GPI_FIQ_PAD_SR_IO00             (GPI_FIQ_START + 0)
#define INT_GPI_FIQ_PAD_SR_IO01             (GPI_FIQ_START + 1)
#define INT_GPI_FIQ_PAD_SR_IO02             (GPI_FIQ_START + 2)
#define INT_GPI_FIQ_PAD_SR_IO03             (GPI_FIQ_START + 3)
#define INT_GPI_FIQ_PAD_SR_IO04             (GPI_FIQ_START + 4)
#define INT_GPI_FIQ_PAD_SR_IO05             (GPI_FIQ_START + 5)
#define INT_GPI_FIQ_PAD_SR_IO06             (GPI_FIQ_START + 6)
#define INT_GPI_FIQ_PAD_SR_IO07             (GPI_FIQ_START + 7)
#define INT_GPI_FIQ_PAD_SR_IO08             (GPI_FIQ_START + 8)
#define INT_GPI_FIQ_PAD_SR_IO09             (GPI_FIQ_START + 9)
#define INT_GPI_FIQ_PAD_SR_IO10             (GPI_FIQ_START + 10)
#define INT_GPI_FIQ_PAD_SR_IO11             (GPI_FIQ_START + 11)
#define INT_GPI_FIQ_PAD_SR_IO12             (GPI_FIQ_START + 12)
#define INT_GPI_FIQ_PAD_SR_IO13             (GPI_FIQ_START + 13)
#define INT_GPI_FIQ_PAD_SR_IO14             (GPI_FIQ_START + 14)
#define INT_GPI_FIQ_PAD_SR_IO15             (GPI_FIQ_START + 15)
#define INT_GPI_FIQ_PAD_SR_IO16             (GPI_FIQ_START + 16)
#define INT_GPI_FIQ_PAD_TTL0              (GPI_FIQ_START + 17)
#define INT_GPI_FIQ_PAD_TTL1              (GPI_FIQ_START + 18)
#define INT_GPI_FIQ_PAD_TTL2              (GPI_FIQ_START + 19)
#define INT_GPI_FIQ_PAD_TTL3              (GPI_FIQ_START + 20)
#define INT_GPI_FIQ_PAD_TTL4              (GPI_FIQ_START + 21)
#define INT_GPI_FIQ_PAD_TTL5              (GPI_FIQ_START + 22)
#define INT_GPI_FIQ_PAD_TTL6              (GPI_FIQ_START + 23)
#define INT_GPI_FIQ_PAD_TTL7              (GPI_FIQ_START + 24)
#define INT_GPI_FIQ_PAD_TTL8              (GPI_FIQ_START + 25)
#define INT_GPI_FIQ_PAD_TTL9              (GPI_FIQ_START + 26)
#define INT_GPI_FIQ_PAD_TTL10       (GPI_FIQ_START + 27)
#define INT_GPI_FIQ_PAD_TTL11       (GPI_FIQ_START + 28)
#define INT_GPI_FIQ_PAD_TTL12       (GPI_FIQ_START + 29)
#define INT_GPI_FIQ_PAD_TTL13       (GPI_FIQ_START + 30)
#define INT_GPI_FIQ_PAD_TTL14       (GPI_FIQ_START + 31)
#define INT_GPI_FIQ_PAD_TTL15       (GPI_FIQ_START + 32)
#define INT_GPI_FIQ_PAD_TTL16       (GPI_FIQ_START + 33)
#define INT_GPI_FIQ_PAD_TTL17       (GPI_FIQ_START + 34)
#define INT_GPI_FIQ_PAD_TTL18       (GPI_FIQ_START + 35)
#define INT_GPI_FIQ_PAD_TTL19       (GPI_FIQ_START + 36)
#define INT_GPI_FIQ_PAD_TTL20       (GPI_FIQ_START + 37)
#define INT_GPI_FIQ_PAD_TTL21       (GPI_FIQ_START + 38)
#define INT_GPI_FIQ_PAD_KEY0        (GPI_FIQ_START + 39)
#define INT_GPI_FIQ_PAD_KEY1        (GPI_FIQ_START + 40)
#define INT_GPI_FIQ_PAD_KEY2        (GPI_FIQ_START + 41)
#define INT_GPI_FIQ_PAD_KEY3        (GPI_FIQ_START + 42)
#define INT_GPI_FIQ_PAD_KEY4        (GPI_FIQ_START + 43)
#define INT_GPI_FIQ_PAD_KEY5        (GPI_FIQ_START + 44)
#define INT_GPI_FIQ_PAD_KEY6        (GPI_FIQ_START + 45)
#define INT_GPI_FIQ_PAD_KEY7        (GPI_FIQ_START + 46)
#define INT_GPI_FIQ_PAD_KEY8        (GPI_FIQ_START + 47)
#define INT_GPI_FIQ_PAD_KEY9        (GPI_FIQ_START + 48)
#define INT_GPI_FIQ_PAD_KEY10       (GPI_FIQ_START + 49)
#define INT_GPI_FIQ_PAD_KEY11       (GPI_FIQ_START + 50)
#define INT_GPI_FIQ_PAD_KEY12       (GPI_FIQ_START + 51)
#define INT_GPI_FIQ_PAD_KEY13       (GPI_FIQ_START + 52)
#define INT_GPI_FIQ_PAD_SD_D1       (GPI_FIQ_START + 53)
#define INT_GPI_FIQ_PAD_SD_D0       (GPI_FIQ_START + 54)
#define INT_GPI_FIQ_PAD_SD_CLK      (GPI_FIQ_START + 55)
#define INT_GPI_FIQ_PAD_SD_CMD      (GPI_FIQ_START + 56)
#define INT_GPI_FIQ_PAD_SD_D3       (GPI_FIQ_START + 57)
#define INT_GPI_FIQ_PAD_SD_D2       (GPI_FIQ_START + 58)
#define INT_GPI_FIQ_PAD_SD_GPIO0    (GPI_FIQ_START + 59)
#define INT_GPI_FIQ_PAD_SD_GPIO1    (GPI_FIQ_START + 60)
#define INT_GPI_FIQ_PAD_GPIO0       (GPI_FIQ_START + 61)
#define INT_GPI_FIQ_PAD_GPIO1       (GPI_FIQ_START + 62)
#define INT_GPI_FIQ_PAD_GPIO2       (GPI_FIQ_START + 63)
#define INT_GPI_FIQ_PAD_GPIO3       (GPI_FIQ_START + 64)
#define INT_GPI_FIQ_PAD_GPIO4       (GPI_FIQ_START + 65)
#define INT_GPI_FIQ_PAD_GPIO5       (GPI_FIQ_START + 66)
#define INT_GPI_FIQ_PAD_GPIO6       (GPI_FIQ_START + 67)
#define INT_GPI_FIQ_PAD_GPIO7       (GPI_FIQ_START + 68)
#define INT_GPI_FIQ_PAD_GPIO8       (GPI_FIQ_START + 69)
#define INT_GPI_FIQ_PAD_GPIO9       (GPI_FIQ_START + 70)
#define GPI_FIQ_END                     (GPI_FIQ_START + 71)
#define GPI_FIQ_NUM                     (GPI_FIQ_END - GPI_FIQ_START)

#define GPI_IRQ_START                   0
#define INT_GPI_IRQ_DUMMY00             (GPI_IRQ_START +  0)
#define INT_GPI_IRQ_DUMMY01             (GPI_IRQ_START +  1)
#define INT_GPI_IRQ_DUMMY02             (GPI_IRQ_START +  2)
#define INT_GPI_IRQ_DUMMY03             (GPI_IRQ_START +  3)
#define INT_GPI_IRQ_DUMMY04             (GPI_IRQ_START +  4)
#define INT_GPI_IRQ_DUMMY05             (GPI_IRQ_START +  5)
#define INT_GPI_IRQ_DUMMY06             (GPI_IRQ_START +  6)
#define INT_GPI_IRQ_DUMMY07             (GPI_IRQ_START +  7)
#define GPI_IRQ_END                     (GPI_IRQ_START +  8)
#define GPI_IRQ_NUM                     (GPI_IRQ_END - GPI_IRQ_START)
