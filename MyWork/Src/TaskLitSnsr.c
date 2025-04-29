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
#include "SignStatus.h"
#include "Debounce.h"
#include "Config.h"

#define LIT_DBN_CNT 5

debounce_t dbnLit;

#if GC_MODE == GC_MODE_IO
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
#endif

myPt_t ptLit;
#define this_pt (&ptLit)

msTmr_t tmrLit;
#define this_tmr (&tmrLit)

#define opt4001hi2c (&hi2c1)
static uint8_t opt4001addr[2] = {OPT4001_AD0, OPT4001_AD1};

void SetBright(uint8_t bright)
{
    for (int i = 0; i < 4; i++)
    {
        signExtStatus.bright[i] = bright;
    }
}

void TaskLitSnsrInit()
{
    PT_Reset(this_pt);
    SetBright(24); // default level = 8
}

uint8_t TaskLitSnsr()
{
    PT_BEGIN(this_pt);
    for (int i = 0; i < 2; i++)
    {
        OPT4001Init(opt4001hi2c, opt4001addr[i]);
    }
    DbnInit(&dbnLit, LIT_DBN_CNT, 0);
    for (;;)
    {
        wdt |= WDT_TASK_LS;
        SetMsTmr(this_tmr, 1000);
        {
            int x = this_pt->status++ & 1;
            signExtStatus.lux[x] = GetLux(opt4001hi2c, opt4001addr[x]);
            signExtStatus.vlux = (signExtStatus.lux[0] > signExtStatus.lux[1]) ? signExtStatus.lux[0] : signExtStatus.lux[1];
            DbnCheck(&dbnLit, signExtStatus.vlux > 0);
            if (IsDbnValid(&dbnLit))
            {
                SetFaultSt(FAULT_ST_LITSNSR);
            }
#if GC_MODE == GC_MODE_IO
            if (signExtStatus.vlux)
            {
                enum TwilightStatus tw = GetTwilightStatus(GetTimestamp());
                switch (tw)
                {
                case TW_ST_NIGHT:
                    SetBright(Dimming[1 - 1]);
                    break;
                case TW_ST_DAY:
                    SetBright(Dimming[16 - 1]);
                    break;
                default:
                    SetBright(Dimming[8 - 1]);
                    break;
                }
            }
            else
            {
                for (int i = 0; i < 16; i++)
                {
                    if (signExtStatus.vlux <= Luminance[i])
                    {
                        SetBright(Dimming[i]);
                        break;
                    }
                }
            }
#endif
        }
        MyI2C_CheckBus(opt4001hi2c);
        PT_WAIT_UNTIL(this_pt, IsMsTmrExpired(this_tmr));
    }
    PT_END(this_pt);
}
