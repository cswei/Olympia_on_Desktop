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
	\file vgmatrix.c
	\brief OpenVG matrix, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "vgmatrix.h"
#include "vgcontext.h"
#include "vg_priv.h"

#if defined RIM_VG_SRC
#define VG_API_ENTRY 
#endif


/*!
	\brief Update matrices: if needed it calculates OpenVG inverse matrices, extract scale factors and update matrices flags.
	\param _context context containing OpenVG matrices.
*/
void amMatricesUpdate(void *_context) {

	AMContext *context = (AMContext *)_context;

	AM_ASSERT(context);

	if (context->pathUserToSurfaceFlags & AM_MATRIX_INVERSE_MODIFIED) {
		// affine matrix inversion
		if (!amMatrix33fInvertAffine(&context->inversePathUserToSurface, &context->pathUserToSurface))
			context->pathUserToSurfaceFlags |= AM_MATRIX_SINGULAR;
		else
			context->pathUserToSurfaceFlags &= ((AMuint32)(~0) - AM_MATRIX_SINGULAR);
		context->pathUserToSurfaceFlags &= ((AMuint32)(~0) - AM_MATRIX_INVERSE_MODIFIED);
	}

	if (context->fillPaintToUserFlags & AM_MATRIX_INVERSE_MODIFIED) {
		// affine matrix inversion
		if (!amMatrix33fInvertAffine(&context->inverseFillPaintToUser, &context->fillPaintToUser))
			context->fillPaintToUserFlags |= AM_MATRIX_SINGULAR;
		else
			context->fillPaintToUserFlags &= ((AMuint32)(~0) - AM_MATRIX_SINGULAR);
		context->fillPaintToUserFlags &= ((AMuint32)(~0) - AM_MATRIX_INVERSE_MODIFIED);
	}

	if (context->strokePaintToUserFlags & AM_MATRIX_INVERSE_MODIFIED) {
		// affine matrix inversion
		if (!amMatrix33fInvertAffine(&context->inverseStrokePaintToUser, &context->strokePaintToUser))
			context->strokePaintToUserFlags |= AM_MATRIX_SINGULAR;
		else
			context->strokePaintToUserFlags &= ((AMuint32)(~0) - AM_MATRIX_SINGULAR);
		context->strokePaintToUserFlags &= ((AMuint32)(~0) - AM_MATRIX_INVERSE_MODIFIED);
	}

	if (context->imageUserToSurfaceFlags & AM_MATRIX_INVERSE_MODIFIED) {

		AMfloat determinant;

		// full matrix inversion
		if (!amMatrix33fInvert(&context->inverseImageUserToSurface, &determinant, &context->imageUserToSurface))
			context->imageUserToSurfaceFlags |= AM_MATRIX_SINGULAR;
		else
			context->imageUserToSurfaceFlags &= ((AMuint32)(~0) - AM_MATRIX_SINGULAR);
		context->imageUserToSurfaceFlags &= ((AMuint32)(~0) - AM_MATRIX_INVERSE_MODIFIED);
	}

#if (AM_OPENVG_VERSION >= 110)
	if (context->glyphUserToSurfaceFlags & AM_MATRIX_INVERSE_MODIFIED) {

		// affine matrix inversion
		if (!amMatrix33fInvertAffine(&context->inverseGlyphUserToSurface, &context->glyphUserToSurface))
			context->glyphUserToSurfaceFlags |= AM_MATRIX_SINGULAR;
		else
			context->glyphUserToSurfaceFlags &= ((AMuint32)(~0) - AM_MATRIX_SINGULAR);
		context->glyphUserToSurfaceFlags &= ((AMuint32)(~0) - AM_MATRIX_INVERSE_MODIFIED);
	}
#endif

	// extract scale factors
	if (context->pathUserToSurfaceFlags & AM_MATRIX_SCALE_MODIFIED) {

		if (context->pathUserToSurfaceFlags & AM_MATRIX_SINGULAR) {
			context->pathUserToSurfaceScale[0] = 0.0f;
			context->pathUserToSurfaceScale[1] = 0.0f;
		}
		else {
			AMVect3f scaleFactors;

			amMatrix33fScaleFactors(&scaleFactors, &context->pathUserToSurface);
			context->pathUserToSurfaceScale[0] = amAbsf(scaleFactors.x);
			context->pathUserToSurfaceScale[1] = amAbsf(scaleFactors.y);
		}
		context->pathUserToSurfaceFlags &= ((AMuint32)(~0) - AM_MATRIX_SCALE_MODIFIED);
	}

	if (context->imageUserToSurfaceFlags & AM_MATRIX_SCALE_MODIFIED) {

		AMMatrix33f m = context->imageUserToSurface;

		// extract scale factors from the imageUserToSurface affine portion
		m.a[2][0] = 0.0f;
		m.a[2][1] = 0.0f;
		m.a[2][2] = 1.0f;

		if (context->imageUserToSurfaceFlags & AM_MATRIX_SINGULAR) {
			context->imageUserToSurfaceScale[0] = 0.0f;
			context->imageUserToSurfaceScale[1] = 0.0f;
		}
		else {
			AMVect3f scaleFactors;

			amMatrix33fScaleFactors(&scaleFactors, &m);
			context->imageUserToSurfaceScale[0] = amAbsf(scaleFactors.x);
			context->imageUserToSurfaceScale[1] = amAbsf(scaleFactors.y);
		}
		context->imageUserToSurfaceFlags &= ((AMuint32)(~0) - AM_MATRIX_SCALE_MODIFIED);
	}

	if (context->fillPaintToUserFlags & AM_MATRIX_SCALE_MODIFIED) {

		if (context->fillPaintToUserFlags & AM_MATRIX_SINGULAR) {
			context->fillPaintToUserScale[0] = 0.0f;
			context->fillPaintToUserScale[1] = 0.0f;
		}
		else {
			AMVect3f scaleFactors;

			amMatrix33fScaleFactors(&scaleFactors, &context->fillPaintToUser);
			context->fillPaintToUserScale[0] = amAbsf(scaleFactors.x);
			context->fillPaintToUserScale[1] = amAbsf(scaleFactors.y);
		}
		context->fillPaintToUserFlags &= ((AMuint32)(~0) - AM_MATRIX_SCALE_MODIFIED);
	}

	if (context->strokePaintToUserFlags & AM_MATRIX_SCALE_MODIFIED) {

		if (context->strokePaintToUserFlags & AM_MATRIX_SINGULAR) {
			context->strokePaintToUserScale[0] = 0.0f;
			context->strokePaintToUserScale[1] = 0.0f;
		}
		else {
			AMVect3f scaleFactors;

			amMatrix33fScaleFactors(&scaleFactors, &context->strokePaintToUser);
			context->strokePaintToUserScale[0] = amAbsf(scaleFactors.x);
			context->strokePaintToUserScale[1] = amAbsf(scaleFactors.y);
		}
		context->strokePaintToUserFlags &= ((AMuint32)(~0) - AM_MATRIX_SCALE_MODIFIED);
	}

#if (AM_OPENVG_VERSION >= 110)
	if (context->glyphUserToSurfaceFlags & AM_MATRIX_SCALE_MODIFIED) {

		if (context->glyphUserToSurfaceFlags & AM_MATRIX_SINGULAR) {
			context->glyphUserToSurfaceScale[0] = 0.0f;
			context->glyphUserToSurfaceScale[1] = 0.0f;
		}
		else {
			AMVect3f scaleFactors;

			amMatrix33fScaleFactors(&scaleFactors, &context->glyphUserToSurface);
			context->glyphUserToSurfaceScale[0] = amAbsf(scaleFactors.x);
			context->glyphUserToSurfaceScale[1] = amAbsf(scaleFactors.y);
		}
		context->glyphUserToSurfaceFlags &= ((AMuint32)(~0) - AM_MATRIX_SCALE_MODIFIED);
	}
#endif
}

