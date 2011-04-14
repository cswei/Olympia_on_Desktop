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

#ifndef _VGPATH_H
#define _VGPATH_H

/*!
	\file vgpath.h
	\brief OpenVG paths, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined(AM_GLE) || defined(AM_GLS)
	#include "triangulator.h"
	#include "glcontext.h"
#else
	#include "aabox.h"
#endif
#include "vgstroke.h"

//! Validity bit flag for a path segment length.
#define AM_PATH_SEGMENT_LENGTH_VALID	1
//! CCW direction flag, for an ellipse path segment.
#define AM_PATH_SEGMENT_DIRECTION_CCW	2

//! Path segment structure.
typedef struct _AMPathSegment {
	//! Path segment identifier; one of the following values: AM_MOVE_TO_SEGMENT, AM_LINE_TO_SEGMENT, AM_BEZ2_TO_SEGMENT, AM_BEZ3_TO_SEGMENT, AM_ARC_TO_SEGMENT, AM_CLOSE_SEGMENT.
	AMuint16 id;
	//! Path segment flags; bitwise OR of values AM_PATH_SEGMENT_FLATTEN_VALID, AM_PATH_SEGMENT_LENGTH_VALID, AM_PATH_SEGMENT_DIRECTION_CCW.
	AMuint16 flags;
	//! Point of application, following control points are relative to this one.
	AMVect2f applicationPoint;
	/*!
		Control parameter values that describe the path segment.\n
		For a line segment, ctrlParams[0] is the end point.\n
		For a quadratic Bezier segment, ctrlParams[0] and ctrlParams[1] are the last two control points.
		For a cubic Bezier segment, ctrlParams[0], ctrlParams[1] and ctrlParams[2] are the last three control points.
		For an ellipse arc segment, ctrlParams[0] is the center, ctrlParams[1] are semi axes lengths,
		ctrlParams[2] are cosine and sine of rotation angle, ctrlParams[3] are start/end angles.
	*/
	AMVect2f ctrlParams[4];
	//! Path segment length.
	AMfloat length;
} AMPathSegment;

AM_DYNARRAY_DECLARE(AMPathSegmentDynArray, AMPathSegment, _AMPathSegmentDynArray)

//! Type definition of a function that coordinates from a format to another, applying scale and bias.
typedef void (*AMPathCoordinatesConverterFunction)(void* dst,
												   const void *src,
												   const AMfloat scale,
												   const AMfloat bias);
//! Type definition of a function that calculates a path segment length.
typedef AMfloat (*AMPathSegmentLengthFunction)(AMPathSegment *);
//! type definition of a function that calculate arc-length parametrization of a path segment.
typedef AMfloat (*AMPathSegmentParamFunction)(AMPathSegment *,
											  const AMfloat);
//! Type definition of a function that evaluates a path segment.
typedef void (*AMPathSegmentEvalFunction)(AMfloat *,
										  AMfloat *,
										  AMfloat *,
										  AMfloat *,
										  const AMPathSegment *,
										  const AMfloat);
//! Type definition of a function that calculates the axes-aligned bounding box of a path segment.
typedef void (*AMPathSegmentBoxEvalFunction)(AMAABox2f *,
											 const AMPathSegment *);
//! Type definition of a function that flattens a path segment.
typedef void (*AMPathSegmentFlattenFunction)(AMVect2fDynArray *,
											 const AMPathSegment *,
											 const AMFlattenParams *);
// Initialize path segment function tables.
void amPathSegmentTablesInit(void *_context);

//! Validity bit flag for fill triangles (GLE)
#define AM_FILL_TRIANGLES_INVALID		1
//! Validity bit flag for stroke triangles (GLE)
#define AM_STROKE_TRIANGLES_INVALID		2

