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
	\file vgpaint.c
	\brief OpenVG paint, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "vgcompositing.h"
#include "vgconversions.h"
#if defined(AM_SRE)
	#include "srecompositing.h"
#elif defined(AM_GLE)
	#include "glecompositing.h"
	#include "glestyle.h"
	#include "glpattern.h"
	#include "glimage.h"
	#include "glmask.h"
#elif defined(AM_GLS)
	#include "glscompositing.h"
	#include "glpattern.h"
	#include "glimage.h"
	#include "glmask.h"
#else
	#error Unreachable point.
#endif

#include "vggradients.h"
#include "vgmask.h"

#if defined RIM_VG_SRC
#include "vgimage.h"
#endif

#if defined RIM_VG_SRC
#define VG_API_ENTRY 
#endif

// *********************************************************************
//                         Paints pools management
// *********************************************************************
AMbool amPaintsPoolsManagerInit(AMPaintsPoolsManager *paintsPoolsManager) {

	AM_ASSERT(paintsPoolsManager);

	AM_DYNARRAY_PREINIT(paintsPoolsManager->pools)
	AM_DYNARRAY_PREINIT(paintsPoolsManager->availablePaintsList)
	paintsPoolsManager->initialized = AM_FALSE;

	// initialize the array used to store referencies to available paints
	AM_DYNARRAY_INIT_RESERVE(paintsPoolsManager->availablePaintsList, AMPaintRef, 64)
	if (paintsPoolsManager->availablePaintsList.error)
		return AM_FALSE;

	// allocate pools pointers
	AM_DYNARRAY_INIT(paintsPoolsManager->pools, AMPaintsPoolPtr)
	if (paintsPoolsManager->pools.error) {
		AM_DYNARRAY_DESTROY(paintsPoolsManager->availablePaintsList)
		return AM_FALSE;
	}

	paintsPoolsManager->pools.size = 0;
	paintsPoolsManager->initialized = AM_TRUE;
	return AM_TRUE;
}

void amPaintsPoolsManagerDestroy(AMPaintsPoolsManager *paintsPoolsManager) {

	AMuint32 i;

	AM_ASSERT(paintsPoolsManager);

	// if the pools manager was not previously initialized, simply exit
	if (!paintsPoolsManager->initialized)
		return;

	for (i = 0; i < paintsPoolsManager->pools.size; ++i) {

		AMPaintsPool *poolPtr = paintsPoolsManager->pools.data[i];

		AM_ASSERT(poolPtr);
		amFree(poolPtr);
	}

	AM_DYNARRAY_DESTROY(paintsPoolsManager->pools)
	AM_DYNARRAY_DESTROY(paintsPoolsManager->availablePaintsList)
	paintsPoolsManager->initialized = AM_FALSE;
}

void amPaintsPoolsAvailablesSort(AMPaintsPoolsManager *paintsPoolsManager) {

	amUint32QSort((AMuint32 *)paintsPoolsManager->availablePaintsList.data, paintsPoolsManager->availablePaintsList.size);
}

void amPaintMemoryRetrieve(AMPaint *paint,
						   const AMbool preserveData,
						   const void *_context) {

	(void)_context;

	if (preserveData) {
		AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(paint->sColorStops, AMColorStop)
	}
	else {
		AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(paint->sColorStops, AMColorStop)
		// delete gradient textures
		amTextureDestroy(&paint->gradTexture);
		amTextureDestroy(&paint->reflectGradTexture);
		amTextureDestroy(&paint->gradTextureImgMultiply);
		amTextureDestroy(&paint->reflectGradTextureImgMultiply);
		// reset texture validity flag
		paint->gradTexturesValid = AM_FALSE;
		
	}
}


/*!
	\brief Given a float color in non-linear non-premultiplied color space (components in [0;1] range), it returns the corresponding 32bit color according to the specified format index.
	\param col input color, float components.
	\param formatIndex 32bit destination pixel format index.
	\return the 32bit color value.
*/
AMuint32 amColorPackByFormat(const AMfloat *col,
							 const AMuint32 formatIndex) {

	AMuint32 r = (AMuint32)amFloorf(col[AM_R] * 255.0f + 0.5f);
	AMuint32 g = (AMuint32)amFloorf(col[AM_G] * 255.0f + 0.5f);
	AMuint32 b = (AMuint32)amFloorf(col[AM_B] * 255.0f + 0.5f);
	AMuint32 a = (AMuint32)amFloorf(col[AM_A] * 255.0f + 0.5f);
	AMuint32 flags = pxlFormatTable[formatIndex][FMT_FLAGS];

	AM_ASSERT(col[AM_R] >= 0.0f && col[AM_R] <= 1.0f);
	AM_ASSERT(col[AM_G] >= 0.0f && col[AM_G] <= 1.0f);
	AM_ASSERT(col[AM_B] >= 0.0f && col[AM_B] <= 1.0f);
	AM_ASSERT(col[AM_A] >= 0.0f && col[AM_A] <= 1.0f);
	AM_ASSERT(r <= 255 && g <= 255 && b <= 255 && a <= 255);
	AM_ASSERT(pxlFormatTable[formatIndex][FMT_BITS] == 32);
	
	// s --> l
	if (flags & FMT_L) {
		r = AM_GAMMA_INV_TABLE(r);
		g = AM_GAMMA_INV_TABLE(g);
		b = AM_GAMMA_INV_TABLE(b);
	}
	// alpha premultiplication
	if (flags & FMT_PRE) {
		MULT_DIV_255(r, r, a)
		MULT_DIV_255(g, g, a)
		MULT_DIV_255(b, b, a)
	}

	return ((r << pxlFormatTable[formatIndex][FMT_R_SH]) |
			(g << pxlFormatTable[formatIndex][FMT_G_SH]) |
			(b << pxlFormatTable[formatIndex][FMT_B_SH]) |
			(a << pxlFormatTable[formatIndex][FMT_A_SH]));
}

/*!
	\brief Given a float color transformation, it returns its fixed point equivalent.
	\param dst output color transformation, integer (fixed point) components.
	\param src input color transformation, float components.
*/
void amColorTransformFToI(AMint32 *dst,
						  const AMfloat *src) {

	AM_ASSERT(dst);
	AM_ASSERT(src);

	// scale
	dst[0] = (AMint32)(AM_CLAMP(src[0], -AM_COLOR_TRANSFORM_SCALE_BIAS_MAX_F, AM_COLOR_TRANSFORM_SCALE_BIAS_MAX_F) * AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION_F);
	dst[1] = (AMint32)(AM_CLAMP(src[1], -AM_COLOR_TRANSFORM_SCALE_BIAS_MAX_F, AM_COLOR_TRANSFORM_SCALE_BIAS_MAX_F) * AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION_F);
	dst[2] = (AMint32)(AM_CLAMP(src[2], -AM_COLOR_TRANSFORM_SCALE_BIAS_MAX_F, AM_COLOR_TRANSFORM_SCALE_BIAS_MAX_F) * AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION_F);
	dst[3] = (AMint32)(AM_CLAMP(src[3], -AM_COLOR_TRANSFORM_SCALE_BIAS_MAX_F, AM_COLOR_TRANSFORM_SCALE_BIAS_MAX_F) * AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION_F);
	// bias
	dst[4] = (AMint32)(AM_CLAMP(src[4], -AM_COLOR_TRANSFORM_SCALE_BIAS_MAX_F, AM_COLOR_TRANSFORM_SCALE_BIAS_MAX_F) * (AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION_F * 256.0f));
	dst[5] = (AMint32)(AM_CLAMP(src[5], -AM_COLOR_TRANSFORM_SCALE_BIAS_MAX_F, AM_COLOR_TRANSFORM_SCALE_BIAS_MAX_F) * (AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION_F * 256.0f));
	dst[6] = (AMint32)(AM_CLAMP(src[6], -AM_COLOR_TRANSFORM_SCALE_BIAS_MAX_F, AM_COLOR_TRANSFORM_SCALE_BIAS_MAX_F) * (AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION_F * 256.0f));
	dst[7] = (AMint32)(AM_CLAMP(src[7], -AM_COLOR_TRANSFORM_SCALE_BIAS_MAX_F, AM_COLOR_TRANSFORM_SCALE_BIAS_MAX_F) * (AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION_F * 256.0f));	
}

// Given a paint descriptor, it returns AM_TRUE if the paint is opaque (taking care of color transform, alpha masking and so on), else AM_FALSE.
AMbool amPaintIsOpaque(const AMPaintDesc *paintDesc,
					   const AMbool includeColorTransform,
					   const void *_context) {

	const AMContext *context = (const AMContext *)_context;
	AMfloat alpha;
	AMbool res;
	AMImage *pattern;

	AM_ASSERT(paintDesc);
	AM_ASSERT(context);

	res = AM_TRUE;
	switch (paintDesc->paintType) {

		case VG_PAINT_TYPE_COLOR:
			alpha = paintDesc->paintColor[AM_A];
		#if (AM_OPENVG_VERSION >= 110)
			// check for the color transform
			if (includeColorTransform && paintDesc->colorTransform)
				alpha = alpha * context->clampedColorTransformValues[3] + context->clampedColorTransformValues[7];
		#endif
			if (alpha < 1.0f)
				res = AM_FALSE;
			break;

		case VG_PAINT_TYPE_LINEAR_GRADIENT:
		case VG_PAINT_TYPE_RADIAL_GRADIENT:
	#if defined(VG_MZT_conical_gradient)
		case VG_PAINT_TYPE_CONICAL_GRADIENT_MZT:
	#endif
		#if (AM_OPENVG_VERSION >= 110)
			if (includeColorTransform && paintDesc->colorTransform) {

				AMfloat minAlpha = paintDesc->paint->gradientMinAlpha * context->clampedColorTransformValues[3] + context->clampedColorTransformValues[7];
				AMfloat maxAlpha = paintDesc->paint->gradientMaxAlpha * context->clampedColorTransformValues[3] + context->clampedColorTransformValues[7];

				res = (minAlpha >= 1.0f && maxAlpha >= 1.0f) ? AM_TRUE : AM_FALSE;
			}
			else
				res = (paintDesc->paint->gradientMinAlpha >= 1.0f) ? AM_TRUE : AM_FALSE;
		#else
			res = (paintDesc->paint->gradientMinAlpha >= 1.0f) ? AM_TRUE : AM_FALSE;
		#endif
			break;
		
		default:
			AM_ASSERT(paintDesc->paintType == VG_PAINT_TYPE_PATTERN);
			AM_ASSERT(paintDesc->paint);
			AM_ASSERT(paintDesc->paint->pattern);

			pattern = (AMImage *)context->handles->createdHandlesList.data[paintDesc->paint->pattern];

			if (!amImageIsOpaque(pattern)) {
				// original pattern image is not opaque...
				res = AM_FALSE;
			#if (AM_OPENVG_VERSION >= 110)
				// ... but we could have a color transform that makes the pattern opaque
				if (includeColorTransform && paintDesc->colorTransform && context->clampedColorTransformValues[3] >= 0.0f && context->clampedColorTransformValues[7] >= 1.0f)
					res = AM_TRUE;
			#endif
			}
			else {
				// original pattern image is opaque...
			#if (AM_OPENVG_VERSION >= 110)
				// ... but we could have a color transform that makes the pattern non-opaque
				if (includeColorTransform && paintDesc->colorTransform && context->clampedColorTransformValues[3] < 1.0f - context->clampedColorTransformValues[7])
					res = AM_FALSE;
			#endif
			}

			if (res && paintDesc->paint->tilingMode == VG_TILE_FILL) {
				alpha = paintDesc->tileFillColor[AM_A];
			#if (AM_OPENVG_VERSION >= 110)
				// check for the color transform
				if (includeColorTransform && paintDesc->colorTransform)
					alpha = alpha * context->clampedColorTransformValues[3] + context->clampedColorTransformValues[7];
			#endif
				if (alpha < 1.0f)
					res = AM_FALSE;
			}
			break;
	}
	return res;
}

