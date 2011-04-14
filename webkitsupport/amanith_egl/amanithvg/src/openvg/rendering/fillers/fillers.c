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

/*!
	\file fillers.c
	\brief Scanline fillers and paint generation routines, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/
#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif



#if defined(AM_SRE)

#include "fillers.h"
#include "vgconversions.h"
#include "vggradients.h"
#include "pixel_utils.h"
#if defined(AM_LITE_PROFILE)
	#include "fillers_prototypes.h"
	#include "img_fillers_prototypes.h"
#endif

/*!
	\brief It returns the path filler according to the paint descriptor.
	\param context context where pathFillersFunctions table is contained.
	\param paintDesc the paint descriptor.
*/
AMScanlineFillerFunction amFilPathSelect(const AMContext *context,
										 const AMPaintDesc *paintDesc) {

	AM_ASSERT(context);
	AM_ASSERT(paintDesc);

#if !defined(AM_LITE_PROFILE)
	if (paintDesc->masking)
		return context->pathFillersFunctions[amPaintTypeGetIndex(paintDesc->paintType)]
											[amBlendModeGetIndex(paintDesc->blendMode)]
											[1];
	else
		return context->pathFillersFunctions[amPaintTypeGetIndex(paintDesc->paintType)]
											[amBlendModeGetIndex(paintDesc->blendMode)]
											[0];
#else
	(void)context;
	if (paintDesc->paintType == VG_PAINT_TYPE_COLOR && !paintDesc->masking) {

		switch (paintDesc->blendMode) {
			case VG_BLEND_SRC:
				return amFilPath_ColorSrc;
			case VG_BLEND_SRC_OVER:
				return amFilPath_ColorSrcOver;
			default:
				return amFilPath;
		}
	}
	else
		return amFilPath;
#endif
}

/*!
	\brief It returns the image filler according to the paint descriptor.
	\param context context where imageFillersFunctions table is contained.
	\param paintDesc the paint descriptor.
*/
AMScanlineFillerFunction amFilImageSelect(const AMContext *context,
										  const AMPaintDesc *paintDesc) {

	AM_ASSERT(context);
	AM_ASSERT(paintDesc);

#if !defined(AM_LITE_PROFILE)
	if (paintDesc->userToSurfaceDesc->userToSurfaceAffine)
		return context->imageFillersFunctions[amImageModeGetIndex(paintDesc->imageMode)]
											 [amBlendModeGetIndex(paintDesc->blendMode)]
											 [0];
	else
		return context->imageFillersFunctions[amImageModeGetIndex(paintDesc->imageMode)]
											 [amBlendModeGetIndex(paintDesc->blendMode)]
											 [1];
#else
	(void)context;
	switch (paintDesc->imageMode) {
		case VG_DRAW_IMAGE_NORMAL:
			return (paintDesc->userToSurfaceDesc->userToSurfaceAffine) ? amFilImage_NormalAffine : amFilImage_NormalProjective;
		case VG_DRAW_IMAGE_MULTIPLY:
			return amFilImage_Multiply;
		default:
			return amFilImage_Stencil;
	}
#endif
}

