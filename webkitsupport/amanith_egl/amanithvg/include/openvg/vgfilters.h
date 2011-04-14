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

#ifndef _VGFILTERS_H
#define _VGFILTERS_H

/*!
	\file vgfilters.h
	\brief Image filters, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "amanith_globals.h"

#if !defined(AM_LITE_PROFILE)

/*!
	\brief Type definition of a function that converts a single pixel, from one of these 32bit
	formats (sRGBA, sRGBApre, lRGBA, lRGBApre) to another one (every VGImageFormat enumeration values).
*/
typedef void (*AMPixel32ConverterFunction)(void *dstPixel,
										   AMuint32 r,
										   AMuint32 g,
										   AMuint32 b,
										   AMuint32 a,
										   const AMuint32 x,
										   const VGbitfield colMask);
// Initialize pixel conversion table.
void amPxlConversionTableInit(void *_context);

#endif

#endif