//! Path cache slot Structure, containing polygons to fill and stroke a path.
typedef struct _AMPathCacheSlot {
	//! Lower bound of the deviation range that indicates the data validity of this cache slot.
	AMfloat deviationMin;
	//! Upper bound of the deviation range that indicates the data validity of this cache slot.
	AMfloat deviationMax;
	//! Points (floating point coordinates) that represent the polygon approximation of the path.
	AMVect2fDynArray flattenPts;
#if defined(AM_FIXED_POINT_PIPELINE)
	//! Points (fixed point coordinates) that represent the polygon approximation of the path.
	AMVect2xDynArray flattenPtsx;
#endif

	//! Number of flatten points for each path sub-polygon.
	AMInt32DynArray ptsPerContour;
	//! Closure flags (AM_TRUE for closed contours, AM_FALSE for open contours), for each path sub-polygon.
	AMBoolDynArray subPathsClosed;

	//! Number of flatten points for each path segment (used to identify flatten points where apply a real curve join).
	AMInt32DynArray ptsCountPerSegment;
#if defined(AM_GLE) || defined(AM_GLS)
	//! Fillrule of the stored fill triangles (software cache and VBO).
	VGFillRule fillRule;
	//! Fill software cache (triangles).
	AMTriangulation fillTriangles;
	//! Stroke software cache (triangles).
	AMTriangulation strokeTriangles;
	//! VBO fill triangles cache.
	AMGLTriangulation vboFillTriangles;
	//! VBO stroke triangles cache.
	AMGLTriangulation vboStrokeTriangles;
	//! Triangles validity bit flags, a bitwise OR of values AM_FILL_TRIANGLES_INVALID, AM_STROKE_TRIANGLES_INVALID.
	AMuint32 trianglesFlag;
#elif defined(AM_SRE)
#if defined(AM_FIXED_POINT_PIPELINE)
	//! Points (fixed point coordinates) that represent the polygon approximation of the path stroke (stroke software cache).
	AMVect2xDynArray strokePts;
#else
	//! Points (floating point coordinates) that represent the polygon approximation of the path stroke (stroke software cache).
	AMVect2fDynArray strokePts;
#endif
	//! Number of points for each stroke sub-polygon (stroke software cache).
	AMInt32DynArray strokePtsPerContour;
#else
	#error Unreachable point.
#endif
	//! Stroke descriptor.
	AMStrokeCacheDesc strokeDesc;
} AMPathCacheSlot;

//! Validity flag for path flattening points.
#define AM_PATH_FLATTEN_VALID	1
//! Flag that indicates that a path is made of only moveto/lineto/hlineto/vlineto/closeto commands.
#define AM_PATH_MADE_OF_LINES	2
//! Validity flag for path box.
#define AM_PATH_BOX_VALID		4

//! Path structure, used to implement OpenVG paths.
typedef struct _AMPath {
	//! VGHandle type identifier, it can be AM_PATH_HANDLE_ID or AM_INVALID_HANDLE_ID.
	AMuint16 id;
	//! It's always AM_PATH_HANDLE_ID, never changed.
	AMuint16 type;
	//! VG handle (index inside the context createdHandlesList).
	VGHandle vgHandle;
	//! Reference counter.
	AMuint32 referenceCounter;
	//! Path format.
	AMint32 format;
	//! Path datatype, for geometric coordinates (it could be S8, S16, S32 or F).
	VGPathDatatype dataType;
	//! Path scale value.
	AMfloat scale;
	//! Path bias value.
	AMfloat bias;
	//! Path capabilities.
	VGbitfield capabilities;
	//! Array of path segments commands.
	AMUint8DynArray commands;
	//! Path data, an array of path float coordinates.
	AMFloatDynArray coordinatesF;
	//! Number of coordinates; it could not be equal to coordinatesF.size because of "modify/append" capabilities.
	AMuint32 coordinatesCount;
	//! Untransformed axes-aligned bounding box.
	AMAABox2f box;
	//! Array of path segments.
	AMPathSegmentDynArray segments;
	//! Path flags, bitwise OR of values AM_PATH_FLATTEN_VALID, AM_PATH_MADE_OF_LINES, AM_PATH_BOX_VALID.
	AMuint32 flags;
	//! Min deviation of the first "original" cache slot.
	AMfloat cacheSlotBaseDeviation;
	//! Factor equal to maxDeviation / minDeviation, of each path cache slot. 
	AMfloat cacheRangeFactor;
	//! Path cache slots.
	AMPathCacheSlot cache[AM_PATH_CACHE_SLOTS_COUNT];
} AMPath;

