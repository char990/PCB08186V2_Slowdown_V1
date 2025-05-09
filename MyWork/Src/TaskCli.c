/*
 * TaskCli.c
 *
 *  Created on: Feb 18, 2025
 *      Author: lq
 */
#include <TaskCli.h>
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

#include "main.h"

#include <FW_Version.h>
#include "SerialPort.h"
#include "Pwm.h"
#include "MyTmr.h"

#include "Consts.h"
#include "MyPrintf.h"
#include "Tasks.h"
#include "OPT4001.h"
#include "AscHex.h"
#include "MyCrc.h"
#include "MyI2C.h"
#include "ScanAdc.h"
#include "Config.h"
#include "Critical.h"
#include "DS3231.h"
#include "SignStatus.h"

// #define ENABLE_MCP9808 1
// #define ENABLE_CRC32 1

myPt_t ptCli;
#define this_pt (&ptCli)

msTmr_t tmrCli;
#define this_tmr (&tmrCli)

SerialPort_t *spCli = &serialPort[1];
#define this_sp spCli

#define CLI_RX_BUF_SIZE 32
static char cliRxBuf[CLI_RX_BUF_SIZE]; /* The command string.*/
static uint8_t cliRxLen;

void PrintVersion()
{
#ifdef DEBUG
	MyPrintf("\nSign[%u]: DEBUG: Version:%04x Build:%lu\n", signID, FW_VERSION, FW_BUILD);
#else
	MyPrintf("\nSign[%u]: RELEASE: Version:%04x Build:%lu\n", signID, FW_VERSION, FW_BUILD);
#endif
}

typedef enum CMD_RET
{
	SUCCESS_WITHOUT_MSG = 0,
	UNKNOWN_CMD = 1,
	INVALID_PARAM = 2,
	CMD_RET_SIZE
} CMD_RET_t;

const char *CMD_RET_STR[CMD_RET_SIZE] =
	{
		"Success",
		"Unknown command",
		"Invalid parameters",
};

typedef struct command_t
{
	const char *cmd_str;
	CMD_RET_t (*cmd_func)(int argc, char **argv);
	const char *cmd_usage;
} command_t;

#define ARGUMENTS_MAX 16 // including command
char *argv[ARGUMENTS_MAX];
int MakeArg(char *cmdline)
{
	char *token;
	int argc = 0;
	for (int i = 0; i < ARGUMENTS_MAX; i++)
	{
		argv[i] = NULL;
	}
	token = strtok(cmdline, " ");
	while (token != NULL)
	{
		argv[argc++] = token;
		token = strtok(NULL, " ");
		if (argc == ARGUMENTS_MAX)
		{
			break;
		}
	}
	return argc;
}

CMD_RET_t TestPwm(int argc, char **argv)
{
	if (argc != 3)
	{
		return INVALID_PARAM;
	}
	else
	{
		int s = strtol(argv[1], NULL, 0);
		int duty = strtol(argv[2], NULL, 0);
		if (s < 1 || s > 2 || duty < 0 || duty > 255)
		{
			return INVALID_PARAM;
		}
		SetDuty(s - 1, duty);
		return SUCCESS_WITHOUT_MSG;
	}
}

CMD_RET_t TestAd(int argc, char **argv)
{
	HAL_StatusTypeDef st;
	st = HAL_ADCEx_Calibration_Start(scanAdc.hadc);
	if (st != HAL_OK)
	{
		MyPrintf("\nADCEx_Calibration Failed=%u\n", st);
		return SUCCESS_WITHOUT_MSG;
	}
	if (RunAdc() != 0)
	{
		MyPrintf("ADC error: ST = 0x%08X\n", scanAdc.hadc->State);
	}
	else
	{
		MyPuts("\nADC:");
		for (int i = 0; i < ADC_SIZE; i++)
		{
			MyPrintf(" [%u] = %u%c", i, scanAdc.dma_buf[i], (i == ADC_SIZE - 1) ? '\n' : ',');
		}
		MyPuts("HC:");
		for (int i = 0; i < ADC_HC_SIZE; i++)
		{
			MyPrintf(" [%d] = %u mv = %d ma%c",
					 i + 1,
					 Get_mv(scanAdc.dma_buf[i], scanAdc.dma_buf[ADC_VREF]),
					 GetHCmA(i),
					 (i == ADC_HC_SIZE - 1) ? '\n' : ',');
		}
		MyPrintf("T = %d('C)\n", GetTcpu());
	}
	return SUCCESS_WITHOUT_MSG;
}

