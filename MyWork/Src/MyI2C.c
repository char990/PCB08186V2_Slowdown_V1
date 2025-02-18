/*
 * MyI2C.c
 *
 *  Created on: Oct 5, 2023
 *      Author: lq
 */
#include "MyI2C.h"

HAL_StatusTypeDef MyI2C_Read(I2C_HandleTypeDef *hi2c, uint8_t addr, uint8_t reg, uint16_t *val)
{
    uint8_t x[2];
    if (HAL_I2C_Master_Transmit(hi2c, addr << 1, &reg, 1, 5) == HAL_OK &&
        HAL_I2C_Master_Receive(hi2c, addr << 1, x, 2, 5) == HAL_OK)
    {
        *val = x[0] * 0x100 + x[1];
        return HAL_OK;
    }
    else
    {
        return HAL_ERROR;
    }
}

void MyI2C_CheckBus(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->ErrorCode != HAL_I2C_ERROR_NONE)
    {
        HAL_I2C_DeInit(hi2c);
        HAL_I2C_Init(hi2c);
    }
}
