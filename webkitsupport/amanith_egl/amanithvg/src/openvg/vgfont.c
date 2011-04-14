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
	\file vgfont.c
	\brief OpenVG fonts, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
    #pragma push
    #pragma arm
#endif

#if defined(AM_SRE)
	#include "sreprimitives.h"
#elif defined(AM_GLE)
	#include "gleprimitives.h"
#elif defined(AM_GLS)
	#include "glsprimitives.h"
#else
	#error Unreachable point.
#endif
#include "vgmatrix.h"

#if defined RIM_VG_SRC
    #define VG_API_ENTRY 
#endif


#if (AM_OPENVG_VERSION >= 110)

#include "vgfont.h"

AMint32 amFontGlyphsCount(const AMFont *font) {

	AM_ASSERT(font);

	return (AMint32)font->createdGlyphsDesc.size;
}

void amGlyphReset(AMGlyph *glyph) {

	AM_ASSERT(glyph);

	glyph->handle = VG_INVALID_HANDLE;
	AM_VECT2_SET(&glyph->origin, 0.0f, 0.0f)
	AM_VECT2_SET(&glyph->escapement, 0.0f, 0.0f)
	glyph->flags = 0;
}

AMbool amGlyphDescGet(AMGlyphDesc *glyphDesc,
					  AMint32 *indexInList,
					  const AMFont *font,
					  const AMuint32 glyphIndex) {

	AMuint32 count, count2;
	AMint32 i;
	AMGlyphDesc *mid, *first;

	AM_ASSERT(glyphDesc);
	AM_ASSERT(indexInList);
	AM_ASSERT(font);

	first = &(font->createdGlyphsDesc.data[0]);
	count = (AMuint32)font->createdGlyphsDesc.size;

	for (; 0 < count; )	{
		// divide and conquer, find half that contains answer
		count2 = count / 2;
		mid = first;
		mid += count2;
		if (mid->index < glyphIndex) {
			first = ++mid;
			count -= count2 + 1;
		}
		else
			count = count2;
	}
	i = (AMint32)(first - &(font->createdGlyphsDesc.data[0]));

	*indexInList = i;
	if (i < (AMint32)font->createdGlyphsDesc.size && font->createdGlyphsDesc.data[i].index == glyphIndex) {
		*glyphDesc = font->createdGlyphsDesc.data[i];
		return AM_TRUE;
	}
	return AM_FALSE;
}

AMGlyph *amGlyphGet(const AMFont *font,
					const AMGlyphDesc *glyphDesc) {

	AM_ASSERT(font);
	AM_ASSERT(glyphDesc);
	AM_ASSERT(glyphDesc->pool < font->glyphPools.size);
	AM_ASSERT(glyphDesc->poolIdx < font->glyphPools.data[glyphDesc->pool]->size);

	return &font->glyphPools.data[glyphDesc->pool]->data[glyphDesc->poolIdx];
}

AMGlyph *amGlyphNew(AMFont *font,
					AMGlyphDesc *glyphDesc,
					const AMuint32 glyphIndex) {

	AMGlyphPool *pool;
	AMGlyph *result;

    #if defined (RIM_VG_SRC)
        AMContext *currentContext = amCtxCurrentGet();
    #endif

	AM_ASSERT(font);
	AM_ASSERT(glyphDesc);

	// check if there are available glyph descriptors
	if (font->availableGlyphDesc.size > 0) {
		// extract an available glyph descriptor (previously deleted)
		font->availableGlyphDesc.size--;
		glyphDesc->index = glyphIndex;
		glyphDesc->pool = font->availableGlyphDesc.data[font->availableGlyphDesc.size].pool;
		glyphDesc->poolIdx = font->availableGlyphDesc.data[font->availableGlyphDesc.size].poolIdx;
		// extract the relative pool
		pool = font->glyphPools.data[glyphDesc->pool];
		AM_ASSERT(pool);
		// initialize the new glyph
		result = &pool->data[glyphDesc->poolIdx];
		amGlyphReset(result);
		return result;
	}

	if (font->glyphPools.data[font->glyphPools.size - 1]->size == AM_FONT_GLYPH_POOL_CAPACITY) {

		AMuint32 i;

		// try to allocate a new glyph pool
		pool = (AMGlyphPool *)amMalloc(sizeof(AMGlyphPool));
    #if defined (RIM_VG_SRC)
        if (!pool){
           amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
           return NULL;
        }
    #else
		if (!pool)
			return NULL;
    #endif
		// initialize all glyphs inside the allocated pool
		for (i = 0; i < AM_FONT_GLYPH_POOL_CAPACITY; ++i)
			amGlyphReset(&pool->data[i]);
		pool->size = 0;
		AM_DYNARRAY_PUSH_BACK(font->glyphPools, AMGlyphPoolPtr, pool)
		// check for memory errors
		if (font->glyphPools.error) {
			font->glyphPools.error = AM_DYNARRAY_NO_ERROR;
			amFree(pool);
			return NULL;
		}
	}
	else
		pool = font->glyphPools.data[font->glyphPools.size - 1];

	// fill glyph descriptor fields
	glyphDesc->index = glyphIndex;
	glyphDesc->pool = (AMuint16)(font->glyphPools.size - 1);
	glyphDesc->poolIdx = pool->size;
	// return the new glyph
	result = &pool->data[pool->size];
	amGlyphReset(result);
	pool->size++;
	return result;
}

