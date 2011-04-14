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
	\file vgprimitives.c
	\brief Path and image drawing functions entry point, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "vgprimitives.h"
#if defined(AM_SRE)
	#include "sreprimitives.h"
	#include "sredrawingsurface.h"  // for the optimized amSreDrawingSurfacePixelsSetInvertedY
#elif defined(AM_GLE)
	#include "gleprimitives.h"
#elif defined(AM_GLS)
	#include "glsprimitives.h"
#else
	#error Unreachable point.
#endif
#include "vgdrawingsurface.h"
#include "vgmatrix.h"

#if defined RIM_VG_SRC
#include "vgimage.h"
#endif

#if defined (RIM_VG_SRC)
extern AMbool amDrawingSurfacePixelsBlend(AMContext *context,
								 AMDrawingSurface *surface,
								 AMint32 dx,
								 AMint32 dy,
								 VGImage src,
								 AMint32 sx,
								 AMint32 sy,
								 AMint32 width,
								 AMint32 height,
								 AMbool blend,
								 AMbool flip);
#endif

/*!
	Update an axes-aligned box, according to a specified affine matrix.
	\param dst output (transformed) axes-aligned box.
	\param src input axes-aligned box.
	\param matrix affine transformation matrix.
*/
void amAABox2fTransform(AMAABox2f *dst,
						const AMAABox2f *src,
						const AMMatrix33f *matrix) {

	AMVect2f p0, p1, p2, p3;
	
	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(matrix);

	p0 = src->minPoint;
	p2 = src->maxPoint;
	p1.x = p2.x;
	p1.y = p0.y;
	p3.x = p0.x;
	p3.y = p2.y;

	AM_MATRIX33_VECT2_SELF_MUL(matrix, &p0, AMVect2f)
	AM_MATRIX33_VECT2_SELF_MUL(matrix, &p1, AMVect2f)
	AM_MATRIX33_VECT2_SELF_MUL(matrix, &p2, AMVect2f)
	AM_MATRIX33_VECT2_SELF_MUL(matrix, &p3, AMVect2f)

	AM_AABOX2_SET(dst, &p0, &p1)
	AM_AABOX2_EXTEND_TO_INCLUDE(dst, &p2)
	AM_AABOX2_EXTEND_TO_INCLUDE(dst, &p3)
}

#if defined(AM_FIXED_POINT_PIPELINE)
// Update an axes-aligned box, according to a specified fixed point matrix.
void amAABox2fTransformx(AMAABox2x *dst,
						 const AMAABox2f *src,
						 const AMMatrix33x *matrix) {

	AMVect2x p0, p1, p2, p3;
	AMVect2x q0, q1, q2, q3;

	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(matrix);

	AM_VECT2_SET(&p0, amFloatToFixed1616(src->minPoint.x), amFloatToFixed1616(src->minPoint.y))
	AM_VECT2_SET(&p2, amFloatToFixed1616(src->maxPoint.x), amFloatToFixed1616(src->maxPoint.y))
	AM_VECT2_SET(&p1, p2.x, p0.y)
	AM_VECT2_SET(&p3, p0.x, p2.y)

	amRasVertexTransform(&q0, &p0, matrix);
	amRasVertexTransform(&q1, &p1, matrix);
	amRasVertexTransform(&q2, &p2, matrix);
	amRasVertexTransform(&q3, &p3, matrix);

	AM_AABOX2_SET(dst, &q0, &q1)
	AM_AABOX2_EXTEND_TO_INCLUDE(dst, &q2)
	AM_AABOX2_EXTEND_TO_INCLUDE(dst, &q3)
}
#endif

