#include "gpio_table.h"

#define _CONCAT( a, b )     a##b
#define CONCAT( a, b )      _CONCAT( a, b )

#define GPIO0_PAD        PAD_GPIO0
#define GPIO0_OEN        0x103C00, BIT5
#define GPIO0_OUT        0x103C00, BIT4
#define GPIO0_IN         0x103C00, BIT0

#define GPIO1_PAD        PAD_GPIO1
#define GPIO1_OEN        0x103C02, BIT5
#define GPIO1_OUT        0x103C02, BIT4
#define GPIO1_IN         0x103C02, BIT0

#define GPIO2_PAD        PAD_GPIO2
#define GPIO2_OEN        0x103C04, BIT5
#define GPIO2_OUT        0x103C04, BIT4
#define GPIO2_IN         0x103C04, BIT0

#define GPIO3_PAD        PAD_GPIO3
#define GPIO3_OEN        0x103C06, BIT5
#define GPIO3_OUT        0x103C06, BIT4
#define GPIO3_IN         0x103C06, BIT0

#define GPIO4_PAD        PAD_GPIO4
#define GPIO4_OEN        0x103C08, BIT5
#define GPIO4_OUT        0x103C08, BIT4
#define GPIO4_IN         0x103C08, BIT0

#define GPIO5_PAD        PAD_GPIO5
#define GPIO5_OEN        0x103C0A, BIT5
#define GPIO5_OUT        0x103C0A, BIT4
#define GPIO5_IN         0x103C0A, BIT0

#define GPIO6_PAD        PAD_GPIO6
#define GPIO6_OEN        0x103C0C, BIT5
#define GPIO6_OUT        0x103C0C, BIT4
#define GPIO6_IN         0x103C0C, BIT0

#define GPIO7_PAD        PAD_GPIO7
#define GPIO7_OEN        0x103C0E, BIT5
#define GPIO7_OUT        0x103C0E, BIT4
#define GPIO7_IN         0x103C0E, BIT0

#define GPIO8_PAD        PAD_GPIO8
#define GPIO8_OEN        0x103C10, BIT5
#define GPIO8_OUT        0x103C10, BIT4
#define GPIO8_IN         0x103C10, BIT0

#define GPIO9_PAD        PAD_GPIO9
#define GPIO9_OEN        0x103C12, BIT5
#define GPIO9_OUT        0x103C12, BIT4
#define GPIO9_IN         0x103C12, BIT0

#define GPIO10_PAD        PAD_GPIO10
#define GPIO10_OEN        0x103C14, BIT5
#define GPIO10_OUT        0x103C14, BIT4
#define GPIO10_IN         0x103C14, BIT0

#define GPIO11_PAD        PAD_GPIO11
#define GPIO11_OEN        0x103C16, BIT5
#define GPIO11_OUT        0x103C16, BIT4
#define GPIO11_IN         0x103C16, BIT0

#define GPIO12_PAD        PAD_GPIO12
#define GPIO12_OEN        0x103C18, BIT5
#define GPIO12_OUT        0x103C18, BIT4
#define GPIO12_IN         0x103C18, BIT0

#define GPIO13_PAD        PAD_GPIO13
#define GPIO13_OEN        0x103C1A, BIT5
#define GPIO13_OUT        0x103C1A, BIT4
#define GPIO13_IN         0x103C1A, BIT0

#define GPIO14_PAD        PAD_GPIO14
#define GPIO14_OEN        0x103C1C, BIT5
#define GPIO14_OUT        0x103C1C, BIT4
#define GPIO14_IN         0x103C1C, BIT0

#define GPIO15_PAD        PAD_FUART_RX
#define GPIO15_OEN        0x103C28, BIT5
#define GPIO15_OUT        0x103C28, BIT4
#define GPIO15_IN         0x103C28, BIT0

#define GPIO16_PAD        PAD_FUART_TX
#define GPIO16_OEN        0x103C2A, BIT5
#define GPIO16_OUT        0x103C2A, BIT4
#define GPIO16_IN         0x103C2A, BIT0

