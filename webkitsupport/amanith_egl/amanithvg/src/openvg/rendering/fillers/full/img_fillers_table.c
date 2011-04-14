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
	\file img_fillers_table.c
	\brief Image fillers table, implementation ( \b generated \b file ).
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#if defined(AM_SRE) && !defined(AM_LITE_PROFILE)

#include "fillers.h"
#include "img_fillers_prototypes.h"

/*!
	\brief Initialize image fillers table. It fill context->imageFillersFunctions[imageMode][blendMode][projective/affine] function table ( \b generated \b function ).
	\param context context where the table is contained.
*/
void amFilImageTableInit(AMContext *context) {

	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_SRC)][0] = amFilImage_NormalAffineSrc;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_SRC_OVER)][0] = amFilImage_NormalAffineSrcOver;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_DST_OVER)][0] = amFilImage_NormalAffineDstOver;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_SRC_IN)][0] = amFilImage_NormalAffineSrcIn;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_DST_IN)][0] = amFilImage_NormalAffineDstIn;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_MULTIPLY)][0] = amFilImage_NormalAffineMultiply;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_SCREEN)][0] = amFilImage_NormalAffineScreen;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_DARKEN)][0] = amFilImage_NormalAffineDarken;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_LIGHTEN)][0] = amFilImage_NormalAffineLighten;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_ADDITIVE)][0] = amFilImage_NormalAffineAdditive;
#if defined(VG_MZT_advanced_blend_modes)
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_CLEAR_MZT)][0] = NULL;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_DST_MZT)][0] = NULL;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_SRC_OUT_MZT)][0] = amFilImage_NormalAffineSrcOut;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_DST_OUT_MZT)][0] = amFilImage_NormalAffineDstOut;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_SRC_ATOP_MZT)][0] = amFilImage_NormalAffineSrcAtop;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_DST_ATOP_MZT)][0] = amFilImage_NormalAffineDstAtop;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_XOR_MZT)][0] = amFilImage_NormalAffineXor;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_OVERLAY_MZT)][0] = amFilImage_NormalAffineOverlay;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_COLOR_DODGE_MZT)][0] = amFilImage_NormalAffineColorDodge;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_COLOR_BURN_MZT)][0] = amFilImage_NormalAffineColorBurn;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_HARD_LIGHT_MZT)][0] = amFilImage_NormalAffineHardLight;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_SOFT_LIGHT_MZT)][0] = amFilImage_NormalAffineSoftLight;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_DIFFERENCE_MZT)][0] = amFilImage_NormalAffineDifference;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_EXCLUSION_MZT)][0] = amFilImage_NormalAffineExclusion;
#endif

	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_SRC)][0] = amFilImage_MultiplyAffineSrc;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_SRC_OVER)][0] = amFilImage_MultiplyAffineSrcOver;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_DST_OVER)][0] = amFilImage_MultiplyAffineDstOver;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_SRC_IN)][0] = amFilImage_MultiplyAffineSrcIn;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_DST_IN)][0] = amFilImage_MultiplyAffineDstIn;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_MULTIPLY)][0] = amFilImage_MultiplyAffineMultiply;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_SCREEN)][0] = amFilImage_MultiplyAffineScreen;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_DARKEN)][0] = amFilImage_MultiplyAffineDarken;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_LIGHTEN)][0] = amFilImage_MultiplyAffineLighten;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_ADDITIVE)][0] = amFilImage_MultiplyAffineAdditive;
