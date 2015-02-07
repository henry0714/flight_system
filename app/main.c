#include "main.h"

/* Private variables ---------------------------------------------------------*/
RCC_ClocksTypeDef RCC_Clocks;
data_buf USART_TxBuffer, USART_RxBuffer;
data_buf Bluetooth_TxBuffer, Bluetooth_RxBuffer;
TaskHandle_t xHandle_shell = NULL;
TaskHandle_t xHandle_bluetooth = NULL;

/* Private function prototypes -----------------------------------------------*/
static void prvSetupHardware(void);
void vLEDTask1(void*);
void vLEDTask2(void*);
void vTask_shell(void*);
void vTask_bluetooth(void*);
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
	buffer_init(&Bluetooth_RxBuffer);
	buffer_init(&Bluetooth_TxBuffer);

	USART1_puts("System boot...\r\n");
    
    xTaskCreate(vLEDTask1, "LED_TOP", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vLEDTask2, "LED_BOTTOM", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(vTask_shell, "SHELL", configMINIMAL_STACK_SIZE, NULL, 4, &xHandle_shell);
    xTaskCreate(vTask_bluetooth, "BLUETOOTH", configMINIMAL_STACK_SIZE, NULL, 3, &xHandle_bluetooth);

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
		USART3_puts("DrunkardSystem> ");
		//USART1_puts("DrunkardSystem> ");
		vTaskSuspend(NULL);
		command_parse(&USART_RxBuffer.data[0] + USART_RxBuffer.tail);

		// Clean Rx Buffer.
		buffer_init(&USART_RxBuffer);
	}
    vTaskDelete(NULL);
}

void vTask_bluetooth(void* pvParameters)
{
	while(1)
	{
		vTaskSuspend(NULL);
		USART3_puts("\r\nBluetooth MSG: ");
		USART3_puts(&Bluetooth_RxBuffer.data[0] + Bluetooth_RxBuffer.tail);
		USART3_puts("\r\n");

		// Clean Rx Buffer.
		buffer_init(&Bluetooth_RxBuffer);
	}
    vTaskDelete(NULL);
}

void command_parse(char* command)
{
	if(strlen(command) == 0)
		return;

	if(strcmp(command, "start") == 0)
		USART3_puts("OK!\r\n");
	else
	{
		USART3_puts(command);
		USART3_puts(": command not found!\r\n");
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
  * @brief  Configure the UART interface connecting to the BC04 module.
  * @param  None
  * @retval None
  */
void hw_Bluetooth_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

    /* Initialize GPIO pins */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;            // Use the alternative pin functions
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_7);    // PB6 -> USART1_TX
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_7);    // PB7 -> USART1_RX

	/* Initialize NVIC */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x09;
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
    USART_Init(USART1, &USART_InitStructure);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	USART_Cmd(USART1, ENABLE);
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
    hw_Bluetooth_Init();
}

void usart_putc(char c, data_buf* buf, USART_TypeDef* usart)
{
	while(buffer_putc(buf, c));
	if(buf->ready)
	{
		buf->ready = 0;
		USART_ITConfig(usart, USART_IT_TXE, ENABLE);	// Raise TXE interrupt
	}
}

void usart_puts(char* str, data_buf* buf, USART_TypeDef* usart)
{
	int i;
	for(i=0; str[i]!='\0'; i++)
		usart_putc(str[i], buf, usart);
}

void USART1_puts(char* str)
{
	usart_puts(str, &Bluetooth_TxBuffer, USART1);
}

void USART3_puts(char* str)
{
	usart_puts(str, &USART_TxBuffer, USART3);
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
