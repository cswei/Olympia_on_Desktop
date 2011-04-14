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
	\file vgscissors.c
	\brief Scissoring related functions, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "vgcontext.h"
#include "vg_priv.h"

/*!
	\brief Build and sort the list of scissor events, starting from the specified scissor rectangles (previously clipped against the drawing surface).
	\param scissorEvents output list of scissor events.
	\param clippedScissorRects input (clipped) scissor rectangles.
*/
void amScissorEventsBuildAndSort(AMScissorEventDynArray *scissorEvents,
								 const AMScissorRectDynArray *clippedScissorRects) {

	AMint32 i, j, eventsCount;

	AM_ASSERT(scissorEvents);
	AM_ASSERT(clippedScissorRects);

	if (clippedScissorRects->size == 0) {
		scissorEvents->size = 0;
		return;
	}

	for (i = 0; i < (AMint32)clippedScissorRects->size; ++i) {

		AMScissorEvent enterEv;
		AMScissorEvent exitEv;

		// enter event
		enterEv.x = clippedScissorRects->data[i].bottomLeft.p.x;
		enterEv.bottom = clippedScissorRects->data[i].bottomLeft.p.y;
		enterEv.top = clippedScissorRects->data[i].topRight.p.y;
		enterEv.eventType = AM_SCISSOR_EVENT_ENTER;
		scissorEvents->data[i * 2] = enterEv;
		// exit event
		exitEv.x = clippedScissorRects->data[i].topRight.p.x;
		exitEv.bottom = clippedScissorRects->data[i].bottomLeft.p.y;
		exitEv.top = clippedScissorRects->data[i].topRight.p.y;
		exitEv.eventType = AM_SCISSOR_EVENT_EXIT;
		scissorEvents->data[i * 2 + 1] = exitEv;
	}
	// sort events
	eventsCount = (AMint32)clippedScissorRects->size * 2;
	scissorEvents->size = eventsCount;

	for (i = 0; i < eventsCount - 1; ++i) {
		for (j = i + 1; j < eventsCount; ++j) {
			if (scissorEvents->data[j].x < scissorEvents->data[i].x) {
				AMScissorEvent swapEv = scissorEvents->data[j];
				scissorEvents->data[j] = scissorEvents->data[i];
				scissorEvents->data[i] = swapEv;
			}
		}
	}
}

/*!
	\brief Given an enter scissor event, it builds the corresponding vertical span, and perform an ordered
	insertion in the specified span list.
	\param spanList output span list, where to insert the span relative to the event.
	\param ev input event.
*/
void amScissorEventEnter(AMScissorYSpanDynArray *spanList,
						 AMScissorEvent *ev) {

	AMuint32 count, count2;
	AMint32 i;
	AMScissorYSpan *mid, *first, newSpan;

	newSpan.bottom = ev->bottom;
	newSpan.top = ev->top;
	// ordered insertion
	first = &(spanList->data[0]);
	count = (AMuint32)spanList->size;
	for (; 0 < count; )	{
		count2 = count >> 1;
		mid = first;
		mid += count2;
		if (mid->bottom < ev->bottom) {
			first = ++mid;
			count -= count2 + 1;
		}
		else
			count = count2;
	}
	i = (AMint32)(first - &(spanList->data[0]));
	// now insert the new span at 'i' position
	AM_DYNARRAY_INSERT(*spanList, AMScissorYSpan, i, newSpan)
}

/*!
	\brief Given an exit scissor event, it builds the corresponding vertical span, and remove it from
	the specified span list.
	\param spanList output span list, where to remove the span relative to the event.
	\param ev input event.
*/
void amScissorEventExit(AMScissorYSpanDynArray *spanList,
						AMScissorEvent *ev) {

	AMuint32 count, count2;
	AMint32 i;
	AMScissorYSpan *mid, *first;

	AM_ASSERT(!spanList->error);

	// ordered insertion
	first = &(spanList->data[0]);
	count = (AMuint32)spanList->size;
	for (; 0 < count; )	{
		count2 = count >> 1;
		mid = first;
		mid += count2;
		if (mid->bottom < ev->bottom) {
			first = ++mid;
			count -= count2 + 1;
		}
		else
			count = count2;
	}

	i = (AMint32)(first - &(spanList->data[0]));
	AM_ASSERT(ev->bottom == spanList->data[i].bottom);
	for (; i < (AMint32)spanList->size; ++i) {
		if (spanList->data[i].top == ev->top) {
			AM_DYNARRAY_ERASE(*spanList, (AMuint32)i)
			return;
		}
	}
	// span must be found!
	AM_ASSERT(0 == 1);
}

