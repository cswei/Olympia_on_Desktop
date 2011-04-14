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

#ifndef _VGIMAGE_H
#define _VGIMAGE_H

/*!
	\file vgimage.h
	\brief OpenVG images, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "vgtexture.h"
#include "dynarray.h"

// *********************************************************************
//                                AMImage
// *********************************************************************

// pixel format flags
#define FMT_L			1	// linear color space
#define FMT_PRE			2	// premultiplied color space
#define FMT_ALPHA		4	// format includes an alpha value
// pixel format descriptors
#define FMT_R_SH		0	// red shift
#define FMT_G_SH		1	// green shift
#define FMT_B_SH		2	// blue shift
#define FMT_A_SH		3	// alpha shift
#define FMT_BITS		4	// bits per pixel
#define FMT_R_BITS		5   // red channel bits
#define FMT_G_BITS		6	// green channel bits
#define FMT_B_BITS		7	// blue channel bits
#define FMT_A_BITS		8	// alpha channel bits
#define FMT_ORDER		9	// one of the AM_PIXEL_FMT_RGBA/ABGR/ARGB/BGRA
#define FMT_FLAGS		10	// format flags, see three constant above

AM_EXTERN_C const AMint32 pxlFormatTable[AM_IMAGE_FORMATS_COUNT][FMT_FLAGS - FMT_R_SH + 1];

//! Given an image format, it returns the corresponding index in the internal pixel format table.
#define AM_FMT_GET_INDEX(_imageFormat) ((((_imageFormat) >> 6) & 0x03) * (AM_LAST_IMAGE_FORMAT - AM_FIRST_IMAGE_FORMAT + 1) + ((_imageFormat) & 0x0F))

// forward declarations
struct _AMImage;
typedef struct _AMImage *AMImagePtr;

// array type used to store image children
AM_DYNARRAY_DECLARE(AMImagePtrDynArray, AMImagePtr, _AMImagePtrDynArray)

//! Image structure, used to implement OpenVG images.
typedef struct _AMImage {
	//! VGHandle type identifier, it can be AM_IMAGE_HANDLE_ID or AM_INVALID_HANDLE_ID.
	AMuint16 id;
	//! It's always AM_IMAGE_HANDLE_ID, never changed.
	AMuint16 type;
	//! VG handle (index inside the context createdHandlesList).
	VGHandle vgHandle;
	//! Reference counter.
	AMuint32 referenceCounter;
	//! Image format.
	VGImageFormat format;
	//! Image (allowed) quality.
	VGbitfield allowedQuality;
	//! Image width, in pixels.
	AMint32 width;
	//! Image height, in pixels.
	AMint32 height;
	//! Image pixels.
	AMuint8 *pixels;
	//! Data stride (bytes per line).
	AMint32 dataStride;
	//! Offset amount, respect to root image, in the x direction (0 for "root" images, > 0 for child images).
	AMint32 x;
	//! Offset amount, respect to root image, in the y direction (0 for "root" images, > 0 for child images).
	AMint32 y;
	//! Root image, the image that owns pixels memory.
	struct _AMImage *root;
	//! Parent image handle, 0 if this image has no parent.
	VGImage parent;
	//! Array of children.
	AMImagePtrDynArray children;
	//! AM_TRUE if both width and height are power of two, else AM_FALSE.
	AMbool isPow2;
	//! If power of two, it represents shift such as (1 << widthShift) = width.
	AMuint32 widthShift;
	//! If power of two, it represents shift such as (1 << heightShift) = height.
	AMuint32 heightShift;
#if !defined(AM_STANDALONE) || defined(RIM_VG_SRC) || defined(TORCH_VG_SRC)
	//! AM_TRUE if the image is bound to a pbuffer (eglCreatePbufferFromClientBuffer)
	AMbool inUseByEgl;
#endif

#if defined(AM_GLE) || defined(AM_GLS)
	//! OpenGL / OpenGL|ES main texture.
	AMTexture patternMainTexture;
	//! OpenGL / OpenGL|ES "tile reflect" texture.
	AMTexture patternReflectTexture;
	//! OpenGL / OpenGL|ES "tile fill" texture.
	AMTexture patternFillTexture;
	//! Validity flag for textures used for pattern.
	AMbool patternTexturesValid;
	// Current pixels format of patternMainTexture / patternReflectTexture / patternFillTexture.
	VGImageFormat patternTexturePixelsFormat;

	//! OpenGL / OpenGL|ES texture used by vgDrawImage.
	AMTexture drawImageTexture;
	//! Validity flag for textures used for vgDrawImage.
	AMbool drawImageTextureValid;
	//! AM_TRUE if drawImageTexture pixels are in the format 0.5 * rgba + 0.5.
	AMbool drawImageTexturePixelsAreStencil;
	// Current pixels format of drawImageTexture.
	VGImageFormat drawImageTexturePixelsFormat;

#if (AM_OPENVG_VERSION >= 110)
	//!	Hash value corresponding to a color transform configuration, for pattern. When color transform changes, hash changes, and textures must be recalculated.
	AMuint32 ctPatternTexturesHash;
	//!	Hash value corresponding to a color transform configuration, for draw image. When color transform changes, hash	changes, and textures must be recalculated.
	AMuint32 ctDrawImageTextureHash;
#endif
#endif
} AMImage;

/*!
	\brief Structure containing parameters to be passed to an image pixel sampler.
*/
typedef struct _AMImageSamplerParams {
	//! The source image.
	const AMImage *image;
	/*!
		Format index corresponding to the source image (srcIdx = AM_FMT_GET_INDEX(image)). It's passed
		as a parameter just to avoid its extraction at runtime.
	*/
	AMuint32 srcIdx;
	//! x-coordinate at which take the sample, in 16.16 fixed point format.
	AMint32 x;
	//! y-coordinate at which take the sample, in 16.16 fixed point format.
	AMint32 y;
	//! Tiling mode to use when sample coordinates are outside the image boundaries.
	VGTilingMode tilingMode;
	//! 32bit tile color in the format corresponding to dstIdx format index.
	AMuint32 tileFillColor;
	//! Format index corresponding to the destination 32bit pixel format.
	AMuint32 dstIdx;
	//! AM_TRUE to use bilinear filtering, else AM_FALSE.
	AMbool bilinear;
	//! Color transformation values, in fixed point.
#if (AM_OPENVG_VERSION >= 110)
	//! Optional color transformation values.
	AMint32 colorTransformValues[8];
	//! Optional color transformation (it may be NULL or point to colorTransformValues).
	const AMint32 *colorTransformation;
#endif
} AMImageSamplerParams;

