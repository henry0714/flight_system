/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#define DATABUFFER_SIZE 32

/* Includes ------------------------------------------------------------------*/
#include "stm32f30x.h"
#include "stm32f3_discovery.h"
#include "stm32f30x_it.h"

/* FreeRTOS header files */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

struct DataBuffer
{
	char data[DATABUFFER_SIZE];
	int head;
	int tail;
	unsigned char empty;
	unsigned char full;
	unsigned char ready;
};
typedef struct DataBuffer data_buf;

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void hw_Init(void);
void hw_LED_Init(void);
void hw_UART_Init(void);
void hw_Timer_Init(void);
void hw_LCD_Init(void);
void usart_putc(char);
void usart_puts(char*);
void buffer_init(data_buf*);
int buffer_putc(data_buf*, char);
int buffer_getc(data_buf*, char*);
int strcmp(const char*, const char*);
int strlen(const char*);

#endif /* __MAIN_H */
