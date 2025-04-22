/*
 * TaskProtc.c
 *
 *  Created on: Sep 12, 2023
 *      Author: lq
 */
#include <TaskProtc.h>

#if GC_MODE == GC_MODE_RS485

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
#include "SignStatus.h"
#include "FW_Version.h"
#include "DS3231.h"

myPt_t ptPtc;
#define this_pt (&ptPtc)

msTmr_t tmrPtc;
#define this_tmr (&tmrPtc)

SerialPort_t *spPtc = &serialPort[0];
#define this_sp spPtc

// All commands
#define PROTOCOL_VER_MAJOR 0x03
#define PROTOCOL_VER_MINOR 0x02

#define CMD_REQ_ST 0x05
#define CMD_RPL_ST 0x06
#define CMD_REQ_EXT_ST 0x07
#define CMD_RPL_EXT_ST 0x08
// #define CMD_SET_TXT_FRM 0x0A
// #define CMD_SET_GFX_FRM 0x0B
#define CMD_SET_SPC_FRM 0xFC
#define CMD_DISP_FRM 0x0E
#define CMD_STR_FRM 0x0F
// #define CMD_SYNC 0x09
#define CMD_SET_RTC 0x51
#define CMD_REQ_RTC 0x52
#define CMD_RPL_RTC 0x53
#define CMD_REQ_VER 0x5E
#define CMD_RPL_VER 0x5F
// #define CMD_START 0xF0

int CMD_ReqSt(int len);
int CMD_ReqExtSt(int len);
// int CMD_SetTxtFrm(int len);
// int CMD_SetGfxFrm(int len);
int CMD_SetSpecailFrm(int len);
// int CMD_DisplayFrm(int len);
// int CMD_StoredFrm(int len);
int CMD_SD_Frm(int len); // CMD_DisplayFrm + CMD_StoredFrm
int CMD_ReqVer(int len);
// int CMD_Sync(int len);
int CMD_SetRtc(int len);
int CMD_ReqRtc(int len);
// int CMD_Start(int len);

const cmd_t GRP_SIGN_CMD[] = {
	{CMD_ReqSt, CMD_REQ_ST, 2},
	{CMD_ReqExtSt, CMD_REQ_EXT_ST, 11},
	{CMD_ReqVer, CMD_REQ_VER, 2},
	{CMD_SetRtc, CMD_SET_RTC, 10},
	{CMD_ReqRtc, CMD_REQ_RTC, 2},
	//	{CMD_Start, CMD_START, 14},
	//	{CMD_Sync, CMD_SYNC, 2},
	//	{CMD_SetTxtFrm, CMD_SET_TXT_FRM, 0},
	//  {CMD_SetGfxFrm, CMD_SET_GFX_FRM, 0},
	{CMD_SD_Frm, CMD_DISP_FRM, 3},
	{CMD_SD_Frm, CMD_STR_FRM, 3},
	{CMD_SetSpecailFrm, CMD_SET_SPC_FRM, 2},
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
	signStatus.lantn_fan_fault = 0x40; // set boot flag
}