CMD_RET_t TestI2C(int argc, char **argv)
{
	I2C_HandleTypeDef *hi2c;
	const char *si2c;
	if (argc != 2 && argc != 4)
	{
		return INVALID_PARAM;
	}
	long i2c = strtol(argv[1], NULL, 0);
	if (i2c == 1)
	{
		hi2c = &hi2c1;
		si2c = "I2C1";
	}
	else if (i2c == 2)
	{
		hi2c = &hi2c2;
		si2c = "I2C2";
	}
	else
	{
		return INVALID_PARAM;
	}
	uint8_t x[256];
	x[0] = 0;
	if (argc == 2)
	{
		for (int i = 1; i < 128; i++)
		{
			x[i] = (HAL_I2C_Master_Receive(hi2c, i << 1, &x[i], 1, 10) == HAL_OK) ? i : 0;
		}
		PrintUint8(x, 128, si2c);
	}
	else // (argc == 3)
	{
		long addr = strtol(argv[2], NULL, 0);
		long len = strtol(argv[3], NULL, 0);
		if (addr <= 0 || addr > 127 || len <= 0 || len > 128)
		{
			return INVALID_PARAM;
		}
		if (HAL_I2C_Master_Transmit(hi2c, addr << 1, x, 1, 1000) == HAL_OK &&
			HAL_I2C_Master_Receive(hi2c, addr << 1, x, len, 1000) == HAL_OK)
		{
			PrintUint8(x, len, si2c);
		}
		else
		{
			MyPrintf("\n%s Failed\n", si2c);
		}
	}
	return SUCCESS_WITHOUT_MSG;
}

#if ENABLE_MCP9808 == 1
CMD_RET_t MCP9808(int argc, char **argv)
{
	I2C_HandleTypeDef *hi2c = &hi2c1;
	if (argc != 2)
	{
		return INVALID_PARAM;
	}
	long addr = strtol(argv[1], NULL, 0);
	if (addr < 0x18 || addr > 0x1F)
	{
		return INVALID_PARAM;
	}
	uint16_t x[9];
	for (uint8_t reg = 1; reg < 9; reg++)
	{
		if (HAL_I2C_Master_Transmit(hi2c, addr << 1, &reg, 1, 1000) == HAL_OK &&
			HAL_I2C_Master_Receive(hi2c, addr << 1, (uint8_t *)&x[reg], (reg == 8) ? 1 : 2, 1000) == HAL_OK)
		{
			if (reg == 8)
			{
				x[reg] >>= 8;
				MyPrintf("\n[%u]=0x%02X\n", reg, x[reg]);
			}
			else
			{
				x[reg] = (x[reg] >> 8) | (x[reg] << 8);
				MyPrintf("\n[%u]=0x%04X\n", reg, x[reg]);
			}
		}
		else
		{
			MyPrintf("\n%s Failed\n", "I2C");
			break;
		}
	}
	const char *dot_t[16] = {
		"0",
		"0625",
		"125",
		"1875",
		"25",
		"3125",
		"375",
		"4375",
		"5",
		"5625",
		"625",
		"6875",
		"75",
		"5125",
		"875",
		"9375",
	};
	x[5] &= 0x1FFF;
	if (x[5] & 0x1000)
	{ // T < 0'C
	}
	else
	{
		MyPrintf("\nT=%d.%s'C\n", x[5] / 16, dot_t[x[5] & 0x000F]);
	}
	return SUCCESS_WITHOUT_MSG;
}
#endif

CMD_RET_t OPT4001(int argc, char **argv)
{
	MyPrintf("\nlux[2]: [0] = %u, [1] = %u\n", signExtStatus.lux[0], signExtStatus.lux[1]);
	return SUCCESS_WITHOUT_MSG;
}

