/********************************************************************************/
/*!
	@file			hw_config.h
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2011.06.12
	@brief          Based on ST Microelectronics's Sample Thanks!

    @section HISTORY
		2011.06.12	V1.00	Start Here.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/
#ifndef __HW_CONFIG_H
#define __HW_CONFIG_H 0x0100

#ifdef __cplusplus
 extern "C" {
#endif

/* General Inclusion */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>
#ifndef TEST
#include "stm32f4xx.h"
#include "stm32f4xx_it.h"
#include "stm32f4xx_conf.h"
#include "platform_config.h"

/* Function Inclusion */
#include "systick.h"
#include "uart_support.h"
#include "rtc_support.h"

/* High Level Function */
#include "diskio.h"
#include "ff.h"
#endif

/* Macros */
#define countof(a)   (sizeof(a) / sizeof(*(a)))

/* Defines -------------------------------------------------------------------*/
#define KEY_NUM    0x01 //
#define KEY_DEL    0x07
#define KEY_BS     0x08
#define KEY_TAB    0x09
#define KEY_ESC    0x1B
#define KEY_SHIFT  0x03
#define KEY_CTRL   0x04
#define KEY_ALT    0x04
#define KEY_SPACE  0x20
//#define KEY_RETURN 0x0d
#define KEY_RETURN 0x0a
#define KEY_CAPS   0x02
#define KEY_CURSOR 0x05 //
#define KEY_UP     0x11
#define KEY_DOWN   0x12
#define KEY_LEFT   0x12
#define KEY_RIGHT  0x13

/* Externals */
extern __IO uint16_t CmdKey;
extern unsigned char kgetc (void);
extern void Set_System(void);
extern void NVIC_Configuration(void);
extern void KEY_Configuration(void);
extern void LED_Configuration(void);
extern void disk_timerproc(void);
extern void JoyInp_Chk(void);

#ifdef __cplusplus
}
#endif

#endif  /*__HW_CONFIG_H*/