/*!
	\brief Initialize paint generator structure, for a vgDrawPath call.
	\param paintGen output paint generator structure to initialize.
	\param context input context, containing the atan2 precalculated table.
	\param surface input drawing surface, where the paint generation will be applied on.
	\param rasterizer input rasterizer, containing the coverage deltas for a single (current) scanline.
	\param paintDesc input paint descriptor.
	\param ctReferenceHash hash of the desired color transform to use for the (path) paint.
*/
void amPaintGenPathInit(AMPaintGen *paintGen,
						const AMContext *context,
						const AMDrawingSurface *surface,
						const AMRasterizer *rasterizer,
						const AMPaintDesc *paintDesc,
						const AMuint32 ctReferenceHash) {

	const AMUserToSurfaceDesc *userToSurfaceDesc = paintDesc->userToSurfaceDesc;
	AMuint32 surfaceIdx = AM_FMT_GET_INDEX(amSrfFormat32Get(surface));
	AMuint32 srfByteOrder = pxlFormatTable[surfaceIdx][FMT_ORDER];
	AMMatrix33f m, r;
	AMfloat directionLen;
	AMfloat col[4];
#if defined(VG_MZT_conical_gradient)
	AMVect2f dirTC, n, p;
#endif

	AM_ASSERT(paintGen);
	AM_ASSERT(surface);
	AM_ASSERT(paintDesc);

	paintGen->paintDesc = paintDesc;
	paintGen->coverageLineDeltas = rasterizer->coverageLineDeltas;
#if defined(VG_MZT_conical_gradient)
	paintGen->atan2Table = context->atan2Table;
#endif

	switch (paintDesc->paintType) {

		case VG_PAINT_TYPE_COLOR:
			// paintDesc->paintColor components are in the [0; 1] range
			col[AM_R] = paintDesc->paintColor[AM_R];
			col[AM_G] = paintDesc->paintColor[AM_G];
			col[AM_B] = paintDesc->paintColor[AM_B];
			col[AM_A] = paintDesc->paintColor[AM_A];
			paintGen->paintColor32 = amColorPackByFormat(col, surfaceIdx);
			// used to draw images in multiply mode: byte order must match the drawing surface one, but the color space must be the original paint color space
			// in the case of color, OpenVG always uses non-linear color spaces
			switch (srfByteOrder) {
				case AM_PIXEL_FMT_RGBA:
					paintGen->paintColor32ImgMultiply = amColorPackByFormat(col, AM_FMT_GET_INDEX(VG_sRGBA_8888_PRE));
					break;
				case AM_PIXEL_FMT_ARGB:
					paintGen->paintColor32ImgMultiply = amColorPackByFormat(col, AM_FMT_GET_INDEX(VG_sARGB_8888_PRE));
					break;
				case AM_PIXEL_FMT_BGRA:
					paintGen->paintColor32ImgMultiply = amColorPackByFormat(col, AM_FMT_GET_INDEX(VG_sBGRA_8888_PRE));
					break;
				default:
					paintGen->paintColor32ImgMultiply = amColorPackByFormat(col, AM_FMT_GET_INDEX(VG_sABGR_8888_PRE));
					break;
			}
			break;

		case VG_PAINT_TYPE_LINEAR_GRADIENT:
			AM_MATRIX33_MUL(&m, paintDesc->invPaintToUser, userToSurfaceDesc->inverseUserToSurface, AMMatrix33f)
			AM_MATRIX33_SET(&r, 1.0f, 0.0f, -paintDesc->paint->linGradPt0[AM_X],
								0.0f, 1.0f, -paintDesc->paint->linGradPt0[AM_Y],
								0.0f, 0.0f, 1.0f)
			AM_MATRIX33_MUL(&paintGen->screenToPaintMatrix, &r, &m, AMMatrix33f);

			// direction
			paintGen->direction.x = paintDesc->paint->patchedLinGradTarget.x - paintDesc->paint->linGradPt0[AM_X];
			paintGen->direction.y = paintDesc->paint->patchedLinGradTarget.y - paintDesc->paint->linGradPt0[AM_Y];
			directionLen = AM_VECT2_SQR_LENGTH(&paintGen->direction);
			paintGen->direction.x /= directionLen;
			paintGen->direction.y /= directionLen;

			// deltas are in 16.16
			paintGen->DUx = (AMint32)(ONE_IN_16_16F * (paintGen->direction.x * paintGen->screenToPaintMatrix.a[0][0] +
													   paintGen->direction.y * paintGen->screenToPaintMatrix.a[1][0]));
			// select the correct gradient texture
			if (paintDesc->paint->colorRampSpreadMode == VG_COLOR_RAMP_SPREAD_REFLECT) {
				paintGen->gradTexture = paintDesc->paint->reflectGradTexture.pixels;
				paintGen->gradTextureImgMultiply = paintDesc->paint->reflectGradTextureImgMultiply.pixels;
			}
			else {
				paintGen->gradTexture = paintDesc->paint->gradTexture.pixels;
				paintGen->gradTextureImgMultiply = paintDesc->paint->gradTextureImgMultiply.pixels;
			}
			break;

		case VG_PAINT_TYPE_RADIAL_GRADIENT:
			AM_MATRIX33_MUL(&m, paintDesc->invPaintToUser, userToSurfaceDesc->inverseUserToSurface, AMMatrix33f)
			AM_MATRIX33_SET(&r, 1.0f, 0.0f, -paintDesc->paint->patchedRadGradFocus.x,
								0.0f, 1.0f, -paintDesc->paint->patchedRadGradFocus.y,
								0.0f, 0.0f, 1.0f)
			AM_MATRIX33_MUL(&paintGen->screenToPaintMatrix, &r, &m, AMMatrix33f);

			paintGen->DUf = paintGen->screenToPaintMatrix.a[0][0];
			paintGen->DVf = paintGen->screenToPaintMatrix.a[1][0];
			paintGen->gradP0 = paintDesc->paint->patchedRadGradFocus;
			paintGen->gradP1.x = paintDesc->paint->radGradCenter[AM_X];
			paintGen->gradP1.y = paintDesc->paint->radGradCenter[AM_Y];
			paintGen->radiusSqr = paintDesc->paint->patchedRadGradRadius * paintDesc->paint->patchedRadGradRadius;

			paintGen->dA = paintGen->DUf * (paintGen->gradP0.x - paintGen->gradP1.x) + paintGen->DVf * (paintGen->gradP0.y - paintGen->gradP1.y);
			paintGen->dB = paintGen->DUf * (paintGen->gradP0.y - paintGen->gradP1.y) - paintGen->DVf * (paintGen->gradP0.x - paintGen->gradP1.x);
							
			// select the correct gradient texture
			if (paintDesc->paint->colorRampSpreadMode == VG_COLOR_RAMP_SPREAD_REFLECT) {
				paintGen->gradTexture = paintDesc->paint->reflectGradTexture.pixels;
				paintGen->gradTextureImgMultiply = paintDesc->paint->reflectGradTextureImgMultiply.pixels;
			}
			else {
				paintGen->gradTexture = paintDesc->paint->gradTexture.pixels;
				paintGen->gradTextureImgMultiply = paintDesc->paint->gradTextureImgMultiply.pixels;
			}
			break;

	#if defined(VG_MZT_conical_gradient)
		case VG_PAINT_TYPE_CONICAL_GRADIENT_MZT:
			// apply paint matrix
			AM_VECT2_SET(&p, paintDesc->paint->conGradPt0[AM_X], paintDesc->paint->conGradPt0[AM_Y])
			AM_VECT2_SUB(&dirTC, &paintDesc->paint->patchedConGradTarget, &p)
			directionLen = amSqrtf(AM_VECT2_SQR_LENGTH(&dirTC));
			// n have values in [0..1] expressed in 16.16
			AM_VECT2_SET(&n, dirTC.x / directionLen, dirTC.y / directionLen)
			AM_MATRIX33_SET(&r,  n.x, n.y, -(n.x * p.x + n.y * p.y),
								-n.y, n.x,  (n.y * p.x - n.x * p.y),
								0.0f, 0.0f, 1.0f)
			AM_MATRIX33_MUL(&m, paintDesc->invPaintToUser, userToSurfaceDesc->inverseUserToSurface, AMMatrix33f)
			AM_MATRIX33_MUL(&paintGen->screenToPaintMatrix, &r, &m, AMMatrix33f);

			paintGen->DUf = paintGen->screenToPaintMatrix.a[0][0];
			paintGen->DVf = paintGen->screenToPaintMatrix.a[1][0];
			// select the correct gradient texture
			paintGen->gradTexture = paintDesc->paint->gradTexture.pixels;
			paintGen->gradTextureImgMultiply = paintDesc->paint->gradTextureImgMultiply.pixels;
			paintGen->repeats = (AMuint32)(paintDesc->paint->patchedConGradRepeats * (AMfloat)(1 << AM_GRADIENTS_CONICAL_REPEATS_PRECISION_BITS));
			break;
	#endif

		default: {
			const AMImage *pattern = (const AMImage *)context->handles->createdHandlesList.data[paintDesc->paint->pattern];
			AMuint32 patternIdx = AM_FMT_GET_INDEX(pattern->format);
			AMuint32 flags = pxlFormatTable[patternIdx][FMT_FLAGS];
			AMuint32 tileFillColor32;

			AM_ASSERT(paintDesc->paintType == VG_PAINT_TYPE_PATTERN);
			AM_ASSERT(paintDesc->paint->pattern);

			if (paintDesc->paint->tilingMode == VG_TILE_FILL) {

				col[AM_R] = paintDesc->tileFillColor[AM_R];
				col[AM_G] = paintDesc->tileFillColor[AM_G];
				col[AM_B] = paintDesc->tileFillColor[AM_B];
				col[AM_A] = paintDesc->tileFillColor[AM_A];

				// build the tileFillColor to match fillers pipeline
				switch (pxlFormatTable[patternIdx][FMT_BITS]) {

					case 32:
						tileFillColor32 = amColorPackByFormat(col, patternIdx);
						break;

					case 16:
						switch (pxlFormatTable[patternIdx][FMT_ORDER]) {
							case AM_PIXEL_FMT_RGBA:
							case AM_PIXEL_FMT_BGRA:
								if (flags & FMT_PRE)
									tileFillColor32 = amColorPackByFormat(col, (flags & FMT_L) ? AM_FMT_GET_INDEX(VG_lRGBA_8888_PRE) : AM_FMT_GET_INDEX(VG_sRGBA_8888_PRE));
								else
									tileFillColor32 = amColorPackByFormat(col, (flags & FMT_L) ? AM_FMT_GET_INDEX(VG_lRGBA_8888) : AM_FMT_GET_INDEX(VG_sRGBA_8888));
								break;
							default:
								AM_ASSERT(pxlFormatTable[patternIdx][FMT_ORDER] == AM_PIXEL_FMT_ARGB || pxlFormatTable[patternIdx][FMT_ORDER] == AM_PIXEL_FMT_ABGR);
								if (flags & FMT_PRE)
									tileFillColor32 = amColorPackByFormat(col, (flags & FMT_L) ? AM_FMT_GET_INDEX(VG_lARGB_8888_PRE) : AM_FMT_GET_INDEX(VG_sARGB_8888_PRE));
								else
									tileFillColor32 = amColorPackByFormat(col, (flags & FMT_L) ? AM_FMT_GET_INDEX(VG_lARGB_8888) : AM_FMT_GET_INDEX(VG_sARGB_8888));
								break;
						}
						break;

					default:
						if (pattern->format == VG_A_8)
							tileFillColor32 = (AMuint32)amFloorf(col[AM_A] * 255.0f + 0.5f);
					#if (AM_OPENVG_VERSION >= 110)
						else
						if (pattern->format == VG_A_4) {
							tileFillColor32 = (AMuint32)amFloorf(col[AM_A] * 15.0f + 0.5f);
							tileFillColor32 = (tileFillColor32 << 4) | tileFillColor32;
						}
						else
						if (pattern->format == VG_A_1)
							tileFillColor32 = (col[AM_A] > 0.5f) ? 0xFFFFFFFF : 0;
					#endif
						else {
							// lL8
							if (flags & FMT_L)
								tileFillColor32 = (AMuint32)(255.0f * (col[AM_R] * 0.2126f + col[AM_G] * 0.7152f + col[AM_B] * 0.0722f) + 0.5f);
							// sL8
							else {
								// s --> l
								col[AM_R] = amGammaInvConversion(col[AM_R]);
								col[AM_G] = amGammaInvConversion(col[AM_G]);
								col[AM_B] = amGammaInvConversion(col[AM_B]);
								tileFillColor32 = (AMuint32)(255.0f * (col[AM_R] * 0.2126f + col[AM_G] * 0.7152f + col[AM_B] * 0.0722f) + 0.5f);
								// l --> s
								tileFillColor32 = AM_GAMMA_TABLE(tileFillColor32);
							}
							// repeat the b/w bit inside tileFillColor32
							if (pattern->format == VG_BW_1)
								tileFillColor32 = (tileFillColor32 > 127) ? 0xFFFFFFFF : 0;
						}
						break;
				}
			}
			else
				tileFillColor32 = 0;

			AM_MATRIX33_MUL(&paintGen->screenToPaintMatrix, paintDesc->invPaintToUser, userToSurfaceDesc->inverseUserToSurface, AMMatrix33f)
			// deltas are in 16.16
			paintGen->DUx = (AMint32)(ONE_IN_16_16F * paintGen->screenToPaintMatrix.a[0][0]);
			paintGen->DVx = (AMint32)(ONE_IN_16_16F * paintGen->screenToPaintMatrix.a[1][0]);
			// extract pattern sampler function and bilinear flag
			paintGen->patternSampler = amImageSamplerGet(pattern->format);
			paintGen->patternSamplerParams.image = pattern;
			paintGen->patternSamplerParams.srcIdx = patternIdx;
			paintGen->patternSamplerParams.tilingMode = paintGen->paintDesc->paint->tilingMode;
			paintGen->patternSamplerParams.tileFillColor = tileFillColor32;
			paintGen->patternSamplerParams.dstIdx = surfaceIdx;
			paintGen->patternSamplerParams.bilinear = (paintDesc->patternQuality == VG_IMAGE_QUALITY_NONANTIALIASED) ? AM_FALSE : AM_TRUE;
		#if (AM_OPENVG_VERSION >= 110)
			if (paintDesc->colorTransform && ctReferenceHash != AM_COLOR_TRANSFORM_IDENTITY_HASH) {
				amColorTransformFToI(paintGen->patternSamplerParams.colorTransformValues, context->clampedColorTransformValues);
				paintGen->patternSamplerParams.colorTransformation = paintGen->patternSamplerParams.colorTransformValues;
			}
			else
				paintGen->patternSamplerParams.colorTransformation = NULL;
		#endif
			}
			break;
	}
}