//! Function prototype, used to read and transform an image sample.
typedef AMuint32 (*AMImageSampler)(const AMImageSamplerParams *);
// Get an image sampler corresponding to the given image format.
AMImageSampler amImageSamplerGet(const VGImageFormat format);
// Check if the given image format is valid.
AMbool amImageFormatValid(const VGImageFormat format);
// Check if two images overlap.
AMbool amImagesOverlap(const AMImage *img1,
					   const AMImage *img2);
// Return bytes required to store a pixel of a given image format.
AMint32 amImageBytesPerPixel(const VGImageFormat format);
#if defined(AM_GLE) || defined(AM_GLS)
// Resize the given source image, using bilinear filter (and applying an optional color trasformation), and write the output to the destination 32bit image.
void amImageBilinearResize(AMImage *dst,
						   const AMImage *src,
						   const AMfloat *colorTransformation,
						   const AMbool halfScaleBias);
// It invalidates texture flags, taking care of children.
void amImageTexturesInvalidate(AMImage *image,
							   const void *_context);
#endif
// Given an image, it returns AM_TRUE if the image is opaque, else AM_FALSE.
AMbool amImageIsOpaque(const AMImage *img);
// Initialize a given image structure.
AMbool amImageInit(AMImage *image,
				   const VGImageFormat format,
				   const VGbitfield allowedQuality,
				   const AMint32 x,
				   const AMint32 y,
				   const AMint32 width,
				   const AMint32 height,
				   VGImage parent,
				   void *_context);
// Destroy image resources.
void amImageResourcesDestroy(AMImage *image,
							 void *_context);
// Destroy the specified image.
void amImageDestroy(AMImage *image,
					void *_context);
// Fill a given rectangle of an image with the specified color.
void amImageClear(AMImage *image,
				  AMint32 x,
				  AMint32 y,
				  AMint32 width,
				  AMint32 height,
				  const AMfloat *sRGBAcolor,
				  const void *_context);
// It reads pixel values from memory, performs format conversion if necessary, and stores the resulting
// pixels into a rectangular portion of an image.
void amImageSubDataSet(AMImage *image,
					   const void *data,
					   const VGImageFormat dataFormat,
					   const AMint32 dataStride,
					   AMint32 x,
					   AMint32 y,
					   AMint32 width,
					   AMint32 height,
					   const void *_context);
// It reads pixel values from a rectangular portion of an image, performs format conversion if
// necessary, and stores the resulting pixels into memory.
void amImageSubDataGet(void *data,
					   const VGImageFormat dataFormat,
					   const AMint32 dataStride,
					   AMint32 dx,
					   AMint32 dy,
					   const AMImage *image,
					   AMint32 sx,
					   AMint32 sy,
					   AMint32 width,
					   AMint32 height);
// Copy a rectangular region from a source image to a destination image.
AMbool amImageCopy(AMImage *dst,
				   AMint32 dx,
				   AMint32 dy,
				   const AMImage *src,
				   AMint32 sx,
				   AMint32 sy,
				   AMint32 width,
				   AMint32 height,
				   const AMbool dither,
				   void *_context);

#if defined RIM_VG_SRC
// Given an image, it returns AM_TRUE if the image is opaque, else AM_FALSE.
AMbool amImageIsOpaque(const AMImage *img);
#endif
#endif
