/*
 *
 * @file arch/arm/mach-drime4/include/mach/gpio_map_d4_galaxynx_simulator.h
 *
 * We define gpio for only common port for many different kinds of board of GALAXYNX SIMULATOR
 * 
 */
#ifndef __GPIO_MAP_D4_GALAXYNX_SIMULATOR_H__
#define __GPIO_MAP_D4_GALAXYNX_SIMULATOR_H__

#include <mach/version_information.h>

//                                                              													ES                    			 DV1					DV2

// GPIO Group 00 -------------------------------------------------------------------------------------------------------------------------
#define GPIO_HDMI_CEC			DRIME4_GPIO0(0)
#define GPIO_HDMI_DET			DRIME4_GPIO0(1)
#define GPIO_G00_P02_NC		DRIME4_GPIO0(2)
#define GPIO_LCD_SCK			DRIME4_GPIO0(3)
#define GPIO_G00_P04_TP36		DRIME4_GPIO0(4)
#define GPIO_LCD_SDI			DRIME4_GPIO0(5)							// GALAXYNX SIMULATOR
#define GPIO_LCD_CS				DRIME4_GPIO0(6)
#define GPIO_LCD_nRESET		DRIME4_GPIO0(7)


// GPIO Group 01 -------------------------------------------------------------------------------------------------------------------------
#define GPIO_POWER_ON			DRIME4_GPIO1(0)        
#define GPIO_DISP_3_3V_ON		DRIME4_GPIO1(1)							// DISP_3_3V_ON			DISP_3_3V_ON		NA
#define GPIO_LCD_VLCLK			DRIME4_GPIO1(2)
#define GPIO_LCD_DATA_EN		DRIME4_GPIO1(3)
#define GPIO_LCD_VSYNC			DRIME4_GPIO1(4)
#define GPIO_LCD_HSYNC			DRIME4_GPIO1(5)
#define GPIO_LCD_D_LCD_R0		DRIME4_GPIO1(6)
#define GPIO_LCD_D_LCD_R1		DRIME4_GPIO1(7)


// GPIO Group 02 -------------------------------------------------------------------------------------------------------------------------
#define GPIO_LCD_D_LCD_R2		DRIME4_GPIO2(0)
#define GPIO_LCD_D_LCD_R3		DRIME4_GPIO2(1)
#define GPIO_LCD_D_LCD_R4		DRIME4_GPIO2(2)
#define GPIO_LCD_D_LCD_R5		DRIME4_GPIO2(3)
#define GPIO_LCD_D_LCD_R6		DRIME4_GPIO2(4)
#define GPIO_LCD_D_LCD_R7		DRIME4_GPIO2(5)
#define GPIO_LCD_D_LCD_G0		DRIME4_GPIO2(6)
#define GPIO_LCD_D_LCD_G1		DRIME4_GPIO2(7)


// GPIO Group 03 -------------------------------------------------------------------------------------------------------------------------
#define GPIO_LCD_D_LCD_G2		DRIME4_GPIO3(0)
#define GPIO_LCD_D_LCD_G3		DRIME4_GPIO3(1)
#define GPIO_LCD_D_LCD_G4		DRIME4_GPIO3(2)
#define GPIO_LCD_D_LCD_G5		DRIME4_GPIO3(3)
#define GPIO_LCD_D_LCD_G6		DRIME4_GPIO3(4)
#define GPIO_LCD_D_LCD_G7		DRIME4_GPIO3(5)
#define GPIO_LCD_D_LCD_B0		DRIME4_GPIO3(6)
#define GPIO_LCD_D_LCD_B1		DRIME4_GPIO3(7)


// GPIO Group 04 -------------------------------------------------------------------------------------------------------------------------
#define GPIO_LCD_D_LCD_B2		DRIME4_GPIO4(0)
#define GPIO_LCD_D_LCD_B3		DRIME4_GPIO4(1)
#define GPIO_LCD_D_LCD_B4		DRIME4_GPIO4(2)
#define GPIO_LCD_D_LCD_B5		DRIME4_GPIO4(3)
#define GPIO_LCD_D_LCD_B6		DRIME4_GPIO4(4)
#define GPIO_LCD_D_LCD_B7		DRIME4_GPIO4(5)
#define GPIO_ISP_SPI_SCK		DRIME4_GPIO4(6)							// GALAXYNX SIMULATOR
#define GPIO_ISP_SPI_DOUT		DRIME4_GPIO4(7)							// GALAXYNX SIMULATOR


