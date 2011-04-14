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

#ifndef _VGSCISSORS_H
#define _VGSCISSORS_H

/*!
	\file vgscissors.h
	\brief Scissoring related functions, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "dynarray.h"
#include "aabox.h"

/*!
	\brief Fixed point vertex, in drawing surface space, used to represent a scissor rectangle corner.
	It differs from AMSrfSpaceVertex in the components order.
*/
typedef union _AMScissorVertex {
	struct {
	#if defined(AM_BIG_ENDIAN)
		// big endian machines (e.g. PowerPC).
		//! 16bit x component.
		AMuint16 x;
		//! 16bit y component.
		AMuint16 y;
	#elif defined(AM_LITTLE_ENDIAN)
		// little endian machines (e.g. Intel).
		//! 16bit y component.
		AMuint16 y;
		//! 16bit x component.
		AMuint16 x;
	#else
		#error Unreachable point, please define target machine endianess (check config.h inclusion).
	#endif
	} p;
	//! Alias to refer the whole 32bit vertex value.
	AMuint32 xy;
} AMScissorVertex;

//! Scissor rectangle structure.
typedef struct _AMScissorRect {
	//! Bottom-left corner.
	AMScissorVertex bottomLeft;
	//! Top-right corner.
	AMScissorVertex topRight;
} AMScissorRect;

//! Scissor event flag value, entering events.
#define AM_SCISSOR_EVENT_ENTER 1
//! Scissor event flag value, exiting events.
#define AM_SCISSOR_EVENT_EXIT 2

//! Structure to represent a scissor event.
typedef struct _AMScissorEvent {
	//! x-coordinate of the event.
	AMuint16 x;
	//! y-coordinate of the scissor rectangle bottom border.
	AMuint16 bottom;
	//! y-coordinate of the scissor rectangle top border.
	AMuint16 top;
	//! Event type (enter/exit event).
	AMuint16 eventType;
} AMScissorEvent;

//! Vertical span structure.
typedef struct _AMScissorYSpan {
	//! span bottom y-coordinate.
	AMuint16 bottom;
	//! span top y-coordinate.
	AMuint16 top;
} AMScissorYSpan;
AM_DYNARRAY_DECLARE(AMScissorYSpanDynArray, AMScissorYSpan, _AMScissorYSpanDynArray)

//! Structure to represent multiple merged vertical spans.
typedef struct _AMScissorYMergedSpan {
	//! Span bottom y-coordinate.
	AMuint16 bottom;
	//! Span top y-coordinate.
	AMuint16 top;
	//! Flag that indicates if this span must be added to the current active span list.
	AMuint16 toAdd;
} AMScissorYMergedSpan;
AM_DYNARRAY_DECLARE(AMScissorYMergedSpanDynArray, AMScissorYMergedSpan, _AMScissorYMergedSpanDynArray)

//! Structure to represent a vertical span event.
typedef struct _AMScissorYSpanEvent {
	//! x-coordinate of the event.
	AMuint16 x;
	//! y-coordinate of the scissor rectangle bottom border.
	AMuint16 bottom;
	//! y-coordinate of the scissor rectangle top border.
	AMuint16 top;
} AMScissorYSpanEvent;
AM_DYNARRAY_DECLARE(AMScissorYSpanEventDynArray, AMScissorYSpanEvent, _AMScissorYSpanEventDynArray)

AM_DYNARRAY_DECLARE(AMScissorRectDynArray, AMScissorRect, _AMYScissorRectDynArray)
AM_DYNARRAY_DECLARE(AMScissorEventDynArray, AMScissorEvent, _AMYScissorEventDynArray)

// Rectangles decomposition into a set of non-overlapping rectangles.
AMbool amRectsDecompose(AMuint32 *area,
						AMScissorRectDynArray *splitRects,
						AMAABox2i *splitRectsUnionBox,
						const AMInt32DynArray *srcRects,
						void *_surface);

// Scissor rectangles decomposition into a set of non-overlapping rectangles.
AMbool amScissorRectsDecompose(void *_context,
							   void *_surface);
#endif
