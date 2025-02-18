/*
 * Config.h
 *
 *  Created on: Oct 21, 2023
 *      Author: lq
 */

#ifndef INC_CONFIG_H_
#define INC_CONFIG_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define SLV_ID 0xA2

#define CONSPICUITY_0_CHECKING_SEC 300

#define ADC_DELAY_US 100
#define ADC_TIMEOUT_US 500
#define CALIBRATION_TIME (3600 * 1000U)

#if 1
#define RISING(pb) HAL_GPIO_WritePin(PB##pb##_GPIO_Port, PB##pb##_Pin, GPIO_PIN_SET)
#define FALLING(pb) HAL_GPIO_WritePin(PB##pb##_GPIO_Port, PB##pb##_Pin, GPIO_PIN_RESET)
#else
#define RISING(pb)
#define FALLING(pb)
#endif

//#define SKIP_CRC

#ifdef __cplusplus
}
#endif

#endif /* INC_CONFIG_H_ */
