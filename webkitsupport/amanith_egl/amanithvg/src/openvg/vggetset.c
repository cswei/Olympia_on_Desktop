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
	\file vggetset.c
	\brief OpenVG get/set context/handles parameters, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "vgpaint.h"
#if (AM_OPENVG_VERSION >= 110)
	#include "vgfont.h"
#endif

#if defined RIM_VG_SRC
#define VG_API_ENTRY 
#endif

// *********************************************************************
//                   Getters and setters for context
// *********************************************************************

/*!
	\brief Set the value of a parameter on the current context, scalar float value.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param type parameter to set.
	\param value input float value to assign.
*/
VG_API_CALL void VG_API_ENTRY vgSetf(VGParamType type,
                                     VGfloat value) VG_API_EXIT {

	AMint32 ival;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgSetf");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// handle NaN and Inf values
	value = amNanInfFix(value);

	switch (type) {

		case VG_MATRIX_MODE:
			ival = (AMint32)amFloorf(value);
			if (ival < AM_FIRST_MATRIX_MODE || ival > AM_LAST_MATRIX_MODE)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				amCtxMatrixModeSet(currentContext, (VGMatrixMode)ival);
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_FILL_RULE:
			ival = (AMint32)amFloorf(value);
			if (ival < VG_EVEN_ODD || ival > VG_NON_ZERO)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->fillRule = (VGFillRule)ival;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_IMAGE_QUALITY:
			ival = (AMint32)amFloorf(value);
			if (ival <= 0 || ival > (VG_IMAGE_QUALITY_NONANTIALIASED | VG_IMAGE_QUALITY_FASTER | VG_IMAGE_QUALITY_BETTER))
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				amCtxImageQualitySet(currentContext, ival);
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_RENDERING_QUALITY:
			ival = (AMint32)amFloorf(value);
			if (ival < VG_RENDERING_QUALITY_NONANTIALIASED || ival > VG_RENDERING_QUALITY_BETTER)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				amCtxRenderingQualitySet(currentContext, (VGRenderingQuality)ival);
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_BLEND_MODE:
			ival = (AMint32)amFloorf(value);
		#if defined(VG_MZT_advanced_blend_modes)
			if (ival < VG_BLEND_SRC || ival > VG_BLEND_EXCLUSION_MZT)
		#else
			if (ival < VG_BLEND_SRC || ival > VG_BLEND_ADDITIVE)
		#endif
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->strokeBlendMode = currentContext->fillBlendMode = (VGBlendMode)ival;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

	#if defined(VG_MZT_separable_blend_modes)
		case VG_STROKE_BLEND_MODE_MZT:
			ival = (AMint32)amFloorf(value);
		#if defined(VG_MZT_advanced_blend_modes)
			if (ival < VG_BLEND_SRC || ival > VG_BLEND_EXCLUSION_MZT)
		#else
			if (ival < VG_BLEND_SRC || ival > VG_BLEND_ADDITIVE)
		#endif
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->strokeBlendMode = (VGBlendMode)ival;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_FILL_BLEND_MODE_MZT:
			ival = (AMint32)amFloorf(value);
		#if defined(VG_MZT_advanced_blend_modes)
			if (ival < VG_BLEND_SRC || ival > VG_BLEND_EXCLUSION_MZT)
		#else
			if (ival < VG_BLEND_SRC || ival > VG_BLEND_ADDITIVE)
		#endif
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->fillBlendMode = (VGBlendMode)ival;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;
	#endif

		case VG_IMAGE_MODE:
			ival = (AMint32)amFloorf(value);
			if (ival < VG_DRAW_IMAGE_NORMAL || ival > VG_DRAW_IMAGE_STENCIL)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->imageMode = (VGImageMode)ival;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_SCISSOR_RECTS:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case VG_COLOR_TRANSFORM:
			ival = (AMint32)amFloorf(value);
			amCtxColorTransformSet(currentContext, ((VGboolean)ival != VG_FALSE) ? VG_TRUE : VG_FALSE);
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;

		case VG_COLOR_TRANSFORM_VALUES:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			break;
	#endif

		case VG_STROKE_LINE_WIDTH:
			amCtxStrokeWidthSet(currentContext, value);
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;

		case VG_STROKE_CAP_STYLE:
			ival = (AMint32)amFloorf(value);
			if (ival < VG_CAP_BUTT || ival > VG_CAP_SQUARE)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->startCapStyle = currentContext->endCapStyle = (VGCapStyle)ival;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

	#if defined(VG_MZT_separable_cap_style)
		case VG_STROKE_START_CAP_STYLE_MZT:
			ival = (AMint32)amFloorf(value);
			if (ival < VG_CAP_BUTT || ival > VG_CAP_SQUARE)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->startCapStyle = (VGCapStyle)ival;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;
		case VG_STROKE_END_CAP_STYLE_MZT:
			ival = (AMint32)amFloorf(value);
			if (ival < VG_CAP_BUTT || ival > VG_CAP_SQUARE)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->endCapStyle = (VGCapStyle)ival;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;
	#endif

		case VG_STROKE_JOIN_STYLE:
			ival = (AMint32)amFloorf(value);
			if (ival < VG_JOIN_MITER || ival > VG_JOIN_BEVEL)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->joinStyle = (VGJoinStyle)ival;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_STROKE_MITER_LIMIT:
			amCtxStrokeMiterLimitSet(currentContext, value);
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;

		case VG_STROKE_DASH_PATTERN:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			break;

		case VG_STROKE_DASH_PHASE:
			amCtxStrokeDashPhaseSet(currentContext, value);
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;

		case VG_STROKE_DASH_PHASE_RESET:
			ival = (AMint32)amFloorf(value);
			currentContext->dashPhaseReset = ((VGboolean)ival != VG_FALSE) ? VG_TRUE : VG_FALSE;
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;

		case VG_TILE_FILL_COLOR:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			break;

		case VG_CLEAR_COLOR:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case VG_GLYPH_ORIGIN:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			break;
	#endif

		case VG_MASKING:
			ival = (AMint32)amFloorf(value);
			currentContext->masking = ((VGboolean)ival != VG_FALSE) ? VG_TRUE : VG_FALSE;
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;

		case VG_SCISSORING:
			ival = (AMint32)amFloorf(value);
			currentContext->scissoring = ((VGboolean)ival != VG_FALSE) ? VG_TRUE : VG_FALSE;
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;

		case VG_PIXEL_LAYOUT:
			ival = (AMint32)amFloorf(value);
			if (ival < VG_PIXEL_LAYOUT_UNKNOWN || ival > VG_PIXEL_LAYOUT_BGR_HORIZONTAL)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->pixelLayout = (VGPixelLayout)ival;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_SCREEN_LAYOUT:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;

		case VG_FILTER_FORMAT_LINEAR:
			ival = (AMint32)amFloorf(value);
			currentContext->filterFormatLinear = ((VGboolean)ival != VG_FALSE) ? VG_TRUE : VG_FALSE;
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;

		case VG_FILTER_FORMAT_PREMULTIPLIED:
			ival = (AMint32)amFloorf(value);
			currentContext->filterFormatPremultiplied = ((VGboolean)ival != VG_FALSE) ? VG_TRUE : VG_FALSE;
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;

		case VG_FILTER_CHANNEL_MASK:
			ival = (AMint32)amFloorf(value);
			// undefined bits are ignored (this is the behavior of the reference implementation)
			currentContext->filterChannelMask = ival;
			amCtxErrorSet(currentContext, VG_NO_ERROR);
		#if 0
			// take care of negative (or invalid) values
			if (ival <= 0 || ival > (VG_RED | VG_GREEN | VG_BLUE | VG_ALPHA))
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->filterChannelMask = ival;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
		#endif
			break;

		case VG_MAX_SCISSOR_RECTS:
		case VG_MAX_DASH_COUNT:
		case VG_MAX_KERNEL_SIZE:
		case VG_MAX_SEPARABLE_KERNEL_SIZE:
		case VG_MAX_COLOR_RAMP_STOPS:
		case VG_MAX_IMAGE_WIDTH:
		case VG_MAX_IMAGE_HEIGHT:
		case VG_MAX_IMAGE_PIXELS:
		case VG_MAX_IMAGE_BYTES:
		case VG_MAX_FLOAT:
		case VG_MAX_GAUSSIAN_STD_DEVIATION:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;
		default:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			break;
	}
	AM_MEMORY_LOG("vgSetf");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief Set the value of a parameter on the current context, scalar integer value.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param type parameter to set.
	\param value input integer value to assign.
*/
VG_API_CALL void VG_API_ENTRY vgSeti(VGParamType type,
                                     VGint value) VG_API_EXIT {

	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgSeti");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	switch (type) {

		case VG_MATRIX_MODE:
			if (value < AM_FIRST_MATRIX_MODE || value > AM_LAST_MATRIX_MODE)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				amCtxMatrixModeSet(currentContext, (VGMatrixMode)value);
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_FILL_RULE:
			if (value < VG_EVEN_ODD || value > VG_NON_ZERO)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->fillRule = (VGFillRule)value;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_IMAGE_QUALITY:
			if (value <= 0 || value > (VG_IMAGE_QUALITY_NONANTIALIASED | VG_IMAGE_QUALITY_FASTER | VG_IMAGE_QUALITY_BETTER))
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				amCtxImageQualitySet(currentContext, value);
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_RENDERING_QUALITY:
			if (value < VG_RENDERING_QUALITY_NONANTIALIASED || value > VG_RENDERING_QUALITY_BETTER)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				amCtxRenderingQualitySet(currentContext, (VGRenderingQuality)value);
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_BLEND_MODE:
		#if defined(VG_MZT_advanced_blend_modes)
			if (value < VG_BLEND_SRC || value > VG_BLEND_EXCLUSION_MZT)
		#else
			if (value < VG_BLEND_SRC || value > VG_BLEND_ADDITIVE)
		#endif
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->strokeBlendMode = currentContext->fillBlendMode = (VGBlendMode)value;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

	#if defined(VG_MZT_separable_blend_modes)
		case VG_STROKE_BLEND_MODE_MZT:
		#if defined(VG_MZT_advanced_blend_modes)
			if (value < VG_BLEND_SRC || value > VG_BLEND_EXCLUSION_MZT)
		#else
			if (value < VG_BLEND_SRC || value > VG_BLEND_ADDITIVE)
		#endif
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->strokeBlendMode = (VGBlendMode)value;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_FILL_BLEND_MODE_MZT:
		#if defined(VG_MZT_advanced_blend_modes)
			if (value < VG_BLEND_SRC || value > VG_BLEND_EXCLUSION_MZT)
		#else
			if (value < VG_BLEND_SRC || value > VG_BLEND_ADDITIVE)
		#endif
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->fillBlendMode = (VGBlendMode)value;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;
	#endif

		case VG_IMAGE_MODE:
			if (value < VG_DRAW_IMAGE_NORMAL || value > VG_DRAW_IMAGE_STENCIL)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->imageMode = (VGImageMode)value;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_SCISSOR_RECTS:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case VG_COLOR_TRANSFORM:
			amCtxColorTransformSet(currentContext, ((VGboolean)value != VG_FALSE) ? VG_TRUE : VG_FALSE);
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;

		case VG_COLOR_TRANSFORM_VALUES:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			break;
	#endif

		case VG_STROKE_LINE_WIDTH:
			amCtxStrokeWidthSet(currentContext, (VGfloat)value);
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;

		case VG_STROKE_CAP_STYLE:
			if (value < VG_CAP_BUTT || value > VG_CAP_SQUARE)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->startCapStyle = currentContext->endCapStyle = (VGCapStyle)value;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;
	#if defined(VG_MZT_separable_cap_style)
		case VG_STROKE_START_CAP_STYLE_MZT:
			if (value < VG_CAP_BUTT || value > VG_CAP_SQUARE)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->startCapStyle = (VGCapStyle)value;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_STROKE_END_CAP_STYLE_MZT:
			if (value < VG_CAP_BUTT || value > VG_CAP_SQUARE)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->endCapStyle = (VGCapStyle)value;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;
	#endif

		case VG_STROKE_JOIN_STYLE:
			if (value < VG_JOIN_MITER || value > VG_JOIN_BEVEL)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->joinStyle = (VGJoinStyle)value;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_STROKE_MITER_LIMIT:
			amCtxStrokeMiterLimitSet(currentContext, (VGfloat)value);
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;

		case VG_STROKE_DASH_PATTERN:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			break;

		case VG_STROKE_DASH_PHASE:
			amCtxStrokeDashPhaseSet(currentContext, (VGfloat)value);
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;

		case VG_STROKE_DASH_PHASE_RESET:
			currentContext->dashPhaseReset = ((VGboolean)value != VG_FALSE) ? VG_TRUE : VG_FALSE;
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;

		case VG_TILE_FILL_COLOR:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			break;

		case VG_CLEAR_COLOR:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case VG_GLYPH_ORIGIN:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			break;
	#endif

		case VG_MASKING:
			currentContext->masking = ((VGboolean)value != VG_FALSE) ? VG_TRUE : VG_FALSE;
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;

		case VG_SCISSORING:
			currentContext->scissoring = ((VGboolean)value != VG_FALSE) ? VG_TRUE : VG_FALSE;
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;

		case VG_PIXEL_LAYOUT:
			if (value < VG_PIXEL_LAYOUT_UNKNOWN || value > VG_PIXEL_LAYOUT_BGR_HORIZONTAL)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->pixelLayout = (VGPixelLayout)value;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_SCREEN_LAYOUT:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;

		case VG_FILTER_FORMAT_LINEAR:
			currentContext->filterFormatLinear = ((VGboolean)value != VG_FALSE) ? VG_TRUE : VG_FALSE;
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;

		case VG_FILTER_FORMAT_PREMULTIPLIED:
			currentContext->filterFormatPremultiplied = ((VGboolean)value != VG_FALSE) ? VG_TRUE : VG_FALSE;
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;

		case VG_FILTER_CHANNEL_MASK:
			// undefined bits are ignored (this is the behavior of the reference implementation)
			currentContext->filterChannelMask = value;
			amCtxErrorSet(currentContext, VG_NO_ERROR);
		#if 0
			// take care of negative (or invalid) values
			if (value <= 0 || value > (VG_RED | VG_GREEN | VG_BLUE | VG_ALPHA))
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->filterChannelMask = value;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
		#endif
			break;

		case VG_MAX_SCISSOR_RECTS:
		case VG_MAX_DASH_COUNT:
		case VG_MAX_KERNEL_SIZE:
		case VG_MAX_SEPARABLE_KERNEL_SIZE:
		case VG_MAX_COLOR_RAMP_STOPS:
		case VG_MAX_IMAGE_WIDTH:
		case VG_MAX_IMAGE_HEIGHT:
		case VG_MAX_IMAGE_PIXELS:
		case VG_MAX_IMAGE_BYTES:
		case VG_MAX_FLOAT:
		case VG_MAX_GAUSSIAN_STD_DEVIATION:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;
		default:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			break;
	}
	AM_MEMORY_LOG("vgSeti");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief Set the value of a parameter on the current context, vector of float values.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param type parameter to set.
	\param count number of elements.
	\param values array of input values to assign.
*/
VG_API_CALL void VG_API_ENTRY vgSetfv(VGParamType type,
                                      VGint count,
                                      const VGfloat *values) VG_API_EXIT {

	AMint32 ival, i, j, tmp;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgSetfv");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	if ((count < 0) || (!values && count > 0) || !amPointerIsAligned(values, 4)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgSetfv");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	switch (type) {

		case VG_MATRIX_MODE:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				ival = (AMint32)amFloorf(amNanInfFix(values[0]));
				if (ival < AM_FIRST_MATRIX_MODE || ival > AM_LAST_MATRIX_MODE)
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					amCtxMatrixModeSet(currentContext, (VGMatrixMode)ival);
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;

		case VG_FILL_RULE:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				ival = (AMint32)amFloorf(amNanInfFix(values[0]));
				if (ival < VG_EVEN_ODD || ival > VG_NON_ZERO)
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					currentContext->fillRule = (VGFillRule)ival;
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;

		case VG_IMAGE_QUALITY:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				ival = (AMint32)amFloorf(amNanInfFix(values[0]));
				if (ival <= 0 || ival > (VG_IMAGE_QUALITY_NONANTIALIASED | VG_IMAGE_QUALITY_FASTER | VG_IMAGE_QUALITY_BETTER))
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					amCtxImageQualitySet(currentContext, ival);
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;

		case VG_RENDERING_QUALITY:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				ival = (AMint32)amFloorf(amNanInfFix(values[0]));
				if (ival < VG_RENDERING_QUALITY_NONANTIALIASED || ival > VG_RENDERING_QUALITY_BETTER)
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					amCtxRenderingQualitySet(currentContext, (VGRenderingQuality)ival);
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;

		case VG_BLEND_MODE:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				ival = (AMint32)amFloorf(amNanInfFix(values[0]));
			#if defined(VG_MZT_advanced_blend_modes)
				if (ival < VG_BLEND_SRC || ival > VG_BLEND_EXCLUSION_MZT)
			#else
				if (ival < VG_BLEND_SRC || ival > VG_BLEND_ADDITIVE)
			#endif
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					currentContext->strokeBlendMode = currentContext->fillBlendMode = (VGBlendMode)ival;
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;

	#if defined(VG_MZT_separable_blend_modes)
		case VG_STROKE_BLEND_MODE_MZT:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				ival = (AMint32)amFloorf(amNanInfFix(values[0]));
			#if defined(VG_MZT_advanced_blend_modes)
				if (ival < VG_BLEND_SRC || ival > VG_BLEND_EXCLUSION_MZT)
			#else
				if (ival < VG_BLEND_SRC || ival > VG_BLEND_ADDITIVE)
			#endif
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					currentContext->strokeBlendMode = (VGBlendMode)ival;
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;

		case VG_FILL_BLEND_MODE_MZT:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				ival = (AMint32)amFloorf(amNanInfFix(values[0]));
			#if defined(VG_MZT_advanced_blend_modes)
				if (ival < VG_BLEND_SRC || ival > VG_BLEND_EXCLUSION_MZT)
			#else
				if (ival < VG_BLEND_SRC || ival > VG_BLEND_ADDITIVE)
			#endif
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					currentContext->fillBlendMode = (VGBlendMode)ival;
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;
	#endif

		case VG_IMAGE_MODE:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				ival = (AMint32)amFloorf(amNanInfFix(values[0]));
				if (ival < VG_DRAW_IMAGE_NORMAL || ival > VG_DRAW_IMAGE_STENCIL)
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					currentContext->imageMode = (VGImageMode)ival;
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;

		case VG_SCISSOR_RECTS:
			// count must be a multiple of four for OpenVG 1.0.1+
			if (count & 3)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				tmp = AM_MIN(count, currentContext->maxScissorRects);
				if (!amCtxScissorRectsSetf(currentContext, tmp, values))
					amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
				else
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case VG_COLOR_TRANSFORM:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				ival = (AMint32)amFloorf(amNanInfFix(values[0]));
				amCtxColorTransformSet(currentContext, ((VGboolean)ival != VG_FALSE) ? VG_TRUE : VG_FALSE);
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_COLOR_TRANSFORM_VALUES:
			if (count != 8)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				amCtxColorTransformValuesSetf(currentContext, values);
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;
	#endif

		case VG_STROKE_LINE_WIDTH:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				amCtxStrokeWidthSet(currentContext, amNanInfFix(values[0]));
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_STROKE_CAP_STYLE:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				ival = (AMint32)amFloorf(amNanInfFix(values[0]));
				if (ival < VG_CAP_BUTT || ival > VG_CAP_SQUARE)
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					currentContext->startCapStyle = currentContext->endCapStyle = (VGCapStyle)ival;
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;
	#if defined(VG_MZT_separable_cap_style)
		case VG_STROKE_START_CAP_STYLE_MZT:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				ival = (AMint32)amFloorf(amNanInfFix(values[0]));
				if (ival < VG_CAP_BUTT || ival > VG_CAP_SQUARE)
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					currentContext->startCapStyle = (VGCapStyle)ival;
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;

		case VG_STROKE_END_CAP_STYLE_MZT:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				ival = (AMint32)amFloorf(amNanInfFix(values[0]));
				if (ival < VG_CAP_BUTT || ival > VG_CAP_SQUARE)
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					currentContext->endCapStyle = (VGCapStyle)ival;
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;
	#endif

		case VG_STROKE_JOIN_STYLE:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				ival = (AMint32)amFloorf(amNanInfFix(values[0]));
				if (ival < VG_JOIN_MITER || ival > VG_JOIN_BEVEL)
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					currentContext->joinStyle = (VGJoinStyle)ival;
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;

		case VG_STROKE_MITER_LIMIT:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				amCtxStrokeMiterLimitSet(currentContext, amNanInfFix(values[0]));
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_STROKE_DASH_PATTERN:
			tmp = AM_MIN(count, currentContext->maxDashCount);
			if (amCtxStrokeDashPatternSetf(currentContext, tmp, values))
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			else
				amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
			break;

		case VG_STROKE_DASH_PHASE:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				amCtxStrokeDashPhaseSet(currentContext, amNanInfFix(values[0]));
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_STROKE_DASH_PHASE_RESET:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				ival = (AMint32)amFloorf(amNanInfFix(values[0]));
				currentContext->dashPhaseReset = ((VGboolean)ival != VG_FALSE) ? VG_TRUE : VG_FALSE;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_TILE_FILL_COLOR:
			if (count != 4)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				j = AM_MIN(count, 4);
				for (i = 0; i < j; ++i) currentContext->tileFillColor[i] = amNanInfFix(values[i]);
				for (i = j; i < 4; ++i) currentContext->tileFillColor[i] = 0.0f;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_CLEAR_COLOR:
			if (count != 4)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				j = AM_MIN(count, 4);
				for (i = 0; i < j; ++i) currentContext->clearColor[i] = amNanInfFix(values[i]);
				for (i = j; i < 4; ++i) currentContext->clearColor[i] = 0.0f;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case VG_GLYPH_ORIGIN:
			if (count != 2)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				for (i = 0; i < 2; ++i) currentContext->glyphOrigin[i] = amNanInfFix(values[i]);
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;
	#endif

		case VG_MASKING:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				ival = (AMint32)amFloorf(amNanInfFix(values[0]));
				currentContext->masking = ((VGboolean)ival != VG_FALSE) ? VG_TRUE : VG_FALSE;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_SCISSORING:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				ival = (AMint32)amFloorf(amNanInfFix(values[0]));
				currentContext->scissoring = ((VGboolean)ival != VG_FALSE) ? VG_TRUE : VG_FALSE;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_PIXEL_LAYOUT:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				ival = (AMint32)amFloorf(amNanInfFix(values[0]));
				if (ival < VG_PIXEL_LAYOUT_UNKNOWN || ival > VG_PIXEL_LAYOUT_BGR_HORIZONTAL)
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					currentContext->pixelLayout = (VGPixelLayout)ival;
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;

		case VG_SCREEN_LAYOUT:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;

		case VG_FILTER_FORMAT_LINEAR:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				ival = (AMint32)amFloorf(amNanInfFix(values[0]));
				currentContext->filterFormatLinear = ((VGboolean)ival != VG_FALSE) ? VG_TRUE : VG_FALSE;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_FILTER_FORMAT_PREMULTIPLIED:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				ival = (AMint32)amFloorf(amNanInfFix(values[0]));
				currentContext->filterFormatPremultiplied = ((VGboolean)ival != VG_FALSE) ? VG_TRUE : VG_FALSE;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_FILTER_CHANNEL_MASK:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				ival = (AMint32)amFloorf(amNanInfFix(values[0]));
				// undefined bits are ignored (this is the behavior of the reference implementation)
				currentContext->filterChannelMask = ival;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			#if 0
				// take care of negative (or invalid) values
				if (ival <= 0 || ival > (VG_RED | VG_GREEN | VG_BLUE | VG_ALPHA))
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					currentContext->filterChannelMask = ival;
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			#endif
			}
			break;

		case VG_MAX_SCISSOR_RECTS:
		case VG_MAX_DASH_COUNT:
		case VG_MAX_KERNEL_SIZE:
		case VG_MAX_SEPARABLE_KERNEL_SIZE:
		case VG_MAX_COLOR_RAMP_STOPS:
		case VG_MAX_IMAGE_WIDTH:
		case VG_MAX_IMAGE_HEIGHT:
		case VG_MAX_IMAGE_PIXELS:
		case VG_MAX_IMAGE_BYTES:
		case VG_MAX_FLOAT:
		case VG_MAX_GAUSSIAN_STD_DEVIATION:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;
		default:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			break;
	}
	AM_MEMORY_LOG("vgSetfv");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief Set the value of a parameter on the current context, vector of integer values.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param type parameter to set.
	\param count number of elements.
	\param values array of input values to assign.
