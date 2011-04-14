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

#ifndef _SRECOMPOSITING_H
#define _SRECOMPOSITING_H

/*!
	\file srecompositing.h
	\brief Compositing utilities (SRE), header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined(AM_SRE)

#include "vgpaint.h"
#include "vgcontext.h"

// Patch the blend mode of the given paint descriptor, choosing a more convenient (faster) blend mode when possible.
void amSrePathBlendModePatch(AMPaintDesc *paintDesc,
							 const AMContext *context);
// Patch the blend mode of the given paint descriptor, choosing a more convenient (faster) blend mode when possible.
void amSreImageBlendModePatch(AMPaintDesc *paintDesc,
							  const AMContext *context);

#endif
#endif
