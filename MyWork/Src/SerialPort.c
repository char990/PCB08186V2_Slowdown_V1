#include "SerialPort.h"

// #include "stdio.h"
#include "string.h"

#include "Critical.h"

#define SERIAL_PORT_SEND_YIELD() \
	do                           \
	{                            \
	} while (0)

/* tx & rx dma buffer should be set as Non-cached in MPU*/
/* 'section' is set in STM32H723ZGTX_FLASH.ld*/
#define UART_TX_CACHE_BLOCKS 2
#define UART_TX_DMA_BUF_SIZE (32 * UART_TX_CACHE_BLOCKS)
static volatile uint8_t sp_tx_dma_buf[TOTAL_SERIALPORT][UART_TX_DMA_BUF_SIZE];

#define UART_RX_CACHE_BLOCKS 2
#define UART_RX_DMA_BUF_SIZE (32 * UART_RX_CACHE_BLOCKS)
#define UART_RX_DMA_BUF_SIZE_HALF (UART_RX_DMA_BUF_SIZE / 2)
// double buffer for rx
static volatile uint8_t sp_rx_dma_buf[TOTAL_SERIALPORT][UART_RX_DMA_BUF_SIZE];

#define UART_RX_BUF_SIZE (256)
static volatile uint8_t sp_rx_buf[TOTAL_SERIALPORT][UART_RX_BUF_SIZE];

#define TX_TIMEOUT_Factor 5
#define TX_TIMEOUT_BPS(v) (UART_TX_DMA_BUF_SIZE * TX_TIMEOUT_Factor * 10 * 1000 / v)

// for 9600, timeout is 666ms
#define TX_TIMEOUT_BPS9600 TX_TIMEOUT_BPS(9600)

// for 115200, timeout is 55ms
#define TX_TIMEOUT_BPS115200 TX_TIMEOUT_BPS(115200)

#define HAL_ERROR_FLAG (1 << 31)

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;
extern void Error_Handler();
extern void MX_USART1_UART_Init();
extern void MX_USART2_UART_Init();

SerialPort_t serialPort[TOTAL_SERIALPORT];

UART_HandleTypeDef *HUARTS[TOTAL_SERIALPORT] =
	{
		&huart1,
		&huart2,
};
DMA_HandleTypeDef *HDMA_UART_RXS[TOTAL_SERIALPORT] =
	{
		&hdma_usart1_rx,
		&hdma_usart2_rx,
};

void SerialPortInit()
{
	for (int i = 0; i < TOTAL_SERIALPORT; i++)
	{
		serialPort[i].huart = HUARTS[i];
		serialPort[i].hdma_rx = HDMA_UART_RXS[i];
		serialPort[i].tx_dma_buf = &sp_tx_dma_buf[i][0];
		serialPort[i].rx_dma_buf = &sp_rx_dma_buf[i][0];
		RB_Init(&serialPort[i].rx_ringbuf, &sp_rx_buf[i][0], UART_RX_BUF_SIZE);
		serialPort[i].error_code = HAL_UART_ERROR_NONE;
		serialPort[i].tx_timeout = TX_TIMEOUT_BPS(serialPort[i].huart->Init.BaudRate);
	}
}

void SerialPortReInit(SerialPort_t *sp)
{
	HAL_UART_Abort(sp->huart);
	RB_Clear(&sp->rx_ringbuf);
	HAL_UART_DeInit(sp->huart);
	if(sp->huart == &huart1)
	{
		MX_USART1_UART_Init();
	}
	else if(sp->huart == &huart2)
	{
		MX_USART2_UART_Init();
	}
	else
	{
		while (1)
		{
			/* code */
		}
	}
	sp->error_code = HAL_UART_ERROR_NONE;
}

void SerialPortStartRx(SerialPort_t *sp)
{
	HAL_StatusTypeDef st = HAL_UARTEx_ReceiveToIdle_DMA(sp->huart, sp->rx_dma_buf, UART_RX_DMA_BUF_SIZE);
	sp->error_code = (st == HAL_OK) ? 0 : (HAL_ERROR_FLAG | sp->huart->ErrorCode);
}

int IsSerialPortRx(SerialPort_t *sp)
{
	if (sp->huart == NULL)
	{
		return 0;
	}
	return (sp->huart->ReceptionType == HAL_UART_RECEPTION_TOIDLE);
}

