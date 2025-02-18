/*
 * CmdData.h
 *
 *  Created on: Aug 16, 2023
 *      Author: lq
 */

#ifndef INC_CMDDATA_H_
#define INC_CMDDATA_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdint.h"

#define CHAR_STX 0x02
#define CHAR_ETX 0x03

#define CMD_STX_CRC_ETX_BYTES 10

#define CMD_INDEX_SLVID 0
#define CMD_INDEX_CODE 1

	typedef struct
	{
		/*
		 *	@brief	function to check input
		 *	@retval	length of data
		 */
		int (*AnyChars)();
		/*
		 *	@brief	function to get a char
		 *	@retval	char
		 */
		char (*Getchar)();

		uint8_t *buffer; // rx buffer

		uint16_t size; // rx buffer size

		uint16_t rxLen;
	} rxcmd_t;

	int RxCmd(rxcmd_t *rxcmd);

	typedef struct cmd_t
	{
		/*
		 *	@brief	Command function pointer
		 *	@param	len:	data length
		 *	@retval	length of reply data
		 */
		int (*func)(int len);
		/*
		 *	@brief	Command ID / MI code
		 */
		uint8_t cmd_id;
		/*
		 *	@brief	Command length after decode. Stripped STX, CRC and ETX.
		 *	@note 0 means variable length
		 */
		uint16_t cmd_len;
	} cmd_t;

	/*
	All data except for the control characters (STX=0x02 & ETX=0x03) will be ASCII-HEX encoded using upper case characters.
	ASCII-HEX encoded values(data)will be transmitted most significant digit first (big endian).
	The packet format shall be:
	  <STX> + ID + CODE + DATA + CRC + <ETX>

	Where:
	•	STX=0x02 is start control character
	•	ID is identification number of the slave (ID of Module)
	•	CODE is the action to be carried out by the slave (Operation Code)
	•	DATA is a multi-byte packet
	•	CODE and DATA are together known as Application Packet
	•	CRC is a 4-byte cyclic redundancy check
	•	ETX=0x03 is end control character
	*/

	/*
	Use STM32 hardware CRC module with ethernet CRC-32.
	Per the standard, this computation is done using the left shifting CRC-32 (polynomial = 0x04C11DB7, initial CRC = 0xFFFFFFFF, CRC is post complemented, verify value = 0x38FB2284) algorithm.
	CRC parametrization
	Name: CRC-32 ethernet
	Width: 32 bits
	Poly: 0x04C11DB7
	Init: 0xFFFFFFFF
	Input reflected: False
	Output reflected: False
	XorOut: 0xFFFFFFFF

	E.g. <STX>0105E34ABFF2<ETX>
	<STX>		// STX
	01			// ID
	05			// MICODE
	E34ABFF2	// CRC
	<ETX>		// ETX
	CRC:
	{
		02		// STX
		30 31	// ID
		30 35	// MICDOE
	} => E34ABFF2

	E.g. <STX>0206000000000001014DF4014DF4AD1B0D9A<ETX>
	<STX>						// STX
	02							// ID
	06							// MICDOE
	000000000001014DF4014DF4	// DATA in packet
	AD1B0D9A					// CRC
	<ETX>
	CRC:
	{
		02		// STX
		30 32	// ID
		30 36	// MICDOE
		30 30 30 30 30 30 30 30 30 30 30 31 30 31 34 44 46 34 30 31 34 44 46 34	// DATA in packet
	} => AD1B0D9A
	*/

	/* @brief   Decode command
	 * @param   src: input data buffer, raw command frame: <STX> + ID + CODE + DATA + CRC + <ETX>
	 * @param   dst: output data buffer, pure command data in HEX: ID + CODE + DATA
	 * @param   len: data length of inbuf
	 * @retval  length of decoded data. 0 means failed
	 */
	int Cmd_Decode(uint8_t *src, uint8_t *dst, int len);

	/* @brief   Decode command
	 * @param   src: input data buffer, pure command data in HEX: ID + CODE + DATA
	 * @param   dst: output data buffer, raw command frame: <STX> + ID + CODE + DATA + CRC + <ETX>
	 * @param   len: data length of inbuf
	 * @retval  length of encoded data
	 */
	int Cmd_Encode(uint8_t *src, uint8_t *dst, int len);

#ifdef __cplusplus
}
#endif

#endif /* INC_CMDDATA_H_ */
