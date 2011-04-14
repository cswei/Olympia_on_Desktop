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
	\file vgcontext.c
	\brief OpenVG context, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "vggradients.h"
#include "fillers.h"
#include "vgmask.h"
#include "svn_revision.h"
#if (AM_OPENVG_VERSION >= 110)
	#include "vgfont.h"
#endif
#if defined(AM_GLE) || defined(AM_GLS)
	#include "gl_abstraction.h"
#endif

#if defined RIM_VG_SRC
#define VG_API_ENTRY 
#endif

//! A threshold indicating the number of OpenVG calls (handles creation/destruction and drawing functions) to be done before to sort the list of available handles (inside the context).
#define AM_OPENVG_CALLS_BEFORE_AVAILABLE_HANDLES_SORT 64

/*!
	\brief Set image quality on the given context.
	\param context context where to set image quality.
	\param quality input image quality bitfield.
*/
void amCtxImageQualitySet(AMContext *context,
						  const VGint quality) {

	AM_ASSERT(context);

	if (quality & VG_IMAGE_QUALITY_BETTER)
		context->imageQuality = VG_IMAGE_QUALITY_BETTER;
	else
	if (quality & VG_IMAGE_QUALITY_FASTER)
		context->imageQuality = VG_IMAGE_QUALITY_FASTER;
	else
		context->imageQuality = VG_IMAGE_QUALITY_NONANTIALIASED;
}

/*!
	\brief Set rendering quality on the given context.
	\param context context where to set rendering quality.
	\param quality input rendering quality.
*/
void amCtxRenderingQualitySet(AMContext *context,
							  const VGRenderingQuality quality) {

	AM_ASSERT(context);

	context->renderingQuality = quality;
}

/*!
	\brief Set stroke line width on the given context.
	\param context context where to set stroke width.
	\param width input stroke width.
	\note some stroke derived data are recalculated.
*/
void amCtxStrokeWidthSet(AMContext *context,
						 const VGfloat width) {

	AM_ASSERT(context);

	context->strokeLineWidth = width;
	context->strokeLineThickness = context->strokeLineWidth / 2.0f;
	context->miterMulThickness = context->miterLimit * context->strokeLineThickness;
	context->miterMulThicknessSqr = context->miterMulThickness * context->miterMulThickness;
}

/*!
	\brief Set stroke miter limit on the given context.
	\param context context where to set miter limit.
	\param miterLimit input stroke miter limit.
	\note some stroke derived data are recalculated.
*/
void amCtxStrokeMiterLimitSet(AMContext *context,
							  const VGfloat miterLimit) {

	AM_ASSERT(context);

	if (miterLimit < 1.0f) {
		context->miterLimit = 1.0f;
		context->miterMulThickness = context->strokeLineThickness;
	}
	else {
		context->miterLimit = miterLimit;
		context->miterMulThickness = context->miterLimit * context->strokeLineThickness;
	}

	context->miterMulThicknessSqr = context->miterMulThickness * context->miterMulThickness;
}

/*!
	\brief Set stroke dash phase on the given context.
	\param context context where to set stroke dash phase.
	\param dashPhase input stroke dash phase.
*/
void amCtxStrokeDashPhaseSet(AMContext *context,
							 const VGfloat dashPhase) {

	AM_ASSERT(context);

	context->dashPhase = dashPhase;
}

/*!
	\brief Set stroke dash pattern on the given context (integer values).
	\param context context where to set stroke dash pattern.
	\param count number of dash values.
	\param values input integer dash values.
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
	\note some stroke derived data are recalculated.
*/
AMbool amCtxStrokeDashPatternSeti(AMContext *context,
								const VGint count,
								const VGint *values) {

	AM_ASSERT(count >= 0);

	if (count == 0) {
		AM_ASSERT(!values);
		// clear dash pattern array
		context->dashPattern.size = 0;
		context->patchedDashPattern.size = 0;
		context->dashPatternSum = 0.0f;
	}
	else {
		AMint32 i, j;

		if ((AMint32)context->dashPattern.capacity < count) {
			// allocate additional memory
			AM_DYNARRAY_CLEAR_RESERVE(context->dashPattern, AMfloat, count)
			if (context->dashPattern.error) {
				context->dashPattern.error = AM_DYNARRAY_NO_ERROR;
				return AM_FALSE;
			}
		}
		context->dashPattern.size = 0;
		// copy user values inside the context array
		for (i = 0; i < count; ++i) {
			AM_DYNARRAY_PUSH_BACK_LIGHT(context->dashPattern, (AMfloat)values[i])
		}

		j = count;
		// for an odd number of dashes, according to OpenVG specifications, last element must be ignored
		if (j & 1)
			j = j - 1;

		context->dashPatternSum = 0.0f;
		context->patchedDashPattern.size = 0;
		for (i = 0; i < j; ++i) {

			AMfloat val = AM_MAX(0.0f, context->dashPattern.data[i]);

			context->dashPatternSum += val;
			AM_DYNARRAY_PUSH_BACK(context->patchedDashPattern, AMfloat, val)
		}
		if (context->dashPatternSum <= AM_EPSILON_FLOAT) {
			context->patchedDashPattern.size = 0;
			context->dashPatternSum = 0.0f;
		}
	}
	// check for memory errors
	if (context->patchedDashPattern.error) {
		context->patchedDashPattern.error = AM_DYNARRAY_NO_ERROR;
		// in this case we force size = 0, in order to avoid inconsistencies
		context->patchedDashPattern.size = 0;
		return AM_FALSE;
	}
		return AM_TRUE;
}

/*!
	\brief Set stroke dash pattern on the given context (float values).
	\param context context where to set stroke dash pattern.
	\param count number of dash values.
	\param values input float dash values.
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
	\note some stroke derived data are recalculated.
*/
AMbool amCtxStrokeDashPatternSetf(AMContext *context,
								const VGint count,
								const VGfloat *values) {

	AM_ASSERT(count >= 0);

	if (count == 0) {
		AM_ASSERT(!values);
		// clear dash pattern array
		context->dashPattern.size = 0;
		context->patchedDashPattern.size = 0;
		context->dashPatternSum = 0.0f;
	}
	else {
		AMint32 i, j;

		if ((AMint32)context->dashPattern.capacity < count) {
			// allocate additional memory
			AM_DYNARRAY_CLEAR_RESERVE(context->dashPattern, AMfloat, count)
			if (context->dashPattern.error) {
				context->dashPattern.error = AM_DYNARRAY_NO_ERROR;
				return AM_FALSE;
			}
		}
		context->dashPattern.size = 0;
		// copy user values inside the context array
		for (i = 0; i < count; ++i) {
			AM_DYNARRAY_PUSH_BACK_LIGHT(context->dashPattern, amNanInfFix(values[i]))
		}

		j = count;
		// for an odd number of dashes, according to OpenVG specifications, last element must be ignored
		if (j & 1)
			j = j - 1;

		context->dashPatternSum = 0.0f;
		context->patchedDashPattern.size = 0;
		for (i = 0; i < j; ++i) {
		
			AMfloat val = AM_MAX(0.0f, context->dashPattern.data[i]);

			context->dashPatternSum += val;
			AM_DYNARRAY_PUSH_BACK(context->patchedDashPattern, AMfloat, val)
		}

		if (context->dashPatternSum <= AM_EPSILON_FLOAT) {
			context->patchedDashPattern.size = 0;
			context->dashPatternSum = 0.0f;
		}
	}
	// check for memory errors
	if (context->patchedDashPattern.error) {
		context->patchedDashPattern.error = AM_DYNARRAY_NO_ERROR;
		// in this case we force size = 0, in order to avoid inconsistencies
		context->patchedDashPattern.size = 0;
		return AM_FALSE;
	}
		return AM_TRUE;
}

/*!
	\brief Set matrix mode on the given context.
	\param context context where to set image mode.
	\param mode input matrix mode.
*/
void amCtxMatrixModeSet(AMContext *context,
						const VGMatrixMode mode) {

	AM_ASSERT(context);

	context->matrixMode = mode;

	switch (mode) {
		case VG_MATRIX_PATH_USER_TO_SURFACE:
			context->selectedMatrix = &context->pathUserToSurface;
			context->selectedInverseMatrix = &context->inversePathUserToSurface;
			context->selectedMatrixScale = context->pathUserToSurfaceScale;
			context->selectedMatrixFlags = &context->pathUserToSurfaceFlags;
			break;
		case VG_MATRIX_IMAGE_USER_TO_SURFACE:
			context->selectedMatrix = &context->imageUserToSurface;
			context->selectedInverseMatrix = &context->inverseImageUserToSurface;
			context->selectedMatrixScale = context->imageUserToSurfaceScale;
			context->selectedMatrixFlags = &context->imageUserToSurfaceFlags;
			break;
		case VG_MATRIX_FILL_PAINT_TO_USER:
			context->selectedMatrix = &context->fillPaintToUser;
			context->selectedInverseMatrix = &context->inverseFillPaintToUser;
			context->selectedMatrixScale = context->fillPaintToUserScale;
			context->selectedMatrixFlags = &context->fillPaintToUserFlags;
			break;
		case VG_MATRIX_STROKE_PAINT_TO_USER:
			context->selectedMatrix = &context->strokePaintToUser;
			context->selectedInverseMatrix = &context->inverseStrokePaintToUser;
			context->selectedMatrixScale = context->strokePaintToUserScale;
			context->selectedMatrixFlags = &context->strokePaintToUserFlags;
			break;
	#if (AM_OPENVG_VERSION >= 110)
		case VG_MATRIX_GLYPH_USER_TO_SURFACE:
			context->selectedMatrix = &context->glyphUserToSurface;
			context->selectedInverseMatrix = &context->inverseGlyphUserToSurface;
			context->selectedMatrixScale = context->glyphUserToSurfaceScale;
			context->selectedMatrixFlags = &context->glyphUserToSurfaceFlags;
			break;
	#endif
		default:
			AM_ASSERT(0 == 1);
			break;
	}
}