#define GPIO17_PAD        PAD_FUART_CTS
#define GPIO17_OEN        0x103C2C, BIT5
#define GPIO17_OUT        0x103C2C, BIT4
#define GPIO17_IN         0x103C2C, BIT0

#define GPIO18_PAD        PAD_FUART_RTS
#define GPIO18_OEN        0x103C2E, BIT5
#define GPIO18_OUT        0x103C2E, BIT4
#define GPIO18_IN         0x103C2E, BIT0

#define GPIO19_PAD        PAD_TTL0
#define GPIO19_OEN        0x103C40, BIT5
#define GPIO19_OUT        0x103C40, BIT4
#define GPIO19_IN         0x103C40, BIT0

#define GPIO20_PAD        PAD_TTL1
#define GPIO20_OEN        0x103C42, BIT5
#define GPIO20_OUT        0x103C42, BIT4
#define GPIO20_IN         0x103C42, BIT0

#define GPIO21_PAD        PAD_TTL2
#define GPIO21_OEN        0x103C44, BIT5
#define GPIO21_OUT        0x103C44, BIT4
#define GPIO21_IN         0x103C44, BIT0

#define GPIO22_PAD        PAD_TTL3
#define GPIO22_OEN        0x103C46, BIT5
#define GPIO22_OUT        0x103C46, BIT4
#define GPIO22_IN         0x103C46, BIT0

#define GPIO23_PAD        PAD_TTL4
#define GPIO23_OEN        0x103C48, BIT5
#define GPIO23_OUT        0x103C48, BIT4
#define GPIO23_IN         0x103C48, BIT0

#define GPIO24_PAD        PAD_TTL5
#define GPIO24_OEN        0x103C4A, BIT5
#define GPIO24_OUT        0x103C4A, BIT4
#define GPIO24_IN         0x103C4A, BIT0

#define GPIO25_PAD        PAD_TTL6
#define GPIO25_OEN        0x103C4C, BIT5
#define GPIO25_OUT        0x103C4C, BIT4
#define GPIO25_IN         0x103C4C, BIT0

#define GPIO26_PAD        PAD_TTL7
#define GPIO26_OEN        0x103C4E, BIT5
#define GPIO26_OUT        0x103C4E, BIT4
#define GPIO26_IN         0x103C4E, BIT0

#define GPIO27_PAD        PAD_TTL8
#define GPIO27_OEN        0x103C50, BIT5
#define GPIO27_OUT        0x103C50, BIT4
#define GPIO27_IN         0x103C50, BIT0

#define GPIO28_PAD        PAD_TTL9
#define GPIO28_OEN        0x103C52, BIT5
#define GPIO28_OUT        0x103C52, BIT4
#define GPIO28_IN         0x103C52, BIT0

#define GPIO29_PAD        PAD_TTL10
#define GPIO29_OEN        0x103C54, BIT5
#define GPIO29_OUT        0x103C54, BIT4
#define GPIO29_IN         0x103C54, BIT0

#define GPIO30_PAD        PAD_TTL11
#define GPIO30_OEN        0x103C56, BIT5
#define GPIO30_OUT        0x103C56, BIT4
#define GPIO30_IN         0x103C56, BIT0

#define GPIO31_PAD        PAD_TTL12
#define GPIO31_OEN        0x103C58, BIT5
#define GPIO31_OUT        0x103C58, BIT4
#define GPIO31_IN         0x103C58, BIT0

#define GPIO32_PAD        PAD_TTL13
#define GPIO32_OEN        0x103C5A, BIT5
#define GPIO32_OUT        0x103C5A, BIT4
#define GPIO32_IN         0x103C5A, BIT0

#define GPIO33_PAD        PAD_TTL14
#define GPIO33_OEN        0x103C5C, BIT5
#define GPIO33_OUT        0x103C5C, BIT4
#define GPIO33_IN         0x103C5C, BIT0