#if ENABLE_CRC32 == 1
CMD_RET_t CRC32(int argc, char **argv)
{
	if (argc != 2)
	{
		return INVALID_PARAM;
	}
	uint32_t len = strlen(argv[1]);
	uint32_t crc = CRC_Calculate((uint8_t *)argv[1], len);
	MyPrintf("\ncrc=0x%08X\n", crc);
	return SUCCESS_WITHOUT_MSG;
}
#endif

void PrintTime(time_t t)
{
	struct tm localtm;
	localtime_r(&t, &localtm);
	MyPrintf("%02d/%02d/%04d %02d:%02d:%02d\n",
			 localtm.tm_mday, localtm.tm_mon + 1, localtm.tm_year + 1900,
			 localtm.tm_hour, localtm.tm_min, localtm.tm_sec);
}

CMD_RET_t Date(int argc, char **argv)
{
	time_t t;
	if (argc == 1)
	{
		MyPrintf("\nSystem time: ");
		t = GetTimestamp();
		PrintTime(t);
		MyPrintf("\nDS3231 time: ");
		t = DS3231GetTime(&hi2c2);
		if (t > 0)
		{
			PrintTime(t);
		}
		else
		{
			MyPrintf("NG\n");
		}
		MyPrintf("DS3231-T: ");
		int temp;
		if (DS3231GetTemp(&hi2c2, &temp) == DS3231_OK)
		{
			MyPrintf("%d('C)\n", temp);
		}
		else
		{
			MyPrintf("NG\n");
		}
		return SUCCESS_WITHOUT_MSG;
	}
	else if (argc == 3)
	{
		struct tm stm;
		int d, mon, y, h, min, s;
		if (sscanf(argv[1], "%d/%d/%d", &d, &mon, &y) == 3 &&
			sscanf(argv[2], "%d:%d:%d", &h, &min, &s) == 3)
		{
			if (y >= 25 && y <= 99)
			{
				stm.tm_mday = d;
				stm.tm_mon = mon - 1;
				stm.tm_year = y + 2000 - 1900;
				stm.tm_hour = h;
				stm.tm_min = min;
				stm.tm_sec = s;
				t = mktime(&stm);
				SetTimestamp(t);
				if (DS3231SetTime(&hi2c2, t) == DS3231_NG)
				{
					MyPrintf("DS3231SetTime failed\n");
				}
				return SUCCESS_WITHOUT_MSG;
			}
		}
	}
	return INVALID_PARAM;
}

void PrintSignStatus()
{
	MyPrintf("\nSign Status: "
			 "single_led(FSt)=0x%02X, litsnsr_fault=0x%02X, current_id=%u, current_crc=0x%04X, next_id=%u, next_crc=0x%04X\n",
			 signStatus.single_led, signStatus.litsnsr_fault,
			 signStatus.current_id, signStatus.current_crc, signStatus.next_id, signStatus.next_crc);
}

void PrintSignExtStatus()
{
	MyPrintf("\nSign Ext Status: "
			 "hours=%u, t_board=%d, lux[0]=%u, lux[1]=%u, vlux=%u, bright[0]=%u, fault_led=%u\n",
			 signExtStatus.hours, signExtStatus.t_board, signExtStatus.lux[0], signExtStatus.lux[1],
			 signExtStatus.vlux, signExtStatus.bright[0], signExtStatus.fault_led);
}

CMD_RET_t PrintStatus(int argc, char **argv)
{
	PrintVersion();
	PrintSignStatus();
	PrintSignExtStatus();
	Date(1, NULL);
	return SUCCESS_WITHOUT_MSG;
}

// declare here
CMD_RET_t Help(int argc, char **argv);

