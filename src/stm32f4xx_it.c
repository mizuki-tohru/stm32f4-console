/********************************************************************************/
/*!
	@file			stm32f4xx_it.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2011.10.16
	@brief          Cortex-M4 Processor Exceptions Handlers.				@n
					And STM32F4xx Peripherals Interrupt Handlers.			@n
					Device Dependent Section.

    @section HISTORY
		2011.10.16	V1.00	Start Here.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "hw_config.h"
#include "systick.h"
#include "usb_core.h"
#include "usbd_core.h"
#include "usb_conf.h"
#include "usb_dcd_int.h"
#include "glcd.h"

#define USB_OTG_HS_DEDICATED_EP1_ENABLED 1

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern USB_OTG_CORE_HANDLE  USB_OTG_dev;

/* Private function prototypes -----------------------------------------------*/
extern uint32_t USBD_OTG_ISR_Handler (USB_OTG_CORE_HANDLE *pdev);

//#ifdef USB_OTG_HS_DEDICATED_EP1_ENABLED 
extern uint32_t USBD_OTG_EP1IN_ISR_Handler (USB_OTG_CORE_HANDLE *pdev);
extern uint32_t USBD_OTG_EP1OUT_ISR_Handler (USB_OTG_CORE_HANDLE *pdev);
//#endif

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**************************************************************************/
/*! 
    @brief	Handles NMI exception.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void NMI_Handler(void)
{
}


/**************************************************************************/
/*! 
    @brief	Handles Hard Fault exception.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}


/**************************************************************************/
/*! 
    @brief	Handles Memory Manage exception.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}


/**************************************************************************/
/*! 
    @brief	Handles Bus Fault exception.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}


/**************************************************************************/
/*! 
    @brief	Handles Usage Fault exception.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}


/**************************************************************************/
/*! 
    @brief	Handles SVCall exception.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void SVC_Handler(void)
{
}


/**************************************************************************/
/*! 
    @brief	Handles Debug Monitor exception.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void DebugMon_Handler(void)
{
}


/**************************************************************************/
/*! 
    @brief	Handles PendSVC exception.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void PendSV_Handler(void)
{
}


/**************************************************************************/
/*! 
    @brief	Handles SysTick Handler.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
//	2013.04.07 データーロガーのために追加
void getDataHandler(void);

void SysTick_Handler(void)
{
	static uint8_t  cntdiskio=0;
	static uint8_t  cntdiskioms=0;
	static uint16_t cnt=0;
	 /* every 100 mSec */
#if	defined(USE_ADS7843)
//	if( cnt++ >= 1000 ) {
//		scaned_tc =1;
#else
//	if( cnt++ >= 10000 ) {
#endif
//		cnt = 0;
//		glcd_TransFromBuf();
//	}

	 /* every 10mSec */
	if ((cntdiskio++) >= 100 ) {
		cntdiskio = 0;
//		disk_timerproc();
	}

	/* every 1mSec */
	if ((cntdiskioms++) >= 10 ) {
		cntdiskioms = 0;
		TimingDelay_Decrement();
	}
//	ts_timer();
//	ff_support_timerproc();
	/* key inputs */
	JoyInp_Chk();

//	2013.04.07 データーロガーのために追加
	getDataHandler();
}



/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32F4xx_xx.s).                                            */
/******************************************************************************/



/**************************************************************************/
/*! 
    @brief	Handles RealTimeClock interrupts requests.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void RTC_IRQHandler(void)
{
}
/**************************************************************************/
/*! 
    @brief	Handles Timer2 interrupts requests.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void TIM2_IRQHandler(void)
{
	
}
/**************************************************************************/
/*! 
    @brief	Handles Timer3 interrupts requests.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void TIM3_IRQHandler(void)
{
	
}
/**************************************************************************/
/*! 
    @brief	Handles Timer4 interrupts requests.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void TIM4_IRQHandler(void)
{
	
}
/**************************************************************************/
/*! 
    @brief	Handles USART2 global interrupt request.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void USART2_IRQHandler(void)
{

	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		/* Advance buffer head. */
		uint16_t tempRX_Head = ((&USART2_Buf)->RX_Head + 1) & (UART_BUFSIZE-1);

		/* Check for overflow. */
		uint16_t tempRX_Tail = (&USART2_Buf)->RX_Tail;
		uint8_t data =  USART_ReceiveData(USART2);

		if (tempRX_Head == tempRX_Tail) {
			/* Disable the USART2 Receive interrupt */
			USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
		}else{
			(&USART2_Buf)->RX[(&USART2_Buf)->RX_Head] = data;
			(&USART2_Buf)->RX_Head = tempRX_Head;
		}
	
	}

	if(USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
	{   
		/* Check if all data is transmitted. */
		uint16_t tempTX_Tail = (&USART2_Buf)->TX_Tail;
		if ((&USART2_Buf)->TX_Head == tempTX_Tail){
			/* Overflow MAX size Situation */
			/* Disable the USART2 Transmit interrupt */
			USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
		}else{
			/* Start transmitting. */
			uint8_t data = (&USART2_Buf)->TX[(&USART2_Buf)->TX_Tail];
			USART2->DR = data;

			/* Advance buffer tail. */
			(&USART2_Buf)->TX_Tail = ((&USART2_Buf)->TX_Tail + 1) & (UART_BUFSIZE-1);
		}

	}
}


