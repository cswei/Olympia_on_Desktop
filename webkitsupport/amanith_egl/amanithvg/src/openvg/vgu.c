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
	\file vgu.c
	\brief OpenVG VGU functions, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "vgu.h"
#include "vg_priv.h"
#include "vgcontext.h"

#if defined (RIM_VG_SRC)
#define VGU_API_CALL 
#endif


// *********************************************************************
//                        Private implementations
// *********************************************************************

/*!
	\brief It appends a line segment to the given path.
	\param path the destination path.
	\param x0 x coordinate of the start point.
	\param y0 y coordinate of the start point.
	\param x1 x coordinate of the end point.
	\param y1 y coordinate of the end point.
	\param context context containing coordinate converters.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
*/
AMbool amVguLine(AMPath *path,
				 const AMfloat x0,
				 const AMfloat y0,
				 const AMfloat x1,
				 const AMfloat y1,
				 const AMContext *context) {

	AMuint8 commands[2] = { VG_MOVE_TO_ABS, VG_LINE_TO_ABS };
	AMint8 s8Data[4];
	AMint16 s16Data[4];
	AMint32 s32Data[4];
	AMfloat fData[4];
	AMbool res;

	AM_ASSERT(path);
	AM_ASSERT((AMint32)path->dataType != (AMint32)VG_PATH_DATATYPE_INVALID);

	switch (path->dataType) {

		case VG_PATH_DATATYPE_S_8:
			s8Data[0] = (AMint8)x0;
			s8Data[1] = (AMint8)y0;
			s8Data[2] = (AMint8)x1;
			s8Data[3] = (AMint8)y1;
			res = amPathDataAppend(path, 2, commands, (const void *)s8Data, path->dataType, context, AM_FALSE);
			break;
		case VG_PATH_DATATYPE_S_16:
			s16Data[0] = (AMint16)x0;
			s16Data[1] = (AMint16)y0;
			s16Data[2] = (AMint16)x1;
			s16Data[3] = (AMint16)y1;
			res = amPathDataAppend(path, 2, commands, (const void *)s16Data, path->dataType, context, AM_FALSE);
			break;
		case VG_PATH_DATATYPE_S_32:
			s32Data[0] = (AMint32)x0;
			s32Data[1] = (AMint32)y0;
			s32Data[2] = (AMint32)x1;
			s32Data[3] = (AMint32)y1;
			res = amPathDataAppend(path, 2, commands, (const void *)s32Data, path->dataType, context, AM_FALSE);
			break;
		default:
			AM_ASSERT(path->dataType == VG_PATH_DATATYPE_F);
			fData[0] = x0;
			fData[1] = y0;
			fData[2] = x1;
			fData[3] = y1;
			res = amPathDataAppend(path, 2, commands, (const void *)fData, path->dataType, context, AM_FALSE);
			break;
	}
	return res;
}

/*!
	\brief It appends a polyline (connected sequence of line segments) or polygon to the given path.
	\param path the destination path.
	\param points input polyline points.
	\param count number of points.
	\param closed AM_TRUE if the specified polyline is closed, else AM_FALSE.
	\param context context containing coordinate converters.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
	\pre count > 0.
*/
AMbool amVguPolygon(AMPath *path,
					const AMfloat *points,
					const AMint32 count,
					const AMbool closed,
					AMContext *context) {

	AMint32 i, cmdCount;
	AMbool res;

	AM_ASSERT(path);
	AM_ASSERT((AMint32)path->dataType != (AMint32)VG_PATH_DATATYPE_INVALID);
	AM_ASSERT(points);
	AM_ASSERT(count > 0);

	context->vguAuxS8Data.size = 0;
	context->vguAuxS16Data.size = 0;
	context->vguAuxS32Data.size = 0;
	context->vguAuxF32Data.size = 0;


	// allocate commands array, and return an error if allocation has failed
	cmdCount = (closed) ? count + 1 : count;
	if ((AMint32)context->vguAuxCommands.capacity < cmdCount) {
		AM_DYNARRAY_CLEAR_RESERVE(context->vguAuxCommands, AMuint8, cmdCount)
	}
	context->vguAuxCommands.size = 0;
	if (context->vguAuxCommands.error) {
		context->vguAuxCommands.error = AM_DYNARRAY_NO_ERROR;
		return AM_FALSE;
	}

	// build commands array
	context->vguAuxCommands.data[0] = VG_MOVE_TO_ABS;
	for (i = 1; i < count; ++i)
		context->vguAuxCommands.data[i] = VG_LINE_TO_ABS;
	if (closed)
		context->vguAuxCommands.data[count] = VG_CLOSE_PATH;

	switch (path->dataType) {
		case VG_PATH_DATATYPE_S_8:
			if ((AMint32)context->vguAuxS8Data.capacity < 2 * count) {
				AM_DYNARRAY_CLEAR_RESERVE(context->vguAuxS8Data, AMint8, 2 * count)
			}
			context->vguAuxS8Data.size = 0;
			if (context->vguAuxS8Data.error) {
				context->vguAuxS8Data.error = AM_DYNARRAY_NO_ERROR;
				return AM_FALSE;
			}
			for (i = 0; i < count; ++i) {
				context->vguAuxS8Data.data[i * 2] = (AMint8)points[i * 2];
				context->vguAuxS8Data.data[i * 2 + 1] = (AMint8)points[i * 2 + 1];
			}
			res = amPathDataAppend(path, cmdCount, context->vguAuxCommands.data, (const void *)context->vguAuxS8Data.data, path->dataType, context, AM_FALSE);
			break;
		case VG_PATH_DATATYPE_S_16:
			if ((AMint32)context->vguAuxS16Data.capacity < 2 * count) {
				AM_DYNARRAY_CLEAR_RESERVE(context->vguAuxS16Data, AMint16, 2 * count)
			}
			context->vguAuxS16Data.size = 0;
			if (context->vguAuxS16Data.error) {
				context->vguAuxS16Data.error = AM_DYNARRAY_NO_ERROR;
				return AM_FALSE;
			}
			for (i = 0; i < count; ++i) {
				context->vguAuxS16Data.data[i * 2] = (AMint16)points[i * 2];
				context->vguAuxS16Data.data[i * 2 + 1] = (AMint16)points[i * 2 + 1];
			}
			res = amPathDataAppend(path, cmdCount, context->vguAuxCommands.data, (const void *)context->vguAuxS16Data.data, path->dataType, context, AM_FALSE);
			break;
		case VG_PATH_DATATYPE_S_32:
			if ((AMint32)context->vguAuxS32Data.capacity < 2 * count) {
				AM_DYNARRAY_CLEAR_RESERVE(context->vguAuxS32Data, AMint32, 2 * count)
			}
			context->vguAuxS32Data.size = 0;
			if (context->vguAuxS32Data.error) {
				context->vguAuxS32Data.error = AM_DYNARRAY_NO_ERROR;
				return AM_FALSE;
			}
			for (i = 0; i < count; ++i) {
				context->vguAuxS32Data.data[i * 2] = (AMint32)points[i * 2];
				context->vguAuxS32Data.data[i * 2 + 1] = (AMint32)points[i * 2 + 1];
			}
			res = amPathDataAppend(path, cmdCount, context->vguAuxCommands.data, (const void *)context->vguAuxS32Data.data, path->dataType, context, AM_FALSE);
			break;
		default:
			AM_ASSERT(path->dataType == VG_PATH_DATATYPE_F);
			if ((AMint32)context->vguAuxF32Data.capacity < 2 * count) {
				AM_DYNARRAY_CLEAR_RESERVE(context->vguAuxF32Data, AMfloat, 2 * count)
			}
			context->vguAuxF32Data.size = 0;
			if (context->vguAuxF32Data.error) {
				context->vguAuxF32Data.error = AM_DYNARRAY_NO_ERROR;
				return AM_FALSE;
			}
			for (i = 0; i < count; ++i) {
				context->vguAuxF32Data.data[i * 2] = points[i * 2];
				context->vguAuxF32Data.data[i * 2 + 1] = points[i * 2 + 1];
			}
			res = amPathDataAppend(path, cmdCount, context->vguAuxCommands.data, (const void *)context->vguAuxF32Data.data, path->dataType, context, AM_FALSE);
			break;
	}

	return res;
}