/*!
	\brief Set scissor rectangles on the given context (integer coordinates).
	\param context context where to set scissor rectangles.
	\param count number of integer coordinates.
	\param values input integer coordinates.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
	\note each scissor rectangles is specified by four values (x, y, width, height).
*/
AMbool amCtxScissorRectsSeti(AMContext *context,
						   const VGint count,
						   const VGint *values) {

	AM_ASSERT(count >= 0);

	if (count == 0) {
		AM_ASSERT(!values);
		// clear scissor rectangles array
		context->scissorRects.size = 0;
	}
	else {
		AMint32 i;

		AM_ASSERT(values);

		if ((AMint32)context->scissorRects.capacity < count) {
			// allocate additional memory
			AM_DYNARRAY_CLEAR_RESERVE(context->scissorRects, AMint32, count)
			if (context->scissorRects.error) {
				context->scissorRects.error = AM_DYNARRAY_NO_ERROR;
				return AM_FALSE;
			}
		}
		context->scissorRects.size = 0;

		for (i = 0; i < count; ++i) {
			AM_DYNARRAY_PUSH_BACK_LIGHT(context->scissorRects, values[i])
		}
	}
	context->scissorRectsModified = AM_TRUE;
#if defined(AM_GLE) || defined(AM_GLS)
	context->scissorRectsNeedUpload = AM_TRUE;
#endif
	return AM_TRUE;
}

/*!
	\brief Set scissor rectangles on the given context (float coordinates).
	\param context context where to set scissor rectangles.
	\param count number of integer coordinates.
	\param values input float coordinates.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
	\note each scissor rectangles is specified by four values (x, y, width, height).
*/
AMbool amCtxScissorRectsSetf(AMContext *context,
						   const VGint count,
						   const VGfloat *values) {

	AM_ASSERT(count >= 0);

	if (count == 0) {
		AM_ASSERT(!values);
		// clear scissor rectangles array
		context->scissorRects.size = 0;
	}
	else {
		AMint32 i;

		AM_ASSERT(values);

		if ((AMint32)context->scissorRects.capacity < count) {
			// allocate additional memory
			AM_DYNARRAY_CLEAR_RESERVE(context->scissorRects, AMint32, count)
			if (context->scissorRects.error) {
				context->scissorRects.error = AM_DYNARRAY_NO_ERROR;
				return AM_FALSE;
			}
		}
		context->scissorRects.size = 0;

		for (i = 0; i < count; ++i) {
			AM_DYNARRAY_PUSH_BACK_LIGHT(context->scissorRects, (AMint32)amFloorf(amNanInfFix(values[i])))
		}
	}
	context->scissorRectsModified = AM_TRUE;
#if defined(AM_GLE) || defined(AM_GLS)
	context->scissorRectsNeedUpload = AM_TRUE;
#endif
	return AM_TRUE;
}

#if (AM_OPENVG_VERSION >= 110)
void amCtxColorTransformHash(AMContext *context) {

	context->colorTransformHash = amHashCalculate(context->clampedColorTransformValues, 8);
	if (context->clampedColorTransformValues[0] <= 1.0f &&
		context->clampedColorTransformValues[1] <= 1.0f &&
		context->clampedColorTransformValues[2] <= 1.0f &&
		context->clampedColorTransformValues[3] <= 1.0f &&
		context->clampedColorTransformValues[4] == 0.0f &&
		context->clampedColorTransformValues[5] == 0.0f &&
		context->clampedColorTransformValues[6] == 0.0f &&
		context->clampedColorTransformValues[7] == 0.0f)
		context->ctNormalizedValues = AM_TRUE;
	else
		context->ctNormalizedValues = AM_FALSE;
}

void amCtxColorTransformClamp(AMfloat *clampedColorTransformValues,
							  const AMfloat *colorTransformValues) {

	AMuint32 i;

	AM_ASSERT(clampedColorTransformValues);
	AM_ASSERT(colorTransformValues);
	AM_ASSERT(clampedColorTransformValues != colorTransformValues);

	// scale between -127 and +127
	for (i = 0; i < 4; ++i)
		clampedColorTransformValues[i] = AM_CLAMP(colorTransformValues[i], -127.0f, 127.0f);
	// bias between -1 and +1
	for (i = 4; i < 8; ++i)
		clampedColorTransformValues[i] = AM_CLAMP(colorTransformValues[i], -1.0f, 1.0f);
}

/*!
	\brief Set color transform flag on the given context.
	\param context context containing color transform flag and values.
	\param colorTransform input color transform flag to be set.
*/
void amCtxColorTransformSet(AMContext *context,
							const VGboolean colorTransform) {

	AMfloat ctIdentity[8] = { 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f };

		context->colorTransform = colorTransform;
	// if color transform is disabled, we want clampedColorTransformValues (and the hash) to be equal to the identity transformation
	amCtxColorTransformClamp(context->clampedColorTransformValues, (context->colorTransform == VG_TRUE) ? context->colorTransformValues : ctIdentity);
	amCtxColorTransformHash(context);
}

/*!
	\brief Set color transform values on the given context (float values).
	\param context context containing color transform flag and values.
	\param colorTransformValues input color transform values to be set.
*/
void amCtxColorTransformValuesSetf(AMContext *context,
								   const VGfloat *colorTransformValues) {

	AMfloat ctIdentity[8] = { 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f };
		AMuint32 i;

	// copy scale and bias values inside the context
	for (i = 0; i < 8; ++i)
		context->colorTransformValues[i] = amNanInfFix(colorTransformValues[i]);

	// if color transform is disabled, we want clampedColorTransformValues (and the hash) to be equal to the identity transformation
	amCtxColorTransformClamp(context->clampedColorTransformValues, (context->colorTransform == VG_TRUE) ? context->colorTransformValues : ctIdentity);
	amCtxColorTransformHash(context);
}

