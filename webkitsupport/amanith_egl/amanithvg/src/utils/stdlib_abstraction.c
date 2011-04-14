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
	\file stdlib_abstraction.c
	\brief Standard library abstraction layer, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "stdlib_abstraction.h"
#if defined(AM_DEBUG_MEMORY) && defined(AM_STANDALONE) && (defined(AM_OS_WIN) || defined(AM_OS_WINCE) || defined(AM_OS_LINUX))
	#include <malloc.h> // for _msize/malloc_usable_size functions
	#include "vgcontext.h"
	#include "vg_priv.h"
#endif

/*!
	\brief Convert a string to a double-precision value.
	\param nptr null-terminated string to convert.
	\param endptr pointer to character that stops scan.
	\return the value corresponding to the given string.
*/
AMdouble amStrtod(const char *nptr,
				  char **endptr) {

	const char *p = nptr;
	long AMdouble value = 0.0;
	AMint32 sign  = 1;
	long AMdouble factor;
	AMuint32 expo;

	while (amIsspace(*p))
		p++;

	if (*p == '-') {
		sign = -1;
		p++;
	}
	else
	if (*p == '+')
		p++;

	while ((AMuint32)(*p - '0') < 10u)
		value = value * 10 + (*p++ - '0');

	if (*p == '.') {
		factor = 1.0;
		p++;
		while ((AMuint32)(*p - '0') < 10u) {
			factor *= 0.1;
			value += (*p++ - '0') * factor;
		}
	}

	if ((*p | 32) == 'e') {
		expo = 0;
		factor = 10.0;

		p++;
		if (*p == '-') {
			factor = 0.1;
			p++;
		}
		else
		if (*p == '+')
			p++;
		else
		if (*p < '0' || *p > '9') {
			value = 0.0;
			p = nptr;
			goto done;
		}

		while ((AMuint32)(*p - '0') < 10u)
			expo = 10 * expo + (*p++ - '0');

		for (;;) {
			if (expo & 1)
				value *= factor;
			if ((expo >>= 1) == 0)
				break;
			factor *= factor;
		}
	}

done:
	if (endptr != NULL)
		*endptr = (char *)p;
	return value * sign;
}

/*!
	\brief Convert a string to integer.
	\param str string to be converted.
	\return integer value corresponding to the given string.
*/
AMint32 amAtoi(const char *str) {

	AMint32 result = 0;
	AMint32 sign = 1;
	char *p = (char *)str;

	while (amIsspace(*p))
		p++;

	if (*p == '-') {
		sign = -1;
		p++;
	}
	else
	if (*p == '+')
		p++;

	while ((*p >= '0') && (*p <= '9')) {
		result *= 10;
		result += *p - '0';
		p++;
	}

	return (sign == -1) ? -result : result;
}