/*!
	\brief It appends an axis-aligned rectangle with its lower-left corner at (x, y) and a given width
	and height to the given path.
	\param path the destination path.
	\param x x coordinate of the lower-left corner.
	\param y y coordinate of the lower-left corner.
	\param width width of the rectangle.
	\param height height of the rectangle.
	\param context context containing coordinate converters.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
	\pre width, height > 0.
*/
AMbool amVguRect(AMPath *path,
				 const AMfloat x,
				 const AMfloat y,
				 const AMfloat width,
				 const AMfloat height,
				 const AMContext *context) {

	AMuint8 commands[5] = { VG_MOVE_TO_ABS, VG_HLINE_TO_REL, VG_VLINE_TO_REL, VG_HLINE_TO_REL, VG_CLOSE_PATH };
	AMint8 s8Data[5];
	AMint16 s16Data[5];
	AMint32 s32Data[5];
	AMfloat fData[5];
	AMbool res;

	AM_ASSERT(width > 0.0f);
	AM_ASSERT(height > 0.0f);
	AM_ASSERT(path);
	AM_ASSERT((AMint32)path->dataType != (AMint32)VG_PATH_DATATYPE_INVALID);

	switch (path->dataType) {

		case VG_PATH_DATATYPE_S_8:
			s8Data[0] = (AMint8)x;
			s8Data[1] = (AMint8)y;
			s8Data[2] = (AMint8)width;
			s8Data[3] = (AMint8)height;
			s8Data[4] = (AMint8)(-width);
			res = amPathDataAppend(path, 5, commands, (const void *)s8Data, path->dataType, context, AM_FALSE);
			break;
		case VG_PATH_DATATYPE_S_16:
			s16Data[0] = (AMint16)x;
			s16Data[1] = (AMint16)y;
			s16Data[2] = (AMint16)width;
			s16Data[3] = (AMint16)height;
			s16Data[4] = (AMint16)(-width);
			res = amPathDataAppend(path, 5, commands, (const void *)s16Data, path->dataType, context, AM_FALSE);
			break;
		case VG_PATH_DATATYPE_S_32:
			s32Data[0] = (AMint32)x;
			s32Data[1] = (AMint32)y;
			s32Data[2] = (AMint32)width;
			s32Data[3] = (AMint32)height;
			s32Data[4] = (AMint32)(-width);
			res = amPathDataAppend(path, 5, commands, (const void *)s32Data, path->dataType, context, AM_FALSE);
			break;
		default:
			AM_ASSERT(path->dataType == VG_PATH_DATATYPE_F);
			fData[0] = x;
			fData[1] = y;
			fData[2] = width;
			fData[3] = height;
			fData[4] = -width;
			res = amPathDataAppend(path, 5, commands, (const void *)fData, path->dataType, context, AM_FALSE);
			break;
	}
	return res;
}

/*!
	\brief It appends an axis-aligned round-cornered rectangle with the lower-left corner of its rectangular
	bounding box at (x, y) and a given width, height, arcWidth, and arcHeight to the given path.
	\param path the destination path.
	\param x x coordinate of the lower-left corner.
	\param y y coordinate of the lower-left corner.
	\param width width of the rectangle.
	\param height height of the rectangle.
	\param arcWidth length of the round corner horizontal axis.
	\param arcHeight length of the round corner vertical axis.
	\param context context containing coordinate converters.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
	\pre width, height > 0.
*/
AMbool amVguRoundRect(AMPath *path,
					  const AMfloat x,
					  const AMfloat y,
					  const AMfloat width,
					  const AMfloat height,
					  const AMfloat arcWidth,
					  const AMfloat arcHeight,
					  const AMContext *context) {

	AMfloat arcW, arcH, halfArcW, halfArcH;
	AMuint8 commands[10] = { VG_MOVE_TO_ABS, VG_HLINE_TO_REL, VG_SCCWARC_TO_REL, VG_VLINE_TO_REL,
							 VG_SCCWARC_TO_REL, VG_HLINE_TO_REL, VG_SCCWARC_TO_REL, VG_VLINE_TO_REL,
							 VG_SCCWARC_TO_REL, VG_CLOSE_PATH };
	AMint8 s8Data[26];
	AMint16 s16Data[26];
	AMint32 s32Data[26];
	AMfloat fData[26];
	AMbool res;

	AM_ASSERT(path);
	AM_ASSERT((AMint32)path->dataType != (AMint32)VG_PATH_DATATYPE_INVALID);
	AM_ASSERT(width > 0.0f);
	AM_ASSERT(height > 0.0f);

	arcW = AM_CLAMP(arcWidth, 0.0f, width);
	arcH = AM_CLAMP(arcHeight, 0.0f, height);
	halfArcW = arcW * 0.5f;
	halfArcH = arcH * 0.5f;

	switch (path->dataType) {
		case VG_PATH_DATATYPE_S_8:
			s8Data[0] = (AMint8)(x + halfArcW);
			s8Data[1] = (AMint8)y;
			s8Data[2] = (AMint8)(width - arcW);
			s8Data[3] = (AMint8)halfArcW;
			s8Data[4] = (AMint8)halfArcH;
			s8Data[5] = 0;
			s8Data[6] = (AMint8)halfArcW;
			s8Data[7] = (AMint8)halfArcH;
			s8Data[8] = (AMint8)(height - arcH);
			s8Data[9] = (AMint8)halfArcW;
			s8Data[10] = (AMint8)halfArcH;
			s8Data[11] = 0;
			s8Data[12] = (AMint8)(-halfArcW);
			s8Data[13] = (AMint8)halfArcH;
			s8Data[14] = (AMint8)(-(width - arcW));
			s8Data[15] = (AMint8)halfArcW;
			s8Data[16] = (AMint8)halfArcH;
			s8Data[17] = 0;
			s8Data[18] = (AMint8)(-halfArcW);
			s8Data[19] = (AMint8)(-halfArcH);
			s8Data[20] = (AMint8)(-(height - arcH));
			s8Data[21] = (AMint8)halfArcW;
			s8Data[22] = (AMint8)halfArcH;
			s8Data[23] = 0;
			s8Data[24] = (AMint8)halfArcW;
			s8Data[25] = (AMint8)(-halfArcH);
			res = amPathDataAppend(path, 10, commands, (const void *)s8Data, path->dataType, context, AM_FALSE);
			break;
		case VG_PATH_DATATYPE_S_16:
			s16Data[0] = (AMint16)(x + halfArcW);
			s16Data[1] = (AMint16)y;
			s16Data[2] = (AMint16)(width - arcW);
			s16Data[3] = (AMint16)halfArcW;
			s16Data[4] = (AMint16)halfArcH;
			s16Data[5] = 0;
			s16Data[6] = (AMint16)halfArcW;
			s16Data[7] = (AMint16)halfArcH;
			s16Data[8] = (AMint16)(height - arcH);
			s16Data[9] = (AMint16)halfArcW;
			s16Data[10] = (AMint16)halfArcH;
			s16Data[11] = 0;
			s16Data[12] = (AMint16)(-halfArcW);
			s16Data[13] = (AMint16)halfArcH;
			s16Data[14] = (AMint16)(-(width - arcW));
			s16Data[15] = (AMint16)halfArcW;
			s16Data[16] = (AMint16)halfArcH;
			s16Data[17] = 0;
			s16Data[18] = (AMint16)(-halfArcW);
			s16Data[19] = (AMint16)(-halfArcH);
			s16Data[20] = (AMint16)(-(height - arcH));
			s16Data[21] = (AMint16)halfArcW;
			s16Data[22] = (AMint16)halfArcH;
			s16Data[23] = 0;
			s16Data[24] = (AMint16)halfArcW;
			s16Data[25] = (AMint16)(-halfArcH);
			res = amPathDataAppend(path, 10, commands, (const void *)s16Data, path->dataType, context, AM_FALSE);
			break;
		case VG_PATH_DATATYPE_S_32:
			s32Data[0] = (AMint32)(x + halfArcW);
			s32Data[1] = (AMint32)y;
			s32Data[2] = (AMint32)(width - arcW);
			s32Data[3] = (AMint32)halfArcW;
			s32Data[4] = (AMint32)halfArcH;
			s32Data[5] = 0;
			s32Data[6] = (AMint32)halfArcW;
			s32Data[7] = (AMint32)halfArcH;
			s32Data[8] = (AMint32)(height - arcH);
			s32Data[9] = (AMint32)halfArcW;
			s32Data[10] = (AMint32)halfArcH;
			s32Data[11] = 0;
			s32Data[12] = (AMint32)(-halfArcW);
			s32Data[13] = (AMint32)halfArcH;
			s32Data[14] = (AMint32)(-(width - arcW));
			s32Data[15] = (AMint32)halfArcW;
			s32Data[16] = (AMint32)halfArcH;
			s32Data[17] = 0;
			s32Data[18] = (AMint32)(-halfArcW);
			s32Data[19] = (AMint32)(-halfArcH);
			s32Data[20] = (AMint32)(-(height - arcH));
			s32Data[21] = (AMint32)halfArcW;
			s32Data[22] = (AMint32)halfArcH;
			s32Data[23] = 0;
			s32Data[24] = (AMint32)halfArcW;
			s32Data[25] = (AMint32)(-halfArcH);
			res = amPathDataAppend(path, 10, commands, (const void *)s32Data, path->dataType, context, AM_FALSE);
			break;
		default:
			AM_ASSERT(path->dataType == VG_PATH_DATATYPE_F);
			fData[0] = (x + halfArcW);
			fData[1] = y;
			fData[2] = (width - arcW);
			fData[3] = halfArcW;
			fData[4] = halfArcH;
			fData[5] = 0.0f;
			fData[6] = halfArcW;
			fData[7] = halfArcH;
			fData[8] = (height - arcH);
			fData[9] = halfArcW;
			fData[10] = halfArcH;
			fData[11] = 0.0f;
			fData[12] = (-halfArcW);
			fData[13] = halfArcH;
			fData[14] = (-(width - arcW));
			fData[15] = halfArcW;
			fData[16] = halfArcH;
			fData[17] = 0.0f;
			fData[18] = (-halfArcW);
			fData[19] = (-halfArcH);
			fData[20] = (-(height - arcH));
			fData[21] = halfArcW;
			fData[22] = halfArcH;
			fData[23] = 0.0f;
			fData[24] = halfArcW;
			fData[25] = (-halfArcH);
			res = amPathDataAppend(path, 10, commands, (const void *)fData, path->dataType, context, AM_FALSE);
			break;
	}
	return res;
}

