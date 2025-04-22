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
#define WDT_TASK_PROTC (1 << 1)
#define WDT_TASK_INPUTS (1 << 1)
#define WDT_TASK_LS (1 << 2)
#define WDT_TASK_CLI (1 << 3)
#define WDT_TASK_RTC (1 << 4)

#define TASK_ALL (WDT_TASK_FL | WDT_TASK_PROTC | WDT_TASK_LS | WDT_TASK_CLI | WDT_TASK_RTC | WDT_TASK_INPUTS)

    extern uint8_t wdt;

    void TasksRun();

    extern uint8_t conspicuity_changed;
    extern uint8_t st_conspicuity;

#define LUX_FRONT 0
#define LUX_BACK 1

#define PORT_HC1 0
#define PORT_HC2 1

#ifdef __cplusplus
}
#endif

#endif /* INC_TASKS_H_ */
