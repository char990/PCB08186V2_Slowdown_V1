/*
 * AscHex.c
 *
 *  Created on: Aug 16, 2023
 *      Author: lq
 */
#include "AscHex.h"
#include "ctype.h"

const char ASC[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

int IsHexStr(const uint8_t *frm, int len)
{
    if (len < 2 || len & 1)
    {
        return -1;
    }
    for (int i = 0; i < len; i++)
    {
        if (!isxdigit(*frm++))
        {
            return -2;
        }
    }
    return 0;
}

int IsText(const uint8_t *frm, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (*frm < 0x20 || *frm >= 0x7F)
        {
            return -1;
        }
        frm++;
    }
    return 0;
}

uint8_t Cnvt_Reverse(uint8_t n)
{
    static uint8_t lookup[16] = {
        0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
        0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf};
    // Reverse the top and bottom nibble then swap them.
    return (lookup[n & 0b1111] << 4) | lookup[n >> 4];
}

int Cnvt_ParseToU8(const char *p)
{
    int k = 0;
    for (int i = 0; i < 2; i++)
    {
        int j;
        if (*p >= '0' && *p <= '9') // '0' - '9'
        {
            j = (*p - '0');
        }
        else if (*p >= 'A' && *p <= 'F') // 'A' - 'F'
        {
            j = (*p - 'A' + 0x0A);
        }
        else if (*p >= 'a' && *p <= 'f') // 'a' - 'f'
        {
            j = (*p - 'a' + 0x0A);
        }
        else
        {
            return -1;
        }
        k = k * 16 + j;
        p++;
    }
    return k;
}

int Cnvt_ParseToU8Array(const char *src, uint8_t *dst, int srclen)
{
    if ((srclen & 1) == 1 || srclen <= 0)
        return -1;
    int len = srclen / 2;
    for (int i = 0; i < len; i++)
    {
        int x = Cnvt_ParseToU8(src);
        if (x < 0)
            return -1;
        *dst = x;
        dst++;
        src += 2;
    }
    return 0;
}

int Cnvt_ParseToU16(const char *src)
{
    int k = 0;
    for (int i = 0; i < 2; i++)
    {
        int x = Cnvt_ParseToU8(src);
        if (x < 0)
            return -1;
        k = k * 0x100 + x;
        src += 2;
    }
    return k;
}

int64_t Cnvt_ParseToU32(const char *src)
{
    int64_t k = 0;
    for (int i = 0; i < 4; i++)
    {
        int x = Cnvt_ParseToU8(src);
        if (x < 0)
            return -1;
        k = k * 0x100 + x;
        src += 2;
    }
    return k;
}

char *Cnvt_ParseToAsc(uint8_t h, char *p)
{
    *p++ = ASC[(h >> 4)];
    *p++ = ASC[h & 0x0F];
    return p;
}

char *Cnvt_ParseToAscArray(uint8_t *src, char *dst, int srclen)
{
    for (int i = 0; i < srclen; i++)
    {
        Cnvt_ParseToAsc(*src, dst);
        src++;
        dst += 2;
    }
    return dst;
}

char *Cnvt_ParseToStr(uint8_t *src, char *dst, int srclen)
{
    char *p = Cnvt_ParseToAscArray(src, dst, srclen);
    *p = '\0';
    return p;
}

char *Cnvt_ParseUintToAsc(uint8_t *src, char *dst, int srclen)
{
    uint8_t *p = src + srclen - 1;
    for (int i = 0; i < srclen; i++)
    {
        Cnvt_ParseToAsc(*p, dst);
        p--;
        dst += 2;
    }
    return dst;
}

char *Cnvt_ParseU16ToAsc(uint16_t h, char *p)
{
    return Cnvt_ParseUintToAsc((uint8_t *)&h, p, 2);
}

char *Cnvt_ParseU32ToAsc(uint32_t h, char *p)
{
    return Cnvt_ParseUintToAsc((uint8_t *)&h, p, 4);
}

uint16_t Cnvt_GetU16(uint8_t *p)
{
    return (*p) * 0x100 + (*(p + 1));
}

uint8_t *Cnvt_PutU16(uint16_t v, uint8_t *p)
{
    *p++ = HIGH_BYTE_U16(v);
    *p++ = LOW_BYTE_U16(v);
    return p;
}

uint32_t Cnvt_GetU32(uint8_t *p)
{
    uint32_t x = 0;
    for (int i = 0; i < 4; i++)
    {
        x *= 0x100;
        x += *p++;
    }
    return x;
}

uint8_t *Cnvt_PutU32(uint32_t v, uint8_t *p)
{
    p += 3;
    for (int i = 0; i < 4; i++)
    {
        *p-- = v & 0xFF;
        v >>= 8;
    }
    return p + 5;
}

typedef union
{
    uint8_t u8a[2];
    int16_t i16;
} si16_;

int16_t Cnvt_GetS16hl(uint8_t *p)
{
    si16_ s;
    s.u8a[1] = p[0];
    s.u8a[0] = p[1];
    return s.i16;
}

int16_t Cnvt_GetS16lh(uint8_t *p)
{
    si16_ s;
    s.u8a[0] = p[0];
    s.u8a[1] = p[1];
    return s.i16;
}

typedef union
{
    uint8_t u8a[2];
    uint16_t ui16;
} ui16_;

uint16_t Cnvt_GetU16hl(uint8_t *p)
{
    ui16_ s;
    s.u8a[1] = p[0];
    s.u8a[0] = p[1];
    return s.ui16;
}

uint16_t Cnvt_GetU16lh(uint8_t *p)
{
    ui16_ s;
    s.u8a[0] = p[0];
    s.u8a[1] = p[1];
    return s.ui16;
}