void amFontDynResourcesInit(AMFont *font) {

	AM_ASSERT(font);

	AM_DYNARRAY_PREINIT(font->createdGlyphsDesc)
	AM_DYNARRAY_PREINIT(font->availableGlyphDesc)
	AM_DYNARRAY_PREINIT(font->glyphPools)
}

void amFontDynResourcesDestroy(AMFont *font) {

	AM_ASSERT(font);

	AM_DYNARRAY_DESTROY(font->glyphPools)
	// destroy glyphs descriptors
	AM_DYNARRAY_DESTROY(font->createdGlyphsDesc)
	AM_DYNARRAY_DESTROY(font->availableGlyphDesc)
}

/*!
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
*/
AMbool amFontInit(AMFont *font,
				  const AMint32 glyphCapacityHint) {

	AMuint32 i;
	AMGlyphPool *pool;

	AM_ASSERT(font);
	amFontDynResourcesInit(font);

	// allocate glyph descriptors
	if (glyphCapacityHint > 0) {
		AM_DYNARRAY_INIT_RESERVE(font->createdGlyphsDesc, AMGlyphDesc, glyphCapacityHint)
	}
	else {
		AM_DYNARRAY_INIT(font->createdGlyphsDesc, AMGlyphDesc)
	}
	// check for memory errors
	if (font->createdGlyphsDesc.error) {
		amFontDynResourcesDestroy(font);
		return AM_FALSE;
	}

	// initialize the list of available glyph descriptors, to be reused
	AM_DYNARRAY_INIT(font->availableGlyphDesc, AMGlyphDesc)
	if (font->availableGlyphDesc.error) {
		amFontDynResourcesDestroy(font);
		return AM_FALSE;
	}

	// allocate pools pointers
	AM_DYNARRAY_INIT(font->glyphPools, AMGlyphPoolPtr)
	if (font->glyphPools.error) {
		amFontDynResourcesDestroy(font);
		return AM_FALSE;
	}

	// allocate first pool
	font->glyphPools.data[0] = (AMGlyphPool *)amMalloc(sizeof(AMGlyphPool));
		// check for memory errors
	if (!font->glyphPools.data[0]) {
		amFontDynResourcesDestroy(font);
		return AM_FALSE;
	}

	pool = font->glyphPools.data[0];
	pool->size = 0;
	for (i = 0; i < AM_FONT_GLYPH_POOL_CAPACITY; ++i)
		amGlyphReset(&pool->data[i]);

	font->glyphPools.size = 1;
	font->id = AM_FONT_HANDLE_ID;
	font->type = AM_FONT_HANDLE_ID;
	font->referenceCounter = 1;
	return AM_TRUE;
}

// Destroy font resources.
void amFontResourcesDestroy(AMFont *font,
							AMContext *context) {

	AMuint32 i;

	AM_ASSERT(font);
	AM_ASSERT(context);

	for (i = 0; i < font->glyphPools.size; ++i) {

		AMuint32 j;
		AMGlyphPool *poolPtr = font->glyphPools.data[i];

		for (j = 0; j < AM_FONT_GLYPH_POOL_CAPACITY; ++j) {

			AMGlyph *glyph = &poolPtr->data[j];

			if (glyph->handle != VG_INVALID_HANDLE) {
				// decrement the previous handle reference counter
				AMhandle object = context->handles->createdHandlesList.data[glyph->handle];
				AM_ASSERT(object);
				if (amCtxHandleRefCounterDec(object, context) == 0) {
					if (glyph->flags & AM_GLYPH_IS_PATH)
						amPathRemove(context, (AMPath *)object);
					else {
						amCtxHandleRemove(context, glyph->handle);
						amFree(object);
					}
				}
			}
			glyph->handle = VG_INVALID_HANDLE;
		}

		AM_ASSERT(poolPtr);
		amFree(poolPtr);
	}

	amFontDynResourcesDestroy(font);
}

