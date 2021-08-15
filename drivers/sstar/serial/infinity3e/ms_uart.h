/*
* ms_uart.h- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: edie.chen <edie.chen@sigmastar.com.tw>
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

#ifndef _MS_UART_H_
#define _MS_UART_H_

#define MUX_PM_UART 0
#define MUX_FUART   1
#define MUX_UART0   2
#define MUX_UART1   3

int ms_uart_get_padmux(int tx_pad, u8 *padmux, u8 *pad_mode);

#endif