uint8_t TaskProtc()
{
	PT_BEGIN(this_pt);
	for (;;)
	{
		wdt |= WDT_TASK_PROTC;
		if (RxCmd(&rxCmd) > 0)
		{
			if (rxCmd.buffer[CMD_INDEX_SLVID] == signID || rxCmd.buffer[CMD_INDEX_SLVID] == 0xFF)
			{
				SetMsTmr(this_tmr, SLV_TIMEOUT);
				for (int i = 0; i < sizeof(GRP_SIGN_CMD) / sizeof(GRP_SIGN_CMD[0]); i++)
				{
					if (rxCmd.buffer[CMD_INDEX_CODE] == GRP_SIGN_CMD[i].cmd_id &&
						(GRP_SIGN_CMD[i].cmd_len == 0 || GRP_SIGN_CMD[i].cmd_len == rxCmd.rxLen))
					{
						int len = (*GRP_SIGN_CMD[i].func)(rxCmd.rxLen);
						if (rxCmd.buffer[CMD_INDEX_SLVID] == signID && len > 0)
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
	*p++ = signID;
	*p++ = CMD_RPL_ST;
	*p++ = 0; // signStatus.chain_fault;
	*p++ = 0; // signStatus.over_temp;
	*p++ = 0; // signStatus.self_test;
	*p++ = signStatus.single_led;
	*p++ = signStatus.lantn_fan_fault;
	*p++ = signStatus.litsnsr_fault & 0x03;
	*p++ = signStatus.current_id;
	p = Cnvt_PutU16(signStatus.current_crc, p);
	*p++ = signStatus.next_id;
	p = Cnvt_PutU16(signStatus.next_crc, p);
	signStatus.lantn_fan_fault &= ~0x40; // clear boot flag
	return p - ptcTxBufHalf;
}

#define REQ_EXT_ST_PWR_FAN 2
#define REQ_EXT_ST_DIMMING 3
int CMD_ReqExtSt(int len)
{
	uint8_t *p = &rxCmd.buffer[REQ_EXT_ST_DIMMING];
	for (int i = 0; i < 4; i++)
	{
		signExtStatus.gain[i] = *p++;
		signExtStatus.bright[i] = *p++;
	}
	p = ptcTxBufHalf;
	*p++ = signID;
	*p++ = CMD_RPL_EXT_ST;
	*p++ = rxCmd.buffer[REQ_EXT_ST_PWR_FAN]; //signExtStatus.control
	uint8_t *pDimming = &rxCmd.buffer[REQ_EXT_ST_DIMMING];
	for (int i = 0; i < 8; i++)
	{
		*p++ = *pDimming++;
	}
	*p++ = Cnvt_PutU16(12000 /*signExtStatus.vin[AD_VIN_CHN]*/, p);
	signExtStatus.hours = Get_sys_seconds() / 3600;
	p = Cnvt_PutU16(signExtStatus.hours, p);
	p = Cnvt_PutU16(signExtStatus.t_board, p);
	*p++ = 0; // signExtStatus.humidity;
	p = Cnvt_PutU16(signExtStatus.vlux, p);
	*p++ = 1; // thisSign->tiles;
	*p++ = 1; // thisTile->pixelColors;
	*p++ = signExtStatus.fault_led;
	return p - ptcTxBufHalf;
}

#define SET_SPC_FRM_FRMID 2
#define SET_SPC_FRM_SPCID 6
#define SPC_FRM_STATIC 1
#define SPC_FRM_FLASHING 2
#define SPC_FRM_EMPTY 3

int CMD_SetSpecailFrm(int len)
{
	uint8_t frmid = rxCmd.buffer[SET_SPC_FRM_FRMID];
	uint8_t spcid = rxCmd.buffer[SET_SPC_FRM_SPCID];
	if ((frmid == 1) &&																	  // frame ID, only 1 is allowed
		(spcid == SPC_FRM_STATIC || spcid == SPC_FRM_FLASHING || spcid == SPC_FRM_EMPTY)) // special frame id
	{
		frame_t *f = &frames[frmid];
		if (spcid == SPC_FRM_STATIC)
		{
			f->conspicuity = CONSPICUITY_ALL_ON;
		}
		else if (spcid == SPC_FRM_FLASHING)
		{
			f->conspicuity = CONSPICUITY_ALL_FLASH;
		}
		else if (spcid == SPC_FRM_EMPTY)
		{
			f->conspicuity = CONSPICUITY_ALL_OFF;
		}
		f->crc = signStatus.next_crc = rxCmd.buffer[len - 2] * 0x100 + rxCmd.buffer[len - 1];
		signStatus.next_id = frmid;
	}
	return 0;
}

static int new_frmid = -1;
int GetDispNewFrame()
{
	return new_frmid;
}

void SetDispNewFrame(int frmid)
{
	new_frmid = frmid;
	if (new_frmid >= 0)
	{
		signStatus.current_id = signStatus.next_id = frmid;
		signStatus.current_crc = signStatus.next_crc = frames[frmid].crc;
	}
}

#define SD_FRM_FRMID 2
int CMD_SD_Frm(int len)
{
	uint8_t frmid = rxCmd.buffer[SD_FRM_FRMID];
	if (frmid < FRAMES_SIZE)
	{
		SetDispNewFrame(frmid);
	}
	return 0;
}

int CMD_ReqVer(int len)
{
	uint8_t *p = ptcTxBufHalf;
	*p++ = signID;
	*p++ = CMD_RPL_VER;
	*p++ = PROTOCOL_VER_MAJOR;
	*p++ = PROTOCOL_VER_MINOR;
	p = Cnvt_PutU16(FW_VERSION, p);
	p = Cnvt_PutU32(FW_BUILD, p);
	return p - ptcTxBufHalf;
}

int CMD_ReqRtc(int len)
{
	uint8_t *p = ptcTxBufHalf;
	*p++ = signID;
	time_t ts = GetTimestamp();
	p = Cnvt_PutU32((uint32_t)(ts >> 32), p);
	p = Cnvt_PutU32((uint32_t)(ts), p);
	return p - ptcTxBufHalf;
}

#define SET_RTC_TIME_T 2
int CMD_SetRtc(int len)
{
	time_t tv_sec;
	uint8_t *p = &rxCmd.buffer[SET_RTC_TIME_T];
	tv_sec = Cnvt_GetU32(p) * 0x100000000;
	p += 4;
	tv_sec += Cnvt_GetU32(p);
	SetTimestamp(tv_sec);
	if (DS3231SetTime(&hi2c2, tv_sec) == DS3231_NG)
	{ // ignore error
	  // MyPrintf("DS3231SetTime failed\n");
	}
	return 0;
}

void TaskProtcInit()
{
	this_Init();
	PT_Reset(this_pt);
	SerialPortStartRx(this_sp);
}

#endif // GC_MODE == GC_MODE_RS485
