/*
 * TaskInputs.c
 *
 *  Created on: Sep 12, 2023
 *      Author: lq
 */
#include <TaskInputs.h>
#if GC_MODE == GC_MODE_IO
#include "MyTmr.h"
#include "Consts.h"
#include "Tasks.h"
#include "Debounce.h"
#include "main.h"
#include "SignStatus.h"

myPt_t ptInputs;
#define this_pt (&ptInputs)

msTmr_t tmrInputs;

debounce_t inputs;

#define PB12_INBIT (0)
#define PB12_MASK (1 << PB12_INBIT)
#define PB13_INBIT (1)
#define PB13_MASK (1 << PB13_INBIT)

uint8_t RdInputs()
{
	uint8_t val = 0;
	if (HAL_GPIO_ReadPin(GPIOB, PB12_Pin))
	{
		val |= PB12_MASK; // PB12
	}
	if (HAL_GPIO_ReadPin(GPIOB, PB13_Pin))
	{
		val |= PB13_MASK; // PB13
	}
	return val;
}

uint8_t TaskInputs()
{
	PT_BEGIN(this_pt);
	for (;;)
	{
		wdt |= WDT_TASK_INPUTS;
		SetMsTmr(&tmrInputs, 10);	   // 10ms
		DbnCheck(&inputs, RdInputs()); // Read the input pin
		if (IsDbnValid(&inputs))
		{
			if (inputs.event)
			{
				switch (inputs.curVal)
				{
				case 0:
					SetDispNewFrame(0);
					frames[1].conspicuity = CONSPICUITY_ALL_OFF;
					break;
				case PB12_MASK:
					SetDispNewFrame(1);
					frames[1].conspicuity = CONSPICUITY_ALL_ON;
					break;
				case PB13_MASK:
					SetDispNewFrame(2);
					frames[1].conspicuity = CONSPICUITY_ALL_FLASH;
					break;
				case PB12_MASK | PB13_MASK:
					SetDispNewFrame(3);
					frames[1].conspicuity = CONSPICUITY_ALL_OFF;
					break;
				}
				DbnClrEvt(&inputs);
			}
		}
		PT_WAIT_UNTIL(this_pt, IsMsTmrExpired(&tmrInputs));
	}
	PT_END(this_pt);
}

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

static void this_Init()
{
	DbnInit(&inputs, 5, 0xFF); // 5*10ms debounce time
}

void TaskInputsInit()
{
	this_Init();
	PT_Reset(this_pt);
}
#endif