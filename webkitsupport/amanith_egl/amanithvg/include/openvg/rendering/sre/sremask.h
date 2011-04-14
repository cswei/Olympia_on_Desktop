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

#ifndef _SREMASK_H
#define _SREMASK_H

/*!
	\file sremask.h
	\brief Masking and clearing (SRE), header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined(AM_SRE)

#include "vg_priv.h"
#include "vgcontext.h"

// It fills the portion of the drawing surface intersecting the specified rectangle with a constant
// color value, taken from the VG_CLEAR_COLOR parameter (SRE).
AMbool amSreDrawingSurfaceClear(AMContext *context,
								AMDrawingSurface *surface,
								AMint32 x,
								AMint32 y,
								AMint32 width,
								AMint32 height);
#if defined RIM_VG_SRC
void amSreInternalDrawingSurfaceClear(AMDrawingSurface *surface,
							          AMint32 x,
							          AMint32 y,
							          AMint32 width,
							          AMint32 height);
#endif

#endif
#endif
