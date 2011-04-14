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

#ifndef _VGSTROKE_H
#define _VGSTROKE_H

/*!
	\file vgstroke.h
	\brief Stroking functions, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "dynarray.h"

//! Structure that describes "normalized" (phase = 0) dash pattern.
typedef struct _AMDashDesc {
	//! First "normalized" dash value.
	AMfloat firstDashValue;
	//! After normalization, the index of the first dash value.
	AMuint32 firstDashIndex;
	//! AM_TRUE if the first value represents an empty dash trait, else AM_FALSE.
	AMbool firstEmpty;
} AMDashDesc;

/*!
	\brief Stroke cache descriptor; it's used to invalidate previously created stroke polygons, in the case
	that one or more of the following parameters changed.
*/
typedef struct _AMStrokeCacheDesc {
#if defined(VG_MZT_separable_cap_style)
	//! Start cap style.
	VGCapStyle startCapStyle;
	//! End cap style.
	VGCapStyle endCapStyle;
#else
	//! Cap style.
	VGCapStyle capStyle;
#endif
	//! Join style.
	VGJoinStyle joinStyle;
	//! Miter limit.
	AMfloat miterLimit;
	//! Line width.
	AMfloat lineWidth;
	//! Dash phase.
	AMfloat dashPhase;
	//! Size of the dash pattern.
	AMuint32 dashPatternSize;
	//! Dash pattern hash.
	AMuint32 dashPatternHash;
} AMStrokeCacheDesc;

// Check is one or more stroke parameters have been changed from previously stored ones.
AMbool amStrokeChanged(AMStrokeCacheDesc *strokeCacheDesc,
					   const void *_context);
// Generate polygons that realize the stroke.
AMbool amStrokeGenerate(void *_context,
						const void *_slot,
						const AMfloat roundJoinAuxCoef);
#endif