/*!
	\brief It appends an axis-aligned ellipse to the given path. The center of the ellipse is given by (cx, cy) and
	the dimensions of the axis-aligned rectangle enclosing the ellipse are given by width and height. The
	ellipse begins at (cx + width/2, cy) and is stroked as two equal counter-clockwise arcs.
	\param path the destination path.
	\param cx x coordinate of the center.
	\param cy y coordinate of the center.
	\param width width of horizontal axis.
	\param height height of vertical axis.
	\param context context containing coordinate converters.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
	\pre width, height > 0.
*/
AMbool amVguEllipse(AMPath *path,
					const AMfloat cx,
					const AMfloat cy,
					const AMfloat width,
					const AMfloat height,
					const AMContext *context) {

#if defined (RIM_VG_SRC)
	AMuint8 commands[4] = { VG_MOVE_TO_ABS, VG_SCCWARC_TO_REL, VG_SCCWARC_TO_REL, VG_CLOSE_PATH };
#else
	AMuint8 commands[4] = { VG_MOVE_TO_ABS, VG_SCCWARC_TO_REL, VG_SCCWARC_TO_ABS, VG_CLOSE_PATH };
#endif
	AMint8 s8Data[12];
	AMint16 s16Data[12];
	AMint32 s32Data[12];
#if defined (RIM_VG_SRC)
	AMfloat fData[12], x0, y0, halfWidth, halfHeight;
#else
	AMfloat fData[12], x0, y0, y1, halfWidth, halfHeight;
#endif
	AMbool res;

	AM_ASSERT(path);
	AM_ASSERT((AMint32)path->dataType != (AMint32)VG_PATH_DATATYPE_INVALID);
	AM_ASSERT(width > 0.0f);
	AM_ASSERT(height > 0.0f);

	halfWidth = (width * 0.5f);
	halfHeight = (height * 0.5f);

	x0 = halfWidth + cx;
	y0 = cy;
#if defined (RIM_VG_SRC)
    // Deleted.
#else
	y1 = cy;
#endif

	switch (path->dataType) {
		case VG_PATH_DATATYPE_S_8:
			s8Data[0] = (AMint8)x0;
			s8Data[1] = (AMint8)y0;
			s8Data[2] = (AMint8)halfWidth;
			s8Data[3] = (AMint8)halfHeight;
			s8Data[4] = 0;
			s8Data[5] = (AMint8)(-width);
			s8Data[6] = 0;
			s8Data[7] = (AMint8)halfWidth;
			s8Data[8] = (AMint8)halfHeight;
			s8Data[9] = 0;
#if defined (RIM_VG_SRC)
			s8Data[10] = (AMint8)width;
			s8Data[11] = (AMint8)0;
#else
			s8Data[10] = (AMint8)x0;
			s8Data[11] = (AMint8)y1;
#endif
			res = amPathDataAppend(path, 4, commands, (const void *)s8Data, path->dataType, context, AM_FALSE);
			break;
		case VG_PATH_DATATYPE_S_16:
			s16Data[0] = (AMint16)x0;
			s16Data[1] = (AMint16)y0;
			s16Data[2] = (AMint16)halfWidth;
			s16Data[3] = (AMint16)halfHeight;
			s16Data[4] = 0;
			s16Data[5] = (AMint16)(-width);
			s16Data[6] = 0;
			s16Data[7] = (AMint16)halfWidth;
			s16Data[8] = (AMint16)halfHeight;
			s16Data[9] = 0;
#if defined (RIM_VG_SRC)
			s16Data[10] = (AMint16)width;
			s16Data[11] = (AMint16)0;
#else
			s16Data[10] = (AMint16)x0;
			s16Data[11] = (AMint16)y1;
#endif
			res = amPathDataAppend(path, 4, commands, (const void *)s16Data, path->dataType, context, AM_FALSE);
			break;
		case VG_PATH_DATATYPE_S_32:
			s32Data[0] = (AMint32)x0;
			s32Data[1] = (AMint32)y0;
			s32Data[2] = (AMint32)halfWidth;
			s32Data[3] = (AMint32)halfHeight;
			s32Data[4] = 0;
			s32Data[5] = (AMint32)(-width);
			s32Data[6] = 0;
			s32Data[7] = (AMint32)halfWidth;
			s32Data[8] = (AMint32)halfHeight;
			s32Data[9] = 0;
#if defined (RIM_VG_SRC)
			s32Data[10] = (AMint32)width;
			s32Data[11] = (AMint32)0;
#else
			s32Data[10] = (AMint32)x0;
			s32Data[11] = (AMint32)y1;
#endif
			res = amPathDataAppend(path, 4, commands, (const void *)s32Data, path->dataType, context, AM_FALSE);
			break;
		default:
			AM_ASSERT(path->dataType == VG_PATH_DATATYPE_F);
			fData[0] = x0;
			fData[1] = y0;
			fData[2] = halfWidth;
			fData[3] = halfHeight;
			fData[4] = 0.0f;
			fData[5] = -width;
			fData[6] = 0.0f;
			fData[7] = halfWidth;
			fData[8] = halfHeight;
			fData[9] = 0.0f;
#if defined (RIM_VG_SRC)
			fData[10] = width;
			fData[11] = 0.0f;
#else
			fData[10] = x0;
			fData[11] = y1;
#endif
			res = amPathDataAppend(path, 4, commands, (const void *)fData, path->dataType, context, AM_FALSE);
			break;
	}
	return res;
}