/*!
	\brief Set color transform values on the given context (integer values).
*/
void amCtxColorTransformValuesSeti(AMContext *context,
								   const VGint *colorTransformValues) {

	AMfloat ctIdentity[8] = { 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	AMuint32 i;

	// copy scale and bias values inside the context
	for (i = 0; i < 8; ++i)
		context->colorTransformValues[i] = (AMfloat)colorTransformValues[i];

	// if color transform is disabled, we want clampedColorTransformValues (and the hash) to be equal to the identity transformation
	amCtxColorTransformClamp(context->clampedColorTransformValues, (context->colorTransform == VG_TRUE) ? context->colorTransformValues : ctIdentity);
	amCtxColorTransformHash(context);
}
#endif

void amMemMngRetrieve(AMContext *context,
					  const AMbool maxMemoryRetrieval) {

	AMuint32 newCapacity, i, j;
	AMUint32DynArray *availableHandlesList = &context->handles->availableHandlesList;
	AMHandleDynArray *createdHandlesList = &context->handles->createdHandlesList;

	AM_ASSERT(context);
	AM_ASSERT(createdHandlesList->size > 0);

	// retrieve memory from context arrays
	AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(context->scissorRects, AMint32)
	AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(context->splitScissorRects, AMScissorRect)
	AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(context->tmpFlatteningPts, AMVect2f)
#if defined(AM_FIXED_POINT_PIPELINE)
	AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(context->strokeAuxPts, AMVect2x)
	AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(context->strokeAuxPtsDx, AMVect2x)
#else
	AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(context->strokeAuxPts, AMVect2f)
	AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(context->strokeAuxPtsDx, AMVect2f)
#endif
	AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(context->strokeAuxPtsPerContour, AMint32)
	AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(context->dashPattern, AMfloat)
	AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(context->patchedDashPattern, AMfloat)
	AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(context->vguAuxCommands, AMuint8)
	AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(context->vguAuxS8Data, AMint8)
	AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(context->vguAuxS16Data, AMint16)
	AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(context->vguAuxS32Data, AMint32)
	AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(context->vguAuxF32Data, AMfloat)

	// retrieve memory from the rasterizer
	amRasMemoryRetrieve(context->rasterizer, maxMemoryRetrieval);
#if defined(AM_GLE) || defined(AM_GLS)
	// retrieve memory from the trinagulator
	amTriMemoryRetrieve(context->triangulator, maxMemoryRetrieval);
#endif

	if (availableHandlesList->size == 0)
		goto retrievePathsMemory;

#if defined(AM_DEBUG_MEMORY)
	amCtxCheckConsistence(context);
#endif

	// in this case there are some unused elements at the end of createdHandlesList, that can be retrieved
	if (availableHandlesList->data[0] == createdHandlesList->size - 1) {

		AMuint32 cur, prev, k;

		// count how many unused consecutive elements can be retrieved
		prev = availableHandlesList->data[0];
		i = 1;
		while (i < availableHandlesList->size) {
			cur = availableHandlesList->data[i];
			prev = availableHandlesList->data[i - 1];
			if (prev != cur + 1)
				break;
			i++;
		}
		// it's not convenient to retrieve memory for less than 64 elements
		if (i < 64)
			goto retrievePathsMemory;

		// delete unused element also from availableHandlesList
		newCapacity = prev;
		k = availableHandlesList->size - i;
		for (j = 0; j < k; ++j)
			availableHandlesList->data[j] = availableHandlesList->data[j + i];

		if (availableHandlesList->capacity > 64 + k) {
			AM_DYNARRAY_RESERVE(*availableHandlesList, AMuint32, 64 + k)
			}
		availableHandlesList->size = k;
		}
	else
		newCapacity = createdHandlesList->size;

	// retrieve the memory from createdHandlesList (unused consecutive elements)
	if (newCapacity < createdHandlesList->capacity) {
		AM_DYNARRAY_RESERVE(*createdHandlesList, AMhandle, newCapacity)
	}

retrievePathsMemory:
	#if defined(AM_DEBUG_MEMORY)
		amCtxCheckConsistence(context);
	#endif

	// compact paths pools
	if (context->handles->pathsPools.pools.size > 0 && context->handles->pathsPools.availablePathsList.size > 0) {

		AMPathsPoolPtrDynArray *pools = &context->handles->pathsPools.pools;
		AMPathRefDynArray *availablePathsList = &context->handles->pathsPools.availablePathsList;
		AMuint32 skippedHoles = 0;

		// first sort the list of available paths
		amPathsPoolsAvailablesSort(&context->handles->pathsPools);

		for (i = pools->size; i != 0; --i) {

			AMPathsPool *srcPool = pools->data[i - 1];
			AM_ASSERT(srcPool->size > 0);

			for (j = srcPool->size; (j != 0) && (skippedHoles < availablePathsList->size && availablePathsList->size > 0); --j) {

				if (srcPool->data[j - 1].referenceCounter == 0) {
					// unreferenced paths can be destroyed without problems
					amPathResourcesDestroy(&srcPool->data[j - 1], context);
					skippedHoles++;
	}
	else {
					AMPathsPool *dstPool;
					AMPathRef pathRef;

					pathRef = availablePathsList->data[availablePathsList->size - 1];
					AM_ASSERT(pathRef.c.pool < pools->size);

					// find the destination entry (an unreferenced path) where to move the current path
					dstPool = pools->data[pathRef.c.pool];
					AM_ASSERT(pathRef.c.poolIdx < dstPool->size);
					AM_ASSERT(dstPool->data[pathRef.c.poolIdx].referenceCounter == 0);
					AM_ASSERT(createdHandlesList->data[srcPool->data[j - 1].vgHandle] == &srcPool->data[j - 1]);
					// unreferenced paths can be destroyed without problems
					amPathResourcesDestroy(&dstPool->data[pathRef.c.poolIdx], context);
					// copy the "alive/referenced" path inside the destination hole
					dstPool->data[pathRef.c.poolIdx] = srcPool->data[j - 1];
					// manage the correct connection with the list of created OpenVG handles
					createdHandlesList->data[srcPool->data[j - 1].vgHandle] = &dstPool->data[pathRef.c.poolIdx];
					// remove an available entry
					availablePathsList->size--;
				}
				srcPool->size--;				
		}
			// if the pool is empty simply free it
			if (srcPool->size == 0) {
				amFree(srcPool);
				pools->size--;
		}
		}

		AM_ASSERT(skippedHoles == availablePathsList->size);
		availablePathsList->size = 0;
	}

#if defined(AM_DEBUG_MEMORY)
	amCtxCheckConsistence(context);
#endif

	// try to retrieve memory from "alive/referenced" paths
	if (context->handles->pathsPools.pools.size > 0) {

		AMPathsPoolPtrDynArray *pools = &context->handles->pathsPools.pools;

		for (i = 0; i < pools->size; ++i) {

			AMPathsPool *pool = pools->data[i];

			for (j = 0; j < pool->size; ++j) {
				AM_ASSERT(pool->data[j].referenceCounter > 0);
				amPathMemoryRetrieve(&pool->data[j], !maxMemoryRetrieval, context);
			}
			}
			}

#if defined(AM_DEBUG_MEMORY)
	amCtxCheckConsistence(context);
#endif

	// compact paints pools
	if (context->handles->paintsPools.pools.size > 0 && context->handles->paintsPools.availablePaintsList.size > 0) {

		AMPaintsPoolPtrDynArray *pools = &context->handles->paintsPools.pools;
		AMPaintRefDynArray *availablePaintsList = &context->handles->paintsPools.availablePaintsList;
		AMuint32 skippedHoles = 0;

		// first sort the list of available paints
		amPaintsPoolsAvailablesSort(&context->handles->paintsPools);

		for (i = pools->size; i != 0; --i) {

			AMPaintsPool *srcPool = pools->data[i - 1];
			AM_ASSERT(srcPool->size > 0);

			for (j = srcPool->size; (j != 0) && (skippedHoles < availablePaintsList->size && availablePaintsList->size > 0); --j) {

				if (srcPool->data[j - 1].referenceCounter == 0) {
					// unreferenced paints can be destroyed without problems
					amPaintResourcesDestroy(&srcPool->data[j - 1], context);
					skippedHoles++;
		}
				else {
					AMPaintsPool *dstPool;
					AMPaintRef paintRef;

					paintRef = availablePaintsList->data[availablePaintsList->size - 1];
					AM_ASSERT(paintRef.c.pool < pools->size);

					// find the destination entry (an unreferenced paint) where to move the current paint
					dstPool = pools->data[paintRef.c.pool];
					AM_ASSERT(paintRef.c.poolIdx < dstPool->size);
					AM_ASSERT(dstPool->data[paintRef.c.poolIdx].referenceCounter == 0);
					AM_ASSERT(createdHandlesList->data[srcPool->data[j - 1].vgHandle] == &srcPool->data[j - 1]);
					// unreferenced paints can be destroyed without problems
					amPaintResourcesDestroy(&dstPool->data[paintRef.c.poolIdx], context);
					// copy the "alive/referenced" paint inside the destination hole
					dstPool->data[paintRef.c.poolIdx] = srcPool->data[j - 1];
					// manage the correct connection with the list of created OpenVG handles
					createdHandlesList->data[srcPool->data[j - 1].vgHandle] = &dstPool->data[paintRef.c.poolIdx];
					// remove an available entry
					availablePaintsList->size--;
			}
				srcPool->size--;				
			}
			// if the pool is empty simply free it
			if (srcPool->size == 0) {
				amFree(srcPool);
				pools->size--;
		}
			}

		AM_ASSERT(skippedHoles == availablePaintsList->size);
		availablePaintsList->size = 0;
			}

#if defined(AM_DEBUG_MEMORY)
	amCtxCheckConsistence(context);
#endif

	// try to retrieve memory from "alive/referenced" paints
	if (context->handles->paintsPools.pools.size > 0) {

		AMPaintsPoolPtrDynArray *pools = &context->handles->paintsPools.pools;

		for (i = 0; i < pools->size; ++i) {
		
			AMPaintsPool *pool = pools->data[i];

			for (j = 0; j < pool->size; ++j) {
				AM_ASSERT(pool->data[j].referenceCounter > 0);
				amPaintMemoryRetrieve(&pool->data[j], !maxMemoryRetrieval, context);
		}
	}
	}

#if defined(AM_DEBUG_MEMORY)
	amCtxCheckConsistence(context);
#endif

	/*
	// DEBUG STUFF
	if (maxMemoryRetrieval) {
		AM_MEMORY_LOG("--------------------------------------amMemMngRetrieve(max)");
	}
	else {
		AM_MEMORY_LOG("--------------------------------------amMemMngRetrieve(lite)");
	}
	*/
}

// Decrement the counter of the memory manager.
void amCtxMemMngCountDown(AMContext *context) {

	AMContextHandlesList *handles = context->handles;

	AM_ASSERT(context);

	// retrieve memory if memMngCreatedHandles counter reaches 0
	handles->memMngCreatedHandles--;
	if (handles->memMngCreatedHandles == 0) {
		amUint32QSort(handles->availableHandlesList.data, handles->availableHandlesList.size);
		handles->memMngAvailableHandles = AM_OPENVG_CALLS_BEFORE_AVAILABLE_HANDLES_SORT;
		amMemMngRetrieve(context, AM_FALSE);
		handles->memMngCreatedHandles = AM_OPENVG_CALLS_BEFORE_MEMORY_RECOVERY;
	}
	// sort available handles list, in order to let new created handles to have the lowest memory location
	// available in the created handles list
	handles->memMngAvailableHandles--;
	if (handles->memMngAvailableHandles == 0) {
		amUint32QSort(handles->availableHandlesList.data, handles->availableHandlesList.size);
		handles->memMngAvailableHandles = AM_OPENVG_CALLS_BEFORE_AVAILABLE_HANDLES_SORT;
	}
}

#if defined(AM_DEBUG_MEMORY)
void amCtxCheckConsistence(AMContext *context) {

	AMuint32 i, j;
	AMPathsPoolPtrDynArray *pathsPools = &context->handles->pathsPools.pools;
	AMPaintsPoolPtrDynArray *paintsPools = &context->handles->paintsPools.pools;
	AMHandleDynArray *createdHandlesList = &context->handles->createdHandlesList;
	AMPathRefDynArray *availablePathsList = &context->handles->pathsPools.availablePathsList;
	AMPaintRefDynArray *availablePaintsList = &context->handles->paintsPools.availablePaintsList;

	// check paths
	for (i = 0; i < pathsPools->size; ++i) {

		AMPathsPool *pool = pathsPools->data[i];

		for (j = 0; j < pool->size; ++j) {

			AMPath *path = &pool->data[j];

			if (path->referenceCounter == 0) {
				if (createdHandlesList->data[path->vgHandle] != NULL) {
					AM_ASSERT(createdHandlesList->data[path->vgHandle] == NULL);
				}
			}
			else {
				if (createdHandlesList->data[path->vgHandle] != path) {
					AM_ASSERT(createdHandlesList->data[path->vgHandle] == path);
				}
			}
		}
	}

	for (i = 0; i < availablePathsList->size; ++i) {

		AMPathRef pathRef = availablePathsList->data[i];
		AMPathsPool *pool;
		AMPath *path;

		AM_ASSERT(pathRef.c.pool < pathsPools->size);
		AM_ASSERT(pathRef.c.poolIdx < pathsPools->data[pathRef.c.pool]->size);

		pool = pathsPools->data[pathRef.c.pool];
		path = &pool->data[pathRef.c.poolIdx];
		AM_ASSERT(path->referenceCounter == 0);
		}

	// check paints
	for (i = 0; i < paintsPools->size; ++i) {

		AMPaintsPool *pool = paintsPools->data[i];

		for (j = 0; j < pool->size; ++j) {

			AMPaint *paint = &pool->data[j];
		
			if (paint->referenceCounter == 0) {
				if (createdHandlesList->data[paint->vgHandle] != NULL) {
					AM_ASSERT(createdHandlesList->data[paint->vgHandle] == NULL);
				}
			}
			else {
				if (createdHandlesList->data[paint->vgHandle] != paint) {
					AM_ASSERT(createdHandlesList->data[paint->vgHandle] == paint);
		}
	}
		}
	}

	for (i = 0; i < availablePaintsList->size; ++i) {

		AMPaintRef paintRef = availablePaintsList->data[i];
		AMPaintsPool *pool;
		AMPaint *paint;

		AM_ASSERT(paintRef.c.pool < paintsPools->size);
		AM_ASSERT(paintRef.c.poolIdx < paintsPools->data[paintRef.c.pool]->size);

		pool = paintsPools->data[paintRef.c.pool];
		paint = &pool->data[paintRef.c.poolIdx];
		AM_ASSERT(paint->referenceCounter == 0);
	}
}
#endif

/*!
	\brief Create a new VGHandle.
	\param context context containing the list of created/available handles.
	\param object the pointer to associate to the created VGHandle.
	\return the new VGHandle.
*/
VGHandle amCtxHandleNew(AMContext *context,
						AMhandle object) {

	VGHandle res;
	AMuint16 *idPtr = (AMuint16 *)object;
	VGHandle *handlePtr = (VGHandle *)(idPtr + 2);
	AMContextHandlesList *handles = context->handles;

	AM_ASSERT(context);
	AM_ASSERT(object);

	if (handles->availableHandlesList.size > 0) {
		handles->availableHandlesList.size--;
		res = handles->availableHandlesList.data[handles->availableHandlesList.size];
		handles->availableHandlesList.data[handles->availableHandlesList.size] = 0;
		handles->createdHandlesList.data[res] = object;
	}
	else {
		AM_DYNARRAY_PUSH_BACK(handles->createdHandlesList, AMhandle, object)
		if (handles->createdHandlesList.error) {
			handles->createdHandlesList.error = AM_DYNARRAY_NO_ERROR;
			return VG_INVALID_HANDLE;
		}
		res = (VGHandle)(handles->createdHandlesList.size - 1);
	}
	// copy the returned OpenVG handle inside the objects (AMPath / AMPaint / AMImage / AMFont) vgHandle field
	*handlePtr = res;
	return res;
}

/*!
	\brief Remove a VGHandle from the given context.
	\param context context containing the list of created/available handles.
	\param handle the VGHandle to remove.
	\return the removed handle.
*/
AMhandle amCtxHandleRemove(AMContext *context,
						   const VGHandle handle) {

	AMhandle object;
	AMuint16 *idPtr;
	VGHandle *handlePtr;
	AMContextHandlesList *handles = context->handles;

	AM_ASSERT(handle > 0 && handle < handles->createdHandlesList.size);
	AM_ASSERT(handles->createdHandlesList.data[handle]);

	object = handles->createdHandlesList.data[handle];
	handles->createdHandlesList.data[handle] = NULL;

	AM_DYNARRAY_PUSH_BACK(handles->availableHandlesList, VGHandle, handle)
	// if the was a memory error in the push back, we loose and handle entry, but we can do nothing
	if (handles->availableHandlesList.error)
		handles->availableHandlesList.error = AM_DYNARRAY_NO_ERROR;
	// set NULL OpenVG handle inside the objects (AMPath / AMPaint / AMImage / AMFont) vgHandle field
	idPtr = (AMuint16 *)object;
	handlePtr = (VGHandle *)(idPtr + 2);
	*handlePtr = VG_INVALID_HANDLE;
	return object;
}

/*!
	\brief Check if a VGHandle is valid for the given context; if valid, it returns an handle identifier.
	\param context context containing the list of created handles.
	\param handle the VGHandle to check for validity.
	\return AM_INVALID_HANDLE_ID if handle is not valid for the given context, else one of the following
	handle identifiers: AM_PATH_HANDLE_ID, AM_IMAGE_HANDLE_ID, AM_PAINT_HANDLE_ID, AM_LAYER_HANDLE_ID, AM_FONT_HANDLE_ID.
*/
AMuint32 amCtxHandleValid(const AMContext *context,
						  const VGHandle handle) {

	AMuint32 res;
	const AMuint16 *idPtr;
	const AMhandle object;
	const AMPath *path;
	const AMImage *image;
#if (AM_OPENVG_VERSION >= 110)
	const AMImage *layer;
#endif
	const AMContextHandlesList *handles = context->handles;

	if (!handle || handle >= handles->createdHandlesList.size)
		return AM_INVALID_HANDLE_ID;

	object = handles->createdHandlesList.data[handle];
	if (!object)
		return AM_INVALID_HANDLE_ID;

	idPtr = (const AMuint16 *)object;

	switch (*idPtr) {
		case AM_PATH_HANDLE_ID:
			path = (AMPath *)object;
			res = (path->format != VG_PATH_DATATYPE_INVALID) ? AM_PATH_HANDLE_ID : AM_INVALID_HANDLE_ID;
			break;
		case AM_IMAGE_HANDLE_ID:
			image = (AMImage *)object;
			res = (image->format != (VGImageFormat)VG_IMAGE_FORMAT_INVALID) ? AM_IMAGE_HANDLE_ID : AM_INVALID_HANDLE_ID;
			break;
		case AM_PAINT_HANDLE_ID:
			res = AM_PAINT_HANDLE_ID;
			break;
	#if (AM_OPENVG_VERSION >= 110)
		case AM_LAYER_HANDLE_ID:
			layer = (AMImage *)object;
			res = (layer->format == VG_A_8) ? AM_LAYER_HANDLE_ID : AM_INVALID_HANDLE_ID;
			break;
		case AM_FONT_HANDLE_ID:
			res = AM_FONT_HANDLE_ID;
			break;
	#endif
		default:
			res = AM_INVALID_HANDLE_ID;
	}
	return res;
}

/*
	\brief Increment the reference counter of a given object (path, image, paint, mask layer, font).
	\param object pointer to the object whose reference counter is to increment.
	\return the reference counter value after the increment.
*/
AMuint32 amCtxHandleRefCounterInc(AMhandle object) {

	AMPath *path;
	AMImage *image;
	AMPaint *paint;
#if (AM_OPENVG_VERSION >= 110)
	AMImage *layer;
	AMFont *font;
#endif
	AMuint16 *idPtr;
	AMuint32 res;

	AM_ASSERT(object);
	idPtr = (AMuint16 *)object;
	idPtr++;
	switch (*idPtr) {
		case AM_PATH_HANDLE_ID:
			path = (AMPath *)object;
			path->referenceCounter++;
			res = path->referenceCounter;
			break;
		case AM_IMAGE_HANDLE_ID:
			image = (AMImage *)object;
			image->referenceCounter++;
			res = image->referenceCounter;
			break;
		case AM_PAINT_HANDLE_ID:
			paint = (AMPaint *)object;
			paint->referenceCounter++;
			res = paint->referenceCounter;
			break;
	#if (AM_OPENVG_VERSION >= 110)
		case AM_LAYER_HANDLE_ID:
			layer = (AMImage *)object;
			layer->referenceCounter++;
			res = layer->referenceCounter;
			break;
		case AM_FONT_HANDLE_ID:
			font = (AMFont *)object;
			font->referenceCounter++;
			res = font->referenceCounter;
			break;
	#endif
		default:
			AM_ASSERT(0 == 1);
			res = 0;
			break;
	}
	return res;
}

/*
	\brief Decrement the reference counter of a given object (path, image, paint, mask layer, font).
	\param object pointer to the object whose reference counter is to decrement.
	\return the reference counter value after the decrement.
*/
AMuint32 amCtxHandleRefCounterDec(AMhandle object,
								  AMContext *context) {

	AMPath *path;
	AMImage *image;
	AMPaint *paint;
#if (AM_OPENVG_VERSION >= 110)
	AMImage *layer;
	AMFont *font;
#endif
	AMuint16 *idPtr;
	AMuint32 res;

	AM_ASSERT(object);
	idPtr = (AMuint16 *)object;
	idPtr++;
	switch (*idPtr) {
		case AM_PATH_HANDLE_ID:
			path = (AMPath *)object;
			AM_ASSERT(path->referenceCounter > 0);
			path->referenceCounter--;
			// path memory is handled by a pools manager
			res = path->referenceCounter;
			break;
		case AM_IMAGE_HANDLE_ID:
			image = (AMImage *)object;
			AM_ASSERT(image->referenceCounter > 0);
			image->referenceCounter--;
			if (image->referenceCounter == 0)
				amImageResourcesDestroy(image, context);
			res = image->referenceCounter;
			break;
		case AM_PAINT_HANDLE_ID:
			paint = (AMPaint *)object;
			AM_ASSERT(paint->referenceCounter > 0);
			paint->referenceCounter--;
			// paint memory is handled by a pools manager
			res = paint->referenceCounter;
			break;
	#if (AM_OPENVG_VERSION >= 110)
		case AM_LAYER_HANDLE_ID:
			layer = (AMImage *)object;
			AM_ASSERT(layer->referenceCounter > 0);
			layer->referenceCounter--;
			if (layer->referenceCounter == 0)
				amImageResourcesDestroy(layer, context);
			res = layer->referenceCounter;
			break;
		case AM_FONT_HANDLE_ID:
			font = (AMFont *)object;
			AM_ASSERT(font->referenceCounter > 0);
			font->referenceCounter--;
			if (font->referenceCounter == 0)
				amFontResourcesDestroy(font, context);
			res = font->referenceCounter;
			break;
	#endif
		default:
			AM_ASSERT(0 == 1);
			res = 0;
			break;
	}
	return res;
}

#if defined(AM_SRE) && defined(VG_MZT_conical_gradient)
/*!
	\brief Initialize atan2 table values.
	\param context containing the atan2 table.
	\return AM_TRUE if the operation has been performed successfully, else AM_FALSE (memory allocation error).
*/
AMbool amCtxAtan2TableInit(AMContext *context) {

	AMuint32 ofs0, ofs, x, y;
	AMuint32 size = AM_GRADIENTS_CONICAL_TEXTURE_WIDTH;
	AMuint32 sizeSqr = size * size;
	AMfloat halfSizef = (AMfloat)(size >> 1);
	AMfloat k = ((AMfloat)(1 << AM_ATAN2_TABLE_PRECISION_BITS)) / AM_2PI;

	AM_ASSERT(context);

	context->atan2Table = (AMuint32 *)amMalloc(sizeSqr * sizeof(AMuint32));
	if (!context->atan2Table) {
		AM_DEBUG("Cannot allocate memory for atan2 lookup table.");
		return AM_FALSE;
	}

	ofs0 = 0;
	for (y = 0; y < size; ++y) {
		AMfloat ry = (AMfloat)y - halfSizef;
		for (x = 0; x < size; ++x) {
			AMfloat rx = (AMfloat)x - halfSizef;
			AMfloat atan2Val = amAtan2f(ry, rx);
			if (atan2Val < 0.0f)
				atan2Val = AM_2PI + atan2Val;
			ofs = ofs0 + x;
			AM_ASSERT(ofs < sizeSqr);
			context->atan2Table[ofs] = (AMuint32)(atan2Val * k);
			AM_ASSERT(context->atan2Table[ofs] <= ((AMfloat)(1 << AM_ATAN2_TABLE_PRECISION_BITS)));
		}
		ofs0 += size;
	}
	return AM_TRUE;
}
#endif

void amCtxDynResourcesInit(AMContext *context) {

	AM_ASSERT(context);

	AM_DYNARRAY_PREINIT(context->scissorRects)
	AM_DYNARRAY_PREINIT(context->dashPattern)
	AM_DYNARRAY_PREINIT(context->splitScissorRects)
	AM_DYNARRAY_PREINIT(context->tmpFlatteningPts)
	AM_DYNARRAY_PREINIT(context->strokeAuxPts)
	AM_DYNARRAY_PREINIT(context->strokeAuxPtsDx)
	AM_DYNARRAY_PREINIT(context->strokeAuxPtsPerContour)
	AM_DYNARRAY_PREINIT(context->patchedDashPattern)
	AM_DYNARRAY_PREINIT(context->vguAuxCommands)
	AM_DYNARRAY_PREINIT(context->vguAuxS8Data)
	AM_DYNARRAY_PREINIT(context->vguAuxS16Data)
	AM_DYNARRAY_PREINIT(context->vguAuxS32Data)
	AM_DYNARRAY_PREINIT(context->vguAuxF32Data)

	context->handles = NULL;
	context->sharedContext = NULL;
#if !defined(AM_LITE_PROFILE)
	context->gaussianKernelX = NULL;
	context->gaussianKernelY = NULL;
#endif
	context->rasterizer = NULL;
#if defined(AM_GLE) || defined(AM_GLS)
	context->triangulator = NULL;
#endif
#if defined(AM_SRE) && defined(VG_MZT_conical_gradient)
	context->atan2Table = NULL;
#endif
}

void amCtxHandlesDestroy(AMContext *context) {

	AMuint32 i, j;
	AMContextHandlesList *handles;

	AM_ASSERT(context);
	AM_ASSERT(context->handles);
	AM_ASSERT(context->handles->referenceCounter == 0);

#if defined(AM_DEBUG_MEMORY)
	amCtxCheckConsistence(context);
#endif

	handles = context->handles;
	j = handles->createdHandlesList.size;
	for (i = 0; i < j; ++i) {

		AMuint16 *idPtr;
		AMImage *image;
	#if (AM_OPENVG_VERSION >= 110)
		AMImage *layer;
		AMFont *font;
	#endif
		AMhandle object = handles->createdHandlesList.data[i];

		if (!object)
			continue;

		idPtr = (AMuint16 *)object;
		idPtr++;
		switch (*idPtr) {
			case AM_PATH_HANDLE_ID:
				// path memory is handled by a pools manager
				break;
			case AM_IMAGE_HANDLE_ID:
				image = (AMImage *)object;
				if (image->referenceCounter > 0)
					amImageResourcesDestroy(image, context);
				amFree(image);
				break;
			case AM_PAINT_HANDLE_ID:
				// paint memory is handled by a pools manager
				break;
		#if (AM_OPENVG_VERSION >= 110)
			case AM_LAYER_HANDLE_ID:
				layer = (AMImage *)object;
				if (layer->referenceCounter > 0)
					amImageResourcesDestroy(layer, context);
				amFree(layer);
				break;
			case AM_FONT_HANDLE_ID:
				font = (AMFont *)object;
				if (font->referenceCounter > 0)
					amFontResourcesDestroy(font, context);
				amFree(font);
				break;
		#endif
		}
	}

	// destroy resources owned by each created path
	if (handles->pathsPools.initialized) {

		AMPathsPoolPtrDynArray *pools = &handles->pathsPools.pools;

		for (i = 0; i < pools->size; ++i) {

			AMPathsPool *pool = pools->data[i];

			for (j = 0; j < pool->size; ++j)
				amPathResourcesDestroy(&pool->data[j], context);
		}
	}

	// destroy resources owned by each created paint
	if (handles->paintsPools.initialized) {

		AMPaintsPoolPtrDynArray *pools = &handles->paintsPools.pools;

		for (i = 0; i < pools->size; ++i) {

			AMPaintsPool *pool = pools->data[i];

			for (j = 0; j < pool->size; ++j)
				amPaintResourcesDestroy(&pool->data[j], context);
		}
	}
}

void amCtxDynResourcesDestroy(AMContext *context) {

	AM_ASSERT(context);

	AM_DYNARRAY_DESTROY(context->vguAuxF32Data)
	AM_DYNARRAY_DESTROY(context->vguAuxS32Data)
	AM_DYNARRAY_DESTROY(context->vguAuxS16Data)
	AM_DYNARRAY_DESTROY(context->vguAuxS8Data)
	AM_DYNARRAY_DESTROY(context->vguAuxCommands)
	AM_DYNARRAY_DESTROY(context->patchedDashPattern)
	AM_DYNARRAY_DESTROY(context->strokeAuxPtsPerContour)
	AM_DYNARRAY_DESTROY(context->strokeAuxPtsDx)
	AM_DYNARRAY_DESTROY(context->strokeAuxPts)
	AM_DYNARRAY_DESTROY(context->tmpFlatteningPts)
	AM_DYNARRAY_DESTROY(context->splitScissorRects)
	AM_DYNARRAY_DESTROY(context->dashPattern)
	AM_DYNARRAY_DESTROY(context->scissorRects)

	if (context->handles) {
		// decrement reference counter and free handles + memory if handles are not more referenced
		context->handles->referenceCounter--;
		if (context->handles->referenceCounter == 0) {
			amCtxHandlesDestroy(context);
			AM_DYNARRAY_DESTROY(context->handles->createdHandlesList)
			AM_DYNARRAY_DESTROY(context->handles->availableHandlesList)
			amPathsPoolsManagerDestroy(&context->handles->pathsPools);
			amPaintsPoolsManagerDestroy(&context->handles->paintsPools);
			amFree(context->handles);
		}
	}

#if !defined(AM_LITE_PROFILE)
	if (context->gaussianKernelX) {
		amFree(context->gaussianKernelX);
		context->gaussianKernelX = NULL;
	}
	if (context->gaussianKernelY) {
		amFree(context->gaussianKernelY);
		context->gaussianKernelY = NULL;
	}
#endif
	if (context->rasterizer) {
		amRasDestroy(context->rasterizer);
		context->rasterizer = NULL;
	}

#if defined(AM_GLE) || defined(AM_GLS)
	if (context->triangulator) {
		amTriDestroy(context->triangulator);
		context->triangulator = NULL;
	}
#endif

#if defined(AM_SRE) && defined(VG_MZT_conical_gradient)
	if (context->atan2Table) {
		amFree(context->atan2Table);
		context->atan2Table = NULL;
	}
#endif
}

/*!
	\brief Initialize a given OpenVG context.
	\param context context to initialize.
	\param sharedContext an optional shared context: if not NULL, then all shareable data, as defined by OpenVG
	will be shared by sharedContext, all other contexts sharedContext already shares with, and the newly created context.
	\param confParams input configuration parameters (set by default or read from the external configuration file).
	\return AM_TRUE if initialization has been executed successfully, else AM_FALSE.
	\todo read external configuration file on BREW.
*/
AMbool amCtxInit(AMContext *context,
				 AMContext *sharedContext,
				 const AMContextConfParams *confParams) {

	AMPaint *paint;

	AM_ASSERT(context);
	AM_ASSERT(context != sharedContext);

	if (context->initialized)
		return AM_TRUE;

	// check for correct endianess
#if defined(AM_LITTLE_ENDIAN)
	if (amSystemEndianessGet() == AM_BIG_ENDIANESS) {
		AM_DEBUG("Compile-time and run-time endianess mismatch.");
		return AM_FALSE;
	}
#elif defined(AM_BIG_ENDIAN)
	if (amSystemEndianessGet() == AM_LITTLE_ENDIANESS) {
		AM_DEBUG("Compile-time and run-time endianess mismatch.");
		return AM_FALSE;
	}
#else
	#error Unreachable point.
#endif

#if defined(AM_DEBUG_MEMORY) && defined(AM_STANDALONE) && (defined(AM_OS_WIN) || defined(AM_OS_WINCE) || defined(AM_OS_LINUX))
	context->allocatedMemory = 0;
	#if defined(AM_OS_WINCE)
	context->memoryLog = fopen("\\Windows\\amanithvg_memory_log.txt", "wt");
	#else
		context->memoryLog = fopen("amanithvg_memory_log.txt", "wt");
	#endif
#endif

	// initialize (as unallocated) all dynamic resources
	amCtxDynResourcesInit(context);

	// original (unpatched) OpenVG values
	context->matrixMode = VG_MATRIX_PATH_USER_TO_SURFACE;
	context->fillRule = VG_EVEN_ODD;
	context->imageQuality = VG_IMAGE_QUALITY_FASTER;
	context->renderingQuality = VG_RENDERING_QUALITY_BETTER;
	context->strokeBlendMode = VG_BLEND_SRC_OVER;
	context->fillBlendMode = VG_BLEND_SRC_OVER;
	context->imageMode = VG_DRAW_IMAGE_NORMAL;
	AM_DYNARRAY_INIT_RESERVE(context->scissorRects, AMint32, 16)
	if (context->scissorRects.error) {
		amCtxDynResourcesDestroy(context);
		return AM_FALSE;
	}
#if (AM_OPENVG_VERSION >= 110)
    context->colorTransform = VG_FALSE;
	context->colorTransformValues[0] = context->clampedColorTransformValues[0] = 1.0f;
	context->colorTransformValues[1] = context->clampedColorTransformValues[1] = 1.0f;
	context->colorTransformValues[2] = context->clampedColorTransformValues[2] = 1.0f;
	context->colorTransformValues[3] = context->clampedColorTransformValues[3] = 1.0f;
	context->colorTransformValues[4] = context->clampedColorTransformValues[4] = 0.0f;
	context->colorTransformValues[5] = context->clampedColorTransformValues[5] = 0.0f;
	context->colorTransformValues[6] = context->clampedColorTransformValues[6] = 0.0f;
	context->colorTransformValues[7] = context->clampedColorTransformValues[7] = 0.0f;
#endif
	context->strokeLineWidth = 1.0f;
	context->startCapStyle = context->endCapStyle = VG_CAP_BUTT;
	context->joinStyle = VG_JOIN_MITER;
	context->miterLimit = 4.0f;
	AM_DYNARRAY_INIT_RESERVE(context->dashPattern, AMfloat, 8)
	if (context->dashPattern.error) {
		amCtxDynResourcesDestroy(context);
		return AM_FALSE;
	}
	context->dashPhase = 0.0f;
	context->dashPhaseReset = VG_FALSE;
	context->tileFillColor[AM_R] = 0.0f;
	context->tileFillColor[AM_G] = 0.0f;
	context->tileFillColor[AM_B] = 0.0f;
	context->tileFillColor[AM_A] = 0.0f;
	context->clearColor[AM_R] = 0.0f;
	context->clearColor[AM_G] = 0.0f;
	context->clearColor[AM_B] = 0.0f;
	context->clearColor[AM_A] = 0.0f;
#if (AM_OPENVG_VERSION >= 110)
	context->glyphOrigin[0] = 0.0f;
	context->glyphOrigin[1] = 0.0f;
#endif
	context->masking = VG_FALSE;
	context->scissoring = VG_FALSE;
	context->pixelLayout = VG_PIXEL_LAYOUT_UNKNOWN;
	context->screenLayout = VG_PIXEL_LAYOUT_UNKNOWN;
	context->filterFormatLinear = VG_FALSE;
	context->filterFormatPremultiplied = VG_FALSE;
	context->filterChannelMask = (VG_RED | VG_GREEN | VG_BLUE | VG_ALPHA);
	context->maxScissorRects = VG_MAXINT;
	context->maxDashCount = VG_MAXINT;
	context->maxKernelSize = 255;
	context->maxSeparableKernelSize = 255;
	context->maxColorRampStops = VG_MAXINT;
#if defined (RIM_VG_SRC)
	context->maxImageWidth = 1024;
	context->maxImageHeight = 1024;
#else
	context->maxImageWidth = 23170;
	context->maxImageHeight = 23170;
#endif
	context->maxImagePixels = context->maxImageWidth * context->maxImageHeight;
	context->maxImageBytes = context->maxImagePixels * 4;
	context->maxFloat = AM_MAX_FLOAT;
	context->maxGaussianStdDeviation = 128;
	context->error = VG_NO_ERROR;
	AM_MATRIX33_IDENTITY(&context->pathUserToSurface);
	AM_MATRIX33_IDENTITY(&context->imageUserToSurface);
	AM_MATRIX33_IDENTITY(&context->fillPaintToUser);
	AM_MATRIX33_IDENTITY(&context->strokePaintToUser);
#if (AM_OPENVG_VERSION >= 110)
	AM_MATRIX33_IDENTITY(&context->glyphUserToSurface);
#endif
	context->strokePaint = 0;
	context->fillPaint = 0;

	// take care of shared context
	if (!sharedContext) {

		context->sharedContext = NULL;
		context->handles = (AMContextHandlesList *)amMalloc(sizeof(AMContextHandlesList));
		if (!context->handles) {
			amCtxDynResourcesDestroy(context);
			return AM_FALSE;
		}
		context->handles->referenceCounter = 1;
		AM_DYNARRAY_PREINIT(context->handles->createdHandlesList)
		AM_DYNARRAY_PREINIT(context->handles->availableHandlesList)

	// list of created VGHandle(s): VGPaint, VGPath, VGImage, VGMaskLayer, VGFont
		AM_DYNARRAY_INIT_RESERVE(context->handles->createdHandlesList, AMhandle, 64)
		if (context->handles->createdHandlesList.error) {
			amCtxDynResourcesDestroy(context);
			return AM_FALSE;
		}
		// initialize the paths (AMPath) pools manager 
		if (!amPathsPoolsManagerInit(&context->handles->pathsPools)) {
			amCtxDynResourcesDestroy(context);
			return AM_FALSE;
		}
		// initialize the paints (AMPaint) pools manager 
		if (!amPaintsPoolsManagerInit(&context->handles->paintsPools)) {
			amCtxDynResourcesDestroy(context);
			return AM_FALSE;
		}

		context->handles->createdHandlesList.size = 1;
		context->handles->createdHandlesList.data[0] = NULL;
		context->handles->memMngCreatedHandles = AM_OPENVG_CALLS_BEFORE_MEMORY_RECOVERY;
	// list of available VGHandle(s), to be reused
		AM_DYNARRAY_INIT_RESERVE(context->handles->availableHandlesList, AMuint32, 64)
		if (context->handles->availableHandlesList.error) {
			amCtxDynResourcesDestroy(context);
			return AM_FALSE;
		}
		context->handles->memMngAvailableHandles = AM_OPENVG_CALLS_BEFORE_AVAILABLE_HANDLES_SORT;
		
	}
	else {
		context->sharedContext = sharedContext;
		context->handles = sharedContext->handles;
		context->handles->referenceCounter++;
	}

#if defined(AM_GLE) || defined(AM_GLS)
	// initialize the OpenGL / OpenGL ES context
	context->glContext.initialized = AM_FALSE;
	if (!amGlCtxInit(&context->glContext)) {
		AM_DEBUG("Cannot initialize the OpenGL / OpenGL ES context.");
		amCtxDynResourcesDestroy(context);
		return AM_FALSE;
	}
	// create and initialize the triangulator
	if (!amTriCreate(&context->triangulator)) {
		AM_DEBUG("Cannot allocate memory for the triangulator.");
		amCtxDynResourcesDestroy(context);
		return AM_FALSE;
	}
#endif

	// auxiliary structures used to decompose OpenVG scissor rectangles into a set of non-overlapping rectangles
	context->scissorRectsModified = AM_TRUE;
#if defined(AM_GLE) || defined(AM_GLS)
	context->scissorRectsNeedUpload = AM_TRUE;
	context->scissoringLastDraw = (context->scissoring == VG_TRUE) ? VG_FALSE : VG_TRUE;
#endif

	AM_DYNARRAY_INIT(context->splitScissorRects, AMScissorRect)
	if (context->splitScissorRects.error) {
		amCtxDynResourcesDestroy(context);
		return AM_FALSE;
	}
	AM_VECT2_SET(&context->splitScissorRectsBox.minPoint, 0, 0)
	AM_VECT2_SET(&context->splitScissorRectsBox.maxPoint, 0, 0)

#if (AM_OPENVG_VERSION >= 110)
	// calculate hash for the current color transformation
	amCtxColorTransformHash(context);
	context->ctNormalizedValues = AM_TRUE;
#endif

#if !defined(AM_LITE_PROFILE)
	// image filters related structures.
	context->gaussianKernelX = (AMint16 *)amMalloc((6 * context->maxGaussianStdDeviation + 1) * sizeof(AMint16));
	if (!context->gaussianKernelX) {
		AM_DEBUG("Cannot allocate memory for gaussianKernelX");
		amCtxDynResourcesDestroy(context);
		return AM_FALSE;
	}
	context->gaussianKernelY = (AMint16 *)amMalloc((6 * context->maxGaussianStdDeviation + 1) * sizeof(AMint16));
	if (!context->gaussianKernelY) {
		AM_DEBUG("Cannot allocate memory for gaussianKernelY");
		amCtxDynResourcesDestroy(context);
		return AM_FALSE;
	}
#endif

	// matrices related values and status
	AM_MATRIX33_IDENTITY(&context->inversePathUserToSurface);
	context->pathUserToSurfaceScale[0] = context->pathUserToSurfaceScale[1] = 1.0f;
#if defined(AM_GLE)
	context->pathUserToSurfaceFlags = AM_MATRIX_MODELVIEW_MODIFIED;
#else
	context->pathUserToSurfaceFlags = 0;
#endif

	AM_MATRIX33_IDENTITY(&context->inverseImageUserToSurface);
	context->imageUserToSurfaceScale[0] = context->imageUserToSurfaceScale[1] = 1.0f;
#if defined(AM_GLE)
	context->imageUserToSurfaceFlags = AM_MATRIX_MODELVIEW_MODIFIED;
#else
	context->imageUserToSurfaceFlags = 0;
#endif

	AM_MATRIX33_IDENTITY(&context->inverseFillPaintToUser);
	context->fillPaintToUserScale[0] = context->fillPaintToUserScale[1] = 1.0f;
#if defined(AM_GLE)
	context->fillPaintToUserFlags = AM_MATRIX_MODELVIEW_MODIFIED;
#else
	context->fillPaintToUserFlags = 0;
#endif

	AM_MATRIX33_IDENTITY(&context->inverseStrokePaintToUser);
	context->strokePaintToUserScale[0] = context->strokePaintToUserScale[1] = 1.0f;
#if defined(AM_GLE)
	context->strokePaintToUserFlags = AM_MATRIX_MODELVIEW_MODIFIED;
#else
	context->strokePaintToUserFlags = 0;
#endif

#if (AM_OPENVG_VERSION >= 110)
	AM_MATRIX33_IDENTITY(&context->inverseGlyphUserToSurface);
	context->glyphUserToSurfaceScale[0] = context->glyphUserToSurfaceScale[1] = 1.0f;
#if defined(AM_GLE)
	context->glyphUserToSurfaceFlags = AM_MATRIX_MODELVIEW_MODIFIED;
#else
	context->glyphUserToSurfaceFlags = 0;
#endif
#endif

	context->selectedMatrix = &context->pathUserToSurface;
	context->selectedInverseMatrix = &context->inversePathUserToSurface;
	context->selectedMatrixScale = context->pathUserToSurfaceScale;
	context->selectedMatrixFlags = &context->pathUserToSurfaceFlags;

	// curves flattening structures
	AM_DYNARRAY_INIT_RESERVE(context->tmpFlatteningPts, AMVect2f, 64)
	if (context->tmpFlatteningPts.error) {
		amCtxDynResourcesDestroy(context);
		return AM_FALSE;
	}

	context->curvesQuality = confParams->curvesQuality;
	context->flattenParams.deviation = -1.0f;
	context->flattenParams.flatness = -1.0f;
	context->flattenParams.two_sqrt_flatness = -1.0f;
	context->flattenParams.three_over_flatness = -1.0f;
	context->flattenParams.two_sqrt_flatness_over_three = -1.0f;
	context->flattenParams.two_cuberoot_flatness_over_three = -1.0f;
	context->flattenParams.sixtyfour_flatness = -1.0f;
	context->flattenParams.degenerative_curve_segments = -1;

	// stroking structures and derived data
	context->strokeLineThickness = context->strokeLineWidth / 2.0f;
	context->miterMulThickness = context->miterLimit * context->strokeLineThickness;
	context->miterMulThicknessSqr = context->miterMulThickness * context->miterMulThickness;
#if defined(AM_FIXED_POINT_PIPELINE)
	AM_DYNARRAY_INIT_RESERVE(context->strokeAuxPts, AMVect2x, 128)
	AM_DYNARRAY_INIT_RESERVE(context->strokeAuxPtsDx, AMVect2x, 64)
#else
	AM_DYNARRAY_INIT_RESERVE(context->strokeAuxPts, AMVect2f, 128)
	AM_DYNARRAY_INIT_RESERVE(context->strokeAuxPtsDx, AMVect2f, 64)
#endif
	AM_DYNARRAY_INIT_RESERVE(context->strokeAuxPtsPerContour, AMint32, 16)
	if (context->strokeAuxPts.error || context->strokeAuxPtsDx.error || context->strokeAuxPtsPerContour.error) {
		amCtxDynResourcesDestroy(context);
		return AM_FALSE;
	}

	context->strokeAuxPtsOldSize = 0;
	AM_VECT2_SET(&context->strokeLeftPoint, 0.0f, 0.0f)
	AM_VECT2_SET(&context->strokeRightPoint, 0.0f, 0.0f)
	AM_VECT2_SET(&context->strokeMiddlePoint, 0.0f, 0.0f)
	context->lastJoinSeparated = AM_TRUE;
	AM_DYNARRAY_INIT_RESERVE(context->patchedDashPattern, VGfloat, 8)
	if (context->patchedDashPattern.error) {
		amCtxDynResourcesDestroy(context);
		return AM_FALSE;
	}
	context->dashPatternSum = 0.0f;

	// auxiliary arrays, used by VGU functions
	AM_DYNARRAY_INIT_RESERVE(context->vguAuxCommands, AMuint8, 64)
	if (context->vguAuxCommands.error) {
		amCtxDynResourcesDestroy(context);
		return AM_FALSE;
	}
	AM_DYNARRAY_INIT_RESERVE(context->vguAuxS8Data, AMint8, 64)
	if (context->vguAuxS8Data.error) {
		amCtxDynResourcesDestroy(context);
		return AM_FALSE;
	}
	AM_DYNARRAY_INIT_RESERVE(context->vguAuxS16Data, AMint16, 64)
	if (context->vguAuxS16Data.error) {
		amCtxDynResourcesDestroy(context);
		return AM_FALSE;
	}
	AM_DYNARRAY_INIT_RESERVE(context->vguAuxS32Data, AMint32, 64)
	if (context->vguAuxS32Data.error) {
		amCtxDynResourcesDestroy(context);
		return AM_FALSE;
	}
	AM_DYNARRAY_INIT_RESERVE(context->vguAuxF32Data, AMfloat, 64)
	if (context->vguAuxF32Data.error) {
		amCtxDynResourcesDestroy(context);
		return AM_FALSE;
	}

	// rasterizer structures and derived data
	if (!amRasCreate(&context->rasterizer)) {
		AM_DEBUG("Cannot allocate memory for the main rasterizer.");
		amCtxDynResourcesDestroy(context);
		return AM_FALSE;
	}

	// constant function tables
#if !defined(AM_LITE_PROFILE)
	amPxlConversionTableInit(context);
#endif
	amPathSegmentTablesInit(context);
#if defined(AM_SRE) && !defined(AM_LITE_PROFILE)
	amFilPathTableInit(context);
	amFilImageTableInit(context);
#endif

	// create default paint
	paint = (AMPaint *)amMalloc(sizeof(AMPaint));
	if (!paint) {
		AM_DEBUG("Cannot allocate memory for default paint.");
		amCtxDynResourcesDestroy(context);
		return AM_FALSE;
	}
	else {
		if (!amPaintInit(paint)) {
			amCtxDynResourcesDestroy(context);
			return AM_FALSE;
		}
		context->defaultPaint = amCtxHandleNew(context, paint);
		if (context->defaultPaint == VG_INVALID_HANDLE) {
			amFree(paint);
			amCtxDynResourcesDestroy(context);
			return AM_FALSE;
		}
	}

#if defined(AM_SRE) && defined(VG_MZT_conical_gradient)
	context->atan2Table = NULL;
	if (!amCtxAtan2TableInit(context)) {
		amFree(paint);
		amCtxDynResourcesDestroy(context);
		return AM_FALSE;
	}
#endif

	// get word size and endian type
	context->endianess = amSystemEndianessGet();
	context->wordSize = amSystemWordSizeGet();

#if defined(VG_MZT_statistics)
	context->statisticsInfo.flatteningPointsCount = 0;
	context->statisticsInfo.flatteningTimeMS = 0;
	context->statisticsInfo.triangulationTrianglesCount = 0;
	context->statisticsInfo.triangulationTimeMS = 0;
	context->statisticsInfo.strokerPointsCount = 0;
	context->statisticsInfo.strokerTimeMS = 0;
	context->statisticsInfo.glDrawElementsCount = 0;
	context->statisticsInfo.glDrawArraysCount = 0;
#endif

	context->initialized = AM_TRUE;
	return AM_TRUE;
}

/*!
	\brief Destroy a given OpenVG context, freeing allocated memory.
	\param context context to destroy.
*/
void amCtxDestroy(AMContext *context) {

	AMPaint *defaultPaint;

	AM_ASSERT(context);

	// we can destroy only contexts successfully initialized
	if (!context->initialized)
		return;

	// destroy defaultPaint
	defaultPaint = (AMPaint *)context->handles->createdHandlesList.data[context->defaultPaint];
	amPaintResourcesDestroy(defaultPaint, context);
	amCtxHandleRemove(context, context->defaultPaint);
	amFree(defaultPaint);

#if defined(AM_GLE) || defined(AM_GLS)
	// destroy the OpenGL / OpenGL ES context
	amGlCtxDestroy(&context->glContext);
#endif

	// destroy all handles and dynamic arrays
	amCtxDynResourcesDestroy(context);

	AM_MEMORY_LOG("vgDestroyContextAM");
#if defined(AM_DEBUG_MEMORY) && defined(AM_STANDALONE) && (defined(AM_OS_WIN) || defined(AM_OS_WINCE) || defined(AM_OS_LINUX))
	context->allocatedMemory = 0;
	if (context->memoryLog) {
		fclose(context->memoryLog);
		context->memoryLog = NULL;
	}
#endif

	context->initialized = AM_FALSE;
}

/*!
	\brief Get the oldest error code provided by an API call on the current context since the previous call
	to vgGetError on that context (or since the creation of the context).\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\return the oldest error code.
*/
VG_API_CALL VGErrorCode VG_API_ENTRY vgGetError(void) VG_API_EXIT {

	VGErrorCode err;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgGetError");
		OPENVG_RETURN(VG_NO_CONTEXT_ERROR)
	}

	err = currentContext->error;
	currentContext->error = VG_NO_ERROR;
	AM_MEMORY_LOG("vgGetError");
	OPENVG_RETURN(err)
}