#if defined(VG_MZT_advanced_blend_modes)
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_CLEAR_MZT)][0] = NULL;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_DST_MZT)][0] = NULL;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_SRC_OUT_MZT)][0] = amFilImage_MultiplyAffineSrcOut;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_DST_OUT_MZT)][0] = amFilImage_MultiplyAffineDstOut;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_SRC_ATOP_MZT)][0] = amFilImage_MultiplyAffineSrcAtop;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_DST_ATOP_MZT)][0] = amFilImage_MultiplyAffineDstAtop;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_XOR_MZT)][0] = amFilImage_MultiplyAffineXor;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_OVERLAY_MZT)][0] = amFilImage_MultiplyAffineOverlay;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_COLOR_DODGE_MZT)][0] = amFilImage_MultiplyAffineColorDodge;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_COLOR_BURN_MZT)][0] = amFilImage_MultiplyAffineColorBurn;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_HARD_LIGHT_MZT)][0] = amFilImage_MultiplyAffineHardLight;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_SOFT_LIGHT_MZT)][0] = amFilImage_MultiplyAffineSoftLight;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_DIFFERENCE_MZT)][0] = amFilImage_MultiplyAffineDifference;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_EXCLUSION_MZT)][0] = amFilImage_MultiplyAffineExclusion;
#endif

	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_SRC)][0] = amFilImage_StencilAffineSrc;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_SRC_OVER)][0] = amFilImage_StencilAffineSrcOver;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_DST_OVER)][0] = amFilImage_StencilAffineDstOver;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_SRC_IN)][0] = amFilImage_StencilAffineSrcIn;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_DST_IN)][0] = amFilImage_StencilAffineDstIn;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_MULTIPLY)][0] = amFilImage_StencilAffineMultiply;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_SCREEN)][0] = amFilImage_StencilAffineScreen;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_DARKEN)][0] = amFilImage_StencilAffineDarken;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_LIGHTEN)][0] = amFilImage_StencilAffineLighten;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_ADDITIVE)][0] = amFilImage_StencilAffineAdditive;
#if defined(VG_MZT_advanced_blend_modes)
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_CLEAR_MZT)][0] = NULL;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_DST_MZT)][0] = NULL;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_SRC_OUT_MZT)][0] = amFilImage_StencilAffineSrcOut;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_DST_OUT_MZT)][0] = amFilImage_StencilAffineDstOut;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_SRC_ATOP_MZT)][0] = amFilImage_StencilAffineSrcAtop;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_DST_ATOP_MZT)][0] = amFilImage_StencilAffineDstAtop;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_XOR_MZT)][0] = amFilImage_StencilAffineXor;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_OVERLAY_MZT)][0] = amFilImage_StencilAffineOverlay;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_COLOR_DODGE_MZT)][0] = amFilImage_StencilAffineColorDodge;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_COLOR_BURN_MZT)][0] = amFilImage_StencilAffineColorBurn;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_HARD_LIGHT_MZT)][0] = amFilImage_StencilAffineHardLight;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_SOFT_LIGHT_MZT)][0] = amFilImage_StencilAffineSoftLight;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_DIFFERENCE_MZT)][0] = amFilImage_StencilAffineDifference;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_EXCLUSION_MZT)][0] = amFilImage_StencilAffineExclusion;
#endif

	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_SRC)][1] = amFilImage_NormalProjectiveSrc;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_SRC_OVER)][1] = amFilImage_NormalProjectiveSrcOver;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_DST_OVER)][1] = amFilImage_NormalProjectiveDstOver;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_SRC_IN)][1] = amFilImage_NormalProjectiveSrcIn;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_DST_IN)][1] = amFilImage_NormalProjectiveDstIn;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_MULTIPLY)][1] = amFilImage_NormalProjectiveMultiply;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_SCREEN)][1] = amFilImage_NormalProjectiveScreen;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_DARKEN)][1] = amFilImage_NormalProjectiveDarken;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_LIGHTEN)][1] = amFilImage_NormalProjectiveLighten;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_ADDITIVE)][1] = amFilImage_NormalProjectiveAdditive;
#if defined(VG_MZT_advanced_blend_modes)
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_CLEAR_MZT)][1] = NULL;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_DST_MZT)][1] = NULL;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_SRC_OUT_MZT)][1] = amFilImage_NormalProjectiveSrcOut;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_DST_OUT_MZT)][1] = amFilImage_NormalProjectiveDstOut;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_SRC_ATOP_MZT)][1] = amFilImage_NormalProjectiveSrcAtop;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_DST_ATOP_MZT)][1] = amFilImage_NormalProjectiveDstAtop;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_XOR_MZT)][1] = amFilImage_NormalProjectiveXor;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_OVERLAY_MZT)][1] = amFilImage_NormalProjectiveOverlay;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_COLOR_DODGE_MZT)][1] = amFilImage_NormalProjectiveColorDodge;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_COLOR_BURN_MZT)][1] = amFilImage_NormalProjectiveColorBurn;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_HARD_LIGHT_MZT)][1] = amFilImage_NormalProjectiveHardLight;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_SOFT_LIGHT_MZT)][1] = amFilImage_NormalProjectiveSoftLight;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_DIFFERENCE_MZT)][1] = amFilImage_NormalProjectiveDifference;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_NORMAL)][amBlendModeGetIndex(VG_BLEND_EXCLUSION_MZT)][1] = amFilImage_NormalProjectiveExclusion;
#endif

	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_SRC)][1] = amFilImage_MultiplyAffineSrc;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_SRC_OVER)][1] = amFilImage_MultiplyAffineSrcOver;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_DST_OVER)][1] = amFilImage_MultiplyAffineDstOver;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_SRC_IN)][1] = amFilImage_MultiplyAffineSrcIn;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_DST_IN)][1] = amFilImage_MultiplyAffineDstIn;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_MULTIPLY)][1] = amFilImage_MultiplyAffineMultiply;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_SCREEN)][1] = amFilImage_MultiplyAffineScreen;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_DARKEN)][1] = amFilImage_MultiplyAffineDarken;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_LIGHTEN)][1] = amFilImage_MultiplyAffineLighten;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_ADDITIVE)][1] = amFilImage_MultiplyAffineAdditive;