/*!
	\brief Set a paint inside a context.
	\param _context context containing the current fill and stroke paints.
	\param paint the paint handle to set.
	\param paintModes a bitwise OR of values from the VGPaintMode enumeration, determining whether the paint
	object is to be used for filling (VG_FILL_PATH), stroking (VG_STROKE_PATH), or both (VG_FILL_PATH |	VG_STROKE_PATH).
*/
void amPaintSet(void *_context,
				VGHandle paint,
				const VGbitfield paintModes) {

	AMContext *context = (AMContext *)_context;
	AMContextHandlesList *handles = context->handles;
	AMPaint *strokePnt = handles->createdHandlesList.data[context->strokePaint];
	AMPaint *fillPnt = handles->createdHandlesList.data[context->fillPaint];
	AMPaint *pnt = handles->createdHandlesList.data[paint];

	AM_ASSERT(context);

	if (pnt) {
		if (paintModes & VG_STROKE_PATH && strokePnt != pnt) {
			if (strokePnt) {
				if (amCtxHandleRefCounterDec(strokePnt, context) == 0) {
					// remove a possible reference to pattern image
					amPaintPatternSet(strokePnt, 0, context);
					// remove paint object from context internal list
					amPaintRemove(context, strokePnt);
				}
			}
			// set a new stroke paint, increasing its reference counter
			context->strokePaint = paint;
			amCtxHandleRefCounterInc(pnt);
		}
		if (paintModes & VG_FILL_PATH && fillPnt != pnt) {
			if (fillPnt) {
				if (amCtxHandleRefCounterDec(fillPnt, context) == 0) {
					// remove a possible reference to pattern image
					amPaintPatternSet(fillPnt, 0, context);
					// remove paint object from context internal list
					amPaintRemove(context, fillPnt);
				}
			}
			// set a new fill paint, increasing its reference counter
			context->fillPaint = paint;
			amCtxHandleRefCounterInc(pnt);
		}
	}
	else {
		if (paintModes & VG_STROKE_PATH) {
			// if context has a stroke paint, we must decrement its reference counter
			if (strokePnt) {
				if (amCtxHandleRefCounterDec(strokePnt, context) == 0) {
					// remove a possible reference to pattern image
					amPaintPatternSet(strokePnt, 0, context);
					// remove paint object from context internal list
					amPaintRemove(context, strokePnt);
				}
			}
			// set a NULL stroke paint
			context->strokePaint = VG_INVALID_HANDLE;
		}
		if (paintModes & VG_FILL_PATH) {
			// if context has a fill paint, we must decrement its reference counter
			if (fillPnt) {
				if (amCtxHandleRefCounterDec(fillPnt, context) == 0) {
					// remove a possible reference to pattern image
					amPaintPatternSet(fillPnt, 0, context);
					// remove paint object from context internal list
					amPaintRemove(context, fillPnt);
				}
			}
			// set a NULL fill paint
			context->fillPaint = VG_INVALID_HANDLE;
		}
	}
}

/*!
	\brief Set the paint color of a given paint.
	\param paint the destination paint.
	\param sColor paint color to set, in the non-linear unpremultiplied color space.
*/
void amPaintColorSet(AMPaint *paint,
					 const AMfloat sColor[4]) {

	AM_ASSERT(sColor);

	paint->sColor[0] = amNanInfFix(sColor[0]);
	paint->sColor[1] = amNanInfFix(sColor[1]);
	paint->sColor[2] = amNanInfFix(sColor[2]);
	paint->sColor[3] = amNanInfFix(sColor[3]);
}

/*!
	\brief Set the paint type of a given paint.
	\param paint the destination paint.
	\param paintType paint type to set.
*/
void amPaintTypeSet(AMPaint *paint,
					const VGPaintType paintType) {

	AM_ASSERT(paint);

	paint->paintType = paintType;
}

/*!
	\brief Set the color ramp spread mode of a given paint.
	\param paint the destination paint.
	\param spreadMode the spread mode to set.
*/
void amPaintColorRampSpreadModeSet(AMPaint *paint,
								   const VGColorRampSpreadMode spreadMode) {

	AM_ASSERT(paint);
	AM_ASSERT(spreadMode == VG_COLOR_RAMP_SPREAD_PAD ||
			  spreadMode == VG_COLOR_RAMP_SPREAD_REPEAT ||
			  spreadMode == VG_COLOR_RAMP_SPREAD_REFLECT);

	paint->colorRampSpreadMode = spreadMode;
}

/*!
	\brief Set the color ramp premultiplied flag of a given paint.
	\param paint the destination paint.
	\param premultiplied premultiplied flag to set.
*/
void amPaintColorRampPremultipliedSet(AMPaint *paint,
									  const VGboolean premultiplied) {
	
	AM_ASSERT(paint);

	// if color ramp premultiplied flag is going to be changed, we have to invalidate gradient textures
	if (paint->colorRampPremultiplied != premultiplied)
		paint->gradTexturesValid |= AM_FALSE;
	paint->colorRampPremultiplied = premultiplied;
}

#if defined(VG_MZT_color_ramp_interpolation)
/*!
	\brief Set the color ramp interpolation type of a given paint.
	\param paint the destination paint.
	\param interpolationType the interpolation type to set.
*/
void amPaintColorRampInterpolationTypeSet(AMPaint *paint,
										  const VGColorRampInterpolationTypeMzt interpolationType) {

	AM_ASSERT(paint);
	AM_ASSERT(interpolationType == VG_COLOR_RAMP_INTERPOLATION_LINEAR_MZT ||
			  interpolationType == VG_COLOR_RAMP_INTERPOLATION_SMOOTH_MZT);

	// if interpolation type is going to be changed, we have to invalidate gradient textures
	if (paint->colorRampInterpolationType != interpolationType)
		paint->gradTexturesValid = AM_FALSE;
	paint->colorRampInterpolationType = interpolationType;
}
#endif

/*!
	\brief Set the tiling mode of a given paint.
	\param paint the destination paint.
	\param tilingMode the tiling mode to set.
*/
void amPaintTilingModeSet(AMPaint *paint,
						  const VGTilingMode tilingMode) {

	AM_ASSERT(paint);
	AM_ASSERT(tilingMode == VG_TILE_FILL ||
			  tilingMode == VG_TILE_PAD ||
			  tilingMode == VG_TILE_REPEAT ||
			  tilingMode == VG_TILE_REFLECT);

	paint->tilingMode = tilingMode;
}

/*!
	\brief Set color stops of a given paint, float values.
	\param paint the destination paint.
	\param stops the color stops to set.
	\param stopsCount number of color stops to set.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
	\pre stopsCount >= 0.
*/
AMbool amPaintColorStopsSetf(AMPaint *paint,
							 const AMfloat *stops,
							 const AMint32 stopsCount) {

	const AMfloat *keys = stops;
	AMint32 i, keyCount;

	AM_ASSERT(paint);
	AM_ASSERT(stopsCount >= 0);

	if (!stops && stopsCount > 0)
		return AM_TRUE;

	keyCount = (stopsCount > 0) ? stopsCount : 2;
	if (!paint->sColorStops.data) {
		// initialize the array
		AM_DYNARRAY_INIT_RESERVE(paint->sColorStops, AMColorStop, keyCount)
	}
	else
	if ((AMint32)paint->sColorStops.capacity < keyCount) {
		// allocate additional memory
	AM_DYNARRAY_CLEAR_RESERVE(paint->sColorStops, AMColorStop, keyCount)
	}
	// check for memory errors
	if (paint->sColorStops.error) {
		paint->sColorStops.error = AM_DYNARRAY_NO_ERROR;
		return AM_FALSE;
	}

	paint->sColorStops.size = 0;
	for (i = 0; i < stopsCount; ++i) {

		AMColorStop key;

		key.position = amNanInfFix(keys[0]);
		key.color[0] = amNanInfFix(keys[1]);
		key.color[1] = amNanInfFix(keys[2]);
		key.color[2] = amNanInfFix(keys[3]);
		key.color[3] = amNanInfFix(keys[4]);
		AM_DYNARRAY_PUSH_BACK_LIGHT(paint->sColorStops, key)
		keys += 5;
	}

	// invalidate texture flags
	paint->gradTexturesValid = AM_FALSE;
	return AM_TRUE;
}

/*!
	\brief Set color stops of a given paint, integer values.
	\param paint the destination paint.
	\param stops the color stops to set.
	\param stopsCount number of color stops to set.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
	\pre stopsCount >= 0.
*/
AMbool amPaintColorStopsSeti(AMPaint *paint,
							 const AMint32 *stops,
							 const AMint32 stopsCount) {

	const AMint32 *keys = stops;
	AMint32 i, keyCount;

	AM_ASSERT(paint);
	AM_ASSERT(stopsCount >= 0);

	if (!stops && stopsCount > 0)
		return AM_TRUE;

	keyCount = (stopsCount > 0) ? stopsCount : 2;
	if (!paint->sColorStops.data) {
		// initialize the array
		AM_DYNARRAY_INIT_RESERVE(paint->sColorStops, AMColorStop, keyCount)
	}
	else
	if ((AMint32)paint->sColorStops.capacity < keyCount) {
		// allocate additional memory
	AM_DYNARRAY_CLEAR_RESERVE(paint->sColorStops, AMColorStop, keyCount)
	}
	// check for memory errors
	if (paint->sColorStops.error) {
		paint->sColorStops.error = AM_DYNARRAY_NO_ERROR;
		return AM_FALSE;
	}

	paint->sColorStops.size = 0;
	for (i = 0; i < stopsCount; ++i) {

		AMColorStop key;

		key.position = (VGfloat)keys[0];
		key.color[0] = (VGfloat)keys[1];
		key.color[1] = (VGfloat)keys[2];
		key.color[2] = (VGfloat)keys[3];
		key.color[3] = (VGfloat)keys[4];
		AM_DYNARRAY_PUSH_BACK_LIGHT(paint->sColorStops, key)
		keys += 5;
	}

	// invalidate texture flag
	paint->gradTexturesValid = AM_FALSE;
	return AM_TRUE;
}

/*!
	\brief Set linear gradient parameters of a given paint.
	\param paint the destination paint.
	\param start linear gradient start point coordinates.
	\param end linear gradient end point coordinates.
*/
void amPaintLinGradParametersSet(AMPaint *paint,
								 const AMfloat start[2],
								 const AMfloat end[2]) {

	AMfloat l;
	AMVect2f p;

	AM_ASSERT(paint);
	AM_ASSERT(start);
	AM_ASSERT(end);

	paint->linGradPt0[AM_X] = p.x = start[AM_X];
	paint->linGradPt0[AM_Y] = p.y = start[AM_Y];
	paint->linGradPt1[AM_X] = paint->patchedLinGradTarget.x = end[AM_X];
	paint->linGradPt1[AM_Y] = paint->patchedLinGradTarget.y = end[AM_Y];

	l = amSqrtf(AM_VECT2_SQR_DISTANCE(&p, &paint->patchedLinGradTarget));
	if (l <= AM_EPSILON_FLOAT) {
		paint->patchedLinGradTarget = p;
		paint->patchedLinGradTarget.x += 1.0f;
	}
}

