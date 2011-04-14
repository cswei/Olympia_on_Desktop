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
	\file aabox.c
	\brief Axes-aligned box, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/
#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "aabox.h"

/*!
	\brief Check if two boxes overlap, integer coordinates.
	\param box0 first input box, integer coordinates.
	\param box1 second input box, integer coordinates.
	\return AM_TRUE if boxes overlap, else AM_FALSE;
*/
AMbool amAABox2iOverlap(const AMAABox2i *box0,
						const AMAABox2i *box1) {

	AMint32 width0 = AM_AABOX2_WIDTH(box0);
	AMint32 height0 = AM_AABOX2_HEIGHT(box0);
	AMint32 width1 = AM_AABOX2_WIDTH(box1);
	AMint32 height1 = AM_AABOX2_HEIGHT(box1);

	if (amAbs(box1->minPoint.x + box1->maxPoint.x - box0->minPoint.x - box0->maxPoint.x) < width0 + width1 &&
		amAbs(box1->minPoint.y + box1->maxPoint.y - box0->minPoint.y - box0->maxPoint.y) < height0 + height1)
		return AM_TRUE;
	return AM_FALSE;
}

AMbool amAABox2iIntersect(AMAABox2i *dst,
						  const AMAABox2i *box0,
						  const AMAABox2i *box1) {

	const AMAABox2i *lo;
	const AMAABox2i *hi;

	AM_ASSERT(dst);
	AM_ASSERT(box0);
	AM_ASSERT(box1);
	AM_ASSERT(box0 != dst);
	AM_ASSERT(box1 != dst);

	// x direction
	if (box0->minPoint.x < box1->minPoint.x) {
		lo = box0;
		hi = box1;
	}
	else {
		lo = box1;
		hi = box0;
	}
	if (hi->minPoint.x >= lo->maxPoint.x)
		return AM_FALSE;
	dst->minPoint.x = hi->minPoint.x;
	dst->maxPoint.x = AM_MIN(lo->maxPoint.x, hi->maxPoint.x);

	// y direction
	if (box0->minPoint.y < box1->minPoint.y) {
		lo = box0;
		hi = box1;
	}
	else {
		lo = box1;
		hi = box0;
	}
	if (hi->minPoint.y >= lo->maxPoint.y)
		return AM_FALSE;
	dst->minPoint.y = hi->minPoint.y;
	dst->maxPoint.y = AM_MIN(lo->maxPoint.y, hi->maxPoint.y);

	return AM_TRUE;
}

#if defined (RIM_VG_SRC)
#pragma pop
#endif