*/
VG_API_CALL void VG_API_ENTRY vgSetiv(VGParamType type,
                                      VGint count,
                                      const VGint *values) VG_API_EXIT {

	AMint32 i, j, tmp;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgSetiv");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	if ((count < 0) || (!values && count > 0) || !amPointerIsAligned(values, 4)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgSetiv");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	switch (type) {

		case VG_MATRIX_MODE:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				if (values[0] < AM_FIRST_MATRIX_MODE || values[0] > AM_LAST_MATRIX_MODE)
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					amCtxMatrixModeSet(currentContext, (VGMatrixMode)values[0]);
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;

		case VG_FILL_RULE:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				if (values[0] < VG_EVEN_ODD || values[0] > VG_NON_ZERO)
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					currentContext->fillRule = (VGFillRule)values[0];
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;

		case VG_IMAGE_QUALITY:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				if (values[0] <= 0 || values[0] > (VG_IMAGE_QUALITY_NONANTIALIASED | VG_IMAGE_QUALITY_FASTER | VG_IMAGE_QUALITY_BETTER))
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					amCtxImageQualitySet(currentContext, values[0]);
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;

		case VG_RENDERING_QUALITY:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				if (values[0] < VG_RENDERING_QUALITY_NONANTIALIASED || values[0] > VG_RENDERING_QUALITY_BETTER)
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					amCtxRenderingQualitySet(currentContext, (VGRenderingQuality)values[0]);
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;

		case VG_BLEND_MODE:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
			#if defined(VG_MZT_advanced_blend_modes)
				if (values[0] < VG_BLEND_SRC || values[0] > VG_BLEND_EXCLUSION_MZT)
			#else
				if (values[0] < VG_BLEND_SRC || values[0] > VG_BLEND_ADDITIVE)
			#endif
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					currentContext->strokeBlendMode = currentContext->fillBlendMode = (VGBlendMode)values[0];
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;

	#if defined(VG_MZT_separable_blend_modes)
		case VG_STROKE_BLEND_MODE_MZT:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
			#if defined(VG_MZT_advanced_blend_modes)
				if (values[0] < VG_BLEND_SRC || values[0] > VG_BLEND_EXCLUSION_MZT)
			#else
				if (values[0] < VG_BLEND_SRC || values[0] > VG_BLEND_ADDITIVE)
			#endif
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					currentContext->strokeBlendMode = (VGBlendMode)values[0];
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;

		case VG_FILL_BLEND_MODE_MZT:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
			#if defined(VG_MZT_advanced_blend_modes)
				if (values[0] < VG_BLEND_SRC || values[0] > VG_BLEND_EXCLUSION_MZT)
			#else
				if (values[0] < VG_BLEND_SRC || values[0] > VG_BLEND_ADDITIVE)
			#endif
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					currentContext->fillBlendMode = (VGBlendMode)values[0];
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;
	#endif

		case VG_IMAGE_MODE:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				if (values[0] < VG_DRAW_IMAGE_NORMAL || values[0] > VG_DRAW_IMAGE_STENCIL)
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					currentContext->imageMode = (VGImageMode)values[0];
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;

		case VG_SCISSOR_RECTS:
			// count must be a multiple of four for OpenVG 1.0.1+
			if (count & 3)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				tmp = AM_MIN(count, currentContext->maxScissorRects);
				if (!amCtxScissorRectsSeti(currentContext, tmp, values))
					amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
				else
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case VG_COLOR_TRANSFORM:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				amCtxColorTransformSet(currentContext, ((VGboolean)values[0] != VG_FALSE) ? VG_TRUE : VG_FALSE);
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_COLOR_TRANSFORM_VALUES:
			if (count != 8)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				amCtxColorTransformValuesSeti(currentContext, values);
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;
	#endif

		case VG_STROKE_LINE_WIDTH:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				amCtxStrokeWidthSet(currentContext, (VGfloat)values[0]);
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_STROKE_CAP_STYLE:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				if (values[0] < VG_CAP_BUTT || values[0] > VG_CAP_SQUARE)
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					currentContext->startCapStyle = currentContext->endCapStyle = (VGCapStyle)values[0];
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;
	#if defined(VG_MZT_separable_cap_style)
		case VG_STROKE_START_CAP_STYLE_MZT:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				if (values[0] < VG_CAP_BUTT || values[0] > VG_CAP_SQUARE)
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					currentContext->startCapStyle = (VGCapStyle)values[0];
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;

		case VG_STROKE_END_CAP_STYLE_MZT:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				if (values[0] < VG_CAP_BUTT || values[0] > VG_CAP_SQUARE)
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					currentContext->endCapStyle = (VGCapStyle)values[0];
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;
	#endif

		case VG_STROKE_JOIN_STYLE:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				if (values[0] < VG_JOIN_MITER || values[0] > VG_JOIN_BEVEL)
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					currentContext->joinStyle = (VGJoinStyle)values[0];
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;

		case VG_STROKE_MITER_LIMIT:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				amCtxStrokeMiterLimitSet(currentContext, (VGfloat)values[0]);
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_STROKE_DASH_PATTERN:
			tmp = AM_MIN(count, currentContext->maxDashCount);
			if (amCtxStrokeDashPatternSeti(currentContext, tmp, values))
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			else
				amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
			break;

		case VG_STROKE_DASH_PHASE:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				amCtxStrokeDashPhaseSet(currentContext, (VGfloat)values[0]);
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_STROKE_DASH_PHASE_RESET:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				if (values[0] != VG_FALSE)
					currentContext->dashPhaseReset = VG_TRUE;
				else
					currentContext->dashPhaseReset = VG_FALSE;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_TILE_FILL_COLOR:
			if (count != 4)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				j = AM_MIN(count, 4);
				for (i = 0; i < j; ++i) currentContext->tileFillColor[i] = (VGfloat)values[i];
				for (i = j; i < 4; ++i) currentContext->tileFillColor[i] = 0.0f;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_CLEAR_COLOR:
			if (count != 4)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				j = AM_MIN(count, 4);
				for (i = 0; i < j; ++i) currentContext->clearColor[i] = (VGfloat)values[i];
				for (i = j; i < 4; ++i) currentContext->clearColor[i] = 0.0f;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case VG_GLYPH_ORIGIN:
			if (count != 2)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				for (i = 0; i < 2; ++i) currentContext->glyphOrigin[i] = (VGfloat)values[i];
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;
	#endif

		case VG_MASKING:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->masking = ((VGboolean)values[0] != VG_FALSE) ? VG_TRUE : VG_FALSE;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_SCISSORING:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->scissoring = ((VGboolean)values[0] != VG_FALSE) ? VG_TRUE : VG_FALSE;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_PIXEL_LAYOUT:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				if (values[0] < VG_PIXEL_LAYOUT_UNKNOWN || values[0] > VG_PIXEL_LAYOUT_BGR_HORIZONTAL)
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					currentContext->pixelLayout = (VGPixelLayout)values[0];
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			}
			break;

		case VG_SCREEN_LAYOUT:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;

		case VG_FILTER_FORMAT_LINEAR:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->filterFormatLinear = ((VGboolean)values[0] != VG_FALSE) ? VG_TRUE : VG_FALSE;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_FILTER_FORMAT_PREMULTIPLIED:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				currentContext->filterFormatPremultiplied = ((VGboolean)values[0] != VG_FALSE) ? VG_TRUE : VG_FALSE;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_FILTER_CHANNEL_MASK:
			if (count != 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				// undefined bits are ignored (this is the behavior of the reference implementation)
				currentContext->filterChannelMask = values[0];
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			#if 0
				// take care of negative (or invalid) values
				if (values[0] <= 0 || values[0] > (VG_RED | VG_GREEN | VG_BLUE | VG_ALPHA))
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
				else {
					currentContext->filterChannelMask = values[0];
					amCtxErrorSet(currentContext, VG_NO_ERROR);
				}
			#endif
			}
			break;

		case VG_MAX_SCISSOR_RECTS:
		case VG_MAX_DASH_COUNT:
		case VG_MAX_KERNEL_SIZE:
		case VG_MAX_SEPARABLE_KERNEL_SIZE:
		case VG_MAX_COLOR_RAMP_STOPS:
		case VG_MAX_IMAGE_WIDTH:
		case VG_MAX_IMAGE_HEIGHT:
		case VG_MAX_IMAGE_PIXELS:
		case VG_MAX_IMAGE_BYTES:
		case VG_MAX_FLOAT:
		case VG_MAX_GAUSSIAN_STD_DEVIATION:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			break;
		default:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			break;
	}
	AM_MEMORY_LOG("vgSetiv");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief Return the value of a parameter on the current context, scalar float value.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param type parameter to get.
	\return value of the requested parameter.