void amFontDestroy(AMFont *font,
				   AMContext *context) {

	// decrement reference counter, it possibly leads to resources deallocation
	amCtxHandleRefCounterDec(font, context);

	// sign that this font is not more valid
	font->id = AM_INVALID_HANDLE_ID;
}

/*!
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
*/
AMbool amGlyphToPathSet(AMFont *font,
						const AMuint32 glyphIndex,
						VGPath path,
						const AMbool isHinted,
						const AMfloat glyphOrigin[2],
						const AMfloat escapement[2],
						AMContext *context) {

	AMGlyphDesc glyphDesc;
	AMGlyph *glyph;
	AMint32 indexInList;
	AMhandle object;
	AMContextHandlesList *handles = context->handles;

	AM_ASSERT(font);
	AM_ASSERT(glyphOrigin);
	AM_ASSERT(escapement);
	AM_ASSERT(context);

	if (amGlyphDescGet(&glyphDesc, &indexInList, font, glyphIndex))
		glyph = amGlyphGet(font, &glyphDesc);
	else {
		glyph = amGlyphNew(font, &glyphDesc, glyphIndex);
		// check for out of memory error
		if (!glyph)
			return AM_FALSE;
		AM_DYNARRAY_INSERT(font->createdGlyphsDesc, AMGlyphDesc, indexInList, glyphDesc)
		// check for memory errors
		if (font->createdGlyphsDesc.error) {
			font->createdGlyphsDesc.error = AM_DYNARRAY_NO_ERROR;
			return AM_FALSE;
		}
	}

	AM_ASSERT(path < handles->createdHandlesList.size);
	if (path != VG_INVALID_HANDLE) {
		// increment path reference counter
		object = handles->createdHandlesList.data[path];
		AM_ASSERT(object);
		amCtxHandleRefCounterInc(object);
	}

	AM_ASSERT(glyph);
	AM_ASSERT(glyph->handle < handles->createdHandlesList.size);

	if (glyph->handle != VG_INVALID_HANDLE) {
		// decrement the previous handle reference counter
		object = handles->createdHandlesList.data[glyph->handle];
		AM_ASSERT(object);
		if (amCtxHandleRefCounterDec(object, context) == 0) {
			if (glyph->flags & AM_GLYPH_IS_PATH)
				amPathRemove(context, (AMPath *)object);
			else {
				amCtxHandleRemove(context, glyph->handle);
				amFree(object);
			}
		}
	}
	// fill glyph fields
	glyph->handle = path;
	AM_VECT2_SET(&glyph->origin, glyphOrigin[AM_X], glyphOrigin[AM_Y])
	AM_VECT2_SET(&glyph->escapement, escapement[AM_X], escapement[AM_Y])
	glyph->flags = (isHinted) ? AM_GLYPH_IS_PATH | AM_GLYPH_IS_HINTED : AM_GLYPH_IS_PATH;
	return AM_TRUE;
}

/*!
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
*/
AMbool amGlyphToImageSet(AMFont *font,
						 const AMuint32 glyphIndex,
						 VGImage image,
						 const AMfloat glyphOrigin[2],
						 const AMfloat escapement[2],
						 AMContext *context) {

	AMGlyphDesc glyphDesc;
	AMGlyph *glyph;
	AMint32 indexInList;
	AMhandle object;
	AMContextHandlesList *handles = context->handles;

	AM_ASSERT(font);
	AM_ASSERT(glyphOrigin);
	AM_ASSERT(escapement);
	AM_ASSERT(context);

	if (amGlyphDescGet(&glyphDesc, &indexInList, font, glyphIndex))
		glyph = amGlyphGet(font, &glyphDesc);
	else {
		glyph = amGlyphNew(font, &glyphDesc, glyphIndex);
		// check for out of memory error
		if (!glyph)
			return AM_FALSE;
		AM_DYNARRAY_INSERT(font->createdGlyphsDesc, AMGlyphDesc, indexInList, glyphDesc)
		// check for memory errors
		if (font->createdGlyphsDesc.error) {
			font->createdGlyphsDesc.error = AM_DYNARRAY_NO_ERROR;
			return AM_FALSE;
		}
	}

	AM_ASSERT(image < handles->createdHandlesList.size);
	if (image != VG_INVALID_HANDLE) {
		// increment image reference counter
		object = handles->createdHandlesList.data[image];
		AM_ASSERT(object);
		amCtxHandleRefCounterInc(object);
	}

	AM_ASSERT(glyph);
	AM_ASSERT(glyph->handle < handles->createdHandlesList.size);

	if (glyph->handle != VG_INVALID_HANDLE) {
		// decrement the previous handle reference counter
		object = handles->createdHandlesList.data[glyph->handle];
		AM_ASSERT(object);
		if (amCtxHandleRefCounterDec(object, context) == 0) {
			if (glyph->flags & AM_GLYPH_IS_PATH)
				amPathRemove(context, (AMPath *)object);
			else {
				amCtxHandleRemove(context, glyph->handle);
				amFree(object);
			}
		}
	}
	// fill glyph fields
	glyph->handle = image;
	AM_VECT2_SET(&glyph->origin, glyphOrigin[AM_X], glyphOrigin[AM_Y])
	AM_VECT2_SET(&glyph->escapement, escapement[AM_X], escapement[AM_Y])
	glyph->flags = 0;
	return AM_TRUE;
}