/*!
	\brief It appends an elliptical arc to the given path, possibly along with one or two line segments, according
	to the arcType parameter. The startAngle and angleExtent parameters are given in degrees, proceeding
	counter-clockwise from the positive x axis. The arc is defined on the unit circle, then scaled by the
	width and height of the ellipse.
	\param path the destination path.
	\param x x coordinate of the arc center.
	\param y y coordinate of the arc center.
	\param width width of horizontal axis.
	\param height height of vertical axis.
	\param startAngle start angle, in degrees.
	\param angleExtent angle extent, in degrees.
	\param arcType one of the following arc types: VGU_ARC_OPEN, VGU_ARC_CHORD,	VGU_ARC_PIE.
	\param context context containing coordinate converters.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
	\pre width, height > 0.
*/
AMbool amVguArc(AMPath *path,
				const AMfloat x,
				const AMfloat y,
				const AMfloat width,
				const AMfloat height,
				const AMfloat startAngle,
				const AMfloat angleExtent,
				const VGUArcType arcType,
				AMContext *context) {

	AMfloat last, halfWidth, halfHeight, angle;
	AMint8 x8, y8;
	AMint16 x16, y16;
	AMint32 x32, y32;
	AMfloat xf, yf;
	AMbool res;

	AM_ASSERT(path);
	AM_ASSERT((AMint32)path->dataType != (AMint32)VG_PATH_DATATYPE_INVALID);
	AM_ASSERT(width > 0.0f);
	AM_ASSERT(height > 0.0f);

	context->vguAuxCommands.size = 0;
	context->vguAuxS8Data.size = 0;
	context->vguAuxS16Data.size = 0;
	context->vguAuxS32Data.size = 0;
	context->vguAuxF32Data.size = 0;

	halfWidth = width * 0.5f;
	halfHeight = height * 0.5f;
	last = startAngle + angleExtent;

	// push move_to command
	AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_MOVE_TO_ABS)

	switch (path->dataType) {

		case VG_PATH_DATATYPE_S_8:
			// push move_to coordinates
			x8 = (AMint8)(x + amCosf(amDeg2Radf(startAngle)) * halfWidth);
			y8 = (AMint8)(y + amSinf(amDeg2Radf(startAngle)) * halfHeight);
			AM_DYNARRAY_PUSH_BACK(context->vguAuxS8Data, AMint8, x8)
			AM_DYNARRAY_PUSH_BACK(context->vguAuxS8Data, AMint8, y8)
			// push all needed ellipse arcs, in the correct order
			if (angleExtent > 0.0f) {
				angle = startAngle + 180.0f;
				while (angle < last) {
					AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_SCCWARC_TO_ABS)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS8Data, AMint8, (AMint8)halfWidth)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS8Data, AMint8, (AMint8)halfHeight)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS8Data, AMint8, 0)
					x8 = (AMint8)(x + amCosf(amDeg2Radf(angle)) * halfWidth);
					y8 = (AMint8)(y + amSinf(amDeg2Radf(angle)) * halfHeight);
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS8Data, AMint8, x8)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS8Data, AMint8, y8)
					angle += 180.0f;
				}
				AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_SCCWARC_TO_ABS)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS8Data, AMint8, (AMint8)halfWidth)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS8Data, AMint8, (AMint8)halfHeight)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS8Data, AMint8, 0)
				x8 = (AMint8)(x + amCosf(amDeg2Radf(last)) * halfWidth);
				y8 = (AMint8)(y + amSinf(amDeg2Radf(last)) * halfHeight);
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS8Data, AMint8, x8)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS8Data, AMint8, y8)
			}
			else {
				angle = startAngle - 180.0f;
				while (angle > last) {
					AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_SCWARC_TO_ABS)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS8Data, AMint8, (AMint8)halfWidth)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS8Data, AMint8, (AMint8)halfHeight)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS8Data, AMint8, 0)
					x8 = (AMint8)(x + amCosf(amDeg2Radf(angle)) * halfWidth);
					y8 = (AMint8)(y + amSinf(amDeg2Radf(angle)) * halfHeight);
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS8Data, AMint8, x8)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS8Data, AMint8, y8)
					angle -= 180.0f;
				}
				AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_SCWARC_TO_ABS)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS8Data, AMint8, (AMint8)halfWidth)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS8Data, AMint8, (AMint8)halfHeight)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS8Data, AMint8, 0)
				x8 = (AMint8)(x + amCosf(amDeg2Radf(last)) * halfWidth);
				y8 = (AMint8)(y + amSinf(amDeg2Radf(last)) * halfHeight);
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS8Data, AMint8, x8)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS8Data, AMint8, y8)
			}
			if (arcType == VGU_ARC_PIE) {
				AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_LINE_TO_ABS)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS8Data, AMint8, (AMint8)x)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS8Data, AMint8, (AMint8)y)
			}
			if (arcType == VGU_ARC_PIE || arcType == VGU_ARC_CHORD) {
				AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_CLOSE_PATH)
			}

			if (context->vguAuxCommands.error || context->vguAuxS8Data.error) {
				context->vguAuxCommands.error = AM_DYNARRAY_NO_ERROR;
				context->vguAuxS8Data.error = AM_DYNARRAY_NO_ERROR;
				return AM_FALSE;
			}
			res = amPathDataAppend(path, (AMint32)context->vguAuxCommands.size, context->vguAuxCommands.data, (const void *)context->vguAuxS8Data.data, path->dataType, context, AM_FALSE);
			break;

		case VG_PATH_DATATYPE_S_16:
			// push move_to coordinates
			x16 = (AMint16)(x + amCosf(amDeg2Radf(startAngle)) * halfWidth);
			y16 = (AMint16)(y + amSinf(amDeg2Radf(startAngle)) * halfHeight);
			AM_DYNARRAY_PUSH_BACK(context->vguAuxS16Data, AMint16, x16)
			AM_DYNARRAY_PUSH_BACK(context->vguAuxS16Data, AMint16, y16)
			// push all needed ellipse arcs, in the correct order
			if (angleExtent > 0.0f) {
				angle = startAngle + 180.0f;
				while (angle < last) {
					AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_SCCWARC_TO_ABS)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS16Data, AMint16, (AMint16)halfWidth)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS16Data, AMint16, (AMint16)halfHeight)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS16Data, AMint16, 0)
					x16 = (AMint16)(x + amCosf(amDeg2Radf(angle)) * halfWidth);
					y16 = (AMint16)(y + amSinf(amDeg2Radf(angle)) * halfHeight);
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS16Data, AMint16, x16)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS16Data, AMint16, y16)
					angle += 180.0f;
				}
				AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_SCCWARC_TO_ABS)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS16Data, AMint16, (AMint16)halfWidth)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS16Data, AMint16, (AMint16)halfHeight)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS16Data, AMint16, 0)
				x16 = (AMint16)(x + amCosf(amDeg2Radf(last)) * halfWidth);
				y16 = (AMint16)(y + amSinf(amDeg2Radf(last)) * halfHeight);
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS16Data, AMint16, x16)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS16Data, AMint16, y16)
			}
			else {
				angle = startAngle - 180.0f;
				while (angle > last) {
					AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_SCWARC_TO_ABS)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS16Data, AMint16, (AMint16)halfWidth)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS16Data, AMint16, (AMint16)halfHeight)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS16Data, AMint16, 0)
					x16 = (AMint16)(x + amCosf(amDeg2Radf(angle)) * halfWidth);
					y16 = (AMint16)(y + amSinf(amDeg2Radf(angle)) * halfHeight);
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS16Data, AMint16, x16)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS16Data, AMint16, y16)
					angle -= 180.0f;
				}
				AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_SCWARC_TO_ABS)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS16Data, AMint16, (AMint16)halfWidth)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS16Data, AMint16, (AMint16)halfHeight)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS16Data, AMint16, 0)
				x16 = (AMint16)(x + amCosf(amDeg2Radf(last)) * halfWidth);
				y16 = (AMint16)(y + amSinf(amDeg2Radf(last)) * halfHeight);
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS16Data, AMint16, x16)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS16Data, AMint16, y16)
			}
			if (arcType == VGU_ARC_PIE) {
				AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_LINE_TO_ABS)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS16Data, AMint16, (AMint16)x)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS16Data, AMint16, (AMint16)y)
			}
			if (arcType == VGU_ARC_PIE || arcType == VGU_ARC_CHORD) {
				AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_CLOSE_PATH)
			}
			if (context->vguAuxCommands.error || context->vguAuxS16Data.error) {
				context->vguAuxCommands.error = AM_DYNARRAY_NO_ERROR;
				context->vguAuxS16Data.error = AM_DYNARRAY_NO_ERROR;
				return AM_FALSE;
			}
			res = amPathDataAppend(path, (AMint32)context->vguAuxCommands.size, context->vguAuxCommands.data, (const void *)context->vguAuxS16Data.data, path->dataType, context, AM_FALSE);
			break;

		case VG_PATH_DATATYPE_S_32:
			// push move_to coordinates
			x32 = (AMint32)(x + amCosf(amDeg2Radf(startAngle)) * halfWidth);
			y32 = (AMint32)(y + amSinf(amDeg2Radf(startAngle)) * halfHeight);
			AM_DYNARRAY_PUSH_BACK(context->vguAuxS32Data, AMint32, x32)
			AM_DYNARRAY_PUSH_BACK(context->vguAuxS32Data, AMint32, y32)
			// push all needed ellipse arcs, in the correct order
			if (angleExtent > 0.0f) {
				angle = startAngle + 180.0f;
				while (angle < last) {
					AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_SCCWARC_TO_ABS)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS32Data, AMint32, (AMint32)halfWidth)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS32Data, AMint32, (AMint32)halfHeight)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS32Data, AMint32, 0)
					x32 = (AMint32)(x + amCosf(amDeg2Radf(angle)) * halfWidth);
					y32 = (AMint32)(y + amSinf(amDeg2Radf(angle)) * halfHeight);
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS32Data, AMint32, x32)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS32Data, AMint32, y32)
					angle += 180.0f;
				}
				AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_SCCWARC_TO_ABS)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS32Data, AMint32, (AMint32)halfWidth)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS32Data, AMint32, (AMint32)halfHeight)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS32Data, AMint32, 0)
				x32 = (AMint32)(x + amCosf(amDeg2Radf(last)) * halfWidth);
				y32 = (AMint32)(y + amSinf(amDeg2Radf(last)) * halfHeight);
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS32Data, AMint32, x32)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS32Data, AMint32, y32)
			}
			else {
				angle = startAngle - 180.0f;
				while (angle > last) {
					AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_SCWARC_TO_ABS)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS32Data, AMint32, (AMint32)halfWidth)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS32Data, AMint32, (AMint32)halfHeight)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS32Data, AMint32, 0)
					x32 = (AMint32)(x + amCosf(amDeg2Radf(angle)) * halfWidth);
					y32 = (AMint32)(y + amSinf(amDeg2Radf(angle)) * halfHeight);
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS32Data, AMint32, x32)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxS32Data, AMint32, y32)
					angle -= 180.0f;
				}
				AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_SCWARC_TO_ABS)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS32Data, AMint32, (AMint32)halfWidth)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS32Data, AMint32, (AMint32)halfHeight)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS32Data, AMint32, 0)
				x32 = (AMint32)(x + amCosf(amDeg2Radf(last)) * halfWidth);
				y32 = (AMint32)(y + amSinf(amDeg2Radf(last)) * halfHeight);
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS32Data, AMint32, x32)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS32Data, AMint32, y32)
			}
			if (arcType == VGU_ARC_PIE) {
				AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_LINE_TO_ABS)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS32Data, AMint32, (AMint32)x)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxS32Data, AMint32, (AMint32)y)
			}
			if (arcType == VGU_ARC_PIE || arcType == VGU_ARC_CHORD) {
				AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_CLOSE_PATH)
			}
			if (context->vguAuxCommands.error || context->vguAuxS32Data.error) {
				context->vguAuxCommands.error = AM_DYNARRAY_NO_ERROR;
				context->vguAuxS32Data.error = AM_DYNARRAY_NO_ERROR;
				return AM_FALSE;
			}
			res = amPathDataAppend(path, (AMint32)context->vguAuxCommands.size, context->vguAuxCommands.data, (const void *)context->vguAuxS32Data.data, path->dataType, context, AM_FALSE);
			break;

		default:
			AM_ASSERT(path->dataType == VG_PATH_DATATYPE_F);
			// push move_to coordinates
			xf = (x + amCosf(amDeg2Radf(startAngle)) * halfWidth);
			yf = (y + amSinf(amDeg2Radf(startAngle)) * halfHeight);
			AM_DYNARRAY_PUSH_BACK(context->vguAuxF32Data, AMfloat, xf)
			AM_DYNARRAY_PUSH_BACK(context->vguAuxF32Data, AMfloat, yf)
			// push all needed ellipse arcs, in the correct order
			if (angleExtent > 0.0f) {
				angle = startAngle + 180.0f;
				while (angle < last) {
					AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_SCCWARC_TO_ABS)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxF32Data, AMfloat, halfWidth)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxF32Data, AMfloat, halfHeight)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxF32Data, AMfloat, 0.0f)
					xf = (x + amCosf(amDeg2Radf(angle)) * halfWidth);
					yf = (y + amSinf(amDeg2Radf(angle)) * halfHeight);
					AM_DYNARRAY_PUSH_BACK(context->vguAuxF32Data, AMfloat, xf)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxF32Data, AMfloat, yf)
					angle += 180.0f;
				}
				AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_SCCWARC_TO_ABS)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxF32Data, AMfloat, halfWidth)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxF32Data, AMfloat, halfHeight)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxF32Data, AMfloat, 0.0f)
				xf = (x + amCosf(amDeg2Radf(last)) * halfWidth);
				yf = (y + amSinf(amDeg2Radf(last)) * halfHeight);
				AM_DYNARRAY_PUSH_BACK(context->vguAuxF32Data, AMfloat, xf)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxF32Data, AMfloat, yf)
			}
			else {
				angle = startAngle - 180.0f;
				while (angle > last) {
					AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_SCWARC_TO_ABS)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxF32Data, AMfloat, halfWidth)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxF32Data, AMfloat, halfHeight)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxF32Data, AMfloat, 0.0f)
					xf = (x + amCosf(amDeg2Radf(angle)) * halfWidth);
					yf = (y + amSinf(amDeg2Radf(angle)) * halfHeight);
					AM_DYNARRAY_PUSH_BACK(context->vguAuxF32Data, AMfloat, xf)
					AM_DYNARRAY_PUSH_BACK(context->vguAuxF32Data, AMfloat, yf)
					angle -= 180.0f;
				}
				AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_SCWARC_TO_ABS)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxF32Data, AMfloat, halfWidth)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxF32Data, AMfloat, halfHeight)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxF32Data, AMfloat, 0.0f)
				xf = (x + amCosf(amDeg2Radf(last)) * halfWidth);
				yf = (y + amSinf(amDeg2Radf(last)) * halfHeight);
				AM_DYNARRAY_PUSH_BACK(context->vguAuxF32Data, AMfloat, xf)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxF32Data, AMfloat, yf)
			}
			if (arcType == VGU_ARC_PIE) {
				AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_LINE_TO_ABS)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxF32Data, AMfloat, x)
				AM_DYNARRAY_PUSH_BACK(context->vguAuxF32Data, AMfloat, y)
			}
			if (arcType == VGU_ARC_PIE || arcType == VGU_ARC_CHORD) {
				AM_DYNARRAY_PUSH_BACK(context->vguAuxCommands, AMuint8, VG_CLOSE_PATH)
			}
			if (context->vguAuxCommands.error || context->vguAuxF32Data.error) {
				context->vguAuxCommands.error = AM_DYNARRAY_NO_ERROR;
				context->vguAuxF32Data.error = AM_DYNARRAY_NO_ERROR;
				return AM_FALSE;
			}
			res = amPathDataAppend(path, (AMint32)context->vguAuxCommands.size, context->vguAuxCommands.data, (const void *)context->vguAuxF32Data.data, path->dataType, context, AM_FALSE);
			break;
	}

	return res;
}