#define GPIO34_PAD        PAD_TTL15
#define GPIO34_OEN        0x103C5E, BIT5
#define GPIO34_OUT        0x103C5E, BIT4
#define GPIO34_IN         0x103C5E, BIT0

#define GPIO35_PAD        PAD_TTL16
#define GPIO35_OEN        0x103C60, BIT5
#define GPIO35_OUT        0x103C60, BIT4
#define GPIO35_IN         0x103C60, BIT0

#define GPIO36_PAD        PAD_TTL17
#define GPIO36_OEN        0x103C62, BIT5
#define GPIO36_OUT        0x103C62, BIT4
#define GPIO36_IN         0x103C62, BIT0

#define GPIO37_PAD        PAD_TTL18
#define GPIO37_OEN        0x103C64, BIT5
#define GPIO37_OUT        0x103C64, BIT4
#define GPIO37_IN         0x103C64, BIT0

#define GPIO38_PAD        PAD_TTL19
#define GPIO38_OEN        0x103C66, BIT5
#define GPIO38_OUT        0x103C66, BIT4
#define GPIO38_IN         0x103C66, BIT0

#define GPIO39_PAD        PAD_TTL20
#define GPIO39_OEN        0x103C68, BIT5
#define GPIO39_OUT        0x103C68, BIT4
#define GPIO39_IN         0x103C68, BIT0

#define GPIO40_PAD        PAD_TTL21
#define GPIO40_OEN        0x103C6A, BIT5
#define GPIO40_OUT        0x103C6A, BIT4
#define GPIO40_IN         0x103C6A, BIT0

#define GPIO41_PAD        PAD_TTL22
#define GPIO41_OEN        0x103C6C, BIT5
#define GPIO41_OUT        0x103C6C, BIT4
#define GPIO41_IN         0x103C6C, BIT0

#define GPIO42_PAD        PAD_TTL23
#define GPIO42_OEN        0x103C6E, BIT5
#define GPIO42_OUT        0x103C6E, BIT4
#define GPIO42_IN         0x103C6E, BIT0

#define GPIO43_PAD        PAD_TTL24
#define GPIO43_OEN        0x103C70, BIT5
#define GPIO43_OUT        0x103C70, BIT4
#define GPIO43_IN         0x103C70, BIT0

#define GPIO44_PAD        PAD_TTL25
#define GPIO44_OEN        0x103C72, BIT5
#define GPIO44_OUT        0x103C72, BIT4
#define GPIO44_IN         0x103C72, BIT0

#define GPIO45_PAD        PAD_TTL26
#define GPIO45_OEN        0x103C74, BIT5
#define GPIO45_OUT        0x103C74, BIT4
#define GPIO45_IN         0x103C74, BIT0

#define GPIO46_PAD        PAD_TTL27
#define GPIO46_OEN        0x103C76, BIT5
#define GPIO46_OUT        0x103C76, BIT4
#define GPIO46_IN         0x103C76, BIT0

#define GPIO47_PAD        PAD_UART0_RX
#define GPIO47_OEN        0x103C30, BIT5
#define GPIO47_OUT        0x103C30, BIT4
#define GPIO47_IN         0x103C30, BIT0

#define GPIO48_PAD        PAD_UART0_TX
#define GPIO48_OEN        0x103C32, BIT5
#define GPIO48_OUT        0x103C32, BIT4
#define GPIO48_IN         0x103C32, BIT0

#define GPIO49_PAD        PAD_UART1_RX
#define GPIO49_OEN        0x103C34, BIT5
#define GPIO49_OUT        0x103C34, BIT4
#define GPIO49_IN         0x103C34, BIT0

#define GPIO50_PAD        PAD_UART1_TX
#define GPIO50_OEN        0x103C36, BIT5
#define GPIO50_OUT        0x103C36, BIT4
#define GPIO50_IN         0x103C36, BIT0

