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

#ifndef _FILLERS_H
#define _FILLERS_H

/*!
	\file fillers.h
	\brief Scanline fillers and paint generation routines, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "vgpaint.h"
#include "vgcontext.h"

#define ONE_IN_16_16F 65536.0f
#define SIXTEEN_IN_16_16F 1048576.0f

/*!
	\brief Internal structure used by fillers. It contains all information needed to generate paint at
	each pixel.
*/
typedef struct _AMPaintGen {

	//! Pointer to a paint descriptor.
	const AMPaintDesc *paintDesc;
	//! Array that contains coverage deltas for a single (current) scanline (the pointer comes from an AMRasterizer structure).
	AMint32 *coverageLineDeltas;
#if defined(VG_MZT_conical_gradient)
	//! Table that contains atan2 function values, used by conical gradients (the pointer comes from an AMContext structure).
	const AMuint32 *atan2Table;
#endif

	//!	\name Radial and conical gradient UV coordinates and their forward differences.
	//@{
	//! Current U coordinate, float (used by radial/conical gradients).
	AMfloat Uf;
	//! Current V coordinate, float (used by radial/conical gradients).
	AMfloat Vf;
	//! Current dU (forward difference), float (used by radial/conical gradients).
	AMfloat DUf;
	//! Current dV (forward difference), float (used by radial/conical gradients).
	AMfloat DVf;
	//@}

	/*!
		\name Radial gradient coefficients.
		\brief Coefficients (and their forward differences) used by radial gradient.
	*/
	//@{
	AMfloat K;
	AMfloat A, B;
	AMfloat dA, dB;
	//@}

	//!	\name Linear gradient and pattern UV coordinates and their forward differences.
	//@{
	//! Current U coordinate, integer (used by linear gradients and pattern).
	AMint32 Ux;
	//! Current V coordinate, integer (used by linear gradients and pattern).
	AMint32 Vx;
	//! Current dU (forward difference), integer (used by linear gradients and pattern).
	AMint32 DUx;
	//! Current dV (forward difference), integer (used by linear gradients and pattern).
	AMint32 DVx;
	//@}

	//!	\name Image (perspective) UVW coordinates and their forward differences.
	//@{
	//! Current U coordinate, float.
	AMfloat iUf;
	//! Current V coordinate, float.
	AMfloat iVf;
	//! Current W coordinate, float.
	AMfloat iWf;
	//! Current dU (forward difference), float.
	AMfloat iDUf;
	//! Current dV (forward difference), float.
	AMfloat iDVf;
	//! Current dW (forward difference), float.
	AMfloat iDWf;
	//@}

	//!	\name Image (affine) UV coordinates and their forward differences.
	//@{
	//! Current U coordinate, integer.
	AMint32 iUx;
	//! Current V coordinate, integer.
	AMint32 iVx;
	//! Current dU (forward difference), integer.
	AMint32 iDUx;
	//! Current dV (forward difference), integer.
	AMint32 iDVx;
	//@}

	/*!
		\name Precalculated values.
		\brief Precalculated values inside amPaintGenPathInit and amPaintGenImageInit, at each rasterization.
	*/
	//@{
	//! Color in the pixel format of the current drawing surface (used when paintType is VG_PAINT_TYPE_COLOR).
	AMuint32 paintColor32;
	//! Pointer to gradient texture pixels (used when paintType is VG_PAINT_TYPE_LINEAR_GRADIENT, VG_PAINT_TYPE_RADIAL_GRADIENT, VG_PAINT_TYPE_CONICAL_GRADIENT_MZT).
	const AMuint32 *gradTexture;
#if defined(AM_SRE)
	//! Color in the sPRE pixel format and byteorder of the current drawing surface (used to draw an image in multiply mode when paintType is VG_PAINT_TYPE_COLOR).
	AMuint32 paintColor32ImgMultiply;
	//! Pointer to gradient texture pixels (used to draw images in multiply mode when paintType is VG_PAINT_TYPE_LINEAR_GRADIENT, VG_PAINT_TYPE_RADIAL_GRADIENT, VG_PAINT_TYPE_CONICAL_GRADIENT_MZT).
	const AMuint32 *gradTextureImgMultiply;
#endif
	//! Gradient normalized direction (used when paintType is VG_PAINT_TYPE_LINEAR_GRADIENT).
	AMVect2f direction;
	//! Gradient focus point (used when paintType is VG_PAINT_TYPE_RADIAL_GRADIENT).
	AMVect2f gradP0;
	//! Gradient center point (used when paintType is VG_PAINT_TYPE_RADIAL_GRADIENT).
	AMVect2f gradP1;
	//! Gradient squared radius (used when paintType is VG_PAINT_TYPE_RADIAL_GRADIENT).
	AMfloat radiusSqr;

#if defined(VG_MZT_conical_gradient)
	//! Conical gradient repeats in AM_GRADIENTS_CONICAL_REPEATS_PRECISION_BITS precision (used when paintType is VG_PAINT_TYPE_CONICAL_GRADIENT_MZT).
	AMuint32 repeats;
#endif

	//! Pixel sampler used to read pattern pixels.
	AMImageSampler patternSampler;
	//! Pixel sampler parameters used to read pattern pixels.
	AMImageSamplerParams patternSamplerParams;

	//! Affine 3x3 matrix to do screen->paint transformation (used when paintType is different than VG_PAINT_TYPE_COLOR).
	AMMatrix33f screenToPaintMatrix;
	//! Pixel sampler used to read image pixels.
	AMImageSampler imageSampler;
	//! Pixel sampler parameters used to read image pixels.
	AMImageSamplerParams imageSamplerParams;
	//! Optional color transformation values, for images in multiply mode.
	AMint32 colorTransformValuesImgMultiply[8];
	//! Optional color transformation (it may be NULL or point to colorTransformValuesImgMultiply) for images in multiply mode.
	const AMint32 *colorTransformationImgMultiply;
#if (AM_OPENVG_VERSION >= 110)
	//! Clip box used for render to mask only.
	const AMAABox2i *clipBox;
#endif
	//@}
} AMPaintGen;