/*!
	\brief It sets the entries of the given matrix to a projective transformation that maps the point (0, 0)
	to (dx0, dy0); (1, 0) to (dx1, dy1); (0, 1) to (dx2, dy2); and (1, 1) to (dx3, dy3). If no non-degenerate
	matrix satisfies the constraints, VGU_BAD_WARP_ERROR is returned and matrix is unchanged.
	\param matrix the destination matrix.
	\param dx0 x coordinate of the first input point.
	\param dy0 y coordinate of the first input point.
	\param dx1 x coordinate of the second input point.
	\param dy1 y coordinate of the second input point.
	\param dx2 x coordinate of the third input point.
	\param dy2 y coordinate of the third input point.
	\param dx3 x coordinate of the fourth input point.
	\param dy3 y coordinate of the fourth input point.
	\return AM_FALSE if no non-degenerate matrix satisfies the constraints, else AM_TRUE.
*/
AMbool amVguComputeWarpSquareToQuad(AMMatrix33f *matrix,
									const AMfloat dx0,
									const AMfloat dy0,
									const AMfloat dx1,
									const AMfloat dy1,
									const AMfloat dx2,
									const AMfloat dy2,
									const AMfloat dx3,
									const AMfloat dy3) {

	// (0, 0) --> (dx0, dy0)
	// (1, 0) --> (dx1, dy1)
	// (1, 1) --> (dx2, dy2)
	// (0, 1) --> (dx3, dy3)
	AMfloat px = dx0 - dx1 + dx2 - dx3;
	AMfloat py = dy0 - dy1 + dy2 - dy3;

	#define DET2(_a, _b, _c, _d) ((_a) * (_d) - (_b) * (_c))

	AM_ASSERT(matrix);

	if (amAbsf(px) <= AM_EPSILON_FLOAT && amAbsf(py) <= AM_EPSILON_FLOAT) {
		matrix->a[0][0] = dx1 - dx0;
		matrix->a[1][0] = dx2 - dx1;
		matrix->a[2][0] = dx0;
		matrix->a[0][1] = dy1 - dy0;
		matrix->a[1][1] = dy2 - dy1;
		matrix->a[2][1] = dy0;
		matrix->a[0][2] = 0.0f;
		matrix->a[1][2] = 0.0f;
		matrix->a[2][2] = 1.0f;
	}
	else {
		AMfloat qx1 = dx1 - dx2;
		AMfloat qx2 = dx3 - dx2;
		AMfloat qy1 = dy1 - dy2;
		AMfloat qy2 = dy3 - dy2;
		AMfloat det = DET2(qx1, qx2, qy1, qy2);

		if (amAbsf(det) <= AM_EPSILON_FLOAT)
			return AM_FALSE;

		matrix->a[0][2] = DET2(px, qx2, py, qy2) / det;
		matrix->a[1][2] = DET2(qx1, px, qy1, py) / det;
		matrix->a[2][2] = 1.0f;
		matrix->a[0][0] = dx1 - dx0 + matrix->a[0][2] * dx1;
		matrix->a[1][0] = dx3 - dx0 + matrix->a[1][2] * dx3;
		matrix->a[2][0] = dx0;
		matrix->a[0][1] = dy1 - dy0 + matrix->a[0][2] * dy1;
		matrix->a[1][1] = dy3 - dy0 + matrix->a[1][2] * dy3;
		matrix->a[2][1] = dy0;
	}
	return AM_TRUE;
	#undef DET2
}

/*!
	\brief It sets the entries of the given matrix to a projective transformation that maps the point
	(sx0, sy0) to (0, 0); (sx1, sy1) to (1, 0); (sx2, sy2) to (0, 1); and (sx3, sy3) to (1, 1). If no
	non-degenerate matrix satisfies the constraints, VGU_BAD_WARP_ERROR is returned and matrix is unchanged.
	\param matrix the destination matrix.
	\param sx0 x coordinate of the first input point.
	\param sy0 y coordinate of the first input point.
	\param sx1 x coordinate of the second input point.
	\param sy1 y coordinate of the second input point.
	\param sx2 x coordinate of the third input point.
	\param sy2 y coordinate of the third input point.
	\param sx3 x coordinate of the fourth input point.
	\param sy3 y coordinate of the fourth input point.
	\return AM_FALSE if no non-degenerate matrix satisfies the constraints, else AM_TRUE.
*/
AMbool amVguComputeWarpQuadToSquare(AMMatrix33f *matrix,
									const AMfloat sx0,
									const AMfloat sy0,
									const AMfloat sx1,
									const AMfloat sy1,
									const AMfloat sx2,
									const AMfloat sy2,
									const AMfloat sx3,
									const AMfloat sy3) {

	// (sx0, sy0) --> (0, 0)
	// (sx1, sy1) --> (1, 0)
	// (sx2, sy2) --> (1, 1)
	// (sx3, sy3) --> (0, 1)

	AMfloat det;
	AMMatrix33f RQ, invRQ;	// rect->quad transform

	AM_ASSERT(matrix);

	// first find mapping from unit uv square to xy quadrilateral
	if (!amVguComputeWarpSquareToQuad(&RQ, sx0, sy0, sx1, sy1, sx2, sy2, sx3, sy3))
		return AM_FALSE;

	if (amMatrix33fInvert(&invRQ, &det, &RQ)) {
		*matrix = invRQ;
		return AM_TRUE;
	}
	else
		return AM_FALSE;
}