#define GPIO51_PAD        PAD_SD_CLK
#define GPIO51_OEN        0x103CAA, BIT5
#define GPIO51_OUT        0x103CAA, BIT4
#define GPIO51_IN         0x103CAA, BIT0

#define GPIO52_PAD        PAD_SD_CMD
#define GPIO52_OEN        0x103CA8, BIT5
#define GPIO52_OUT        0x103CA8, BIT4
#define GPIO52_IN         0x103CA8, BIT0

#define GPIO53_PAD        PAD_SD_D0
#define GPIO53_OEN        0x103CA0, BIT5
#define GPIO53_OUT        0x103CA0, BIT4
#define GPIO53_IN         0x103CA0, BIT0

#define GPIO54_PAD        PAD_SD_D1
#define GPIO54_OEN        0x103CA2, BIT5
#define GPIO54_OUT        0x103CA2, BIT4
#define GPIO54_IN         0x103CA2, BIT0

#define GPIO55_PAD        PAD_SD_D2
#define GPIO55_OEN        0x103CA4, BIT5
#define GPIO55_OUT        0x103CA4, BIT4
#define GPIO55_IN         0x103CA4, BIT0

#define GPIO56_PAD        PAD_SD_D3
#define GPIO56_OEN        0x103CA6, BIT5
#define GPIO56_OUT        0x103CA6, BIT4
#define GPIO56_IN         0x103CA6, BIT0

#define GPIO57_PAD        PAD_SD_GPIO
#define GPIO57_OEN        0x103CAC, BIT5
#define GPIO57_OUT        0x103CAC, BIT4
#define GPIO57_IN         0x103CAC, BIT0

#define GPIO58_PAD        PAD_PM_SD_CDZ
#define GPIO58_OEN        0xF8E, BIT0
#define GPIO58_OUT        0xF8E, BIT1
#define GPIO58_IN         0xF8E, BIT2

#define GPIO59_PAD        PAD_PM_IRIN
#define GPIO59_OEN        0xF28, BIT0
#define GPIO59_OUT        0xF28, BIT1
#define GPIO59_IN         0xF28, BIT2

#define GPIO60_PAD        PADA_IDAC_OUT_B
#define GPIO60_OEN        0x112724, BIT0
#define GPIO60_OUT        0x112750, BIT0
#define GPIO60_IN         0x112722, BIT0

#define GPIO61_PAD        PADA_IDAC_OUT_G
#define GPIO61_OEN        0x112724, BIT1
#define GPIO61_OUT        0x112750, BIT1
#define GPIO61_IN         0x112722, BIT1

#define GPIO62_PAD        PADA_IDAC_OUT_R
#define GPIO62_OEN        0x112724, BIT2
#define GPIO62_OUT        0x112750, BIT2
#define GPIO62_IN         0x112722, BIT2

#define GPIO63_PAD        PAD_PM_SPI_CZ
#define GPIO63_OEN        0xF30, BIT0
#define GPIO63_OUT        0xF30, BIT1
#define GPIO63_IN         0xF30, BIT2

#define GPIO64_PAD        PAD_PM_SPI_CK
#define GPIO64_OEN        0xF32, BIT0
#define GPIO64_OUT        0xF32, BIT1
#define GPIO64_IN         0xF32, BIT2

#define GPIO65_PAD        PAD_PM_SPI_DI
#define GPIO65_OEN        0xF34, BIT0
#define GPIO65_OUT        0xF34, BIT1
#define GPIO65_IN         0xF34, BIT2

#define GPIO66_PAD        PAD_PM_SPI_DO
#define GPIO66_OEN        0xF36, BIT0
#define GPIO66_OUT        0xF36, BIT1
#define GPIO66_IN         0xF36, BIT2

#define GPIO67_PAD        PAD_PM_SPI_WPZ
#define GPIO67_OEN        0xF88, BIT0
#define GPIO67_OUT        0xF88, BIT1
#define GPIO67_IN         0xF88, BIT2

