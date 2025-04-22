/*
 * Version.c
 *
 *  Created on: Sep 12, 2023
 *      Author: lq
 */
#include "FW_Version.h"

#define BUILD_NUMBER(yyMMdd, HHmm) (yyMMdd * 10000UL + HHmm)

const uint32_t FW_BUILD = BUILD_NUMBER(250422, 1345); // yyMMddHHmm

const uint16_t FW_VERSION = 0x0100; // 0130, max size = 4