/*!
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
*/
AMbool amGlyphClear(AMFont *font,
					const AMGlyphDesc *glyphDesc,
					const AMint32 indexInList,
					AMContext *context) {

	AMGlyph *glyph;
	AMContextHandlesList *handles = context->handles;

	AM_ASSERT(font);
	AM_ASSERT(glyphDesc);
	AM_ASSERT(indexInList < (AMint32)font->createdGlyphsDesc.size);

	glyph = amGlyphGet(font, glyphDesc);
	AM_ASSERT(glyph);
	AM_ASSERT(glyph->handle < handles->createdHandlesList.size);

	if (glyph->handle != VG_INVALID_HANDLE) {
		// decrement the previous handle reference counter
		AMhandle object = handles->createdHandlesList.data[glyph->handle];
		AM_ASSERT(object);
		if (amCtxHandleRefCounterDec(object, context) == 0) {
			if (glyph->flags & AM_GLYPH_IS_PATH)
				amPathRemove(context, (AMPath *)object);
			else {
				amCtxHandleRemove(context, glyph->handle);
				amFree(object);
			}
		}
	}
	amGlyphReset(glyph);

	// copy the glyph descriptor inside the list of available glyph descriptors
	AM_DYNARRAY_PUSH_BACK(font->availableGlyphDesc, AMGlyphDesc, *glyphDesc)
	// erase the glyph descriptor
	AM_DYNARRAY_ERASE(font->createdGlyphsDesc, (AMuint32)indexInList)
	if (font->availableGlyphDesc.error) {
		font->availableGlyphDesc.error = AM_DYNARRAY_NO_ERROR;
		return AM_FALSE;
	}
	return AM_TRUE;
}

