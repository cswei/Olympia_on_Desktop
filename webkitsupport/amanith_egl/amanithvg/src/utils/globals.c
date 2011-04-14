/****************************************************************************
** Copyright (C) 2004-2009 Mazatech S.r.l. All rights reserved.
**
** This file is part of AmanithVG software, an OpenVG implementation.
** This file is strictly confidential under the signed Mazatech Software
** Non-disclosure agreement and it's provided according to the signed
** Mazatech Software licensing agreement.
**
** Khronos and OpenVG are trademarks of The Khronos Group Inc.
** OpenGL is a registered trademark and OpenGL ES is a trademark of
** Silicon Graphics, Inc.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** For any information, please contact info@mazatech.com
**
****************************************************************************/

/*!
	\file globals.c
	\brief Global types and constants defintion, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "amanith_globals.h"

#if defined(_DEBUG)
	#if defined(AM_OS_BREW)
		#include <AEEStdLib.h>
	#elif defined(AM_OS_WIN)
		#include <Windows.h>
	#elif defined(AM_OS_WINCE)
		#include <Windows.h>
		#include <tchar.h>
	#else
		#include <stdio.h>
	#endif
#endif

const AMfloat AM_EPSILON_FLOAT = 1.192092896e-07f;

#if defined(_DEBUG)
void amDebug(const char *message) {
	#if defined(AM_OS_BREW)
	DBGPRINTF("%s\n", message);
#elif defined(AM_OS_WINCE)
	TCHAR tmsg[256];
	AMint32 i = 0;

	while (message[i] != '\0' && i < 255) {
		tmsg[i] = (TCHAR)(message[i]);
		i++;
	}
	tmsg[i] = (TCHAR)('\0');
	MessageBox(NULL, tmsg, _T("amDebug"), MB_OK | MB_ICONEXCLAMATION);
#elif defined(AM_OS_WIN)
	MessageBox(NULL, message, "amDebug", MB_OK | MB_ICONWARNING);
#else
	fprintf(stderr, "%s\n", message);
#endif
}
#endif

//! Get system endianess.
AMuint32 amSystemWordSizeGet(void) {

	// word size
	AMuint32 wordSize = 0;
	AMulong n = (AMulong)(~0);

	while (n) {
		wordSize++;
		n /= 2;
	}
	return wordSize;
}

//! Get system word size.
AMEndianessType amSystemEndianessGet(void) {

	// byte order
	AMbool be16, be32;
	short ns = 0x1234;
	AMint32 nl = 0x12345678;
	unsigned char *p = (unsigned char *)(&ns);
	AMuint32 wordSize = amSystemWordSizeGet();

	be16 = *p == 0x12;
	p = (unsigned char *)(&nl);			// 32-bit integer
	if (p[0] == 0x12 && p[1] == 0x34 && p[2] == 0x56 && p[3] == 0x78)
		be32 = AM_TRUE;
	else
	if (p[0] == 0x78 && p[1] == 0x56 && p[2] == 0x34 && p[3] == 0x12)
		be32 = AM_FALSE;
	else
		be32 = !be16;

	if ((be16 == be32) && ((wordSize == 64) || (wordSize == 32) || (wordSize == 16)))
		return (be32) ? AM_BIG_ENDIANESS : AM_LITTLE_ENDIANESS;
	return AM_LITTLE_ENDIANESS;
}

/*!
	\brief Calculate the hash value corresponding to a set of float values.
	\param values input array of float values.
	\param count number of values to consider during hash calculation.
	\return the calculated hash value.
	\pre values != NULL, count > 0.
*/
AMuint32 amHashCalculate(const AMfloat *values,
						 const AMuint32 count) {

	AMuint32 res = 0;
	AMuint32 *p = (AMuint32 *)values;
	AMuint32 i;

	AM_ASSERT(values);
	AM_ASSERT(sizeof(AMuint32) == sizeof(AMfloat));

	for (i = 0; i < count; ++i)
		res = 31 * res + p[i];
	return res % AM_HASH_INVALID;
}

#if defined (RIM_VG_SRC)
#pragma pop
#endif

