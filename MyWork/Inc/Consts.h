#ifndef __CONSTS_H__
#define __CONSTS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

#define KEY_BEL 	0x07
#define KEY_BS  	0x08
#define KEY_LF	 	0x0A
#define KEY_CR	 	0x0D
#define KEY_ESC 	0x1B
#define KEY_SPACE 	0x20

extern const uint8_t BIT_MASK_8[];
extern const uint16_t BIT_MASK_16[];
extern const uint32_t BIT_MASK_32[];

#define COUNT_OF_ARRAY(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#ifdef __cplusplus
}
#endif

#endif
