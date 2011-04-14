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

#ifndef _SREDRAWINGSURFACE_H
#define _SREDRAWINGSURFACE_H

/*!
	\file sredrawingsurface.h
	\brief OpenVG drawing surface (SRE), header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined(AM_SRE)

#include "vg_priv.h"
#include "vgcontext.h"

// It copies pixel data from the source image onto the drawing surface, performing a SRC_OVER operation for each pixel.
// Full clip of the specified bound to the current drawing surface is performed too.
// NB: image format must match exactly with the surface format.
AMbool amSreDrawingSurfacePixelsBlend(AMContext *context,
									  AMDrawingSurface *surface,
									  AMint32 dx,
									  AMint32 dy,
									  VGImage src,
									  AMint32 sx,
									  AMint32 sy,
									  AMint32 width,
									  AMint32 height);

// It copies pixel data from the source image onto the drawing surface swapping y coordinates, performing a SRC_OVER operation for each pixel.
// Full clip of the specified bound to the current drawing surface is performed too.
// NB: image format must match exactly with the surface format.
AMbool amSreDrawingSurfacePixelsBlendInvertedY(AMContext *context,
											   AMDrawingSurface *surface,
											   AMint32 dx,
											   AMint32 dy,
											   VGImage src,
											   AMint32 sx,
											   AMint32 sy,
											   AMint32 width,
											   AMint32 height);

// It copies pixel data from the source image onto the drawing surface swapping y coordinates, performing
// full clip of the specified bound to the current drawing surface.
AMbool amSreDrawingSurfacePixelsSetInvertedY(AMContext *context,
											 AMDrawingSurface *surface,
											 AMint32 dx,
											 AMint32 dy,
											 VGImage src,
											 AMint32 sx,
											 AMint32 sy,
											 AMint32 width,
											 AMint32 height);

// It copies pixel data from the source image onto the drawing surface.
AMbool amSreDrawingSurfacePixelsSet(AMContext *context,
									AMDrawingSurface *surface,
									const AMint32 dx,
									const AMint32 dy,
									VGImage src,
									const AMint32 sx,
									const AMint32 sy,
									const AMint32 width,
									const AMint32 height);
// It copies pixel data onto the drawing surface, without the creation of a VGImage object.
AMbool amSreDrawingSurfacePixelsWrite(AMContext *context,
									  AMDrawingSurface *surface,
									  const AMint32 dx,
									  const AMint32 dy,
									  const void *data,
									  const VGImageFormat dataFormat,
									  const AMint32 dataStride,
									  const AMint32 width,
									  const AMint32 height);
// It retrieves pixel data from the drawing surface into the destination image.
AMbool amSreDrawingSurfacePixelsGet(AMImage *dst,
									const AMint32 dx,
									const AMint32 dy,
									AMContext *context,
									const AMDrawingSurface *surface,
									const AMint32 sx,
									const AMint32 sy,
									const AMint32 width,
									const AMint32 height);
// It allows pixel data to be copied from the drawing surface without the creation of a VGImage object.
AMbool amSreDrawingSurfacePixelsRead(void *data,
									 const VGImageFormat dataFormat,
									 const AMint32 dataStride,
									 const AMint32 dx,
									 const AMint32 dy,
									 const AMContext *context,
									 const AMDrawingSurface *surface,
									 const AMint32 sx,
									 const AMint32 sy,
									 const AMint32 width,
									 const AMint32 height);
// It copies pixels from one region of the drawing surface to another.
AMbool amSreDrawingSurfacePixelsCopy(AMContext *context,
									 AMDrawingSurface *surface,
									 const AMint32 dx,
									 const AMint32 dy,
									 const AMint32 sx,
									 const AMint32 sy,
									 const AMint32 width,
									 const AMint32 height);
#endif
#endif
