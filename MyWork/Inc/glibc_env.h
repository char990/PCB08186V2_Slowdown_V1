/*
 * glibc_env.h
 *
 *  Created on: Nov 21, 2023
 *      Author: lq
 */

#ifndef INC_UTILS_GLIBC_ENV_H_
#define INC_UTILS_GLIBC_ENV_H_

#ifdef __cplusplus
extern "C" {
#endif

extern void GlibcEnvInit();
extern void SetTZ_LOCAL();
extern void SetTZ_UTC();

#ifdef __cplusplus
}
#endif

#endif /* INC_UTILS_GLIBC_ENV_H_ */