/*!
	\brief Initialize paint generator structure, for a vgDrawImage call.
	\param paintGen output paint generator structure to initialize.
	\param context input context, containing the atan2 table.
	\param surface input drawing surface, where the paint generation will be applied on.
	\param rasterizer input rasterizer, containing the coverage deltas for a single (current) scanline.
	\param paintDesc input paint descriptor.
*/
void amPaintGenImageInit(AMPaintGen *paintGen,
						 const AMContext *context,
						 const AMDrawingSurface *surface,
						 const AMRasterizer *rasterizer,
						 const AMPaintDesc *paintDesc) {

	const AMUserToSurfaceDesc *userToSurfaceDesc = paintDesc->userToSurfaceDesc;

	AM_ASSERT(paintGen);
	AM_ASSERT(paintDesc);
	AM_ASSERT(context);
	AM_ASSERT(paintDesc->image);

	paintGen->paintDesc = paintDesc;
	paintGen->coverageLineDeltas = rasterizer->coverageLineDeltas;
#if defined(VG_MZT_conical_gradient)
	paintGen->atan2Table = context->atan2Table;
#endif

	// extract image sampler function and bilinear flag
	paintGen->imageSampler = amImageSamplerGet(paintDesc->image->format);
	paintGen->imageSamplerParams.image = paintDesc->image;
	paintGen->imageSamplerParams.srcIdx = AM_FMT_GET_INDEX(paintDesc->image->format);
	paintGen->imageSamplerParams.tilingMode = VG_TILE_PAD;
	paintGen->imageSamplerParams.bilinear = (paintDesc->imageQuality == VG_IMAGE_QUALITY_NONANTIALIASED) ? AM_FALSE : AM_TRUE;

	// extract uv(w) increments form userToSurface matrix
	if (userToSurfaceDesc->userToSurfaceAffine) {
		paintGen->iDUx = (AMint32)(ONE_IN_16_16F * userToSurfaceDesc->inverseUserToSurface->a[0][0]);
		paintGen->iDVx = (AMint32)(ONE_IN_16_16F * userToSurfaceDesc->inverseUserToSurface->a[1][0]);
	}
	else {
		// include inside du, dv float -> fixedPoint 16.16 scale factor
		paintGen->iDUf = ONE_IN_16_16F * userToSurfaceDesc->inverseUserToSurface->a[0][0];
		paintGen->iDVf = ONE_IN_16_16F * userToSurfaceDesc->inverseUserToSurface->a[1][0];
		paintGen->iDWf = userToSurfaceDesc->inverseUserToSurface->a[2][0];
	}
	if (paintDesc->imageMode != VG_DRAW_IMAGE_NORMAL) {

		AMuint32 flags = pxlFormatTable[paintGen->imageSamplerParams.srcIdx][FMT_FLAGS];
		AMuint32 srfByteOrder = pxlFormatTable[AM_FMT_GET_INDEX(amSrfFormat32Get(surface))][FMT_ORDER];

		// patching of imageMode has already been done in amImageFillPaintDesc inside amDrawImage
		AM_ASSERT(amMatrix33fIsAffine(userToSurfaceDesc->userToSurface));
		// initialize paint generator
		amPaintGenPathInit(paintGen, context, surface, rasterizer, paintDesc,
					#if (AM_OPENVG_VERSION >= 110)
						(paintDesc->colorTransform && paintDesc->imageMode == VG_DRAW_IMAGE_STENCIL) ? context->colorTransformHash : AM_COLOR_TRANSFORM_IDENTITY_HASH
					#else
						AM_COLOR_TRANSFORM_IDENTITY_HASH
					#endif
						);

		switch (srfByteOrder) {
			case AM_PIXEL_FMT_RGBA:
				paintGen->imageSamplerParams.dstIdx = (flags & FMT_L) ? (AM_FMT_GET_INDEX(VG_lRGBA_8888_PRE)) : (AM_FMT_GET_INDEX(VG_sRGBA_8888_PRE));
				break;
			case AM_PIXEL_FMT_ARGB:
				paintGen->imageSamplerParams.dstIdx = (flags & FMT_L) ? (AM_FMT_GET_INDEX(VG_lARGB_8888_PRE)) : (AM_FMT_GET_INDEX(VG_sARGB_8888_PRE));
				break;
			case AM_PIXEL_FMT_BGRA:
				paintGen->imageSamplerParams.dstIdx = (flags & FMT_L) ? (AM_FMT_GET_INDEX(VG_lBGRA_8888_PRE)) : (AM_FMT_GET_INDEX(VG_sBGRA_8888_PRE));
				break;
			default:
				paintGen->imageSamplerParams.dstIdx = (flags & FMT_L) ? (AM_FMT_GET_INDEX(VG_lABGR_8888_PRE)) : (AM_FMT_GET_INDEX(VG_sABGR_8888_PRE));
				break;
		}

		if (paintDesc->imageMode == VG_DRAW_IMAGE_MULTIPLY && paintDesc->paintType == VG_PAINT_TYPE_PATTERN) {
			
			const AMImage *pattern = (const AMImage *)context->handles->createdHandlesList.data[paintDesc->paint->pattern];

			// for multiply images, pattern sample must be converted in the same byte order of the drawing surface
			flags = pxlFormatTable[AM_FMT_GET_INDEX(pattern->format)][FMT_FLAGS];
			switch (srfByteOrder) {
				case AM_PIXEL_FMT_RGBA:
					paintGen->patternSamplerParams.dstIdx = (flags & FMT_L) ? (AM_FMT_GET_INDEX(VG_lRGBA_8888_PRE)) : (AM_FMT_GET_INDEX(VG_sRGBA_8888_PRE));
					break;
				case AM_PIXEL_FMT_ARGB:
					paintGen->patternSamplerParams.dstIdx = (flags & FMT_L) ? (AM_FMT_GET_INDEX(VG_lARGB_8888_PRE)) : (AM_FMT_GET_INDEX(VG_sARGB_8888_PRE));
					break;
				case AM_PIXEL_FMT_BGRA:
					paintGen->patternSamplerParams.dstIdx = (flags & FMT_L) ? (AM_FMT_GET_INDEX(VG_lBGRA_8888_PRE)) : (AM_FMT_GET_INDEX(VG_sBGRA_8888_PRE));
					break;
				default:
					paintGen->patternSamplerParams.dstIdx = (flags & FMT_L) ? (AM_FMT_GET_INDEX(VG_lABGR_8888_PRE)) : (AM_FMT_GET_INDEX(VG_sABGR_8888_PRE));
					break;
			}
		}
	}
	else
		paintGen->imageSamplerParams.dstIdx = AM_FMT_GET_INDEX(amSrfFormat32Get(surface));

#if (AM_OPENVG_VERSION >= 110)
	if (paintDesc->imageMode == VG_DRAW_IMAGE_NORMAL) {
		if (paintDesc->colorTransform && context->colorTransformHash != AM_COLOR_TRANSFORM_IDENTITY_HASH) {
			// associate the color transformation on image sampler
			amColorTransformFToI(paintGen->imageSamplerParams.colorTransformValues, context->clampedColorTransformValues);
			paintGen->imageSamplerParams.colorTransformation = paintGen->imageSamplerParams.colorTransformValues;
		}
		else
			paintGen->imageSamplerParams.colorTransformation = NULL;
	}
	else
	if (paintDesc->imageMode == VG_DRAW_IMAGE_MULTIPLY) {

		if (paintDesc->colorTransform && context->colorTransformHash != AM_COLOR_TRANSFORM_IDENTITY_HASH) {
			// associate the color transformation on image sampler
			amColorTransformFToI(paintGen->colorTransformValuesImgMultiply, context->clampedColorTransformValues);
			paintGen->colorTransformationImgMultiply = paintGen->colorTransformValuesImgMultiply;
		}
		else
			paintGen->colorTransformationImgMultiply = NULL;
		// in this case color transform in the image sampler MUST be NULL.
		paintGen->imageSamplerParams.colorTransformation = NULL;
	}
	else
		paintGen->imageSamplerParams.colorTransformation = NULL;
#endif
}

