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
	\file fillers_table.c
	\brief Path fillers table, implementation ( \b generated \b file ).
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#if defined(AM_SRE) && !defined(AM_LITE_PROFILE)

#include "fillers.h"
#include "fillers_prototypes.h"

/*!
	\brief Initialize path fillers table. It fill context->pathFillersFunctions[paintType][blendMode][masking] function table ( \b generated \b function ).
	\param context context where the table is contained.
*/
void amFilPathTableInit(AMContext *context) {

	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_SRC)][0] = amFilPath_ColorSrc;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_SRC_OVER)][0] = amFilPath_ColorSrcOver;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_DST_OVER)][0] = amFilPath_ColorDstOver;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_SRC_IN)][0] = amFilPath_ColorSrcIn;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_DST_IN)][0] = amFilPath_ColorDstIn;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_MULTIPLY)][0] = amFilPath_ColorMultiply;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_SCREEN)][0] = amFilPath_ColorScreen;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_DARKEN)][0] = amFilPath_ColorDarken;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_LIGHTEN)][0] = amFilPath_ColorLighten;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_ADDITIVE)][0] = amFilPath_ColorAdditive;
#if defined(VG_MZT_advanced_blend_modes)
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_CLEAR_MZT)][0] = NULL;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_DST_MZT)][0] = NULL;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_SRC_OUT_MZT)][0] = amFilPath_ColorSrcOut;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_DST_OUT_MZT)][0] = amFilPath_ColorDstOut;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_SRC_ATOP_MZT)][0] = amFilPath_ColorSrcAtop;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_DST_ATOP_MZT)][0] = amFilPath_ColorDstAtop;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_XOR_MZT)][0] = amFilPath_ColorXor;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_OVERLAY_MZT)][0] = amFilPath_ColorOverlay;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_COLOR_DODGE_MZT)][0] = amFilPath_ColorColorDodge;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_COLOR_BURN_MZT)][0] = amFilPath_ColorColorBurn;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_HARD_LIGHT_MZT)][0] = amFilPath_ColorHardLight;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_SOFT_LIGHT_MZT)][0] = amFilPath_ColorSoftLight;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_DIFFERENCE_MZT)][0] = amFilPath_ColorDifference;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_EXCLUSION_MZT)][0] = amFilPath_ColorExclusion;
#endif

	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SRC)][0] = amFilPath_LinGradSrc;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SRC_OVER)][0] = amFilPath_LinGradSrcOver;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DST_OVER)][0] = amFilPath_LinGradDstOver;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SRC_IN)][0] = amFilPath_LinGradSrcIn;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DST_IN)][0] = amFilPath_LinGradDstIn;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_MULTIPLY)][0] = amFilPath_LinGradMultiply;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SCREEN)][0] = amFilPath_LinGradScreen;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DARKEN)][0] = amFilPath_LinGradDarken;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_LIGHTEN)][0] = amFilPath_LinGradLighten;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_ADDITIVE)][0] = amFilPath_LinGradAdditive;
#if defined(VG_MZT_advanced_blend_modes)
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_CLEAR_MZT)][0] = NULL;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DST_MZT)][0] = NULL;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SRC_OUT_MZT)][0] = amFilPath_LinGradSrcOut;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DST_OUT_MZT)][0] = amFilPath_LinGradDstOut;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SRC_ATOP_MZT)][0] = amFilPath_LinGradSrcAtop;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DST_ATOP_MZT)][0] = amFilPath_LinGradDstAtop;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_XOR_MZT)][0] = amFilPath_LinGradXor;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_OVERLAY_MZT)][0] = amFilPath_LinGradOverlay;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_COLOR_DODGE_MZT)][0] = amFilPath_LinGradColorDodge;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_COLOR_BURN_MZT)][0] = amFilPath_LinGradColorBurn;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_HARD_LIGHT_MZT)][0] = amFilPath_LinGradHardLight;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SOFT_LIGHT_MZT)][0] = amFilPath_LinGradSoftLight;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DIFFERENCE_MZT)][0] = amFilPath_LinGradDifference;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_EXCLUSION_MZT)][0] = amFilPath_LinGradExclusion;
#endif

	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SRC)][0] = amFilPath_RadGradSrc;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SRC_OVER)][0] = amFilPath_RadGradSrcOver;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DST_OVER)][0] = amFilPath_RadGradDstOver;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SRC_IN)][0] = amFilPath_RadGradSrcIn;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DST_IN)][0] = amFilPath_RadGradDstIn;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_MULTIPLY)][0] = amFilPath_RadGradMultiply;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SCREEN)][0] = amFilPath_RadGradScreen;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DARKEN)][0] = amFilPath_RadGradDarken;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_LIGHTEN)][0] = amFilPath_RadGradLighten;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_ADDITIVE)][0] = amFilPath_RadGradAdditive;
