/*
 * Pwm.c
 *
 *  Created on: Aug 25, 2023
 *      Author: lq
 */
#include "Pwm.h"
#include "tim.h"

#define PWM_CHNS 2

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

void SetDuty(uint8_t c, uint8_t duty)
{
	TIM_HandleTypeDef *htim = pwm[c].htim;
	uint32_t chn = pwm[c].chn;
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
		sConfigOC.Pulse = duty;
	}
	HAL_TIM_PWM_Stop(htim, chn);
	if (HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, chn) != HAL_OK)
	{
		Error_Handler();
	}
	__HAL_TIM_ENABLE_IT(htim, TIM_IT_UPDATE);
	htim->Instance->CNT = 0;
	HAL_TIM_PWM_Start(htim, chn);
}

void SetHcAllDuty(uint8_t duty1, uint8_t duty2)
{
	uint8_t duty[PWM_CHNS] = {duty1, duty2};
	for (int i = 0; i < PWM_CHNS; i++)
	{
		TIM_HandleTypeDef *htim = pwm[i].htim;
		uint32_t chn = pwm[i].chn;
		if (duty[i] == 0)
		{
			sConfigOC.OCMode = TIM_OCMODE_FORCED_INACTIVE;
		}
		else if (duty[i] == 255)
		{
			sConfigOC.OCMode = TIM_OCMODE_FORCED_ACTIVE;
		}
		else
		{
			sConfigOC.OCMode = TIM_OCMODE_PWM1;
			sConfigOC.Pulse = duty[i];
		}
		HAL_TIM_PWM_Stop(htim, chn);
		if (HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, chn) != HAL_OK)
		{
			Error_Handler();
		}
		__HAL_TIM_ENABLE_IT(htim, TIM_IT_UPDATE);
		htim->Instance->CNT = 0;
		TIM_CHANNEL_STATE_SET(htim, chn, HAL_TIM_CHANNEL_STATE_BUSY);
		TIM_CCxChannelCmd(htim->Instance, chn, TIM_CCx_ENABLE);
		if (IS_TIM_BREAK_INSTANCE(htim->Instance) != RESET)
		{
			__HAL_TIM_MOE_ENABLE(htim);
		}
	}
	__disable_irq();
	__HAL_TIM_ENABLE(&htim16);
	__HAL_TIM_ENABLE(&htim17);
	__enable_irq();
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
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
