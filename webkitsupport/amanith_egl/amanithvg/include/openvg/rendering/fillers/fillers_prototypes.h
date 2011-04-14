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

#ifndef _FILLERS_PROTOTYPES_H
#define _FILLERS_PROTOTYPES_H

/*!
	\file fillers_prototypes.h
	\brief Fillers prototypes, header ( \b generated \b file ).
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined(AM_SRE)

#include "amanith_globals.h"

#if !defined(AM_LITE_PROFILE)

void amFilPath_ColorSrc(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradSrc(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradSrc(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternSrc(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradSrc(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

void amFilPath_ColorSrcMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradSrcMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradSrcMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternSrcMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradSrcMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

// ------------------------------------------------------------------------------------------------------

void amFilPath_ColorSrcOver(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradSrcOver(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradSrcOver(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternSrcOver(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradSrcOver(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

void amFilPath_ColorSrcOverMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradSrcOverMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradSrcOverMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternSrcOverMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradSrcOverMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

// ------------------------------------------------------------------------------------------------------

void amFilPath_ColorDstOver(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradDstOver(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradDstOver(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternDstOver(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradDstOver(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

void amFilPath_ColorDstOverMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradDstOverMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradDstOverMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternDstOverMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradDstOverMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

// ------------------------------------------------------------------------------------------------------

void amFilPath_ColorSrcIn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradSrcIn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradSrcIn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternSrcIn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradSrcIn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

void amFilPath_ColorSrcInMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradSrcInMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradSrcInMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternSrcInMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradSrcInMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

// ------------------------------------------------------------------------------------------------------

void amFilPath_ColorDstIn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradDstIn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradDstIn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternDstIn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradDstIn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

void amFilPath_ColorDstInMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradDstInMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradDstInMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternDstInMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradDstInMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

// ------------------------------------------------------------------------------------------------------

void amFilPath_ColorMultiply(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradMultiply(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradMultiply(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternMultiply(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradMultiply(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

void amFilPath_ColorMultiplyMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradMultiplyMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradMultiplyMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternMultiplyMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradMultiplyMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

// ------------------------------------------------------------------------------------------------------

void amFilPath_ColorScreen(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradScreen(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradScreen(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternScreen(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradScreen(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

void amFilPath_ColorScreenMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradScreenMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradScreenMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternScreenMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradScreenMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

// ------------------------------------------------------------------------------------------------------

void amFilPath_ColorDarken(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradDarken(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradDarken(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternDarken(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradDarken(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

void amFilPath_ColorDarkenMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradDarkenMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradDarkenMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternDarkenMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradDarkenMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

// ------------------------------------------------------------------------------------------------------

void amFilPath_ColorLighten(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradLighten(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradLighten(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternLighten(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradLighten(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

void amFilPath_ColorLightenMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradLightenMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradLightenMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternLightenMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradLightenMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

// ------------------------------------------------------------------------------------------------------

void amFilPath_ColorAdditive(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradAdditive(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradAdditive(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternAdditive(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradAdditive(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

void amFilPath_ColorAdditiveMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradAdditiveMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradAdditiveMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternAdditiveMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradAdditiveMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

// ------------------------------------------------------------------------------------------------------

#if defined(VG_MZT_advanced_blend_modes)
void amFilPath_ColorSrcOut(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradSrcOut(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradSrcOut(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternSrcOut(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradSrcOut(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

void amFilPath_ColorSrcOutMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradSrcOutMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradSrcOutMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternSrcOutMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradSrcOutMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif
#endif

// ------------------------------------------------------------------------------------------------------

#if defined(VG_MZT_advanced_blend_modes)
void amFilPath_ColorDstOut(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradDstOut(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradDstOut(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternDstOut(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradDstOut(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

void amFilPath_ColorDstOutMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradDstOutMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradDstOutMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternDstOutMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradDstOutMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif
#endif

// ------------------------------------------------------------------------------------------------------

#if defined(VG_MZT_advanced_blend_modes)
void amFilPath_ColorSrcAtop(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradSrcAtop(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradSrcAtop(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternSrcAtop(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradSrcAtop(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

void amFilPath_ColorSrcAtopMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradSrcAtopMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradSrcAtopMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternSrcAtopMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradSrcAtopMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif
#endif

// ------------------------------------------------------------------------------------------------------

#if defined(VG_MZT_advanced_blend_modes)
void amFilPath_ColorDstAtop(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradDstAtop(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradDstAtop(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternDstAtop(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradDstAtop(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

void amFilPath_ColorDstAtopMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradDstAtopMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradDstAtopMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternDstAtopMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradDstAtopMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif
#endif

// ------------------------------------------------------------------------------------------------------

#if defined(VG_MZT_advanced_blend_modes)
void amFilPath_ColorXor(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradXor(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradXor(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternXor(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradXor(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

void amFilPath_ColorXorMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradXorMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradXorMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternXorMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradXorMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif
#endif

// ------------------------------------------------------------------------------------------------------

#if defined(VG_MZT_advanced_blend_modes)
void amFilPath_ColorOverlay(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradOverlay(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradOverlay(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternOverlay(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradOverlay(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

void amFilPath_ColorOverlayMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradOverlayMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradOverlayMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternOverlayMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradOverlayMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif
#endif

// ------------------------------------------------------------------------------------------------------

#if defined(VG_MZT_advanced_blend_modes)
void amFilPath_ColorColorDodge(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradColorDodge(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradColorDodge(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternColorDodge(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradColorDodge(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

void amFilPath_ColorColorDodgeMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradColorDodgeMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradColorDodgeMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternColorDodgeMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradColorDodgeMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif
#endif

// ------------------------------------------------------------------------------------------------------

#if defined(VG_MZT_advanced_blend_modes)
void amFilPath_ColorColorBurn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradColorBurn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradColorBurn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternColorBurn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradColorBurn(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

void amFilPath_ColorColorBurnMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradColorBurnMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradColorBurnMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternColorBurnMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradColorBurnMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif
#endif

// ------------------------------------------------------------------------------------------------------

#if defined(VG_MZT_advanced_blend_modes)
void amFilPath_ColorHardLight(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradHardLight(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradHardLight(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternHardLight(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradHardLight(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

void amFilPath_ColorHardLightMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradHardLightMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradHardLightMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternHardLightMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradHardLightMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif
#endif

// ------------------------------------------------------------------------------------------------------

#if defined(VG_MZT_advanced_blend_modes)
void amFilPath_ColorSoftLight(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradSoftLight(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradSoftLight(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternSoftLight(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradSoftLight(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

void amFilPath_ColorSoftLightMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradSoftLightMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradSoftLightMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternSoftLightMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradSoftLightMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif
#endif

// ------------------------------------------------------------------------------------------------------

#if defined(VG_MZT_advanced_blend_modes)
void amFilPath_ColorDifference(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradDifference(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradDifference(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternDifference(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradDifference(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

void amFilPath_ColorDifferenceMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradDifferenceMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradDifferenceMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternDifferenceMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradDifferenceMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif
#endif

// ------------------------------------------------------------------------------------------------------

#if defined(VG_MZT_advanced_blend_modes)
void amFilPath_ColorExclusion(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradExclusion(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradExclusion(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternExclusion(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradExclusion(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif

void amFilPath_ColorExclusionMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_LinGradExclusionMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_RadGradExclusionMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_PatternExclusionMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#if defined(VG_MZT_conical_gradient)
void amFilPath_ConGradExclusionMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
#endif
#endif

// ------------------------------------------------------------------------------------------------------

#else

void amFilPath_ColorSrc(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath_ColorSrcOver(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);
void amFilPath(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1);

#endif // AM_LITE_PROFILE

#endif // AM_SRE
#endif