/*!
	\brief Initialize linear gradient paint generation at each scanline.
	\param paintGen output paint generation structure.
	\param x x coordinate of the scanline origin (first pixel to be filled).
	\param y y coordinate of the scanline origin (first pixel to be filled).
*/
void amPaintLinGradInit(AMPaintGen *paintGen,
						const AMint32 x,
						const AMint32 y) {

	AMVect2f p, q;

	AM_ASSERT(paintGen);

	// transform the point into paint coordinate system
	AM_VECT2_SET(&p, (AMfloat)x + 0.5f, (AMfloat)y + 0.5f)
	AM_MATRIX33_VECT2_MUL(&q, &paintGen->screenToPaintMatrix, &p)
	paintGen->Ux = (AMint32)(ONE_IN_16_16F * AM_VECT2_DOT(&q, &paintGen->direction));
}

/*!
	\brief Initialize radial gradient paint generation at each scanline.
	\param paintGen output paint generation structure.
	\param x x coordinate of the scanline origin (first pixel to be filled).
	\param y y coordinate of the scanline origin (first pixel to be filled).
*/
void amPaintRadGradInit(AMPaintGen *paintGen,
						const AMint32 x,
						const AMint32 y) {

	AMfloat fx, fy, cx, cy;
	AMVect2f p, q;

	AM_ASSERT(paintGen);

	// transform the point into paint coordinate system
	AM_VECT2_SET(&p, (AMfloat)x + 0.5f, (AMfloat)y + 0.5f)
	AM_MATRIX33_VECT2_MUL(&q, &paintGen->screenToPaintMatrix, &p)

	paintGen->Uf = q.x;
	paintGen->Vf = q.y;

	fx = paintGen->gradP0.x;
	fy = paintGen->gradP0.y;
	cx = paintGen->gradP1.x;
	cy = paintGen->gradP1.y;
	paintGen->K = SIXTEEN_IN_16_16F / (paintGen->radiusSqr - (fx - cx) * (fx - cx) - (fy - cy) * (fy - cy));
	paintGen->A = paintGen->Uf * (fx - cx) + paintGen->Vf * (fy - cy);
	paintGen->B = paintGen->Uf * (fy - cy) - paintGen->Vf * (fx - cx);
}