/*!
	\brief Update flattening parameters inside the context, according to the current VG_MATRIX_PATH_USER_TO_SURFACE	matrix and stroke line thickness.
	\param context context containing (output) flattening parameters.
	\param path used to check for paths made of straight lines only.
	\param paintModes used to check if path must be stroked.
	\param srfSpaceBox path box, in surface space.
	\param userToSurfaceDesc descriptor for the "user to surface" matrix that will be applied during rendering operations, for the specifed path.
*/
void amPathDeviationUpdate(AMContext *context,
						   const AMPath *path,
						   const VGbitfield paintModes,
						   const AMAABox2i *srfSpaceBox,
						   const AMUserToSurfaceDesc *userToSurfaceDesc) {

	// calculate the global deviation
	AMfloat maxScale = AM_MAX(userToSurfaceDesc->userToSurfaceScale[AM_X], userToSurfaceDesc->userToSurfaceScale[AM_Y]);
	AMfloat res = context->curvesQuality / (maxScale * maxScale);

	if ((paintModes & VG_STROKE_PATH) && context->strokeLineThickness > 1.0f) {

		AMfloat boxW = (AMfloat)(AM_AABOX2_WIDTH(srfSpaceBox));
		AMfloat boxH = (AMfloat)(AM_AABOX2_HEIGHT(srfSpaceBox));
		AMfloat d = AM_MIN(boxW, boxH);

		if (d > AM_EPSILON_FLOAT) {
			AMfloat t = 1.0f + context->strokeLineThickness / d;

			t = 1.0f / (t * t);
			if (context->dashPatternSum > 0.0f)
				t *= 1.0f / context->strokeLineThickness;
			res *= t;
		}
	}

	// if the pathUserToSurface matrix has changed, update all flattening derived parameters
	// NB: avoid to recalculate these constants if path is a polygon
	if (context->flattenParams.deviation <= 0.0f ||
		(res != context->flattenParams.deviation && !(path->flags & AM_PATH_MADE_OF_LINES))) {
		context->flattenParams.deviation = res;
		context->flattenParams.flatness = amSqrtf(context->flattenParams.deviation);
		context->flattenParams.two_sqrt_flatness = 2.0f * amSqrtf(context->flattenParams.flatness);
		context->flattenParams.three_over_flatness = 3.0f / context->flattenParams.flatness;
		context->flattenParams.two_sqrt_flatness_over_three = 2.0f * amSqrtf(context->flattenParams.flatness / 3.0f);
		context->flattenParams.two_cuberoot_flatness_over_three = 2.0f * amPowf(context->flattenParams.flatness / 3.0f, 1.0f / 3.0f);
		context->flattenParams.sixtyfour_flatness = 64.0f * context->flattenParams.flatness;
		context->flattenParams.degenerative_curve_segments = (AMint32)(1.0f / (3.0f * amSqrtf(amSqrtf(context->flattenParams.flatness)))) + 1;
	}
}

