/*
 * MyTmr.c
 *
 *  Created on: Sep 12, 2023
 *      Author: lq
 */
#include "MyTmr.h"
#include "stm32f0xx_hal.h"


void MakeMsTmrExpired(msTmr_t *tmr)
{
	tmr->tickstart = HAL_GetTick() - 1;
	tmr->wait = 1;
}

void SetMsTmr(msTmr_t *tmr, ms_t delay)
{
	tmr->tickstart = HAL_GetTick();
	tmr->wait = delay;
}

void AddMsTmr(msTmr_t *tmr, ms_t delay)

{
	tmr->tickstart = (tmr->wait != 0) ? (tmr->tickstart + tmr->wait) : HAL_GetTick();
	tmr->wait = delay;
}

void ClrMsTmr(msTmr_t *tmr)
{
	SetMsTmr(tmr, 0);
}

int IsMsTmrExpired(msTmr_t *tmr)
{
	return (tmr->wait != 0) && ((ms_t)(HAL_GetTick() - tmr->tickstart) >= tmr->wait);
}

int IsMsTmrSet(msTmr_t *tmr)
{
	return tmr->wait;
}

#define MY_US_TIM TIM14
void SetUsTmr(usTmr_t *tmr, us_t delay)
{
	tmr->tickstart = MY_US_TIM->CNT;
	tmr->wait = (delay >= US_MAX) ? 65535 : delay * US_TIM_MHZ;
}

void ClrUsTmr(usTmr_t *tmr)
{
	SetUsTmr(tmr, 0);
}

int IsUsTmrExpired(usTmr_t *tmr)
{
	return (tmr->wait != 0) && ((us_t)((us_t)(MY_US_TIM->CNT) - tmr->tickstart) >= tmr->wait);
}

int IsUsTmrSet(usTmr_t *tmr)
{
	return tmr->wait;
}

void Delay_us(us_t delay)
{
	usTmr_t tmr;
	SetUsTmr(&tmr, delay);
	while ((us_t)((us_t)(MY_US_TIM->CNT) - tmr.tickstart) < tmr.wait)
		;
}

static __IO uint32_t tick_ms;
static __IO time_t tick_seconds;

extern __IO uint32_t uwTick;
extern HAL_TickFreqTypeDef uwTickFreq;
void HAL_IncTick()
{
	uwTick += uwTickFreq;
	tick_ms += uwTickFreq;
	if (tick_ms >= 1000)
	{
		tick_ms -= 1000;
		tick_seconds++;
	}
}

time_t GetTimestamp()
{
	return tick_seconds;
}

void SetTimestamp(time_t t)
{
	tick_seconds = t;
}

time_t GetLocalTime(struct tm *localtm)
{
    time_t t = GetTimestamp();
    if (t > 0)
    {
		localtime_r(&t, localtm);
    }
	return t;
}
