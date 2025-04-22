/*
 * TaskInputs.h
 *
 *  Created on: Sep 12, 2023
 *      Author: lq
 */

#ifndef INC_TASKInputs_H_
#define INC_TASKInputs_H_

#ifdef __cplusplus
extern "C" {
#endif

#if GC_MODE == GC_MODE_IO
#include "MyPt.h"

void TaskInputsInit();
uint8_t TaskInputs();

int GetDispNewFrame();
void SetDispNewFrame(int frmid);

#endif


#ifdef __cplusplus
}
#endif

#endif /* INC_TASKInputs_H_ */