/*!
	\brief Quick sort (descending order) of unsigned 32bit integers.
	\param base pointer to the first element to sort.
	\param num number of elements to sort.
*/
void amUint32QSort(AMuint32 *base,
				   const AMuint32 num) {

	#define TOO_SMALL_FOR_QSORT 8
	#define STACK_SIZE (8 * sizeof(void *) - 2)
	#define SWAP_ELEMENT(_a, _b) \
		swapTmp = *(_a); \
		*(_a) = *(_b); \
		*(_b) = swapTmp;

	AMuint32 *lo = base;
	AMuint32 *hi = base + (num - 1);
	AMuint32 *mid, *loElement, *hiElement, *loStack[STACK_SIZE], *hiStack[STACK_SIZE], swapTmp;
	AMuint32 size;
	AMint32 stackPtr = 0;

	if (num < 2)
		return;

recurse:
	size = (AMuint32)(hi - lo) + 1;

	if (size <= TOO_SMALL_FOR_QSORT) {
		AMuint32 *p, *max;
		while (hi > lo) {
			max = lo;
			for (p = lo + 1; p <= hi; ++p) {
				if (*p < *max)
					max = p;
			}
			SWAP_ELEMENT(max, hi)
			hi--;
		}
	}
	else {
		mid = lo + (size >> 1);

		if (*lo < *mid) {
			SWAP_ELEMENT(lo, mid)
		}
		if (*lo < *hi) {
			SWAP_ELEMENT(lo, hi)
		}
		if (*mid < *hi) {
			SWAP_ELEMENT(mid, hi)
		}

		loElement = lo;
		hiElement = hi;

		for (;;) {
			if (mid > loElement) {
				do  {
					loElement++;
				} while (loElement < mid && *loElement >= *mid);
			}
			if (mid <= loElement) {
				do  {
					loElement++;
				} while (loElement <= hi && *loElement >= *mid);
			}
			do  {
				hiElement--;
			} while (hiElement > mid && *hiElement < *mid);

			if (hiElement < loElement)
				break;

			SWAP_ELEMENT(loElement, hiElement)

			if (mid == hiElement)
				mid = loElement;
		}

		hiElement++;
		if (mid < hiElement)
			hiElement--;

		if (mid >= hiElement)
			hiElement--;

		if (hiElement - lo >= hi - loElement) {
			if (lo < hiElement) {
				loStack[stackPtr] = lo;
				hiStack[stackPtr] = hiElement;
				++stackPtr;
			}
			if (loElement < hi) {
				lo = loElement;
				goto recurse;
			}
		}
		else {
			if (loElement < hi) {
				loStack[stackPtr] = loElement;
				hiStack[stackPtr] = hi;
				++stackPtr;
			}
			if (lo < hiElement) {
				hi = hiElement;
				goto recurse;
			}
		}
	}
	--stackPtr;
	if (stackPtr >= 0) {
		lo = loStack[stackPtr];
		hi = hiStack[stackPtr];
		goto recurse;
	}

	#undef TOO_SMALL_FOR_QSORT
	#undef STACK_SIZE
	#undef SWAP_ELEMENT
}

#if defined(AM_DEBUG_MEMORY) && defined(AM_STANDALONE) && (defined(AM_OS_WIN) || defined(AM_OS_WINCE) || defined(AM_OS_LINUX))

	#if defined(AM_OS_LINUX)
		#define MEMORY_SIZE(_ptr) (malloc_usable_size(_ptr))
	#else
		#define MEMORY_SIZE(_ptr) (_msize(_ptr))
	#endif

	void *amMalloc(size_t size) {

		AMContext *currentContext;
		AMDrawingSurface *currentSurface;
		void *p;

		amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
		
		if (currentContext && currentContext->allocatedMemory + size > AM_DEBUG_MEMORY)
			return NULL;

		p = malloc(size);
		if (p && currentContext) {

			char msg[255];

			currentContext->allocatedMemory += (AMuint32)MEMORY_SIZE(p);
			sprintf(msg, "M (%7d): %7d", size, currentContext->allocatedMemory);
			AM_MEMORY_LOG(msg);
		}
		return p;
	}
	
	void *amRealloc(void *memblock, size_t size) {

		AMContext *currentContext;
		AMDrawingSurface *currentSurface;
		void *p;
		size_t oldSize = (memblock)? MEMORY_SIZE(memblock) : 0;

		amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);

		if (currentContext && currentContext->allocatedMemory + size - oldSize > AM_DEBUG_MEMORY)
			return NULL;

		p = realloc(memblock, size);
		if (p && currentContext) {

			char msg[255];

			currentContext->allocatedMemory += (AMuint32)MEMORY_SIZE(p) - (AMuint32)oldSize;
			sprintf(msg, "R (%7d): %7d", (int)size - (int)oldSize, currentContext->allocatedMemory);
			AM_MEMORY_LOG(msg);
		}
		return p;
	}

	void amFree(void *memblock) {

		AMContext *currentContext;
		AMDrawingSurface *currentSurface;
		size_t curSize = MEMORY_SIZE(memblock);

		amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
		if (currentContext) {

			char msg[255];

			currentContext->allocatedMemory -= (AMuint32)curSize;
			sprintf(msg, "F (%7d): %7d", curSize, currentContext->allocatedMemory);
			AM_MEMORY_LOG(msg);
		}
		free(memblock);
	}

	void amMemoryLog(const AMchar8 *msg) {

		AMContext *currentContext;
		AMDrawingSurface *currentSurface;

		amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
		if (currentContext && currentContext->memoryLog) {
			//fprintf(currentContext->memoryLog, "%s\n", msg);
			//fflush(currentContext->memoryLog);
		}
	}

#endif
