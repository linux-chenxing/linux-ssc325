/*
* uart_pads.c- Sigmastar
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

#include <linux/serial.h>

#include "ms_uart.h"
#include "gpio.h"
#include "ms_platform.h"

#define REG_UART_MODE          0x1F2079B4
#define REG_FUART_MODE         0x1F2079B8
#define REG_UART_SEL           0x1F203D4C
#define REG_UART_SEL4          0x1F203D50
#define REG_FORCE_RX_DISABLE   0x1F203D5C


void ms_uart_select_pad( u8 select, u8 padmux, u8 pad_mode)
{
    switch(padmux)
    {
        case MUX_PM_UART:
            OUTREGMSK16(REG_UART_SEL, select << 0, 0xF << 0);
            break;
        case MUX_FUART:
            OUTREGMSK16(REG_UART_SEL, select << 4, 0xF << 4);
            OUTREGMSK16(REG_FUART_MODE, pad_mode << 8, 0xF << 8);
            break;
        case MUX_UART0:
            OUTREGMSK16(REG_UART_SEL, select << 8, 0xF << 8);
            OUTREGMSK16(REG_UART_MODE, pad_mode << 0, 0x7 << 0);
            break;
        case MUX_UART1:
            OUTREGMSK16(REG_UART_SEL, select << 12, 0xF << 12);
            OUTREGMSK16(REG_UART_MODE, pad_mode << 4, 0xF << 4);
            break;
#ifdef CONFIG_MS_SUPPORT_UART2
        case MUX_UART2:
            OUTREGMSK16(REG_UART_SEL4, select << 0, 0xF << 0);
            OUTREGMSK16(REG_UART_MODE, pad_mode << 8, 0x7 << 8);
            break;
#endif
        default:
                printk("[%s] TBD !!\n", __func__);
            break;
    }
}
int ms_uart_get_padmux(int tx_pad, u8 *padmux, u8 *pad_mode)
{
    *pad_mode = tx_pad;
    return 0;
}
