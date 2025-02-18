/*
 * MyCrc.h
 *
 *  Created on: Oct 20, 2023
 *      Author: lq
 */

#ifndef INC_MYCRC_H_
#define INC_MYCRC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdint.h"

uint32_t CRC_Calculate(uint8_t *pData, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* INC_MYCRC_H_ */
