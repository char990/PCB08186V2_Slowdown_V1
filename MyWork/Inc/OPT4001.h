/*
 * OPT4001.h
 *
 *  Created on: Oct 4, 2023
 *      Author: lq
 */

#ifndef INC_MCUDRV_OPT4001_H_
#define INC_MCUDRV_OPT4001_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "i2c.h"

#define OPT4001_ERROR 0
#define OPT4001_OK 1
#define OPT4001_AD0 0x44
#define OPT4001_AD1 0x45

/*
 * @brief   Init. Set OPT4001 in auto-range, 200ms, continous mode
 * @param   addr: address of OPT4001, 0x44|0x45 depends on ADDR
 * @retval  Return 1 means success.
 *          Return 0 means hardware error.
*/
uint16_t OPT4001Init(I2C_HandleTypeDef *hi2c, uint8_t addr);

/*
 * @brief   Check OPT4001 on address 0x44 or 0x45. If exist, call Init
 * @retval  Return 0x44/0x45 means success.
 *          Return 0 means hardware error.
*/
uint8_t OPT4001Check(I2C_HandleTypeDef *hi2c);

/*
 * @brief   Get ambient temperature
 * @param   addr: address of OPT4001, 0x44|0x45 depends on ADDR
 * @retval  lux: 0 ~ 65535. If it's fully dark(lux = 0), return 1. If lux is greater than 65535, return 65535.
 *          Return 0 means hardware error.
*/
uint16_t GetLux(I2C_HandleTypeDef *hi2c, uint8_t addr);

#ifdef __cplusplus
}
#endif

#endif /* INC_MCUDRV_OPT4001_H_ */
