/*
 * TaskInputs.c
 *
 *  Created on: Sep 12, 2023
 *      Author: lq
 */
#include <TaskInputs.h>
#include "MyTmr.h"
#include "Consts.h"
#include "Tasks.h"
#include "Debounce.h"
#include "main.h"
#include "SignStatus.h"
#include "Config.h"

myPt_t ptInputs;
#define this_pt (&ptInputs)

msTmr_t tmrInputs;

uint8_t RdInputs(GPIO_TypeDef *GPIOx, uint16_t *GPIO_Pin, int size)
{
	uint8_t val = 0x00;
	uint8_t vbit = 0x01;
	for (int i = 0; i < size; i++)
	{
		if (HAL_GPIO_ReadPin(GPIOx, GPIO_Pin[i]) == GPIO_PIN_SET)
		{
			val |= vbit;
		}
		vbit <<= 1;
	}
	return val;
}

#if GC_MODE == GC_MODE_IO
static int new_frmid = -1;
int GetDispNewFrame()
{
	return new_frmid;
}

void SetDispNewFrame(int frmid)
{
	new_frmid = frmid;
	if (new_frmid >= 0)
	{
		signStatus.current_id = signStatus.next_id = frmid;
		signStatus.current_crc = signStatus.next_crc = frames[frmid].crc;
	}
}

debounce_t inputPB;

#define PB12_INBIT (0)
#define PB12_MASK (1 << PB12_INBIT)
#define PB13_INBIT (1)
#define PB13_MASK (1 << PB13_INBIT)

uint16_t PIN_PB[2] = {PB12_Pin, PB13_Pin};

void ProcessPB()
{
	DbnCheck(&inputPB, RdInputs(GPIOB, PIN_PB, 2));
	if (IsDbnValid(&inputPB) && inputPB.event)
	{
		switch (inputPB.curVal)
		{
		case 0:
		case PB12_MASK | PB13_MASK:
			SetDispNewFrame(0);
			break;
		case PB12_MASK:
			SetDispNewFrame(1);
			break;
		case PB13_MASK:
			SetDispNewFrame(2);
			break;
		}
		DbnClrEvt(&inputPB);
	}
}
#endif

debounce_t inputBtn;

#define BTN1_INBIT (0)
#define BTN1_MASK (1 << BTN1_INBIT)

uint16_t PIN_BTN[1] = {BTN1_Pin};

void ProcessBtn()
{
	extern void SetDispNewFrame(int frmid);
	DbnCheck(&inputBtn, RdInputs(GPIOA, PIN_BTN, COUNT_OF_ARRAY(PIN_BTN)));
	if (IsDbnValid(&inputBtn) && inputBtn.event)
	{
		if (inputBtn.rising == 1)
		{
			if (signStatus.self_test)
			{
				SetDispNewFrame(0);
				signStatus.self_test = 0;
			}
			else
			{
				SetDispNewFrame(2);
				signStatus.self_test = 1;
			}
		}
		DbnClrEvt(&inputBtn);
	}
}

uint8_t TaskInputs()
{
	PT_BEGIN(this_pt);
	for (;;)
	{
		wdt |= WDT_TASK_INPUTS;
		SetMsTmr(&tmrInputs, 10); // 10ms
#if GC_MODE == GC_MODE_IO
		ProcessPB();
#endif
		ProcessBtn();
		PT_WAIT_UNTIL(this_pt, IsMsTmrExpired(&tmrInputs));
	}
	PT_END(this_pt);
}

static void this_Init()
{
#if GC_MODE == GC_MODE_IO
	DbnInit(&inputPB, 5, (1 << COUNT_OF_ARRAY(PIN_PB)) - 1); // 5*10ms debounce time
#endif
	DbnInit(&inputBtn, 3, (1 << COUNT_OF_ARRAY(PIN_BTN)) - 1); // 3*10ms debounce time
}

void TaskInputsInit()
{
	this_Init();
	PT_Reset(this_pt);
}
