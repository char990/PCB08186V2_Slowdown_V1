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
#include "twilight.h"

uint8_t Dimming[16] = {
    3, 4, 5, 7,
    10, 13, 18, 24,
    32, 43, 58, 78,
    105, 141, 190, 255};
uint16_t Luminance[16] = {
    4, 13, 40, 86,
    186, 400, 634, 1005,
    1592, 2524, 4000, 6340,
    10048, 15924, 25238, 65535};

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
        wdt |= WDT_TASK_LS;
        SetMsTmr(this_tmr, 1000);
        {
            int x = this_pt->status++ & 1;
            st_lux[x] = GetLux(opt4001hi2c, opt4001addr[x]);
            if (st_lux[0] == 0 && st_lux[1] == 0)
            {
                enum TwilightStatus tw = GetTwilightStatus(GetTimestamp());
                switch (tw)
                {
                case TW_ST_NIGHT:
                    st_pwm = Dimming[1 - 1];
                    break;
                case TW_ST_DAY:
                    st_pwm = Dimming[16 - 1];
                    break;
                default:
                    st_pwm = Dimming[8 - 1];
                    break;
                }
            }
            else
            {
                uint32_t lux = (st_lux[0] > st_lux[1]) ? st_lux[0] : st_lux[1];
                for (int i = 0; i < 16; i++)
                {
                    if (lux <= Luminance[i])
                    {
                        st_pwm = Dimming[i];
                        break;
                    }
                }
            }
        }
        MyI2C_CheckBus(opt4001hi2c);
        PT_WAIT_UNTIL(this_pt, IsMsTmrExpired(this_tmr));
    }
    PT_END(this_pt);
}