#define GPIO68_PAD        PAD_PM_SPI_HLD
#define GPIO68_OEN        0xF8A, BIT0
#define GPIO68_OUT        0xF8A, BIT1
#define GPIO68_IN         0xF8A, BIT2

#define GPIO69_PAD        PAD_PM_LED0
#define GPIO69_OEN        0xF94, BIT0
#define GPIO69_OUT        0xF94, BIT1
#define GPIO69_IN         0xF94, BIT2

#define GPIO70_PAD        PAD_PM_LED1
#define GPIO70_OEN        0xF96, BIT0
#define GPIO70_OUT        0xF96, BIT1
#define GPIO70_IN         0xF96, BIT2

#define GPIO71_PAD        PAD_SAR_GPIO0
#define GPIO71_OEN        0x1423, BIT0
#define GPIO71_OUT        0x1424, BIT0
#define GPIO71_IN         0x1425, BIT0

#define GPIO72_PAD        PAD_SAR_GPIO1
#define GPIO72_OEN        0x1423, BIT1
#define GPIO72_OUT        0x1424, BIT1
#define GPIO72_IN         0x1425, BIT1

#define GPIO73_PAD        PAD_SAR_GPIO2
#define GPIO73_OEN        0x1423, BIT2
#define GPIO73_OUT        0x1424, BIT2
#define GPIO73_IN         0x1425, BIT2

#define GPIO74_PAD        PAD_SAR_GPIO3
#define GPIO74_OEN        0x1423, BIT3
#define GPIO74_OUT        0x1424, BIT3
#define GPIO74_IN         0x1425, BIT3

#define GPIO75_PAD        PAD_ETH_RN
#define GPIO75_OEN        0x33E2, BIT4
#define GPIO75_OUT        0x33E4, BIT0
#define GPIO75_IN         0x33E4, BIT4

#define GPIO76_PAD        PAD_ETH_RP
#define GPIO76_OEN        0x33E2, BIT5
#define GPIO76_OUT        0x33E4, BIT1
#define GPIO76_IN         0x33E4, BIT5

#define GPIO77_PAD        PAD_ETH_TN
#define GPIO77_OEN        0x33E2, BIT6
#define GPIO77_OUT        0x33E4, BIT2
#define GPIO77_IN         0x33E4, BIT6

#define GPIO78_PAD        PAD_ETH_TP
#define GPIO78_OEN        0x33E2, BIT7
#define GPIO78_OUT        0x33E4, BIT3
#define GPIO78_IN         0x33E4, BIT7

#define GPIO79_PAD        PAD_DM_P1
#define GPIO79_OEN        0x14210a, BIT4
#define GPIO79_OUT        0x14210a, BIT2
#define GPIO79_IN         0x142131, BIT5

#define GPIO80_PAD        PAD_DP_P1
#define GPIO80_OEN        0x14210a, BIT5
#define GPIO80_OUT        0x14210a, BIT3
#define GPIO80_IN         0x142131, BIT4

#define GPIO81_PAD        PAD_DM_P2
#define GPIO81_OEN        0x14250a, BIT4
#define GPIO81_OUT        0x14250a, BIT2
#define GPIO81_IN         0x142531, BIT5

#define GPIO82_PAD        PAD_DP_P2
#define GPIO82_OEN        0x14250a, BIT5
#define GPIO82_OUT        0x14250a, BIT3
#define GPIO82_IN         0x142531, BIT4

#define GPIO83_PAD        PAD_DM_P3
#define GPIO83_OEN        0x14290a, BIT4
#define GPIO83_OUT        0x14290a, BIT2
#define GPIO83_IN         0x142931, BIT5

#define GPIO84_PAD        PAD_DP_P3
#define GPIO84_OEN        0x14290a, BIT5
#define GPIO84_OUT        0x14290a, BIT3
#define GPIO84_IN         0x142931, BIT4

