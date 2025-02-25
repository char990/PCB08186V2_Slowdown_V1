/*
 * TaskProtc.c
 *
 *  Created on: Sep 12, 2023
 *      Author: lq
 */
#include <TaskProtc.h>
#include "stdlib.h"
#include "string.h"

#include "main.h"

#include "SerialPort.h"
#include "Pwm.h"
#include "MyTmr.h"
#include "CmdData.h"

#include "Consts.h"
#include "MyPrintf.h"
#include "Tasks.h"
#include "OPT4001.h"
#include "AscHex.h"
#include "MyCrc.h"
#include "MyI2C.h"
#include "ScanAdc.h"
#include "Config.h"

myPt_t ptPtc;
#define this_pt (&ptPtc)

msTmr_t tmrPtc;
#define this_tmr (&tmrPtc)

SerialPort_t *spPtc = &serialPort[0];
#define this_sp spPtc

// All commands
#define CMD_REQ_ST 0x35
#define CMD_RPL_ST 0x36
#define CMD_SET_CONSPICUITY 0x37
#define CMD_RPL_CONSPICUITY 0x38

int CMD_ReqSt(int len);
int CMD_SetConspicuity(int len);

const cmd_t SLV_CMD[] = {
	{CMD_ReqSt, CMD_REQ_ST, 2},
	{CMD_SetConspicuity, CMD_SET_CONSPICUITY, 4},
};

#define PTC_TX_BUF_SIZE 64
static uint8_t ptcTxBuf[PTC_TX_BUF_SIZE];
static uint8_t *ptcTxBufHalf = &ptcTxBuf[PTC_TX_BUF_SIZE / 2];

#define PTC_RX_BUF_SIZE 32
static uint8_t ptcRxBuf[PTC_RX_BUF_SIZE];

rxcmd_t rxCmd;

static int this_AnyChars()
{
	return SpAnyChars(this_sp);
}

static char this_GetChar()
{
	return SpGetchar(this_sp);
}

#define SLV_TIMEOUT 10000
static void this_Init()
{
	rxCmd.rxLen = 0;
	rxCmd.size = PTC_RX_BUF_SIZE;
	rxCmd.buffer = ptcRxBuf;
	rxCmd.AnyChars = this_AnyChars;
	rxCmd.Getchar = this_GetChar;
	SetMsTmr(this_tmr, SLV_TIMEOUT);
}

uint8_t TaskProtc()
{
	PT_BEGIN(this_pt);
	for (;;)
	{
		wdt |= WDT_TASK_PROTC;
		if (RxCmd(&rxCmd) > 0)
		{
			if (rxCmd.buffer[CMD_INDEX_SLVID] == SLV_ID || rxCmd.buffer[CMD_INDEX_SLVID] == 0xFF)
			{
				SetMsTmr(this_tmr, SLV_TIMEOUT);
				for (int i = 0; i < sizeof(SLV_CMD) / sizeof(SLV_CMD[0]); i++)
				{
					if (rxCmd.buffer[CMD_INDEX_CODE] == SLV_CMD[i].cmd_id &&
						(SLV_CMD[i].cmd_len == 0 || SLV_CMD[i].cmd_len == rxCmd.rxLen))
					{
						int len = (*SLV_CMD[i].func)(rxCmd.rxLen);
						if (rxCmd.buffer[CMD_INDEX_SLVID] == SLV_ID && len > 0)
						{
							len = Cmd_Encode(ptcTxBufHalf, ptcTxBuf, len);
							if (len > 0)
							{
								SpWrite(this_sp, ptcTxBuf, len);
							}
						}
						break;
					}
				}
			}
			rxCmd.rxLen = 0;
		}
		if (IsMsTmrExpired(this_tmr))
		{ // timeout, turn off conspicuity
			ClrMsTmr(this_tmr);
			if (st_conspicuity)
			{
				st_conspicuity = 0;
				conspicuity_changed = 1;
			}
		}
		PT_YIELD(this_pt);
	}
	PT_END(this_pt);
}

int CMD_ReqSt(int len)
{
	uint8_t *p = ptcTxBufHalf;
	*p++ = SLV_ID;
	*p++ = CMD_RPL_ST;
	*p++ = st_conspicuity;
	*p++ = st_pwm;
	p = Cnvt_PutU16(st_lux[LUX_FRONT], p); // LUX_FRONT
	p = Cnvt_PutU16(st_lux[LUX_BACK], p);  // LUX_BACK
	// increase by 1 to avoid to report 0
	p = Cnvt_PutU16(st_flasherCurrent[PORT_HC1] + 1, p); // upper flasher current
	p = Cnvt_PutU16(st_flasherCurrent[PORT_HC2] + 1, p); // lower flasher current
	*p++ = GetTcpu() | st_bootup;
	st_bootup = 0;
	return p - ptcTxBufHalf;
}

int CMD_SetConspicuity(int len)
{
	conspicuity_changed = 1;
	st_conspicuity = rxCmd.buffer[2] & 0x03;
	st_pwm = rxCmd.buffer[3];
	uint8_t *p = ptcTxBufHalf;
	*p++ = SLV_ID;
	*p++ = CMD_RPL_CONSPICUITY;
	*p++ = st_conspicuity;
	*p++ = st_pwm;
	return p - ptcTxBufHalf;
}

void TaskProtcInit()
{
	this_Init();
	PT_Reset(this_pt);
	SerialPortStartRx(this_sp);
}