#if defined(VG_MZT_advanced_blend_modes)
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_CLEAR_MZT)][0] = NULL;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DST_MZT)][0] = NULL;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SRC_OUT_MZT)][0] = amFilPath_RadGradSrcOut;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DST_OUT_MZT)][0] = amFilPath_RadGradDstOut;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SRC_ATOP_MZT)][0] = amFilPath_RadGradSrcAtop;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DST_ATOP_MZT)][0] = amFilPath_RadGradDstAtop;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_XOR_MZT)][0] = amFilPath_RadGradXor;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_OVERLAY_MZT)][0] = amFilPath_RadGradOverlay;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_COLOR_DODGE_MZT)][0] = amFilPath_RadGradColorDodge;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_COLOR_BURN_MZT)][0] = amFilPath_RadGradColorBurn;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_HARD_LIGHT_MZT)][0] = amFilPath_RadGradHardLight;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SOFT_LIGHT_MZT)][0] = amFilPath_RadGradSoftLight;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DIFFERENCE_MZT)][0] = amFilPath_RadGradDifference;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_EXCLUSION_MZT)][0] = amFilPath_RadGradExclusion;
#endif

	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_SRC)][0] = amFilPath_PatternSrc;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_SRC_OVER)][0] = amFilPath_PatternSrcOver;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_DST_OVER)][0] = amFilPath_PatternDstOver;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_SRC_IN)][0] = amFilPath_PatternSrcIn;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_DST_IN)][0] = amFilPath_PatternDstIn;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_MULTIPLY)][0] = amFilPath_PatternMultiply;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_SCREEN)][0] = amFilPath_PatternScreen;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_DARKEN)][0] = amFilPath_PatternDarken;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_LIGHTEN)][0] = amFilPath_PatternLighten;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_ADDITIVE)][0] = amFilPath_PatternAdditive;
#if defined(VG_MZT_advanced_blend_modes)
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_CLEAR_MZT)][0] = NULL;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_DST_MZT)][0] = NULL;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_SRC_OUT_MZT)][0] = amFilPath_PatternSrcOut;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_DST_OUT_MZT)][0] = amFilPath_PatternDstOut;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_SRC_ATOP_MZT)][0] = amFilPath_PatternSrcAtop;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_DST_ATOP_MZT)][0] = amFilPath_PatternDstAtop;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_XOR_MZT)][0] = amFilPath_PatternXor;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_OVERLAY_MZT)][0] = amFilPath_PatternOverlay;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_COLOR_DODGE_MZT)][0] = amFilPath_PatternColorDodge;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_COLOR_BURN_MZT)][0] = amFilPath_PatternColorBurn;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_HARD_LIGHT_MZT)][0] = amFilPath_PatternHardLight;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_SOFT_LIGHT_MZT)][0] = amFilPath_PatternSoftLight;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_DIFFERENCE_MZT)][0] = amFilPath_PatternDifference;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_EXCLUSION_MZT)][0] = amFilPath_PatternExclusion;
#endif

