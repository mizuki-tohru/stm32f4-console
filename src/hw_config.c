/********************************************************************************/
/*!
	@file			hw_config.c
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

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "hw_config.h"

#define NEW_KEY_MODE 0
#define OLD_KEY_MODE 1
//#define KEYMODE NEW_KEY_MODE
#define KEYMODE 0

/* Constants -----------------------------------------------------------------*/
#if KEYMODE
int8_t keymap[4][5][9] = {	/*[3][5][9]*/
{{'@','0','/',KEY_NUM,0x5C,KEY_BS,KEY_CURSOR,KEY_ESC,0x00},
 {'Q','W','E','R','T','Y','U','I','O'},
 {'A','S','D','F','G','H','J','K','L'},
 {KEY_SHIFT,'Z','X','C','V','B','N','M','P'},
 {KEY_CTRL,KEY_SPACE,'<','>','?','+','*',KEY_RETURN,0x00}},
{{'[',']','/',KEY_NUM,0x5c,KEY_BS,KEY_CURSOR,KEY_ESC,0x00},
 {'q','w','e','r','t','y','u','i','o'},
 {'a','s','d','f','g','h','j','k','l'},
 {KEY_SHIFT,'z','x','c','v','b','n','m','p'},
 {KEY_CTRL,KEY_SPACE,'<','>','?','+','*',KEY_RETURN,0x00}},
{{'@','0','^',KEY_NUM,'_',KEY_BS,KEY_CURSOR,KEY_ESC,0x00},
 {'1','2','3','4','5','6','7','8','9'},
 {'!','"','#','$','%','&',0x27,'(',')'},
 {KEY_SHIFT,'~','=','-','|','`','{','}',KEY_TAB},
 {KEY_CTRL,KEY_SPACE,',','.','/',':',';',KEY_RETURN,0x00}},
{{0x00,0x00,0x00,0x00,0x00,KEY_BS,KEY_CURSOR,KEY_ESC,0x00},
 {0x00,0x00,0x00,0x00,0x00,0x00,0x00,KEY_UP,0x00},
 {0x00,0x00,0x00,0x00,0x00,0x00,KEY_LEFT,KEY_RIGHT,0x00},
 {0x00,'Z','X',0x00,0x00,0x00,0x00,KEY_DOWN,0x00},
 {0x00,KEY_SPACE,0x00,0x00,0x00,0x00,0x00,KEY_RETURN,0x00}}};
#else 
 int8_t keymap[3][5][9] = {
{{'p',KEY_RETURN,KEY_SPACE,'0',KEY_BS,KEY_CAPS,KEY_ALT,KEY_ESC,0x00},
 {'1','2','3','4','5','6','7','8','9'},
 {'q','w','e','r','t','y','u','i','o'},
 {'a','s','d','f','g','h','j','k','l'},
 {KEY_SHIFT,'z','x','c','v','b','n','m',0x00}},
{{'P',KEY_RETURN,KEY_SPACE,'0',KEY_BS,KEY_CAPS,KEY_ALT,KEY_ESC,0x00},
 {'1','2','3','4','5','6','7','8','9'},
 {'Q','W','E','R','T','Y','U','I','O'},
 {'A','S','D','F','G','H','J','K','L'},
 {KEY_SHIFT,'Z','X','C','V','B','N','M',0x00}},
{{'"',KEY_RETURN,KEY_SPACE,'_',KEY_DEL,KEY_CAPS,KEY_ALT,KEY_ESC,0x00},
 {'!','@','#','$','%','&',0x27,'(',')'},
 {'-','+','=','<','>','[',']',KEY_UP,';'},
 {'~','|',0x5C,'{','}','^','.',KEY_LEFT,KEY_RIGHT},
 {KEY_SHIFT,KEY_TAB,':','?','/','*',',',KEY_DOWN,0x00}}};
#endif

/* Variables -----------------------------------------------------------------*/;
__IO uint16_t CmdKey=0;
__IO uint16_t ModeKey=0;
__IO uint8_t  keyCount=0;
__IO uint8_t  ModeCursor=0;
__IO uint8_t  CAPSLock=0;
__IO uint8_t  ALTLock=0;
__IO uint8_t  ModeShift=0;

static uint32_t keynow=0;
static uint32_t keyold=0;
static uint8_t keypulse[9];

/* Prototypes ----------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
/**************************************************************************/
/*! 
    @brief	Return CommandData Wrapper.
	@param	None
    @retval	CommandData.
*/
/**************************************************************************/
unsigned char kgetc (void)
{
	unsigned char c;
	c = CmdKey;
	CmdKey = 0;
	return c;
}

/**************************************************************************/
/*! 
    @brief	Configures Main system clocks & power.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void Set_System(void)
{
	/* SystemInit was already executed in asm startup */

	/* NVIC configuration */
	NVIC_Configuration();
	KEY_Configuration();

}