// GPIO Group 05 -------------------------------------------------------------------------------------------------------------------------
#define GPIO_ISP_SPI_DIN		DRIME4_GPIO5(0)							// GALAXYNX SIMULATOR
#define GPIO_ISP_SPI_CS			DRIME4_GPIO5(1)							// GALAXYNX SIMULATOR
#define GPIO_G05_P03_NC			DRIME4_GPIO5(2)
#define TSP_SCL					DRIME4_GPIO5(3)
#define TSP_SDA					DRIME4_GPIO5(4)
#define GPIO_STR_CTL			DRIME4_GPIO5(5)
#define GPIO_HPDET				DRIME4_GPIO5(6)
#define GPIO_SH_KEY1_MUIC		DRIME4_GPIO5(7)


// GPIO Group 06 -------------------------------------------------------------------------------------------------------------------------
#define GPIO_SHUTTER_KEY1_DSP	DRIME4_GPIO6(0)
#define GPIO_SHUTTER_KEY2		DRIME4_GPIO6(1)
#define SH_PI1_OUT				DRIME4_GPIO6(2)
#define SH_PI2_OUT				DRIME4_GPIO6(3)
#define GPIO_G06_P04_NC			DRIME4_GPIO6(4)
#define GPIO_TSP_INT			DRIME4_GPIO6(5)
#define GPIO_LENS_RB			DRIME4_GPIO6(6)
#define GPIO_JACK_INT			DRIME4_GPIO6(7)


// GPIO Group 07 -------------------------------------------------------------------------------------------------------------------------
#define GPIO_EXT_STR_DET			DRIME4_GPIO7(0)
#define GPIO_ROTATION_INT		DRIME4_GPIO7(1)
#define GPIO_JOG_L				DRIME4_GPIO7(2)
#define GPIO_JOG_R				DRIME4_GPIO7(3)
#define GPIO_REC_KEY			DRIME4_GPIO7(4)
#define GPIO_G07_P05_NC			DRIME4_GPIO7(5)
#define GPIO_G07_P06_NC			DRIME4_GPIO7(6)							// GALAXYNX SIMULATOR
#define GPIO_G07_P07_NC			DRIME4_GPIO7(7)


// GPIO Group 08 -------------------------------------------------------------------------------------------------------------------------
#define GPIO_G08_P00_NC			DRIME4_GPIO8(0)
#define GPIO_G08_P01_NC			DRIME4_GPIO8(1)
#define GPIO_G08_P02_NC			DRIME4_GPIO8(2)
#define GPIO_G08_P03_NC			DRIME4_GPIO8(3)
#define GPIO_SH_MOT_IN1			DRIME4_GPIO8(4)
#define GPIO_SH_MOT_IN2			DRIME4_GPIO8(5)
#define GPIO_G08_P06_NC			DRIME4_GPIO8(6)
#define LCD_BL_ON				DRIME4_GPIO8(7)							// GALAXYNX SIMULATOR


// GPIO Group 09 -------------------------------------------------------------------------------------------------------------------------
#define GPIO_SH_KEY2_MUIC		DRIME4_GPIO9(0)
#define WIFI_PWR_CNT			DRIME4_GPIO9(1)
//#define WIFI_PWR_CNT			DRIME4_GPIO9(2)
#define WIFI_WAKE_N				DRIME4_GPIO9(3)
#define GPIO_USB_SCL			DRIME4_GPIO9(4)
#define GPIO_USB_SDA			DRIME4_GPIO9(5)
#define GPIO_AUDIO_SCL			DRIME4_GPIO9(6)
#define GPIO_AUDIO_SDA			DRIME4_GPIO9(7)


// GPIO Group 10 -------------------------------------------------------------------------------------------------------------------------
#define GPIO_LENS_SCK			DRIME4_GPIO10(0)
#define GPIO_LENS_DIN			DRIME4_GPIO10(1)
#define GPIO_LENS_DOUT			DRIME4_GPIO10(2)
#define GPIO_G10_P03_NC			DRIME4_GPIO10(3)
#define GPIO_AUDIO_DO			DRIME4_GPIO10(4)
#define GPIO_DDR3_CKEIN			DRIME4_GPIO10(5)
#define GPIO_UART0_RX			DRIME4_GPIO10(6)
#define GPIO_UART0_TX			DRIME4_GPIO10(7)


