/*
 * ScanAdc.c
 *
 *  Created on: Dec 4, 2023
 *      Author: lq
 */
#include "ScanAdc.h"
#include "adc.h"
#include "Config.h"
#include "MyTmr.h"
#include "stm32f0xx_hal_adc.h"
#include "stm32f0xx_ll_adc.h"

int Get_mv(adcval_t adv, adcval_t ref)
{
	return adv * 3300UL / 4096;
	return (ref == 0) ? (0) : ((VREFINT_CAL_VREF * (*VREFINT_CAL_ADDR) / 4096) * adv / ref);
}

adcval_t pData[ADC_SIZE];

adc_t scanAdc;
void AdcInit()
{
	AdcCalibration();
	scanAdc.hadc = &hadc;
	scanAdc.adc_st = 0;
	scanAdc.chs = ADC_SIZE;
	scanAdc.dma_buf = pData;
}

void AdcCalibration()
{
	HAL_ADCEx_Calibration_Start(&hadc);
}

int RunAdc()
{
	adc_t *adc = &scanAdc;
	uint32_t timeout_us = ADC_TIMEOUT_US;
	adc->adc_st = 0;
	int rtv = 0;
	if (HAL_OK != HAL_ADC_Start_DMA(adc->hadc, (uint32_t *)(adc->dma_buf), adc->chs))
	{
		rtv = 1;
	}
	else
	{
		usTmr_t us;
		SetUsTmr(&us, timeout_us);
		while (adc->adc_st == 0)
		{
			if (IsUsTmrExpired(&us))
			{
				rtv = 1;
				break;
			}
		}
		if (HAL_OK != HAL_ADC_Stop_DMA(adc->hadc))
		{
			rtv = 1;
		}
	}
	if (rtv)
	{
		adc->adc_st = -1;
		for (int i = 0; i < ADC_SIZE; i++)
		{
			pData[i] = 0;
		}
	}
	return rtv;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
	if (hadc == scanAdc.hadc)
	{
		scanAdc.adc_st = 1;
	}
}

/// @brief  Get HC1-2(0-1)
int GetHCmA(int chn)
{
	int ma = (scanAdc.adc_st >= 0) ? (Get_mv(pData[chn], pData[ADC_VREF]) * 2) : 0;
	return ma;
}

/*
 *         TEMPSENSOR_CAL2_TEMP - TEMPSENSOR_CAL1_TEMP
 * T('c) = ------------------------------------------- X (TEMPSENSOR_data - TEMPSENSOR_CAL1) + 30
 *              TEMPSENSOR_CAL2 - TEMPSENSOR_CAL1
 *
 * TEMPSENSOR_CAL_VREFANALOG = 3300
 * TEMPSENSOR_CAL2_TEMP = 110
 * TEMPSENSOR_CAL1_TEMP = 30
 * TEMPSENSOR_CAL2 = (*TEMPSENSOR_CAL2_ADDR) value when vref=3300
 * TEMPSENSOR_CAL1 = (*TEMPSENSOR_CAL1_ADDR) value when vref=3300
 */
int8_t GetTcpu()
{
	if (scanAdc.adc_st >= 0)
	{
		int c1 = *TEMPSENSOR_CAL1_ADDR;
		int c2 = *TEMPSENSOR_CAL2_ADDR;
		int t = ((TEMPSENSOR_CAL2_TEMP - TEMPSENSOR_CAL1_TEMP) * (pData[ADC_T_CPU] - c1)) / (c2 - c1) + 30;
		return (t < 0) ? 0 : ((t > 127) ? 127 : t);
	}
	return 0;
}

/// @brief  Get Vref
/// @return adcval_t
adcval_t GetVref()
{
	return (scanAdc.adc_st >= 0) ? pData[ADC_VREF] : 0;
}