SerialPort_t *GetSerialPort(UART_HandleTypeDef *huart)
{
	for (int i = 0; i < TOTAL_SERIALPORT; i++)
	{
		if (huart == serialPort[i].huart)
		{
			return &serialPort[i];
		}
	}
	return NULL;
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	SerialPort_t *sp = GetSerialPort(huart);
	sp->error_code = huart->ErrorCode;
}

void SpErrorCheck(SerialPort_t *sp)
{
	if (sp->error_code != HAL_UART_ERROR_NONE)
	{
		if ((sp->error_code & HAL_ERROR_FLAG) ||
			((sp->error_code & HAL_ERROR_FLAG) == 0 &&
			 (sp->error_code & (HAL_UART_ERROR_DMA | HAL_UART_ERROR_RTO))))
		{
			SerialPortReInit(sp);
		}
		SerialPortStartRx(sp);
	}
}

void SpWrite(SerialPort_t *sp, const uint8_t *buf, int len)
{
	uint32_t time_start;
	HAL_StatusTypeDef st;
	SpErrorCheck(sp);
	while (len > 0)
	{
		time_start = HAL_GetTick();
		while (sp->huart->gState != HAL_UART_STATE_READY)
		{
			SERIAL_PORT_SEND_YIELD();
			if ((HAL_GetTick() - time_start) > sp->tx_timeout)
			{
				sp->error_code = HAL_ERROR_FLAG;
				return;
			}
		}
		int xlen = (len < UART_TX_DMA_BUF_SIZE) ? len : UART_TX_DMA_BUF_SIZE;
		memcpy(sp->tx_dma_buf, buf, xlen);
		ENTER_CRITICAL();
		st = HAL_UART_Transmit_DMA(sp->huart, sp->tx_dma_buf, xlen);
		EXIT_CRITICAL();
		if (st != HAL_OK)
		{
			sp->error_code = HAL_ERROR_FLAG | sp->huart->ErrorCode;
			return;
		}
		len -= xlen;
		buf += xlen;
	};
	return;
}

int SpRead(SerialPort_t *sp, uint8_t *buf, int len)
{
	// SpErrorCheck(sp);
	// Do NOT run ErrorCheck
	// SpRead should be called after AnyChars
	return RB_Pop(&sp->rx_ringbuf, buf, len);
}

int SpAnyChars(SerialPort_t *sp)
{
	SpErrorCheck(sp);
	return RB_Space_Used(&sp->rx_ringbuf);
}

char SpGetchar(SerialPort_t *sp)
{
	while (RB_Space_Used(&sp->rx_ringbuf) == 0)
	{
		SERIAL_PORT_SEND_YIELD();
		SpErrorCheck(sp);
	}
	return RB_Pop_c(&sp->rx_ringbuf);
}

int SpPutchar(SerialPort_t *sp, const char c)
{
	SpWrite(sp, (uint8_t *)&c, 1);
	return c;
}

int SpPuts(SerialPort_t *sp, const char *s)
{
	int len = strlen(s);
	if (len <= 0)
	{
		return -1;
	}
	SpWrite(sp, (uint8_t *)s, len);
	return len;
}

int SpFlush(SerialPort_t *sp)
{
	while (sp->huart->gState != HAL_UART_STATE_READY)
	{
		SERIAL_PORT_SEND_YIELD();
		SpErrorCheck(sp);
	}
	return 0;
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	SerialPort_t *sp = GetSerialPort(huart);
	if (huart->RxEventType == HAL_UART_RXEVENT_HT) // Half full
	{
		RB_Push(&sp->rx_ringbuf, sp->rx_dma_buf, UART_RX_DMA_BUF_SIZE_HALF);
	}
	else if (huart->RxEventType == HAL_UART_RXEVENT_TC) // Complete
	{
		RB_Push(&sp->rx_ringbuf, sp->rx_dma_buf + UART_RX_DMA_BUF_SIZE_HALF, UART_RX_DMA_BUF_SIZE_HALF);
		SerialPortStartRx(sp);
	}
	else if (huart->RxEventType == HAL_UART_RXEVENT_IDLE) // Idle
	{
		if (Size > 0 && Size < UART_RX_DMA_BUF_SIZE_HALF)
		{
			RB_Push(&sp->rx_ringbuf, sp->rx_dma_buf, Size);
		}
		else if (Size > UART_RX_DMA_BUF_SIZE_HALF && Size < UART_RX_DMA_BUF_SIZE)
		{
			RB_Push(&sp->rx_ringbuf, sp->rx_dma_buf + UART_RX_DMA_BUF_SIZE_HALF, Size - UART_RX_DMA_BUF_SIZE_HALF);
		}
		SerialPortStartRx(sp);
	}
	return;
}