*/
VG_API_CALL VGfloat VG_API_ENTRY vgGetf(VGParamType type) VG_API_EXIT {

	VGfloat res;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgGetf");
		OPENVG_RETURN(0.0f)
	}

	switch (type) {

		case VG_MATRIX_MODE:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->matrixMode;
			break;

		case VG_FILL_RULE:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->fillRule;
			break;

		case VG_IMAGE_QUALITY:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->imageQuality;
			break;

		case VG_RENDERING_QUALITY:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->renderingQuality;
			break;

		case VG_BLEND_MODE:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->fillBlendMode;
			break;

	#if defined(VG_MZT_separable_blend_modes)
		case VG_STROKE_BLEND_MODE_MZT:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->strokeBlendMode;
			break;
		case VG_FILL_BLEND_MODE_MZT:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->fillBlendMode;
			break;
	#endif

		case VG_IMAGE_MODE:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->imageMode;
			break;

		case VG_SCISSOR_RECTS:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			res = 0.0f;
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case VG_COLOR_TRANSFORM:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->colorTransform;
			break;

		case VG_COLOR_TRANSFORM_VALUES:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			res = 0.0f;
			break;
	#endif

		case VG_STROKE_LINE_WIDTH:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->strokeLineWidth;
			break;

		case VG_STROKE_CAP_STYLE:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->startCapStyle;
			break;

	#if defined(VG_MZT_separable_cap_style)
		case VG_STROKE_START_CAP_STYLE_MZT:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->startCapStyle;
			break;
		case VG_STROKE_END_CAP_STYLE_MZT:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->endCapStyle;
			break;
	#endif

		case VG_STROKE_JOIN_STYLE:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->joinStyle;
			break;

		case VG_STROKE_MITER_LIMIT:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->miterLimit;
			break;

		case VG_STROKE_DASH_PATTERN:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			res = 0.0f;
			break;

		case VG_STROKE_DASH_PHASE:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->dashPhase;
			break;

		case VG_STROKE_DASH_PHASE_RESET:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->dashPhaseReset;
			break;

		case VG_TILE_FILL_COLOR:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			res = 0.0f;
			break;

		case VG_CLEAR_COLOR:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			res = 0.0f;
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case VG_GLYPH_ORIGIN:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			res = 0.0f;
			break;
	#endif

		case VG_MASKING:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->masking;
			break;

		case VG_SCISSORING:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->scissoring;
			break;

		case VG_PIXEL_LAYOUT:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->pixelLayout;
			break;

		case VG_SCREEN_LAYOUT:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->screenLayout;
			break;

		case VG_FILTER_FORMAT_LINEAR:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->filterFormatLinear;
			break;

		case VG_FILTER_FORMAT_PREMULTIPLIED:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->filterFormatPremultiplied;
			break;

		case VG_FILTER_CHANNEL_MASK:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->filterChannelMask;
			break;

		case VG_MAX_SCISSOR_RECTS:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->maxScissorRects;
			break;

		case VG_MAX_DASH_COUNT:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->maxDashCount;
			break;

		case VG_MAX_KERNEL_SIZE:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->maxKernelSize;
			break;

		case VG_MAX_SEPARABLE_KERNEL_SIZE:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->maxSeparableKernelSize;
			break;

		case VG_MAX_COLOR_RAMP_STOPS:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->maxColorRampStops;
			break;

		case VG_MAX_IMAGE_WIDTH:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->maxImageWidth;
			break;

		case VG_MAX_IMAGE_HEIGHT:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->maxImageHeight;
			break;

		case VG_MAX_IMAGE_PIXELS:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->maxImagePixels;
			break;

		case VG_MAX_IMAGE_BYTES:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->maxImageBytes;
			break;

		case VG_MAX_FLOAT:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->maxFloat;
			break;

		case VG_MAX_GAUSSIAN_STD_DEVIATION:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGfloat)currentContext->maxGaussianStdDeviation;
			break;

		default:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			res = 0.0f;
			break;
	}
	AM_MEMORY_LOG("vgGetf");
	OPENVG_RETURN(res)
}

