/*
 * DS3231.h
 *
 *  Created on: Feb 21, 2025
 *      Author: lq
 */

#ifndef INC_DS3231_H_
#define INC_DS3231_H_

#include "i2c.h"
#include "time.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define DS3231_ADDR 0x68

typedef enum DS3231_ST_t
{
    DS3231_NG = 0,
    DS3231_OK = DS3231_ADDR
} DS3231_ST_t;


#define DS3231_BYTES 0x13

#define DS3231_REG_SEC 0x00
#define DS3231_REG_MIN 0x01
#define DS3231_REG_HOUR 0x02
#define DS3231_REG_DAY 0x03
#define DS3231_REG_DATE 0x04
#define DS3231_REG_MONTH 0x05
#define DS3231_REG_YEAR 0x06
#define DS3231_REG_A1SEC 0x07
#define DS3231_REG_A1MIN 0x08
#define DS3231_REG_A1HOUR 0x09
#define DS3231_REG_A1DAY 0x0A
#define DS3231_REG_A2MIN 0x0B
#define DS3231_REG_A2HOUR 0x0C
#define DS3231_REG_A2DAY 0x0D
#define DS3231_REG_CONTROL 0x0E
#define DS3231_REG_STATUS 0x0F
#define DS3231_REG_AGING 0x10
#define DS3231_REG_TEMP 0x11

    /*
     * @brief   Init. Set controll & status registers
     * @param   hi2c: I2C_HandleTypeDef
     * @retval  Return DS3231_OK/NG
     */
    DS3231_ST_t DS3231Init(I2C_HandleTypeDef *hi2c);

    /*
     * @brief   Get timestamp from DS3231
     * @param   hi2c: I2C_HandleTypeDef
     * @retval  Return timestamp. -1: Error
     */
    time_t DS3231GetTime(I2C_HandleTypeDef *ds3231_i2c);

    /*
     * @brief   Set timestamp to DS3231
     * @param   hi2c: I2C_HandleTypeDef
     * @retval  Return timestamp. -1: Error
     */
    time_t DS3231SetTime(I2C_HandleTypeDef *ds3231_i2c, time_t t);

    /*
     * @brief   Get localtime from DS3231
     * @param   hi2c: I2C_HandleTypeDef
     * @param   localtm: output localtime in brokentime format
     * @retval  Return timestamp. -1: Error
     */
    time_t DS3231GetLocalTime(I2C_HandleTypeDef *ds3231_i2c, struct tm *localtm);

    /*
     * @brief   Set localtime to DS3231
     * @param   hi2c: I2C_HandleTypeDef
     * @param   localtm: localtime in brokentime format
     * @retval  Return timestamp. -1: Error
     */
    time_t DS3231SetLocalTime(I2C_HandleTypeDef *ds3231_i2c, struct tm *localtm);

    /*
     * @brief   Start temperature convertion in DS3231
     * @param   hi2c: I2C_HandleTypeDef
     * @retval  Return DS3231_OK/NG
     */
    DS3231_ST_t DS3231StartTemp(I2C_HandleTypeDef *ds3231_i2c);

    /*
     * @brief   Read temperature in DS3231
     * @param   hi2c: I2C_HandleTypeDef
     * @param   t: output temperature ('C)
     * @retval  Return DS3231_OK/NG
     */
    DS3231_ST_t DS3231GetTemp(I2C_HandleTypeDef *ds3231_i2c, int *t);

#ifdef __cplusplus
}
#endif

#endif /* INC_DS3231_H_ */