/*!
	\brief Given the current active span list, it merges them together.
	\param mergedSpanList output merged span list.
	\param curSpanList input current span list.
*/
void amScissorSpanListMerge(AMScissorYMergedSpanDynArray *mergedSpanList,
							const AMScissorYSpanDynArray *curSpanList) {

	AMScissorYMergedSpan oldSpan, curSpan;
	AMint32 i;

	// curSpanList is already sorted by 'bottom'
	mergedSpanList->size = 0;
	if (curSpanList->size == 0)
		return;

	oldSpan.bottom = curSpanList->data[0].bottom;
	oldSpan.top = curSpanList->data[0].top;
	oldSpan.toAdd = 1;
	curSpan.toAdd = 1;

	i = 1;
	while (i < (AMint32)curSpanList->size) {

		curSpan.bottom = curSpanList->data[i].bottom;
		curSpan.top = curSpanList->data[i].top;
		i++;
		// now try to merge oldSpan with curSpan: if they could be merged oldSpan = merge(oldSpan, curSpan)
		// else output(oldSpan) and oldSpan = curSpan
		AM_ASSERT(curSpan.bottom >= oldSpan.bottom);

		if (curSpan.bottom <= oldSpan.top) {
			if (oldSpan.top < curSpan.top)
				oldSpan.top = curSpan.top;
		}
		else {
			AM_DYNARRAY_PUSH_BACK(*mergedSpanList, AMScissorYMergedSpan, oldSpan)
			oldSpan = curSpan;
		}
	}

	AM_DYNARRAY_PUSH_BACK(*mergedSpanList, AMScissorYMergedSpan, oldSpan)
}

/*!
	\brief Given the list of scissor events and spans, it produces a set of non-overlapping rectangles.
	\param splitRects array of output non-overlapping rectangles.
	\param surface surface containing input scissor events list, from which to produce output non-overlapping rectangles.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
*/
AMbool amScissorRectsFromSpans(AMuint32 *area,
							   AMScissorRectDynArray *splitRects,
							   AMDrawingSurface *surface) {

	AMint32 i, j, k, idx;
	AMbool res;
	AMScissorYSpanDynArray *curSpanList = &surface->curSpanList;
	AMScissorYMergedSpanDynArray *mergedSpanList = &surface->mergedSpanList;
	AMScissorYSpanEventDynArray *curSpanEvents = &surface->curSpanEvents;
	AMScissorEventDynArray *scissorEvents = &surface->scissorEvents;

	// rewind temporary arrays
	curSpanList->size = 0;
	mergedSpanList->size = 0;
	curSpanEvents->size = 0;
	*area = 0;
	i = 0;
	while (i < (AMint32)scissorEvents->size) {

		AMScissorEvent curEvent = scissorEvents->data[i];

		// do all events with the same x
		do {
			if (scissorEvents->data[i].eventType == AM_SCISSOR_EVENT_ENTER) {
				amScissorEventEnter(curSpanList, &scissorEvents->data[i]);
				// check for memory errors
				if (curSpanList->error) {
					curSpanList->error = AM_DYNARRAY_NO_ERROR;
					return AM_FALSE;
				}
			}
			else
				amScissorEventExit(curSpanList, &scissorEvents->data[i]);
			i++;
		} while (i < (AMint32)scissorEvents->size && scissorEvents->data[i].x == curEvent.x);

		// merge the current list
		amScissorSpanListMerge(mergedSpanList, curSpanList);

		for (j = 0; j < (AMint32)curSpanEvents->size;) {
			
			idx = -1;
			for (k = 0; k < (AMint32)mergedSpanList->size; ++k) {
				if (curSpanEvents->data[j].bottom == mergedSpanList->data[k].bottom &&
					curSpanEvents->data[j].top == mergedSpanList->data[k].top) {
					mergedSpanList->data[k].toAdd = 0;
					idx = k;
					break;
				}
			}
			// not found
			if (idx < 0) {

				AMScissorRect newRect;

				newRect.bottomLeft.p.x = curSpanEvents->data[j].x;
				newRect.bottomLeft.p.y = curSpanEvents->data[j].bottom;
				newRect.topRight.p.x = curEvent.x;
				newRect.topRight.p.y = curSpanEvents->data[j].top;
				AM_DYNARRAY_PUSH_BACK(*splitRects, AMScissorRect, newRect)
				*area += (newRect.topRight.p.x - newRect.bottomLeft.p.x) * (newRect.topRight.p.y - newRect.bottomLeft.p.y);
				AM_DYNARRAY_ERASE(*curSpanEvents, (AMuint32)j)
			}
			else
				j++;
		}

		for (j = 0; j < (AMint32)mergedSpanList->size; ++j) {

			if (mergedSpanList->data[j].toAdd) {

				AMScissorYSpanEvent newEvent;

				newEvent.x = curEvent.x;
				newEvent.bottom = mergedSpanList->data[j].bottom;
				newEvent.top = mergedSpanList->data[j].top;
				AM_DYNARRAY_PUSH_BACK(*curSpanEvents, AMScissorYSpanEvent, newEvent);
			}
		}
	}

	if (curSpanList->error || mergedSpanList->error || curSpanEvents->error || splitRects->error) {
		curSpanList->error = AM_DYNARRAY_NO_ERROR;
		mergedSpanList->error = AM_DYNARRAY_NO_ERROR;
		curSpanEvents->error = AM_DYNARRAY_NO_ERROR;
		splitRects->error = AM_DYNARRAY_NO_ERROR;
		res = AM_FALSE;
	}
	else
		res = AM_TRUE;

	return res;
}