/*!
	\brief Set radial gradient parameters of a given paint.
	\param paint the destination paint.
	\param center radial gradient center point coordinates.
	\param focus radial gradient focus point coordinates.
	\param radius radial gradient radius.
*/
void amPaintRadGradParametersSet(AMPaint *paint,
								 const AMfloat center[2],
								 const AMfloat focus[2],
								 const AMfloat radius) {

	AM_ASSERT(paint);
	AM_ASSERT(center);
	AM_ASSERT(focus);

	paint->radGradFocus[AM_X] = paint->patchedRadGradFocus.x = focus[AM_X];
	paint->radGradFocus[AM_Y] = paint->patchedRadGradFocus.y = focus[AM_Y];
	paint->radGradRadius = paint->patchedRadGradRadius = radius;
	paint->radGradCenter[AM_X] = center[AM_X];
	paint->radGradCenter[AM_Y] = center[AM_Y];

	// patch possible degenerations
	if (paint->patchedRadGradRadius <= AM_EPSILON_FLOAT) {
		AM_VECT2_SET(&paint->patchedRadGradFocus, paint->radGradCenter[AM_X], paint->radGradCenter[AM_Y])
		paint->patchedRadGradRadius = 0.01f;
	}
	else {
		AMfloat lx = paint->patchedRadGradFocus.x - paint->radGradCenter[AM_X];
		AMfloat ly = paint->patchedRadGradFocus.y - paint->radGradCenter[AM_Y];
		AMfloat l = amSqrtf(lx * lx + ly * ly);
		// According to official OpenVG specification: "To avoid a division by 0, the implementation may move
		// the focal point along the line towards the center of the circle by an amount sufficient to
		// avoid numerical instability, provided the new location lies at a distance of at least .99r from
		// the circle center"
		AMfloat r = 0.99f * paint->patchedRadGradRadius;
		
		if (l >= r) {
			r /= l;
			AM_VECT2_SET(&paint->patchedRadGradFocus, paint->radGradCenter[AM_X] + r * lx, paint->radGradCenter[AM_Y] + r * ly)
		}
	}
}

#if defined(VG_MZT_conical_gradient)
/*!
	\brief Set conical gradient parameters of a given paint.
	\param paint the destination paint.
	\param center conical gradient center point coordinates.
	\param target conical gradient target point coordinates.
	\param repeats conical gradient repeats number.
*/
void amPaintConGradParametersSet(AMPaint *paint,
								 const AMfloat center[2],
								 const AMfloat target[2],
								 const AMfloat repeats) {

	AMVect2f p;

	AM_ASSERT(paint);
	AM_ASSERT(center);
	AM_ASSERT(target);

	paint->conGradPt0[AM_X] = p.x = center[AM_X];
	paint->conGradPt0[AM_Y] = p.y = center[AM_Y];
	paint->conGradPt1[AM_X] = paint->patchedConGradTarget.x = target[AM_X];
	paint->conGradPt1[AM_Y] = paint->patchedConGradTarget.y = target[AM_Y];
	paint->conGradRepeats = repeats;
	paint->patchedConGradRepeats = AM_CLAMP(repeats, 0.0f, ((AMfloat)AM_GRADIENTS_CONICAL_MAX_REPEATS));

	if (amSqrtf(AM_VECT2_SQR_DISTANCE(&p, &paint->patchedConGradTarget)) <= AM_EPSILON_FLOAT) {
		paint->patchedConGradTarget = p;
		paint->patchedConGradTarget.x += 1.0f;
	}
}
#endif

AMbool amGradientTexturesNeedsUpload(AMPaintDesc *paintDesc,
									 const AMuint32 ctReferenceHash,
									 const AMContext *context,
									 const AMuint32 srfFormat) {

#if defined(AM_SRE)
	(void)context;
	(void)srfFormat;
	if (!paintDesc->paint->gradTexturesValid
	#if (AM_OPENVG_VERSION >= 110)
		|| paintDesc->paint->ctTexturesHash != ctReferenceHash
	#endif
		)
		return AM_TRUE;
	return AM_FALSE;
#elif defined(AM_GLE) || defined(AM_GLS)

	#if (AM_OPENVG_VERSION >= 110)

		AMuint32 srfFlags = pxlFormatTable[AM_FMT_GET_INDEX(srfFormat)][FMT_FLAGS];

		// for a vgDrawImage in stencil mode, the trick to use the global glColor cannot be used; in such case the glColor is used to realize stencil image mode
		if (!paintDesc->image &&
			context->ctNormalizedValues &&
			(!(srfFlags & FMT_L))) {
			// set the color transform in the global GL color
			const AMfloat *colorTransformValues = context->clampedColorTransformValues;
			paintDesc->ctGlColor[AM_A] = (colorTransformValues[3] <= 0.0f) ? 0.0f : colorTransformValues[AM_A];
			paintDesc->ctGlColor[AM_R] = (colorTransformValues[0] <= 0.0f) ? 0.0f : colorTransformValues[AM_R] * paintDesc->ctGlColor[AM_A];
			paintDesc->ctGlColor[AM_G] = (colorTransformValues[1] <= 0.0f) ? 0.0f : colorTransformValues[AM_G] * paintDesc->ctGlColor[AM_A];
			paintDesc->ctGlColor[AM_B] = (colorTransformValues[2] <= 0.0f) ? 0.0f : colorTransformValues[AM_B] * paintDesc->ctGlColor[AM_A];
			paintDesc->ctUseGlColor = AM_TRUE;
		}
		else {
			// set the default global GL color
			paintDesc->ctGlColor[AM_R] = 1.0f;
			paintDesc->ctGlColor[AM_G] = 1.0f;
			paintDesc->ctGlColor[AM_B] = 1.0f;
			paintDesc->ctGlColor[AM_A] = 1.0f;
			paintDesc->ctUseGlColor = AM_FALSE;
		}

		if (paintDesc->paint->gradTexturesValid) {
			if (paintDesc->ctUseGlColor)
				return (paintDesc->paint->ctTexturesHash == AM_COLOR_TRANSFORM_IDENTITY_HASH) ? AM_FALSE : AM_TRUE;
			else
				// color transformation is realized inside the texture
				return (paintDesc->paint->ctTexturesHash == ctReferenceHash) ? AM_FALSE : AM_TRUE;
		}
		else
			return AM_TRUE;
	#else
		(void)ctReferenceHash;
		(void)context;
		(void)srfFormat;
		return (paintDesc->paint->gradTexturesValid) ? AM_FALSE : AM_TRUE;
	#endif
#else
	#error Undefined AmanithVG engine type.
#endif
}

/*!
	\brief Update a paint descriptor.
	\param paintDesc paint descriptor to update.
	\param context context containing the list of created handles and color transform values (as well the GL context for AmanithVG GLE).
	\param surface drawing surface containing the alpha mask.
	\param ctReferenceHash hash of the desired color transform to use for the texture pixels generation.
*/
AMbool amPaintDescPaintUpdate(AMPaintDesc *paintDesc,
							  AMContext *context,
							  AMDrawingSurface *surface,
							  const AMuint32 ctReferenceHash) {

#if defined(AM_GLE)
	AMImage *pattern;
	VGImageFormat patternRequiredPixelsFormat;
#endif

	AM_ASSERT(paintDesc);
	AM_ASSERT(context);
	AM_ASSERT(surface);

	switch (paintDesc->paintType) {
		case VG_PAINT_TYPE_COLOR:
			// patch paint color, ensuring components in [0; 1] range
			AM_CLAMP4(paintDesc->paintColor, paintDesc->paintColor, 0.0f, 1.0f)
		#if (AM_OPENVG_VERSION >= 110)
			// apply color transform
			if (paintDesc->colorTransform && ctReferenceHash != AM_COLOR_TRANSFORM_IDENTITY_HASH) {
				paintDesc->paintColor[AM_R] = paintDesc->paintColor[AM_R] * context->clampedColorTransformValues[0] + context->clampedColorTransformValues[4];
				paintDesc->paintColor[AM_G] = paintDesc->paintColor[AM_G] * context->clampedColorTransformValues[1] + context->clampedColorTransformValues[5];
				paintDesc->paintColor[AM_B] = paintDesc->paintColor[AM_B] * context->clampedColorTransformValues[2] + context->clampedColorTransformValues[6];
				paintDesc->paintColor[AM_A] = paintDesc->paintColor[AM_A] * context->clampedColorTransformValues[3] + context->clampedColorTransformValues[7];
				// patch paint color, ensuring components in [0; 1] range
				AM_CLAMP4(paintDesc->paintColor, paintDesc->paintColor, 0.0f, 1.0f)
			}
		#endif
			break;

		case VG_PAINT_TYPE_LINEAR_GRADIENT:
		case VG_PAINT_TYPE_RADIAL_GRADIENT:
	#if defined(VG_MZT_conical_gradient)
		case VG_PAINT_TYPE_CONICAL_GRADIENT_MZT:
	#endif
			if (amGradientTexturesNeedsUpload(paintDesc, ctReferenceHash, context, amSrfFormat32Get(surface))) {
				// upgrade gradient textures if necessary
				if (!amGradientsTexturesUpdate(paintDesc, context, surface, ctReferenceHash))
					return AM_FALSE;
			}
			break;
		case VG_PAINT_TYPE_PATTERN:
			// patch tile fill color, ensuring components in [0; 1] range
			AM_CLAMP4(paintDesc->tileFillColor, context->tileFillColor, 0.0f, 1.0f)
		#if (AM_OPENVG_VERSION >= 110)
			// apply color transform
			if (paintDesc->colorTransform && ctReferenceHash != AM_COLOR_TRANSFORM_IDENTITY_HASH) {
				paintDesc->tileFillColor[AM_R] = paintDesc->tileFillColor[AM_R] * context->clampedColorTransformValues[0] + context->clampedColorTransformValues[4];
				paintDesc->tileFillColor[AM_G] = paintDesc->tileFillColor[AM_G] * context->clampedColorTransformValues[1] + context->clampedColorTransformValues[5];
				paintDesc->tileFillColor[AM_B] = paintDesc->tileFillColor[AM_B] * context->clampedColorTransformValues[2] + context->clampedColorTransformValues[6];
				paintDesc->tileFillColor[AM_A] = paintDesc->tileFillColor[AM_A] * context->clampedColorTransformValues[3] + context->clampedColorTransformValues[7];
				// patch tile fill color, ensuring components in [0; 1] range
				AM_CLAMP4(paintDesc->tileFillColor, paintDesc->tileFillColor, 0.0f, 1.0f)
			}
		#endif

		#if defined(AM_GLE)
			pattern = (AMImage *)context->handles->createdHandlesList.data[paintDesc->paint->pattern];
			AM_ASSERT(pattern);
			// NB: the returned pixelFormat is ALWAYS a premultiplied format
			patternRequiredPixelsFormat = amGlPatternRequiredFormat(paintDesc, context, surface);
			// upgrade pattern textures if necessary
			if (amGlePatternTexturesNeedsUpload(paintDesc, pattern, patternRequiredPixelsFormat, ctReferenceHash, context, (VGImageFormat)amSrfFormat32Get(surface))) {
				if (!amGlPatternTexturesUpdate(paintDesc, context, ctReferenceHash, patternRequiredPixelsFormat))
					return AM_FALSE;
			}
		#elif defined(AM_GLS)
			// TO DO
		#endif
			break;
		default:
			AM_ASSERT(0 == 1);
			break;
	}

	// create alpha mask if masking is on, and make sure that alpha mask texture (GL) is updated
	if (context->masking == VG_TRUE) {
	#if defined(AM_GLE) || defined(AM_GLS)
		if (!surface->alphaMaskTextureValid && surface->alphaMaskPixels) {
			if (!amAlphaMaskTextureUpdate(surface, context))
				return AM_FALSE;
		}
	#endif
	}
	return AM_TRUE;
}

