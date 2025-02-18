/*
 * Critical.h
 *
 *  Created on: Aug 22, 2023
 *      Author: lq
 */

#ifndef INC_UTILS_CRITICAL_H_
#define INC_UTILS_CRITICAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#if 1
#define ENTER_CRITICAL()                \
	uint32_t _critical_flag = __get_PRIMASK() & 0x1; \
	__disable_irq()

#define EXIT_CRITICAL() \
	if (_critical_flag == 0)         \
	{                      \
		__enable_irq();    \
	}
#else
#include "cmsis_os2.h"
#define ENTER_CRITICAL() int32_t _critical_state = osKernelLock()
#define EXIT_CRITICAL() osKernelRestoreLock(_critical_state)
#endif


#ifdef __cplusplus
}
#endif

#endif /* INC_UTILS_CRITICAL_H_ */