/*!
	\brief It sets the current matrix to the identity matrix.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
*/
VG_API_CALL void VG_API_ENTRY vgLoadIdentity(void) VG_API_EXIT {

	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgLoadIdentity");
		return;
	}

	AM_ASSERT(currentContext->selectedMatrix);
	AM_ASSERT(currentContext->selectedInverseMatrix);
	AM_ASSERT(currentContext->selectedMatrixScale);
	AM_ASSERT(currentContext->selectedMatrixFlags);

	AM_MATRIX33_IDENTITY(currentContext->selectedMatrix);
	AM_MATRIX33_IDENTITY(currentContext->selectedInverseMatrix);
	currentContext->selectedMatrixScale[0] = currentContext->selectedMatrixScale[1] = 1.0f;
#if defined(AM_GLE)
	*currentContext->selectedMatrixFlags = AM_MATRIX_MODELVIEW_MODIFIED;
#endif
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgLoadIdentity");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief It loads an arbitrary set of matrix values into the current matrix. Nine matrix values are read from
	the specified parameter.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param m input float matrix values.
	\note if the targeted matrix is affine (i.e., the matrix mode is not VG_MATRIX_IMAGE_USER_TO_SURFACE), the
	values defining the perspective matrix portion are ignored and replaced by the values { 0, 0, 1 }.