/*!
	\brief Check if an image can be drawn.
	\param srfSpaceBox output surface space box of the image.
	\param q0 first (output) image corner transformed according to the current VG_MATRIX_IMAGE_USER_TO_SURFACE matrix.
	\param q1 second (output) image corner transformed according to the current VG_MATRIX_IMAGE_USER_TO_SURFACE matrix.
	\param q2 third (output) image corner transformed according to the current VG_MATRIX_IMAGE_USER_TO_SURFACE matrix.
	\param q3 fourth (output) image corner transformed according to the current VG_MATRIX_IMAGE_USER_TO_SURFACE matrix.
	\param image input image to check.
	\param userToSurfaceDesc descriptor for the "user to surface" matrix that will be applied during rendering operations, for the specifed image.
	\return AM_TRUE if image can be drawn, else AM_FALSE.
*/
AMbool amImageToDraw(AMAABox2i *srfSpaceBox,
					 AMVect2f *q0,
					 AMVect2f *q1,
					 AMVect2f *q2,
					 AMVect2f *q3,
					 const AMImage *image,
					 const AMUserToSurfaceDesc *userToSurfaceDesc) {

	#define TRANSFORM_CORNER(_dst, _src, _m) \
		(_dst)->x = (_m)->a[0][0] * (_src).x + (_m)->a[0][1] * (_src).y + (_m)->a[0][2]; \
		(_dst)->y = (_m)->a[1][0] * (_src).x + (_m)->a[1][1] * (_src).y + (_m)->a[1][2]; \
		z = (_m)->a[2][0] * (_src).x + (_m)->a[2][1] * (_src).y + (_m)->a[2][2]; \
		if (z <= AM_EPSILON_FLOAT) \
			return AM_FALSE; \
		else { \
		   (_dst)->x /= z; \
		   (_dst)->y /= z; \
		}

	AMVect2f p0, p1, p2, p3;
	AMAABox2f tmpBox;
	AMfloat z;

	AM_ASSERT(srfSpaceBox);
	AM_ASSERT(image);
	AM_ASSERT(image->width > 0 && image->height > 0);
	AM_ASSERT(userToSurfaceDesc);

	if (userToSurfaceDesc->flags & AM_MATRIX_SINGULAR)
		return AM_FALSE;

	AM_VECT2_SET(&p0, 0.0f, 0.0f)
	AM_VECT2_SET(&p2, (AMfloat)image->width, (AMfloat)image->height)
	AM_VECT2_SET(&p1, p2.x, p0.y)
	AM_VECT2_SET(&p3, p0.x, p2.y)

	TRANSFORM_CORNER(q0, p0, userToSurfaceDesc->userToSurface)
	TRANSFORM_CORNER(q1, p1, userToSurfaceDesc->userToSurface)
	TRANSFORM_CORNER(q2, p2, userToSurfaceDesc->userToSurface)
	TRANSFORM_CORNER(q3, p3, userToSurfaceDesc->userToSurface)

	AM_AABOX2_SET(&tmpBox, q0, q1)
	AM_AABOX2_EXTEND_TO_INCLUDE(&tmpBox, q2)
	AM_AABOX2_EXTEND_TO_INCLUDE(&tmpBox, q3)

	// align drawing surface space box to its nearest integer pixel
	tmpBox.minPoint.x = amFloorf(tmpBox.minPoint.x);
	tmpBox.minPoint.y = amFloorf(tmpBox.minPoint.y);
	tmpBox.maxPoint.x = amCeilf(tmpBox.maxPoint.x);
	tmpBox.maxPoint.y = amCeilf(tmpBox.maxPoint.y);
	AM_SRFSPACE_CORNER_CLAMP(srfSpaceBox->minPoint, tmpBox.minPoint)
	AM_SRFSPACE_CORNER_CLAMP(srfSpaceBox->maxPoint, tmpBox.maxPoint)
	return AM_TRUE;

	#undef TRANSFORM_CORNER
}

