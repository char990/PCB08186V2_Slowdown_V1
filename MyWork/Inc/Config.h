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

/* { Definitions_for_Conditional_Compilation */
#define GC_MODE_RS485 485
#define GC_MODE_IO 10

#define GC_MODE GC_MODE_RS485

#define PB15_EN 0
#define HC2_ADC_EN 0
/* Definitions_for_Conditional_Compilation } */

// Flasher current range(mA)
#define FC_MIN 0
#define FC_MAX 500

#define CONSPICUITY_0_CHECKING_SEC 300

#define ADC_DELAY_US 100
#define ADC_TIMEOUT_US 500
#define CALIBRATION_TIME (3600 * 1000U)

// PB12-13: INPUT
#define GetPB(pb) HAL_GPIO_ReadPin(PB##pb##_GPIO_Port, PB##pb##_Pin)
#define IsPBHigh(pb) (HAL_GPIO_ReadPin(PB##pb##_GPIO_Port, PB##pb##_Pin) == GPIO_PIN_SET)
#define IsPBLow(pb) (HAL_GPIO_ReadPin(PB##pb##_GPIO_Port, PB##pb##_Pin) == GPIO_PIN_RESET)

// PB14-15: OUTPUT
#define SetPB(pb) HAL_GPIO_WritePin(PB##pb##_GPIO_Port, PB##pb##_Pin, GPIO_PIN_SET)
#define ClrPB(pb) HAL_GPIO_WritePin(PB##pb##_GPIO_Port, PB##pb##_Pin, GPIO_PIN_RESET)

//#define SKIP_CRC

#ifdef __cplusplus
}
#endif

#endif /* INC_CONFIG_H_ */