/*!
	\brief Return the value of a parameter on the current context, scalar integer value.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param type parameter to get.
	\return value of the requested parameter.
*/
VG_API_CALL VGint VG_API_ENTRY vgGeti(VGParamType type) VG_API_EXIT {

	VGint res;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgGeti");
		OPENVG_RETURN(0)
	}

	switch (type) {

		case VG_MATRIX_MODE:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->matrixMode;
			break;

		case VG_FILL_RULE:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->fillRule;
			break;

		case VG_IMAGE_QUALITY:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->imageQuality;
			break;

		case VG_RENDERING_QUALITY:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->renderingQuality;
			break;

		case VG_BLEND_MODE:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->fillBlendMode;
			break;

	#if defined(VG_MZT_separable_blend_modes)
		case VG_STROKE_BLEND_MODE_MZT:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->strokeBlendMode;
			break;
		case VG_FILL_BLEND_MODE_MZT:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->fillBlendMode;
			break;
	#endif

		case VG_IMAGE_MODE:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->imageMode;
			break;

		case VG_SCISSOR_RECTS:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			res = 0;
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case VG_COLOR_TRANSFORM:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGint)currentContext->colorTransform;
			break;

		case VG_COLOR_TRANSFORM_VALUES:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			res = 0;
			break;
	#endif

		case VG_STROKE_LINE_WIDTH:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = ((VGint)amFloorf(currentContext->strokeLineWidth));
			break;

		case VG_STROKE_CAP_STYLE:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->startCapStyle;
			break;

	#if defined(VG_MZT_separable_cap_style)
		case VG_STROKE_START_CAP_STYLE_MZT:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->startCapStyle;
			break;
		case VG_STROKE_END_CAP_STYLE_MZT:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->endCapStyle;
			break;
	#endif

		case VG_STROKE_JOIN_STYLE:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->joinStyle;
			break;

		case VG_STROKE_MITER_LIMIT:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = ((VGint)amFloorf(currentContext->miterLimit));
			break;

		case VG_STROKE_DASH_PATTERN:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			res = 0;
			break;

		case VG_STROKE_DASH_PHASE:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = ((VGint)amFloorf(currentContext->dashPhase));
			break;

		case VG_STROKE_DASH_PHASE_RESET:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGint)currentContext->dashPhaseReset;
			break;

		case VG_TILE_FILL_COLOR:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			res = 0;
			break;

		case VG_CLEAR_COLOR:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			res = 0;
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case VG_GLYPH_ORIGIN:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			res = 0;
			break;
	#endif

		case VG_MASKING:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGint)currentContext->masking;
			break;

		case VG_SCISSORING:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGint)currentContext->scissoring;
			break;

		case VG_PIXEL_LAYOUT:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->pixelLayout;
			break;

		case VG_SCREEN_LAYOUT:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->screenLayout;
			break;

		case VG_FILTER_FORMAT_LINEAR:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGint)currentContext->filterFormatLinear;
			break;

		case VG_FILTER_FORMAT_PREMULTIPLIED:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGint)currentContext->filterFormatPremultiplied;
			break;

		case VG_FILTER_CHANNEL_MASK:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->filterChannelMask;
			break;

		case VG_MAX_SCISSOR_RECTS:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->maxScissorRects;
			break;

		case VG_MAX_DASH_COUNT:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->maxDashCount;
			break;

		case VG_MAX_KERNEL_SIZE:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->maxKernelSize;
			break;

		case VG_MAX_SEPARABLE_KERNEL_SIZE:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->maxSeparableKernelSize;
			break;

		case VG_MAX_COLOR_RAMP_STOPS:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->maxColorRampStops;
			break;

		case VG_MAX_IMAGE_WIDTH:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->maxImageWidth;
			break;

		case VG_MAX_IMAGE_HEIGHT:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->maxImageHeight;
			break;

		case VG_MAX_IMAGE_PIXELS:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->maxImagePixels;
			break;

		case VG_MAX_IMAGE_BYTES:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->maxImageBytes;
			break;

		case VG_MAX_FLOAT:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = VG_MAXINT;
			break;

		case VG_MAX_GAUSSIAN_STD_DEVIATION:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = currentContext->maxGaussianStdDeviation;
			break;

		default:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			res = 0;
			break;
	}
	AM_MEMORY_LOG("vgGeti");
	OPENVG_RETURN(res)
}

/*!
	\brief Returns the maximum number of elements in the vector that will be retrieved by the vgGetiv or
	vgGetfv functions if called with the given param type argument.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param type parameter type.
	\return maximum number of elements.
*/
VG_API_CALL VGint VG_API_ENTRY vgGetVectorSize(VGParamType type) VG_API_EXIT {

	VGint res;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgGetVectorSize");
		OPENVG_RETURN(0)
	}

	switch (type) {

		case VG_MATRIX_MODE:
		case VG_FILL_RULE:
		case VG_IMAGE_QUALITY:
		case VG_RENDERING_QUALITY:
		case VG_BLEND_MODE:
	#if defined(VG_MZT_separable_blend_modes)
		case VG_STROKE_BLEND_MODE_MZT:
		case VG_FILL_BLEND_MODE_MZT:
	#endif
		case VG_IMAGE_MODE:
	#if (AM_OPENVG_VERSION >= 110)
		case VG_COLOR_TRANSFORM:
	#endif
		case VG_STROKE_LINE_WIDTH:
		case VG_STROKE_CAP_STYLE:
	#if defined(VG_MZT_separable_cap_style)
		case VG_STROKE_START_CAP_STYLE_MZT:
		case VG_STROKE_END_CAP_STYLE_MZT:
	#endif
		case VG_STROKE_JOIN_STYLE:
		case VG_STROKE_MITER_LIMIT:
		case VG_STROKE_DASH_PHASE:
		case VG_STROKE_DASH_PHASE_RESET:
		case VG_MASKING:
		case VG_SCISSORING:
		case VG_PIXEL_LAYOUT:
		case VG_SCREEN_LAYOUT:
		case VG_FILTER_FORMAT_LINEAR:
		case VG_FILTER_FORMAT_PREMULTIPLIED:
		case VG_FILTER_CHANNEL_MASK:
		case VG_MAX_SCISSOR_RECTS:
		case VG_MAX_DASH_COUNT:
		case VG_MAX_KERNEL_SIZE:
		case VG_MAX_SEPARABLE_KERNEL_SIZE:
		case VG_MAX_COLOR_RAMP_STOPS:
		case VG_MAX_IMAGE_WIDTH:
		case VG_MAX_IMAGE_HEIGHT:
		case VG_MAX_IMAGE_PIXELS:
		case VG_MAX_IMAGE_BYTES:
		case VG_MAX_FLOAT:
		case VG_MAX_GAUSSIAN_STD_DEVIATION:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = 1;
			break;

		case VG_SCISSOR_RECTS:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGint)currentContext->scissorRects.size;
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case VG_COLOR_TRANSFORM_VALUES:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = 8;
			break;
	#endif

		case VG_STROKE_DASH_PATTERN:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = (VGint)currentContext->dashPattern.size;
			break;

		case VG_TILE_FILL_COLOR:
		case VG_CLEAR_COLOR:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = 4;
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case VG_GLYPH_ORIGIN:
			amCtxErrorSet(currentContext, VG_NO_ERROR);
			res = 2;
			break;
	#endif

		default:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			res = 0;
			break;
	}
	AM_MEMORY_LOG("vgGetVectorSize");
	OPENVG_RETURN(res)
}

/*!
	\brief Return the value of a parameter on the current context, vector of float values.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param type parameter to get.
	\param count number of elements to get.
	\param values array of output values.
*/
VG_API_CALL void VG_API_ENTRY vgGetfv(VGParamType type,
                                      VGint count,
                                      VGfloat *values) VG_API_EXIT {

	AMint32 i, j;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgGetfv");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	if (!values || count <= 0 || !amPointerIsAligned(values, 4)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgGetfv");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	switch (type) {

		case VG_MATRIX_MODE:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->matrixMode;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_FILL_RULE:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->fillRule;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_IMAGE_QUALITY:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->imageQuality;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_RENDERING_QUALITY:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->renderingQuality;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_BLEND_MODE:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->fillBlendMode;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

	#if defined(VG_MZT_separable_blend_modes)
		case VG_STROKE_BLEND_MODE_MZT:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->strokeBlendMode;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_FILL_BLEND_MODE_MZT:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->fillBlendMode;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;
	#endif

		case VG_IMAGE_MODE:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->imageMode;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_STROKE_LINE_WIDTH:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->strokeLineWidth;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_STROKE_CAP_STYLE:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->startCapStyle;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

	#if defined(VG_MZT_separable_cap_style)
		case VG_STROKE_START_CAP_STYLE_MZT:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->startCapStyle;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;
		case VG_STROKE_END_CAP_STYLE_MZT:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->endCapStyle;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
	#if defined RIM_VG_SRC
			break;
	#endif
	#endif

		case VG_STROKE_JOIN_STYLE:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->joinStyle;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_STROKE_MITER_LIMIT:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->miterLimit;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_STROKE_DASH_PHASE:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->dashPhase;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_STROKE_DASH_PHASE_RESET:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->dashPhaseReset;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_MASKING:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->masking;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_SCISSORING:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->scissoring;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_PIXEL_LAYOUT:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->pixelLayout;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_SCREEN_LAYOUT:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->screenLayout;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_FILTER_FORMAT_LINEAR:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->filterFormatLinear;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_FILTER_FORMAT_PREMULTIPLIED:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->filterFormatPremultiplied;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_FILTER_CHANNEL_MASK:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->filterChannelMask;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_MAX_SCISSOR_RECTS:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->maxScissorRects;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_MAX_DASH_COUNT:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->maxDashCount;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_MAX_KERNEL_SIZE:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->maxKernelSize;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_MAX_SEPARABLE_KERNEL_SIZE:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->maxSeparableKernelSize;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_MAX_COLOR_RAMP_STOPS:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->maxColorRampStops;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_MAX_IMAGE_WIDTH:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->maxImageWidth;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_MAX_IMAGE_HEIGHT:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->maxImageHeight;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_MAX_IMAGE_PIXELS:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->maxImagePixels;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_MAX_IMAGE_BYTES:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->maxImageBytes;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_MAX_FLOAT:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->maxFloat;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_MAX_GAUSSIAN_STD_DEVIATION:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->maxGaussianStdDeviation;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_SCISSOR_RECTS:
			j = (VGint)currentContext->scissorRects.size;
			if (count > j)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				j = AM_MIN(j, count);
				for (i = 0; i < j; ++i) values[i] = (VGfloat)currentContext->scissorRects.data[i];
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case VG_COLOR_TRANSFORM:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGfloat)currentContext->colorTransform;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_COLOR_TRANSFORM_VALUES:
			if (count > 8)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				j = AM_MIN(8, count);
				for (i = 0; i < j; ++i) values[i] = currentContext->colorTransformValues[i];
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;
	#endif

		case VG_STROKE_DASH_PATTERN:
			j = (VGint)currentContext->dashPattern.size;
			if (count > j)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				j = AM_MIN(j, count);
				for (i = 0; i < j; ++i) values[i] = currentContext->dashPattern.data[i];
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_TILE_FILL_COLOR:
			if (count > 4)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				j = AM_MIN(4, count);
				for (i = 0; i < j; ++i) values[i] = currentContext->tileFillColor[i];
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_CLEAR_COLOR:
			if (count > 4)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				j = AM_MIN(4, count);
				for (i = 0; i < j; ++i) values[i] = currentContext->clearColor[i];
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case VG_GLYPH_ORIGIN:
			if (count > 2)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				j = AM_MIN(2, count);
				for (i = 0; i < j; ++i) values[i] = currentContext->glyphOrigin[i];
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;
	#endif

		default:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			break;
	}
	AM_MEMORY_LOG("vgGetfv");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief Return the value of a parameter on the current context, vector of integer values.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param type parameter to get.
	\param count number of elements to get.
	\param values array of output values.