// GPIO Group 11 -------------------------------------------------------------------------------------------------------------------------
#define GPIO_LENS_DET			DRIME4_GPIO11(0)
#define GPIO_PCU_GPIO			DRIME4_GPIO11(1)
#define GPIO_G11_P02_NC			DRIME4_GPIO11(2)
#define REQ_DPC					DRIME4_GPIO11(3)
#define GPIO_X_TRIG				DRIME4_GPIO11(4)
#define GPIO_G11_P05_NC			DRIME4_GPIO11(5)
#define GPIO_G11_P06_NC			DRIME4_GPIO11(6)
#define GPIO_G11_P07_NC			DRIME4_GPIO11(7)


// GPIO Group 12 -------------------------------------------------------------------------------------------------------------------------
#define GPIO_G12_P00_NC			DRIME4_GPIO12(0)
#define GPIO_ISP_POWER_INT		DRIME4_GPIO12(1)						// GALAXYNX SIMULATOR
#define GPIO_G12_P02_NC			DRIME4_GPIO12(2)
#define GPIO_AP_INT				DRIME4_GPIO12(3)						// GALAXYNX SIMULATOR
#define GPIO_DBG_TP0_TP15		DRIME4_GPIO12(4)
#define GPIO_G12_P05_NC			DRIME4_GPIO12(5)
#define GPIO_G12_P06_NC			DRIME4_GPIO12(6)
#define GPIO_SHUTTER_MG1_ON		DRIME4_GPIO12(7)


// GPIO Group 13 -------------------------------------------------------------------------------------------------------------------------
#define GPIO_SHUTTER_MG2_ON		DRIME4_GPIO13(0)
//#define GPIO_DRS_PWM_PLUS		DRIME4_GPIO13(1)
//#define GPIO_DRS_PWM_MINUS		DRIME4_GPIO13(2)
//#define GPIO_G13_P03_NC			DRIME4_GPIO13(3)
//#define GPIO_EXT_STR_TRG		DRIME4_GPIO13(4)
//#define GPIO_STR_TRG_DLY		DRIME4_GPIO13(5)
//#define GPIO_G13_P06_NC			DRIME4_GPIO13(6)
//#define GPIO_G13_P07_NC			DRIME4_GPIO13(7)


// GPIO Group 14 -------------------------------------------------------------------------------------------------------------------------
#define GPIO_G14_P00_NC			DRIME4_GPIO14(0)
#define GPIO_ROTATION_SCK		DRIME4_GPIO14(1)
#define GPIO_ROTATION_DIN		DRIME4_GPIO14(2)
#define GPIO_ROTATION_DOUT		DRIME4_GPIO14(3)
#define GPIO_ROTATION_CS		DRIME4_GPIO14(4)
#define GPIO_EXT_STR_SCK		DRIME4_GPIO14(5)
#define GPIO_EXT_STR_DIN		DRIME4_GPIO14(6)
#define GPIO_EXT_STR_DOUT		DRIME4_GPIO14(7)


// GPIO Group 15 -------------------------------------------------------------------------------------------------------------------------
#define GPIO_EXT_STR_EN			DRIME4_GPIO15(0)
//#define GPIO_G15_P01_NC			DRIME4_GPIO15(1)
//#define GPIO_G15_P02_NC			DRIME4_GPIO15(2)
//#define GPIO_G15_P03_NC			DRIME4_GPIO15(3)
//#define GPIO_G15_P04_NC			DRIME4_GPIO15(4)
#define GPIO_ISP_SCL			DRIME4_GPIO15(5)							// GALAXYNX SIMUALTOR
#define GPIO_ISP_SDA			DRIME4_GPIO15(6)							// GALAXYNX SIMUALTOR
#define GPIO_HDSCL				DRIME4_GPIO15(7)


// GPIO Group 16 -------------------------------------------------------------------------------------------------------------------------
#define GPIO_HDSDA				DRIME4_GPIO16(0)
#define GPIO_USB_DET			DRIME4_GPIO16(1)
#define GPIO_SL_TXD0_P			DRIME4_GPIO16(2)
#define GPIO_SL_TXD0_N			DRIME4_GPIO16(3)
#define GPIO_SL_TXCLK0_P		DRIME4_GPIO16(4)
#define GPIO_SL_TXCLK0_N		DRIME4_GPIO16(5)
//#define GPIO_G16_P06_NC			DRIME4_GPIO16(6)
//#define GPIO_G16_P07_NC			DRIME4_GPIO16(7)