AMbool amGlyphDoDraw(AMContext *context,
					 AMDrawingSurface *surface,
					 AMFont *font,
					 const AMGlyphDesc *glyphDesc,
					 const AMfloat adjustmentX,
					 const AMfloat adjustmentY,
					 const VGbitfield paintModes,
					 const AMbool autoHinting,
					 const AMbool allowImageBlitPipeline) {

	AMGlyph *glyph;
	AMbool res;

	AM_ASSERT(context);
	AM_ASSERT(font);
	AM_ASSERT(glyphDesc);
	AM_ASSERT(!(context->glyphUserToSurfaceFlags & AM_MATRIX_SINGULAR));
	AM_ASSERT(amMatrix33fIsAffine(&context->glyphUserToSurface));
	(void)autoHinting;

	res = AM_TRUE;
	glyph = amGlyphGet(font, glyphDesc);
	AM_ASSERT(glyph);

	if (paintModes) {

		AMUserToSurfaceDesc userToSurfaceDesc;
		AMMatrix33f userToSurface, inverseUserToSurface;
		AMMatrix33f *glyphUserToSurface = &context->glyphUserToSurface;
		AMMatrix33f *invGlyphUserToSurface = &context->inverseGlyphUserToSurface;
		AMContextHandlesList *handles = context->handles;
	// calculate the overall translation for the glyph
		AMfloat tx = context->glyphOrigin[AM_X] - glyph->origin.x;
		AMfloat ty = context->glyphOrigin[AM_Y] - glyph->origin.y;

		// build user-to-surface matrix, including adjustments translation
		AM_MATRIX33_SET(&userToSurface,
						glyphUserToSurface->a[0][0], glyphUserToSurface->a[0][1], glyphUserToSurface->a[0][0] * tx + glyphUserToSurface->a[0][1] * ty + glyphUserToSurface->a[0][2],
						glyphUserToSurface->a[1][0], glyphUserToSurface->a[1][1], glyphUserToSurface->a[1][0] * tx + glyphUserToSurface->a[1][1] * ty + glyphUserToSurface->a[1][2],
						0.0f, 0.0f, 1.0f)

		// build inverse user-to-surface matrix, including inverse adjustments translation
		AM_MATRIX33_SET(&inverseUserToSurface,
						invGlyphUserToSurface->a[0][0], invGlyphUserToSurface->a[0][1], invGlyphUserToSurface->a[0][2] - tx,
						invGlyphUserToSurface->a[1][0], invGlyphUserToSurface->a[1][1], invGlyphUserToSurface->a[1][2] - ty,
						0.0f, 0.0f, 1.0f)

	// fill user-to-surface descriptor
	userToSurfaceDesc.userToSurface = &userToSurface;
#if defined(AM_FIXED_POINT_PIPELINE)
	amRasMatrixFToX(&userToSurfaceDesc.userToSurfacex, userToSurfaceDesc.userToSurface);
#endif
	userToSurfaceDesc.inverseUserToSurface = &inverseUserToSurface;
	userToSurfaceDesc.userToSurfaceScale = context->glyphUserToSurfaceScale;
	userToSurfaceDesc.flags = context->glyphUserToSurfaceFlags
						#if defined(AM_GLE)
							  | AM_MATRIX_MODELVIEW_MODIFIED
						#endif
					    ;
	userToSurfaceDesc.userToSurfaceAffine = AM_TRUE;

	if (glyph->handle) {
		if (glyph->flags & AM_GLYPH_IS_PATH) {
				// extract the path
			AMPath *path = (AMPath *)handles->createdHandlesList.data[glyph->handle];
			// draw the path
			AM_ASSERT(path);
			if (path->segments.size > 0)
			#if defined(AM_SRE)
				res = amSrePathDraw(context, surface, &userToSurfaceDesc, path, paintModes);
				if (!res) {
					if (context->beforePathRasterization) {
						// try to retrieve as much memory as possbile
						amMemMngRetrieve(context, AM_TRUE);
						// re-extract the path pointer, because it could be changed by amMemMngRetrieve
						path = (AMPath *)handles->createdHandlesList.data[glyph->handle];
						AM_ASSERT(path);
						// try to re-draw the path glyph
						res = amSrePathDraw(context, surface, &userToSurfaceDesc, path, paintModes);
					}
				}
			#elif defined(AM_GLE)
				res = amGlePathDraw(context, surface, &userToSurfaceDesc, path, paintModes);
			#elif defined(AM_GLS)
				res = amGlsPathDraw(context, surface, &userToSurfaceDesc, path, paintModes);
			#else
				#error Unreachable point.
			#endif
		}
		else {
				// extract the image
			AMImage *image = (AMImage *)handles->createdHandlesList.data[glyph->handle];
			// draw the image
			AM_ASSERT(image);

		#if defined(AM_SRE)
				#if !defined(AM_LITE_PROFILE)
					if (allowImageBlitPipeline && image->format == amSrfRealFormatGet(surface))
						res = amSreImageGlyphDraw(context, surface, &userToSurfaceDesc, image);
					else
				#endif
			res = amSreImageDraw(context, surface, &userToSurfaceDesc, image);
		#elif defined(AM_GLE)
			res = amGleImageDraw(context, surface, &userToSurfaceDesc, image);
		#elif defined(AM_GLS)
			res = amGlsImageDraw(context, surface, &userToSurfaceDesc, image);
		#else
			#error Unreachable point.
		#endif
		}
	}

	context->glyphUserToSurfaceFlags = userToSurfaceDesc.flags;
#if defined(AM_GLE)
	context->pathUserToSurfaceFlags |= AM_MATRIX_MODELVIEW_MODIFIED;
#if (AM_OPENVG_VERSION >= 110)
	context->imageUserToSurfaceFlags |= AM_MATRIX_MODELVIEW_MODIFIED;
#endif
#endif
	}

	// update glyph origin inside the context
	context->glyphOrigin[AM_X] += glyph->escapement.x + adjustmentX;
	context->glyphOrigin[AM_Y] += glyph->escapement.y + adjustmentY;
	return res;
}