*/
VG_API_CALL void VG_API_ENTRY vgGetiv(VGParamType type,
                                      VGint count,
                                      VGint *values) VG_API_EXIT {

	AMint32 i, j;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgGetiv");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	if (!values || count <= 0 || !amPointerIsAligned(values, 4)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgGetiv");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	switch (type) {

		case VG_MATRIX_MODE:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->matrixMode;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_FILL_RULE:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->fillRule;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_IMAGE_QUALITY:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->imageQuality;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_RENDERING_QUALITY:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->renderingQuality;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_BLEND_MODE:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->fillBlendMode;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

	#if defined(VG_MZT_separable_blend_modes)
		case VG_STROKE_BLEND_MODE_MZT:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->strokeBlendMode;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_FILL_BLEND_MODE_MZT:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->fillBlendMode;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;
	#endif

		case VG_IMAGE_MODE:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->imageMode;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_STROKE_LINE_WIDTH:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGint)amFloorf(currentContext->strokeLineWidth);
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_STROKE_CAP_STYLE:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->startCapStyle;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

	#if defined(VG_MZT_separable_cap_style)
		case VG_STROKE_START_CAP_STYLE_MZT:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->startCapStyle;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_STROKE_END_CAP_STYLE_MZT:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->endCapStyle;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;
	#endif

		case VG_STROKE_JOIN_STYLE:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->joinStyle;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_STROKE_MITER_LIMIT:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGint)amFloorf(currentContext->miterLimit);
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_STROKE_DASH_PHASE:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGint)amFloorf(currentContext->dashPhase);
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_STROKE_DASH_PHASE_RESET:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGint)currentContext->dashPhaseReset;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_MASKING:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGint)currentContext->masking;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_SCISSORING:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGint)currentContext->scissoring;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_PIXEL_LAYOUT:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->pixelLayout;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_SCREEN_LAYOUT:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->screenLayout;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_FILTER_FORMAT_LINEAR:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGint)currentContext->filterFormatLinear;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_FILTER_FORMAT_PREMULTIPLIED:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGint)currentContext->filterFormatPremultiplied;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_FILTER_CHANNEL_MASK:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->filterChannelMask;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_MAX_SCISSOR_RECTS:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->maxScissorRects;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_MAX_DASH_COUNT:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->maxDashCount;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_MAX_KERNEL_SIZE:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->maxKernelSize;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_MAX_SEPARABLE_KERNEL_SIZE:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->maxSeparableKernelSize;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_MAX_COLOR_RAMP_STOPS:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->maxColorRampStops;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_MAX_IMAGE_WIDTH:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->maxImageWidth;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_MAX_IMAGE_HEIGHT:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->maxImageHeight;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_MAX_IMAGE_PIXELS:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->maxImagePixels;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_MAX_IMAGE_BYTES:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->maxImageBytes;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_MAX_FLOAT:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = VG_MAXINT;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_MAX_GAUSSIAN_STD_DEVIATION:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = currentContext->maxGaussianStdDeviation;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_SCISSOR_RECTS:
			j = (VGint)currentContext->scissorRects.size;
			if (count > j)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				j = AM_MIN(j, count);
				for (i = 0; i < j; ++i) values[i] = currentContext->scissorRects.data[i];
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case VG_COLOR_TRANSFORM:
			if (count > 1)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				values[0] = (VGint)currentContext->colorTransform;
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_COLOR_TRANSFORM_VALUES:
			if (count > 8)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				j = AM_MIN(8, count);
				for (i = 0; i < j; ++i) values[i] = (VGint)amFloorf(currentContext->colorTransformValues[i]);
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;
	#endif

		case VG_STROKE_DASH_PATTERN:
			j = (VGint)currentContext->dashPattern.size;
			if (count > j)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				j = AM_MIN(j, count);
				for (i = 0; i < j; ++i) values[i] = (VGint)amFloorf(currentContext->dashPattern.data[i]);
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_TILE_FILL_COLOR:
			if (count > 4)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				j = AM_MIN(4, count);
				for (i = 0; i < j; ++i) values[i] = (VGint)amFloorf(currentContext->tileFillColor[i]);
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

		case VG_CLEAR_COLOR:
			if (count > 4)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				j = AM_MIN(4, count);
				for (i = 0; i < j; ++i) values[i] = (VGint)amFloorf(currentContext->clearColor[i]);
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case VG_GLYPH_ORIGIN:
			if (count > 2)
				amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			else {
				j = AM_MIN(2, count);
				for (i = 0; i < j; ++i) values[i] = (VGint)amFloorf(currentContext->glyphOrigin[i]);
				amCtxErrorSet(currentContext, VG_NO_ERROR);
			}
			break;
	#endif

		default:
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			break;
	}
	AM_MEMORY_LOG("vgGetiv");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}


// *********************************************************************
//               Getters and setters for VGHandle objects
// *********************************************************************

