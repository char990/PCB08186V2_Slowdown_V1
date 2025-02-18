/*
 * AscHex.h
 *
 *  Created on: Aug 16, 2023
 *      Author: lq
 */

#ifndef INC_UTILS_ASCHEX_H_
#define INC_UTILS_ASCHEX_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdint.h"

#define HIGH_BYTE_U16(v) ((uint8_t )(v>>8))
#define LOW_BYTE_U16(v) ((uint8_t)v)

#define CNVT_SWAP_U16(v) ((v >> 8) | (v << 8))


    extern const char ASC[16];

    /* @brief   Check if input is a legal Hex string
     * @param   frm, input Hex string
     * @param   len, length of input Hex string
     * @retval  0:success
     */
    int IsHexStr(const uint8_t *frm, int len);

    /* @brief   Check if input is text(0x20-0x7F)
     * @param   frm, input Hex string
     * @param   len, length of input Hex string
     * @retval  0:success
     */
    int IsText(const uint8_t *frm, int len);

    /// \brief  Reverse a uint8_t Example: 0x1A => 0x85
    uint8_t Cnvt_Reverse(uint8_t n);

    /// \brief  parse 2 Asc to 1 uint8_t Hex. Example: "1F" => 0x1F
    /// \return     int: 0-255:success, -1:failed, there is invalid chars
    int Cnvt_ParseToU8(const char *p);

    /// \brief  parse 2 Asc to 1 Hex. Example: "1F....." => 0x1F......
    /// \param      src : ascii buffer
    /// \param      dst : hex buffer
    /// \param      srclen : ascii len ( = hex_len *2)
    /// \return     int: 0:success, -1:failed, there is invalid chars
    int Cnvt_ParseToU8Array(const char *src, uint8_t *dst, int srclen);

    /// \brief  parse Asc to uint16_t hex. Example: "1F0A" => 0x1F0A
    /// \param      src : ascii buffer
    /// \return     int: >0:success, -1:failed, there is invalid chars
    int Cnvt_ParseToU16(const char *src);

    /// \brief  parse Asc to uint32_t hex. Example: "1F0A3456" => 0x1F0A3456
    /// \param      src : ascii buffer
    /// \return     int64_t: >0:success, -1:failed, there is invalid chars
    int64_t Cnvt_ParseToU32(const char *src);

    /// \brief  parse uint8_t to 2 Asc. Example: 0x1F => "1F"
    /// \return     next byte of dst
    char *Cnvt_ParseToAsc(uint8_t h, char *dst);

    /// \brief  parse uint8_t array to 2 Asc no '\0' attached. Example: 0x1F 0x2A 0x3E ... => "1F2A3E......"
    /// \param      src : hex buffer
    /// \param      dst : ascii buffer
    /// \param      srclen : hex len ( = asc_len / 2)
    /// \return     next byte of dst
    char *Cnvt_ParseToAscArray(uint8_t *src, char *dst, int srclen);

    /// \brief  parse uint8_t array to 2 Asc with '\0' attached. Example: 0x1F 0x2A 0x3E ... => "1F2A3E......\0"
    /// \param      src : hex buffer
    /// \param      dst : ascii buffer
    /// \param      srclen : hex len ( = asc_len / 2)
    /// \return     next byte of dst
    char *ParseToStr(uint8_t *src, char *dst, int srclen);

    /// \brief  parse uint at src to Asc. For ParseU16ToAsc/ParseU32ToAsc
    /// \param      src : hex buffer
    /// \param      dst : ascii buffer
    /// \param      srclen : hex len ( = asc_len / 2)
    /// \return     next byte of dst
    char *Cnvt_ParseUintToAsc(uint8_t *src, char *dst, int srclen);

    /// \brief  parse uint16_t to 4 Asc. Example: 0x1F09 => "1F09"
    /// \return     next byte of dst
    char *Cnvt_ParseU16ToAsc(uint16_t h, char *dst);

    /// \brief  parse uint32_t to 8 Asc. Example: 0x1F09342E => "1F09342E"
    /// \return     next byte of dst
    char *Cnvt_ParseU32ToAsc(uint32_t h, char *dst);

    /// \brief Convert 2 bytes uint8_t to uint16_t
    uint16_t Cnvt_GetU16(uint8_t *p);

    /// \brief Put uint16_t to uint8_t *
    uint8_t *Cnvt_PutU16(uint16_t v, uint8_t *p);

    /// \brief Convert 4 bytes uint8_t to uint32_t
    uint32_t Cnvt_GetU32(uint8_t *p);

    /// \brief Put uint32_t to uint8_t *
    uint8_t *Cnvt_PutU32(uint32_t v, uint8_t *p);

    /// \brief  Get int16_t from uint8_t array, [0]=high byte,[1]=low byte
    int16_t Cnvt_GetS16hl(uint8_t *p);
    /// \brief  Get int16_t from uint8_t array, [0]=low byte,[1]=high byte
    int16_t Cnvt_GetS16lh(uint8_t *p);

    /// \brief  Get uint16_t from uint8_t array, [0]=high byte,[1]=low byte
    uint16_t Cnvt_GetU16hl(uint8_t *p);
    /// \brief  Get uint16_t from uint8_t array, [0]=low byte,[1]=high byte
    uint16_t Cnvt_GetU16lh(uint8_t *p);

#ifdef __cplusplus
}
#endif

#endif /* INC_UTILS_ASCHEX_H_ */
