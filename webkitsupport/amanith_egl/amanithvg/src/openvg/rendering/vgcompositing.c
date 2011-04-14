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
	\file vgcompositing.c
	\brief Compositing utilities, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "vgcompositing.h"
#if defined(AM_SRE)
	#include "srecompositing.h"
#elif defined(AM_GLE)
	#include "glecompositing.h"
#elif defined(AM_GLS)
	#include "glscompositing.h"
#else
	#error Unreachable point.
#endif

/*!
	\brief Patch the blend mode of the given paint descriptor, choosing a more convenient (faster) blend mode when possible.
	\param paintDesc paint descriptor whose blend mode is to patch.
	\param context input context containing color transform values.
	\note used to draw paths.
*/
void amBlendModePathPatch(AMPaintDesc *paintDesc,
						  const AMContext *context) {

#if defined(AM_SRE)
	amSrePathBlendModePatch(paintDesc, context);
#elif defined(AM_GLE)
	amGlePathBlendModePatch(paintDesc, context);
#elif defined(AM_GLS)
	amGlsPathBlendModePatch(paintDesc, context);
#else
	#error Unreachable point.
#endif
}

/*!
	\brief Patch the blend mode of the given paint descriptor, choosing a more convenient (faster) blend mode when possible.
	\param paintDesc paint descriptor whose blend mode is to patch.
	\param context input context containing color transform values.
	\note used to draw images.
*/
void amBlendModeImagePatch(AMPaintDesc *paintDesc,
						   const AMContext *context) {
#if defined(AM_SRE)
	amSreImageBlendModePatch(paintDesc, context);
#elif defined(AM_GLE)
	amGleImageBlendModePatch(paintDesc, context);
#elif defined(AM_GLS)
	amGlsImageBlendModePatch(paintDesc, context);
#else
	#error Unreachable point.
#endif
}

#if defined (RIM_VG_SRC)
#pragma pop
#endif

