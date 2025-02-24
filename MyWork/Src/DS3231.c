/*
 * DS3231.c
 *
 *  Created on: Feb 21, 2025
 *      Author: lq
 */
#include "DS3231.h"
#include "MyI2C.h"
#include "MyTmr.h"
#include "AscHex.h"
#include "glibc_env.h"

#define REG_CONTROL_VALUE 0x26  // START temperature conversion
#define REG_STATUS_VALUE 0x00   // clear status, disalbe 32KHz output

static int DS3231ReadRegs(I2C_HandleTypeDef *ds3231_i2c, int reg_addr, uint8_t *buf, int len)
{
    return (MyI2C_Read(ds3231_i2c, DS3231_ADDR, reg_addr, len, buf) == HAL_OK) ? DS3231_OK : DS3231_NG;
}

static int DS3231WriteRegs(I2C_HandleTypeDef *ds3231_i2c, int reg_addr, const uint8_t *buf, int len)
{
    uint8_t reg_buf[DS3231_BYTES + 1];
    reg_buf[0] = reg_addr;
    for (int i = 0; i < len; i++)
    {
        reg_buf[i + 1] = buf[i];
    }
    return (MyI2C_Write(ds3231_i2c, DS3231_ADDR, len + 1, reg_buf) == HAL_OK) ? DS3231_OK : DS3231_NG;
}

static int DS3231GetControl(I2C_HandleTypeDef *ds3231_i2c, char *v)
{
    uint8_t pr;
    int result;
    result = DS3231ReadRegs(ds3231_i2c, DS3231_REG_CONTROL, &pr, 1);
    if (result < 1)
    {
        return -1;
    }
    *v = pr;
    return 0;
}

static int DS3231SetControl(I2C_HandleTypeDef *ds3231_i2c, char v)
{
    return DS3231WriteRegs(ds3231_i2c, DS3231_REG_CONTROL, (const uint8_t *)&v, 1);
}

static int DS3231GetStatus(I2C_HandleTypeDef *ds3231_i2c, char *v)
{
    uint8_t pr;
    int result;
    result = ReadRegs(ds3231_i2c, DS3231_REG_STATUS, &pr, 1);
    if (result < 1)
    {
        return -1;
    }
    *v = pr;
    return 0;
}

static int DS3231SetStatus(I2C_HandleTypeDef *ds3231_i2c, char v)
{
    return DS3231WriteRegs(ds3231_i2c, DS3231_REG_STATUS, (const uint8_t *)&v, 1);
}

static int DS3231GetUtcTime(I2C_HandleTypeDef *ds3231_i2c, struct tm *utctm)
{
    uint8_t reg[7];
    int result;
    result = DS3231ReadRegs(ds3231_i2c, DS3231_REG_SEC, reg, 7);
    if (result == DS3231_OK)
    {
        utctm->tm_isdst = -1;
        utctm->tm_sec = bcd2hex(reg[0]);
        utctm->tm_min = bcd2hex(reg[1]);
        if (reg[2] & 0x40)
        { // 12hours
            utctm->tm_hour = bcd2hex(reg[2] & 0x1F);
            if ((reg[2] & 0x20) == 0)
            { // am
                if (utctm->tm_hour == 12)
                {
                    utctm->tm_hour = 0; // 12am = 0
                }
            }
            else
            { // pm
                if (utctm->tm_hour < 12)
                {
                    utctm->tm_hour += 12; // 1-11pm = 13-23
                }
            }
        }
        else
        {
            utctm->tm_hour = bcd2hex(reg[2]);
        }
        utctm->tm_wday = bcd2hex(reg[3]) - 1;
        utctm->tm_mday = bcd2hex(reg[4]);
        utctm->tm_mon = bcd2hex(reg[5] & 0x1F) - 1;
        utctm->tm_year = bcd2hex(reg[6]) + ((reg[5] & 0x80) ? 100 : 0);
    }
    return result;
}

static int DS3231SetRtcRegs(I2C_HandleTypeDef *ds3231_i2c, char *rtc)
{
    uint8_t reg[7];
    for (int i = 0; i < 7; i++)
    {
        reg[i] = rtc[i];
    }
    return DS3231WriteRegs(ds3231_i2c, DS3231_REG_SEC, reg, 7);
}

int DS3231Init(I2C_HandleTypeDef *ds3231_i2c)
{
    uint8_t buf[2];
    buf[0] = REG_CONTROL_VALUE;
    buf[1] = REG_STATUS_VALUE;
    return DS3231WriteRegs(ds3231_i2c, DS3231_REG_CONTROL, buf, 2);
}

/*
time_t => struct tm:
    struct tm * localtime_r (const time t *time, struct tm *resultp)
    struct tm * gmtime_r (const time t *time, struct tm *resultp)
struct tm => time_t:
    SetEnvTZ_UTC(); // make UTC time
    time_t mktime (struct tm *brokentime)
    SetEnvTZ_LOCAL();
*/

time_t DS3231GetTime(I2C_HandleTypeDef *ds3231_i2c)
{
    struct tm utc;
    if (DS3231GetUtcTime(ds3231_i2c, &utc) != DS3231_OK)
    {
        return -1;
    }
    SetTZ_UTC(); // make UTC time
    time_t t = mktime(&utc);
    SetTZ_LOCAL();
    return t;
}

int DS3231SetTime(I2C_HandleTypeDef *ds3231_i2c, time_t t)
{
    struct tm utc;
    if (gmtime_r(&t, &utc) != &utc)
    {
        return -1;
    }
    uint8_t reg[7];
    reg[0] = hex2bcd(utc.tm_sec);
    reg[1] = hex2bcd(utc.tm_min);
    reg[2] = hex2bcd(utc.tm_hour);
    reg[3] = hex2bcd(utc.tm_wday + 1);
    reg[4] = hex2bcd(utc.tm_mday);
    if (utc.tm_year >= 100)
    {
        reg[5] = hex2bcd(utc.tm_mon + 1) | 0x80;
        reg[6] = hex2bcd(utc.tm_year - 100);
    }
    else
    {
        reg[5] = hex2bcd(utc.tm_mon + 1);
        reg[6] = hex2bcd(utc.tm_year);
    }
    return DS3231WriteRegs(ds3231_i2c, DS3231_REG_SEC, reg, 7);
}

time_t DS3231GetLocalTime(I2C_HandleTypeDef *ds3231_i2c, struct tm *localtm)
{
    time_t t = DS3231GetTime(ds3231_i2c);
    if (t >= 0)
    {
        localtime_r(&t, localtm);
    }
    return t;
}

time_t DS3231SetLocalTime(I2C_HandleTypeDef *ds3231_i2c, struct tm *localtm)
{
    time_t t = mktime(localtm);
    if (t < 0)
    {
        return t;
    }
    return DS3231SetTime(ds3231_i2c, t);
}

int DS3231StartTemp(I2C_HandleTypeDef *ds3231_i2c)
{
    return DS3231SetControl(ds3231_i2c, REG_CONTROL_VALUE);
}

int DS3231GetTemp(I2C_HandleTypeDef *ds3231_i2c, int *t)
{
    uint8_t tt[2];
    int result;
    result = DS3231ReadRegs(ds3231_i2c, DS3231_REG_TEMP, tt, 2);
    if (result < 1 || (tt[1] & 0x3F) != 0)
    {
        return -1;
    }
    *t = (tt[0] & 0x80) ? 0 : tt[0];
    return 0;
}
