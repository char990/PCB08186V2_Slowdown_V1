/*
 * TaskProtc.h
 *
 *  Created on: Sep 12, 2023
 *      Author: lq
 */

#ifndef INC_TASKPROTC_H_
#define INC_TASKPROTC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Config.h"

#if GC_MODE == GC_MODE_RS485

#include "MyPt.h"

void TaskProtcInit();
uint8_t TaskProtc();

int GetDispNewFrame();
void SetDispNewFrame(int frmid);

#endif

#ifdef __cplusplus
}
#endif

#endif /* INC_TASKPROTC_H_ */
