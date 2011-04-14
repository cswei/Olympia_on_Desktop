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

#ifndef _IMG_FILLERS_PROTOTYPES_H
#define _IMG_FILLERS_PROTOTYPES_H

/*!
	\file img_fillers_prototypes.h
	\brief Image fillers prototypes, header ( \b generated \b file ).
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined(AM_SRE)

#include "amanith_globals.h"

#if !defined(AM_LITE_PROFILE)

void amFilImage_NormalAffineSrc(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_NormalProjectiveSrc(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_MultiplyAffineSrc(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_StencilAffineSrc(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);

// ------------------------------------------------------------------------------------------------------

void amFilImage_NormalAffineSrcOver(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_NormalProjectiveSrcOver(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_MultiplyAffineSrcOver(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_StencilAffineSrcOver(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);

// ------------------------------------------------------------------------------------------------------

void amFilImage_NormalAffineDstOver(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_NormalProjectiveDstOver(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_MultiplyAffineDstOver(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_StencilAffineDstOver(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);

// ------------------------------------------------------------------------------------------------------

void amFilImage_NormalAffineSrcIn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_NormalProjectiveSrcIn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_MultiplyAffineSrcIn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_StencilAffineSrcIn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);

// ------------------------------------------------------------------------------------------------------

void amFilImage_NormalAffineDstIn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_NormalProjectiveDstIn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_MultiplyAffineDstIn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_StencilAffineDstIn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);

// ------------------------------------------------------------------------------------------------------

void amFilImage_NormalAffineMultiply(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_NormalProjectiveMultiply(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_MultiplyAffineMultiply(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_StencilAffineMultiply(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);

// ------------------------------------------------------------------------------------------------------

void amFilImage_NormalAffineScreen(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_NormalProjectiveScreen(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_MultiplyAffineScreen(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_StencilAffineScreen(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);

// ------------------------------------------------------------------------------------------------------

void amFilImage_NormalAffineDarken(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_NormalProjectiveDarken(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_MultiplyAffineDarken(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_StencilAffineDarken(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);

// ------------------------------------------------------------------------------------------------------

void amFilImage_NormalAffineLighten(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_NormalProjectiveLighten(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_MultiplyAffineLighten(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_StencilAffineLighten(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);

// ------------------------------------------------------------------------------------------------------

void amFilImage_NormalAffineAdditive(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_NormalProjectiveAdditive(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_MultiplyAffineAdditive(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_StencilAffineAdditive(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);

// ------------------------------------------------------------------------------------------------------

#if defined(VG_MZT_advanced_blend_modes)
void amFilImage_NormalAffineSrcOut(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_NormalProjectiveSrcOut(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_MultiplyAffineSrcOut(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_StencilAffineSrcOut(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

// ------------------------------------------------------------------------------------------------------

#if defined(VG_MZT_advanced_blend_modes)
void amFilImage_NormalAffineDstOut(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_NormalProjectiveDstOut(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_MultiplyAffineDstOut(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_StencilAffineDstOut(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

// ------------------------------------------------------------------------------------------------------

#if defined(VG_MZT_advanced_blend_modes)
void amFilImage_NormalAffineSrcAtop(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_NormalProjectiveSrcAtop(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_MultiplyAffineSrcAtop(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_StencilAffineSrcAtop(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

// ------------------------------------------------------------------------------------------------------

#if defined(VG_MZT_advanced_blend_modes)
void amFilImage_NormalAffineDstAtop(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_NormalProjectiveDstAtop(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_MultiplyAffineDstAtop(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_StencilAffineDstAtop(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

// ------------------------------------------------------------------------------------------------------

#if defined(VG_MZT_advanced_blend_modes)
void amFilImage_NormalAffineXor(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_NormalProjectiveXor(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_MultiplyAffineXor(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_StencilAffineXor(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

// ------------------------------------------------------------------------------------------------------

#if defined(VG_MZT_advanced_blend_modes)
void amFilImage_NormalAffineOverlay(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_NormalProjectiveOverlay(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_MultiplyAffineOverlay(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_StencilAffineOverlay(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

// ------------------------------------------------------------------------------------------------------

#if defined(VG_MZT_advanced_blend_modes)
void amFilImage_NormalAffineColorDodge(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_NormalProjectiveColorDodge(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_MultiplyAffineColorDodge(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_StencilAffineColorDodge(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

// ------------------------------------------------------------------------------------------------------

#if defined(VG_MZT_advanced_blend_modes)
void amFilImage_NormalAffineColorBurn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_NormalProjectiveColorBurn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_MultiplyAffineColorBurn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_StencilAffineColorBurn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

// ------------------------------------------------------------------------------------------------------

#if defined(VG_MZT_advanced_blend_modes)
void amFilImage_NormalAffineHardLight(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_NormalProjectiveHardLight(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_MultiplyAffineHardLight(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_StencilAffineHardLight(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

// ------------------------------------------------------------------------------------------------------

#if defined(VG_MZT_advanced_blend_modes)
void amFilImage_NormalAffineSoftLight(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_NormalProjectiveSoftLight(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_MultiplyAffineSoftLight(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_StencilAffineSoftLight(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

// ------------------------------------------------------------------------------------------------------

#if defined(VG_MZT_advanced_blend_modes)
void amFilImage_NormalAffineDifference(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_NormalProjectiveDifference(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_MultiplyAffineDifference(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_StencilAffineDifference(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

// ------------------------------------------------------------------------------------------------------

#if defined(VG_MZT_advanced_blend_modes)
void amFilImage_NormalAffineExclusion(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_NormalProjectiveExclusion(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_MultiplyAffineExclusion(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_StencilAffineExclusion(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

// ------------------------------------------------------------------------------------------------------

#else

void amFilImage_NormalAffine(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_NormalProjective(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_Multiply(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilImage_Stencil(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);

#endif // AM_LITE_PROFILE

#endif // AM_SRE
#endif
