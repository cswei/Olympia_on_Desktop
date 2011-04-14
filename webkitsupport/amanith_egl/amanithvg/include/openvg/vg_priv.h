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

#ifndef _VG_PRIV_H
#define _VG_PRIV_H

/*!
	\file vg_priv.h
	\brief AmanithVG private data structures and functions, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "openvg.h"
#include "vgtexture.h"
#include "vgscissors.h"

#define OPENVG_NO_RETVAL

typedef struct _AMDrawingSurface {
	//! AM_TRUE if the surface has been initialized, else AM_FALSE.
	AMbool initialized;
	//! AM_TRUE if the surface has been initialized by address (i.e. specifying pixels pointer), else AM_FALSE.
	AMbool initializedByAddr;
	//! Width, in pixels.
	AMint32 width;
	//! Height, in pixels.
	AMint32 height;
	//! Pointer to pixels.
#if defined(AM_SRE) && defined(AM_LITE_PROFILE)
	AMuint16 *pixels;
#else
	AMuint32 *pixels;
#endif
	//! Datastride.
	AMint32 dataStride;
	//! OpenVG image format.
	AMuint32 realFormat;
	//! Pointer to alpha mask pixels (non NULL for OpenVG surfaces only).
	AMuint8 *alphaMaskPixels;
#if defined(AM_GLE) || defined(AM_GLS)
	//! Inverted alpha mask.
	AMbool alphaMaskTextureValid;
	AMbool invAlphaMaskTextureValid;
	AMTexture alphaMaskTexture;
	AMTexture invAlphaMaskTexture;
#endif
#if defined(RIM_VG_SRC)
    //! A pixmap will not be freed or mallocated by AM.
    AMbool isPixmapSurface; 
#endif
	AMScissorRectDynArray clippedScissorRects;
	AMScissorEventDynArray scissorEvents;
	//! Current vertical spans list.
	AMScissorYSpanDynArray curSpanList;
	//! Merged span list.
	AMScissorYMergedSpanDynArray mergedSpanList;
	//! Current span events.
	AMScissorYSpanEventDynArray curSpanEvents;
#if defined(AM_SRE)
	AMInt32DynArray dirtyRegions;
	AMScissorRectDynArray splitDirtyRegions;
	AMbool wholeCleared;
	AMbool dirtyRegionsOverflow;
	#if defined(AM_LITE_PROFILE)
		AMuint16 clearColor;
	#else
		AMuint32 clearColor;
	#endif
#endif
} AMDrawingSurface;

// Get the pointer to the current context and drawing surface. AM_FALSE is returned if the operation failed.
void amCtxSrfCurrentGet(void **_context,
						void **_surface);

void amCtxSrfCurrentSet(void *_context,
						void *_surface);

#if defined RIM_VG_SRC
void *amCtxCurrentGet(void);
// Get the pointer to the current surface.
void *amSrfCurrentGet(void);
// Set the current surface to a new surface.
AMDrawingSurface *amSrfCurrentSet(AMDrawingSurface *surface);
#endif

#if defined(AM_STANDALONE)
	#define OPENVG_RETURN(_retValue) return _retValue;
	#define amSrfWidthGet(_surface) ((_surface)->width)
	#define amSrfHeightGet(_surface) ((_surface)->height)
	// get the real format of the drawing surface (a 16bit format for lite profile, else a 32bit premultiplied format)
	#define amSrfRealFormatGet(_surface) ((_surface)->realFormat)
	// in SRE Lite Profile, drawing surfaces are 16bit
	#if defined(AM_SRE) && defined(AM_LITE_PROFILE)
		// get the corresponding 32bit format relative to the real format (in the full profile they are the same)
		#if defined(AM_SURFACE_BYTE_ORDER_RGBA) || defined(AM_SURFACE_BYTE_ORDER_BGRA)
			#define amSrfFormat32Get(_surface) (((_surface)->realFormat == VG_sRGB_565) ? VG_sRGBA_8888_PRE : VG_sBGRA_8888_PRE)
		#else
			#define amSrfFormat32Get(_surface) (((_surface)->realFormat == VG_sRGB_565) ? VG_sARGB_8888_PRE : VG_sABGR_8888_PRE)
		#endif
	#else
		// in SRE, GLE, GLS, drawing surfaces are alwyas 32bit
		#define amSrfFormat32Get(_surface) ((_surface)->realFormat)
	#endif
	#define amSrfDataStrideGet(_surface) ((_surface)->dataStride)
	#define amSrfPixelsGet(_surface) ((_surface)->pixels)

#else
	#define OPENVG_RETURN(_retValue) \
		amMutexRelease(); \
		return _retValue;

	#define amSrfWidthGet(_surface) ((_surface)->mipmaps.data[(_surface)->mipmapLevel].width)
	#define amSrfHeightGet(_surface) ((_surface)->mipmaps.data[(_surface)->mipmapLevel].height)
	// get the real format of the drawing surface (a 16bit format for lite profile, else a 32bit premultiplied format)
	#define amSrfRealFormatGet(_surface) ((_surface)->vgFormat)
	// get the corresponding 32bit format relative to the real format (in the full profile they are the same)
	#define amSrfFormat32Get(_surface) ((_surface)->vgFormat)
	#define amSrfDataStrideGet(_surface) ((_surface)->mipmaps.data[(_surface)->mipmapLevel].dataStride)
	#define amSrfPixelsGet(_surface) ((_surface)->mipmaps.data[(_surface)->mipmapLevel].pixels)

	// Functions used by EGL public implementation
	VG_API_CALL signed int VG_API_ENTRY vgPrivCtxSizeGet(void) VG_API_EXIT;

	VG_API_CALL unsigned int VG_API_ENTRY vgPrivCtxInit(void *_context) VG_API_EXIT;

	VG_API_CALL void VG_API_ENTRY vgPrivCtxDestroy(void *_context) VG_API_EXIT;

	VG_API_CALL signed int VG_API_ENTRY vgPrivSrfMaxSizeGet(void) VG_API_EXIT;

	VG_API_CALL VGImageFormat VG_API_ENTRY vgPrivSrfFormat(const unsigned int linearColorSpace,
														   const unsigned int alphaFormatPre) VG_API_EXIT;

	VG_API_CALL void VG_API_ENTRY vgPrivImageInfoGet(void *_context,
													 VGImage buffer,
													 unsigned int *isOpenVGImage,
													 unsigned int *isInUse,
													 signed int *width,
													 signed int *height,
													 unsigned int *colorSpaceLinear,
													 unsigned int *alphaFormatPre,
													 signed int *redSize,
													 signed int *greenSize,
													 signed int *blueSize,
													 signed int *alphaSize,
													 signed int *luminanceSize) VG_API_EXIT;

	VG_API_CALL unsigned int VG_API_ENTRY vgPrivImageInUseByOpenVG(void *_context,
																   VGHandle buffer) VG_API_EXIT;

	VG_API_CALL unsigned int  VG_API_ENTRY vgPrivImageRefCounterInc(void *_context,
																	VGImage handle);

	VG_API_CALL unsigned int  VG_API_ENTRY vgPrivImageRefCounterDec(void *_context,
																	VGImage handle);

	VG_API_CALL void VG_API_ENTRY vgPrivImageLock(void *_context,
												  VGImage handle,
												  AMDrawingSurface *srf) VG_API_EXIT;

	VG_API_CALL void VG_API_ENTRY vgPrivImageUnlock(void *_context,
													VGImage handle,
													AMDrawingSurface *srf) VG_API_EXIT;

	VG_API_CALL void VG_API_ENTRY vgPrivDirtyRegionsInvalidate(AMDrawingSurface *srf) VG_API_EXIT;
#endif

#if defined(TORCH_VG_SRC)
	// Functions used by EGL public implementation
	VG_API_CALL void VG_API_ENTRY vgPrivImageInfoGet(void *_context,
													 VGImage buffer,
													 unsigned int *isOpenVGImage,
													 unsigned int *isInUse,
													 signed int *width,
													 signed int *height,
													 unsigned int *colorSpaceLinear,
													 unsigned int *alphaFormatPre,
													 signed int *redSize,
													 signed int *greenSize,
													 signed int *blueSize,
													 signed int *alphaSize,
													 signed int *luminanceSize) VG_API_EXIT;

	VG_API_CALL unsigned int VG_API_ENTRY vgPrivImageInUseByOpenVG(void *_context,
																   VGHandle buffer) VG_API_EXIT;


	VG_API_CALL void VG_API_ENTRY vgPrivImageLock(void *_context,
												  VGImage handle,
												  void *srf) VG_API_EXIT;

	VG_API_CALL void VG_API_ENTRY vgPrivImageUnlock(void *_context,
													VGImage handle,
													void *srf) VG_API_EXIT;

#endif

#endif