/*!
	\brief Set the value of a parameter on a given VGHandle-based object, scalar float value.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param object object whose parameter is to set.
	\param paramType type of the parameter.
	\param value input float value to assign.
*/
VG_API_CALL void VG_API_ENTRY vgSetParameterf(VGHandle object,
                                              VGint paramType,
                                              VGfloat value) VG_API_EXIT {

	AMuint32 id;
	AMint32 ival;
	AMhandle obj;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgSetParameterf");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// handle NaN and Inf values
	value = amNanInfFix(value);

	id = amCtxHandleValid(currentContext, object);
	obj = (id != AM_INVALID_HANDLE_ID) ? currentContext->handles->createdHandlesList.data[object] : NULL;

	switch (id) {

		case AM_PATH_HANDLE_ID:
			switch (paramType) {
				case VG_PATH_FORMAT:
				case VG_PATH_DATATYPE:
				case VG_PATH_SCALE:
				case VG_PATH_BIAS:
				case VG_PATH_NUM_SEGMENTS:
				case VG_PATH_NUM_COORDS:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			}
			break;

		case AM_IMAGE_HANDLE_ID:
			switch (paramType) {
				case VG_IMAGE_FORMAT:
				case VG_IMAGE_WIDTH:
				case VG_IMAGE_HEIGHT:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			}
			break;

		case AM_PAINT_HANDLE_ID:
			switch (paramType) {
				case VG_PAINT_TYPE:
					ival = (AMint32)amFloorf(value);
				#if defined(VG_MZT_conical_gradient)
					if (ival < VG_PAINT_TYPE_COLOR || ival > VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)
				#else
					if (ival < VG_PAINT_TYPE_COLOR || ival > VG_PAINT_TYPE_PATTERN)
				#endif
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						amPaintTypeSet((AMPaint *)obj, (VGPaintType)ival);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				case VG_PAINT_COLOR:
				case VG_PAINT_COLOR_RAMP_STOPS:
				case VG_PAINT_LINEAR_GRADIENT:
				case VG_PAINT_RADIAL_GRADIENT:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
				case VG_PAINT_COLOR_RAMP_SPREAD_MODE:
					ival = (AMint32)amFloorf(value);
					if (ival < VG_COLOR_RAMP_SPREAD_PAD || ival > VG_COLOR_RAMP_SPREAD_REFLECT)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						amPaintColorRampSpreadModeSet((AMPaint *)obj, (VGColorRampSpreadMode)ival);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				case VG_PAINT_COLOR_RAMP_PREMULTIPLIED:
					ival = (AMint32)amFloorf(value);
					if (ival != VG_FALSE)
						amPaintColorRampPremultipliedSet((AMPaint *)obj, VG_TRUE);
					else
						amPaintColorRampPremultipliedSet((AMPaint *)obj, VG_FALSE);
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					break;

			#if defined(VG_MZT_color_ramp_interpolation)
				case VG_PAINT_COLOR_RAMP_INTERPOLATION_TYPE_MZT:
					ival = (AMint32)amFloorf(value);
					if (ival < VG_COLOR_RAMP_INTERPOLATION_LINEAR_MZT || ival > VG_COLOR_RAMP_INTERPOLATION_SMOOTH_MZT)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						amPaintColorRampInterpolationTypeSet((AMPaint *)obj, (VGColorRampInterpolationTypeMzt)ival);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
			#endif

				case VG_PAINT_PATTERN_TILING_MODE:
					ival = (AMint32)amFloorf(value);
					if (ival < VG_TILE_FILL || ival > VG_TILE_REFLECT)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						amPaintTilingModeSet((AMPaint *)obj, (VGTilingMode)ival);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			}
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case AM_FONT_HANDLE_ID:
			switch (paramType) {
				case VG_FONT_NUM_GLYPHS:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			}
			break;
	#endif

		default:
			amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
			break;
	}
	AM_MEMORY_LOG("vgSetParameterf");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief Set the value of a parameter on a given VGHandle-based object, scalar integer value.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param object object whose parameter is to set.
	\param paramType type of the parameter.
	\param value input integer value to assign.
*/
VG_API_CALL void VG_API_ENTRY vgSetParameteri(VGHandle object,
                                              VGint paramType,
                                              VGint value) VG_API_EXIT {

	AMuint32 id;
	AMhandle obj;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgSetParameteri");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	id = amCtxHandleValid(currentContext, object);
	obj = (id != AM_INVALID_HANDLE_ID) ? currentContext->handles->createdHandlesList.data[object] : NULL;

	switch (id) {

		case AM_PATH_HANDLE_ID:
			switch (paramType) {
				case VG_PATH_FORMAT:
				case VG_PATH_DATATYPE:
				case VG_PATH_SCALE:
				case VG_PATH_BIAS:
				case VG_PATH_NUM_SEGMENTS:
				case VG_PATH_NUM_COORDS:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			}
			break;

		case AM_IMAGE_HANDLE_ID:
			switch (paramType) {
				case VG_IMAGE_FORMAT:
				case VG_IMAGE_WIDTH:
				case VG_IMAGE_HEIGHT:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			}
			break;

		case AM_PAINT_HANDLE_ID:
			switch (paramType) {
				case VG_PAINT_TYPE:
				#if defined(VG_MZT_conical_gradient)
					if (value < VG_PAINT_TYPE_COLOR || value > VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)
				#else
					if (value < VG_PAINT_TYPE_COLOR || value > VG_PAINT_TYPE_PATTERN)
				#endif
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						amPaintTypeSet((AMPaint *)obj, (VGPaintType)value);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				case VG_PAINT_COLOR:
				case VG_PAINT_COLOR_RAMP_STOPS:
				case VG_PAINT_LINEAR_GRADIENT:
				case VG_PAINT_RADIAL_GRADIENT:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
				case VG_PAINT_COLOR_RAMP_SPREAD_MODE:
					if (value < VG_COLOR_RAMP_SPREAD_PAD || value > VG_COLOR_RAMP_SPREAD_REFLECT)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						amPaintColorRampSpreadModeSet((AMPaint *)obj, (VGColorRampSpreadMode)value);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;

				case VG_PAINT_COLOR_RAMP_PREMULTIPLIED:
					if (value != VG_FALSE)
						amPaintColorRampPremultipliedSet((AMPaint *)obj, VG_TRUE);
					else
						amPaintColorRampPremultipliedSet((AMPaint *)obj, VG_FALSE);
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					break;

			#if defined(VG_MZT_color_ramp_interpolation)
				case VG_PAINT_COLOR_RAMP_INTERPOLATION_TYPE_MZT:
					if (value < VG_COLOR_RAMP_INTERPOLATION_LINEAR_MZT || value > VG_COLOR_RAMP_INTERPOLATION_SMOOTH_MZT)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						amPaintColorRampInterpolationTypeSet((AMPaint *)obj, (VGColorRampInterpolationTypeMzt)value);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
			#endif

				case VG_PAINT_PATTERN_TILING_MODE:
					if (value < VG_TILE_FILL || value > VG_TILE_REFLECT)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						amPaintTilingModeSet((AMPaint *)obj, (VGTilingMode)value);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			}
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case AM_FONT_HANDLE_ID:
			switch (paramType) {
				case VG_FONT_NUM_GLYPHS:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			}
			break;
	#endif

		default:
			amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
			break;
	}
	AM_MEMORY_LOG("vgSetParameteri");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief Set the value of a parameter on a given VGHandle-based object, vector of float values.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param object object whose parameter is to set.
	\param paramType type of the parameter.
	\param count number of elements.
	\param values array of input values to assign.
*/
VG_API_CALL void VG_API_ENTRY vgSetParameterfv(VGHandle object,
                                               VGint paramType,
                                               VGint count,
                                               const VGfloat *values) VG_API_EXIT {

	AMuint32 id;
	AMint32 ival;
	AMhandle obj;
#if defined(VG_MZT_conical_gradient)
	AMfloat repeats;
#endif
	AMfloat pt0[2], pt1[2], radius;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgSetParameterfv");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	if ((!values && count > 0) || (count < 0) || !amPointerIsAligned(values, 4)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgSetParameterfv");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	id = amCtxHandleValid(currentContext, object);
	obj = (id != AM_INVALID_HANDLE_ID) ? currentContext->handles->createdHandlesList.data[object] : NULL;

	switch (id) {

		case AM_PATH_HANDLE_ID:
			switch (paramType) {
				case VG_PATH_FORMAT:
				case VG_PATH_DATATYPE:
				case VG_PATH_SCALE:
				case VG_PATH_BIAS:
				case VG_PATH_NUM_SEGMENTS:
				case VG_PATH_NUM_COORDS:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			}
			break;

		case AM_IMAGE_HANDLE_ID:
			switch (paramType) {
				case VG_IMAGE_FORMAT:
				case VG_IMAGE_WIDTH:
				case VG_IMAGE_HEIGHT:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			}
			break;

		case AM_PAINT_HANDLE_ID:
			switch (paramType) {
				case VG_PAINT_TYPE:
					if (count == 1) {
						ival = (AMint32)amFloorf(amNanInfFix(values[0]));
					#if defined(VG_MZT_conical_gradient)
						if (ival < VG_PAINT_TYPE_COLOR || ival > VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)
					#else
						if (ival < VG_PAINT_TYPE_COLOR || ival > VG_PAINT_TYPE_PATTERN)
					#endif
							amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
						else {
							amPaintTypeSet((AMPaint *)obj, (VGPaintType)ival);
							amCtxErrorSet(currentContext, VG_NO_ERROR);
						}
					}
					else
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;

				case VG_PAINT_COLOR:
					if (count != 4)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						amPaintColorSet((AMPaint *)obj, values);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;

				case VG_PAINT_COLOR_RAMP_STOPS:
					if (count % 5 != 0)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						if (!amPaintColorStopsSetf((AMPaint *)obj, values, count / 5))
							amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
						else
							amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;

				case VG_PAINT_LINEAR_GRADIENT:
					if (count != 4)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						pt0[0] = amNanInfFix(values[0]);
						pt0[1] = amNanInfFix(values[1]);
						pt1[0] = amNanInfFix(values[2]);
						pt1[1] = amNanInfFix(values[3]);
						amPaintLinGradParametersSet((AMPaint *)obj, pt0, pt1);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;

				case VG_PAINT_RADIAL_GRADIENT:
					if (count != 5)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						pt0[0] = amNanInfFix(values[0]);
						pt0[1] = amNanInfFix(values[1]);
						pt1[0] = amNanInfFix(values[2]);
						pt1[1] = amNanInfFix(values[3]);
						radius = amNanInfFix(values[4]);
						amPaintRadGradParametersSet((AMPaint *)obj, pt0, pt1, radius);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;

			#if defined(VG_MZT_conical_gradient)
				case VG_PAINT_CONICAL_GRADIENT_MZT:
					if (count != 5)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						pt0[0] = amNanInfFix(values[0]);
						pt0[1] = amNanInfFix(values[1]);
						pt1[0] = amNanInfFix(values[2]);
						pt1[1] = amNanInfFix(values[3]);
						repeats = amNanInfFix(values[4]);
						amPaintConGradParametersSet((AMPaint *)obj, pt0, pt1, repeats);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
			#endif

				case VG_PAINT_COLOR_RAMP_SPREAD_MODE:
					if (count == 1) {
						ival = (AMint32)amFloorf(amNanInfFix(values[0]));
						if (ival < VG_COLOR_RAMP_SPREAD_PAD || ival > VG_COLOR_RAMP_SPREAD_REFLECT)
							amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
						else {
							amPaintColorRampSpreadModeSet((AMPaint *)obj, (VGColorRampSpreadMode)ival);
							amCtxErrorSet(currentContext, VG_NO_ERROR);
						}
					}
					else
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;

				case VG_PAINT_COLOR_RAMP_PREMULTIPLIED:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						ival = (AMint32)amFloorf(amNanInfFix(values[0]));
						if (ival != VG_FALSE)
							amPaintColorRampPremultipliedSet((AMPaint *)obj, VG_TRUE);
						else
							amPaintColorRampPremultipliedSet((AMPaint *)obj, VG_FALSE);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;

			#if defined(VG_MZT_color_ramp_interpolation)
				case VG_PAINT_COLOR_RAMP_INTERPOLATION_TYPE_MZT:
					if (count == 1) {
						ival = (AMint32)amFloorf(amNanInfFix(values[0]));
						if (ival < VG_COLOR_RAMP_INTERPOLATION_LINEAR_MZT || ival > VG_COLOR_RAMP_INTERPOLATION_SMOOTH_MZT)
							amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
						else {
							amPaintColorRampInterpolationTypeSet((AMPaint *)obj, (VGColorRampInterpolationTypeMzt)ival);
							amCtxErrorSet(currentContext, VG_NO_ERROR);
						}
					}
					else
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			#endif

				case VG_PAINT_PATTERN_TILING_MODE:
					if (count == 1) {
						ival = (AMint32)amFloorf(amNanInfFix(values[0]));
						if (ival < VG_TILE_FILL || ival > VG_TILE_REFLECT)
							amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
						else {
							amPaintTilingModeSet((AMPaint *)obj, (VGTilingMode)ival);
							amCtxErrorSet(currentContext, VG_NO_ERROR);
						}
					}
					else
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			}
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case AM_FONT_HANDLE_ID:
			switch (paramType) {
				case VG_FONT_NUM_GLYPHS:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			}
			break;
	#endif

		default:
			amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
			break;
	}
	AM_MEMORY_LOG("vgSetParameterfv");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief Set the value of a parameter on a given VGHandle-based object, vector of integer values.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param object object whose parameter is to set.
	\param paramType type of the parameter.
	\param count number of elements.
	\param values array of input values to assign.
*/
VG_API_CALL void VG_API_ENTRY vgSetParameteriv(VGHandle object,
                                               VGint paramType,
                                               VGint count,
                                               const VGint *values) VG_API_EXIT {

	AMuint32 id;
	AMint32 i;
	AMhandle obj;
	AMfloat col[4];
	AMint32 ival;
	AMfloat linGrad[4], radGrad[5];
#if defined(VG_MZT_conical_gradient)
	AMfloat conGrad[5], repeats;
#endif
	AMfloat pt0[2], pt1[2], radius;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgSetParameteriv");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	if ((!values && count > 0) || (count < 0) || !amPointerIsAligned(values, 4)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgSetParameteriv");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	id = amCtxHandleValid(currentContext, object);
	obj = (id != AM_INVALID_HANDLE_ID) ? currentContext->handles->createdHandlesList.data[object] : NULL;

	switch (id) {

		case AM_PATH_HANDLE_ID:
			switch (paramType) {
				case VG_PATH_FORMAT:
				case VG_PATH_DATATYPE:
				case VG_PATH_SCALE:
				case VG_PATH_BIAS:
				case VG_PATH_NUM_SEGMENTS:
				case VG_PATH_NUM_COORDS:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			}
			break;

		case AM_IMAGE_HANDLE_ID:
			switch (paramType) {
				case VG_IMAGE_FORMAT:
				case VG_IMAGE_WIDTH:
				case VG_IMAGE_HEIGHT:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			}
			break;

		case AM_PAINT_HANDLE_ID:
			switch (paramType) {
				case VG_PAINT_TYPE:
					if (count == 1) {
						ival = values[0];
					#if defined(VG_MZT_conical_gradient)
						if (ival < VG_PAINT_TYPE_COLOR || ival > VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)
					#else
						if (ival < VG_PAINT_TYPE_COLOR || ival > VG_PAINT_TYPE_PATTERN)
					#endif
							amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
						else {
							amPaintTypeSet((AMPaint *)obj, (VGPaintType)ival);
							amCtxErrorSet(currentContext, VG_NO_ERROR);
						}
					}
					else
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;

				case VG_PAINT_COLOR:
					if (count != 4)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						for (i = 0; i < 4; ++i) col[i] = (AMfloat)values[i];
						amPaintColorSet((AMPaint *)obj, col);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;

				case VG_PAINT_COLOR_RAMP_STOPS:
					if (count % 5 != 0)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						if (!amPaintColorStopsSeti((AMPaint *)obj, values, count / 5))
							amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
						else
							amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;

				case VG_PAINT_LINEAR_GRADIENT:
					if (count != 4)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						for (i = 0; i < 4; ++i) linGrad[i] = (AMfloat)values[i];
						pt0[0] = linGrad[0];
						pt0[1] = linGrad[1];
						pt1[0] = linGrad[2];
						pt1[1] = linGrad[3];
						amPaintLinGradParametersSet((AMPaint *)obj, pt0, pt1);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;

				case VG_PAINT_RADIAL_GRADIENT:
					if (count != 5)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						for (i = 0; i < 5; ++i) radGrad[i] = (AMfloat)values[i];
						pt0[0] = radGrad[0];
						pt0[1] = radGrad[1];
						pt1[0] = radGrad[2];
						pt1[1] = radGrad[3];
						radius = radGrad[4];
						amPaintRadGradParametersSet((AMPaint *)obj, pt0, pt1, radius);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
			#if defined(VG_MZT_conical_gradient)
				case VG_PAINT_CONICAL_GRADIENT_MZT:
					if (count != 5)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						for (i = 0; i < 5; ++i) conGrad[i] = (AMfloat)values[i];
						pt0[0] = conGrad[0];
						pt0[1] = conGrad[1];
						pt1[0] = conGrad[2];
						pt1[1] = conGrad[3];
						repeats = conGrad[4];
						amPaintConGradParametersSet((AMPaint *)obj, pt0, pt1, repeats);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
			#endif

				case VG_PAINT_COLOR_RAMP_SPREAD_MODE:
					if (count == 1) {
						ival = values[0];
						if (ival < VG_COLOR_RAMP_SPREAD_PAD || ival > VG_COLOR_RAMP_SPREAD_REFLECT)
							amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
						else {
							amPaintColorRampSpreadModeSet((AMPaint *)obj, (VGColorRampSpreadMode)ival);
							amCtxErrorSet(currentContext, VG_NO_ERROR);
						}
					}
					else
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;

				case VG_PAINT_COLOR_RAMP_PREMULTIPLIED:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						ival = values[0];
						if (ival != VG_FALSE)
							amPaintColorRampPremultipliedSet((AMPaint *)obj, VG_TRUE);
						else
							amPaintColorRampPremultipliedSet((AMPaint *)obj, VG_FALSE);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;

			#if defined(VG_MZT_color_ramp_interpolation)
				case VG_PAINT_COLOR_RAMP_INTERPOLATION_TYPE_MZT:
					if (count == 1) {
						ival = values[0];
						if (ival < VG_COLOR_RAMP_INTERPOLATION_LINEAR_MZT || ival > VG_COLOR_RAMP_INTERPOLATION_SMOOTH_MZT)
							amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
						else {
							amPaintColorRampInterpolationTypeSet((AMPaint *)obj, (VGColorRampInterpolationTypeMzt)ival);
							amCtxErrorSet(currentContext, VG_NO_ERROR);
						}
					}
					else
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			#endif

				case VG_PAINT_PATTERN_TILING_MODE:
					if (count == 1) {
						ival = values[0];
						if (ival < VG_TILE_FILL || ival > VG_TILE_REFLECT)
							amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
						else {
							amPaintTilingModeSet((AMPaint *)obj, (VGTilingMode)ival);
							amCtxErrorSet(currentContext, VG_NO_ERROR);
						}
					}
					else
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			}
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case AM_FONT_HANDLE_ID:
			switch (paramType) {
				case VG_FONT_NUM_GLYPHS:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			}
			break;
	#endif

		default:
			amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
			break;
	}
	AM_MEMORY_LOG("vgSetParameteriv");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief Return the value of a parameter on a given VGHandle-based object, scalar float value.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param object object whose parameter is to get.
	\param paramType type of the parameter.
	\return value of the requested parameter.
*/
VG_API_CALL VGfloat VG_API_ENTRY vgGetParameterf(VGHandle object,
                                                 VGint paramType) VG_API_EXIT {

	AMuint32 id;
	VGfloat res;
	AMhandle obj;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgGetParameterf");
		OPENVG_RETURN(0.0f)
	}

	id = amCtxHandleValid(currentContext, object);
	obj = (id != AM_INVALID_HANDLE_ID) ? currentContext->handles->createdHandlesList.data[object] : NULL;

	switch (id) {

		case AM_PATH_HANDLE_ID:
			switch (paramType) {
				case VG_PATH_FORMAT:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGfloat)(((AMPath *)obj)->format);
					break;
				case VG_PATH_DATATYPE:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGfloat)(((AMPath *)obj)->dataType);
					break;
				case VG_PATH_SCALE:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = ((AMPath *)obj)->scale;
					break;
				case VG_PATH_BIAS:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = ((AMPath *)obj)->bias;
					break;
				case VG_PATH_NUM_SEGMENTS:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGfloat)(((AMPath *)obj)->segments.size);
					break;
				case VG_PATH_NUM_COORDS:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGfloat)(amPathCoordinatesCount((AMPath *)obj));
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					res = 0.0f;
					break;
			}
			break;

		case AM_IMAGE_HANDLE_ID:
			switch (paramType) {
				case VG_IMAGE_FORMAT:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGfloat)(((AMImage *)obj)->format);
					break;
				case VG_IMAGE_WIDTH:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGfloat)(((AMImage *)obj)->width);
					break;
				case VG_IMAGE_HEIGHT:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGfloat)(((AMImage *)obj)->height);
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					res = 0.0f;
					break;
			}
			break;

		case AM_PAINT_HANDLE_ID:
			switch (paramType) {
				case VG_PAINT_TYPE:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGfloat)(((AMPaint *)obj)->paintType);
					break;
				case VG_PAINT_COLOR:
				case VG_PAINT_COLOR_RAMP_STOPS:
				case VG_PAINT_LINEAR_GRADIENT:
				case VG_PAINT_RADIAL_GRADIENT:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					res = 0.0f;
					break;
				case VG_PAINT_COLOR_RAMP_SPREAD_MODE:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGfloat)(((AMPaint *)obj)->colorRampSpreadMode);
					break;
				case VG_PAINT_COLOR_RAMP_PREMULTIPLIED:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGfloat)(((AMPaint *)obj)->colorRampPremultiplied);
					break;
			#if defined(VG_MZT_color_ramp_interpolation)
				case VG_PAINT_COLOR_RAMP_INTERPOLATION_TYPE_MZT:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGfloat)(((AMPaint *)obj)->colorRampInterpolationType);
					break;
			#endif

				case VG_PAINT_PATTERN_TILING_MODE:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGfloat)(((AMPaint *)obj)->tilingMode);
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					res = 0.0f;
					break;
			}
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case AM_FONT_HANDLE_ID:
			switch (paramType) {
				case VG_FONT_NUM_GLYPHS:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGfloat)amFontGlyphsCount((const AMFont *)obj);
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					res = 0.0f;
					break;
			}
			break;
	#endif

		default:
			amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
			res = 0.0f;
			break;
	}
	AM_MEMORY_LOG("vgGetParameterf");
	OPENVG_RETURN(res)
}