/*!
	\brief It sets the entries of matrix to a projective transformation that maps the point (sx0, sy0) to
	(dx0, dy0); (sx1, sy1) to (dx1, dy1); (sx2, sy2) to (dx2, dy2); and (sx3, sy3) to (dx3, dy3). If no
	non-degenerate matrix satisfies the constraints, VGU_BAD_WARP_ERROR is returned and matrix is unchanged.
	\param matrix the destination matrix.
	\param dx0 x coordinate of the first destination point.
	\param dy0 y coordinate of the first destination point.
	\param dx1 x coordinate of the second destination point.
	\param dy1 y coordinate of the second destination point.
	\param dx2 x coordinate of the third destination point.
	\param dy2 y coordinate of the third destination point.
	\param dx3 x coordinate of the fourth destination point.
	\param dy3 y coordinate of the fourth destination point.
	\param sx0 x coordinate of the first source point.
	\param sy0 y coordinate of the first source point.
	\param sx1 x coordinate of the second source point.
	\param sy1 y coordinate of the second source point.
	\param sx2 x coordinate of the third source point.
	\param sy2 y coordinate of the third source point.
	\param sx3 x coordinate of the fourth source point.
	\param sy3 y coordinate of the fourth source point.
	\return AM_FALSE if no non-degenerate matrix satisfies the constraints, else AM_TRUE.
*/
AMbool amVguComputeWarpQuadToQuad(AMMatrix33f *matrix,
								  const AMfloat dx0,
								  const AMfloat dy0,
								  const AMfloat dx1,
								  const AMfloat dy1,
								  const AMfloat dx2,
								  const AMfloat dy2,
								  const AMfloat dx3,
								  const AMfloat dy3,
								  const AMfloat sx0,
								  const AMfloat sy0,
								  const AMfloat sx1,
								  const AMfloat sy1,
								  const AMfloat sx2,
								  const AMfloat sy2,
								  const AMfloat sx3,
								  const AMfloat sy3) {

	// (sx0, sy0) --> (dx0, dy0)
	// (sx1, sy1) --> (dx1, dy1)
	// (sx2, sy2) --> (dx2, dy2)
	// (sx3, sy3) --> (dx3, dy3)

	AMMatrix33f tmp0, tmp1;

	AM_ASSERT(matrix);

	if (!amVguComputeWarpQuadToSquare(&tmp0, sx0, sy0, sx1, sy1, sx2, sy2, sx3, sy3))
		return AM_FALSE;

	if (!amVguComputeWarpSquareToQuad(&tmp1, dx0, dy0, dx1, dy1, dx2, dy2, dx3, dy3))
		return AM_FALSE;

	AM_MATRIX33_MUL(matrix, &tmp0, &tmp1, AMMatrix33f)
	return AM_TRUE;
}


// *********************************************************************
//                        Public implementations
// *********************************************************************

/*!
	\brief It appends a line segment to the given path.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param path the destination path.
	\param x0 x coordinate of the start point.
	\param y0 y coordinate of the start point.
	\param x1 x coordinate of the end point.
	\param y1 y coordinate of the end point.
	\return VGU_NO_ERROR if the operation was successful, else a VGU error code.
*/
VGU_API_CALL VGUErrorCode vguLine(VGPath path,
								  VGfloat x0,
								  VGfloat y0,
								  VGfloat x1,
								  VGfloat y1) {

	AMPath *pth;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vguLine");
		OPENVG_RETURN(((VGUErrorCode)VG_NO_CONTEXT_ERROR))
	}

	// check if the path handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, path) != AM_PATH_HANDLE_ID) {
		AM_MEMORY_LOG("vguLine");
		OPENVG_RETURN(VGU_BAD_HANDLE_ERROR)
	}

	pth = (AMPath *)currentContext->handles->createdHandlesList.data[path];
	AM_ASSERT(pth);

	// check capabilities for append operations
	if (!(pth->capabilities & VG_PATH_CAPABILITY_APPEND_TO)) {
		AM_MEMORY_LOG("vguLine");
		OPENVG_RETURN(VGU_PATH_CAPABILITY_ERROR)
	}

	// handle NaN and Inf values
	x0 = amNanInfFix(x0);
	y0 = amNanInfFix(y0);
	x1 = amNanInfFix(x1);
	y1 = amNanInfFix(y1);

	if (!amVguLine(pth, x0, y0, x1, y1, currentContext)) {
		AM_MEMORY_LOG("vguLine");
		OPENVG_RETURN(VGU_OUT_OF_MEMORY_ERROR)
	}

	AM_MEMORY_LOG("vguLine");
	OPENVG_RETURN(VGU_NO_ERROR)
}

/*!
	\brief It appends a polyline (connected sequence of line segments) or polygon to the given path.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param path the destination path.
	\param points input polyline points.
	\param count number of points.
	\param closed VG_TRUE if the specified polyline is closed, else VG_FALSE.
	\return VGU_NO_ERROR if the operation was successful, else a VGU error code.
*/
VGU_API_CALL VGUErrorCode vguPolygon(VGPath path,
									 const VGfloat *points,
									 VGint count,
									 VGboolean closed) {

	AMPath *pth;
	AMbool pthClosed = (closed == VG_TRUE) ? AM_TRUE : AM_FALSE;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vguPolygon");
		OPENVG_RETURN(((VGUErrorCode)VG_NO_CONTEXT_ERROR))
	}

	// check if the path handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, path) != AM_PATH_HANDLE_ID) {
		AM_MEMORY_LOG("vguPolygon");
		OPENVG_RETURN(VGU_BAD_HANDLE_ERROR)
	}

	pth = (AMPath *)currentContext->handles->createdHandlesList.data[path];
	AM_ASSERT(pth);

	// check capabilities for append operations
	if (!(pth->capabilities & VG_PATH_CAPABILITY_APPEND_TO)) {
		AM_MEMORY_LOG("vguPolygon");
		OPENVG_RETURN(VGU_PATH_CAPABILITY_ERROR)
	}

	// check for illegal arguments
	if (points == NULL || !amPointerIsAligned(points, 4) || count <= 0) {
		AM_MEMORY_LOG("vguPolygon");
		OPENVG_RETURN(VGU_ILLEGAL_ARGUMENT_ERROR)
	}

	if (!amVguPolygon(pth, points, count, pthClosed, currentContext)) {
		AM_MEMORY_LOG("vguPolygon");
		OPENVG_RETURN(VGU_OUT_OF_MEMORY_ERROR)
	}

	AM_MEMORY_LOG("vguPolygon");
	OPENVG_RETURN(VGU_NO_ERROR)
}

/*!
	\brief It appends an axis-aligned rectangle with its lower-left corner at (x, y) and a given width
	and height to a path.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param path the destination path.
	\param x x coordinate of the lower-left corner.
	\param y y coordinate of the lower-left corner.
	\param width width of the rectangle.
	\param height height of the rectangle.
	\return VGU_NO_ERROR if the operation was successful, else a VGU error code.
*/
VGU_API_CALL VGUErrorCode vguRect(VGPath path,
								  VGfloat x,
								  VGfloat y,
								  VGfloat width,
								  VGfloat height) {

	AMPath *pth;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vguRect");
		OPENVG_RETURN(((VGUErrorCode)VG_NO_CONTEXT_ERROR))
	}

	// check if the path handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, path) != AM_PATH_HANDLE_ID) {
		AM_MEMORY_LOG("vguRect");
		OPENVG_RETURN(VGU_BAD_HANDLE_ERROR)
	}

	pth = (AMPath *)currentContext->handles->createdHandlesList.data[path];
	AM_ASSERT(pth);

	// check capabilities for append operations
	if (!(pth->capabilities & VG_PATH_CAPABILITY_APPEND_TO)) {
		AM_MEMORY_LOG("vguRect");
		OPENVG_RETURN(VGU_PATH_CAPABILITY_ERROR)
	}

	// handle NaN and Inf values
	x = amNanInfFix(x);
	y = amNanInfFix(y);
	width = amNanInfFix(width);
	height = amNanInfFix(height);

	// check for illegal arguments
	if (width <= 0.0f || height <= 0.0f) {
		AM_MEMORY_LOG("vguRect");
		OPENVG_RETURN(VGU_ILLEGAL_ARGUMENT_ERROR)
	}

	if (!amVguRect(pth, x, y, width, height, currentContext)) {
		AM_MEMORY_LOG("vguRect");
		OPENVG_RETURN(VGU_OUT_OF_MEMORY_ERROR)
	}

	AM_MEMORY_LOG("vguRect");
	OPENVG_RETURN(VGU_NO_ERROR);
}