*/
VG_API_CALL void VG_API_ENTRY vgLoadMatrix(const VGfloat *m) VG_API_EXIT {

	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgLoadMatrix");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	AM_ASSERT(currentContext->selectedMatrix);
	AM_ASSERT(currentContext->selectedMatrixFlags);

	if (!m || !amPointerIsAligned(m, 4)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgLoadMatrix");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	currentContext->selectedMatrix->a[0][0] = amNanInfFix(m[0]);
	currentContext->selectedMatrix->a[0][1] = amNanInfFix(m[3]);
	currentContext->selectedMatrix->a[0][2] = amNanInfFix(m[6]);
	currentContext->selectedMatrix->a[1][0] = amNanInfFix(m[1]);
	currentContext->selectedMatrix->a[1][1] = amNanInfFix(m[4]);
	currentContext->selectedMatrix->a[1][2] = amNanInfFix(m[7]);

	if (currentContext->matrixMode != VG_MATRIX_IMAGE_USER_TO_SURFACE) {
		currentContext->selectedMatrix->a[2][0] = 0.0f;
		currentContext->selectedMatrix->a[2][1] = 0.0f;
		currentContext->selectedMatrix->a[2][2] = 1.0f;
	}
	else {
		currentContext->selectedMatrix->a[2][0] = amNanInfFix(m[2]);
		currentContext->selectedMatrix->a[2][1] = amNanInfFix(m[5]);
		currentContext->selectedMatrix->a[2][2] = amNanInfFix(m[8]);
	}
	*currentContext->selectedMatrixFlags |= (AM_MATRIX_SCALE_MODIFIED | AM_MATRIX_INVERSE_MODIFIED);
#if defined(AM_GLE)
	*currentContext->selectedMatrixFlags |= AM_MATRIX_MODELVIEW_MODIFIED;
#endif
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgLoadMatrix");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief It retrieves the value of the current transformation matrix. Nine values are written to the
	specified parameter.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param m output float values.
*/
VG_API_CALL void VG_API_ENTRY vgGetMatrix(VGfloat *m) VG_API_EXIT {

	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgLoadMatrix");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	AM_ASSERT(currentContext->selectedMatrix);

	if (!m || !amPointerIsAligned(m, 4)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgLoadMatrix");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	m[0] = currentContext->selectedMatrix->a[0][0];
	m[1] = currentContext->selectedMatrix->a[1][0];
	m[2] = currentContext->selectedMatrix->a[2][0];
	m[3] = currentContext->selectedMatrix->a[0][1];
	m[4] = currentContext->selectedMatrix->a[1][1];
	m[5] = currentContext->selectedMatrix->a[2][1];
	m[6] = currentContext->selectedMatrix->a[0][2];
	m[7] = currentContext->selectedMatrix->a[1][2];
	m[8] = currentContext->selectedMatrix->a[2][2];
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgLoadMatrix");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief It right-multiplies the current matrix by a given matrix. Nine matrix values are read from
	the specified parameter.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param m input float matrix values.
*/
VG_API_CALL void VG_API_ENTRY vgMultMatrix(const VGfloat *m) VG_API_EXIT {

	AMMatrix33f tmp;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgMultMatrix");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	AM_ASSERT(currentContext->selectedMatrix);
	AM_ASSERT(currentContext->selectedMatrixFlags);

	if (!m || !amPointerIsAligned(m, 4)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgMultMatrix");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// handle NaN and Inf values
	tmp.a[0][0] = amNanInfFix(m[0]);
	tmp.a[1][0] = amNanInfFix(m[1]);
	tmp.a[0][1] = amNanInfFix(m[3]);
	tmp.a[1][1] = amNanInfFix(m[4]);
	tmp.a[0][2] = amNanInfFix(m[6]);
	tmp.a[1][2] = amNanInfFix(m[7]);

	if (currentContext->matrixMode != VG_MATRIX_IMAGE_USER_TO_SURFACE) {
		tmp.a[2][0] = 0.0f;
		tmp.a[2][1] = 0.0f;
		tmp.a[2][2] = 1.0f;
		AM_MATRIX33_MUL(currentContext->selectedMatrix, currentContext->selectedMatrix, &tmp, AMMatrix33f);
	}
	else {
		tmp.a[2][0] = amNanInfFix(m[2]);
		tmp.a[2][1] = amNanInfFix(m[5]);
		tmp.a[2][2] = amNanInfFix(m[8]);
		AM_MATRIX33_MUL(currentContext->selectedMatrix, currentContext->selectedMatrix, &tmp, AMMatrix33f);
	}
	*currentContext->selectedMatrixFlags |= (AM_MATRIX_SCALE_MODIFIED | AM_MATRIX_INVERSE_MODIFIED);
#if defined(AM_GLE)
	*currentContext->selectedMatrixFlags |= AM_MATRIX_MODELVIEW_MODIFIED;
#endif
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgMultMatrix");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief It modifies the current transformation by appending a translation. This is equivalent to
	right-multiplying the current matrix by a translation matrix.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param tx translation along x direction.
	\param ty translation along y direction.
*/
VG_API_CALL void VG_API_ENTRY vgTranslate(VGfloat tx,
                                          VGfloat ty) VG_API_EXIT {

	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgTranslate");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	AM_ASSERT(currentContext->selectedMatrix);
	AM_ASSERT(currentContext->selectedMatrixFlags);

	// handle NaN and Inf values
	tx = amNanInfFix(tx);
	ty = amNanInfFix(ty);

	currentContext->selectedMatrix->a[0][2] += currentContext->selectedMatrix->a[0][0] * tx + currentContext->selectedMatrix->a[0][1] * ty;
	currentContext->selectedMatrix->a[1][2] += currentContext->selectedMatrix->a[1][0] * tx + currentContext->selectedMatrix->a[1][1] * ty;
	currentContext->selectedMatrix->a[2][2] += currentContext->selectedMatrix->a[2][0] * tx + currentContext->selectedMatrix->a[2][1] * ty;

	*currentContext->selectedMatrixFlags |= (AM_MATRIX_INVERSE_MODIFIED);
#if defined(AM_GLE)
	*currentContext->selectedMatrixFlags |= AM_MATRIX_MODELVIEW_MODIFIED;
#endif
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgTranslate");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief It modifies the current transformation by appending a scale. This is equivalent to right-multiplying
	the current matrix by a scale matrix.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param sx scale factor along x direction.
	\param sy scale factor along y direction.
*/
VG_API_CALL void VG_API_ENTRY vgScale(VGfloat sx,
                                      VGfloat sy) VG_API_EXIT {

	AMMatrix33f tmp;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgScale");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	AM_ASSERT(currentContext->selectedMatrix);
	AM_ASSERT(currentContext->selectedMatrixFlags);

	// handle NaN and Inf values
	sx = amNanInfFix(sx);
	sy = amNanInfFix(sy);

	currentContext->selectedMatrixScale[0] *= amAbsf(sx);
	currentContext->selectedMatrixScale[1] *= amAbsf(sy);

	// direct transformation
	tmp.a[0][0] = sx;
	tmp.a[0][1] = 0.0f;
	tmp.a[0][2] = 0.0f;
	tmp.a[1][0] = 0.0f;
	tmp.a[1][1] = sy;
	tmp.a[1][2] = 0.0f;
	tmp.a[2][0] = 0.0f;
	tmp.a[2][1] = 0.0f;
	tmp.a[2][2] = 1.0f;
	AM_MATRIX33_MUL(currentContext->selectedMatrix, currentContext->selectedMatrix, &tmp, AMMatrix33f);

	*currentContext->selectedMatrixFlags |= (AM_MATRIX_INVERSE_MODIFIED);
#if defined(AM_GLE)
	*currentContext->selectedMatrixFlags |= AM_MATRIX_MODELVIEW_MODIFIED;
#endif
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgScale");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}
/*!
	\brief It modifies the current transformation by appending a shear. This is equivalent to right-multiplying
	the current matrix by a shear matrix.
	\param shx share factor along x direction.
	\param shy share factor along y direction.
*/
VG_API_CALL void VG_API_ENTRY vgShear(VGfloat shx,
                                      VGfloat shy) VG_API_EXIT {

	AMMatrix33f tmp;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgShear");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	AM_ASSERT(currentContext->selectedMatrix);
	AM_ASSERT(currentContext->selectedMatrixFlags);

	// handle NaN and Inf values
	shx = amNanInfFix(shx);
	shy = amNanInfFix(shy);

	if (shx == 1.0f && shy == 1.0f) {
		shx += 0.00000001f;
		shy += 0.00000001f;
	}

	// direct transformation
	tmp.a[0][0] = 1.0f;
	tmp.a[0][1] = shx;
	tmp.a[0][2] = 0.0f;
	tmp.a[1][0] = shy;
	tmp.a[1][1] = 1.0f;
	tmp.a[1][2] = 0.0f;
	tmp.a[2][0] = 0.0f;
	tmp.a[2][1] = 0.0f;
	tmp.a[2][2] = 1.0f;
	AM_MATRIX33_MUL(currentContext->selectedMatrix, currentContext->selectedMatrix, &tmp, AMMatrix33f);

	*currentContext->selectedMatrixFlags |= (AM_MATRIX_SCALE_MODIFIED | AM_MATRIX_INVERSE_MODIFIED);
#if defined(AM_GLE)
	*currentContext->selectedMatrixFlags |= AM_MATRIX_MODELVIEW_MODIFIED;
#endif
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgShear");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief It modifies the current transformation by appending a counter-clockwise rotation by a given
	angle (expressed in degrees) about the origin. This is equivalent to right-multiplying the current matrix
	by a rotation matrix.
	\param angle rotation angle, in degrees.
*/
VG_API_CALL void VG_API_ENTRY vgRotate(VGfloat angle) VG_API_EXIT {

	AMMatrix33f tmp;
	AMfloat alpha, c, s;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgRotate");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	AM_ASSERT(currentContext->selectedMatrix);
	AM_ASSERT(currentContext->selectedMatrixFlags);

	// handle NaN and Inf values
	angle = amNanInfFix(angle);

	alpha = amDeg2Radf(angle);
	amSinCosf(&s, &c, alpha);

	// direct transformation
	tmp.a[0][0] = c;
	tmp.a[0][1] = -s;
	tmp.a[0][2] = 0.0f;
	tmp.a[1][0] = s;
	tmp.a[1][1] = c;
	tmp.a[1][2] = 0.0f;
	tmp.a[2][0] = 0.0f;
	tmp.a[2][1] = 0.0f;
	tmp.a[2][2] = 1.0f;
	AM_MATRIX33_MUL(currentContext->selectedMatrix, currentContext->selectedMatrix, &tmp, AMMatrix33f);

	*currentContext->selectedMatrixFlags |= (AM_MATRIX_INVERSE_MODIFIED);
#if defined(AM_GLE)
	*currentContext->selectedMatrixFlags |= AM_MATRIX_MODELVIEW_MODIFIED;
#endif
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgRotate");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

#if defined (RIM_VG_SRC)
#pragma pop
#endif

