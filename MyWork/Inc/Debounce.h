/*
 * Debounce.h
 *
 *  Created on: Aug 24, 2023
 *      Author: lq
 */

#ifndef INC_UTILS_DEBOUNCE_H_
#define INC_UTILS_DEBOUNCE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

typedef struct debounce_t
{
	uint8_t CNT;
	uint8_t preVal;
	uint8_t curVal;
	uint8_t temp;
	uint8_t cnt;
	uint8_t event;
	uint8_t rising;
	uint8_t falling;
}debounce_t;

void DbnInit(debounce_t *db, uint8_t CNT, uint8_t val);

void DbnCheck(debounce_t *db, uint8_t v);

int IsDbnValid(debounce_t *db);

void DbnClrEvt(debounce_t *db);

int DbnToString(debounce_t *db, char *buf);

#ifdef __cplusplus
}
#endif

#endif /* INC_UTILS_DEBOUNCE_H_ */
