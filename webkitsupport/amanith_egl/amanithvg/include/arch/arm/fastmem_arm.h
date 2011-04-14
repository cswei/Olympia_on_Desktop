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

#ifndef _FASTMEM_H
#define _FASTMEM_H

/*!
	\file fastmem.h
	\brief Fast memory routines for ARM processors, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "amanith_globals.h"

// for ARM architecture, compiler different than VisualC

#if (defined(AM_ARCH_ARM) || defined(AM_ARCH_THUMB)) && (defined(AM_CC_GCC) || defined(AM_CC_ARMCC)) && !defined(AM_EXT_LIBC)
void amMemset32_ARM9(unsigned int *_dest,
					 unsigned int _data,
					 unsigned int _count);
#endif

#endif
