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
#include "TaskInputs.h"
#include "glibc_env.h"
#include "SignStatus.h"
#include "Config.h"

uint8_t wdt;

#define FAULT_ST_FAN (1 << 0)
#define FAULT_ST_LITSNSR (1 << 1)
#define FAULT_ST_LANTERN (1 << 2)
#define FAULT_ST_CED (1 << 3)
#define FAULT_ST_CHAIN (1 << 4)
#define FAULT_ST_VOLTAGE (1 << 5)
#define FAULT_ST_TEMPERATURE (1 << 6)
#define FAULT_ST_HUMIDITY (1 << 7)

void TasksRun()
{
	SetFaultSt(~(FAULT_ST_CED | FAULT_ST_LITSNSR));
	GlibcEnvInit();
	SerialPortInit();
	TaskHbInit();

#if GC_MODE == GC_MODE_RS485
	TaskProtcInit();
#endif

	TaskInputsInit();
	TaskCliInit();
	TaskFlasherInit();
	TaskLitSnsrInit();
	TaskRtcInit();
	while (1)
	{
		TaskHb();
#if GC_MODE == GC_MODE_RS485
		TaskProtc();
#endif
		TaskInputs();
		TaskCli();
		TaskFlasher();
		TaskLitSnsr();
		TaskRtc();
	};
}
