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
#ifndef __STM32F4xx_IT_H
#define __STM32F4xx_IT_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#ifndef TEST
#include "stm32f4xx.h"
#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#define IRQ_TIM2_ENABLE 0
#define IRQ_TIM3_ENABLE 0
#define IRQ_TIM4_ENABLE 0
#define IRQ_USART1_ENABLE 1
#define IRQ_USART6_ENABLE 1

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

#if (IRQ_TIM2_ENABLE==1)
void TIM2_IRQHandler(void);
#endif
#if (IRQ_TIM3_ENABLE==1)
void TIM3_IRQHandler(void);
#endif
#if (IRQ_TIM4_ENABLE==1)
void TIM4_IRQHandler(void);
#endif

#if (IRQ_USART1_ENABLE==1)
void USART1_IRQHandler(void);
#endif
#if (IRQ_USART6_ENABLE==1)
void USART6_IRQHandler(void);
#endif
#ifdef __cplusplus
}
#endif

#endif /* __STM32F4xx_IT_H */
