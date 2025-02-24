/*
 * Version.c
 *
 *  Created on: Sep 12, 2023
 *      Author: lq
 */
#include <GetVer.h>
#include "stdio.h"

#define BUILD_NUMBER(yyMMdd, HHmm) (yyMMdd * 10000UL + HHmm)

const uint32_t FW_BUILD = BUILD_NUMBER(250224, 1228); // yyMMddHHmm

const uint16_t FW_VERSION = 0x0100; // 0130, max size = 4

int GetVer(char *buf)
{
#ifdef DEBUG
    const char *dr = "Debug";
#else
    const char *dr = "Release";
#endif
    return sprintf(buf, "%s Version:%04x Build:%lu", dr, FW_VERSION, FW_BUILD);
}