#if defined(VG_MZT_advanced_blend_modes)
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_CLEAR_MZT)][1] = NULL;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_DST_MZT)][1] = NULL;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_SRC_OUT_MZT)][1] = amFilImage_MultiplyAffineSrcOut;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_DST_OUT_MZT)][1] = amFilImage_MultiplyAffineDstOut;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_SRC_ATOP_MZT)][1] = amFilImage_MultiplyAffineSrcAtop;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_DST_ATOP_MZT)][1] = amFilImage_MultiplyAffineDstAtop;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_XOR_MZT)][1] = amFilImage_MultiplyAffineXor;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_OVERLAY_MZT)][1] = amFilImage_MultiplyAffineOverlay;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_COLOR_DODGE_MZT)][1] = amFilImage_MultiplyAffineColorDodge;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_COLOR_BURN_MZT)][1] = amFilImage_MultiplyAffineColorBurn;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_HARD_LIGHT_MZT)][1] = amFilImage_MultiplyAffineHardLight;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_SOFT_LIGHT_MZT)][1] = amFilImage_MultiplyAffineSoftLight;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_DIFFERENCE_MZT)][1] = amFilImage_MultiplyAffineDifference;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_MULTIPLY)][amBlendModeGetIndex(VG_BLEND_EXCLUSION_MZT)][1] = amFilImage_MultiplyAffineExclusion;
#endif

	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_SRC)][1] = amFilImage_StencilAffineSrc;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_SRC_OVER)][1] = amFilImage_StencilAffineSrcOver;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_DST_OVER)][1] = amFilImage_StencilAffineDstOver;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_SRC_IN)][1] = amFilImage_StencilAffineSrcIn;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_DST_IN)][1] = amFilImage_StencilAffineDstIn;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_MULTIPLY)][1] = amFilImage_StencilAffineMultiply;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_SCREEN)][1] = amFilImage_StencilAffineScreen;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_DARKEN)][1] = amFilImage_StencilAffineDarken;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_LIGHTEN)][1] = amFilImage_StencilAffineLighten;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_ADDITIVE)][1] = amFilImage_StencilAffineAdditive;
#if defined(VG_MZT_advanced_blend_modes)
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_CLEAR_MZT)][1] = NULL;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_DST_MZT)][1] = NULL;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_SRC_OUT_MZT)][1] = amFilImage_StencilAffineSrcOut;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_DST_OUT_MZT)][1] = amFilImage_StencilAffineDstOut;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_SRC_ATOP_MZT)][1] = amFilImage_StencilAffineSrcAtop;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_DST_ATOP_MZT)][1] = amFilImage_StencilAffineDstAtop;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_XOR_MZT)][1] = amFilImage_StencilAffineXor;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_OVERLAY_MZT)][1] = amFilImage_StencilAffineOverlay;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_COLOR_DODGE_MZT)][1] = amFilImage_StencilAffineColorDodge;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_COLOR_BURN_MZT)][1] = amFilImage_StencilAffineColorBurn;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_HARD_LIGHT_MZT)][1] = amFilImage_StencilAffineHardLight;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_SOFT_LIGHT_MZT)][1] = amFilImage_StencilAffineSoftLight;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_DIFFERENCE_MZT)][1] = amFilImage_StencilAffineDifference;
	context->imageFillersFunctions[amImageModeGetIndex(VG_DRAW_IMAGE_STENCIL)][amBlendModeGetIndex(VG_BLEND_EXCLUSION_MZT)][1] = amFilImage_StencilAffineExclusion;
#endif

}
#endif

#if defined (RIM_VG_SRC)
#pragma pop
#endif

