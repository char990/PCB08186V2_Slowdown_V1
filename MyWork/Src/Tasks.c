/*
 * Tasks.c
 *
 *  Created on: Sep 12, 2023
 *      Author: lq
 */
#include "Tasks.h"
#include "SerialPort.h"
#include "TaskHb.h"
#include "TaskFlasher.h"
#include "TaskLitSnsr.h"
#include "TaskProtc.h"
#include "TaskCli.h"
#include "TaskRtc.h"
#include "glibc_env.h"

uint8_t wdt;

uint8_t conspicuity_changed;
uint8_t st_conspicuity;
uint8_t st_pwm;
uint8_t st_bootup;

int8_t st_temp;

uint8_t st_ds3231;

uint16_t st_lux[2];
uint16_t st_flasherCurrent[2];

void TasksRun()
{
	GlibcEnvInit();
	SerialPortInit();
	TaskHbInit();
	TaskProtcInit();
	TaskCliInit();
	TaskFlasherInit();
	TaskLitSnsrInit();
	TaskRtcInit();
	while (1)
	{
		TaskHb();
		TaskProtc();
		TaskCli();
		TaskFlasher();
		TaskLitSnsr();
		TaskRtc();
	};
}