/*!
	\brief Return the value of a parameter on a given VGHandle-based object, scalar integer value.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param object object whose parameter is to get.
	\param paramType type of the parameter.
	\return value of the requested parameter.
*/
VG_API_CALL VGint VG_API_ENTRY vgGetParameteri(VGHandle object,
                                               VGint paramType) VG_API_EXIT {

	AMuint32 id;
	VGint res;
	AMhandle obj;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgGetParameteri");
		OPENVG_RETURN(0)
	}

	id = amCtxHandleValid(currentContext, object);
	obj = (id != AM_INVALID_HANDLE_ID) ? currentContext->handles->createdHandlesList.data[object] : NULL;

	switch (id) {

		case AM_PATH_HANDLE_ID:
			switch (paramType) {
				case VG_PATH_FORMAT:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGint)(((AMPath *)obj)->format);
					break;
				case VG_PATH_DATATYPE:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGint)(((AMPath *)obj)->dataType);
					break;
				case VG_PATH_SCALE:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGint)amFloorf(((AMPath *)obj)->scale);
					break;
				case VG_PATH_BIAS:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGint)amFloorf(((AMPath *)obj)->bias);
					break;
				case VG_PATH_NUM_SEGMENTS:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGint)(((AMPath *)obj)->segments.size);
					break;
				case VG_PATH_NUM_COORDS:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = amPathCoordinatesCount((AMPath *)obj);
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					res = 0;
					break;
			}
			break;

		case AM_IMAGE_HANDLE_ID:
			switch (paramType) {
				case VG_IMAGE_FORMAT:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGint)(((AMImage *)obj)->format);
					break;
				case VG_IMAGE_WIDTH:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (((AMImage *)obj)->width);
					break;
				case VG_IMAGE_HEIGHT:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (((AMImage *)obj)->height);
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					res = 0;
					break;
			}
			break;

		case AM_PAINT_HANDLE_ID:
			switch (paramType) {
				case VG_PAINT_TYPE:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGint)(((AMPaint *)obj)->paintType);
					break;
				case VG_PAINT_COLOR:
				case VG_PAINT_COLOR_RAMP_STOPS:
				case VG_PAINT_LINEAR_GRADIENT:
				case VG_PAINT_RADIAL_GRADIENT:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					res = 0;
					break;
				case VG_PAINT_COLOR_RAMP_SPREAD_MODE:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGint)(((AMPaint *)obj)->colorRampSpreadMode);
					break;
				case VG_PAINT_COLOR_RAMP_PREMULTIPLIED:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGint)(((AMPaint *)obj)->colorRampPremultiplied);
					break;
			#if defined(VG_MZT_color_ramp_interpolation)
				case VG_PAINT_COLOR_RAMP_INTERPOLATION_TYPE_MZT:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGint)(((AMPaint *)obj)->colorRampInterpolationType);
					break;
			#endif

				case VG_PAINT_PATTERN_TILING_MODE:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGint)(((AMPaint *)obj)->tilingMode);
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					res = 0;
					break;
			}
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case AM_FONT_HANDLE_ID:
			switch (paramType) {
				case VG_FONT_NUM_GLYPHS:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGint)amFontGlyphsCount((const AMFont *)obj);
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					res = 0;
					break;
			}
			break;
	#endif

		default:
			amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
			res = 0;
			break;
	}
	AM_MEMORY_LOG("vgGetParameteri");
	OPENVG_RETURN(res)
}

/*!
	\brief Return the number of elements in the vector that will be returned by the vgGetParameteriv or
	vgGetParameterfv functions if called with the given param type argument.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param object input object.
	\param paramType type of the parameter.
	\return number of elements.
*/
VG_API_CALL VGint VG_API_ENTRY vgGetParameterVectorSize(VGHandle object,
                                                        VGint paramType) VG_API_EXIT {

	AMuint32 id;
	VGint res;
	AMhandle obj;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgGetParameterVectorSize");
		OPENVG_RETURN(0)
	}

	id = amCtxHandleValid(currentContext, object);
	obj = (id != AM_INVALID_HANDLE_ID) ? currentContext->handles->createdHandlesList.data[object] : NULL;

	switch (id) {

		case AM_PATH_HANDLE_ID:
			switch (paramType) {
				case VG_PATH_FORMAT:
				case VG_PATH_DATATYPE:
				case VG_PATH_SCALE:
				case VG_PATH_BIAS:
				case VG_PATH_NUM_SEGMENTS:
				case VG_PATH_NUM_COORDS:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = 1;
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					res = 0;
					break;
			}
			break;

		case AM_IMAGE_HANDLE_ID:
			switch (paramType) {
				case VG_IMAGE_FORMAT:
				case VG_IMAGE_WIDTH:
				case VG_IMAGE_HEIGHT:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = 1;
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					res = 0;
					break;
			}
			break;

		case AM_PAINT_HANDLE_ID:
			switch (paramType) {
				case VG_PAINT_TYPE:
				case VG_PAINT_COLOR_RAMP_SPREAD_MODE:
				case VG_PAINT_COLOR_RAMP_PREMULTIPLIED:
			#if defined(VG_MZT_color_ramp_interpolation)
				case VG_PAINT_COLOR_RAMP_INTERPOLATION_TYPE_MZT:
			#endif
				case VG_PAINT_PATTERN_TILING_MODE:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = 1;
					break;
				case VG_PAINT_COLOR_RAMP_STOPS:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = (VGint)((((AMPaint *)obj)->sColorStops.size) * 5);
					break;
				case VG_PAINT_LINEAR_GRADIENT:
				case VG_PAINT_COLOR:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = 4;
					break;
			#if defined(VG_MZT_conical_gradient)
				case VG_PAINT_CONICAL_GRADIENT_MZT:
			#endif
				case VG_PAINT_RADIAL_GRADIENT:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = 5;
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					res = 0;
					break;
			}
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case AM_FONT_HANDLE_ID:
			switch (paramType) {
				case VG_FONT_NUM_GLYPHS:
					amCtxErrorSet(currentContext, VG_NO_ERROR);
					res = 1;
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					res = 0;
					break;
			}
			break;
	#endif

		default:
			amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
			res = 0;
			break;
	}
	AM_MEMORY_LOG("vgGetParameterVectorSize");
	OPENVG_RETURN(res)
}