#if defined(VG_MZT_conical_gradient)
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_SRC)][0] = amFilPath_ConGradSrc;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_SRC_OVER)][0] = amFilPath_ConGradSrcOver;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_DST_OVER)][0] = amFilPath_ConGradDstOver;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_SRC_IN)][0] = amFilPath_ConGradSrcIn;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_DST_IN)][0] = amFilPath_ConGradDstIn;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_MULTIPLY)][0] = amFilPath_ConGradMultiply;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_SCREEN)][0] = amFilPath_ConGradScreen;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_DARKEN)][0] = amFilPath_ConGradDarken;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_LIGHTEN)][0] = amFilPath_ConGradLighten;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_ADDITIVE)][0] = amFilPath_ConGradAdditive;
#if defined(VG_MZT_advanced_blend_modes)
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_CLEAR_MZT)][0] = NULL;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_DST_MZT)][0] = NULL;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_SRC_OUT_MZT)][0] = amFilPath_ConGradSrcOut;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_DST_OUT_MZT)][0] = amFilPath_ConGradDstOut;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_SRC_ATOP_MZT)][0] = amFilPath_ConGradSrcAtop;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_DST_ATOP_MZT)][0] = amFilPath_ConGradDstAtop;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_XOR_MZT)][0] = amFilPath_ConGradXor;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_OVERLAY_MZT)][0] = amFilPath_ConGradOverlay;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_COLOR_DODGE_MZT)][0] = amFilPath_ConGradColorDodge;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_COLOR_BURN_MZT)][0] = amFilPath_ConGradColorBurn;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_HARD_LIGHT_MZT)][0] = amFilPath_ConGradHardLight;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_SOFT_LIGHT_MZT)][0] = amFilPath_ConGradSoftLight;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_DIFFERENCE_MZT)][0] = amFilPath_ConGradDifference;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_EXCLUSION_MZT)][0] = amFilPath_ConGradExclusion;
#endif
#endif

	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_SRC)][1] = amFilPath_ColorSrcMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_SRC_OVER)][1] = amFilPath_ColorSrcOverMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_DST_OVER)][1] = amFilPath_ColorDstOverMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_SRC_IN)][1] = amFilPath_ColorSrcInMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_DST_IN)][1] = amFilPath_ColorDstInMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_MULTIPLY)][1] = amFilPath_ColorMultiplyMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_SCREEN)][1] = amFilPath_ColorScreenMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_DARKEN)][1] = amFilPath_ColorDarkenMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_LIGHTEN)][1] = amFilPath_ColorLightenMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_ADDITIVE)][1] = amFilPath_ColorAdditiveMask;
#if defined(VG_MZT_advanced_blend_modes)
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_CLEAR_MZT)][1] = NULL;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_DST_MZT)][1] = NULL;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_SRC_OUT_MZT)][1] = amFilPath_ColorSrcOutMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_DST_OUT_MZT)][1] = amFilPath_ColorDstOutMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_SRC_ATOP_MZT)][1] = amFilPath_ColorSrcAtopMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_DST_ATOP_MZT)][1] = amFilPath_ColorDstAtopMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_XOR_MZT)][1] = amFilPath_ColorXorMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_OVERLAY_MZT)][1] = amFilPath_ColorOverlayMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_COLOR_DODGE_MZT)][1] = amFilPath_ColorColorDodgeMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_COLOR_BURN_MZT)][1] = amFilPath_ColorColorBurnMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_HARD_LIGHT_MZT)][1] = amFilPath_ColorHardLightMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_SOFT_LIGHT_MZT)][1] = amFilPath_ColorSoftLightMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_DIFFERENCE_MZT)][1] = amFilPath_ColorDifferenceMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_COLOR)][amBlendModeGetIndex(VG_BLEND_EXCLUSION_MZT)][1] = amFilPath_ColorExclusionMask;
#endif

	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SRC)][1] = amFilPath_LinGradSrcMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SRC_OVER)][1] = amFilPath_LinGradSrcOverMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DST_OVER)][1] = amFilPath_LinGradDstOverMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SRC_IN)][1] = amFilPath_LinGradSrcInMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DST_IN)][1] = amFilPath_LinGradDstInMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_MULTIPLY)][1] = amFilPath_LinGradMultiplyMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SCREEN)][1] = amFilPath_LinGradScreenMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DARKEN)][1] = amFilPath_LinGradDarkenMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_LIGHTEN)][1] = amFilPath_LinGradLightenMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_ADDITIVE)][1] = amFilPath_LinGradAdditiveMask;
