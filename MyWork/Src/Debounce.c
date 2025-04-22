/*
 * Debounce.c
 *
 *  Created on: Aug 24, 2023
 *      Author: lq
 */
#include "Debounce.h"
#include "stdio.h"

void DbnInit(debounce_t *db, uint8_t CNT, uint8_t val)
{
	db->CNT = CNT;
	db->cnt = 0;
	db->curVal = db->preVal = db->temp = val;
	DbnClrEvt(db);
}

void DbnCheck(debounce_t *db, uint8_t v)
{
	if (db->temp == v)
	{
		if (db->cnt < db->CNT)
		{
			db->cnt++;
			if (db->cnt == db->CNT)
			{
				if (v != db->curVal)
				{
					db->preVal = db->curVal;
					db->curVal = v;
					db->event |= db->preVal ^ db->curVal;
					db->rising |= db->curVal & db->event;
					db->falling |= (~db->curVal) & db->event;
				}
				else
				{
					// noise filtered, no change
				}
			}
		}
	}
	else
	{
		db->cnt = 0;
		db->temp = v;
	}
}

int IsDbnValid(debounce_t *db)
{
	return db->cnt == db->CNT;
}

void DbnClrEvt(debounce_t *db)
{
	db->event = db->rising = db->falling = 0;
}

int DbnToString(debounce_t *db, char *buf)
{
	return sprintf(buf,
				   "CNT=%u, cnt=%u, preVal=0x%02X, curVal=0x%02X, temp=0x%02X, event=0x%02X, rising=0x%02X, falling=0x%02X",
				   db->CNT, db->cnt, db->preVal, db->curVal, db->temp, db->event, db->rising, db->falling);
}
