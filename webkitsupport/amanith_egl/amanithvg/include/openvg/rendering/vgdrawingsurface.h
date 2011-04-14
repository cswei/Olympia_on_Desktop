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

#ifndef _VGDRAWINGSURFACE_H
#define _VGDRAWINGSURFACE_H

/*!
	\file vgdrawingsurface.h
	\brief OpenVG drawing surface, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "vg_priv.h"
#include "vgcontext.h"

// It copies pixel data from the source image onto the drawing surface.
AMbool amDrawingSurfacePixelsSet(AMContext *context,
								 AMDrawingSurface *surface,
								 AMint32 dx,
								 AMint32 dy,
								 VGImage src,
								 AMint32 sx,
								 AMint32 sy,
								 AMint32 width,
								 AMint32 height);
// It copies pixel data onto the drawing surface, without the creation of a VGImage object.
AMbool amDrawingSurfacePixelsWrite(AMContext *context,
								   AMDrawingSurface *surface,
								   AMint32 dx,
								   AMint32 dy,
								   const void *data,
								   const VGImageFormat dataFormat,
								   const AMint32 dataStride,
								   AMint32 width,
								   AMint32 height);
// It retrieves pixel data from the drawing surface into the destination image.
AMbool amDrawingSurfacePixelsGet(AMImage *dst,
								 AMint32 dx,
								 AMint32 dy,
								 AMContext *context,
								 const AMDrawingSurface *surface,
								 AMint32 sx,
								 AMint32 sy,
								 AMint32 width,
								 AMint32 height);
// It allows pixel data to be copied from the drawing surface without the creation of a VGImage object.
AMbool amDrawingSurfacePixelsRead(void *data,
								  const VGImageFormat dataFormat,
								  const AMint32 dataStride,
								  AMContext *context,
								  const AMDrawingSurface *surface,
								  AMint32 sx,
								  AMint32 sy,
								  AMint32 width,
								  AMint32 height);
// It copies pixels from one region of the drawing surface to another.
AMbool amDrawingSurfacePixelsCopy(AMContext *context,
								  AMDrawingSurface *surface,
								  AMint32 dx,
								  AMint32 dy,
								  AMint32 sx,
								  AMint32 sy,
								  AMint32 width,
								  AMint32 height);

#endif
