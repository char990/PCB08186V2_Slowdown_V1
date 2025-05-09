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
#include "SignStatus.h"
#include "Debounce.h"

#define ONE_PLUS_PWM (0x80)
#define NO_ADC (0)
#define RUN_ADC (1)

#define FC_DNB_CNT 5

enum conspicuity_state_t
{
    CONSPICUITY_ST_ALL_OFF = 0,
    CONSPICUITY_ST_FLASH_ON = 1,
    CONSPICUITY_ST_FLASH_OFF = 2,
    CONSPICUITY_ST_ALL_ON = 3,
};

enum frma_conspicuity_setting_t
{
    FRM_CONSPICUITY_OFF = 0,
    FRM_CONSPICUITY_ALL_FLASH = 4,
    FRM_CONSPICUITY_ALL_ON = 5,
};

myPt_t ptFlasher;
#define this_pt (&ptFlasher)

msTmr_t tmrFlasher;
#define this_tmr (&tmrFlasher)

uint16_t conspicuity_0_cnt;

msTmr_t tmrAdcCalibration;

debounce_t dbnCurrent;

uint8_t st_conspicuity;

void TaskFlasherInit()
{
    PT_Reset(this_pt);
    SetDuty(PORT_HC1, 0);
    SetDuty(PORT_HC2, 0);
    st_conspicuity = CONSPICUITY_ST_ALL_OFF;
    AdcInit();
    AdcCalibration();
    SetMsTmr(&tmrAdcCalibration, CALIBRATION_TIME);
    DbnInit(&dbnCurrent, FC_DNB_CNT, 0);
}

void SetLocalFlasher(uint8_t adc_flag, uint8_t pwm_b1b0)
{
    static uint8_t pwm_b1b0_bak = 0xFF;
    static uint8_t pwm_bright = 1;
    if (IsMsTmrExpired(&tmrAdcCalibration))
    {
        AdcCalibration();
        SetMsTmr(&tmrAdcCalibration, CALIBRATION_TIME);
    }

    if (pwm_b1b0_bak != pwm_b1b0 || pwm_bright != signExtStatus.bright[0])
    {
        pwm_b1b0_bak = pwm_b1b0;
        pwm_bright = signExtStatus.bright[0];
        SetDuty(PORT_HC1, (pwm_b1b0 & (1 << PORT_HC1)) ? pwm_bright : 0);
    }

    if (adc_flag)
    {
        if (RunAdc() != 0)
        {
            adc_flag = 0; // skip this round
        }
    }

    if (pwm_b1b0 & ONE_PLUS_PWM)
    {
        SetDuty(PORT_HC1, 0);
        pwm_b1b0_bak = 0;
    }

    if (adc_flag)
    {
        if (pwm_b1b0 & (1 << PORT_HC1))
        {
            signExtStatus.fl_cur = GetHCmA(PORT_HC1);
            DbnCheck(&dbnCurrent, signExtStatus.fl_cur >= FC_MIN && signExtStatus.fl_cur <= FC_MAX);
        }
    }
}

void CheckNewFrame()
{
    int newFrm = GetDispNewFrame();
    if (newFrm >= 0 && newFrm < FRAMES_SIZE)
    {
        if (newFrm == 0)
        {
            if (st_conspicuity != CONSPICUITY_ST_ALL_OFF)
            {
                conspicuity_0_cnt = 0;
            }
            st_conspicuity = CONSPICUITY_ST_ALL_OFF;
            SetLocalFlasher(NO_ADC, 0);
        }
        else
        {
            st_conspicuity = (frames[newFrm].conspicuity) == FRM_CONSPICUITY_ALL_FLASH ? CONSPICUITY_ST_FLASH_OFF : CONSPICUITY_ST_ALL_ON;
            conspicuity_0_cnt = 0;
        }
        SetDispNewFrame(-1);
    }
}

uint8_t TaskFlasher()
{
    PT_BEGIN(this_pt);
    for (int i = 0; i < FC_DNB_CNT * 2; i++)
    {
        SetLocalFlasher(RUN_ADC, (1 << PORT_HC1));
        if (IsDbnValid(&dbnCurrent))
        {
            SetFaultSt(FAULT_ST_CED);
            break;
        }
    }
    SetLocalFlasher(NO_ADC, 0);
    for (;;)
    {
        wdt |= WDT_TASK_FL;
        SetMsTmr(this_tmr, 500);
        CheckNewFrame();
        if (st_conspicuity == CONSPICUITY_ST_ALL_OFF)
        { // off
            if (++conspicuity_0_cnt >= (CONSPICUITY_0_CHECKING_SEC * 2))
            {
                SetLocalFlasher(RUN_ADC, ONE_PLUS_PWM | (1 << PORT_HC1));
                conspicuity_0_cnt = 0;
            }
        }
        else if (st_conspicuity == CONSPICUITY_ST_ALL_ON)
        { // on
            SetLocalFlasher(RUN_ADC, (1 << PORT_HC1));
        }
        else
        { // Flash
            if (st_conspicuity == CONSPICUITY_ST_FLASH_OFF)
            {
                SetLocalFlasher(RUN_ADC, (1 << PORT_HC1));
                st_conspicuity = CONSPICUITY_ST_FLASH_ON;
            }
            else // if (st_conspicuity == CONSPICUITY_ST_ALL_ON)
            {
                SetLocalFlasher(NO_ADC, 0);
                st_conspicuity = CONSPICUITY_ST_FLASH_OFF;
            }
        }
        PT_WAIT_UNTIL(this_pt, IsMsTmrExpired(this_tmr));
    }
    PT_END(this_pt);
}
