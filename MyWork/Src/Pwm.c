/*
 * Pwm.c
 *
 *  Created on: Aug 25, 2023
 *      Author: lq
 */
#include "Pwm.h"
#include "tim.h"

#define PWM_CHNS 2
static uint8_t duty_set[PWM_CHNS];

TIM_OC_InitTypeDef sConfigOC = {
	.OCPolarity = TIM_OCPOLARITY_LOW,
	//.OCNPolarity = TIM_OCNPOLARITY_HIGH,
	.OCFastMode = TIM_OCFAST_ENABLE,
	.OCIdleState = TIM_OCIDLESTATE_SET,
	.OCNIdleState = TIM_OCNIDLESTATE_RESET,
};

typedef struct pwm_t
{
	TIM_HandleTypeDef *htim;
	uint32_t chn;
} pwm_t;

volatile int pwm_status[PWM_CHNS];

const pwm_t pwm[PWM_CHNS] =
	{
		{&htim16, TIM_CHANNEL_1},
		{&htim17, TIM_CHANNEL_1},
};

#if 0
static void ReStartCnt()
{
	__disable_irq();
	TIM16->CNT=0;
	TIM17->CNT=0;
	TIM16->CR1|=TIM_CR1_CEN;
	TIM17->CR1|=TIM_CR1_CEN;
	__enable_irq();
}

static void RefreshHcAllDuty()
{
	for (int i = 0; i < PWM_CHNS; i++)
	{
		TIM_HandleTypeDef *htim = pwm[i].htim;
		uint32_t chn = pwm[i].chn;
		if (duty_set[i] == 0)
		{
			sConfigOC.OCMode = TIM_OCMODE_FORCED_INACTIVE;
		}
		else if (duty_set[i] == 255)
		{
			sConfigOC.OCMode = TIM_OCMODE_FORCED_ACTIVE;
		}
		else
		{
			sConfigOC.OCMode = TIM_OCMODE_PWM1;
			sConfigOC.Pulse = duty_set[i]*128;
		}
		HAL_TIM_PWM_Stop(htim, chn);
		HAL_TIM_Base_Stop(htim);
		if (HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, chn) != HAL_OK)
		{
			Error_Handler();
		}
		__HAL_TIM_ENABLE_IT(htim, TIM_IT_UPDATE);
		TIM_CHANNEL_STATE_SET(htim, chn, HAL_TIM_CHANNEL_STATE_BUSY);
		TIM_CCxChannelCmd(htim->Instance, chn, TIM_CCx_ENABLE);
		if (IS_TIM_BREAK_INSTANCE(htim->Instance) != RESET)
		{
			__HAL_TIM_MOE_ENABLE(htim);
		}
	}
	ReStartCnt();
}

void SetHcAllDuty(uint8_t duty1, uint8_t duty2)
{
	duty_set[0] = duty1;
	duty_set[1] = duty2;
	RefreshHcAllDuty();
}

void SetDuty(uint8_t c, uint8_t duty)
{
	duty_set[c] = duty;
	RefreshHcAllDuty();
}

#endif
static void _SetDuty(uint8_t i, uint8_t duty)
{
	TIM_HandleTypeDef *htim = pwm[i].htim;
	TIM_TypeDef *timInstance = htim->Instance;
	uint32_t chn = pwm[i].chn;
	if (duty == 0)
	{
		sConfigOC.OCMode = TIM_OCMODE_FORCED_INACTIVE;
	}
	else if (duty == 255)
	{
		sConfigOC.OCMode = TIM_OCMODE_FORCED_ACTIVE;
	}
	else
	{
		sConfigOC.OCMode = TIM_OCMODE_PWM1;
		sConfigOC.Pulse = duty * 128;
	}
	HAL_TIM_PWM_Stop(htim, chn);
	HAL_TIM_Base_Stop(htim);
	if (HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, chn) != HAL_OK)
	{
		Error_Handler();
	}
	__HAL_TIM_ENABLE_IT(htim, TIM_IT_UPDATE);
	TIM_CHANNEL_STATE_SET(htim, chn, HAL_TIM_CHANNEL_STATE_BUSY);
	TIM_CCxChannelCmd(timInstance, chn, TIM_CCx_ENABLE);
	if (IS_TIM_BREAK_INSTANCE(timInstance) != RESET)
	{
		__HAL_TIM_MOE_ENABLE(htim);
	}
}

void SetDuty(uint8_t i, uint8_t duty)
{
	if (i > 1)
	{
		return;
	}
	_SetDuty(i, duty);
	TIM_TypeDef *timInstance = pwm[i].htim->Instance;
	__disable_irq();
	timInstance->CNT = 0;
	timInstance->CR1 |= TIM_CR1_CEN;
	__enable_irq();
}

void SetHcAllDuty(uint8_t duty1, uint8_t duty2)
{
	_SetDuty(1 - 1, duty1);
	_SetDuty(2 - 1, duty2);
	TIM_TypeDef *timInstance0 = pwm[0].htim->Instance;
	TIM_TypeDef *timInstance1 = pwm[1].htim->Instance;
	__disable_irq();
	timInstance0->CNT = 0;
	timInstance1->CNT = 0;
	timInstance0->CR1 |= TIM_CR1_CEN;
	timInstance1->CR1 |= TIM_CR1_CEN;
	__enable_irq();
}

void PwmCallback(TIM_HandleTypeDef *htim)
{
	if (htim == pwm[0].htim)
	{
		pwm_status[0] = 1;
	}
	else if (htim == pwm[1].htim)
	{
		pwm_status[1] = 1;
	}
}