/**************************************************************************/
/*! 
    @brief	Handles USART1 global interrupt request.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void USART1_IRQHandler(void)
{

	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		/* Advance buffer head. */
		uint16_t tempRX_Head = (USART1_Buf.RX_Head+1) & (UART_BUFSIZE-1);

		/* Check for overflow. */
		uint16_t tempRX_Tail = USART1_Buf.RX_Tail;
		uint8_t data =  USART_ReceiveData(USART1);

		if (tempRX_Head == tempRX_Tail) {
			/* Disable the USART1 Receive interrupt */
//			USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
		}else{
			USART1_Buf.RX[USART1_Buf.RX_Head] = data;
			USART1_Buf.RX_Head = tempRX_Head;
		}
	
	}

	if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
	{   
		/* Check if all data is transmitted. */
		uint16_t tempTX_Tail = USART1_Buf.TX_Tail;
		if (USART1_Buf.TX_Head == tempTX_Tail){
			/* Overflow MAX size Situation */
			/* Disable the USART1 Transmit interrupt */
			USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
		}else{
			/* Start transmitting. */
			uint8_t data = USART1_Buf.TX[USART1_Buf.TX_Tail];
			USART1->DR = data;

			/* Advance buffer tail. */
			USART1_Buf.TX_Tail = (USART1_Buf.TX_Tail+1) & (UART_BUFSIZE-1);
		}

	}
}
#if (IRQ_USART6_ENABLE==1)
void USART6_IRQHandler(void)
{

	if(USART_GetITStatus(USART6, USART_IT_RXNE) != RESET)
	{
		/* Advance buffer head. */
		uint16_t tempRX_Head = ((&USART6_Buf)->RX_Head + 1) & (UART_BUFSIZE-1);

		/* Check for overflow. */
		uint16_t tempRX_Tail = (&USART6_Buf)->RX_Tail;
		uint8_t data =  USART_ReceiveData(USART6);

		if (tempRX_Head == tempRX_Tail) {
			/* Disable the USART6 Receive interrupt */
			USART_ITConfig(USART6, USART_IT_RXNE, DISABLE);
		}else{
			(&USART6_Buf)->RX[(&USART6_Buf)->RX_Head] = data;
			(&USART6_Buf)->RX_Head = tempRX_Head;
		}
	
	}

	if(USART_GetITStatus(USART6, USART_IT_TXE) != RESET)
	{   
		/* Check if all data is transmitted. */
		uint16_t tempTX_Tail = (&USART6_Buf)->TX_Tail;
		if ((&USART6_Buf)->TX_Head == tempTX_Tail){
			/* Overflow MAX size Situation */
			/* Disable the USART6 Transmit interrupt */
			USART_ITConfig(USART6, USART_IT_TXE, DISABLE);
		}else{
			/* Start transmitting. */
			uint8_t data = (&USART6_Buf)->TX[(&USART6_Buf)->TX_Tail];
			USART6->DR = data;

			/* Advance buffer tail. */
			(&USART6_Buf)->TX_Tail = ((&USART6_Buf)->TX_Tail + 1) & (UART_BUFSIZE-1);
		}

	}
}
#endif
#ifdef USE_USB_OTG_HS  
void OTG_HS_IRQHandler(void)
{
  USBD_OTG_ISR_Handler (&USB_OTG_dev);
}
#endif

#ifdef USE_USB_OTG_FS  
void OTG_FS_IRQHandler(void)
{
  USBD_OTG_ISR_Handler (&USB_OTG_dev);
}
#endif

/**
  * @brief  This function handles EP1_IN Handler.
  * @param  None
  * @retval None
  */
void OTG_HS_EP1_IN_IRQHandler(void)
{
  USBD_OTG_EP1IN_ISR_Handler (&USB_OTG_dev);
}

/**
  * @brief  This function handles EP1_OUT Handler.
  * @param  None
  * @retval None
  */
void OTG_HS_EP1_OUT_IRQHandler(void)
{
  USBD_OTG_EP1OUT_ISR_Handler (&USB_OTG_dev);
}


/**************************************************************************/
/*! 
    @brief	Handles PPP interrupt request.								@n
			Function Templates
	@param	None.
    @retval	None.
*/
/**************************************************************************/
/*void PPP_IRQHandler(void)
{
}*/


/* End Of File ---------------------------------------------------------------*/