command_t CLI_CMD[] =
	{
		/*********************** info ***********************/
		{"help", Help,
		 "\r\nhelp"
		 "\r\n  This help"},
		{"ps", PrintStatus,
		 "\r\nps"
		 "\r\n  Print status"},
		{"date", Date,
		 "\r\nDate [dd/MM/yy hh:mm:ss]"
		 "\r\n  Print RTC+temprature or Set RTC"},

		/*********************** test ***********************/
		{"testpwm", TestPwm,
		 "\r\ntestpwm HC duty"
		 "\r\n  HC=1|2, duty=0-255"},
		{"testad", TestAd,
		 "\r\ntestad"
		 "\r\n  test all ad channels"},
		{"testi2c", TestI2C,
		 "\r\ntesti2c 1|2 [addr len]"
		 "\r\n  testi2c 1|2: List all slaves on I2C1|2"
		 "\r\n  testi2c 1 0x45 32: Print regs[0-31](max=256) of slave[0x45] on I2C1"},
#if ENABLE_MCP9808 == 1
		{"MCP9808", MCP9808,
		 "\r\nMCP9808 address"
		 "\r\n  List all register in MCP9808 and print temperature"},
#endif
		{"OPT4001", OPT4001,
		 "\r\nOPT4001"
		 "\r\n  Print lux[2]"},
#if ENABLE_CRC32 == 1
		{"CRC32", CRC32,
		 "\r\nCRC32 12sdlfkjsdf"
		 "\r\n  Calculate crc32"},
#endif
};

CMD_RET_t Help(int argc, char **argv)
{
	CMD_RET_t ret = INVALID_PARAM;
	for (int i = 1; i < sizeof(CLI_CMD) / sizeof(CLI_CMD[0]); i++)
	{
		if (argc == 1 || strcmp(CLI_CMD[i].cmd_str, argv[1]) == 0)
		{
			ret = SUCCESS_WITHOUT_MSG;
			MyPuts(CLI_CMD[i].cmd_usage);
			MyPuts("\r\n");
		}
	}
	return ret;
}

uint8_t TaskCli()
{
	PT_BEGIN(this_pt);
	PrintVersion();
	for (;;)
	{
		wdt |= WDT_TASK_CLI;
		if (SpAnyChars(this_sp))
		{
			char rxc = SpGetchar(this_sp);
			if (rxc == KEY_LF || rxc == KEY_CR)
			{
				cliRxBuf[cliRxLen] = '\0';
				MyPuts("\r\n");
				if (cliRxLen > 0)
				{
					CMD_RET_t cmd_ret = UNKNOWN_CMD;
					int argc = MakeArg(cliRxBuf);
					for (int i = 0; i < sizeof(CLI_CMD) / sizeof(CLI_CMD[0]); i++)
					{
						if (strcasecmp(CLI_CMD[i].cmd_str, argv[0]) == 0)
						{
							cmd_ret = (*CLI_CMD[i].cmd_func)(argc, argv);
							break;
						}
					}
					if (cmd_ret > 0 && cmd_ret < CMD_RET_SIZE)
					{
						MyPuts(CMD_RET_STR[cmd_ret]);
					}
					cliRxLen = 0;
					cliRxBuf[cliRxLen] = '\0';
				}
				MyPuts("\r\n=>");
			}
			else
			{
				/* The if() clause performs the processing after a newline character
				 is received.  This else clause performs the processing if any other
				 character is received. */

				if (rxc == '\b')
				{
					/* Backspace was pressed.  Erase the last character in the input
					 buffer - if there are any. */
					if (cliRxLen > 0)
					{
						cliRxLen--;
						cliRxBuf[cliRxLen] = '\0';
						MyPuts("\b \b");
					}
				}
				else
				{
					/* A character was entered.  It was not a new line, backspace
					 or carriage return, so it is accepted as part of the input and
					 placed into the input buffer.  When a n is entered the complete
					 string will be passed to the command interpreter. */
					if (cliRxLen < CLI_RX_BUF_SIZE - 1)
					{
						cliRxBuf[cliRxLen++] = rxc;
						cliRxBuf[cliRxLen] = '\0';
						SpPutchar(this_sp, rxc);
					}
					else
					{
						SpPutchar(this_sp, 0x07); // beep
					}
				}
			}
		}
		PT_YIELD(this_pt);
	}
	PT_END(this_pt);
}

static void this_Init()
{
	cliRxLen = 0;
	cliRxBuf[0] = '\0';
}

void TaskCliInit()
{
	this_Init();
	PT_Reset(this_pt);
	SerialPortStartRx(this_sp);
}
