/*
 * MyCrc.c
 *
 *  Created on: Oct 20, 2023
 *      Author: lq
 */
#include "MyCrc.h"
#include "crc.h"


uint32_t CRC_Calculate(uint8_t *pData, uint32_t len)
{
    return HAL_CRC_Calculate(&hcrc, (uint32_t *)pData, len) ^ 0xFFFFFFFF;
}
