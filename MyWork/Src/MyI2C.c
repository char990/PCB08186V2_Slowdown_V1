/*
 * MyI2C.c
 *
 *  Created on: Oct 5, 2023
 *      Author: lq
 */
#include "MyI2C.h"

HAL_StatusTypeDef MyI2C_Read(I2C_HandleTypeDef *hi2c, uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{
    MyI2C_CheckBus(hi2c);
    if (len > 0)
    {
        if (HAL_I2C_Master_Transmit(hi2c, addr << 1, &reg, 1, 10) == HAL_OK &&
            HAL_I2C_Master_Receive(hi2c, addr << 1, buf, len, len + 10) == HAL_OK)
        {
            return HAL_OK;
        }
    }
    return HAL_ERROR;
}

HAL_StatusTypeDef MyI2C_Write(I2C_HandleTypeDef *hi2c, uint8_t addr, uint8_t len, uint8_t * reg_buf)
{
    MyI2C_CheckBus(hi2c);
    if (len > 0)
    {
        return HAL_I2C_Master_Transmit(hi2c, addr << 1, reg_buf, len, len + 10);
    }
    return HAL_ERROR;
}

void MyI2C_CheckBus(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->ErrorCode != HAL_I2C_ERROR_NONE)
    {
        HAL_I2C_DeInit(hi2c);
        HAL_I2C_Init(hi2c);
    }
}