/*!
	\brief It appends an axis-aligned round-cornered rectangle with the lower-left corner of its rectangular
	bounding box at (x, y) and a given width, height, arcWidth, and arcHeight to the given path.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param path the destination path.
	\param x x coordinate of the lower-left corner.
	\param y y coordinate of the lower-left corner.
	\param width width of the rectangle.
	\param height height of the rectangle.
	\param arcWidth length of the round corner horizontal axis.
	\param arcHeight length of the round corner vertical axis.
	\return VGU_NO_ERROR if the operation was successful, else a VGU error code.
*/
VGU_API_CALL VGUErrorCode vguRoundRect(VGPath path,
									   VGfloat x,
									   VGfloat y,
									   VGfloat width,
									   VGfloat height,
									   VGfloat arcWidth,
									   VGfloat arcHeight) {

	AMPath *pth;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vguRoundRect");
		OPENVG_RETURN(((VGUErrorCode)VG_NO_CONTEXT_ERROR))
	}

	// check if the path handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, path) != AM_PATH_HANDLE_ID) {
		AM_MEMORY_LOG("vguRoundRect");
		OPENVG_RETURN(VGU_BAD_HANDLE_ERROR)
	}

	pth = (AMPath *)currentContext->handles->createdHandlesList.data[path];
	AM_ASSERT(pth);

	// check capabilities for append operations
	if (!(pth->capabilities & VG_PATH_CAPABILITY_APPEND_TO)) {
		AM_MEMORY_LOG("vguRoundRect");
		OPENVG_RETURN(VGU_PATH_CAPABILITY_ERROR)
	}

	// handle NaN and Inf values
	x = amNanInfFix(x);
	y = amNanInfFix(y);
	width = amNanInfFix(width);
	height = amNanInfFix(height);
	arcWidth = amNanInfFix(arcWidth);
	arcHeight = amNanInfFix(arcHeight);

	// check for illegal arguments
	if (width <= 0.0f || height <= 0.0f) {
		AM_MEMORY_LOG("vguRoundRect");
		OPENVG_RETURN(VGU_ILLEGAL_ARGUMENT_ERROR)
	}

	if (!amVguRoundRect(pth, x, y, width, height, arcWidth, arcHeight, currentContext)) {
		AM_MEMORY_LOG("vguRoundRect");
		OPENVG_RETURN(VGU_OUT_OF_MEMORY_ERROR)
	}

	AM_MEMORY_LOG("vguRoundRect");
	OPENVG_RETURN(VGU_NO_ERROR)
}

/*!
	\brief It appends an axis-aligned ellipse to the given path. The center of the ellipse is given by (cx, cy) and
	the dimensions of the axis-aligned rectangle enclosing the ellipse are given by width and height. The
	ellipse begins at (cx + width/2, cy) and is stroked as two equal counter-clockwise arcs.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param path the destination path.
	\param cx x coordinate of the center.
	\param cy y coordinate of the center.
	\param width width of horizontal axis.
	\param height height of vertical axis.
	\return VGU_NO_ERROR if the operation was successful, else a VGU error code.
*/
VGU_API_CALL VGUErrorCode vguEllipse(VGPath path,
									 VGfloat cx,
									 VGfloat cy,
									 VGfloat width,
									 VGfloat height) {

	AMPath *pth;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vguEllipse");
		OPENVG_RETURN(((VGUErrorCode)VG_NO_CONTEXT_ERROR))
	}

	// check if the path handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, path) != AM_PATH_HANDLE_ID) {
		AM_MEMORY_LOG("vguEllipse");
		OPENVG_RETURN(VGU_BAD_HANDLE_ERROR)
	}

	pth = (AMPath *)currentContext->handles->createdHandlesList.data[path];
	AM_ASSERT(pth);

	// check capabilities for append operations
	if (!(pth->capabilities & VG_PATH_CAPABILITY_APPEND_TO)) {
		AM_MEMORY_LOG("vguEllipse");
		OPENVG_RETURN(VGU_PATH_CAPABILITY_ERROR)
	}

	// handle NaN and Inf values
	cx = amNanInfFix(cx);
	cy = amNanInfFix(cy);
	width = amNanInfFix(width);
	height = amNanInfFix(height);

	// check for illegal arguments
	if (width <= 0.0f || height <= 0.0f) {
		AM_MEMORY_LOG("vguEllipse");
		OPENVG_RETURN(VGU_ILLEGAL_ARGUMENT_ERROR)
	}

	if (!amVguEllipse(pth, cx, cy, width, height, currentContext)) {
		AM_MEMORY_LOG("vguEllipse");
		OPENVG_RETURN(VGU_OUT_OF_MEMORY_ERROR)
	}

	AM_MEMORY_LOG("vguEllipse");
	OPENVG_RETURN(VGU_NO_ERROR)
}

/*!
	\brief It appends an elliptical arc to the given path, possibly along with one or two line segments, according
	to the arcType parameter. The startAngle and angleExtent parameters are given in degrees, proceeding
	counter-clockwise from the positive x axis. The arc is defined on the unit circle, then scaled by the
	width and height of the ellipse.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param path the destination path.
	\param x x coordinate of the arc center.
	\param y y coordinate of the arc center.
	\param width width of horizontal axis.
	\param height height of vertical axis.
	\param startAngle start angle, in degrees.
	\param angleExtent angle extent, in degrees.
	\param arcType one of the following arc types: VGU_ARC_OPEN, VGU_ARC_CHORD,	VGU_ARC_PIE.
	\return VGU_NO_ERROR if the operation was successful, else a VGU error code.
*/
VGU_API_CALL VGUErrorCode vguArc(VGPath path,
								 VGfloat x,
								 VGfloat y,
								 VGfloat width,
								 VGfloat height,
								 VGfloat startAngle,
								 VGfloat angleExtent,
								 VGUArcType arcType) {

	AMPath *pth;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vguArc");
		OPENVG_RETURN(((VGUErrorCode)VG_NO_CONTEXT_ERROR))
	}

	// check if the path handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, path) != AM_PATH_HANDLE_ID) {
		AM_MEMORY_LOG("vguArc");
		OPENVG_RETURN(VGU_BAD_HANDLE_ERROR)
	}

	pth = (AMPath *)currentContext->handles->createdHandlesList.data[path];
	AM_ASSERT(pth);

	// check capabilities for append operations
	if (!(pth->capabilities & VG_PATH_CAPABILITY_APPEND_TO)) {
		AM_MEMORY_LOG("vguArc");
		OPENVG_RETURN(VGU_PATH_CAPABILITY_ERROR)
	}

	// handle NaN and Inf values
	x = amNanInfFix(x);
	y = amNanInfFix(y);
	width = amNanInfFix(width);
	height = amNanInfFix(height);
	startAngle = amNanInfFix(startAngle);
	angleExtent = amNanInfFix(angleExtent);

	// check for illegal arguments
	if (width <= 0.0f || height <= 0.0f) {
		AM_MEMORY_LOG("vguArc");
		OPENVG_RETURN(VGU_ILLEGAL_ARGUMENT_ERROR)
	}
	if (arcType != VGU_ARC_OPEN && arcType != VGU_ARC_CHORD && arcType != VGU_ARC_PIE) {
		AM_MEMORY_LOG("vguArc");
		OPENVG_RETURN(VGU_ILLEGAL_ARGUMENT_ERROR)
	}

	if (!amVguArc(pth, x, y, width, height, startAngle, angleExtent, arcType, currentContext)) {
		AM_MEMORY_LOG("vguArc");
		OPENVG_RETURN(VGU_OUT_OF_MEMORY_ERROR)
	}

	AM_MEMORY_LOG("vguArc");
	OPENVG_RETURN(VGU_NO_ERROR)
}

