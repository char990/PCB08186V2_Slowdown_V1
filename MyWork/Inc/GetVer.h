/*
 * Version.h
 *
 *  Created on: Sep 12, 2023
 *      Author: lq
 */

#ifndef INC_GETVER_H_
#define INC_GETVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

    extern const uint32_t FW_BUILD;
    extern const uint16_t FW_VERSION;

    /* @brief   output version & build to buf
     * @param   buf, allocation size should be larger than 36 bytes
     * @retval  bytes of sprintf, max=35, not including the terminating NULL
     */
    int GetVer(char *buf);



#ifdef __cplusplus
}
#endif

#endif /* INC_GETVER_H_ */
