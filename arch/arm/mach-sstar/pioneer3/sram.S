/*
* sram.S- Sigmastar
*
* Copyright (C) 2018 Sigmastar Technology Corp.
*
* Author: XXXX <XXXX@sigmastar.com.tw>
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
/*------------------------------------------------------------------------------
    Function Code
-------------------------------------------------------------------------------*/
#include <linux/linkage.h>
#include <asm/assembler.h>
#include <asm/asm-offsets.h>
#include <asm/hardware/cache-l2x0.h>

#define ARCH_us(x) (6*x)
    .align 3
    .globl sram_suspend_imi
    .globl v7_cpu_resume

/* A macro about using arch timer to delay
    inputs :
    t : us to delay
    l : For different branch naming
*/
.macro arch_usdelay t, l
    ldr   r8, =ARCH_us(\t)     /*delay tms*/
    mrrc  p15, 0, r9, r10, cr14
    adds  r9, r9, r8            /* The LSB of target tick */
    adc   r10, r10, #0x00       /* Add with carry instruction */
delay_retry\l:
    isb sy
    mrrc  p15, 0, r11, r12, cr14
    cmp   r12, r10              /* compare MSB part*/
    blt   delay_retry\l         /* jump to delay_retry in case r10 is bigger (N==1) */
    cmp   r11, r9               /* compare LSB part*/
    blt   delay_retry\l         /* jump to delay_retry in case r9 is bigger (N==1) */
.endm

////WREG_B
.macro  WREG_B  reg_addr32, val
        ldr     r7, =\reg_addr32
        ldr     r6, =\val
        strb    r6, [r7]
.endm

////RREG
.macro  RREG    dest_reg, reg_addr32
        ldr     r7, =\reg_addr32
        ldr     \dest_reg, [r7]
.endm

////RREG_B
.macro  RREG_B  dest_reg, reg_addr32
        ldr     r7, =\reg_addr32
        ldrb    \dest_reg, [r7]
.endm

////WREG
.macro  WREG    reg_addr32, vall
        ldr     r7, =\reg_addr32
        ldr     r6, =\vall
        str     r6, [r7]
        b 1011f
        .ltorg
1011:
.endm

