/*
 * SignStatus.h
 *
 *  Created on: Aug 18, 2023
 *      Author: lq
 */
#include "SignStatus.h"

uint8_t signID;

signStatus_t signStatus;

signExtStatus_t signExtStatus;

frame_t frames[FRAMES_SIZE];

void ClearStoredFrm()
{
    for (int i = 0; i < FRAMES_SIZE; i++)
    {
        frames[i].crc = 0;
        frames[i].frmSt = 0;
        frames[i].conspicuity = 0;
    }
}

uint8_t GetFaultSt()
{
    return signStatus.single_led;
}

void SetFaultSt(uint8_t i)
{
    signStatus.single_led |= i;
}

void ClrFaultSt(uint8_t i)
{
    signStatus.single_led &= ~i;
}
