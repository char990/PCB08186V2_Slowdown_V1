/*
 * Pwm.h
 *
 *  Created on: Aug 25, 2023
 *      Author: lq
 */

#ifndef INC_MCUDRV_PWM_H_
#define INC_MCUDRV_PWM_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdint.h"

    /*
     * @brief Set pwm duty of on pwm-c
     * @param c: pwm of 0|1 ( = HC1|2)
     * @param duty: 0-255, 0=LOW and 255=HIGH
     *
     */
    void SetDuty(uint8_t c, uint8_t duty);

    void SetHcAllDuty(uint8_t duty1, uint8_t duty2);

    extern volatile int pwm_status[2];

#ifdef __cplusplus
}
#endif

#endif /* INC_MCUDRV_PWM_H_ */