#if defined(VG_MZT_conical_gradient)
/*!
	\brief Initialize conical gradient paint generation at each scanline.
	\param paintGen output paint generation structure.
	\param x x coordinate of the scanline origin (first pixel to be filled).
	\param y y coordinate of the scanline origin (first pixel to be filled).
*/
void amPaintConGradInit(AMPaintGen *paintGen,
						const AMint32 x,
						const AMint32 y) {

	AMVect2f p, q;

	AM_ASSERT(paintGen);

	// transform the point into paint coordinate system
	AM_VECT2_SET(&p, (AMfloat)x + 0.5f, (AMfloat)y + 0.5f)
	AM_MATRIX33_VECT2_MUL(&q, &paintGen->screenToPaintMatrix, &p)

	paintGen->Uf = q.x;
	paintGen->Vf = q.y;
}
#endif

/*!
	\brief Initialize pattern paint generation at each scanline.
	\param paintGen output paint generation structure.
	\param x x coordinate of the scanline origin (first pixel to be filled).
	\param y y coordinate of the scanline origin (first pixel to be filled).
*/
void amPaintPatternInit(AMPaintGen *paintGen,
						const AMint32 x,
						const AMint32 y) {

	AMVect2f uv, p;

	AM_ASSERT(paintGen);

	// transform the point into paint coordinate system
	AM_VECT2_SET(&p, (AMfloat)x + 0.5f, (AMfloat)y + 0.5f)
	AM_MATRIX33_VECT2_MUL(&uv, &paintGen->screenToPaintMatrix, &p)

	paintGen->Ux = (AMint32)(ONE_IN_16_16F * uv.x);
	paintGen->Vx = (AMint32)(ONE_IN_16_16F * uv.y);
}

