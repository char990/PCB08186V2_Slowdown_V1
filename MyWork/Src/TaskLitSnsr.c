/*
 * TaskLitSnsr.c
 *
 *  Created on: Oct 18, 2023
 *      Author: lq
 */
#include "TaskLitSnsr.h"
#include "MyTmr.h"
#include "main.h"
#include "MyI2C.h"
#include "OPT4001.h"
#include "Tasks.h"

myPt_t ptLit;
#define this_pt (&ptLit)

msTmr_t tmrLit;
#define this_tmr (&tmrLit)

#define opt4001hi2c (&hi2c1)
static uint8_t opt4001addr[2] = {OPT4001_AD0, OPT4001_AD1};
// TODO 2 light sensor
void TaskLitSnsrInit()
{
    PT_Reset(this_pt);
}

uint8_t TaskLitSnsr()
{
    PT_BEGIN(this_pt);
    for (int i = 0; i < 2; i++)
    {
        OPT4001Init(opt4001hi2c, opt4001addr[i]);
    }
    for (;;)
    {
        wdt |= WDT_TASK_LT;
        SetMsTmr(this_tmr, 1000);
        {
            int x = this_pt->status++ & 1;
            st_lux[x] = GetLux(opt4001hi2c, opt4001addr[x]);
        }
        MyI2C_CheckBus(opt4001hi2c);
        PT_WAIT_UNTIL(this_pt, IsMsTmrExpired(this_tmr));
    }
    PT_END(this_pt);
}