/*!
	\brief Update a paint descriptor, used by vgDrawPath function.
	\param paintDesc paint descriptor to update.
	\param context context containing the list of created handles and color transform values (as well the GL context for AmanithVG GLE).
	\param surface drawing surface containing the alpha mask.
*/
AMbool amPaintDescPathUpdate(AMPaintDesc *paintDesc,
							 AMContext *context,
							 AMDrawingSurface *surface) {

	AMuint32 ctReferenceHash;
	AMuint32 patFlags;
	AMImage *pattern;
	AMuint32 srfFlags = pxlFormatTable[AM_FMT_GET_INDEX(amSrfFormat32Get(surface))][FMT_FLAGS];

#if (AM_OPENVG_VERSION >= 110)
	switch (paintDesc->paintType) {
		case VG_PAINT_TYPE_LINEAR_GRADIENT:
		case VG_PAINT_TYPE_RADIAL_GRADIENT:
	#if defined(VG_MZT_conical_gradient)
		case VG_PAINT_TYPE_CONICAL_GRADIENT_MZT:
	#endif
			ctReferenceHash = (context->ctNormalizedValues && (!(srfFlags & FMT_L))) ? AM_COLOR_TRANSFORM_IDENTITY_HASH : context->colorTransformHash;
			break;
		case VG_PAINT_TYPE_PATTERN:
			pattern = (AMImage *)context->handles->createdHandlesList.data[paintDesc->paint->pattern];
			AM_ASSERT(pattern);
			patFlags = pxlFormatTable[AM_FMT_GET_INDEX(pattern->format)][FMT_FLAGS];
			ctReferenceHash = (context->ctNormalizedValues && (patFlags & FMT_L) == (srfFlags & FMT_L)) ? AM_COLOR_TRANSFORM_IDENTITY_HASH : context->colorTransformHash;
			break;
		default:
			ctReferenceHash = context->colorTransformHash;
			break;
	}
#else
	ctReferenceHash = AM_COLOR_TRANSFORM_IDENTITY_HASH;
#endif

	return amPaintDescPaintUpdate(paintDesc, context, surface, ctReferenceHash);
}

#if defined(AM_GLE) || defined(AM_GLS)
AMbool amImageTexturesNeedsUpload(AMPaintDesc *paintDesc,
								  const VGImageFormat imgRequiredPixelsFormat,
								  const AMuint32 ctReferenceHash,
								  const AMContext *context,
								  const VGImageFormat srfFormat) {

#if (AM_OPENVG_VERSION >= 110)
	AMuint32 srfFlags = pxlFormatTable[AM_FMT_GET_INDEX(srfFormat)][FMT_FLAGS];
	AMuint32 imgFlags = pxlFormatTable[AM_FMT_GET_INDEX(paintDesc->image->format)][FMT_FLAGS];
	AMbool ctUseGlColor;

	AM_ASSERT(srfFlags & FMT_PRE);

	if (context->ctNormalizedValues && paintDesc->imageMode != VG_DRAW_IMAGE_STENCIL) {

		if ((imgFlags & FMT_L) == (srfFlags & FMT_L)) {
			// set the color transform in the global GL color
			const AMfloat *colorTransformValues = context->clampedColorTransformValues;
			paintDesc->ctGlColor[AM_A] = (colorTransformValues[3] <= 0.0f) ? 0.0f : colorTransformValues[AM_A];
			paintDesc->ctGlColor[AM_R] = (colorTransformValues[0] <= 0.0f) ? 0.0f : colorTransformValues[AM_R] * paintDesc->ctGlColor[AM_A];
			paintDesc->ctGlColor[AM_G] = (colorTransformValues[1] <= 0.0f) ? 0.0f : colorTransformValues[AM_G] * paintDesc->ctGlColor[AM_A];
			paintDesc->ctGlColor[AM_B] = (colorTransformValues[2] <= 0.0f) ? 0.0f : colorTransformValues[AM_B] * paintDesc->ctGlColor[AM_A];
			ctUseGlColor = AM_TRUE;
		}
		else {
	// set the default global GL color
	paintDesc->ctGlColor[AM_R] = 1.0f;
	paintDesc->ctGlColor[AM_G] = 1.0f;
	paintDesc->ctGlColor[AM_B] = 1.0f;
	paintDesc->ctGlColor[AM_A] = 1.0f;
			ctUseGlColor = AM_FALSE;
				}
			}
			else {
		// set the default global GL color
		paintDesc->ctGlColor[AM_R] = 1.0f;
		paintDesc->ctGlColor[AM_G] = 1.0f;
		paintDesc->ctGlColor[AM_B] = 1.0f;
		paintDesc->ctGlColor[AM_A] = 1.0f;
		ctUseGlColor = AM_FALSE;
	}

	if (paintDesc->image->drawImageTextureValid &&
		paintDesc->image->drawImageTexturePixelsFormat == imgRequiredPixelsFormat &&
		((paintDesc->imageMode == VG_DRAW_IMAGE_STENCIL && paintDesc->image->drawImageTexturePixelsAreStencil) ||
		(paintDesc->imageMode != VG_DRAW_IMAGE_STENCIL && !paintDesc->image->drawImageTexturePixelsAreStencil))) {
		if (ctUseGlColor)
			return (paintDesc->image->ctDrawImageTextureHash == AM_COLOR_TRANSFORM_IDENTITY_HASH) ? AM_FALSE : AM_TRUE;
		else
			// color transformation is realized inside the texture
			return (paintDesc->image->ctDrawImageTextureHash == ctReferenceHash) ? AM_FALSE : AM_TRUE;
				}
	else
				return AM_TRUE;
		#else
	(void)paintDesc;
	(void)ctReferenceHash;
	(void)context;
	(void)srfFormat;
	return (paintDesc->image->drawImageTextureValid &&
			paintDesc->image->drawImageTexturePixelsFormat == imgRequiredPixelsFormat &&
			((paintDesc->imageMode == VG_DRAW_IMAGE_STENCIL && paintDesc->image->drawImageTexturePixelsAreStencil) ||
			(paintDesc->imageMode != VG_DRAW_IMAGE_STENCIL && !paintDesc->image->drawImageTexturePixelsAreStencil))) ? AM_FALSE : AM_TRUE;
		#endif
		}

AMbool amGlImageUpdate(AMPaintDesc *paintDesc,
							  AMContext *context,
							  AMDrawingSurface *surface) {

	VGImageFormat imgRequiredPixelsFormat = amGlImageRequiredFormat(paintDesc, surface);
	VGImageFormat srfFormat = (VGImageFormat)amSrfFormat32Get(surface);
	AMuint32 imgColorTransformHash;
	AMfloat *colorTransformValues;

	AM_ASSERT(paintDesc);
	AM_ASSERT(paintDesc->image);
	AM_ASSERT(context);

#if (AM_OPENVG_VERSION >= 110)
	if (paintDesc->imageMode == VG_DRAW_IMAGE_NORMAL) {

		AMuint32 imgFlags = pxlFormatTable[AM_FMT_GET_INDEX(paintDesc->image->format)][FMT_FLAGS];
		AMuint32 srfFlags = pxlFormatTable[AM_FMT_GET_INDEX(srfFormat)][FMT_FLAGS];

		imgColorTransformHash = (context->ctNormalizedValues && (imgFlags & FMT_L) == (srfFlags & FMT_L)) ? AM_COLOR_TRANSFORM_IDENTITY_HASH : context->colorTransformHash;
	}
	else
		imgColorTransformHash = AM_COLOR_TRANSFORM_IDENTITY_HASH;
	colorTransformValues = context->clampedColorTransformValues;
#else
	imgColorTransformHash = AM_COLOR_TRANSFORM_IDENTITY_HASH;
	colorTransformValues = NULL;
#endif

	// update texture used by vgDrawImage, if needed
	if (amImageTexturesNeedsUpload(paintDesc, imgRequiredPixelsFormat, imgColorTransformHash, context, srfFormat)) {

		AMbool opaquePixels = amImageIsOpaque(paintDesc->image);

		if (paintDesc->imageMode == VG_DRAW_IMAGE_STENCIL ||
			// scale * 1 + bias < 1
			(paintDesc->imageMode == VG_DRAW_IMAGE_NORMAL && colorTransformValues && (colorTransformValues[3] < 1.0f - colorTransformValues[7])))
			opaquePixels = AM_FALSE;
	
		if (!amGlImageTextureUpdate(paintDesc, context, imgColorTransformHash, imgRequiredPixelsFormat, opaquePixels))
			return AM_FALSE;
	}
	return AM_TRUE;
}
#endif

/*!
	\brief Update a paint descriptor, used by vgDrawImage function.
	\param paintDesc paint descriptor to update.
	\param _context context containing the color transform values (as well the GL context for AmanithVG GLE).
	\param surface drawing surface containing the alpha mask.
*/
AMbool amPaintDescImageUpdate(AMPaintDesc *paintDesc,
							  void *_context,
							  AMDrawingSurface *surface) {

	AMContext *context = (AMContext *)_context;
	AMbool res;

	AM_ASSERT(paintDesc);
	AM_ASSERT(paintDesc->image);
	AM_ASSERT(context);

	// update paint
	res = AM_TRUE;
	switch (paintDesc->imageMode) {

		case VG_DRAW_IMAGE_NORMAL:
			// create alpha mask if masking is on, and make sure that alpha mask texture (GL) is updated
			if (context->masking == VG_TRUE) {
			#if defined(AM_GLE) || defined(AM_GLS)
				if (!surface->alphaMaskTextureValid && surface->alphaMaskPixels) {
					if (!amAlphaMaskTextureUpdate(surface, context))
						return AM_FALSE;
				}
			#endif
			}
			break;

		case VG_DRAW_IMAGE_MULTIPLY:
			// it takes care also of alpha mask update too
			res = amPaintDescPaintUpdate(paintDesc, context, surface, AM_COLOR_TRANSFORM_IDENTITY_HASH);
			break;

		default:
			AM_ASSERT(paintDesc->imageMode == VG_DRAW_IMAGE_STENCIL);
			// it takes care also of alpha mask update too
			res = amPaintDescPaintUpdate(paintDesc, context, surface,
								#if (AM_OPENVG_VERSION >= 110)
									context->colorTransformHash
								#else
									AM_COLOR_TRANSFORM_IDENTITY_HASH 
								#endif
								);
			break;
	}

	if (!res)
		return AM_FALSE;

	// update image
#if defined(AM_GLE) || defined(AM_GLS)
	return amGlImageUpdate(paintDesc, context, surface);
#else
	return AM_TRUE;
#endif
}