/*!
	\brief Entry point for path drawing function.
	\param context context containing all OpenVG states.
	\param surface the drawing surface where the path is going to be drawn to.
	\param path the path to draw.
	\param paintModes paint modes to use (fill and/or stroke).
*/
AMbool amPathDraw(AMContext *context,
				  AMDrawingSurface *surface,
				  AMPath *path,
				  const VGbitfield paintModes) {

	AMbool res;
	AMUserToSurfaceDesc userToSurfaceDesc;

	// if the path has no segments, just exit
	if (path->segments.size == 0)
		return AM_TRUE;

	// update matrices and scale factors; now possible flags are a bitwise of the following values
	// AM_MATRIX_SINGULAR, AM_MATRIX_MODELVIEW_MODIFIED (GLE)
	amMatricesUpdate(context);

	// if path matrix is singular, path won't be drawn
	if (context->pathUserToSurfaceFlags & AM_MATRIX_SINGULAR)
		return AM_TRUE;

	// fill userToSurfaceDesc fields
	userToSurfaceDesc.userToSurface = &context->pathUserToSurface;
#if defined(AM_FIXED_POINT_PIPELINE)
	amRasMatrixFToX(&userToSurfaceDesc.userToSurfacex, userToSurfaceDesc.userToSurface);
#endif
	userToSurfaceDesc.inverseUserToSurface = &context->inversePathUserToSurface;
	userToSurfaceDesc.userToSurfaceScale = context->pathUserToSurfaceScale;
	userToSurfaceDesc.flags = context->pathUserToSurfaceFlags;
	userToSurfaceDesc.userToSurfaceAffine = AM_TRUE;

#if defined(AM_SRE)
	res = amSrePathDraw(context, surface, &userToSurfaceDesc, path, paintModes);
#elif defined(AM_GLE)
	res = amGlePathDraw(context, surface, &userToSurfaceDesc, path, paintModes);
	if (!res)
		amGlStatesRecover(context);
#elif defined(AM_GLS)
	res = amGlsPathDraw(context, surface, &userToSurfaceDesc, path, paintModes);
#else
	#error Unreachable point.
#endif

	// update flags (at the moment only AM_MATRIX_MODELVIEW_MODIFIED bit can be modified)
	context->pathUserToSurfaceFlags = userToSurfaceDesc.flags;
#if defined(AM_GLE)
	context->imageUserToSurfaceFlags |= AM_MATRIX_MODELVIEW_MODIFIED;
	#if (AM_OPENVG_VERSION >= 110)
		context->glyphUserToSurfaceFlags |= AM_MATRIX_MODELVIEW_MODIFIED;
	#endif
#endif

	return res;
}