ENTRY(sram_suspend_imi)
	//Below flow is run at SRAM(IMI)
	// set debug flag
	// wriu    0x100400 0x3333
	WREG    0xFD200800, 0x3333

	// 1. DDR enter self-refresh
	// wriu -w   0x1012e0  0x0000
	// wriu -w   0x101246  0xfffe
	// wriu -w   0x101266  0xffff
	// wriu -w   0x101286  0xffff
	// wriu -w   0x1012a6  0xffff
	// wriu -w   0x1011e6  0x00fe
	// wait  20

	ldr   r1, =0xFD000000
	ldr   r3, =0x101200
	ldr   r4, =0x101100
	ldr   r5, =0x101000
	add   r2, r1, r3, lsl #1
	ldr   r0, =0x0000
	str   r0, [r2, #0xE0 << 1]
	ldr   r0, =0xFFFE
	str   r0, [r2, #0x46 << 1]
	ldr   r0, =0xFFFF
	str   r0, [r2, #0x66 << 1]
	str   r0, [r2, #0x86 << 1]
	str   r0, [r2, #0xA6 << 1]
	add   r2, r1, r4, lsl #1
	ldr   r0, =0x00FE
	str   r0, [r2, #0xE6 << 1]
	//delay
	arch_usdelay 1000,__LINE__

	//Pre-charge all then one refresh
	//wriu -w   0x101218  0x0400
	add   r2, r1, r3, lsl #1
	ldr   r0, =0x0400
	str   r0, [r2, #0x18 << 1]
	//wriu -b   0x101200  0x30 0x20
	ldrb  r0, [r2, #0x00]
	and   r0, r0, #0xCF
	orr   r0, r0, #0x20
	strb  r0, [r2, #0x00]
	//wriu -b   0x101201  0x1e 0x04
	ldrb  r0, [r2, #0x01]
	and   r0, r0, #0xE1
	orr   r0, r0, #0x04
	strb  r0, [r2, #0x01]
	//wriu -b   0x101201  0x01 0x00
	ldrb  r0, [r2, #0x01]
	and   r0, r0, #0xFE
	strb  r0, [r2, #0x01]
	//wriu -b   0x101201  0x01 0x01
	ldrb  r0, [r2, #0x01]
	orr   r0, r0, #0x01
	strb  r0, [r2, #0x01]
	//wriu -b   0x101201  0x01 0x00
	ldrb  r0, [r2, #0x01]
	and   r0, r0, #0xFE
	strb  r0, [r2, #0x01]
	//wriu -b   0x101201  0x1e 0x02
	ldrb  r0, [r2, #0x01]
	and   r0, r0, #0xE1
	orr   r0, r0, #0x02
	strb  r0, [r2, #0x01]
	//wriu -b   0x101201  0x01 0x00
	ldrb  r0, [r2, #0x01]
	and   r0, r0, #0xFE
	strb  r0, [r2, #0x01]
	//wriu -b   0x101201  0x01 0x01
	ldrb  r0, [r2, #0x01]
	orr   r0, r0, #0x01
	strb  r0, [r2, #0x01]
	//wriu -b   0x101201  0x01 0x00
	ldrb  r0, [r2, #0x01]
	and   r0, r0, #0xFE
	strb  r0, [r2, #0x01]
	//delay, DO NOT REMOVE THIS DELAY!!!!
	arch_usdelay 1000,__LINE__

	//wriu -w   0x101200  0x202e //Enter self-refresh
	ldr   r0, =0x202e
	str   r0, [r2, #0x00 << 1]
	//wait 10
	//delay
	arch_usdelay 1000,__LINE__

	// 05. AN power down
	//wriu -b 0x101203 0xF0 0xF0
	ldrb  r0, [r2, #0x05]
	orr   r0, r0, #0xF0
	strb  r0, [r2, #0x05]
	//wriu -b 0x101000 0x08 0x08
	//wriu -b 0x101000 0x10 0x10
	add   r2, r1, r5, lsl #1
	ldrb  r0, [r2, #0x00 << 1]
	orr   r0, r0, #0x08
	strb  r0, [r2, #0x00 << 1]
	ldrb  r0, [r2, #0x00 << 1]
	orr   r0, r0, #0x10
	strb  r0, [r2, #0x00 << 1]

	WREG    0xFD200800, 0x3322

	//wriu -b 0x101054 0x70 0x70
	ldrb  r0, [r2, #0xA8]
	orr   r0, r0, #0x70
	strb  r0, [r2, #0xA8]
	//wriu -b 0x101008 0x3F 0x00
	ldrb  r0, [r2, #0x10]
	and   r0, r0, #0xC0
	strb  r0, [r2, #0x10]
 
	// set debug flag
	// wriu    0x100400 0x3333
	WREG    0xFD200800, 0x3334

	// ETH power down
	WREG_B    0xFD0067E4, 0x80  //	wriu 0x000033f2 0x80
	WREG_B    0xFD0067E5, 0x0f  //	wriu 0x000033f3 0x0f
	WREG_B    0xFD00647D, 0x40  //	wriu 0x0000323f 0x40
	WREG_B    0xFD006575, 0x40  //	wriu 0x000032bb 0x40 //[6]: PD_REF
	WREG_B    0xFD006598, 0x50  //	wriu 0x000032cc 0x50 //[4]: PD_ETHPLL_REG
	WREG_B    0xFD00656D, 0x10  //	wriu 0x000032b7 0x10 //[4]: PD_ADC
	WREG_B    0xFD0065F8, 0x02  //	wriu 0x000032fc 0x02 //[1]: PD_REG25
	WREG_B    0xFD0065F9, 0x01  //	wriu 0x000032fd 0x01 //[0]: PD_LDO11
	WREG_B    0xFD0065A5, 0xc0  //	wriu 0x000032d3 0xc0 //[15:14]: TX_OFF & LPF_INOFF
	WREG_B    0xFD006710, 0x10  //	wriu 0x00003388 0x10 //[4]: RX_OFF
	WREG_B    0xFD006741, 0x30  //	wriu 0x000033a1 0x30
	WREG_B    0xFD006674, 0x03  // 	wriu 0x0000333a 0x03
	WREG_B    0xFD0067E1, 0x3c  //	wriu 0x000033f1 0x3c
	WREG    0xFD200800, 0x3335
	// IDAC_PLL power down
	WREG_B    0xFD2072D8, 0x01  // wriu 0x0010396c 0x01
	// LPLL power down	
	WREG_B    0xFD206701, 0x80  // wriu 0x00103381 0x80
	WREG    0xFD200800, 0x3336
	// SATA power down
	WREG_B    0xFD2A4E80, 0x01  //wriu 0x00152740 0x01
	WREG_B    0xFD2A4EC0, 0x01  //wriu 0x00152760 0x01
	WREG_B    0xFD2A4EC4, 0x04  //wriu 0x00152763 0x04
	WREG_B    0xFD2A4C20, 0xff  //wriu 0x00152610 0xff
	WREG_B    0xFD2A4C21, 0xff   //wriu 0x00152611 0xff
	WREG_B    0xFD2A4C00, 0x5b  //wriu 0x00152600 0x5b
	WREG_B    0xFD2A4C01, 0x10  //wriu 0x00152601 0x10
	WREG_B    0xFD2A4C04, 0x00  //wriu 0x00152602 0x00
	WREG    0xFD200800, 0x3337

	//check Wakeup Source
	RREG r0, 0xFD203CC0
	and r0, #0x100 //USB_EN
	cmp r0, #1
	beq SKIP_USB_PW

	// UPLL_0 power down
	WREG_B    0xFD284000, 0x32  //	wriu 0x00142000 0x32
	WREG_B    0xFD284001, 0x00  //	wriu 0x00142001 0x00
	WREG_B    0xFD284004, 0x00  //	wriu 0x00142002 0x00
	WREG_B    0xFD284008, 0x00  //	wriu 0x00142004 0x00
	WREG_B    0xFD284009, 0x00  //	wriu 0x00142005 0x00
	WREG_B    0xFD28401C, 0x04  //	wriu 0x0014200e 0x04
	WREG    0xFD200800, 0x3338
	// USB2.0 P1 power down
	WREG_B    0xFD284200, 0x05  //wriu 0x00142100 0x05
	WREG_B    0xFD284201, 0xff  //wriu 0x00142101 0xff
	WREG_B    0xFD284210, 0x80  //wriu 0x00142108 0x80
	WREG_B    0xFD284211, 0x00  //wriu 0x00142109 0x00
	WREG_B    0xFD284220, 0x00  //wriu 0x00142110 0x00
	WREG    0xFD200800, 0x3339
	//MIPIRX
	WREG_B    0xFD2A7c38, 0x01  //wriu 0x153e1c 0x01 //[0]: 0 for only 1 RX
	WREG_B    0xFD2A7c44, 0x00  //wriu 0x153e22 0x00 //[1:0]: 00 for 2lane RX
	WREG_B    0xFD2A7000, 0x40  //wriu 0x153800 0x40 //[6]: PD_LDO
	WREG_B    0xFD2A7004, 0x03  //wriu 0x153802 0x03 //[0]: PD_IB_DPHY
	WREG_B    0xFD2A7010, 0xa0  //wriu 0x153808 0xa0 //[4]: LPTX0_EN; [6]: LPRX0_EN
	WREG_B    0xFD2A7011, 0x04  //wriu 0x153809 0x04 //[1:0]: HSTX0_EN[1:0]
	WREG_B    0xFD2A7020, 0xa0  //wriu 0x153810 0xa0 //[4]: LPTX1_EN; [6]: LPRX1_EN
	WREG_B    0xFD2A7021, 0x04  //wriu 0x153811 0x04 //[1:0]: HSTX1_EN[1:0]
	WREG_B    0xFD2A7030, 0xa0  //wriu 0x153818 0xa0 //[4]: LPTX2_EN; [6]: LPRX2_EN
	WREG_B    0xFD2A7031, 0x04  //wriu 0x153819 0x04 //[1:0]: HSTX2_EN[1:0]
	WREG_B    0xFD2A7015, 0x08  //wriu 0x15380b 0x08 //[2]: HSRX0_EN
	WREG_B    0xFD2A7025, 0x08  //wriu 0x153813 0x08 //[2]: HSRX1_EN
	WREG_B    0xFD2A7035, 0x08  //wriu 0x15381b 0x08 //[2]: HSRX2_EN
#if 0
	// AUDIO power down --audio_pd.txt
	WREG_B    0xFD20680C, 0xFF  //wriu 0x00103406 0xFF
	WREG_B    0xFD20680D, 0xFF  //wriu 0x00103407 0xFF
	WREG_B    0xFD20682C, 0xAD  //wriu 0x00103416 0xAD
	WREG_B    0xFD20680D, 0x01  //wriu 0x00103407 0x01
	WREG    0xFD200800, 0x333A
#endif

SKIP_USB_PW:
	// AUDIO power down --sync from edward.tsai (2020/10/22)
	WREG    0xFD206800, 0x0000  //wriu -w 0x103400 0x0000
	WREG    0xFD206804, 0x0000  //wriu -w 0x103402 0x0000
	WREG    0xFD20680C, 0xFFFF  //wriu -w 0x103406 0xFFFF
	WREG    0xFD20682C, 0xFFFF  //wriu 0x00103416 0xFFFF

	WREG    0xFD200800, 0x333A
	// set ckg_mcu to xtali
	ldr r1, =0xFD000000
	ldr r3, =0x103800
	add   r2, r1, r3, lsl #1
	ldrb  r0, [r2, #0x4]
	and   r0, r0, #0xDF
	strb  r0, [r2, #0x4]

	//switch L3 bridge clock to xtal
	WREG_B    0xFD204404, 0x04
	//switch MIU clock to xtal(12MHz)
	WREG_B    0xFD20705C, 0x08

	// miupll power down
	//wriu    0x00103103 0x01
	WREG_B    0xFD206205, 0x01
	// set debug flag
	WREG    0xFD200800, 0x333B
	//armpll power down
	//switch ARM clock from xtal(12MHz) to ARM PLL
	WREG_B    0xFD2041F0, 0x00
	//ARM PLL PD 
	WREG_B    0xFD206445, 0x01
	// set debug flag
	WREG    0xFD200800, 0x333C

	// MPLL power down
	//WREG_B    0xFD206005, 0x1f  //wriu 0x00103003 0x1f >>>>halt
	WREG_B    0xFD206005, 0x1f
	WREG_B    0xFD206008, 0x00  //wriu 0x00103004 0x00
	WREG_B    0xFD206010, 0x00  //wriu 0x00103008 0x00
	WREG_B    0xFD206011, 0x00  //wriu 0x00103009 0x00
	//wriu -b 0x101033 0xc0 0xc0 // [15:14] pll pd
	movw r2, #0x2000
	movt r2, #0xFD20
	ldrb  r0, [r2, #0x65]
	orr   r0, r0, #0xc0
	strb  r0, [r2, #0x65]

	WREG    0xFD200800, 0x3311

    //wriu -b 0x10108b 0x02 0x02  // [9] treeLDO PD
	ldrb  r0, [r2, #0x115]
	orr   r0, r0, #0x02
	strb  r0, [r2, #0x115]
	WREG    0xFD200800, 0x3222
    //wriu -b 0x101033 0x40 0x40 // [15:14] MasterPLL  pd
	ldrb  r0, [r2, #0x65]
	orr   r0, r0, #0x40
	strb  r0, [r2, #0x65]

	WREG    0xFD200800, 0x3233
    //wriu -b 0x101033 0x80 0x80 // [15:14] SlavePLL  pd
	ldrb  r0, [r2, #0x65]
	orr   r0, r0, #0x80
	strb  r0, [r2, #0x65]



	WREG    0xFD200800, 0x333D

	//check REG_WAKEUP_XTAL_IRQ_EN
	RREG r0, 0xFD203CC0 
	and r0, #0x80
	cmp r0, #0
	beq standby

gate_xtal:
	//sleep event
	WREG_B    0xFD203CC0, 0x8F
	/* TODO*/
	/* 1. do wfi*/
	/* 2. nop nop nop nop*/
	/* 3. TODO PLL power up*/
	/* 4. MIU init*/
	/* 5. exit self refresh*/
	/* 6. Jump to resume entry*/
	nop
	nop
	nop
	nop
	nop
	// soft reset
	// wriu    0x000e5C 0x79  //0xe, 0x2e=0x79
	WREG    0xFD001CB8, 0x79

standby:
	wfi
	nop
	nop
	nop
	nop
	// soft reset
	// wriu    0x000e5C 0x79  //0xe, 0x2e=0x79
	WREG    0xFD001CB8, 0x79

ENDPROC(sram_suspend_imi)
.ltorg