#define GPIO85_PAD        PAD_HSYNC_OUT
#define GPIO85_OEN        0x103C80, BIT5
#define GPIO85_OUT        0x103C80, BIT4
#define GPIO85_IN         0x103C80, BIT0

#define GPIO86_PAD        PAD_VSYNC_OUT
#define GPIO86_OEN        0x103C82, BIT5
#define GPIO86_OUT        0x103C82, BIT4
#define GPIO86_IN         0x103C82, BIT0

#define GPIO87_PAD        PAD_HDMITX_SCL
#define GPIO87_OEN        0x103C84, BIT5
#define GPIO87_OUT        0x103C84, BIT4
#define GPIO87_IN         0x103C84, BIT0

#define GPIO88_PAD        PAD_HDMITX_SDA
#define GPIO88_OEN        0x103C86, BIT5
#define GPIO88_OUT        0x103C86, BIT4
#define GPIO88_IN         0x103C86, BIT0

#define GPIO89_PAD        PAD_HDMITX_HPD
#define GPIO89_OEN        0x103C88, BIT5
#define GPIO89_OUT        0x103C88, BIT4
#define GPIO89_IN         0x103C88, BIT0

#define GPIO90_PAD        PAD_SATA_GPIO
#define GPIO90_OEN        0x103C8A, BIT5
#define GPIO90_OUT        0x103C8A, BIT4
#define GPIO90_IN         0x103C8A, BIT0


//-------------------------------------------------------------------------------------------------
//  Local Variables
//-------------------------------------------------------------------------------------------------
const struct gpio_setting gpio_table[] =
{
#define __GPIO__(_x_)   { CONCAT(CONCAT(GPIO, _x_), _OEN), CONCAT(CONCAT(GPIO, _x_), _OUT), CONCAT(CONCAT(GPIO, _x_), _IN) }
#define __GPIO(_x_)     __GPIO__(_x_)

//
// !! WARNING !! DO NOT MODIFIY !!!!
//
// These defines order must match following
// 1. the PAD name in GPIO excel
// 2. the perl script to generate the package header file
//
    //__GPIO(999), // 0 is not used
    __GPIO(0), __GPIO(1), __GPIO(2), __GPIO(3), __GPIO(4), __GPIO(5), __GPIO(6), __GPIO(7),
    __GPIO(8), __GPIO(9), __GPIO(10), __GPIO(11), __GPIO(12), __GPIO(13), __GPIO(14), __GPIO(15),
    __GPIO(16), __GPIO(17), __GPIO(18), __GPIO(19), __GPIO(20), __GPIO(21), __GPIO(22), __GPIO(23),
    __GPIO(24), __GPIO(25), __GPIO(26), __GPIO(27), __GPIO(28), __GPIO(29), __GPIO(30), __GPIO(31),
    __GPIO(32), __GPIO(33), __GPIO(34), __GPIO(35), __GPIO(36), __GPIO(37), __GPIO(38), __GPIO(39),
    __GPIO(40), __GPIO(41), __GPIO(42), __GPIO(43), __GPIO(44), __GPIO(45), __GPIO(46), __GPIO(47),
    __GPIO(48), __GPIO(49), __GPIO(50), __GPIO(51), __GPIO(52), __GPIO(53), __GPIO(54), __GPIO(55),
    __GPIO(56), __GPIO(57), __GPIO(58), __GPIO(59), __GPIO(60), __GPIO(61), __GPIO(62), __GPIO(63),
    __GPIO(64), __GPIO(65), __GPIO(66), __GPIO(67), __GPIO(68), __GPIO(69), __GPIO(70), __GPIO(71),
    __GPIO(72), __GPIO(73), __GPIO(74), __GPIO(75), __GPIO(76), __GPIO(77), __GPIO(78), __GPIO(79),
    __GPIO(80), __GPIO(81), __GPIO(82), __GPIO(83), __GPIO(84), __GPIO(85), __GPIO(86), __GPIO(87),
    __GPIO(88), __GPIO(89), __GPIO(90),
};