#if defined(VG_MZT_advanced_blend_modes)
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_CLEAR_MZT)][1] = NULL;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DST_MZT)][1] = NULL;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SRC_OUT_MZT)][1] = amFilPath_LinGradSrcOutMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DST_OUT_MZT)][1] = amFilPath_LinGradDstOutMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SRC_ATOP_MZT)][1] = amFilPath_LinGradSrcAtopMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DST_ATOP_MZT)][1] = amFilPath_LinGradDstAtopMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_XOR_MZT)][1] = amFilPath_LinGradXorMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_OVERLAY_MZT)][1] = amFilPath_LinGradOverlayMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_COLOR_DODGE_MZT)][1] = amFilPath_LinGradColorDodgeMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_COLOR_BURN_MZT)][1] = amFilPath_LinGradColorBurnMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_HARD_LIGHT_MZT)][1] = amFilPath_LinGradHardLightMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SOFT_LIGHT_MZT)][1] = amFilPath_LinGradSoftLightMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DIFFERENCE_MZT)][1] = amFilPath_LinGradDifferenceMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_LINEAR_GRADIENT)][amBlendModeGetIndex(VG_BLEND_EXCLUSION_MZT)][1] = amFilPath_LinGradExclusionMask;
#endif

	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SRC)][1] = amFilPath_RadGradSrcMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SRC_OVER)][1] = amFilPath_RadGradSrcOverMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DST_OVER)][1] = amFilPath_RadGradDstOverMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SRC_IN)][1] = amFilPath_RadGradSrcInMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DST_IN)][1] = amFilPath_RadGradDstInMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_MULTIPLY)][1] = amFilPath_RadGradMultiplyMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SCREEN)][1] = amFilPath_RadGradScreenMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DARKEN)][1] = amFilPath_RadGradDarkenMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_LIGHTEN)][1] = amFilPath_RadGradLightenMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_ADDITIVE)][1] = amFilPath_RadGradAdditiveMask;
#if defined(VG_MZT_advanced_blend_modes)
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_CLEAR_MZT)][1] = NULL;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DST_MZT)][1] = NULL;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SRC_OUT_MZT)][1] = amFilPath_RadGradSrcOutMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DST_OUT_MZT)][1] = amFilPath_RadGradDstOutMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SRC_ATOP_MZT)][1] = amFilPath_RadGradSrcAtopMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DST_ATOP_MZT)][1] = amFilPath_RadGradDstAtopMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_XOR_MZT)][1] = amFilPath_RadGradXorMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_OVERLAY_MZT)][1] = amFilPath_RadGradOverlayMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_COLOR_DODGE_MZT)][1] = amFilPath_RadGradColorDodgeMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_COLOR_BURN_MZT)][1] = amFilPath_RadGradColorBurnMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_HARD_LIGHT_MZT)][1] = amFilPath_RadGradHardLightMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_SOFT_LIGHT_MZT)][1] = amFilPath_RadGradSoftLightMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_DIFFERENCE_MZT)][1] = amFilPath_RadGradDifferenceMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_RADIAL_GRADIENT)][amBlendModeGetIndex(VG_BLEND_EXCLUSION_MZT)][1] = amFilPath_RadGradExclusionMask;
#endif

	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_SRC)][1] = amFilPath_PatternSrcMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_SRC_OVER)][1] = amFilPath_PatternSrcOverMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_DST_OVER)][1] = amFilPath_PatternDstOverMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_SRC_IN)][1] = amFilPath_PatternSrcInMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_DST_IN)][1] = amFilPath_PatternDstInMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_MULTIPLY)][1] = amFilPath_PatternMultiplyMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_SCREEN)][1] = amFilPath_PatternScreenMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_DARKEN)][1] = amFilPath_PatternDarkenMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_LIGHTEN)][1] = amFilPath_PatternLightenMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_ADDITIVE)][1] = amFilPath_PatternAdditiveMask;
