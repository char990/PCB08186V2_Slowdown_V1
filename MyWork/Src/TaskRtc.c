/*
 * TaskRtc.c
 *
 *  Created on: Sep 12, 2023
 *      Author: lq
 */
#include <TaskRtc.h>

#include "i2c.h"

#include "MyTmr.h"
#include "DS3231.h"

#include "Consts.h"
#include "Tasks.h"

#include "Config.h"
#include "SignStatus.h"

myPt_t ptRtc;
#define this_pt (&ptRtc)

msTmr_t tmrRdTemp;
msTmr_t tmrRdTime;

uint8_t TaskRtc()
{
	PT_BEGIN(this_pt);
	MakeMsTmrExpired(&tmrRdTime);
	for (;;)
	{
		wdt |= WDT_TASK_RTC;
		SetMsTmr(&tmrRdTemp, 1000); // 1 sec
		{
			int temp;
			if (DS3231GetTemp(&hi2c2, &temp) == DS3231_OK)
			{
				if (temp == 0)
				{
					temp = 1;
				}
				signExtStatus.t_board = temp * 10; // 0.1 degree
			}
			else
			{
				signExtStatus.t_board = 0;
			}
		}
		if (IsMsTmrExpired(&tmrRdTime))
		{
			SetMsTmr(&tmrRdTime, 3600000); // 1 hour
			time_t ts = DS3231GetTime(&hi2c2);
			if (ts != -1)
			{
				if (ts != GetTimestamp())
				{
					SetTimestamp(ts);
				}
			}
		}
		PT_WAIT_UNTIL(this_pt, IsMsTmrExpired(&tmrRdTemp));
	}
	PT_END(this_pt);
}

void this_Init()
{
}

void TaskRtcInit()
{
	this_Init();
	PT_Reset(this_pt);
	DS3231Init(&hi2c2);
}
