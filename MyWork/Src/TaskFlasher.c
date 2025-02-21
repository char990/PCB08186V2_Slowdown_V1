/*
 * TaskFlasher.c
 *
 *  Created on: Oct 18, 2023
 *      Author: lq
 */
#include "TaskFlasher.h"
#include "MyTmr.h"
#include "main.h"
#include "Tasks.h"
#include "Consts.h"
#include "Pwm.h"
#include "MyTmr.h"
#include "ScanAdc.h"
#include "Config.h"
#include "Critical.h"

#define PORT_HC1 0
#define PORT_HC2 1

#define ONE_PLUS_PWM (0x80)
#define NO_ADC (0)
#define RUN_ADC (1)

myPt_t ptFlasher;
#define this_pt (&ptFlasher)

msTmr_t tmrFlasher;
#define this_tmr (&tmrFlasher)

uint16_t conspicuity_0_cnt;

msTmr_t tmrAdcCalibration;

void TaskFlasherInit()
{
    SetDuty(PORT_HC1, 0);
    SetDuty(PORT_HC2, 0);
    PT_Reset(this_pt);
    st_bootup = 0x80;
    st_conspicuity = 0;
    conspicuity_changed = 0;
    AdcInit();
    AdcCalibration();
    SetMsTmr(&tmrAdcCalibration, CALIBRATION_TIME);
}

void SetLocalFlasher(uint8_t adc_flag, uint8_t pwm_b1b0)
{
    static uint8_t pwm_b1b0_bak = 0xFF;
    static uint8_t pwm_v_bak = 1;
    if (IsMsTmrExpired(&tmrAdcCalibration))
    {
        AdcCalibration();
        SetMsTmr(&tmrAdcCalibration, CALIBRATION_TIME);
    }

    if (pwm_b1b0_bak != pwm_b1b0 || pwm_v_bak != st_pwm)
    {
        pwm_b1b0_bak = pwm_b1b0;
        pwm_v_bak = st_pwm;
        SetHcAllDuty((pwm_b1b0 & (1 << PORT_HC1)) ? pwm_v_bak : 0,
                     (pwm_b1b0 & (1 << PORT_HC2)) ? pwm_v_bak : 0);
    }

    if (adc_flag)
    {
        if (RunAdc() != 0)
        {
            adc_flag = 0;   // skip this round
        }
    }

    if (pwm_b1b0 & ONE_PLUS_PWM)
    {
        SetHcAllDuty(0, 0);
        pwm_b1b0_bak = 0;
    }

    if (adc_flag)
    {
        if (pwm_b1b0 & (1 << PORT_HC1))
        {
            st_flasherCurrent[PORT_HC1] = GetHCmA(PORT_HC1);
        }
        if (pwm_b1b0 & (1 << PORT_HC2))
        {
            st_flasherCurrent[PORT_HC2] = GetHCmA(PORT_HC2);
        }
    }
}

uint8_t TaskFlasher()
{
    PT_BEGIN(this_pt);
    for (;;)
    {
        wdt |= WDT_TASK_FL;
        if (conspicuity_changed)
        {
            conspicuity_changed = 0;
            if (st_conspicuity == 0)
            {
                SetLocalFlasher(NO_ADC, 0);
                if (++conspicuity_0_cnt >= (CONSPICUITY_0_CHECKING_SEC * 2))
                {
                    SetLocalFlasher(RUN_ADC, ONE_PLUS_PWM | (1 << PORT_HC1) | (1 << PORT_HC2));
                    conspicuity_0_cnt = 0;
                }
            }
            else
            {
                SetLocalFlasher(RUN_ADC, (1 << PORT_HC1) | (1 << PORT_HC2));
                conspicuity_0_cnt = 0;
            }
        }
        PT_YIELD(this_pt);
    }
    PT_END(this_pt);
}
