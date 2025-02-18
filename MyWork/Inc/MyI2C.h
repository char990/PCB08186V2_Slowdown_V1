/*
 * MyI2C.h
 *
 *  Created on: Oct 5, 2023
 *      Author: lq
 */

#ifndef INC_MCUDRV_MYI2C_H_
#define INC_MCUDRV_MYI2C_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "i2c.h"

/*
 * @brief   read a 16-bit register
 * @param   hi2c: i2c
 * @param   addr: address of i2c device
 * @param   reg: register address in device
 * @param   val: value saved to *val
 * @retval  HAL_StatusTypeDef
*/

HAL_StatusTypeDef MyI2C_Read(I2C_HandleTypeDef *hi2c, uint8_t addr, uint8_t reg, uint16_t * val);

void MyI2C_CheckBus(I2C_HandleTypeDef *hi2c);

#ifdef __cplusplus
}
#endif

#endif /* INC_MCUDRV_MYI2C_H_ */