AMbool amGlyphDraw(AMContext *context,
				   AMDrawingSurface *surface,
				   AMFont *font,
				   const AMuint32 glyphIndex,
				   const VGbitfield paintModes,
				   const AMbool autoHinting) {

	AMGlyphDesc glyphDesc;
	AMint32 indexInList;
	AMMatrix33f *userToSurface;
	AMbool allowImageBlitPipeline;
	AMbool res;

	// update matrices and scale factors; now possible flags are a bitwise of the following values AM_MATRIX_SINGULAR, AM_MATRIX_MODELVIEW_MODIFIED (GLE)
	amMatricesUpdate(context);

	// if glyph matrix is singular, glyphs won't be drawn
	if (context->glyphUserToSurfaceFlags & AM_MATRIX_SINGULAR)
		return AM_TRUE;

	// check if the faster "blit" pipeline can be used to do image drawing
	userToSurface = &context->glyphUserToSurface;
	allowImageBlitPipeline = (context->imageMode == VG_DRAW_IMAGE_NORMAL &&
							  context->masking == VG_FALSE &&
							  (context->colorTransform == VG_FALSE || context->colorTransformHash == AM_COLOR_TRANSFORM_IDENTITY_HASH) &&
							  context->fillBlendMode == VG_BLEND_SRC_OVER &&
							  // no shear
							  amAbsf(userToSurface->a[0][1]) <= AM_EPSILON_FLOAT && amAbsf(userToSurface->a[1][0]) <= AM_EPSILON_FLOAT) ? AM_TRUE : AM_FALSE;

	amGlyphDescGet(&glyphDesc, &indexInList, font, glyphIndex);
	res = amGlyphDoDraw(context, surface, font, &glyphDesc, 0.0f, 0.0f, paintModes, autoHinting, allowImageBlitPipeline);
#if defined(AM_GLE)
	if (!res)
		amGlStatesRecover(context);
#endif
	return res;
}

AMbool amGlyphsDraw(AMContext *context,
					AMDrawingSurface *surface,
					AMFont *font,
					const AMuint32 *glyphIndices,
					const AMuint32 glyphCount,
					const AMfloat *adjustments_x,
					const AMfloat *adjustments_y,
					const VGbitfield paintModes,
					const AMbool autoHinting) {

	AMuint32 i;
	AMMatrix33f *userToSurface;
	AMbool allowImageBlitPipeline;

	AM_ASSERT(context);
	AM_ASSERT(font);
	AM_ASSERT(glyphIndices);
	AM_ASSERT(glyphCount > 0);

	// update matrices and scale factors; now possible flags are a bitwise of the following values AM_MATRIX_SINGULAR, AM_MATRIX_MODELVIEW_MODIFIED (GLE)
	amMatricesUpdate(context);

	// if glyph matrix is singular, glyphs won't be drawn
	if (context->glyphUserToSurfaceFlags & AM_MATRIX_SINGULAR)
		return AM_TRUE;

	// check if the faster "blit" pipeline can be used to do image drawing
	userToSurface = &context->glyphUserToSurface;
	allowImageBlitPipeline = (context->imageMode == VG_DRAW_IMAGE_NORMAL &&
							  context->masking == VG_FALSE &&
							  (context->colorTransform == VG_FALSE || context->colorTransformHash == AM_COLOR_TRANSFORM_IDENTITY_HASH) &&
							  context->fillBlendMode == VG_BLEND_SRC_OVER &&
							  // no shear
							  amAbsf(userToSurface->a[0][1]) <= AM_EPSILON_FLOAT && amAbsf(userToSurface->a[1][0]) <= AM_EPSILON_FLOAT) ? AM_TRUE : AM_FALSE;

	for (i = 0; i < glyphCount; ++i) {

		AMGlyphDesc glyphDesc;
		AMint32 indexInList;
		AMfloat adjustmentX = (adjustments_x) ? adjustments_x[i] : 0.0f;
		AMfloat adjustmentY = (adjustments_y) ? adjustments_y[i] : 0.0f;

		amGlyphDescGet(&glyphDesc, &indexInList, font, glyphIndices[i]);
		if (!amGlyphDoDraw(context, surface, font, &glyphDesc, adjustmentX, adjustmentY, paintModes, autoHinting, allowImageBlitPipeline)) {
		#if defined(AM_GLE)
			amGlStatesRecover(context);
		#endif
			return AM_FALSE;
		}
	}
	return AM_TRUE;
}

// *********************************************************************
//                        Public implementations
// *********************************************************************