/*!
	\brief Clamp the given image quality against a set of allowed qualities.
	\param quality input image quality to clamp.
	\param allowedQuality the set of allowed qualities.
	\return clamped image quality.
*/
VGImageQuality amImageQualityClamp(const VGImageQuality quality,
								   const VGbitfield allowedQuality) {

	VGImageQuality res;

	switch (quality) {
		case VG_IMAGE_QUALITY_BETTER:
			if (allowedQuality & VG_IMAGE_QUALITY_BETTER)
				res = VG_IMAGE_QUALITY_BETTER;
			else
			if (allowedQuality & VG_IMAGE_QUALITY_FASTER)
				res = VG_IMAGE_QUALITY_FASTER;
			else
				res = VG_IMAGE_QUALITY_NONANTIALIASED;
			break;
		case VG_IMAGE_QUALITY_FASTER:
			if (allowedQuality & VG_IMAGE_QUALITY_FASTER)
				res = VG_IMAGE_QUALITY_FASTER;
			else
				res = VG_IMAGE_QUALITY_NONANTIALIASED;
			break;
		default:
			res = VG_IMAGE_QUALITY_NONANTIALIASED;
			break;
	}
	return res;
}

/*!
	\brief Get a paint descriptor for a path fill.
	\param paintDesc output paint descriptor.
	\param _context context containing default paints, list of created handles, color transform values.
	\param surface drawing surface containing the alpha mask.
	\param userToSurfaceDesc descriptor for the "user to surface" matrix that will be applied during rendering operations for paths.
	\param paintModes paint modes used by the calling amDrawPath function; used to downgrade rendering quality when possible.
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
*/
AMbool amPaintDescPathFillGet(AMPaintDesc *paintDesc,
							  void *_context,
							  AMDrawingSurface *surface,
							  AMUserToSurfaceDesc *userToSurfaceDesc,
							  const VGbitfield paintModes) {

	VGHandle handle;
	AMImage *pat;
	AMContext *context = (AMContext *)_context;
	AMContextHandlesList *handles = context->handles;

	AM_ASSERT(paintDesc);
	AM_ASSERT(context);

	handle = (context->fillPaint) ? context->fillPaint : context->defaultPaint;
	paintDesc->paint = (AMPaint *)handles->createdHandlesList.data[handle];
	AM_ASSERT(paintDesc->paint);
	pat = (AMImage *)handles->createdHandlesList.data[paintDesc->paint->pattern];

	paintDesc->paintType = paintDesc->paint->paintType;
	paintDesc->blendMode = context->fillBlendMode;
	paintDesc->paintColor[AM_R] = paintDesc->paint->sColor[AM_R];
	paintDesc->paintColor[AM_G] = paintDesc->paint->sColor[AM_G];
	paintDesc->paintColor[AM_B] = paintDesc->paint->sColor[AM_B];
	paintDesc->paintColor[AM_A] = paintDesc->paint->sColor[AM_A];

	// according to OpenVG specifications:
	// "If the current paint object has its VG_PAINT_TYPE parameter set to VG_PAINT_TYPE_PATTERN, but no pattern
	// image is set, the paint object behaves as if VG_PAINT_TYPE were set to VG_PAINT_TYPE_COLOR"
	if (paintDesc->paintType == VG_PAINT_TYPE_PATTERN) {
		if (paintDesc->paint->pattern == VG_INVALID_HANDLE)
			paintDesc->paintType = VG_PAINT_TYPE_COLOR;
		else
			// define the pattern quality to use during drawing operation
			paintDesc->patternQuality = amImageQualityClamp(context->imageQuality, pat->allowedQuality);
	}

#if defined(VG_MZT_advanced_blend_modes)
	if (paintDesc->blendMode == VG_BLEND_CLEAR_MZT) {
		paintDesc->paintType = VG_PAINT_TYPE_COLOR;
		paintDesc->paintColor[AM_R] = 0.0f;
		paintDesc->paintColor[AM_G] = 0.0f;
		paintDesc->paintColor[AM_B] = 0.0f;
		paintDesc->paintColor[AM_A] = 0.0f;
	}
#endif

	paintDesc->image = NULL;
	paintDesc->imageMode = VG_DRAW_IMAGE_NORMAL;
	paintDesc->fillRule = context->fillRule;
	paintDesc->userToSurfaceDesc = userToSurfaceDesc;
	paintDesc->paintToUser = &context->fillPaintToUser;
	paintDesc->invPaintToUser = &context->inverseFillPaintToUser;
	paintDesc->masking = (context->masking == VG_TRUE && surface->alphaMaskPixels) ? AM_TRUE : AM_FALSE;
	paintDesc->renderingQuality = context->renderingQuality;

#if (AM_OPENVG_VERSION >= 110)
	paintDesc->colorTransform = (context->colorTransform == VG_TRUE && context->colorTransformHash != AM_COLOR_TRANSFORM_IDENTITY_HASH) ? AM_TRUE : AM_FALSE;
#endif
	if (!amPaintDescPathUpdate(paintDesc, context, surface))
		return AM_FALSE;

	amBlendModePathPatch(paintDesc, context);

	// if the stroke is going to be drawn
	if (paintModes & VG_STROKE_PATH && context->strokeLineWidth > 0.0f) {

		AMPaintDesc strokePaintDesc;
		
		amPaintDescPathStrokeGet(&strokePaintDesc, context, surface, userToSurfaceDesc);
		
		// for opaque non-dashed stroking, try to downgrade rendering quality for the fill, only if the stroke covers it
		if (strokePaintDesc.blendMode == VG_BLEND_SRC && context->patchedDashPattern.size == 0) {

			AMfloat maxScale = AM_MAX(context->pathUserToSurfaceScale[0], context->pathUserToSurfaceScale[1]);
			if (maxScale * context->strokeLineWidth > 3.0f)
				paintDesc->renderingQuality = VG_RENDERING_QUALITY_NONANTIALIASED;
			else
			if (maxScale * context->strokeLineWidth > 2.0f) {
				if (context->renderingQuality == VG_RENDERING_QUALITY_BETTER)
					paintDesc->renderingQuality = VG_RENDERING_QUALITY_FASTER;
			}
		}
	}

#if defined(AM_GLE)
	paintDesc->alphaMaskReplace = amGlePathAlphaMaskReplaceRequired(paintDesc);
	if (paintDesc->alphaMaskReplace) {
		// in this case disable masking, in order to perform the blending; masking will be re-activated to perform the replace pass (using the grab portion)
		paintDesc->masking = AM_FALSE;
		// in this case, to perform alpha masking correctly, we need 2 texture units
		if (context->glContext.textureUnitsCount < 2)
			paintDesc->alphaMaskReplace = AM_FALSE;
	}
	else {
		// masking (with a single texture unit) can be performed with color paint type only
		if (paintDesc->masking && paintDesc->paintType != VG_PAINT_TYPE_COLOR && context->glContext.textureUnitsCount < 2)
			paintDesc->masking = AM_FALSE;
	}

	paintDesc->multiplePass = amGlePathDoublePassRequired(paintDesc, &context->glContext);
	paintDesc->minmaxBlendGrab = amGlePathMinMaxBlendGrabRequired(paintDesc, context);
	paintDesc->imgMultiplySoftware = AM_FALSE;
#elif defined(AM_GLS)
	// TO DO
#endif
	return AM_TRUE;
}

/*!
	\brief Get a paint descriptor for a path stroke.
	\param paintDesc output paint descriptor.
	\param _context context containing default paints, list of created handles, color transform values.
	\param surface drawing surface containing the alpha mask.
	\param userToSurfaceDesc descriptor for the "user to surface" matrix that will be applied during rendering operations for paths.
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
*/
AMbool amPaintDescPathStrokeGet(AMPaintDesc *paintDesc,
								void *_context,
								AMDrawingSurface *surface,
								AMUserToSurfaceDesc *userToSurfaceDesc) {

	VGHandle handle;
	AMImage *pat;
	AMContext *context = (AMContext *)_context;
	AMContextHandlesList *handles = context->handles;

	AM_ASSERT(paintDesc);
	AM_ASSERT(context);

	handle = (context->strokePaint) ? context->strokePaint : context->defaultPaint;
	paintDesc->paint = (AMPaint *)handles->createdHandlesList.data[handle];
	AM_ASSERT(paintDesc->paint);
	pat = (AMImage *)handles->createdHandlesList.data[paintDesc->paint->pattern];

	paintDesc->paintType = paintDesc->paint->paintType;
	paintDesc->blendMode = context->strokeBlendMode;
	paintDesc->paintColor[AM_R] = paintDesc->paint->sColor[AM_R];
	paintDesc->paintColor[AM_G] = paintDesc->paint->sColor[AM_G];
	paintDesc->paintColor[AM_B] = paintDesc->paint->sColor[AM_B];
	paintDesc->paintColor[AM_A] = paintDesc->paint->sColor[AM_A];

	// according to OpenVG specifications:
	// "If the current paint object has its VG_PAINT_TYPE parameter set to VG_PAINT_TYPE_PATTERN, but no pattern
	// image is set, the paint object behaves as if VG_PAINT_TYPE were set to VG_PAINT_TYPE_COLOR"
	if (paintDesc->paintType == VG_PAINT_TYPE_PATTERN) {
		if (paintDesc->paint->pattern == VG_INVALID_HANDLE)
			paintDesc->paintType = VG_PAINT_TYPE_COLOR;
		else
			// define the pattern quality to use during drawing operation
			paintDesc->patternQuality = amImageQualityClamp(context->imageQuality, pat->allowedQuality);
	}

#if defined(VG_MZT_advanced_blend_modes)
	if (paintDesc->blendMode == VG_BLEND_CLEAR_MZT) {
		paintDesc->paintType = VG_PAINT_TYPE_COLOR;
		paintDesc->paintColor[AM_R] = 0.0f;
		paintDesc->paintColor[AM_G] = 0.0f;
		paintDesc->paintColor[AM_B] = 0.0f;
		paintDesc->paintColor[AM_A] = 0.0f;
	}
#endif

	paintDesc->image = NULL;
	paintDesc->imageMode = VG_DRAW_IMAGE_NORMAL;
	paintDesc->fillRule = VG_NON_ZERO;
	paintDesc->userToSurfaceDesc = userToSurfaceDesc;
	paintDesc->paintToUser = &context->strokePaintToUser;
	paintDesc->invPaintToUser = &context->inverseStrokePaintToUser;
	paintDesc->masking = (context->masking == VG_TRUE && surface->alphaMaskPixels) ? AM_TRUE : AM_FALSE;
	paintDesc->renderingQuality = context->renderingQuality;

#if (AM_OPENVG_VERSION >= 110)
	paintDesc->colorTransform = (context->colorTransform == VG_TRUE && context->colorTransformHash != AM_COLOR_TRANSFORM_IDENTITY_HASH) ? AM_TRUE : AM_FALSE;
#endif
	if (!amPaintDescPathUpdate(paintDesc, context, surface))
		return AM_FALSE;

	amBlendModePathPatch(paintDesc, context);

#if defined(AM_GLE)
	paintDesc->alphaMaskReplace = amGlePathAlphaMaskReplaceRequired(paintDesc);
	if (paintDesc->alphaMaskReplace) {
		// in this case disable masking, in order to perform the blending; masking will be re-activated to perform the replace pass (using the grab portion)
		paintDesc->masking = AM_FALSE;
		// in this case, to perform alpha masking correctly, we need 2 texture units
		if (context->glContext.textureUnitsCount < 2)
			paintDesc->alphaMaskReplace = AM_FALSE;
	}
	else {
		// masking (with a single texture unit) can be performed with color paint type only
		if (paintDesc->masking && paintDesc->paintType != VG_PAINT_TYPE_COLOR && context->glContext.textureUnitsCount < 2)
			paintDesc->masking = AM_FALSE;
	}

	paintDesc->multiplePass = amGlePathDoublePassRequired(paintDesc, &context->glContext);
	paintDesc->minmaxBlendGrab = amGlePathMinMaxBlendGrabRequired(paintDesc, context);
	paintDesc->imgMultiplySoftware = AM_FALSE;
#elif defined(AM_GLS)
	// TO DO
#endif
	return AM_TRUE;
}

