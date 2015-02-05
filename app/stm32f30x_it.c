/**
  ******************************************************************************
  * @file    stm32f30x_it.c 
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    20-September-2012
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f30x_it.h"

/** @addtogroup STM32F3-Discovery_Demo
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t i = 0;
extern data_buf USART_TxBuffer;
extern data_buf USART_RxBuffer;
extern TaskHandle_t xHandle;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/******************************************************************************/
/*                 STM32F30x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f30x.s).                                            */
/******************************************************************************/
/**
  * @brief  This function handles EXTI0_IRQ Handler.
  * @param  None
  * @retval None
  */
void EXTI0_IRQHandler(void)
{ 
	if((EXTI_GetITStatus(USER_BUTTON_EXTI_LINE)==SET) && (STM_EVAL_PBGetState(BUTTON_USER)!=RESET))
	{
		/* Wait for SEL button to be pressed  */
		while(STM_EVAL_PBGetState(BUTTON_USER) != RESET);
		STM_EVAL_LEDToggle(LED3);

		/* Clear the EXTI line pending bit */
		EXTI_ClearITPendingBit(USER_BUTTON_EXTI_LINE);
	}
}

/**
  * @brief  This function handles USART3_IRQ Handler.
  * @param  None
  * @retval None
  */
void USART3_IRQHandler(void)
{
	char c;
	BaseType_t xYieldRequired;

	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		c = USART_ReceiveData(USART3);
		if(c == '\r')
		{
			USART_SendData(USART3, '\r');
			USART_SendData(USART3, '\n');
			buffer_init(&USART_RxBuffer);

			// Resume the suspended task.
			xYieldRequired = xTaskResumeFromISR(xHandle);

			if(xYieldRequired == pdTRUE)
			{
				// We should switch context so the ISR returns to a different task.
				// NOTE:  How this is done depends on the port you are using.  Check
				// the documentation and examples for your port.
				portYIELD_FROM_ISR(pdTRUE);
			}
		}
		else if(c == '\b' || c == 127)
		{
			if(!USART_RxBuffer.empty)
			{
				USART_SendData(USART3, c);
				buffer_getc(&USART_RxBuffer, &c);
			}
		}
		else
		{
			if(!buffer_putc(&USART_RxBuffer, c))
			{
				USART_SendData(USART3, c);
			}
		}
	}

	if(USART_GetITStatus(USART3, USART_IT_TXE) != RESET)
	{
		if(!buffer_getc(&USART_TxBuffer, &c))
			USART_SendData(USART3, c);
		else
		{
			USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
			USART_TxBuffer.ready = 1;
		}
	}

	if(USART_GetFlagStatus(USART3, USART_FLAG_ORE) != RESET)
	{
		c = USART_ReceiveData(USART3);
		while(USART_GetFlagStatus(USART3, USART_IT_TXE) != RESET);
		USART_SendData(USART3, c);
		USART_ClearFlag(USART3, USART_FLAG_ORE);
	}

	if(USART_GetFlagStatus(USART3, USART_FLAG_NE) != RESET)
	{
		USART_ClearFlag(USART3, USART_FLAG_NE);
	}

	if(USART_GetFlagStatus(USART3, USART_FLAG_FE) != RESET)
	{
		USART_ClearFlag(USART3, USART_FLAG_FE);
	}

	if(USART_GetFlagStatus(USART3, USART_FLAG_PE) != RESET)
	{
		USART_ClearFlag(USART3, USART_FLAG_PE);
	}
}

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