/*!
	\brief Entry point for image drawing function.
	\param context context containing all OpenVG states.
	\param surface the drawing surface where the image is going to be drawn to.
	\param handle the image (handle) to draw.
*/
AMbool amImageDraw(AMContext *context,
				   AMDrawingSurface *surface,
				   VGImage handle) {

	AMImage *image = (AMImage *)context->handles->createdHandlesList.data[handle];
	AMUserToSurfaceDesc userToSurfaceDesc;
	AMbool res;
#if defined(AM_SRE)
	AMbool a11EqualMinusOne, a11EqualOne;
#endif

#if defined(AM_SRE)
#if defined(RIM_VG_SRC) 
    #if defined(RIM_OPENVG_USES_QC_7KMDP_BLT)
	    #define SURFACE_PIXELS_SET_SRC_MDP \
                if (a11EqualOne) \
                    /* amSreDrawingSurfacePixelsSet is not called directly because we need to do bounding box clipping against the drawing surface regtion */ \
                    res = amDrawingSurfacePixelsBlend(context, surface, a02, a12, handle, 0, 0, image->width, image->height, AM_FALSE, AM_FALSE); \
                else { \
                    AM_ASSERT(a11EqualMinusOne); \
                    res = amDrawingSurfacePixelsBlend(context, surface, a02, a12, handle, 0, 0, image->width, image->height, AM_FALSE, AM_TRUE); \
                }   
    	#define SURFACE_PIXELS_SET_SRC_OVER \
                if (a11EqualOne) \
                    res = amDrawingSurfacePixelsBlend(context, surface, a02, a12, handle, 0, 0, image->width, image->height, AM_TRUE, AM_FALSE); \
                else { \
                    AM_ASSERT(a11EqualMinusOne); \
                    res = amDrawingSurfacePixelsBlend(context, surface, a02, a12, handle, 0, 0, image->width, image->height, AM_TRUE, AM_TRUE); \
                }
        #define SURFACE_PIXELS_SET_SRC \
            	if (a11EqualOne) \
            		/* amSreDrawingSurfacePixelsSet is not called directly because we need to do bounding box clipping against the drawing surface regtion */ \
            		res = amDrawingSurfacePixelsSet(context, surface, a02, a12, handle, 0, 0, image->width, image->height); \
            	else { \
            		AM_ASSERT(a11EqualMinusOne); \
            		res = amSreDrawingSurfacePixelsSetInvertedY(context, surface, a02, a12 - image->height, handle, 0, 0, image->width, image->height); \
            	}
    #else
    	#define SURFACE_PIXELS_SET_SRC \
    		if (a11EqualOne) \
    			/* amSreDrawingSurfacePixelsSet is not called directly because we need to do bounding box clipping against the drawing surface regtion */ \
    			res = amDrawingSurfacePixelsSet(context, surface, a02, a12, handle, 0, 0, image->width, image->height); \
    		else { \
    			AM_ASSERT(a11EqualMinusOne); \
    			res = amSreDrawingSurfacePixelsSetInvertedY(context, surface, a02, a12 - image->height, handle, 0, 0, image->width, image->height); \
    		}
    	#define SURFACE_PIXELS_SET_SRC_OVER \
    		if (a11EqualOne) \
    			res = amSreDrawingSurfacePixelsBlend(context, surface, a02, a12, handle, 0, 0, image->width, image->height); \
    		else { \
    			AM_ASSERT(a11EqualMinusOne); \
    			res = amSreDrawingSurfacePixelsBlendInvertedY(context, surface, a02, a12 - image->height, handle, 0, 0, image->width, image->height); \
    		}
    #endif
#endif
#endif



	// update matrices and scale factors
	amMatricesUpdate(context);

#if defined(AM_SRE)
	a11EqualOne = (amAbsf(1.0f - context->imageUserToSurface.a[1][1]) <= AM_EPSILON_FLOAT);
	a11EqualMinusOne = (amAbsf(1.0f + context->imageUserToSurface.a[1][1]) <= AM_EPSILON_FLOAT);

	// under some conditions, we can speed up drawImage using setPixels
	if (context->imageMode == VG_DRAW_IMAGE_NORMAL &&
		context->masking == VG_FALSE &&
		#if defined(RIM_VG_SRC) && defined (RIM_OPENVG_USES_QC_7KMDP_BLT)
		    (context->scissoring == VG_FALSE || (context->scissoring == VG_TRUE && context->scissorRects.size == 4)) &&
		#else
		    context->scissoring == VG_FALSE &&
                #endif
	#if (AM_OPENVG_VERSION >= 110)
		(context->colorTransform == VG_FALSE || context->colorTransformHash == AM_COLOR_TRANSFORM_IDENTITY_HASH) &&
	#endif
		(context->fillBlendMode == VG_BLEND_SRC || context->fillBlendMode == VG_BLEND_SRC_OVER) &&
		// check for identity roto-scale
		(a11EqualOne || a11EqualMinusOne) &&
		amAbsf(1.0f - context->imageUserToSurface.a[0][0]) <= AM_EPSILON_FLOAT &&
		amAbsf(       context->imageUserToSurface.a[0][1]) <= AM_EPSILON_FLOAT &&
		amAbsf(       context->imageUserToSurface.a[1][0]) <= AM_EPSILON_FLOAT &&
		// check for affinity
		amAbsf(       context->imageUserToSurface.a[2][0]) <= AM_EPSILON_FLOAT &&
		amAbsf(       context->imageUserToSurface.a[2][1]) <= AM_EPSILON_FLOAT &&
		amAbsf(1.0f - context->imageUserToSurface.a[2][2]) <= AM_EPSILON_FLOAT) {

		if (context->renderingQuality != VG_RENDERING_QUALITY_NONANTIALIASED) {
			// subPixelPrecision = 1 / 64
			const AMfloat subPixelPrecision =  0.015625f;
			AMint32 a02 = (AMint32)context->imageUserToSurface.a[0][2];
			AMint32 a12 = (AMint32)context->imageUserToSurface.a[1][2];

			// check for integer translations
			if (amAbsf((AMfloat)a02 - context->imageUserToSurface.a[0][2]) < subPixelPrecision  &&
				amAbsf((AMfloat)a12 - context->imageUserToSurface.a[1][2]) < subPixelPrecision) {
                #if defined (RIM_VG_SRC)
                    if (context->fillBlendMode == VG_BLEND_SRC || amImageIsOpaque(image)) {
                       #if defined(RIM_OPENVG_USES_QC_7KMDP_BLT)
                           if (image->format == amSrfRealFormatGet(surface)) {
                               SURFACE_PIXELS_SET_SRC_MDP
                               return res;
                           }else{    
                               SURFACE_PIXELS_SET_SRC
                               return res;
                           }
                       #else 
                           SURFACE_PIXELS_SET_SRC
                           return res;
                       #endif
                    }
                    else
                      if (image->format == amSrfRealFormatGet(surface)) {
                         #if defined(AM_LITE_PROFILE)
                            // in the lite profile, drawing surface has no alpha; in this case we suppose image format = surface format, so even the image has no alpha
                            SURFACE_PIXELS_SET_SRC
                         #else
                            SURFACE_PIXELS_SET_SRC_OVER
                         #endif
                            return res;
                      }
                #endif
			}
		}
		else {
			// find the integer pixel
			AMint32 a02 = (AMint32)amFloorf(context->imageUserToSurface.a[0][2] + 0.5f);
			AMint32 a12 = (AMint32)amFloorf(context->imageUserToSurface.a[1][2] + 0.5f);
            #if defined (RIM_VG_SRC)
    			if (context->fillBlendMode == VG_BLEND_SRC || amImageIsOpaque(image)) {
                   #if defined(RIM_OPENVG_USES_QC_7KMDP_BLT)
                        if (image->format == amSrfRealFormatGet(surface)) {
                            SURFACE_PIXELS_SET_SRC_MDP
                            return res;
                        }else{    
                            SURFACE_PIXELS_SET_SRC
                            return res;
                        }
                   #else 
                        SURFACE_PIXELS_SET_SRC
                        return res;
                   #endif
                }
    			else
        			if (image->format == amSrfRealFormatGet(surface)) {
        			#if defined(AM_LITE_PROFILE)
        				// in the lite profile, drawing surface has no alpha; in this case we suppose image format = surface format, so even the image has no alpha
        				SURFACE_PIXELS_SET_SRC
        			#else
        				SURFACE_PIXELS_SET_SRC_OVER
        			#endif
        				return res;
    			}
            #endif
		}
	}
#endif

	// fill userToSurfaceDesc fields
	userToSurfaceDesc.userToSurface = &context->imageUserToSurface;
	userToSurfaceDesc.inverseUserToSurface = &context->inverseImageUserToSurface;
	userToSurfaceDesc.userToSurfaceScale = context->imageUserToSurfaceScale;
	userToSurfaceDesc.flags = context->imageUserToSurfaceFlags;
	userToSurfaceDesc.userToSurfaceAffine = amMatrix33fIsAffine(&context->imageUserToSurface);

#if defined(AM_SRE)
	res = amSreImageDraw(context, surface, &userToSurfaceDesc, image);
#elif defined(AM_GLE)
	res = amGleImageDraw(context, surface, &userToSurfaceDesc, image);
	if (!res)
		amGlStatesRecover(context);
#elif defined(AM_GLS)
	res = amGlsImageDraw(context, surface, &userToSurfaceDesc, image);
#else
	#error Unreachable point.
#endif

	// update flags (at the moment only AM_MATRIX_MODELVIEW_MODIFIED bit can be modified)
	context->imageUserToSurfaceFlags = userToSurfaceDesc.flags;
#if defined(AM_GLE)
	context->pathUserToSurfaceFlags |= AM_MATRIX_MODELVIEW_MODIFIED;
	#if (AM_OPENVG_VERSION >= 110)
		context->glyphUserToSurfaceFlags |= AM_MATRIX_MODELVIEW_MODIFIED;
	#endif
#endif

	return res;

	#undef SURFACE_PIXELS_SET_SRC
	#undef SURFACE_PIXELS_SET_SRC_OVER
}

#if defined (RIM_VG_SRC)
#pragma pop
#endif
