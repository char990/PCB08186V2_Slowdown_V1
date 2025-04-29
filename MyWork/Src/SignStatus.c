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

const frame_t frames[FRAMES_SIZE] =
    {
        {CONSPICUITY_ALL_OFF, 0},        // frame 0
        {CONSPICUITY_ALL_ON, 0x5A11},    // frame 1
        {CONSPICUITY_ALL_FLASH, 0x5A22}, // frame 2
};

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
