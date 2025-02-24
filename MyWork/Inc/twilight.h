/*
 * twilight.h
 *
 *  Created on: Feb 24, 2025
 *      Author: lq
 */

#ifndef INC_TWILIGHT_H_
#define INC_TWILIGHT_H_

#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif

    enum TwilightStatus
    {
        TW_ST_NIGHT,
        TW_ST_DAWN,
        TW_ST_DAY,
        TW_ST_DUSK
    };
    enum TwilightStatus GetTwilightStatus(time_t t);

#ifdef __cplusplus
}
#endif

#endif /* INC_TWILIGHT_H_ */