/*!
	\brief Get a paint descriptor for an image.
	\param paintDesc output paint descriptor.
	\param _context context containing default paints, list of created handles, color transform values.
	\param surface drawing surface containing the alpha mask.
	\param userToSurfaceDesc descriptor for the "user to surface" matrix that will be applied during rendering operations, for the specifed image.
	\param image image to draw.
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
*/
AMbool amPaintDescImageFillGet(AMPaintDesc *paintDesc,
							   void *_context,
							   AMDrawingSurface *surface,
							   AMUserToSurfaceDesc *userToSurfaceDesc,
							   AMImage *image) {

	VGHandle handle;
	AMImage *pat;
#if defined(AM_SRE)
	AMfloat a00, a01, a10, a11;
#endif
	AMContext *context = (AMContext *)_context;
	AMContextHandlesList *handles = context->handles;

	AM_ASSERT(paintDesc);
	AM_ASSERT(context);
	AM_ASSERT(userToSurfaceDesc);
	AM_ASSERT(image);

	handle = (context->fillPaint) ? context->fillPaint : context->defaultPaint;
	paintDesc->paint = (AMPaint *)handles->createdHandlesList.data[handle];
	AM_ASSERT(paintDesc->paint);
	pat = (AMImage *)handles->createdHandlesList.data[paintDesc->paint->pattern];

	paintDesc->paintType = paintDesc->paint->paintType;
	paintDesc->blendMode = context->fillBlendMode;
	paintDesc->paintColor[AM_R] = paintDesc->paint->sColor[AM_R];
	paintDesc->paintColor[AM_G] = paintDesc->paint->sColor[AM_G];
	paintDesc->paintColor[AM_B] = paintDesc->paint->sColor[AM_B];
	paintDesc->paintColor[AM_A] = paintDesc->paint->sColor[AM_A];

	// according to OpenVG specifications:
	// "If the current paint object has its VG_PAINT_TYPE parameter set to VG_PAINT_TYPE_PATTERN, but no pattern
	// image is set, the paint object behaves as if VG_PAINT_TYPE were set to VG_PAINT_TYPE_COLOR"
	if (paintDesc->paintType == VG_PAINT_TYPE_PATTERN && paintDesc->paint->pattern == VG_INVALID_HANDLE)
		paintDesc->paintType = VG_PAINT_TYPE_COLOR;

#if defined(VG_MZT_advanced_blend_modes)
	if (paintDesc->blendMode == VG_BLEND_CLEAR_MZT) {
		paintDesc->paintType = VG_PAINT_TYPE_COLOR;
		// patch imageMode
		paintDesc->imageMode = VG_DRAW_IMAGE_MULTIPLY;
		paintDesc->paintColor[AM_R] = 0.0f;
		paintDesc->paintColor[AM_G] = 0.0f;
		paintDesc->paintColor[AM_B] = 0.0f;
		paintDesc->paintColor[AM_A] = 0.0f;
	}
#endif

	paintDesc->userToSurfaceDesc = userToSurfaceDesc;

	// NB: for AmanithVG SRE version, behavior is always the one specified in OpenVG 1.0.1:
	// "when a projective transformation is enabled, vgDrawImage always uses VG_DRAW_IMAGE_NORMAL mode"
	// AmanithVG GLE is compatible with the behavior of the OpenVG 1.0 (where projective transformation
	// are allowed for image modes different than VG_DRAW_IMAGE_NORMAL)
	paintDesc->imageMode = context->imageMode;

	if (paintDesc->imageMode != VG_DRAW_IMAGE_NORMAL) {
		
		if (!paintDesc->userToSurfaceDesc->userToSurfaceAffine)
			paintDesc->imageMode = VG_DRAW_IMAGE_NORMAL;
		else {
			// define the pattern quality to use during rendering
			if (paintDesc->paintType == VG_PAINT_TYPE_PATTERN)
				paintDesc->patternQuality = amImageQualityClamp(context->imageQuality, pat->allowedQuality);
		}
	#if defined(AM_GLE)
		// OpenGL / OpenGL ES 1.0+ fallback in the case of unsupported combiners or dot3: STENCIL ---> MULTIPLY
		if (paintDesc->imageMode == VG_DRAW_IMAGE_STENCIL &&
			(!context->glContext.combinersSupported || !context->glContext.dot3Supported || context->glContext.textureUnitsCount < 2))
			paintDesc->imageMode = VG_DRAW_IMAGE_MULTIPLY;

		if (context->glContext.textureUnitsCount < 2 && paintDesc->paintType != VG_PAINT_TYPE_COLOR)
			paintDesc->imageMode = VG_DRAW_IMAGE_NORMAL;
	#endif
	}

	paintDesc->image = image;
	paintDesc->fillRule = VG_EVEN_ODD;
	// define the image quality to use during drawing operation
	paintDesc->imageQuality = amImageQualityClamp(context->imageQuality, image->allowedQuality);

	paintDesc->paintToUser = &context->fillPaintToUser;
	paintDesc->invPaintToUser = &context->inverseFillPaintToUser;
	paintDesc->masking = (context->masking == VG_TRUE && surface->alphaMaskPixels) ? AM_TRUE : AM_FALSE;
	paintDesc->renderingQuality = context->renderingQuality;
#if (AM_OPENVG_VERSION >= 110)
	paintDesc->colorTransform = (context->colorTransform == VG_TRUE && context->colorTransformHash != AM_COLOR_TRANSFORM_IDENTITY_HASH) ? AM_TRUE : AM_FALSE;
#endif

#if defined(AM_SRE)
	// patch paintDesc->imageQuality: check for a rotation multiple of PI / 2
	a00 = userToSurfaceDesc->userToSurface->a[0][0];
	a01 = userToSurfaceDesc->userToSurface->a[0][1];
	a10 = userToSurfaceDesc->userToSurface->a[1][0];
	a11 = userToSurfaceDesc->userToSurface->a[1][1];

	if (paintDesc->imageQuality != VG_IMAGE_QUALITY_NONANTIALIASED &&
		((amAbsf( 1.0f - a00) <= AM_EPSILON_FLOAT && amAbsf(        a01) <= AM_EPSILON_FLOAT && amAbsf(        a10) <= AM_EPSILON_FLOAT && amAbsf( 1.0f - a11) <= AM_EPSILON_FLOAT) ||
		 (amAbsf(-1.0f - a00) <= AM_EPSILON_FLOAT && amAbsf(        a01) <= AM_EPSILON_FLOAT && amAbsf(        a10) <= AM_EPSILON_FLOAT && amAbsf(-1.0f - a11) <= AM_EPSILON_FLOAT) ||
		 (amAbsf(        a00) <= AM_EPSILON_FLOAT && amAbsf( 1.0f - a01) <= AM_EPSILON_FLOAT && amAbsf(-1.0f - a10) <= AM_EPSILON_FLOAT && amAbsf(        a11) <= AM_EPSILON_FLOAT) ||
		 (amAbsf(        a00) <= AM_EPSILON_FLOAT && amAbsf(-1.0f - a01) <= AM_EPSILON_FLOAT && amAbsf( 1.0f - a10) <= AM_EPSILON_FLOAT && amAbsf(        a11) <= AM_EPSILON_FLOAT)) &&
		 // then check for affinity
		 amAbsf(       userToSurfaceDesc->userToSurface->a[2][0]) <= AM_EPSILON_FLOAT &&
		 amAbsf(       userToSurfaceDesc->userToSurface->a[2][1]) <= AM_EPSILON_FLOAT &&
		 amAbsf(1.0f - userToSurfaceDesc->userToSurface->a[2][2]) <= AM_EPSILON_FLOAT) {

		// subPixelPrecision = 1 / 64
		const AMfloat subPixelPrecision = 0.015625f;
		AMint32 a02 = (AMint32)userToSurfaceDesc->userToSurface->a[0][2];
		AMint32 a12 = (AMint32)userToSurfaceDesc->userToSurface->a[1][2];

		// check for integer translations
		if (amAbsf((AMfloat)a02 - userToSurfaceDesc->userToSurface->a[0][2]) < subPixelPrecision &&
			amAbsf((AMfloat)a12 - userToSurfaceDesc->userToSurface->a[1][2]) < subPixelPrecision)
			// in this case bilinear filtering is not performed because not needed
			paintDesc->imageQuality = VG_IMAGE_QUALITY_NONANTIALIASED;
	}
#endif

#if defined(AM_GLE)
	paintDesc->imgMultiplySoftware = amGleImageMultiplySoftware(paintDesc, context, surface);
#endif

	if (!amPaintDescImageUpdate(paintDesc, context, surface))
		return AM_FALSE;

	amBlendModeImagePatch(paintDesc, context);

#if defined(AM_GLE)
	if (context->glContext.textureUnitsCount < 2)
		// in this case, the amGlePathAlphaMaskReplaceRequired call below will return AM_FALSE
		paintDesc->masking = AM_FALSE;

	paintDesc->alphaMaskReplace = amGleImageAlphaMaskReplaceRequired(paintDesc);
	if (paintDesc->alphaMaskReplace)
		// in this case disable masking, in order to perform the blending; masking will be re-activated to perform the replace pass (using the grab portion)
		paintDesc->masking = AM_FALSE;

	paintDesc->multiplePass = amGleImageDoublePassRequired(paintDesc);
	paintDesc->minmaxBlendGrab = amGleImageMinMaxBlendGrabRequired(paintDesc, context);
#elif defined(AM_GLS)
	// TO DO
#endif
	return AM_TRUE;
}

