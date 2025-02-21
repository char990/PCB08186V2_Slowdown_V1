/*
 * glibc_env.c
 *
 *  Created on: Nov 21, 2023
 *      Author: lq
 */
#include "unistd.h"

#include "glibc_env.h"

const char * LOCAL = "TZ=AEST-10AEDT,M10.1.0,M4.1.0/3";
const char * UTC = "TZ=UTC";

char *glibc_env[]={
	NULL,	// TZ
	NULL	// last one
};

void GlibcEnvInit()
{
	EnvSetTZ_LOCAL();
	environ = glibc_env;
}

void EnvSetTZ_LOCAL()
{
	glibc_env[0] = (char *)LOCAL;
}

void EnvSetTZ_UTC()
{
	glibc_env[0] = (char *)UTC;
}
