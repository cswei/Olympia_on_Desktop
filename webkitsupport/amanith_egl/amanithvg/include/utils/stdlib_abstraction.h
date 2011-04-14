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

#ifndef _STDLIB_ABSTRACTION_H
#define _STDLIB_ABSTRACTION_H

/*!
	\file stdlib_abstraction.h
	\brief Standard library abstraction layer, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/
#if !defined (TORCH_VG_SRC)
#include "kd.h"
#endif

#include "amanith_globals.h"
#if defined(AM_ARCH_ARM) || defined(AM_ARCH_THUMB)
	#include "fastmem_arm.h"
#elif (defined(AM_ARCH_I386) || defined(AM_ARCH_AMD64)) && defined(AM_CC_MSVC)
	#include <intrin.h>
#endif

#if defined(AM_OS_BREW)
	#include <AEEStdLib.h>
#else
	#include <stdlib.h>
	#include <string.h>
	#include <ctype.h>
	#include <stdio.h>
#endif

#if !defined(_MAX_PATH)
	// max length of full pathname
	#define _MAX_PATH   260
#endif
#if !defined(_MAX_DRIVE)
	// max length of drive component
	#define _MAX_DRIVE  3
#endif
#if !defined(_MAX_DIR)
	// max length of path component
	#define _MAX_DIR    256
#endif
#if !defined(_MAX_FNAME)
	// max length of file name component
	#define _MAX_FNAME  256
#endif
#if !defined(_MAX_EXT)
	// max length of extension component
	#define _MAX_EXT    256
#endif

#if defined (RIM_VG_SRC)
extern void RasterMemsetC32(DWORD * dst,DWORD colour,DWORD count);
#endif

#define IS_LOWER(_c) \
	((sizeof(_c) == sizeof(char)) \
	 ? (((unsigned char)((_c) - 'a')) < 26) \
	 : (((unsigned int)((_c) - 'a')) < 26))

#define IS_UPPER(_c) \
	((sizeof(_c) == sizeof(char)) \
	 ? (((unsigned char)((_c) - 'A')) < 26) \
	 : (((unsigned int)((_c) - 'A')) < 26))

#define TO_LOWER(_c) (IS_UPPER(_c) ? ((_c) | 0x20) : (_c))
#define TO_UPPER(_c) (IS_LOWER(_c) ? ((_c) ^ 0x20) : (_c))

//! Converts a character to lower case.
AM_INLINE AMint32 amTolower(const AMint32 c) {

	return TO_LOWER(c);
}
//! Converts a character to upper case.
AM_INLINE AMint32 amToupper(const AMint32 c) {

	return TO_UPPER(c);
}
//! Tests whether an element in a locale is a whitespace character.
AM_INLINE AMint32 amIsspace(const AMint32 c) {

	return ((c >= 9 && c < 15) || (c == 32)) ? 1 : 0;
}
// Convert a string to a double-precision value.
AMdouble amStrtod(const char *nptr,
				  char **endptr);
// Convert a string to integer.
AMint32 amAtoi(const char *str);
//! Convert a string to double.
AM_INLINE AMdouble amAtof(const char *str) {

	return amStrtod(str, (char **)NULL);
}
// Quick sort (descending order) of unsigned 32bit integers.
void amUint32QSort(AMuint32 *base,
				   const AMuint32 num);

#if defined(AM_EXT_LIBC)
	void *amanithvg_malloc(size_t size);
	void *amanithvg_realloc(void *memblock,	size_t size);
	void amanithvg_free(void *memblock);
	void *amanithvg_memset(void *dest, int c, size_t count);
	void *amanithvg_memcpy(void *dest, const void *src, size_t count);
	char *amanithvg_strchr(const char *str, int c);
	size_t amanithvg_strlen(const char *str);
	char *amanithvg_strstr(const char *str, const char *strSearch);
	void amanithvg_memset32(unsigned int *dest, unsigned int c, size_t count);

	#define amMalloc	amanithvg_malloc
	#define amRealloc	amanithvg_realloc
	#define amFree		amanithvg_free
	#define amMemset	amanithvg_memset
	#define amMemcpy	amanithvg_memcpy
	#define amStrchr	amanithvg_strchr
	#define amStrlen	amanithvg_strlen
	#define amStrstr	amanithvg_strstr
	#define amMemset32	amanithvg_memset32
	#define AM_MEMORY_LOG(_msg) ((void)0)
#else
#if defined(AM_OS_BREW)
	AM_INLINE void *amMalloc(size_t size) {
		// this avoid the "conversion from size_t to uint32" warning
		return MALLOC((uint32)size);
	}
	AM_INLINE void *amRealloc(void* memblock, size_t size) {
		// this avoid the "conversion from size_t to uint32" warning
		return REALLOC(memblock, (uint32)size);
	}
	#define amFree		FREE
	#define amSprintf	SPRINTF
	#define amStrcat	STRCAT
	#define amStrcpy	STRCPY
	#define amStrlen	STRLEN
	#define amStrcmp	STRCMP
	#define amStrncmp	STRNCMP
	#define amStrncpy	STRNCPY
	#define amStrstr	STRSTR
	#define amStrchr	STRCHR
	#define amMemset	MEMSET
	#define amMemcpy	MEMCPY
	#define AM_MEMORY_LOG(_msg) ((void)0)
#else
#if defined(RIM_VG_SRC)
	#define amMalloc    kdMalloc
	#define amRealloc   kdRealloc
	#define amFree      kdFree
    #define AM_MEMORY_LOG(_msg) kdLogMessage(_msg)
#elif defined(AM_DEBUG_MEMORY) && defined(AM_STANDALONE) && (defined(AM_OS_WIN) || defined(AM_OS_WINCE) || defined(AM_OS_LINUX))
	void *amMalloc(size_t size);
	void *amRealloc(void *memblock, size_t size);
	void amFree(void *memblock);
	void amMemoryLog(const AMchar8 *msg);
	#define AM_MEMORY_LOG(_msg) amMemoryLog(_msg)
#else
	#define amMalloc	malloc
	#define amRealloc	realloc
	#define amFree		free
	#define AM_MEMORY_LOG(_msg) ((void)0)
#endif
	#define amSprintf	sprintf
	#define amStrcat	strcat
	#define amStrcpy	strcpy
	#define amStrlen	strlen
	#define amStrcmp	strcmp
	#define amStrncmp	strncmp
	#define amStrncpy	strncpy
	#define amStrstr	strstr
	#define amStrchr	strchr
	#define amMemset	memset
	#define amMemcpy	memcpy
#endif // AM_OS_BREW
#endif // AM_EXT_LIBC

#if !defined(AM_EXT_LIBC)
/*!
	\brief Fill a buffer with a specified 32bit value.
	\param dest destination buffer.
	\param data input 32bit value.
	\param count number of values to write.
*/
AM_INLINE void amMemset32(AMuint32 *dest,
						  AMuint32 data,
						  size_t count) {

#if !defined (RIM_VG_SRC)
	AM_ASSERT(amPointerIsAligned(dest, 4));

#if defined(AM_CC_MSVC)
	#if defined(AM_ARCH_I386) || defined(AM_ARCH_AMD64)
		__stosd((unsigned long *)dest, data, count);
	#else
		// ARM, ARM Thumb
		size_t i;
		for (i = count; i != 0; --i)
			*dest++ = data;
	#endif
#elif defined(AM_CC_GCC)
	#if defined(AM_ARCH_I386) || defined(AM_ARCH_AMD64)
		__asm__ __volatile__ ("rep stosl"
							: "=D" (dest), "=c" (count)
							: "0" (dest), "1" (count), "a" (data)
							: "memory");
	#elif defined(AM_ARCH_ARM) || defined(AM_ARCH_THUMB)
		amMemset32_ARM9(dest, data, count);
	#else
		// PowerPC
		size_t i;
		for (i = count; i != 0; --i)
			*dest++ = data;
	#endif
#elif defined(AM_CC_ARMCC)
	#if defined(AM_ARCH_ARM) || defined(AM_ARCH_THUMB)
		amMemset32_ARM9(dest, data, count);
	#endif
#else
	size_t i;
	for (i = count; i != 0; --i)
		*dest++ = data;
#endif
#else
    RasterMemsetC32((DWORD*)dest, (DWORD)data, (DWORD)count);
#endif
}
#endif // AM_EXT_LIBC

AM_INLINE void amMemset16(AMuint16 *dest,
						  AMuint16 data,
						  size_t count) {

	if (count > 0) {

		AMuint32 data32 = (((AMuint32)data) << 16) | data;

		if (!amPointerIsAligned(dest, 4)) {
			*dest++ = data;
			count--;
		}

		amMemset32((AMuint32 *)dest, data32, count >> 1);

		if (count & 1)
			dest[count - 1] = data;
	}
}

#endif