// GPIO Group 17 -------------------------------------------------------------------------------------------------------------------------
//#define GPIO_G17_P00_NC			DRIME4_GPIO17(0)
//#define GPIO_G17_P01_NC			DRIME4_GPIO17(1)
//#define GPIO_G17_P02_NC			DRIME4_GPIO17(2)
//#define GPIO_G17_P03_NC			DRIME4_GPIO17(3)
//#define GPIO_G17_P04_NC			DRIME4_GPIO17(4)
//#define GPIO_G17_P05_NC			DRIME4_GPIO17(5)
//#define GPIO_G17_P06_NC			DRIME4_GPIO17(6)
//#define GPIO_G17_P07_NC			DRIME4_GPIO17(7)


// GPIO Group 18 -------------------------------------------------------------------------------------------------------------------------
//#define GPIO_G18_P00_NC			DRIME4_GPIO18(0)
//#define GPIO_G18_P01_NC			DRIME4_GPIO18(1)
//#define GPIO_G18_P02_NC			DRIME4_GPIO18(2)
//#define GPIO_G18_P03_NC			DRIME4_GPIO18(3)
//#define GPIO_G18_P04_NC			DRIME4_GPIO18(4)
//#define GPIO_G18_P05_NC			DRIME4_GPIO18(5)
//#define GPIO_G18_P06_NC			DRIME4_GPIO18(6)
//#define GPIO_G18_P07_NC			DRIME4_GPIO18(7)


// GPIO Group 19 -------------------------------------------------------------------------------------------------------------------------
//#define GPIO_G19_P00_NC			DRIME4_GPIO19(0)
//#define GPIO_G19_P01_NC			DRIME4_GPIO19(1)
//#define GPIO_G19_P02_NC			DRIME4_GPIO19(2)
//#define GPIO_G19_P03_NC			DRIME4_GPIO19(3)
//#define GPIO_G19_P04_NC			DRIME4_GPIO19(4)
//#define GPIO_G19_P05_NC			DRIME4_GPIO19(5)
//#define GPIO_G19_P06_NC			DRIME4_GPIO19(6)
//#define GPIO_G19_P07_NC			DRIME4_GPIO19(7)


// GPIO Group 20 -------------------------------------------------------------------------------------------------------------------------
//#define GPIO_G20_P00_NC			DRIME4_GPIO20(0)
//#define GPIO_G20_P01_NC			DRIME4_GPIO20(1)
//#define GPIO_G20_P02_NC			DRIME4_GPIO20(2)
//#define GPIO_G20_P03_NC			DRIME4_GPIO20(3)
//#define GPIO_G20_P04_NC			DRIME4_GPIO20(4)
//#define GPIO_G20_P05_NC			DRIME4_GPIO20(5)
//#define GPIO_G20_P06_NC			DRIME4_GPIO20(6)
//#define GPIO_G20_P07_NC			DRIME4_GPIO20(7)


// GPIO Group 21 -------------------------------------------------------------------------------------------------------------------------
//#define GPIO_G21_P00_NC			DRIME4_GPIO21(0)
//#define GPIO_G21_P01_NC			DRIME4_GPIO21(1)
//#define GPIO_G21_P02_NC			DRIME4_GPIO21(2)
//#define GPIO_G21_P03_NC			DRIME4_GPIO21(3)
//#define GPIO_G21_P04_NC			DRIME4_GPIO21(4)
//#define GPIO_G21_P05_NC			DRIME4_GPIO21(5)
//#define GPIO_G21_P06_NC			DRIME4_GPIO21(6)
//#define GPIO_G21_P07_NC			DRIME4_GPIO21(7)


// GPIO Group 22 -------------------------------------------------------------------------------------------------------------------------
//#define GPIO_G22_P00_NC			DRIME4_GPIO22(0)
//#define GPIO_G22_P01_NC			DRIME4_GPIO22(1)
//#define GPIO_G22_P02_NC			DRIME4_GPIO22(2)
//#define GPIO_G22_P03_NC			DRIME4_GPIO22(3)
//#define GPIO_G22_P04_NC			DRIME4_GPIO22(4)
//#define GPIO_G22_P05_NC			DRIME4_GPIO22(5)
//#define GPIO_G22_P06_NC			DRIME4_GPIO22(6)
//#define GPIO_G22_P07_NC			DRIME4_GPIO22(7)


// GPIO Group 23 -------------------------------------------------------------------------------------------------------------------------
#define GPIO_G23_P00_NC			DRIME4_GPIO23(0)
#define GPIO_SD1_CDX			DRIME4_GPIO23(1)
#define GPIO_G23_P02_NC			DRIME4_GPIO23(2)
#define GPIO_TSP_3_3V_ON		DRIME4_GPIO23(3)
#define GPIO_CIS_PW1_ON			DRIME4_GPIO23(4)
#define GPIO_CIS_PW2_ON			DRIME4_GPIO23(5)
#define GPIO_CIS_PW3_ON			DRIME4_GPIO23(6)
#define GPIO_LENS_SYNC_EN1		DRIME4_GPIO23(7)