/*!
	\brief Initialize image paint generation at each scanline.
	\param paintGen output paint generation structure.
	\param x x coordinate of the scanline origin (first pixel to be filled).
	\param y y coordinate of the scanline origin (first pixel to be filled).
*/
void amPaintImageInit(AMPaintGen *paintGen,
					  const AMint32 x,
					  const AMint32 y) {

	const AMUserToSurfaceDesc *userToSurfaceDesc = paintGen->paintDesc->userToSurfaceDesc;

	AM_ASSERT(paintGen);

	if (userToSurfaceDesc->userToSurfaceAffine) {
		
		AMVect2f uv, p;
		// transform the point into image coordinate system
		AM_VECT2_SET(&p, (AMfloat)x + 0.5f, (AMfloat)y + 0.5f)
		AM_MATRIX33_VECT2_MUL(&uv, userToSurfaceDesc->inverseUserToSurface, &p)
		paintGen->iUx = (AMint32)(ONE_IN_16_16F * uv.x);
		paintGen->iVx = (AMint32)(ONE_IN_16_16F * uv.y);
	}
	else {
		AMVect3f uvw, p;

		AM_VECT3_SET(&p, (AMfloat)x + 0.5f, (AMfloat)y + 0.5f, 1.0f)
		AM_MATRIX33_VECT3_MUL(&uvw, userToSurfaceDesc->inverseUserToSurface, &p, AMVect3f)
		// include inside u, v float -> fixedPoint 16.16 scale factor
		paintGen->iUf = ONE_IN_16_16F * uvw.x;
		paintGen->iVf = ONE_IN_16_16F * uvw.y;
		paintGen->iWf = uvw.z;
	}
}