/*!
	\brief It returns a value indicating whether a given setting of a property of a type given by key is
	generally accelerated in hardware on the currently running OpenVG implementation.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param key one of the following property key: VG_IMAGE_FORMAT_QUERY, VG_PATH_DATATYPE_QUERY.
	\param setting key value.
	\return one of the values VG_HARDWARE_ACCELERATED or VG_HARDWARE_UNACCELERATED.
*/
VG_API_CALL VGHardwareQueryResult VG_API_ENTRY vgHardwareQuery(VGHardwareQueryType key,
                                                              VGint setting) VG_API_EXIT {

	VGHardwareQueryResult res;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgHardwareQuery");
		OPENVG_RETURN(VG_HARDWARE_UNACCELERATED)
	}

	/* check for illegal arguments */
	if (key != VG_IMAGE_FORMAT_QUERY && key != VG_PATH_DATATYPE_QUERY) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgHardwareQuery");
		OPENVG_RETURN(VG_HARDWARE_UNACCELERATED)
	}

	switch (key) {

		case VG_IMAGE_FORMAT_QUERY:
		
			if (setting < VG_sRGBX_8888 || setting > VG_lABGR_8888_PRE) {
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				res = VG_HARDWARE_UNACCELERATED;
			}
			else {
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			#if defined(AM_GLE) || defined(AM_GLS)
				res = VG_HARDWARE_ACCELERATED;
			#else
				res = VG_HARDWARE_UNACCELERATED;
			#endif
			}
			break;

		case VG_PATH_DATATYPE_QUERY:
			if (setting < VG_PATH_DATATYPE_S_8 || setting > VG_PATH_DATATYPE_F) {
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				res = VG_HARDWARE_UNACCELERATED;
			}
			else {
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			#if defined(AM_GLE) || defined(AM_GLS)
				res = VG_HARDWARE_ACCELERATED;
			#else
				res = VG_HARDWARE_UNACCELERATED;
			#endif
			}
			break;

		default:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			res = VG_HARDWARE_UNACCELERATED;
			break;
	}
	AM_MEMORY_LOG("vgHardwareQuery");
	OPENVG_RETURN(res)
}

