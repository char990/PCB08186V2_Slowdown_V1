/*
 * CmdData.c
 *
 *  Created on: Aug 16, 2023
 *      Author: lq
 */
#include "CmdData.h"
#include "AscHex.h"
#include "MyCrc.h"
#include "Config.h"

int RxCmd(rxcmd_t *rxcmd)
{
	if (rxcmd->AnyChars())
	{
		char rxc = rxcmd->Getchar();
		if (rxc == CHAR_STX)
		{
			rxcmd->rxLen = 0;
			rxcmd->buffer[rxcmd->rxLen++] = rxc;
		}
		else
		{
			if (rxcmd->rxLen >= rxcmd->size)
			{
				rxcmd->rxLen = 0; // overflow, clear pCmd
			}
			if (rxcmd->rxLen > 0)
			{
				rxcmd->buffer[rxcmd->rxLen++] = rxc;
				if (rxc == CHAR_ETX)
				{
					if ((rxcmd->rxLen & 1) == 0 && rxcmd->rxLen > CMD_STX_CRC_ETX_BYTES)
					{
						rxcmd->rxLen = Cmd_Decode(rxcmd->buffer, rxcmd->buffer, rxcmd->rxLen);
						return rxcmd->rxLen;
					}
					rxcmd->rxLen = 0;
				}
			}
		}
	}
	return 0;
}

/* @brief   Check command crc in buf. The CRC is from <STX> to [DATA]
 * @param   buf: data buffer
 * @param   len: whole packet len, from <STX> to <ETX>
 * @retval  0: Matched; -1:Failed
 */
static int Cmd_CheckCrc(uint8_t *buf, int len)
{
#ifdef SKIP_CRC
	#warning "!!! SKIP_CRC is defined. Cmd_CheckCrc() will always return 0. !!!"
	return 0;
#endif
	len -= (CMD_STX_CRC_ETX_BYTES - 1);
	int64_t crc = Cnvt_ParseToU32((char *)buf + len);
	if (crc >= 0)
	{
		if (((uint32_t)crc) == CRC_Calculate(buf, len))
		{
			return 0;
		}
	}
	return -1;
}

int Cmd_Decode(uint8_t *src, uint8_t *dst, int len)
{
	if (Cmd_CheckCrc(src, len) == 0)
	{
		src++;
		len -= CMD_STX_CRC_ETX_BYTES;
		if (Cnvt_ParseToU8Array((char *)src, dst, len) == 0)
		{
			return len / 2;
		}
	}
	return 0;
}

int Cmd_Encode(uint8_t *src, uint8_t *dst, int len)
{
	char * pd = (char * )dst;
	pd[0] = CHAR_STX;
	Cnvt_ParseToAscArray(src, pd+1, len);
	len = len * 2 + 1;
	uint32_t crc = CRC_Calculate(dst, len);
	pd = Cnvt_ParseU32ToAsc(crc, pd + len);
	*pd++ = CHAR_ETX;
	return pd-(char *)dst;
}