// It converts an image color sample (32bit premultiplied pixel) into surface format, applying an optional color transformation.
AMuint32 amImgSampleToSurface(AMuint32 valuePre,
							  const AMuint32 sampleIdx,
							  const AMint32 *colorTransformation,
							  const AMuint32 dstIdx) {

#if defined(AM_SURFACE_BYTE_ORDER_RGBA)
	AMPixel32RGBA t0;
#elif defined(AM_SURFACE_BYTE_ORDER_ARGB)
	AMPixel32ARGB t0;
#elif defined(AM_SURFACE_BYTE_ORDER_BGRA)
	AMPixel32BGRA t0;
#else
	AMPixel32ABGR t0;
#endif
	AMuint32 sampleFlags = pxlFormatTable[sampleIdx][FMT_FLAGS];
	AMuint32 dstFlags = pxlFormatTable[dstIdx][FMT_FLAGS];
	AMint32 r, g, b, a;

	// NB: image sample is in the same byte order of the drawing surface
	t0.value = valuePre;
	r = (AMint32)t0.c.r;
	g = (AMint32)t0.c.g;
	b = (AMint32)t0.c.b;
	a = (AMint32)t0.c.a;
	AM_ASSERT(r <= a && g <= a && b <= a);
	AM_ASSERT(sampleFlags & FMT_PRE);

#if (AM_OPENVG_VERSION >= 110)
	if (colorTransformation) {
		// valuePre is always premultiplied, so we have to remove premultiplication
		AM_UNPREMULTIPLY(r, g, b, r, g, b, a)
		r = (r * colorTransformation[0] + colorTransformation[4]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION;
		g = (g * colorTransformation[1] + colorTransformation[5]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION;
		b = (b * colorTransformation[2] + colorTransformation[6]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION;
		a = (a * colorTransformation[3] + colorTransformation[7]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION;
		r = AM_CLAMP(r, 0, 255);
		g = AM_CLAMP(g, 0, 255);
		b = AM_CLAMP(b, 0, 255);
		a = AM_CLAMP(a, 0, 255);
	}
	else
#else
	(void)colorTransformation;
#endif
	{
		if ((sampleFlags & FMT_L) == (dstFlags & FMT_L)) {
			t0.c.r = (AMuint8)r;
			t0.c.g = (AMuint8)g;
			t0.c.b = (AMuint8)b;
			t0.c.a = (AMuint8)a;
			return t0.value;
		}
		// valuePre is always premultiplied, and source and destination format have different s/l color spaces
		AM_UNPREMULTIPLY(r, g, b, r, g, b, a)
	}

	// l --> s
	if ((sampleFlags & FMT_L) && !(dstFlags & FMT_L)) {
		r = AM_GAMMA_TABLE(r);
		g = AM_GAMMA_TABLE(g);
		b = AM_GAMMA_TABLE(b);
	}
	// s --> l
	else 
	if (!(sampleFlags & FMT_L) && (dstFlags & FMT_L)) {
		r = AM_GAMMA_INV_TABLE(r);
		g = AM_GAMMA_INV_TABLE(g);
		b = AM_GAMMA_INV_TABLE(b);
	}
	AM_ASSERT(dstFlags & FMT_PRE);
	MULT_DIV_255(r, r, a)
	MULT_DIV_255(g, g, a)
	MULT_DIV_255(b, b, a)

	AM_ASSERT(a >= 0 && a <= 255);
	AM_ASSERT(r >= 0 && r <= a);
	AM_ASSERT(g >= 0 && g <= a);
	AM_ASSERT(b >= 0 && b <= a);

	t0.c.r = (AMuint8)r;
	t0.c.g = (AMuint8)g;
	t0.c.b = (AMuint8)b;
	t0.c.a = (AMuint8)a;
	return t0.value;
}

#endif

#if defined (RIM_VG_SRC)
#pragma pop
#endif