#if defined(VG_MZT_advanced_blend_modes)
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_CLEAR_MZT)][1] = NULL;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_DST_MZT)][1] = NULL;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_SRC_OUT_MZT)][1] = amFilPath_PatternSrcOutMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_DST_OUT_MZT)][1] = amFilPath_PatternDstOutMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_SRC_ATOP_MZT)][1] = amFilPath_PatternSrcAtopMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_DST_ATOP_MZT)][1] = amFilPath_PatternDstAtopMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_XOR_MZT)][1] = amFilPath_PatternXorMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_OVERLAY_MZT)][1] = amFilPath_PatternOverlayMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_COLOR_DODGE_MZT)][1] = amFilPath_PatternColorDodgeMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_COLOR_BURN_MZT)][1] = amFilPath_PatternColorBurnMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_HARD_LIGHT_MZT)][1] = amFilPath_PatternHardLightMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_SOFT_LIGHT_MZT)][1] = amFilPath_PatternSoftLightMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_DIFFERENCE_MZT)][1] = amFilPath_PatternDifferenceMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_PATTERN)][amBlendModeGetIndex(VG_BLEND_EXCLUSION_MZT)][1] = amFilPath_PatternExclusionMask;
#endif

#if defined(VG_MZT_conical_gradient)
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_SRC)][1] = amFilPath_ConGradSrcMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_SRC_OVER)][1] = amFilPath_ConGradSrcOverMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_DST_OVER)][1] = amFilPath_ConGradDstOverMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_SRC_IN)][1] = amFilPath_ConGradSrcInMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_DST_IN)][1] = amFilPath_ConGradDstInMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_MULTIPLY)][1] = amFilPath_ConGradMultiplyMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_SCREEN)][1] = amFilPath_ConGradScreenMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_DARKEN)][1] = amFilPath_ConGradDarkenMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_LIGHTEN)][1] = amFilPath_ConGradLightenMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_ADDITIVE)][1] = amFilPath_ConGradAdditiveMask;
#if defined(VG_MZT_advanced_blend_modes)
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_CLEAR_MZT)][1] = NULL;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_DST_MZT)][1] = NULL;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_SRC_OUT_MZT)][1] = amFilPath_ConGradSrcOutMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_DST_OUT_MZT)][1] = amFilPath_ConGradDstOutMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_SRC_ATOP_MZT)][1] = amFilPath_ConGradSrcAtopMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_DST_ATOP_MZT)][1] = amFilPath_ConGradDstAtopMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_XOR_MZT)][1] = amFilPath_ConGradXorMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_OVERLAY_MZT)][1] = amFilPath_ConGradOverlayMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_COLOR_DODGE_MZT)][1] = amFilPath_ConGradColorDodgeMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_COLOR_BURN_MZT)][1] = amFilPath_ConGradColorBurnMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_HARD_LIGHT_MZT)][1] = amFilPath_ConGradHardLightMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_SOFT_LIGHT_MZT)][1] = amFilPath_ConGradSoftLightMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_DIFFERENCE_MZT)][1] = amFilPath_ConGradDifferenceMask;
	context->pathFillersFunctions[amPaintTypeGetIndex(VG_PAINT_TYPE_CONICAL_GRADIENT_MZT)][amBlendModeGetIndex(VG_BLEND_EXCLUSION_MZT)][1] = amFilPath_ConGradExclusionMask;
#endif
#endif

}
#endif

#if defined (RIM_VG_SRC)
#pragma pop
#endif