/**************************************************************************/
/*! 
    @brief	Configures the LED on STM3220G-EVAL.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void LED_Configuration(void)
{
}


/**************************************************************************/
/*! 
    @brief	Configures the KEY-Input on STM3220G-EVAL.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void KEY_Configuration(void)
{
	int i;
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable CTRL Line GPIO Settings */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin 	= 0x40C0;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin 	= 0x0C33;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin 	= 0x0030;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin 	= 0x000F;
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin 	= 0x0008;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOA,GPIO_Pin_6);
	GPIO_ResetBits(GPIOA,GPIO_Pin_7);
	GPIO_ResetBits(GPIOC,GPIO_Pin_4);
	GPIO_ResetBits(GPIOC,GPIO_Pin_5);
	GPIO_ResetBits(GPIOB,GPIO_Pin_0);
	GPIO_ResetBits(GPIOB,GPIO_Pin_1);
	GPIO_SetBits(GPIOB,GPIO_Pin_4);	/*CAPS*/
	GPIO_SetBits(GPIOB,GPIO_Pin_5);	/*ALT*/
//	GPIO_ResetBits(GPIOB,GPIO_Pin_4);	/*CAPS*/
//	GPIO_ResetBits(GPIOB,GPIO_Pin_5);	/*ALT*/
	GPIO_ResetBits(GPIOB,GPIO_Pin_10);
	GPIO_ResetBits(GPIOB,GPIO_Pin_11);
	GPIO_ResetBits(GPIOA,GPIO_Pin_14);

	CmdKey = 0;
	ModeKey = 0;
	keyCount = 0;
	ModeCursor = 0;
	CAPSLock = 0;
	ALTLock = 0;
	ModeShift = 0;

	for(i=0;i<9;i++)keypulse[i] = 0;
}

/**************************************************************************/
/*! 
    @brief	Configures the KEY-Input on EVAL Board.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void JoyInp_Chk(void)
{
	static uint32_t prev;
	/* execute every 10mSec */
		
//	GPIO_Write(GPIOA, 0x0f);
//	GPIO_Write(GPIOC, 0x00);

	keynow =  (GPIO_ReadInputData(GPIOA) & 0x000F)<<1
	   		 |(GPIO_ReadInputData(GPIOC) & 0x0008)>>3;
	
	if(keynow){
		keypulse[keyCount]++;
		if(keypulse[keyCount] == 2)prev = keynow;
	}else{
		keypulse[keyCount] = 0;
		prev = 0;
	}
	
	switch(keyCount){
		case 0:
#if KEYMODE
			if(prev & 0x08){/*SHIFT*/
				prev -= 0x08;
			}
			if(prev & 0x10){/*CTRL*/
				prev -= 0x10;
			}
#else
			if(prev & 0x10){/*SHIFT*/
				prev -= 0x10;
				ModeShift = 1;
			}else{
				ModeShift = 0;
			}
#endif
			GPIO_ResetBits(GPIOA,GPIO_Pin_6|GPIO_Pin_14);
			GPIO_ResetBits(GPIOC,GPIO_Pin_4|GPIO_Pin_5);
			GPIO_ResetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_10|GPIO_Pin_11);
			GPIO_SetBits(GPIOA,GPIO_Pin_7);
			break;
		case 1:
			GPIO_ResetBits(GPIOA,GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_14);
			GPIO_ResetBits(GPIOC,GPIO_Pin_5);
			GPIO_ResetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_10|GPIO_Pin_11);
			GPIO_SetBits(GPIOC,GPIO_Pin_4);
			break;
		case 2:
			GPIO_ResetBits(GPIOA,GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_14);
			GPIO_ResetBits(GPIOC,GPIO_Pin_4);
			GPIO_ResetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_10|GPIO_Pin_11);
			GPIO_SetBits(GPIOC,GPIO_Pin_5);
			break;
		case 3:
#if KEYMODE
			if(prev & 0x01){/*NUM*/
				ModeKey++;
				if(ModeKey > 2)ModeKey = 0;
				prev -= 0x01;
			}
#else
#endif
			GPIO_ResetBits(GPIOA,GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_14);
			GPIO_ResetBits(GPIOC,GPIO_Pin_4|GPIO_Pin_5);
			GPIO_ResetBits(GPIOB,GPIO_Pin_1|GPIO_Pin_10|GPIO_Pin_11);
			GPIO_SetBits(GPIOB,GPIO_Pin_0);
			break;
		case 4:
			GPIO_ResetBits(GPIOA,GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_14);
			GPIO_ResetBits(GPIOC,GPIO_Pin_4|GPIO_Pin_5);
			GPIO_ResetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_10|GPIO_Pin_11);
			GPIO_SetBits(GPIOB,GPIO_Pin_1);
			break;
		case 5:
#if KEYMODE
#else
			if(prev & 0x01){/*CAPS*/
				CAPSLock = (CAPSLock+1) & 0x01;
				if(CAPSLock & 0x01){
					ModeKey = 1;
				}else{
					ModeKey = 0;
				}
				prev -= 0x01;
			}else{
				if(ModeKey > 2){
					ModeKey = 0;
					CAPSLock = 0;
					ALTLock = 0;
				}
			}
#endif
			GPIO_ResetBits(GPIOA,GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_14);
			GPIO_ResetBits(GPIOC,GPIO_Pin_4|GPIO_Pin_5);
			GPIO_ResetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_11);
			GPIO_SetBits(GPIOB,GPIO_Pin_10);
			break;
		case 6:
