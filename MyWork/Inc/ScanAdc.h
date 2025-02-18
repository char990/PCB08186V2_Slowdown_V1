/*
 * ScanAdc.h
 *
 *  Created on: Aug 25, 2023
 *      Author: lq
 */

#ifndef INC_MCUDRV_SCANADC_H_
#define INC_MCUDRV_SCANADC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdint.h"
#include "stm32f0xx_hal.h"

	typedef uint16_t adcval_t;
	typedef struct adc_t
	{
		ADC_HandleTypeDef *hadc;
		uint32_t chs;
		volatile int adc_st;		// -1:failed, 0:running, 1:completed
		adcval_t * dma_buf;
	} adc_t;

	extern adc_t scanAdc;

	void AdcInit();
	
	void AdcCalibration();

	int RunAdc();

#define ADC_HC1 0
#define ADC_HC2 1
#define ADC_T_CPU 2
#define ADC_VREF 3
#define ADC_HC_SIZE 2
#define ADC_SIZE 4

	/// @brief  Get HC1-2(0-1)
	int GetHCmA(int chn);

	/// @brief  Get internal temperature
	/// @return 'C
	int8_t GetTcpu();

	/// @brief  Get Vref
	/// @return adcval_t
	adcval_t GetVref();

    extern int Get_mv(adcval_t adv, adcval_t ref);

#ifdef __cplusplus
}
#endif

#endif /* INC_MCUDRV_SCANADC_H_ */
