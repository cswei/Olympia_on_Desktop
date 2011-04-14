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

#ifndef _BREWCONTEXT_H
#define _BREWCONTEXT_H

/*!
	\file brewcontext.h
	\brief BREW specific context related functions, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined(__cplusplus)
extern "C" {
#endif

// Get the current context.
void *amBrewCurrentContextGet(void);
// Destroy the current context.
void amBrewCurrentContextDestroy(void);

#if defined(__cplusplus)
}
#endif

#endif
