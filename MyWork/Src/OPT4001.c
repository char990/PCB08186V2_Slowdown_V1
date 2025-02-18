/*
 * OPT4001.c
 *
 *  Created on: Oct 4, 2023
 *      Author: lq
 */
#include "OPT4001.h"
#include "MyI2C.h"

#define REG_CFG 0x0A
#define REG_ID 0x11

#define CFG_H 0x32
#define CFG_L 0x70

const uint8_t reg[3] = {REG_CFG, CFG_H, CFG_L}; // address=0x0A, 0x3270 => auto-range, 200ms, continuous mode
uint16_t OPT4001Init(I2C_HandleTypeDef *hi2c, uint8_t addr)
{
    if (addr != OPT4001_AD0 && addr != OPT4001_AD1)
    {
        return OPT4001_ERROR;
    }
    return (HAL_I2C_Master_Transmit(hi2c, addr << 1, reg, 3, 10) == HAL_OK) ? OPT4001_OK : OPT4001_ERROR;
}

uint8_t OPT4001Check(I2C_HandleTypeDef *hi2c)
{
    if (OPT4001Init(hi2c, OPT4001_AD0) == OPT4001_OK)
    {
        return OPT4001_AD0;
    }
    else
    {
        if (OPT4001Init(hi2c, OPT4001_AD1) == OPT4001_OK)
        {
            return OPT4001_AD1;
        }
        else
        {
            return OPT4001_ERROR;
        }
    }
}

uint16_t GetLux(I2C_HandleTypeDef *hi2c, uint8_t addr)
{
    if (addr != OPT4001_AD0 && addr != OPT4001_AD1)
    {
        return OPT4001_ERROR;
    }
    uint16_t id;
    if (HAL_OK == MyI2C_Read(hi2c, addr, REG_ID, &id) && id == 0x0121)
    {
        uint8_t x[22];
        for (int r = 0; r < 2; r++)
        {
            x[0] = 0;
            if (HAL_I2C_Master_Transmit(hi2c, addr << 1, x, 1, 10) != HAL_OK ||
                HAL_I2C_Master_Receive(hi2c, addr << 1, x, 22, 10) != HAL_OK)
            {
                return OPT4001_ERROR;
            }
            if (x[REG_CFG * 2] == CFG_H && x[REG_CFG * 2 + 1] == CFG_L)
            {
                uint32_t lux = 0;
                uint16_t max = 0, min = 65535;
                for (int i = 0; i < 4; i++)
                {
                    uint32_t R = ((x[i * 4] & 0x0F) * 0x10000U + x[i * 4 + 1] * 0x100U + x[i * 4 + 2]) << (x[i * 4] >> 4);
                    // lux = R*437.5E-6 ~=> ((R>>3) * 229)>>16)=436.8E-6, ~= 99.8%
                    if (R >= 149794285) // R=149794285 => lux=65535
                    {
                        R = 65535;
                    }
                    else if (R < 2287) // R=2287 => lux=1
                    {
                        R = 1;
                    }
                    else
                    {
                        R = ((R >> 3) * 229) >> 16;
                    }
                    if (R > max)
                    {
                        max = R;
                    }
                    if (R < min)
                    {
                        min = R;
                    }
                    lux += R;
                }
                lux = (lux - min - max) >> 1;
                return (lux < 1) ? 1 : lux;
            }
            else
            {
                if (OPT4001Init(hi2c, addr) == OPT4001_ERROR)
                {
                    return OPT4001_ERROR;
                }
            }
        }
    }
    return OPT4001_ERROR;
}