// GPIO Group 24 -------------------------------------------------------------------------------------------------------------------------
#define GPIO_LENS_SYNC_EN2		DRIME4_GPIO24(0)
#define GPIO_AF_LED				DRIME4_GPIO24(1)
#define GPIO_CARD_LED			DRIME4_GPIO24(2)
//#define GPIO_G24_P03_NC			DRIME4_GPIO24(3)
#define GPIO_DBG_TP1_TP30		DRIME4_GPIO24(4)
//#define GPIO_G24_P05_NC			DRIME4_GPIO24(5)
#define GPIO_STR_VCC_ON			DRIME4_GPIO24(6)
//#define GPIO_G24_P07_NC			DRIME4_GPIO24(7)


// GPIO Group 25 -------------------------------------------------------------------------------------------------------------------------
#define GPIO_AUDIO_EN			DRIME4_GPIO25(0)
#define GPIO_LENS_3_3V_ON		DRIME4_GPIO25(1)
#define GPIO_LENS_5_0V_ON		DRIME4_GPIO25(2)
//#define GPIO_G25_P03_NC			DRIME4_GPIO25(3)
//#define GPIO_G25_P04_NC			DRIME4_GPIO25(4)
#define GPIO_G25_P05_NC			DRIME4_GPIO25(5)
#define GPIO_G25_P06_NC			DRIME4_GPIO25(6)
#define GPIO_1SHOT_HS			DRIME4_GPIO25(7)


// GPIO Group 26 -------------------------------------------------------------------------------------------------------------------------
#define GPIO_BVC0				DRIME4_GPIO26(0)
#define GPIO_BVC1				DRIME4_GPIO26(1)
#define GPIO_BVC2				DRIME4_GPIO26(2)
#define GPIO_G26_P03_NC			DRIME4_GPIO26(3)
#define GPIO_G26_P04_NC			DRIME4_GPIO26(4)
#define GPIO_WIFI_SDIO_D0		DRIME4_GPIO26(5)
#define GPIO_WIFI_SDIO_D1		DRIME4_GPIO26(6)
#define GPIO_WIFI_SDIO_D2		DRIME4_GPIO26(7)


// GPIO Group 27 -------------------------------------------------------------------------------------------------------------------------
#define GPIO_WIFI_SDIO_D3		DRIME4_GPIO27(0)
#define GPIO_G27_P01_NC			DRIME4_GPIO26(1)
#define GPIO_G27_P02_NC			DRIME4_GPIO26(2)
#define GPIO_G27_P03_NC			DRIME4_GPIO26(3)
#define GPIO_G27_P04_NC			DRIME4_GPIO26(4)
#define GPIO_WIFI_SDIO_CMD		DRIME4_GPIO27(5)
#define GPIO_WIFI_SDIO_CLK		DRIME4_GPIO27(6)
#define GPIO_WIFI_RESET_N		DRIME4_GPIO27(7)


// ETC  ---------------------------------------------------------------------------------------------------------------------------------

#define GPIO_WHEEL_R			DRIME4_GPIO24(4)
#define GPIO_WHEEL_L			DRIME4_GPIO7(5)

#define GPIO_CIS_SCL			DRIME4_GPIO15(5)
#define GPIO_CIS_SDA			DRIME4_GPIO15(6)

#define GPIO_SH_PI_OUT			DRIME4_GPIO12(0)
#define GPIO_SH_ACT_IN1			DRIME4_GPIO12(5)
#define GPIO_SH_ACT_IN2			DRIME4_GPIO12(6)

#define GPIO_HDR_RESET			DRIME4_GPIO25(6)

#define GPIO_EXT_MIC_I2S_LCK	DRIME4_GPIO7(6)
#define GPIO_EXT_MIC_I2S_BCK	DRIME4_GPIO7(7)
#define GPIO_EXT_MIC_I2S_DI		DRIME4_GPIO8(0)
#define GPIO_EXT_MIC_I2S_DO		DRIME4_GPIO8(1)
#define GPIO_DRS_INV_EN			DRIME4_GPIO6(7)//		DRIME4_GPIO24(3)

#define GPIO_EFS_RESET			DRIME4_GPIO9(2)

#endif //__GPIO_MAP_D4_NX300_H__

