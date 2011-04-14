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
	\file srecompositing.c
	\brief Compositing utilities (SRE), implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#if defined(AM_SRE)

#include "srecompositing.h"

/*!
	\brief Patch the blend mode of the given paint descriptor, choosing a more convenient (faster) blend mode when possible.
	\param paintDesc paint descriptor whose blend mode is to patch.
	\param context input context containing color transform values.
	\note used to draw paths.
*/
void amSrePathBlendModePatch(AMPaintDesc *paintDesc,
							 const AMContext *context) {

	AM_ASSERT(paintDesc);
	AM_ASSERT(context);

	// optimization for opaque paints
	if (amPaintIsOpaque(paintDesc, AM_TRUE, context)) {

		switch (paintDesc->blendMode) {
			case VG_BLEND_SRC_OVER:
				paintDesc->blendMode = VG_BLEND_SRC;
				break;
		#if defined(VG_MZT_advanced_blend_modes)
			case VG_BLEND_DST_IN:
				paintDesc->blendMode = VG_BLEND_DST_MZT;
				break;
			case VG_BLEND_SRC_ATOP_MZT:
				paintDesc->blendMode = VG_BLEND_SRC_IN;
				break;
			case VG_BLEND_XOR_MZT:
				paintDesc->blendMode = VG_BLEND_SRC_OUT_MZT;
				break;
		#endif
			default:
				break;
		}
	}

#if defined(VG_MZT_advanced_blend_modes)
	if (paintDesc->blendMode == VG_BLEND_CLEAR_MZT)
		paintDesc->blendMode = VG_BLEND_SRC;
#endif
}

/*!
	\brief Patch the blend mode of the given paint descriptor, choosing a more convenient (faster) blend mode when possible.
	\param paintDesc paint descriptor whose blend mode is to patch.
	\param context input context containing color transform values.
	\note used to draw images.
*/
void amSreImageBlendModePatch(AMPaintDesc *paintDesc,
							  const AMContext *context) {

	const AMImage *img = paintDesc->image;
	AMbool opaque = AM_TRUE;

	AM_ASSERT(paintDesc);
	AM_ASSERT(context);

	if (paintDesc->imageMode == VG_DRAW_IMAGE_NORMAL) {

		if (!amImageIsOpaque(img)) {
			// original image is not opaque...
			opaque = AM_FALSE;
		#if (AM_OPENVG_VERSION >= 110)
			// ... but we could have a color transform that makes the pattern opaque
			if (paintDesc->colorTransform && context->clampedColorTransformValues[3] >= 0.0f && context->clampedColorTransformValues[7] >= 1.0f)
				opaque = AM_TRUE;
		#endif
		}
		else {
			// original image is opaque...
		#if (AM_OPENVG_VERSION >= 110)
			// ... but we could have a color transform that makes the pattern non-opaque
			if (paintDesc->colorTransform && context->clampedColorTransformValues[3] < 1.0f - context->clampedColorTransformValues[7])
				opaque = AM_FALSE;
		#endif
		}
	}
	else
	if (paintDesc->imageMode == VG_DRAW_IMAGE_MULTIPLY) {
		if (!amImageIsOpaque(img) || !amPaintIsOpaque(paintDesc, AM_FALSE, context))
			opaque = AM_FALSE;
	#if (AM_OPENVG_VERSION >= 110)
		// both image and paint (excluding color transform) are opaque, lets check color transformation (alpha channel only)
		else {
			if (paintDesc->colorTransform && context->clampedColorTransformValues[3] < 1.0f - context->clampedColorTransformValues[7])
				opaque = AM_FALSE;
		}
	#endif
	}
	else {
		AM_ASSERT(paintDesc->imageMode == VG_DRAW_IMAGE_STENCIL);
		opaque = AM_FALSE;
	}

	// optimization for opaque drawing
	if (opaque) {

		switch (paintDesc->blendMode) {
			case VG_BLEND_SRC_OVER:
				paintDesc->blendMode = VG_BLEND_SRC;
				break;
		#if defined(VG_MZT_advanced_blend_modes)
			case VG_BLEND_DST_IN:
				paintDesc->blendMode = VG_BLEND_DST_MZT;
				break;
			case VG_BLEND_SRC_ATOP_MZT:
				paintDesc->blendMode = VG_BLEND_SRC_IN;
				break;
			case VG_BLEND_XOR_MZT:
				paintDesc->blendMode = VG_BLEND_SRC_OUT_MZT;
				break;
		#endif
			default:
				break;
		}
	}

#if defined(VG_MZT_advanced_blend_modes)
	if (paintDesc->blendMode == VG_BLEND_CLEAR_MZT)
		paintDesc->blendMode = VG_BLEND_SRC;
#endif
}

#endif

#if defined (RIM_VG_SRC)
#pragma pop
#endif