/*!
	\brief It returns information about the OpenVG implementation, including extension information.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param name one of the following string identifiers: VG_VENDOR, VG_RENDERER, VG_VERSION, VG_EXTENSIONS.
	\return the corresponding string.
*/
VG_API_CALL const VGubyte * VG_API_ENTRY vgGetString(VGStringID name) VG_API_EXIT {

#if defined(AM_SRE)
	#if defined(AM_LITE_PROFILE)
		#define AM_ENGINE_TYPE "SRE Lite"
	#else
	#define AM_ENGINE_TYPE "SRE"
	#endif
#elif defined(AM_GLE)
	#define AM_ENGINE_TYPE "GLE"
#elif defined(AM_GLS)
	#define AM_ENGINE_TYPE "GLS"
#else
	#error Unreachable point.
#endif

#if defined(AM_STANDALONE)
	#define AM_STANDALONE_ENGINE "standalone "
#else
	#define AM_STANDALONE_ENGINE ""
#endif

#if defined(AM_EVALUATE)
	#define AM_EVAL "(evaluation version)"
#else
	#define AM_EVAL ""
#endif

#if (AM_OPENVG_VERSION == 101)
	#define AM_VG_VERSION "1.0"
	#define AM_VG_FULL_VERSION "1.0.1"
#elif (AM_OPENVG_VERSION == 110)
	#define AM_VG_VERSION "1.1"
	#define AM_VG_FULL_VERSION "1.1.0"
#else
	#error Please define AM_OPENVG_VERSION.
#endif

	static const char vendor[] = "Mazatech S.r.l.";
	static const char version[] = AM_VG_VERSION;
	static const char renderer[] = "AmanithVG "AM_ENGINE_TYPE" "AM_STANDALONE_ENGINE""AM_VERSION_STR" "AM_EVAL"- OpenVG "AM_VG_FULL_VERSION;
	static const char extensions[] = ""
	#if defined(VG_MZT_separable_cap_style)
		"VG_MZT_separable_cap_style "
	#endif
	#if defined(VG_MZT_color_ramp_interpolation)
		"VG_MZT_color_ramp_interpolation "
	#endif
	#if defined(VG_MZT_conical_gradient)
		"VG_MZT_conical_gradient "
	#endif
	#if defined(VG_MZT_advanced_blend_modes)
		"VG_MZT_advanced_blend_modes "
	#endif
	#if defined(VG_MZT_separable_blend_modes)
		"VG_MZT_separable_blend_modes "
	#endif
	"";

	static const char emptyString[] = "";
	const VGubyte *res;

	switch (name) {
		case VG_VENDOR:
			res = (const VGubyte *)vendor;
			break;
		case VG_RENDERER:
			res = (const VGubyte *)renderer;
			break;
		case VG_VERSION:
			res = (const VGubyte *)version;
			break;
		case VG_EXTENSIONS:
			res = (const VGubyte *)extensions;
			break;
		default:
			res = (const VGubyte *)emptyString;
			break;
	}
	return res;

	#undef AM_ENGINE_TYPE
	#undef AM_EVAL
	#undef AM_VG_VERSION
	#undef AM_VG_FULL_VERSION
}

/*!
	\brief This function ensures that all outstanding requests on the current context will complete in finite time.
	vgFlush may return prior to the actual completion of all requests.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
*/
VG_API_CALL void VG_API_ENTRY vgFlush(void) VG_API_EXIT {

#if defined(AM_GLE) || defined(AM_GLS)
	amGlFlush();
#endif
}

/*!
	\brief This function forces all outstanding requests on the current context to complete, returning only when
	the last request has completed.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
*/
VG_API_CALL void VG_API_ENTRY vgFinish(void) VG_API_EXIT {

#if defined(AM_GLE) || defined(AM_GLS)
	amGlFinish();
#endif
}

#undef AM_OPENVG_CALLS_BEFORE_AVAILABLE_HANDLES_SORT

#if defined (RIM_VG_SRC)
#pragma pop
#endif


