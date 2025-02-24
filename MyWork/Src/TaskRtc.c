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

myPt_t ptRtc;
#define this_pt (&ptRtc)

msTmr_t tmrRdTemp;
msTmr_t tmrRdTime;

uint8_t TaskRtc()
{
	PT_BEGIN(this_pt);
	for (;;)
	{
		wdt |= WDT_TASK_RTC;
		if (IsMsTmrExpired(&tmrRdTime))
		{
			SetMsTmr(&tmrRdTime, 3600000); // 1 hour
			time_t ts = DS3231GetTime(&hi2c2);
			if (ts != -1)
			{
				st_ds3231 = DS3231_OK;
				if (ts != GetTimestamp())
				{
					SetTimestamp(ts);
				}
			}
			else
			{
				st_ds3231 = DS3231_NG;
			}
		}
		if (IsMsTmrExpired(&tmrRdTemp))
		{
			SetMsTmr(&tmrRdTemp, 1000); // 1 sec
			int temp;
			st_temp = (DS3231GetTemp(&hi2c2, &temp) == DS3231_OK) ? temp : -1;
		}
		PT_YIELD(this_pt);
	}
	PT_END(this_pt);
}

void this_Init()
{
	SetMsTmr(&tmrRdTime, 1);
	SetMsTmr(&tmrRdTemp, 1);
}

void TaskRtcInit()
{
	this_Init();
	PT_Reset(this_pt);
	DS3231Init(&hi2c2);
}