#if defined(AM_SRE)

#if !defined(AM_LITE_PROFILE)
// Initialize path fillers table.
void amFilPathTableInit(AMContext *context);
// Initialize image fillers table.
void amFilImageTableInit(AMContext *context);
#endif

// It returns the path filler according to the paint descriptor.
AMScanlineFillerFunction amFilPathSelect(const AMContext *context,
										 const AMPaintDesc *paintDesc);
// It returns the image filler according to the paint descriptor.
AMScanlineFillerFunction amFilImageSelect(const AMContext *context,
										  const AMPaintDesc *paintDesc);
// Initialize global paint generation values for each path rasterization.
void amPaintGenPathInit(AMPaintGen *paintGen,
						const AMContext *context,
						const AMDrawingSurface *surface,
						const AMRasterizer *rasterizer,
						const AMPaintDesc *paintDesc,
						const AMuint32 ctReferenceHash);
// Initialize global paint generation values for each image rasterization.
void amPaintGenImageInit(AMPaintGen *paintGen,
						 const AMContext *context,
						 const AMDrawingSurface *surface,
						 const AMRasterizer *rasterizer,
						 const AMPaintDesc *paintDesc);
// Initialize linear gradient paint generation at each scanline.
void amPaintLinGradInit(AMPaintGen *paintGen,
						const AMint32 x,
						const AMint32 y);
// Initialize radial gradient paint generation at each scanline.
void amPaintRadGradInit(AMPaintGen *paintGen,
						const AMint32 x,
						const AMint32 y);
#if defined(VG_MZT_conical_gradient)
// Initialize conical gradient paint generation at each scanline.
void amPaintConGradInit(AMPaintGen *paintGen,
						const AMint32 x,
						const AMint32 y);
#endif
// Initialize pattern paint generation at each scanline.
void amPaintPatternInit(AMPaintGen *paintGen,
						const AMint32 x,
						const AMint32 y);
