/*
 * Critical.h
 *
 *  Created on: Aug 22, 2023
 *      Author: lq
 */

#ifndef INC_UTILS_CRITICAL_H_
#define INC_UTILS_CRITICAL_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define ENTER_CRITICAL_pri()                     \
	do                                           \
	{                                            \
		uint32_t priMask_flag = __get_PRIMASK(); \
	__disable_irq()

#define EXIT_CRITICAL_pri()      \
	__set_PRIMASK(priMask_flag); \
	}                            \
	while (0)

#define ATOMIC_CODE() \
	for (uint32_t CATAB(_priMask_, __LINE__) = __get_PRIMASK__disable_irq(), CATAB(_cnt_flag_, __LINE__) = 0; CATAB(_cnt_flag_, __LINE__)++ == 0; __set_PRIMASK(CATAB(_priMask_, __LINE__)))

#ifdef USE_CMSIS_OS
#include "cmsis_os.h"
#define MS_DELAY(ms) osDelay(ms)
#define ENTER_CRITICAL() taskENTER_CRITICAL()
#define EXIT_CRITICAL() taskEXIT_CRITICAL()
#else
#define MS_DELAY(ms) HAL_Delay(ms)
#define ENTER_CRITICAL() ENTER_CRITICAL_pri()
#define EXIT_CRITICAL() EXIT_CRITICAL_pri()
#endif

#ifdef __cplusplus
}
#endif

#endif /* INC_UTILS_CRITICAL_H_ */