void amPaintDefaultValuesSet(AMPaint *paint) {

	// paint color
	paint->sColor[AM_R] = 0.0f;
	paint->sColor[AM_G] = 0.0f;
	paint->sColor[AM_B] = 0.0f;
	paint->sColor[AM_A] = 1.0f;
	// paint type
	paint->paintType = VG_PAINT_TYPE_COLOR;
	// paint color ramp spread mode
	paint->colorRampSpreadMode = VG_COLOR_RAMP_SPREAD_PAD;
#if defined(VG_MZT_color_ramp_interpolation)
	// paint color ramp interpolation type
	paint->colorRampInterpolationType = VG_COLOR_RAMP_INTERPOLATION_LINEAR_MZT;
#endif
	// paint tiling mode
	paint->tilingMode = VG_TILE_FILL;
	// invalidate texture flags
	paint->gradTexturesValid = AM_FALSE;
	// set color ramp premultiplied flag
	paint->colorRampPremultiplied = VG_TRUE;
	// set default min/max gradient alpha values
	paint->gradientMinAlpha = 1.0f;
	paint->gradientMaxAlpha = 1.0f;
	// paint linear gradient points
	paint->linGradPt0[AM_X] = 0.0f;
	paint->linGradPt0[AM_Y] = 0.0f;
	paint->linGradPt1[AM_X] = paint->patchedLinGradTarget.x = 1.0f;
	paint->linGradPt1[AM_Y] = paint->patchedLinGradTarget.y = 0.0f;
	// paint radial gradient center, focus and radius
	paint->radGradCenter[AM_X] = 0.0f;
	paint->radGradCenter[AM_Y] = 0.0f;
	paint->radGradFocus[AM_X] = paint->patchedRadGradFocus.x = 0.0f;
	paint->radGradFocus[AM_Y] = paint->patchedRadGradFocus.y = 0.0f;
	paint->radGradRadius = paint->patchedRadGradRadius = 1.0f;
#if defined(VG_MZT_conical_gradient)
	// paint conical gradient center and target
	paint->conGradPt0[AM_X] = 0.0f;
	paint->conGradPt0[AM_Y] = 0.0f;
	paint->conGradPt1[AM_X] = paint->patchedConGradTarget.x = 1.0f;
	paint->conGradPt1[AM_Y] = paint->patchedConGradTarget.y = 0.0f;
	paint->conGradRepeats = paint->patchedConGradRepeats = 1.0f;
#endif

	// texture validity flag
	paint->gradTexturesValid = AM_FALSE;
#if (AM_OPENVG_VERSION >= 110)
	// hash value
	paint->ctTexturesHash = AM_COLOR_TRANSFORM_IDENTITY_HASH;
#endif
}

/*!
	\brief Initialize a paint structure.
	\param paint paint to initialize.
*/
AMbool amPaintInit(AMPaint *paint) {

	AM_ASSERT(paint);

	// paint color stops
	AM_DYNARRAY_PREINIT(paint->sColorStops)

	paint->id = AM_PAINT_HANDLE_ID;
	paint->type = AM_PAINT_HANDLE_ID;
	paint->referenceCounter = 1;
	amPaintDefaultValuesSet(paint);

	// paint pattern image
	paint->pattern = 0;
	// initialize gradient textures
	amTextureInit(&paint->gradTexture);
	amTextureInit(&paint->reflectGradTexture);
	amTextureInit(&paint->gradTextureImgMultiply);
	amTextureInit(&paint->reflectGradTextureImgMultiply);
	return AM_TRUE;
}

void amPaintRewind(AMPaint *paint,
				   const AMContext *context) {

	AM_ASSERT(paint);

	(void)context;

	// paint color stops
	paint->sColorStops.size = 0;
	paint->id = AM_PAINT_HANDLE_ID;
	paint->type = AM_PAINT_HANDLE_ID;
	paint->referenceCounter = 1;
	amPaintDefaultValuesSet(paint);
	// paint pattern image
	paint->pattern = 0;
}

/*!
	\brief It replaces any previous pattern image defined on the given paint object for the given set of
	paint modes with a new pattern image. A	value of VG_INVALID_HANDLE for the pattern parameter removes
	the current pattern image from the paint object.
	\param paint the destination paint.
	\param pattern the pattern (handle) to set.
	\param _context context containing the list of created handles.
*/
void amPaintPatternSet(AMPaint *paint,
					   VGImage pattern,
					   void *_context) {

	AMContext *context = (AMContext *)_context;
	AMImage *newPat, *prevPat;
	AMContextHandlesList *handles = context->handles;

	AM_ASSERT(paint);
	AM_ASSERT(context);

	newPat = (AMImage *)handles->createdHandlesList.data[pattern];
	prevPat = (AMImage *)handles->createdHandlesList.data[paint->pattern];

	if (newPat) {
		// if paint has a previous pattern (different from the new one), we must decrement its reference counter
		if (prevPat) {
			if (prevPat != newPat) {
				amCtxHandleRefCounterInc(newPat);
				if (amCtxHandleRefCounterDec(prevPat, context) == 0) {
					amCtxHandleRemove(context, paint->pattern);
					amFree(prevPat);
				}
				paint->pattern = pattern;
			}
		}
		else {
			// set a new pattern, increasing its reference counter
			amCtxHandleRefCounterInc(newPat);
			paint->pattern = pattern;
		}
	}
	else {
		// if paint has a previous pattern, we must decrement its reference counter
		if (prevPat) {
			if (amCtxHandleRefCounterDec(prevPat, context) == 0) {
				amCtxHandleRemove(context, paint->pattern);
				amFree(prevPat);
			}
		}
		paint->pattern = 0;
	}
}

/*!
	\brief Destroy resources associated to the given paint.
	\param paint the paint whose resources must be destroyed.
	\param _context context containing the list of created handles.
*/
void amPaintResourcesDestroy(AMPaint *paint,
							 void *_context) {

	AM_ASSERT(paint);
	AM_ASSERT(_context);

	(void)_context;

	// delete gradient textures
	amTextureDestroy(&paint->gradTexture);
	amTextureDestroy(&paint->reflectGradTexture);
	amTextureDestroy(&paint->gradTextureImgMultiply);
	amTextureDestroy(&paint->reflectGradTextureImgMultiply);
	
	// destroy color stops arrays
	AM_DYNARRAY_DESTROY(paint->sColorStops)
}

/*!
	\brief Destroy a paint structure.
	\param paint paint to destroy.
	\param _context context containing the list of created handles.
*/
void amPaintDestroy(AMPaint *paint,
					void *_context) {

	AMContext *context = (AMContext *)_context;

	AM_ASSERT(paint);
	AM_ASSERT(paint->id == AM_PAINT_HANDLE_ID);

	// decrement reference counter, it possibly leads to resources deallocation
	amCtxHandleRefCounterDec(paint, context);
	// sign that this paint is not more valid
	paint->id = AM_INVALID_HANDLE_ID;
}

AMbool amPaintNew(AMPaint **paint,
				  VGPaint *handle,
				  void *_context) {

	AMContext *context = (AMContext *)_context;
	AMPaintsPoolsManager *paintsPoolsManager = &context->handles->paintsPools;
	AMPaintsPoolPtrDynArray *pools = &paintsPoolsManager->pools;
	AMbool mustBeInitialized;
	AMPaintsPool *pool;
	AMPaint *resPaint;
	VGPaint resHandle;

	AM_ASSERT(context);
	AM_ASSERT(paintsPoolsManager);

	// check if there are available handles
	if (paintsPoolsManager->availablePaintsList.size > 0) {

		AMPaintRef paintRef;

		mustBeInitialized = AM_FALSE;
		// extract an available handle descriptor (previously deleted)
		paintsPoolsManager->availablePaintsList.size--;
		paintRef = paintsPoolsManager->availablePaintsList.data[paintsPoolsManager->availablePaintsList.size];
		// extract the relative pool
		pool = paintsPoolsManager->pools.data[paintRef.c.pool];
		resPaint = &pool->data[paintRef.c.poolIdx];
	}
	else {
		mustBeInitialized = AM_TRUE;
		if (pools->size == 0 || pools->data[pools->size - 1]->size == AM_PAINTS_POOL_CAPACITY) {

			// try to allocate a new paints pool
			pool = (AMPaintsPool *)amMalloc(sizeof(AMPaintsPool));
			if (!pool)
				return AM_FALSE;

			AM_DYNARRAY_PUSH_BACK(*pools, AMPaintsPoolPtr, pool)
			if (pools->error) {
				pools->error = AM_DYNARRAY_NO_ERROR;
				// destroy the new allocated pool, if the "push back" operation failed
				amFree(pool);
				return AM_FALSE;
			}
			pool->size = 0;
		}
		else
			pool = pools->data[pools->size - 1];
		
		resPaint = &pool->data[pool->size++];
	}

	// initialize / rewind the paint
	if (mustBeInitialized) {
		// initialize paint
		if (!amPaintInit(resPaint)) {
			// remove the last element from the pool, and if empty free it
			AM_ASSERT(pool->size > 0);
			pool->size--;
			if (pool->size == 0) {
				amFree(pool);
				pools->size--;

			}
			return AM_FALSE;
		}
	}
	else
		// rewind the paint
		amPaintRewind(resPaint, context);

	// create the handle
	resHandle = amCtxHandleNew(context, (AMhandle)resPaint);
	if (resHandle == VG_INVALID_HANDLE) {
		// in order to avoid inconsistencies / assertion, reset the reference counter
		resPaint->referenceCounter = 0;
		// in this case the paint was taken from an hole
		if (!mustBeInitialized)
			paintsPoolsManager->availablePaintsList.size++;
		else {
			// paint was initialized, so its resources must be destroyed
			amPaintResourcesDestroy(resPaint, context);
			// remove the last element from the pool, and if empty free it
			AM_ASSERT(pool->size > 0);
			pool->size--;
			if (pool->size == 0) {
				amFree(pool);
				pools->size--;

			}
		}
		return AM_FALSE;
	}

	*paint = resPaint;
	*handle = resHandle;
	return AM_TRUE;
}

void amPaintRemoveRecover(AMContext *context,
						  AMPaint *paint,
						  AMPaintRef *paintRef) {

	AMContextHandlesList *handles = context->handles;
	AMHandleDynArray *createdHandlesList = &handles->createdHandlesList;
	AMPaintsPoolPtrDynArray *pools = &handles->paintsPools.pools;
	AMPaintRefDynArray *availablePaintsList = &handles->paintsPools.availablePaintsList;
	AMPaintsPool *lastPool = pools->data[pools->size - 1];

	AM_ASSERT(context);
	AM_ASSERT(paint && paint->referenceCounter == 0);
	AM_ASSERT(paintRef);
	AM_ASSERT(createdHandlesList->data[paint->vgHandle] == paint);

	createdHandlesList->data[paint->vgHandle] = NULL;
	AM_DYNARRAY_PUSH_BACK(handles->availableHandlesList, VGHandle, paint->vgHandle)
	paint->vgHandle = VG_INVALID_HANDLE;
	// if the was a memory error in the push back, we loose and handle entry, but we can do nothing
	if (handles->availableHandlesList.error)
		handles->availableHandlesList.error = AM_DYNARRAY_NO_ERROR;

	if (lastPool->data[lastPool->size - 1].referenceCounter > 0) {
		// link handle to the new position (in the pool)
		createdHandlesList->data[lastPool->data[lastPool->size - 1].vgHandle] = &pools->data[paintRef->c.pool]->data[paintRef->c.poolIdx];
		// if the last paint is referenced, we can move it to fill the hole created by the path remotion and we are done
		pools->data[paintRef->c.pool]->data[paintRef->c.poolIdx] = lastPool->data[lastPool->size - 1];
		lastPool->size--;
	}
	else {

		AMuint32 i;

		for (i = 0; i < availablePaintsList->size; ++i) {

			if (availablePaintsList->data[i].c.pool == pools->size - 1 && availablePaintsList->data[i].c.poolIdx == lastPool->size - 1) {
				lastPool->size--;
				availablePaintsList->data[i].key = paintRef->key;
				break;
			}
		}
		// paint must have been found inside the loop
		AM_ASSERT(i < availablePaintsList->size);
	}

	// destroy the resources associated with paint
	amPaintResourcesDestroy(paint, context);

	if (lastPool->size == 0) {
		amFree(lastPool);
		pools->size--;
	}
}