// Initialize a given path structure.
AMbool amPathInit(AMPath *path,
				  const AMint32 pathFormat,
				  const VGPathDatatype datatype,
				  const AMfloat scale,
				  const AMfloat bias,
				  const AMint32 segmentCapacityHint,
				  const AMint32 coordCapacityHint,
				  const VGbitfield capabilities,
				  const void *_context);
// Destroy path resources.
void amPathResourcesDestroy(AMPath *path,
							void *_context);
// Destroy the specified path.
void amPathDestroy(AMPath *path,
				   void *_context);
// Append specified data to a path.
AMbool amPathDataAppend(AMPath *dstPath,
						const AMint32 numCommands,
						const AMuint8 *pathCommands,
						const void *pathCoordinates,
						const VGPathDatatype pathDataType,
						const void *_context,
						const AMbool applyScaleBias);
// Calculate path bounding box.
void amPathBounds(AMfloat *minX,
				  AMfloat *minY,
				  AMfloat *width,
				  AMfloat *height,
				  AMPath *path,
				  const void *_context);
// Flatten the given path, returning the cache slot.
AMbool amPathFlatten(AMuint32 *slotIndex,
					 AMPath *path,
					 void *_context,
					 AMbool *doneFlattening);
// Return the number of coordinates of the given path.
AMint32 amPathCoordinatesCount(const AMPath *path);
// Try to retrieve memory from dynamic arrays inside the given path.
void amPathMemoryRetrieve(AMPath *path,
						  const AMbool preserveData,
						  const void *_context);

// *********************************************************************
//                         Paths pools management
// *********************************************************************
typedef union _AMPathRef {
	struct {
	#if defined(AM_BIG_ENDIAN)
		// big endian machines (e.g. PowerPC).
		//! Index in a pool array.
		AMuint16 pool;
		//! Position inside the pool.
		AMuint16 poolIdx;
	#elif defined(AM_LITTLE_ENDIAN)
		// little endian machines (e.g. Intel).
		//! Position inside the pool.
		AMuint16 poolIdx;
		//! Index in a pool array.
		AMuint16 pool;
	#else
		#error Unreachable point, please define target machine endianess (check config.h inclusion).
	#endif
	} c;
	//! Alias to refer the whole 32bit key value.
	AMuint32 key;
} AMPathRef;

AM_DYNARRAY_DECLARE(AMPathRefDynArray, AMPathRef, _AMPathRefDynArray)

typedef struct _AMPathsPool {
	//! Paths pool.
	AMPath data[AM_PATHS_POOL_CAPACITY];
	//! Number of paths currently stored in the pool.
	AMuint32 size;
} AMPathsPool, *AMPathsPoolPtr;

AM_DYNARRAY_DECLARE(AMPathsPoolPtrDynArray, AMPathsPoolPtr, _AMPathsPoolPtrDynArray)

typedef struct _AMPathsPoolsManager {
	//! Paths pools (pointers).
	AMPathsPoolPtrDynArray pools;
	//! List of available paths, to be reused.
	AMPathRefDynArray availablePathsList;
	//! AM_TRUE if correctly initialized, else AM_FALSE.
	AMbool initialized;
} AMPathsPoolsManager;

AMbool amPathsPoolsManagerInit(AMPathsPoolsManager *pathsPoolsManager);

void amPathsPoolsManagerDestroy(AMPathsPoolsManager *pathsPoolsManager);

void amPathsPoolsAvailablesSort(AMPathsPoolsManager *pathsPoolsManager);

AMbool amPathNew(AMPath **path,
				 VGPath *handle,
				 const AMint32 pathFormat,
				 const VGPathDatatype dataType,
				 const AMfloat scale,
				 const AMfloat bias,
				 const AMint32 segmentCapacityHint,
				 const AMint32 coordCapacityHint,
				 const VGbitfield capabilities,
				 void *_context);

void amPathRemove(void *_context,
				  AMPath *path);

#endif
