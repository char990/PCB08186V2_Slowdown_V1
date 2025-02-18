/*
 * Tasks.h
 *
 *  Created on: Sep 12, 2023
 *      Author: lq
 */

#ifndef INC_TASKS_H_
#define INC_TASKS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdint.h"

#define WDT_TASK_FL (1 << 0)
#define WDT_TASK_SP (1 << 1)
#define WDT_TASK_LT (1 << 2)

#define TASK_ALL (WDT_TASK_FL | WDT_TASK_SP | WDT_TASK_LT)

    extern uint8_t wdt;

    void TasksRun();

    extern uint8_t conspicuity_changed;
    extern uint8_t st_conspicuity;
    extern uint8_t st_pwm;
    extern uint8_t st_bootup;

#define LUX_FRONT 0
#define LUX_BACK 1
    extern uint16_t st_lux[2];

#define PORT_HC1 0
#define PORT_HC2 1
    extern uint16_t st_flasherCurrent[2];

#ifdef __cplusplus
}
#endif

#endif /* INC_TASKS_H_ */