void amPaintRemove(void *_context,
				   AMPaint *paint) {

	AMContext *context = (AMContext *)_context;
	AMPaintsPoolsManager *paintsPoolsManager = &context->handles->paintsPools;
	AMuint32 i, j;

	AM_ASSERT(context);
	AM_ASSERT(paintsPoolsManager);
	AM_ASSERT(paint);
	AM_ASSERT(paint->referenceCounter == 0);
	AM_ASSERT(context->handles->createdHandlesList.data[paint->vgHandle] == paint);

	j = paintsPoolsManager->pools.size;
	for (i = 0; i < j; ++i) {

		AMPaintsPool *pool = paintsPoolsManager->pools.data[i];
		AMPaint *poolBase = pool->data;
		AMPaint *poolEnd = poolBase + AM_PAINTS_POOL_CAPACITY;

		// check if the handle belongs to the pool
		if (paint >= poolBase && paint < poolEnd) {

			AMPaintRef paintRef;

			paintRef.c.pool = i;
			paintRef.c.poolIdx = (AMuint16)(paint - poolBase);

			// copy the paint descriptor inside the list of available paint descriptors
			AM_DYNARRAY_PUSH_BACK(paintsPoolsManager->availablePaintsList, AMPaintRef, paintRef)
			if (paintsPoolsManager->availablePaintsList.error) {
				paintsPoolsManager->availablePaintsList.error = AM_DYNARRAY_NO_ERROR;
				amPaintRemoveRecover(context, paint, &paintRef);
			}
			else
				amCtxHandleRemove(context, paint->vgHandle);
			return;
		}
	}
	AM_ASSERT(0 == 1);
}

// *********************************************************************
//                        Public implementations
// *********************************************************************

/*!
	\brief It creates a new paint object that is initialized to a set of default values and returns a VGPaint
	handle to it. If insufficient memory is available to allocate a new object, VG_INVALID_HANDLE is returned.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\return a VGPaint handle, or VG_INVALID_HANDLE if memory errors occur.
*/
VG_API_CALL VGPaint VG_API_ENTRY vgCreatePaint(void) VG_API_EXIT {

	AMPaint *pnt;
	VGPaint handle;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgCreatePaint");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}

	// allocate the paint
	if (!amPaintNew(&pnt, &handle, currentContext)) {
		// try to retrieve as much memory as possbile
		amMemMngRetrieve(currentContext, AM_TRUE);
		// try to re-create the paint
		if (!amPaintNew(&pnt, &handle, currentContext)) {	
		#if defined(AM_DEBUG_MEMORY)
			amCtxCheckConsistence(currentContext);
		#endif
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
			AM_MEMORY_LOG("vgCreatePaint  (amPaintNew fail)");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}
	}

#if defined(AM_DEBUG_MEMORY)
	amCtxCheckConsistence(currentContext);
#endif

	// decrement the counter of the memory manager
	amCtxMemMngCountDown(currentContext);
	// exit without errors
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgCreatePaint");
	OPENVG_RETURN(handle)
}

/*!
	\brief Destroy a paint, possibly deallocating associated resources. Following the call, the paint handle
	is no longer valid in any of the contexts that shared it. If the paint object is currently active in a
	drawing context, the context continues to access it until it is replaced or the context is destroyed.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param paint the paint to destroy.
*/
VG_API_CALL void VG_API_ENTRY vgDestroyPaint(VGPaint paint) VG_API_EXIT {

	AMPaint *pnt;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgDestroyPaint");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for bad handle
	if (amCtxHandleValid(currentContext, paint) != AM_PAINT_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgDestroyPaint");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	pnt = (AMPaint *)currentContext->handles->createdHandlesList.data[paint];
	AM_ASSERT(pnt);

#if defined(AM_DEBUG_MEMORY)
	amCtxCheckConsistence(currentContext);
#endif
	amPaintDestroy(pnt, currentContext);

	if (pnt->referenceCounter == 0) {
		// remove a possible reference to pattern image
		amPaintPatternSet(pnt, 0, currentContext);
		// remove paint object from context internal list
		amPaintRemove(currentContext, pnt);
	}

	// decrement the counter of the memory manager
	amCtxMemMngCountDown(currentContext);
	// exit without errors
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgDestroyPaint");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief Set a paint inside the current context.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param paint the paint to set.
	\param paintModes a bitwise OR of values from the VGPaintMode enumeration, determining whether the paint
	object is to be used for filling (VG_FILL_PATH), stroking (VG_STROKE_PATH), or both (VG_FILL_PATH |
	VG_STROKE_PATH).\n
	For more information, please refer to the official \b OpenVG \b specifications document.
*/
VG_API_CALL void VG_API_ENTRY vgSetPaint(VGPaint paint,
                                         VGbitfield paintModes) VG_API_EXIT {

	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgSetPaint");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for bad handle
	if (paint) {
		if (amCtxHandleValid(currentContext, paint) != AM_PAINT_HANDLE_ID) {
			amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
			AM_MEMORY_LOG("vgSetPaint");
			OPENVG_RETURN(OPENVG_NO_RETVAL)
		}
	}

	// check for illegal arguments
	if (paintModes == 0 || paintModes > (VG_STROKE_PATH | VG_FILL_PATH)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgSetPaint");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// NULL values are allowed
	amPaintSet(currentContext, paint, paintModes);
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgSetPaint");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief It returns the paint object currently set for the given paintMode, or VG_INVALID_HANDLE if an
	error occurs or if no paint object is set (i.e., the default paint is present) on the current context
	with the given paintMode.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param paintMode input paint mode.
	\return the requested paint handle or VG_INVALID_HANDLE if an error occurs.
*/
VG_API_CALL VGPaint VG_API_ENTRY vgGetPaint(VGPaintMode paintMode) {

	VGPaint res;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgGetPaint");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}

	// check for illegal arguments
	if (paintMode != VG_STROKE_PATH && paintMode != VG_FILL_PATH) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgGetPaint");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}

	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgGetPaint");
#if !defined ( RIM_VG_SRC )
	res = (paintMode == VG_STROKE_PATH) ? currentContext->strokePaint : currentContext->fillPaint;
#else
   res = (paintMode == VG_STROKE_PATH) ? (VGPaint)(currentContext->strokePaint) : (VGPaint)(currentContext->fillPaint);
#endif
	OPENVG_RETURN(res)
}

/*!
	\brief It allows the VG_PAINT_COLOR parameter of a given paint object to be set using a 32-bit
	non-premultiplied sRGBA_8888 representation.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param paint the destination paint.
	\param rgba an unsigned integer with 8 bits of red starting at the most significant bit, followed by 8 bits
	each of green, blue, and alpha.
*/
VG_API_CALL void VG_API_ENTRY vgSetColor(VGPaint paint,
                                         VGuint rgba) VG_API_EXIT {

	AMfloat sCol[4];
	AMPaint *pnt;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgSetColor");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for bad handle
	if (amCtxHandleValid(currentContext, paint) != AM_PAINT_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgSetColor");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	pnt = (AMPaint *)currentContext->handles->createdHandlesList.data[paint];
	AM_ASSERT(pnt);

	sCol[0] = ((AMfloat)((rgba >> 24) & 0xFF)) / 255.0f;
	sCol[1] = ((AMfloat)((rgba >> 16) & 0xFF)) / 255.0f;
	sCol[2] = ((AMfloat)((rgba >> 8) & 0xFF)) / 255.0f;
	sCol[3] = ((AMfloat)(rgba & 0xFF)) / 255.0f;
	amPaintColorSet(pnt, sCol);
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgSetColor");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief It retrieves the current setting of the VG_PAINT_COLOR parameter on a given paint
	object as a 32-bit non-premultiplied sRGBA_8888 value.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param paint the paint whose VG_PAINT_COLOR parameter is to get.
	\return the VG_PAINT_COLOR parameter as a 32-bit non-premultiplied sRGBA_8888 value.
*/
VG_API_CALL VGuint VG_API_ENTRY vgGetColor(VGPaint paint) VG_API_EXIT {

	const AMPaint *pnt;
	VGuint r, g, b, a;
	AMfloat tmp;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgGetColor");
		OPENVG_RETURN(0xFF)
	}

	// check for bad handle
	if (amCtxHandleValid(currentContext, paint) != AM_PAINT_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgGetColor");
		OPENVG_RETURN(0xFF)
	}

	pnt = (const AMPaint *)currentContext->handles->createdHandlesList.data[paint];
	AM_ASSERT(pnt);

	tmp = AM_CLAMP(pnt->sColor[0], 0.0f, 1.0f);
	r = (VGuint)amFloorf(tmp * 255.0f + 0.5f);
	tmp = AM_CLAMP(pnt->sColor[1], 0.0f, 1.0f);
	g = (VGuint)amFloorf(tmp * 255.0f + 0.5f);
	tmp = AM_CLAMP(pnt->sColor[2], 0.0f, 1.0f);
	b = (VGuint)amFloorf(tmp * 255.0f + 0.5f);
	tmp = AM_CLAMP(pnt->sColor[3], 0.0f, 1.0f);
	a = (VGuint)amFloorf(tmp * 255.0f + 0.5f);

	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgGetColor");
	OPENVG_RETURN(((r << 24) | (g << 16) | (b << 8) | a))
}

/*!
	\brief It replaces any previous pattern image defined on the given paint object for the given set of
	paint modes with a new pattern image. A	value of VG_INVALID_HANDLE for the pattern parameter removes
	the current pattern image from the paint object.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param paint the destination paint.
	\param pattern the pattern to set.
*/
VG_API_CALL void VG_API_ENTRY vgPaintPattern(VGPaint paint,
                                             VGImage pattern) VG_API_EXIT {

	AMPaint *pnt;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgPaintPattern");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for bad handles
	if (amCtxHandleValid(currentContext, paint) != AM_PAINT_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgPaintPattern");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if (pattern) {
		if (amCtxHandleValid(currentContext, pattern) != AM_IMAGE_HANDLE_ID) {
			amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
			AM_MEMORY_LOG("vgPaintPattern");
			OPENVG_RETURN(OPENVG_NO_RETVAL)
		}

	#if !defined(AM_STANDALONE) || defined(RIM_VG_SRC) || defined(TORCH_VG_SRC)
		{
			AMImage *pat = (AMImage *)currentContext->handles->createdHandlesList.data[pattern];

			// check if pattern is used by EGL
			AM_ASSERT(pat && pat->type == AM_IMAGE_HANDLE_ID);
			if (pat->inUseByEgl) {
				amCtxErrorSet(currentContext, VG_IMAGE_IN_USE_ERROR);
				AM_MEMORY_LOG("vgPaintPattern");
				OPENVG_RETURN(OPENVG_NO_RETVAL)
			}
		}
	#endif
	}

	pnt = (AMPaint *)currentContext->handles->createdHandlesList.data[paint];
	AM_ASSERT(pnt);

	amPaintPatternSet(pnt, pattern, currentContext);
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgPaintPattern");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

#if defined (RIM_VG_SRC)
#pragma pop
#endif