// Initialize image paint generation at each scanline.
void amPaintImageInit(AMPaintGen *paintGen,
					  const AMint32 x,
					  const AMint32 y);

/*!
	\brief Get a pixel from a gradient texture, according to the spreadMode (used by linear gradients).
	\note All gradient textures are AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH x 1 pixels, the right theoretic line
	would be: (fixedU << AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH_BITS) >> 16\n
	please note that for reflect spreadMode u coordinate is halved because the texture contains two gradients at
	(AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH / 2) x 1 pixels each.
*/
#define AM_GRADIENTS_LINEAR_PIXEL_GET(_res, _u) \
	switch (spreadMode) { \
		case VG_COLOR_RAMP_SPREAD_PAD: \
			(_u) = AM_CLAMP((_u), 0, 0xFFFF); \
			break; \
		case VG_COLOR_RAMP_SPREAD_REPEAT: \
			(_u) &= 0xFFFF; \
			break; \
		default: \
			AM_ASSERT(spreadMode == VG_COLOR_RAMP_SPREAD_REFLECT); \
			(_u) = ((_u) >> 1) & 0xFFFF; \
			break; \
	} \
	(_res) = gradientPixels[(_u) >> (16 - AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH_BITS)];

/*!
	\brief Get a pixel from a gradient texture, according to the spreadMode (used by radial gradients).
	\note All gradient textures are AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH x 1 pixels, the right theoretic line
	would be: (fixedU << AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH_BITS) >> 16\n
	please note that for reflect spreadMode u coordinate is halved because the texture contains two gradients at
	(AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH / 2) x 1 pixels each.
*/
#define AM_GRADIENTS_RADIAL_PIXEL_GET(_res, _u) \
	switch (spreadMode) { \
		case VG_COLOR_RAMP_SPREAD_PAD: \
			(_u) = AM_CLAMP((_u), 0, 0xFFFFF); \
			break; \
		case VG_COLOR_RAMP_SPREAD_REPEAT: \
			(_u) &= 0xFFFFF; \
			break; \
		default: \
			AM_ASSERT(spreadMode == VG_COLOR_RAMP_SPREAD_REFLECT); \
			(_u) = ((_u) >> 1) & 0xFFFFF; \
			break; \
	} \
	(_res) = gradientPixels[(_u) >> (20 - AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH_BITS)];

/*!
	\brief Get a pixel from a gradient texture, according to the spreadMode (used by conical gradients).
*/
#define AM_GRADIENTS_CONICAL_PIXEL_GET(_res, _u, _v) \
	atan2Val = paintGen->atan2Table[((_v) << AM_GRADIENTS_CONICAL_TEXTURE_WIDTH_BITS) + (_u)]; \
	atan2U = (atan2Val * AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH * paintGen->repeats) >> (AM_GRADIENTS_CONICAL_REPEATS_PRECISION_BITS + AM_ATAN2_TABLE_PRECISION_BITS); \
	switch (spreadMode) { \
		case VG_COLOR_RAMP_SPREAD_PAD: \
			if (atan2U >= AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH) \
				atan2U = AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH - 1; \
			break; \
		case VG_COLOR_RAMP_SPREAD_REPEAT: \
			atan2U &= (AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH - 1); \
			break; \
		default: \
			AM_ASSERT(spreadMode == VG_COLOR_RAMP_SPREAD_REFLECT); \
			if ((atan2U >> AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH_BITS) & 1) \
				atan2U = (AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH - 1) - (atan2U & (AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH - 1)); \
			else \
				atan2U &= (AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH - 1); \
			break; \
	} \
	(_res) = gradientPixels[atan2U];


// It converts an image color sample (32bit premultiplied pixel) into surface format, applying an optional color transformation.
AMuint32 amImgSampleToSurface(AMuint32 valuePre,
							  const AMuint32 sampleIdx,
							  const AMint32 *colorTransformation,
							  const AMuint32 dstIdx);

#endif
#endif
