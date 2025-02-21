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
 * @brief   read registers of i2c device
 * @param   hi2c: i2c
 * @param   addr: address of i2c device
 * @param   reg: register address in device
 * @param   len: bytes to be read: 1~255
 * @param   buf: buffer to store data
 * @retval  HAL_StatusTypeDef
*/
HAL_StatusTypeDef MyI2C_Read(I2C_HandleTypeDef *hi2c, uint8_t addr, uint8_t reg, uint8_t len, uint8_t * val);

/*
 * @brief   write data to registers of i2c device
 * @param   hi2c: i2c
 * @param   addr: address of i2c device
 * @param   len: bytes to be read: 1~255
 * @param   reg_buf: reg_buf[0] is register address in device, reg_buf[1]~reg_buf[len-1] is data
 * @retval  HAL_StatusTypeDef
*/
HAL_StatusTypeDef MyI2C_Write(I2C_HandleTypeDef *hi2c, uint8_t addr, uint8_t len, uint8_t * reg_buf);

void MyI2C_CheckBus(I2C_HandleTypeDef *hi2c);

#ifdef __cplusplus
}
#endif

#endif /* INC_MCUDRV_MYI2C_H_ */
