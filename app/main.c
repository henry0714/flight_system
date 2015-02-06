#include "main.h"

/* Private variables ---------------------------------------------------------*/
RCC_ClocksTypeDef RCC_Clocks;
data_buf USART_TxBuffer, USART_RxBuffer;
TaskHandle_t xHandle_shell = NULL;

/* Private function prototypes -----------------------------------------------*/
static void prvSetupHardware(void);
void vLEDTask1(void*);
void vLEDTask2(void*);
void vTask_shell(void*);
void command_parse(char*);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None 
  * @retval None
  */
int main(void)
{
    prvSetupHardware();
	buffer_init(&USART_RxBuffer);
	buffer_init(&USART_TxBuffer);
    
    xTaskCreate(vLEDTask1, "LED_TOP", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
    xTaskCreate(vLEDTask2, "LED_BOTTOM", configMINIMAL_STACK_SIZE, NULL, 3, NULL);
    xTaskCreate(vTask_shell, "SHELL", configMINIMAL_STACK_SIZE, NULL, 4, &xHandle_shell);

    vTaskStartScheduler();
    
    while(1);
}

void vLEDTask1(void* pvParameters)
{
    while(1)
    {
        STM_EVAL_LEDOn(LED3);
        vTaskDelay(100/portTICK_RATE_MS);
        STM_EVAL_LEDOff(LED3);
        vTaskDelay(100/portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

void vLEDTask2(void* pvParameters)
{
    while(1)
    {
        STM_EVAL_LEDOn(LED10);
        vTaskDelay(100/portTICK_RATE_MS);
        STM_EVAL_LEDOff(LED10);
        vTaskDelay(100/portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

void vTask_shell(void* pvParameters)
{
	while(1)
	{
		usart_puts("DrunkardSystem> ");
		vTaskSuspend(NULL);
		command_parse(&USART_RxBuffer.data[0] + USART_RxBuffer.tail);

		// Clean Rx Buffer.
		buffer_init(&USART_RxBuffer);
	}
    vTaskDelete(NULL);
}

void command_parse(char* command)
{
	if(strlen(command) == 0)
		return;

	if(strcmp(command, "start") == 0)
		usart_puts("OK!\r\n");
	else
	{
		usart_puts(command);
		usart_puts(": command not found!\r\n");
	}
}

void hw_LED_Init(void)
{
    /* Initialize LEDs on STM32F3-Discovery board */
    STM_EVAL_LEDInit(LED3);
    STM_EVAL_LEDInit(LED4);
    STM_EVAL_LEDInit(LED5);
    STM_EVAL_LEDInit(LED6);
    STM_EVAL_LEDInit(LED7);
    STM_EVAL_LEDInit(LED8);
    STM_EVAL_LEDInit(LED9);
    STM_EVAL_LEDInit(LED10);
}

void hw_UART_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

    /* Initialize GPIO pins */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;            // Use the alternative pin functions
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_7);   // PC10 -> USART3_TX
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_7);   // PC11 -> USART3_RX

	/* Initialize NVIC */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x08;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Initialize UART */
	USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

	USART_Cmd(USART3, ENABLE);
}

/**
  * @brief  Configure the processor for use with the STM32F3-Discovery board.
  *         This includes setup for the I/O, system clock, and access timings.
  * @param  None
  * @retval None
  */
static void prvSetupHardware(void)
{
    RCC_GetClocksFreq(&RCC_Clocks);
	/* The divisor should match configTICK_RATE_HZ in FreeRTOSConfig.h */
    SysTick_Config(RCC_Clocks.HCLK_Frequency / 100);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);

    hw_LED_Init();
    hw_UART_Init();
}

void usart_putc(char c)
{
	while(buffer_putc(&USART_TxBuffer, c));
	if(USART_TxBuffer.ready)
	{
		USART_TxBuffer.ready = 0;
		USART_ITConfig(USART3, USART_IT_TXE, ENABLE);	// Raise TXE interrupt
	}
}

void usart_puts(char* str)
{
	int i;
	for(i=0; str[i]!='\0'; i++)
		usart_putc(str[i]);
}

void buffer_init(data_buf* buf)
{
	int i;
	for(i = 0; i < DATABUFFER_SIZE; i++)
		buf->data[i] = '\0';

	buf->head = 0;
	buf->tail = 0;
	buf->empty = 1;
	buf->full = 0;
	buf->ready = 1;
}

int buffer_putc(data_buf* buf, char c)
{
	if(!buf->full)
	{
		buf->data[buf->head] = c;
		buf->head = (buf->head < DATABUFFER_SIZE - 1) ? (buf->head + 1) : 0;
		buf->empty = 0;
		buf->full = ((buf->head+1) % DATABUFFER_SIZE == buf->tail) ? 1 : 0;
		return 0;
	}
	else
		return 1;
}

int buffer_getc(data_buf* buf, char* c)
{
	if(!buf->empty)
	{
		*c = buf->data[buf->tail];
		buf->tail = (buf->tail < DATABUFFER_SIZE - 1) ? (buf->tail + 1) : 0;
		buf->empty = (buf->head == buf->tail) ? 1 : 0;
		buf->full = 0;
		return 0;
	}
	else
		return 1;
}

int strcmp(const char* s1, const char* s2)
{
	while(*s1 && (*s1 == *s2))
	{
		s1++;
		s2++;
	}
	return (*s1 - *s2);
}

int strlen(const char* s)
{
	int i = 0;
	while(*s)
	{
		s++;
		i++;
	}
	return i;
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
    /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    
    /* Infinite loop */
    while(1)
    {
    }
}
#endif
