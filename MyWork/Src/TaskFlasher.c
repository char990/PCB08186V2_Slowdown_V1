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

#define FLDRV_LEFT 0
#define FLDRV_RIGHT 1

#define FLASHER_UPPER 0
#define FLASHER_LOWER 1

#define ONE_PLUS_PWM (0x80)
#define NO_ADC (0)
#define RUN_ADC (1)

/*
Light sensor position
         [0][1]                  [1][1]
        +-------------back-------------+
LEFT    |       TOP view of Sign       |    RIGHT
        +-------------front------------+
         [0][0]                  [1][0]

Flasher position
  [0][0]+-------------upper------------+[1][0]
        |                              |
        |                              |
LEFT    |      Front view of Sign      |    RIGHT
        |                              |
        |                              |
  [0][1]+-------------lower------------+[1][1]
*/

#define FLDRV_SIDE FLDRV_RIGHT

myPt_t ptFlasher;
#define this_pt (&ptFlasher)

msTmr_t tmrFlasher;
#define this_tmr (&tmrFlasher)

uint16_t conspicuity_0_cnt;

msTmr_t tmrAdcCalibration;

void TaskFlasherInit()
{
    SetDuty(FLASHER_UPPER, 0);
    SetDuty(FLASHER_LOWER, 0);
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
        SetHcAllDuty((pwm_b1b0 & (1 << FLASHER_UPPER)) ? pwm_v_bak : 0,
                     (pwm_b1b0 & (1 << FLASHER_LOWER)) ? pwm_v_bak : 0);
    }

    if (adc_flag)
    {
        __disable_irq();
        pwm_status[0] = 0;
        pwm_status[1] = 0;
        __enable_irq();
        while (pwm_status[0] == 0 && pwm_status[1] == 0)
            ;
        Delay_us(ADC_DELAY_US);
        // RISING(15);
        if (RunAdc() != 0)
        {
            adc_flag = 0;   // skip this round
        }
        // FALLING(15);
    }

    if (pwm_b1b0 & ONE_PLUS_PWM)
    {
        SetHcAllDuty(0, 0);
        pwm_b1b0_bak = 0;
    }

    if (adc_flag)
    {
        if (pwm_b1b0 & (1 << FLASHER_UPPER))
        {
            st_flasherCurrent[FLASHER_UPPER] = GetHCmA(FLASHER_UPPER);
        }
        if (pwm_b1b0 & (1 << FLASHER_LOWER))
        {
            st_flasherCurrent[FLASHER_LOWER] = GetHCmA(FLASHER_LOWER);
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
                    SetLocalFlasher(RUN_ADC, ONE_PLUS_PWM | (1 << FLASHER_UPPER) | (1 << FLASHER_LOWER));
                    conspicuity_0_cnt = 0;
                }
            }
            else
            {
                SetLocalFlasher(RUN_ADC, st_conspicuity);
                conspicuity_0_cnt = 0;
            }
        }
        PT_YIELD(this_pt);
    }
    PT_END(this_pt);
}