/*!
	\brief It sets the entries of the given matrix to a projective transformation that maps the point
	(sx0, sy0) to (0, 0); (sx1, sy1) to (1, 0); (sx2, sy2) to (0, 1); and (sx3, sy3) to (1, 1). If no
	non-degenerate matrix satisfies the constraints, VGU_BAD_WARP_ERROR is returned and matrix is unchanged.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param sx0 x coordinate of the first input point.
	\param sy0 y coordinate of the first input point.
	\param sx1 x coordinate of the second input point.
	\param sy1 y coordinate of the second input point.
	\param sx2 x coordinate of the third input point.
	\param sy2 y coordinate of the third input point.
	\param sx3 x coordinate of the fourth input point.
	\param sy3 y coordinate of the fourth input point.
	\param matrix the destination matrix.
	\return VGU_NO_ERROR if the operation was successful, else a VGU error code.
*/
VGU_API_CALL VGUErrorCode vguComputeWarpQuadToSquare(VGfloat sx0,
													 VGfloat sy0,
													 VGfloat sx1,
													 VGfloat sy1,
													 VGfloat sx2,
													 VGfloat sy2,
													 VGfloat sx3,
													 VGfloat sy3,
													 VGfloat *matrix) {

	AMMatrix33f tmp;

	// check for illegal arguments
	if (!matrix || !amPointerIsAligned(matrix, 4))
		return VGU_ILLEGAL_ARGUMENT_ERROR;

	// handle NaN and Inf values
	sx0 = amNanInfFix(sx0);
	sy0 = amNanInfFix(sy0);
	sx1 = amNanInfFix(sx1);
	sy1 = amNanInfFix(sy1);
	sx2 = amNanInfFix(sx2);
	sy2 = amNanInfFix(sy2);
	sx3 = amNanInfFix(sx3);
	sy3 = amNanInfFix(sy3);

	// we have to invert p2<-->p3 in order to match Heckbert functions
	if (amVguComputeWarpQuadToSquare(&tmp, sx0, sy0, sx1, sy1, sx3, sy3, sx2, sy2)) {
		matrix[0] = tmp.a[0][0];
		matrix[1] = tmp.a[0][1];
		matrix[2] = tmp.a[0][2];
		matrix[3] = tmp.a[1][0];
		matrix[4] = tmp.a[1][1];
		matrix[5] = tmp.a[1][2];
		matrix[6] = tmp.a[2][0];
		matrix[7] = tmp.a[2][1];
		matrix[8] = tmp.a[2][2];
		return VGU_NO_ERROR;
	}
	else {
		matrix[0] = 0.0f;
		matrix[1] = 0.0f;
		matrix[2] = 0.0f;
		matrix[3] = 0.0f;
		matrix[4] = 0.0f;
		matrix[5] = 0.0f;
		matrix[6] = 0.0f;
		matrix[7] = 0.0f;
		matrix[8] = 0.0f;
		return VGU_BAD_WARP_ERROR;
	}
}

/*!
	\brief It sets the entries of the given matrix to a projective transformation that maps the point (0, 0)
	to (dx0, dy0); (1, 0) to (dx1, dy1); (0, 1) to (dx2, dy2); and (1, 1) to (dx3, dy3). If no non-degenerate
	matrix satisfies the constraints, VGU_BAD_WARP_ERROR is returned and matrix is unchanged.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param dx0 x coordinate of the first input point.
	\param dy0 y coordinate of the first input point.
	\param dx1 x coordinate of the second input point.
	\param dy1 y coordinate of the second input point.
	\param dx2 x coordinate of the third input point.
	\param dy2 y coordinate of the third input point.
	\param dx3 x coordinate of the fourth input point.
	\param dy3 y coordinate of the fourth input point.
	\param matrix the destination matrix.
	\return VGU_NO_ERROR if the operation was successful, else a VGU error code.
*/
VGU_API_CALL VGUErrorCode vguComputeWarpSquareToQuad(VGfloat dx0,
													 VGfloat dy0,
													 VGfloat dx1,
													 VGfloat dy1,
													 VGfloat dx2,
													 VGfloat dy2,
													 VGfloat dx3,
													 VGfloat dy3,
													 VGfloat *matrix) {

	AMMatrix33f tmp;

	// check for illegal arguments
	if (!matrix || !amPointerIsAligned(matrix, 4))
		return VGU_ILLEGAL_ARGUMENT_ERROR;

	// handle NaN and Inf values
	dx0 = amNanInfFix(dx0);
	dy0 = amNanInfFix(dy0);
	dx1 = amNanInfFix(dx1);
	dy1 = amNanInfFix(dy1);
	dx2 = amNanInfFix(dx2);
	dy2 = amNanInfFix(dy2);
	dx3 = amNanInfFix(dx3);
	dy3 = amNanInfFix(dy3);

	// we have to invert p2<-->p3 in order to match Heckbert functions
	if (amVguComputeWarpSquareToQuad(&tmp, dx0, dy0, dx1, dy1, dx3, dy3, dx2, dy2)) {
		matrix[0] = tmp.a[0][0];
		matrix[1] = tmp.a[0][1];
		matrix[2] = tmp.a[0][2];
		matrix[3] = tmp.a[1][0];
		matrix[4] = tmp.a[1][1];
		matrix[5] = tmp.a[1][2];
		matrix[6] = tmp.a[2][0];
		matrix[7] = tmp.a[2][1];
		matrix[8] = tmp.a[2][2];
		return VGU_NO_ERROR;
	}
	else {
		matrix[0] = 0.0f;
		matrix[1] = 0.0f;
		matrix[2] = 0.0f;
		matrix[3] = 0.0f;
		matrix[4] = 0.0f;
		matrix[5] = 0.0f;
		matrix[6] = 0.0f;
		matrix[7] = 0.0f;
		matrix[8] = 0.0f;
		return VGU_BAD_WARP_ERROR;
	}
}

/*!
	\brief It sets the entries of matrix to a projective transformation that maps the point (sx0, sy0) to
	(dx0, dy0); (sx1, sy1) to (dx1, dy1); (sx2, sy2) to (dx2, dy2); and (sx3, sy3) to (dx3, dy3). If no
	non-degenerate matrix satisfies the constraints, VGU_BAD_WARP_ERROR is returned and matrix is unchanged.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param dx0 x coordinate of the first destination point.
	\param dy0 y coordinate of the first destination point.
	\param dx1 x coordinate of the second destination point.
	\param dy1 y coordinate of the second destination point.
	\param dx2 x coordinate of the third destination point.
	\param dy2 y coordinate of the third destination point.
	\param dx3 x coordinate of the fourth destination point.
	\param dy3 y coordinate of the fourth destination point.
	\param sx0 x coordinate of the first source point.
	\param sy0 y coordinate of the first source point.
	\param sx1 x coordinate of the second source point.
	\param sy1 y coordinate of the second source point.
	\param sx2 x coordinate of the third source point.
	\param sy2 y coordinate of the third source point.
	\param sx3 x coordinate of the fourth source point.
	\param sy3 y coordinate of the fourth source point.
	\param matrix the destination matrix.
	\return VGU_NO_ERROR if the operation was successful, else a VGU error code.
*/
VGU_API_CALL VGUErrorCode vguComputeWarpQuadToQuad(VGfloat dx0,
												   VGfloat dy0,
												   VGfloat dx1,
												   VGfloat dy1,
												   VGfloat dx2,
												   VGfloat dy2,
												   VGfloat dx3,
												   VGfloat dy3,
												   VGfloat sx0,
												   VGfloat sy0,
												   VGfloat sx1,
												   VGfloat sy1,
												   VGfloat sx2,
												   VGfloat sy2,
												   VGfloat sx3,
												   VGfloat sy3,
												   VGfloat *matrix) {

	AMMatrix33f tmp;

	// check for illegal arguments
	if (!matrix || !amPointerIsAligned(matrix, 4))
		return VGU_ILLEGAL_ARGUMENT_ERROR;

	// handle NaN and Inf values
	dx0 = amNanInfFix(dx0);
	dy0 = amNanInfFix(dy0);
	dx1 = amNanInfFix(dx1);
	dy1 = amNanInfFix(dy1);
	dx2 = amNanInfFix(dx2);
	dy2 = amNanInfFix(dy2);
	dx3 = amNanInfFix(dx3);
	dy3 = amNanInfFix(dy3);
	sx0 = amNanInfFix(sx0);
	sy0 = amNanInfFix(sy0);
	sx1 = amNanInfFix(sx1);
	sy1 = amNanInfFix(sy1);
	sx2 = amNanInfFix(sx2);
	sy2 = amNanInfFix(sy2);
	sx3 = amNanInfFix(sx3);
	sy3 = amNanInfFix(sy3);

	if (amVguComputeWarpQuadToQuad(&tmp, dx0, dy0, dx1, dy1, dx2, dy2, dx3, dy3, sx0, sy0, sx1, sy1, sx2, sy2, sx3, sy3)) {
		matrix[0] = tmp.a[0][0];
		matrix[1] = tmp.a[0][1];
		matrix[2] = tmp.a[0][2];
		matrix[3] = tmp.a[1][0];
		matrix[4] = tmp.a[1][1];
		matrix[5] = tmp.a[1][2];
		matrix[6] = tmp.a[2][0];
		matrix[7] = tmp.a[2][1];
		matrix[8] = tmp.a[2][2];
		return VGU_NO_ERROR;
	}
	else {
		matrix[0] = 0.0f;
		matrix[1] = 0.0f;
		matrix[2] = 0.0f;
		matrix[3] = 0.0f;
		matrix[4] = 0.0f;
		matrix[5] = 0.0f;
		matrix[6] = 0.0f;
		matrix[7] = 0.0f;
		matrix[8] = 0.0f;
		return VGU_BAD_WARP_ERROR;
	}
}

#if defined (RIM_VG_SRC)
#pragma pop
#endif