/*!
	\brief Return the value of a parameter on a given VGHandle-based object, vector of float values.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param object object whose parameter is to get.
	\param paramType type of the parameter.
	\param count number of elements to get.
	\param values array of output values.
*/
VG_API_CALL void VG_API_ENTRY vgGetParameterfv(VGHandle object,
                                               VGint paramType,
                                               VGint count,
                                               VGfloat *values) VG_API_EXIT {

	AMuint32 id;
	AMhandle obj;
	AMint32 i, j;
	AMfloat linGrad[4], radGrad[5];
#if defined(VG_MZT_conical_gradient)
	AMfloat conGrad[5];
#endif
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgGetParameterfv");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	
	if (!values || count <= 0 || !amPointerIsAligned(values, 4)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgGetParameterfv");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	id = amCtxHandleValid(currentContext, object);
	obj = (id != AM_INVALID_HANDLE_ID) ? currentContext->handles->createdHandlesList.data[object] : NULL;

	switch (id) {

		case AM_PATH_HANDLE_ID:
			switch (paramType) {
				case VG_PATH_FORMAT:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGfloat)(((AMPath *)obj)->format);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				case VG_PATH_DATATYPE:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGfloat)(((AMPath *)obj)->dataType);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				case VG_PATH_SCALE:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = ((AMPath *)obj)->scale;
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				case VG_PATH_BIAS:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = ((AMPath *)obj)->bias;
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				case VG_PATH_NUM_SEGMENTS:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGfloat)(((AMPath *)obj)->segments.size);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				case VG_PATH_NUM_COORDS:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGfloat)(amPathCoordinatesCount((AMPath *)obj));
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			}
			break;

		case AM_IMAGE_HANDLE_ID:
			switch (paramType) {
				case VG_IMAGE_FORMAT:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGfloat)(((AMImage *)obj)->format);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				case VG_IMAGE_WIDTH:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGfloat)(((AMImage *)obj)->width);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				case VG_IMAGE_HEIGHT:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGfloat)(((AMImage *)obj)->height);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			}
			break;

		case AM_PAINT_HANDLE_ID:
			switch (paramType) {
				case VG_PAINT_TYPE:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGfloat)(((AMPaint *)obj)->paintType);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;

				case VG_PAINT_COLOR:
					if (count > 4)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						j = AM_MIN(4, count);
						for (i = 0; i < j; ++i) values[i] = ((AMPaint *)obj)->sColor[i];
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;

				case VG_PAINT_COLOR_RAMP_STOPS:
					j = 5 * (AMint32)((AMPaint *)obj)->sColorStops.size;
					if (count > j)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						j = AM_MIN(j, count);
						for (i = 0; i < j / 5; ++i) {
							values[i * 5 + 0] = ((AMPaint *)obj)->sColorStops.data[i].position;
							values[i * 5 + 1] = ((AMPaint *)obj)->sColorStops.data[i].color[0];
							values[i * 5 + 2] = ((AMPaint *)obj)->sColorStops.data[i].color[1];
							values[i * 5 + 3] = ((AMPaint *)obj)->sColorStops.data[i].color[2];
							values[i * 5 + 4] = ((AMPaint *)obj)->sColorStops.data[i].color[3];
						}
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;

				case VG_PAINT_LINEAR_GRADIENT:
					if (count > 4)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						linGrad[0] = ((AMPaint *)obj)->linGradPt0[0];
						linGrad[1] = ((AMPaint *)obj)->linGradPt0[1];
						linGrad[2] = ((AMPaint *)obj)->linGradPt1[0];
						linGrad[3] = ((AMPaint *)obj)->linGradPt1[1];
						j = AM_MIN(4, count);
						for (i = 0; i < j; ++i) values[i] = linGrad[i];
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;

				case VG_PAINT_RADIAL_GRADIENT:
					if (count > 5)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						radGrad[0] = ((AMPaint *)obj)->radGradCenter[0];
						radGrad[1] = ((AMPaint *)obj)->radGradCenter[1];
						radGrad[2] = ((AMPaint *)obj)->radGradFocus[0];
						radGrad[3] = ((AMPaint *)obj)->radGradFocus[1];
						radGrad[4] = ((AMPaint *)obj)->radGradRadius;
						j = AM_MIN(5, count);
						for (i = 0; i < j; ++i) values[i] = radGrad[i];
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;

			#if defined(VG_MZT_conical_gradient)
				case VG_PAINT_CONICAL_GRADIENT_MZT:
					if (count > 5)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						conGrad[0] = ((AMPaint *)obj)->conGradPt0[0];
						conGrad[1] = ((AMPaint *)obj)->conGradPt0[1];
						conGrad[2] = ((AMPaint *)obj)->conGradPt1[0];
						conGrad[3] = ((AMPaint *)obj)->conGradPt1[1];
						conGrad[4] = ((AMPaint *)obj)->conGradRepeats;
						j = AM_MIN(5, count);
						for (i = 0; i < j; ++i) values[i] = conGrad[i];
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
			#endif

				case VG_PAINT_COLOR_RAMP_SPREAD_MODE:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGfloat)(((AMPaint *)obj)->colorRampSpreadMode);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
		
				case VG_PAINT_COLOR_RAMP_PREMULTIPLIED:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGfloat)(((AMPaint *)obj)->colorRampPremultiplied);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;

			#if defined(VG_MZT_color_ramp_interpolation)
				case VG_PAINT_COLOR_RAMP_INTERPOLATION_TYPE_MZT:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGfloat)(((AMPaint *)obj)->colorRampInterpolationType);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
			#endif

				case VG_PAINT_PATTERN_TILING_MODE:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGfloat)(((AMPaint *)obj)->tilingMode);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			}
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case AM_FONT_HANDLE_ID:
			switch (paramType) {
				case VG_FONT_NUM_GLYPHS:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGfloat)amFontGlyphsCount((const AMFont *)obj);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			}
			break;
	#endif

		default:
			amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
			break;
	}
	AM_MEMORY_LOG("vgGetParameterfv");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief Return the value of a parameter on a given VGHandle-based object, vector of integer values.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param object object whose parameter is to get.
	\param paramType type of the parameter.
	\param count number of elements to get.
	\param values array of output values.
*/
VG_API_CALL void VG_API_ENTRY vgGetParameteriv(VGHandle object,
                                               VGint paramType,
                                               VGint count,
                                               VGint *values) VG_API_EXIT {

	AMuint32 id;
	AMhandle obj;
	AMint32 i, j;
	AMint32 linGrad[4], radGrad[5];
#if defined(VG_MZT_conical_gradient)
	AMint32 conGrad[5];
#endif
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgGetParameteriv");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	if (!values || count <= 0 || !amPointerIsAligned(values, 4)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgGetParameteriv");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	id = amCtxHandleValid(currentContext, object);
	obj = (id != AM_INVALID_HANDLE_ID) ? currentContext->handles->createdHandlesList.data[object] : NULL;

	switch (id) {

		case AM_PATH_HANDLE_ID:
			switch (paramType) {
				case VG_PATH_FORMAT:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGint)(((AMPath *)obj)->format);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				case VG_PATH_DATATYPE:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGint)(((AMPath *)obj)->dataType);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				case VG_PATH_SCALE:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGint)amFloorf(((AMPath *)obj)->scale);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				case VG_PATH_BIAS:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGint)amFloorf(((AMPath *)obj)->bias);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				case VG_PATH_NUM_SEGMENTS:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGint)(((AMPath *)obj)->segments.size);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				case VG_PATH_NUM_COORDS:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = amPathCoordinatesCount((AMPath *)obj);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			}
			break;

		case AM_IMAGE_HANDLE_ID:
			switch (paramType) {
				case VG_IMAGE_FORMAT:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGint)(((AMImage *)obj)->format);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				case VG_IMAGE_WIDTH:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (((AMImage *)obj)->width);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				case VG_IMAGE_HEIGHT:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (((AMImage *)obj)->height);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			}
			break;

		case AM_PAINT_HANDLE_ID:
			switch (paramType) {
				case VG_PAINT_TYPE:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGint)(((AMPaint *)obj)->paintType);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;

				case VG_PAINT_COLOR:
					if (count > 4)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						j = AM_MIN(4, count);
						for (i = 0; i < j; ++i) values[i] = (VGint)amFloorf(((AMPaint *)obj)->sColor[i]);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;

				case VG_PAINT_COLOR_RAMP_STOPS:
					j = 5 * (AMint32)((AMPaint *)obj)->sColorStops.size;
					if (count > j)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						j = AM_MIN(j, count);
						for (i = 0; i < j / 5; ++i) {
							values[i * 5 + 0] = (VGint)amFloorf(((AMPaint *)obj)->sColorStops.data[i].position);
							values[i * 5 + 1] = (VGint)amFloorf(((AMPaint *)obj)->sColorStops.data[i].color[0]);
							values[i * 5 + 2] = (VGint)amFloorf(((AMPaint *)obj)->sColorStops.data[i].color[1]);
							values[i * 5 + 3] = (VGint)amFloorf(((AMPaint *)obj)->sColorStops.data[i].color[2]);
							values[i * 5 + 4] = (VGint)amFloorf(((AMPaint *)obj)->sColorStops.data[i].color[3]);
						}
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;

				case VG_PAINT_LINEAR_GRADIENT:
					if (count > 4)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						linGrad[0] = (AMint32)amFloorf(((AMPaint *)obj)->linGradPt0[0]);
						linGrad[1] = (AMint32)amFloorf(((AMPaint *)obj)->linGradPt0[1]);
						linGrad[2] = (AMint32)amFloorf(((AMPaint *)obj)->linGradPt1[0]);
						linGrad[3] = (AMint32)amFloorf(((AMPaint *)obj)->linGradPt1[1]);
						j = AM_MIN(4, count);
						for (i = 0; i < j; ++i) values[i] = linGrad[i];
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;

				case VG_PAINT_RADIAL_GRADIENT:
					if (count > 5)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						radGrad[0] = (AMint32)amFloorf(((AMPaint *)obj)->radGradCenter[0]);
						radGrad[1] = (AMint32)amFloorf(((AMPaint *)obj)->radGradCenter[1]);
						radGrad[2] = (AMint32)amFloorf(((AMPaint *)obj)->radGradFocus[0]);
						radGrad[3] = (AMint32)amFloorf(((AMPaint *)obj)->radGradFocus[1]);
						radGrad[4] = (AMint32)amFloorf(((AMPaint *)obj)->radGradRadius);
						j = AM_MIN(5, count);
						for (i = 0; i < j; ++i) values[i] = radGrad[i];
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;

			#if defined(VG_MZT_conical_gradient)
				case VG_PAINT_CONICAL_GRADIENT_MZT:
					if (count > 5)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						conGrad[0] = (AMint32)amFloorf(((AMPaint *)obj)->conGradPt0[0]);
						conGrad[1] = (AMint32)amFloorf(((AMPaint *)obj)->conGradPt0[1]);
						conGrad[2] = (AMint32)amFloorf(((AMPaint *)obj)->conGradPt1[0]);
						conGrad[3] = (AMint32)amFloorf(((AMPaint *)obj)->conGradPt1[1]);
						conGrad[4] = (AMint32)amFloorf(((AMPaint *)obj)->conGradRepeats);
						j = AM_MIN(5, count);
						for (i = 0; i < j; ++i) values[i] = conGrad[i];
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
			#endif

				case VG_PAINT_COLOR_RAMP_SPREAD_MODE:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGint)(((AMPaint *)obj)->colorRampSpreadMode);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;

				case VG_PAINT_COLOR_RAMP_PREMULTIPLIED:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGint)(((AMPaint *)obj)->colorRampPremultiplied);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;

			#if defined(VG_MZT_color_ramp_interpolation)
				case VG_PAINT_COLOR_RAMP_INTERPOLATION_TYPE_MZT:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGint)(((AMPaint *)obj)->colorRampInterpolationType);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
			#endif

				case VG_PAINT_PATTERN_TILING_MODE:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGint)(((AMPaint *)obj)->tilingMode);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			}
			break;

	#if (AM_OPENVG_VERSION >= 110)
		case AM_FONT_HANDLE_ID:
			switch (paramType) {
				case VG_FONT_NUM_GLYPHS:
					if (count > 1)
						amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					else {
						values[0] = (VGint)amFontGlyphsCount((const AMFont *)obj);
						amCtxErrorSet(currentContext, VG_NO_ERROR);
					}
					break;
				default:
					amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
					break;
			}
			break;
	#endif

		default:
			amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
			break;
	}
	AM_MEMORY_LOG("vgGetParameteriv");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

#if defined (RIM_VG_SRC)
#pragma pop
#endif