/*!
	\param area sum of decomposed rectangles's area.
	\param splitRects output array that will contain decomposed rectangles.
	\param splitRectsUnionBox output bounding box of decomposed rectangles.
	\param srcRects input rectangles to be decomposed.
	\param _surface pointer to a drawing surface, where to clip rectangles against.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
*/
AMbool amRectsDecompose(AMuint32 *area,
						AMScissorRectDynArray *splitRects,
						AMAABox2i *splitRectsUnionBox,
						const AMInt32DynArray *srcRects,
						void *_surface) {

	AMDrawingSurface *surface = (AMDrawingSurface *)_surface;
	AMint32 width, height;
	AMbool res;
	AMuint32 i, j;
	AMVect2i minPoint;
	AMScissorRect r;

	AM_ASSERT(splitRects);
	AM_ASSERT(splitRectsUnionBox);
	AM_ASSERT(srcRects);
	AM_ASSERT(surface);

	// if we don't have source rectangles simply exit
	if (srcRects->size == 0) {
		AM_VECT2_SET(&splitRectsUnionBox->minPoint, 0, 0)
		AM_VECT2_SET(&splitRectsUnionBox->maxPoint, 0, 0)
		splitRects->size = 0;
		return AM_TRUE;
	}

	AM_VECT2_SET(&splitRectsUnionBox->minPoint, AM_MAX_INT32, AM_MAX_INT32)
	AM_VECT2_SET(&splitRectsUnionBox->maxPoint, AM_MIN_INT32, AM_MIN_INT32)
	
	j = (AMuint32)srcRects->size >> 2;
	if (surface->clippedScissorRects.capacity < j) {
		AM_DYNARRAY_CLEAR_RESERVE(surface->clippedScissorRects, AMScissorRect, j)
	// check for memory errors
		if (surface->clippedScissorRects.error) {
			surface->clippedScissorRects.error = AM_DYNARRAY_NO_ERROR;
		return AM_FALSE;
		}
	}
	surface->clippedScissorRects.size = 0;

	for (i = 0; i < j; ++i) {
		// bottom-left corner
		minPoint.x = srcRects->data[i * 4];
		minPoint.y = srcRects->data[i * 4 + 1];
		// width and height
		width = srcRects->data[i * 4 + 2];
		height = srcRects->data[i * 4 + 3];
		// discard all OpenVG invalid scissor rectangles
		if (width <= 0 || height <= 0)
			continue;
		// clamp scissor rectangle against viewport
		if (minPoint.x < 0) {
			width += minPoint.x;
			minPoint.x = 0;
		}
		if (minPoint.y < 0) {
			height += minPoint.y;
			minPoint.y = 0;
		}
		if (minPoint.x + width > amSrfWidthGet(surface)) {
			width = amSrfWidthGet(surface) - minPoint.x;
			if (width <= 0)
				continue;
		}
		if (minPoint.y + height > amSrfHeightGet(surface)) {
			height = amSrfHeightGet(surface) - minPoint.y;
			if (height <= 0)
				continue;
		}
		// reject the scissor rectangles if it lies outside the viewport
		if (minPoint.x >= amSrfWidthGet(surface) || minPoint.y >= amSrfHeightGet(surface) || width <= 0 || height <= 0)
			continue;

		r.bottomLeft.p.x = (AMuint16)minPoint.x;
		r.bottomLeft.p.y = (AMuint16)minPoint.y;

		// verify that the cast is doing well
		AM_ASSERT(r.bottomLeft.p.x == minPoint.x);
		AM_ASSERT(r.bottomLeft.p.y == minPoint.y);

		r.topRight.p.x = (AMuint16)(r.bottomLeft.p.x + width);
		r.topRight.p.y = (AMuint16)(r.bottomLeft.p.y + height);

		// update current bounding box
		if (r.bottomLeft.p.x < splitRectsUnionBox->minPoint.x)
			splitRectsUnionBox->minPoint.x = r.bottomLeft.p.x;
		if (r.bottomLeft.p.y < splitRectsUnionBox->minPoint.y)
			splitRectsUnionBox->minPoint.y = r.bottomLeft.p.y;
		if (r.topRight.p.x > splitRectsUnionBox->maxPoint.x)
			splitRectsUnionBox->maxPoint.x = r.topRight.p.x;
		if (r.topRight.p.y > splitRectsUnionBox->maxPoint.y)
			splitRectsUnionBox->maxPoint.y = r.topRight.p.y;

		AM_DYNARRAY_PUSH_BACK_LIGHT(surface->clippedScissorRects, r)
	}

	// check a valid box
	if (splitRectsUnionBox->minPoint.x > splitRectsUnionBox->maxPoint.x || splitRectsUnionBox->minPoint.y > splitRectsUnionBox->maxPoint.y) {
		// in this case none of the original un-split rectangles has been survived to the clipping
		AM_ASSERT(surface->clippedScissorRects.size == 0);
		AM_VECT2_SET(&splitRectsUnionBox->minPoint, 0, 0)
		AM_VECT2_SET(&splitRectsUnionBox->maxPoint, 0, 0)
		splitRects->size = 0;
		return AM_TRUE;
	}

	if (surface->scissorEvents.capacity < surface->clippedScissorRects.size * 2) {
		AM_DYNARRAY_CLEAR_RESERVE(surface->scissorEvents, AMScissorEvent, surface->clippedScissorRects.size * 2)
	// check for memory errors
		if (surface->scissorEvents.error) {
			surface->scissorEvents.error = AM_DYNARRAY_NO_ERROR;
		return AM_FALSE;
	}
	}
	amScissorEventsBuildAndSort(&surface->scissorEvents, &surface->clippedScissorRects);

	// rewind the output array
	splitRects->size = 0;
	res = amScissorRectsFromSpans(area, splitRects, surface);
	return res;
}

/*!
	\brief It decomposes OpenVG scissor rectangles into a set of non-overlapping rectangles.
	\param _context pointer to a AMContext structure, containing input and output (non-overlapped) scissor rectangles.
	\param _surface pointer to a AMDrawingSurface structure, specifying the drawing surface where original OpenVG scissor rectangles must be clipped against.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
*/
AMbool amScissorRectsDecompose(void *_context,
							   void *_surface) {

	AMuint32 area;
	AMContext *context = (AMContext *)_context;

	AM_ASSERT(context && context->initialized);
	AM_ASSERT(context->scissorRectsModified);

	if (!amRectsDecompose(&area, &context->splitScissorRects, &context->splitScissorRectsBox, &context->scissorRects, _surface))
		return AM_FALSE;

	context->scissorRectsModified = AM_FALSE;
	return AM_TRUE;
}

#if defined (RIM_VG_SRC)
#pragma pop
#endif

