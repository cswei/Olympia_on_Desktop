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

#ifndef _AABOX_H
#define _AABOX_H

/*!
	\file aabox.h
	\brief Axes-aligned box, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "matrix.h"

/*!
	\brief A 2D axes-aligned box, float coordinates.
*/
typedef struct _AMAABox2f {
	//! Corner with the minimum coordinate values.
	AMVect2f minPoint;
	//! Corner with the maximum coordinate values.
	AMVect2f maxPoint;
} AMAABox2f;

/*!
	\brief A 2D axes-aligned box, integer coordinates.
*/
typedef struct _AMAABox2i {
	//! Corner with the minimum coordinate values.
	AMVect2i minPoint;
	//! Corner with the maximum coordinate values.
	AMVect2i maxPoint;
} AMAABox2i;

/*!
	\brief A 2D axes-aligned box, fixed point coordinates.
*/
typedef struct _AMAABox2x {
	//! Corner with the minimum coordinate values.
	AMVect2x minPoint;
	//! Corner with the maximum coordinate values.
	AMVect2x maxPoint;
} AMAABox2x;

/*!
	Construct axes-aligned box \a _dst given 2 points (\a _p0 and \a _p1).
*/
#define AM_AABOX2_SET(_dst, _p0, _p1) \
	if ((_p0)->x < (_p1)->x) { \
		(_dst)->minPoint.x = (_p0)->x; \
		(_dst)->maxPoint.x = (_p1)->x; \
	} \
	else { \
		(_dst)->minPoint.x = (_p1)->x; \
		(_dst)->maxPoint.x = (_p0)->x; \
	} \
	if ((_p0)->y < (_p1)->y) { \
		(_dst)->minPoint.y = (_p0)->y; \
		(_dst)->maxPoint.y = (_p1)->y; \
	} \
	else { \
		(_dst)->minPoint.y = (_p1)->y; \
		(_dst)->maxPoint.y = (_p0)->y; \
	}

/*!
	Return the axes-aligned \a _box width.
*/
#define AM_AABOX2_WIDTH(_box) ((_box)->maxPoint.x - (_box)->minPoint.x)

/*!
	Return the axes-aligned \a _box height.
*/
#define AM_AABOX2_HEIGHT(_box) ((_box)->maxPoint.y - (_box)->minPoint.y)

/*!
	Return the axes-aligned \a _box center.
*/
#define AM_AABOX2_CENTER(_dst, _box) \
	(_dst)->x = ((_box)->minPoint.x + (_box)->maxPoint.x) / 2; \
	(_dst)->y = ((_box)->minPoint.y + (_box)->maxPoint.y) / 2; \

/*!
	Update the axes-aligned \a _box in order to contain the specified \a _point.
*/
#define AM_AABOX2_EXTEND_TO_INCLUDE(_box, _point) \
	if ((_box)->minPoint.x > (_point)->x) \
		(_box)->minPoint.x = (_point)->x; \
	else \
	if ((_box)->maxPoint.x < (_point)->x) \
		(_box)->maxPoint.x = (_point)->x; \
	if ((_box)->minPoint.y > (_point)->y) \
		(_box)->minPoint.y = (_point)->y; \
	else \
	if ((_box)->maxPoint.y < (_point)->y) \
		(_box)->maxPoint.y = (_point)->y;

// Check if box0 contains box1, integer coordinates.
AM_INLINE AMbool amAABox2iContain(const AMAABox2i *box0,
								  const AMAABox2i *box1) {

	return (box1->minPoint.x >= box0->minPoint.x && box1->minPoint.y >= box0->minPoint.y &&
			box1->maxPoint.x <= box0->maxPoint.x && box1->maxPoint.y <= box0->maxPoint.y) ? AM_TRUE : AM_FALSE;
}

// Check if two boxes overlap, integer coordinates.
AMbool amAABox2iOverlap(const AMAABox2i *box0,
						const AMAABox2i *box1);
// Check (and calculate) the intersection between two boxes, integer coordinates.
AMbool amAABox2iIntersect(AMAABox2i *dst,
						  const AMAABox2i *box0,
						  const AMAABox2i *box1);

#endif