#if KEYMODE
			if(prev & 0x01){/*Cursor*/
				ModeCursor ^= 0x01;
				prev -= 0x01;
			}
#else
			if(prev & 0x01){/*ALT*/
				ALTLock = ((ALTLock+1) & 0x01);
				if(ALTLock & 0x01){
					if(CAPSLock & 0x01){
						ModeKey = 1;
					}else{
						ModeKey = 2;
					}
				}else{
					if(CAPSLock & 0x01){
						ModeKey = 1;
					}else{
						ModeKey = 0;
					}
				}
				prev -= 0x01;
			}
#endif
			GPIO_ResetBits(GPIOA,GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_14);
			GPIO_ResetBits(GPIOC,GPIO_Pin_4|GPIO_Pin_5);
			GPIO_ResetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_10);
			GPIO_SetBits(GPIOB,GPIO_Pin_11);
			break;
		case 7:
			GPIO_ResetBits(GPIOA,GPIO_Pin_6|GPIO_Pin_7);
			GPIO_ResetBits(GPIOC,GPIO_Pin_4|GPIO_Pin_5);
			GPIO_ResetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_10|GPIO_Pin_11);
			GPIO_SetBits(GPIOA,GPIO_Pin_14);
			break;
		case 8:
			GPIO_ResetBits(GPIOA,GPIO_Pin_7|GPIO_Pin_14);
			GPIO_ResetBits(GPIOC,GPIO_Pin_4|GPIO_Pin_5);
			GPIO_ResetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_10|GPIO_Pin_11);
			GPIO_SetBits(GPIOA,GPIO_Pin_6);
			break;
		default:
			break;
	}
#if KEYMODE
	if(prev & 0x01){
		if(ModeCursor & 0x01){
			CmdKey = keymap[3][0][keyCount];
		}else{
			CmdKey = keymap[ModeKey][0][keyCount];
		}
	}else if(prev &0x02){
		if(ModeCursor & 0x01){
			CmdKey = keymap[3][1][keyCount];
		}else{
			CmdKey = keymap[ModeKey][1][keyCount];
		}
	}else if(prev & 0x04){
		if(ModeCursor & 0x01){
			CmdKey = keymap[3][2][keyCount];
		}else{
			CmdKey = keymap[ModeKey][2][keyCount];
		}
	}else if(prev & 0x08){
		if(ModeCursor & 0x01){
			CmdKey = keymap[3][3][keyCount];
		}else{
			CmdKey = keymap[ModeKey][3][keyCount];
		}
	}else if(prev & 0x10){
		if(ModeCursor & 0x01){
			CmdKey = keymap[3][4][keyCount];
		}else{
			CmdKey = keymap[ModeKey][4][keyCount];
		}
	}else{
	//	CmdKey = 0x30 + prev;
	//	CmdKey = 0x40 + keyCount;
	}
	keyCount++;
	if(keyCount > 8)keyCount=0;
#else
	if(prev & 0x01){
		if(ModeShift){
			CmdKey = keymap[0][0][keyCount];
		}else{
			CmdKey = keymap[ModeKey][0][keyCount];
		}
	}else if(prev &0x02){
		if(ModeShift){
			CmdKey = keymap[0][1][keyCount];
		}else{
			CmdKey = keymap[ModeKey][1][keyCount];
		}
	}else if(prev & 0x04){
		if(ModeShift){
			CmdKey = keymap[0][2][keyCount];
		}else{
			CmdKey = keymap[ModeKey][2][keyCount];
		}
	}else if(prev & 0x08){
		if(ModeShift){
			CmdKey = keymap[0][3][keyCount];
		}else{
			CmdKey = keymap[ModeKey][3][keyCount];
		}
	}else if(prev & 0x10){
		if(ModeShift){
			CmdKey = keymap[0][4][keyCount];
		}else{
			CmdKey = keymap[ModeKey][4][keyCount];
		}
	}else{
	//	CmdKey = 0x30 + prev;
	//	CmdKey = 0x40 + keyCount;
	}
	keyCount++;
	if(keyCount > 8)keyCount=0;

	if(CAPSLock & 0x01){
		if(ModeShift){
			GPIO_SetBits(GPIOB,GPIO_Pin_4);
		}else{
			GPIO_ResetBits(GPIOB,GPIO_Pin_4);
		}
	}else{
		if(ModeShift){
			GPIO_ResetBits(GPIOB,GPIO_Pin_4);
		}else{
			GPIO_SetBits(GPIOB,GPIO_Pin_4);
		}
	}
	if(ALTLock & 0x01){
		GPIO_ResetBits(GPIOB,GPIO_Pin_5);
	}else{
		GPIO_SetBits(GPIOB,GPIO_Pin_5);
	}
#endif
}

/**************************************************************************/
/*! 
    @brief	Configures Vector Table base location.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void NVIC_Configuration(void)
{
	NVIC_SetPriority(SysTick_IRQn, 0x0);	
}

/* End Of File ---------------------------------------------------------------*/