/*!
	\brief It creates a new font object and returns a VGFont handle to it.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param glyphCapacityHint provides a hint as to the capacity of a VGFont, i.e., the total number of
	glyphs that this VGFont object will be required to accept.
	\return a VGFont handle if the operation was successful, or VG_INVALID_HANDLE if an error occurs.
*/
VG_API_CALL VGFont VG_API_ENTRY vgCreateFont(VGint glyphCapacityHint) VG_API_EXIT {

	AMFont *fnt;
	VGFont handle;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgCreateFont");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}

	// check for illegal arguments
	if (glyphCapacityHint < 0) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgCreateFont");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}
	// allocate a new font structure
	fnt = (AMFont *)amMalloc(sizeof(AMFont));
	if (!fnt) {
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgCreateFont");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}
	// initialize the font structure
	if (!amFontInit(fnt, glyphCapacityHint)) {
		AM_MEMORY_LOG("vgCreateFont (amFontInit fail, now try to recover memory)");
		// try to retrieve as much memory as possbile
		amMemMngRetrieve(currentContext, AM_TRUE);
		// try to re-initialize the font
		if (!amFontInit(fnt, glyphCapacityHint)) {
		amFree(fnt);
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgCreateFont");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}
	}
	// add the new paint to the internal handles list of the context
	handle = amCtxHandleNew(currentContext, (AMhandle)fnt);
	if (handle == VG_INVALID_HANDLE) {
		amFontDestroy(fnt, currentContext);
		amFree(fnt);
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgCreateFont");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}
	// decrement the counter of the memory manager
	amCtxMemMngCountDown(currentContext);
	// exit without errors
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgCreateFont");
	OPENVG_RETURN(handle)
}

/*!
	\brief It destroys the VGFont object pointed to by the font argument.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param font the font to destroy.
*/
VG_API_CALL void VG_API_ENTRY vgDestroyFont(VGFont font) VG_API_EXIT {

	AMFont *fnt;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgDestroyFont");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for bad handle
	if (amCtxHandleValid(currentContext, font) != AM_FONT_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgDestroyFont");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	fnt = (AMFont *)currentContext->handles->createdHandlesList.data[font];
	AM_ASSERT(fnt);
	
	amFontDestroy(fnt, currentContext);
	if (fnt->referenceCounter == 0) {
		// remove font object from context internal list, and free associated pointer
		amCtxHandleRemove(currentContext, font);
		amFree(fnt);
	}
	// decrement the counter of the memory manager
	amCtxMemMngCountDown(currentContext);
	// exit without errors
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgDestroyFont");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

VG_API_CALL void VG_API_ENTRY vgSetGlyphToPath(VGFont font,
                                               VGuint glyphIndex,
                                               VGPath path,
                                               VGboolean isHinted,
                                               const VGfloat glyphOrigin[2],
											   const VGfloat escapement[2]) VG_API_EXIT {

	AMFont *fnt;
	AMbool hinted = (isHinted == VG_TRUE) ? AM_TRUE : AM_FALSE;
	AMfloat fixedOrigin[2], fixedEscapement[2];
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgSetGlyphToPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for bad handle
	if ((amCtxHandleValid(currentContext, font) != AM_FONT_HANDLE_ID) ||
		(path != VG_INVALID_HANDLE && amCtxHandleValid(currentContext, path) != AM_PATH_HANDLE_ID)) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgSetGlyphToPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	// check for illegal arguments
	if (!glyphOrigin || !escapement || !amPointerIsAligned(glyphOrigin, 4) || !amPointerIsAligned(escapement, 4)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgSetGlyphToPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	fnt = (AMFont *)currentContext->handles->createdHandlesList.data[font];
	AM_ASSERT(fnt);

	// handle NaN and Inf values
	fixedOrigin[0] = amNanInfFix(glyphOrigin[0]);
	fixedOrigin[1] = amNanInfFix(glyphOrigin[1]);
	fixedEscapement[0] = amNanInfFix(escapement[0]);
	fixedEscapement[1] = amNanInfFix(escapement[1]);

	// do the real set operation taking care of out of memory error
	if (!amGlyphToPathSet(fnt, glyphIndex, path, hinted, fixedOrigin, fixedEscapement, currentContext)) {
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgSetGlyphToPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgSetGlyphToPath");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

VG_API_CALL void VG_API_ENTRY vgSetGlyphToImage(VGFont font,
                                                VGuint glyphIndex,
                                                VGImage image,
                                                const VGfloat glyphOrigin[2],
											    const VGfloat escapement[2]) VG_API_EXIT {

	AMFont *fnt;
	AMfloat fixedOrigin[2], fixedEscapement[2];
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgSetGlyphToImage");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for bad handle
	if ((amCtxHandleValid(currentContext, font) != AM_FONT_HANDLE_ID) ||
		(image != VG_INVALID_HANDLE && amCtxHandleValid(currentContext, image) != AM_IMAGE_HANDLE_ID)) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgSetGlyphToImage");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	// check for illegal arguments
	if (!glyphOrigin || !escapement || !amPointerIsAligned(glyphOrigin, 4) || !amPointerIsAligned(escapement, 4)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgSetGlyphToImage");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

#if !defined(AM_STANDALONE) || defined(RIM_VG_SRC) || defined(TORCH_VG_SRC)
{
	AMImage *img = (AMImage *)currentContext->handles->createdHandlesList.data[image];
	if (img && img->inUseByEgl) {
		amCtxErrorSet(currentContext, VG_IMAGE_IN_USE_ERROR);
		AM_MEMORY_LOG("vgSetGlyphToImage");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
}
#endif

	fnt = (AMFont *)currentContext->handles->createdHandlesList.data[font];
	AM_ASSERT(fnt);

	// handle NaN and Inf values
	fixedOrigin[0] = amNanInfFix(glyphOrigin[0]);
	fixedOrigin[1] = amNanInfFix(glyphOrigin[1]);
	fixedEscapement[0] = amNanInfFix(escapement[0]);
	fixedEscapement[1] = amNanInfFix(escapement[1]);

	// do the real set operation taking care of out of memory error
	if (!amGlyphToImageSet(fnt, glyphIndex, image, fixedOrigin, fixedEscapement, currentContext)) {
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgSetGlyphToImage");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgSetGlyphToImage");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

VG_API_CALL void VG_API_ENTRY vgClearGlyph(VGFont font,
										   VGuint glyphIndex) VG_API_EXIT {

	AMGlyphDesc glyphDesc;
	AMint32 indexInList;
	AMFont *fnt;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgClearGlyph");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for bad handle
	if (amCtxHandleValid(currentContext, font) != AM_FONT_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgClearGlyph");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	fnt = (AMFont *)currentContext->handles->createdHandlesList.data[font];
	AM_ASSERT(fnt);

	// check for illegal arguments
	if (!amGlyphDescGet(&glyphDesc, &indexInList, fnt, glyphIndex)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgClearGlyph");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	if (!amGlyphClear(fnt, &glyphDesc, indexInList, currentContext))
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
	else
		amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgClearGlyph");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

VG_API_CALL void VG_API_ENTRY vgDrawGlyph(VGFont font,
                                          VGuint glyphIndex,
                                          VGbitfield paintModes,
										  VGboolean allowAutoHinting) VG_API_EXIT {

	AMGlyphDesc glyphDesc;
	AMint32 indexInList;
	AMFont *fnt;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;
	AMbool autoHinting = (allowAutoHinting == VG_TRUE) ? AM_TRUE : AM_FALSE;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgClear");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for bad handle
	if (amCtxHandleValid(currentContext, font) != AM_FONT_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgDrawGlyph");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	fnt = (AMFont *)currentContext->handles->createdHandlesList.data[font];
	AM_ASSERT(fnt);

	// check for illegal arguments
	if ((paintModes & ~(VG_FILL_PATH | VG_STROKE_PATH)) || !amGlyphDescGet(&glyphDesc, &indexInList, fnt, glyphIndex)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgDrawGlyph");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	if (!amGlyphDraw(currentContext, currentSurface, fnt, glyphIndex, paintModes, autoHinting)) {
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgDrawGlyph");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	// decrement the counter of the memory manager
	amCtxMemMngCountDown(currentContext);
	// exit without errors
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgDrawGlyph");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

VG_API_CALL void VG_API_ENTRY vgDrawGlyphs(VGFont font,
                                           VGint glyphCount,
                                           const VGuint *glyphIndices,
                                           const VGfloat *adjustments_x,
                                           const VGfloat *adjustments_y,
                                           VGbitfield paintModes,
										   VGboolean allowAutoHinting) VG_API_EXIT {

	AMint32 i;
	AMFont *fnt;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;
	AMbool autoHinting = (allowAutoHinting == VG_TRUE) ? AM_TRUE : AM_FALSE;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgClear");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for bad handle
	if (amCtxHandleValid(currentContext, font) != AM_FONT_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgDrawGlyphs");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for illegal arguments
	if (glyphCount <= 0 || !glyphIndices || !amPointerIsAligned(glyphIndices, 4) ||
		(adjustments_x && !amPointerIsAligned(adjustments_x, 4)) ||
		(adjustments_y && !amPointerIsAligned(adjustments_y, 4)) ||
		(paintModes & ~(VG_FILL_PATH | VG_STROKE_PATH))) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgDrawGlyphs");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	fnt = (AMFont *)currentContext->handles->createdHandlesList.data[font];
	AM_ASSERT(fnt);

	// check if glyph indices have been defined in the given font object
	for (i = 0; i < glyphCount; ++i) {

		AMGlyphDesc glyphDesc;
		AMint32 indexInList;

		if (!amGlyphDescGet(&glyphDesc, &indexInList, fnt, glyphIndices[i])) {
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			AM_MEMORY_LOG("vgDrawGlyphs");
			OPENVG_RETURN(OPENVG_NO_RETVAL)
		}
	}

	if (!amGlyphsDraw(currentContext, currentSurface, fnt, glyphIndices, glyphCount, adjustments_x, adjustments_y, paintModes, autoHinting)) {
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgDrawGlyphs");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	// decrement the counter of the memory manager
	amCtxMemMngCountDown(currentContext);
	// exit without errors
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgDrawGlyphs");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

#endif

#if defined (RIM_VG_SRC)
    #pragma pop
#endif

