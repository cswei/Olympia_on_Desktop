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
	\file vgpath.c
	\brief OpenVG paths, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "vgprimitives.h"
#include "vgpath.h"
#include "ellipse.h"
#include "bezier.h"

#if defined RIM_VG_SRC
#define VG_API_ENTRY 
#endif


//! Extract the command part from on OpenVG command (e.g. VG_MOVE_TO_ABS -> VG_MOVE_TO).
#define AM_PATH_SEGMENT_COMMAND(_command)	((_command) & 0x1E)
//! Extract the absolute/relative part from on OpenVG command (e.g. VG_MOVE_TO_ABS -> VG_ABSOLUTE).
#define AM_PATH_SEGMENT_ABSREL(_command)	((_command) & 0x01)

// number of bytes for each datatype
const AMint32 numBytesPerDatatype[4] = {
	1, 2, 4, 4
};

// number of coordinates for each command
const AMint32 numCoordsPerCommand[26] = {
	0, -1,  // closepath
	2, -1,  // moveto
	2, -1,  // lineto
	1, -1,  // hlineto
	1, -1,  // vlineto
	4, -1,  // quadto
	6, -1,  // cubicto
	2, -1,  // squadto
	4, -1,  // scubicto
	5, -1,  // sccwarc
	5, -1,  // scwarc
	5, -1,  // lccwarc
	5, -1,  // lcwarc
};

// *********************************************************************
//                         Paths pools management
// *********************************************************************
AMbool amPathsPoolsManagerInit(AMPathsPoolsManager *pathsPoolsManager) {

	AM_ASSERT(pathsPoolsManager);

	AM_DYNARRAY_PREINIT(pathsPoolsManager->pools)
	AM_DYNARRAY_PREINIT(pathsPoolsManager->availablePathsList)
	pathsPoolsManager->initialized = AM_FALSE;

	// initialize the array used to store referencies to available paths
	AM_DYNARRAY_INIT_RESERVE(pathsPoolsManager->availablePathsList, AMPathRef, 64)
	if (pathsPoolsManager->availablePathsList.error)
		return AM_FALSE;

	// allocate pools pointers
	AM_DYNARRAY_INIT(pathsPoolsManager->pools, AMPathsPoolPtr)
	if (pathsPoolsManager->pools.error) {
		AM_DYNARRAY_DESTROY(pathsPoolsManager->availablePathsList)
		return AM_FALSE;
	}

	pathsPoolsManager->pools.size = 0;
	pathsPoolsManager->initialized = AM_TRUE;
	return AM_TRUE;
}

void amPathsPoolsManagerDestroy(AMPathsPoolsManager *pathsPoolsManager) {

	AMuint32 i;

	AM_ASSERT(pathsPoolsManager);

	// if the pools manager was not previously initialized, simply exit
	if (!pathsPoolsManager->initialized)
		return;

	for (i = 0; i < pathsPoolsManager->pools.size; ++i) {

		AMPathsPool *poolPtr = pathsPoolsManager->pools.data[i];

		AM_ASSERT(poolPtr);
		amFree(poolPtr);
	}

	AM_DYNARRAY_DESTROY(pathsPoolsManager->pools)
	AM_DYNARRAY_DESTROY(pathsPoolsManager->availablePathsList)
	pathsPoolsManager->initialized = AM_FALSE;
}

void amPathsPoolsAvailablesSort(AMPathsPoolsManager *pathsPoolsManager) {

	amUint32QSort((AMuint32 *)pathsPoolsManager->availablePathsList.data, pathsPoolsManager->availablePathsList.size);
}

/*!
	\brief Clear and empty all cache slots of a given path.
	\param path path whose cache slots are to clear.
	\param deallocateMemory if AM_TRUE, the function will retrieve the maximum amount of memory from cache arrays; if AM_FALSE a simple rewind is performed.
	\param context input context containing a GL context.
*/
void amPathCacheClear(AMPath *path,
					  const AMbool deallocateMemory,
					  const AMContext *context) {

	AMuint32 i;

	AM_ASSERT(path);
#if defined(AM_SRE)
	(void)context;
#endif

	if (deallocateMemory) {

		for (i = 0; i < AM_PATH_CACHE_SLOTS_COUNT; ++i) {
			// reset deviation range
			path->cache[i].deviationMin = -1.0f;
			path->cache[i].deviationMax = -1.0f;
			// rewind dynamic array
			AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(path->cache[i].flattenPts, AMVect2f)
		#if defined(AM_FIXED_POINT_PIPELINE)
			AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(path->cache[i].flattenPtsx, AMVect2x)
		#endif
			AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(path->cache[i].ptsPerContour, AMint32)
			AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(path->cache[i].subPathsClosed, AMbool)
			AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(path->cache[i].ptsCountPerSegment, AMint32)
		#if defined(AM_GLE) || defined(AM_GLS)
			// clear fill software cache (triangles)
			path->cache[i].fillRule = VG_EVEN_ODD;
			path->cache[i].fillTriangles.isIndexed = AM_TRUE;
			AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(path->cache[i].fillTriangles.triangPoints, GL_VERTEX_TYPE)
			AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(path->cache[i].fillTriangles.ushortIndexes, AMuint16)
			path->cache[i].strokeTriangles.isIndexed = AM_TRUE;
			AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(path->cache[i].strokeTriangles.triangPoints, GL_VERTEX_TYPE)
			AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(path->cache[i].strokeTriangles.ushortIndexes, AMuint16)
			if (context->glContext.vboSupported) {
				// destroy VBO fill triangles
				amGlTriangulationDestroy(&path->cache[i].vboFillTriangles, &context->glContext);
				// destroy VBO stroke triangles
				amGlTriangulationDestroy(&path->cache[i].vboStrokeTriangles, &context->glContext);
			}
			// reset triangles validity flags
			path->cache[i].trianglesFlag = AM_FILL_TRIANGLES_INVALID | AM_STROKE_TRIANGLES_INVALID;
		#elif defined(AM_SRE)
			#if defined(AM_FIXED_POINT_PIPELINE)
				AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(path->cache[i].strokePts, AMVect2x)
			#else
				AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(path->cache[i].strokePts, AMVect2f)
			#endif
			AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(path->cache[i].strokePtsPerContour, AMint32)
		#else
			#error Unreachable point.
		#endif
			// set an invalid hash, so the next time the stroke cache must be reconstructed
			path->cache[i].strokeDesc.dashPatternSize = 0;
			path->cache[i].strokeDesc.dashPatternHash = AM_HASH_INVALID;
		}
	}
	else {
		for (i = 0; i < AM_PATH_CACHE_SLOTS_COUNT; ++i) {
			// reset deviation range
			path->cache[i].deviationMin = -1.0f;
			path->cache[i].deviationMax = -1.0f;
			// rewind dynamic array
			path->cache[i].flattenPts.size = 0;
		#if defined(AM_FIXED_POINT_PIPELINE)
			path->cache[i].flattenPtsx.size = 0;
		#endif
			path->cache[i].ptsPerContour.size = 0;
			path->cache[i].subPathsClosed.size = 0;
			path->cache[i].ptsCountPerSegment.size = 0;
		#if defined(AM_GLE) || defined(AM_GLS)
			// clear fill software cache (triangles)
			path->cache[i].fillRule = VG_EVEN_ODD;
			path->cache[i].fillTriangles.isIndexed = AM_TRUE;
			path->cache[i].fillTriangles.triangPoints.size = 0;
			path->cache[i].fillTriangles.ushortIndexes.size = 0;
			path->cache[i].strokeTriangles.isIndexed = AM_TRUE;
			path->cache[i].strokeTriangles.triangPoints.size = 0;
			path->cache[i].strokeTriangles.ushortIndexes.size = 0;
			if (context->glContext.vboSupported) {
				// destroy VBO fill triangles
				amGlTriangulationDestroy(&path->cache[i].vboFillTriangles, &context->glContext);
				// destroy VBO stroke triangles
				amGlTriangulationDestroy(&path->cache[i].vboStrokeTriangles, &context->glContext);
			}
			// reset triangles validity flags
			path->cache[i].trianglesFlag = AM_FILL_TRIANGLES_INVALID | AM_STROKE_TRIANGLES_INVALID;
		#elif defined(AM_SRE)
			path->cache[i].strokePts.size = 0;
			path->cache[i].strokePtsPerContour.size = 0;
		#else
			#error Unreachable point.
		#endif
			// set an invalid hash, so the next time the stroke cache must be reconstructed
			path->cache[i].strokeDesc.dashPatternSize = 0;
			path->cache[i].strokeDesc.dashPatternHash = AM_HASH_INVALID;
		}
	}
	path->cacheSlotBaseDeviation = -1.0f;
	path->flags &= (((AMuint32)(~0)) - AM_PATH_FLATTEN_VALID);
}

void amPathMemoryRetrieve(AMPath *path,
						  const AMbool preserveData,
						  const void *_context) {

	const AMContext *context = (const AMContext *)_context;
	AMuint32 i;

	AM_ASSERT(path);
	AM_ASSERT(path->referenceCounter > 0);

	AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(path->commands, AMuint8)
	AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(path->coordinatesF, AMfloat)
	path->coordinatesCount = path->coordinatesF.size;
	AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(path->segments, AMPathSegment)

	if (preserveData) {
		for (i = 0; i < AM_PATH_CACHE_SLOTS_COUNT; ++i) {
			AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(path->cache[i].flattenPts, AMVect2f)
		#if defined(AM_FIXED_POINT_PIPELINE)
			AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(path->cache[i].flattenPtsx, AMVect2x)
		#endif
			AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(path->cache[i].ptsPerContour, AMint32)
			AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(path->cache[i].subPathsClosed, AMbool)
			AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(path->cache[i].ptsCountPerSegment, AMint32)
		#if defined(AM_GLE) || defined(AM_GLS)
			AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(path->cache[i].fillTriangles.triangPoints, GL_VERTEX_TYPE)
			AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(path->cache[i].fillTriangles.ushortIndexes, AMuint16)
			AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(path->cache[i].strokeTriangles.triangPoints, GL_VERTEX_TYPE)
			AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(path->cache[i].strokeTriangles.ushortIndexes, AMuint16)
		#elif defined(AM_SRE)
			#if defined(AM_FIXED_POINT_PIPELINE)
				AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(path->cache[i].strokePts, AMVect2x)
			#else
				AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(path->cache[i].strokePts, AMVect2f)
			#endif
			AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(path->cache[i].strokePtsPerContour, AMint32)
		#else
			#error Unreachable point.
		#endif
		}
	}
	else
		amPathCacheClear(path, AM_TRUE, context);
}

// *********************************************************************
//                         Coordinate conversions
// *********************************************************************

/*!
	\brief Convert a path coordinate from VG_PATH_DATATYPE_S_8 to VG_PATH_DATATYPE_F, applying scale and bias.
	\param dst pointer to the destination coordinate.
	\param src pointer to the source coordinate.
	\param scale scale to apply during the conversion.
	\param bias bias to apply during the conversion.
*/
void amPathCoordinates_S8ToF(void *dst,
							 const void *src,
							 const AMfloat scale,
							 const AMfloat bias) {

	AMfloat *d = (AMfloat *)dst;
	AMint8 *s = (AMint8 *)src;

	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(scale != 0.0f);

	*d = (AMfloat)(*s) * scale + bias;
}

/*!
	\brief Convert a path coordinate from VG_PATH_DATATYPE_S_16 to VG_PATH_DATATYPE_F, applying scale and bias.
	\param dst pointer to the destination coordinate.
	\param src pointer to the source coordinate.
	\param scale scale to apply during the conversion.
	\param bias bias to apply during the conversion.
*/
void amPathCoordinates_S16ToF(void *dst,
							  const void *src,
							  const AMfloat scale,
							  const AMfloat bias) {

	AMfloat *d = (AMfloat *)dst;
	AMint16 *s = (AMint16 *)src;

	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(scale != 0.0f);

	*d = (AMfloat)(*s) * scale + bias;
}

/*!
	\brief Convert a path coordinate from VG_PATH_DATATYPE_S_32 to VG_PATH_DATATYPE_F, applying scale and bias.
	\param dst pointer to the destination coordinate.
	\param src pointer to the source coordinate.
	\param scale scale to apply during the conversion.
	\param bias bias to apply during the conversion.
*/
void amPathCoordinates_S32ToF(void *dst,
							  const void *src,
							  const AMfloat scale,
							  const AMfloat bias) {

	AMfloat *d = (AMfloat *)dst;
	AMint32 *s = (AMint32 *)src;

	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(scale != 0.0f);

	*d = (AMfloat)(*s) * scale + bias;
}

/*!
	\brief Convert a path coordinate from VG_PATH_DATATYPE_F to VG_PATH_DATATYPE_F, applying scale and bias.
	\param dst pointer to the destination coordinate.
	\param src pointer to the source coordinate.
	\param scale scale to apply during the conversion.
	\param bias bias to apply during the conversion.
*/
void amPathCoordinates_FToF(void *dst,
							const void *src,
							const AMfloat scale,
							const AMfloat bias) {

	AMfloat *d = (AMfloat *)dst;
	AMfloat *s = (AMfloat *)src;

	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(scale != 0.0f);

	*d = (*s) * scale + bias;
}

/*!
	\brief Convert a path coordinate from VG_PATH_DATATYPE_S_8 to VG_PATH_DATATYPE_S_32, applying scale and bias.
	\param dst pointer to the destination coordinate.
	\param src pointer to the source coordinate.
	\param scale scale to apply during the conversion.
	\param bias bias to apply during the conversion.
*/
void amPathCoordinates_S8ToS32(void *dst,
							   const void *src,
							   const AMfloat scale,
							   const AMfloat bias) {

	AMint32 *d = (AMint32 *)dst;
	AMint8 *s = (AMint8 *)src;

	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(scale != 0.0f);

	*d = (AMint32)((AMfloat)(*s) * scale + bias);
}

/*!
	\brief Convert a path coordinate from VG_PATH_DATATYPE_S_16 to VG_PATH_DATATYPE_S_32, applying scale and bias.
	\param dst pointer to the destination coordinate.
	\param src pointer to the source coordinate.
	\param scale scale to apply during the conversion.
	\param bias bias to apply during the conversion.
*/
void amPathCoordinates_S16ToS32(void *dst,
								const void *src,
								const AMfloat scale,
								const AMfloat bias) {

	AMint32 *d = (AMint32 *)dst;
	AMint16 *s = (AMint16 *)src;

	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(scale != 0.0f);

	*d = (AMint32)((AMfloat)(*s) * scale + bias);
}

/*!
	\brief Convert a path coordinate from VG_PATH_DATATYPE_S_32 to VG_PATH_DATATYPE_S_32, applying scale and bias.
	\param dst pointer to the destination coordinate.
	\param src pointer to the source coordinate.
	\param scale scale to apply during the conversion.
	\param bias bias to apply during the conversion.
*/
void amPathCoordinates_S32ToS32(void *dst,
								const void *src,
								const AMfloat scale,
								const AMfloat bias) {

	AMint32 *d = (AMint32 *)dst;
	AMint32 *s = (AMint32 *)src;

	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(scale != 0.0f);

	*d = (AMint32)((AMfloat)(*s) * scale + bias);
}

/*!
	\brief Convert a path coordinate from VG_PATH_DATATYPE_F to VG_PATH_DATATYPE_S_32, applying scale and bias.
	\param dst pointer to the destination coordinate.
	\param src pointer to the source coordinate.
	\param scale scale to apply during the conversion.
	\param bias bias to apply during the conversion.
*/
void amPathCoordinates_FToS32(void *dst,
							  const void *src,
							  const AMfloat scale,
							  const AMfloat bias) {

	AMint32 *d = (AMint32 *)dst;
	AMfloat *s = (AMfloat *)src;

	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(scale != 0.0f);

	*d = (AMint32)((*s) * scale + bias);
}

/*!
	\brief Convert a path coordinate from VG_PATH_DATATYPE_S_8 to VG_PATH_DATATYPE_S_16, applying scale and bias.
	\param dst pointer to the destination coordinate.
	\param src pointer to the source coordinate.
	\param scale scale to apply during the conversion.
	\param bias bias to apply during the conversion.
*/
void amPathCoordinates_S8ToS16(void *dst,
							   const void *src,
							   const AMfloat scale,
							   const AMfloat bias) {

	AMint16 *d = (VGshort *)dst;
	AMint8 *s = (AMint8 *)src;

	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(scale != 0.0f);

	*d = (AMint16)((AMfloat)(*s) * scale + bias);
}

/*!
	\brief Convert a path coordinate from VG_PATH_DATATYPE_S_16 to VG_PATH_DATATYPE_S_16, applying scale and bias.
	\param dst pointer to the destination coordinate.
	\param src pointer to the source coordinate.
	\param scale scale to apply during the conversion.
	\param bias bias to apply during the conversion.
*/
void amPathCoordinates_S16ToS16(void *dst,
								const void *src,
								const AMfloat scale,
								const AMfloat bias) {

	AMint16 *d = (AMint16 *)dst;
	AMint16 *s = (AMint16 *)src;

	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(scale != 0.0f);

	*d = (AMint16)((AMfloat)(*s) * scale + bias);
}

/*!
	\brief Convert a path coordinate from VG_PATH_DATATYPE_S_32 to VG_PATH_DATATYPE_S_16, applying scale and bias.
	\param dst pointer to the destination coordinate.
	\param src pointer to the source coordinate.
	\param scale scale to apply during the conversion.
	\param bias bias to apply during the conversion.
*/
void amPathCoordinates_S32ToS16(void *dst,
								const void *src,
								const AMfloat scale,
								const AMfloat bias) {

	AMint16 *d = (AMint16 *)dst;
	AMint32 *s = (AMint32 *)src;

	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(scale != 0.0f);

	*d = (AMint16)((AMfloat)(*s) * scale + bias);
}

/*!
	\brief Convert a path coordinate from VG_PATH_DATATYPE_F to VG_PATH_DATATYPE_S_16, applying scale and bias.
	\param dst pointer to the destination coordinate.
	\param src pointer to the source coordinate.
	\param scale scale to apply during the conversion.
	\param bias bias to apply during the conversion.
*/
void amPathCoordinates_FToS16(void *dst,
							  const void *src,
							  const AMfloat scale,
							  const AMfloat bias) {

	AMint16 *d = (AMint16 *)dst;
	AMfloat *s = (AMfloat *)src;

	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(scale != 0.0f);

	*d = (AMint16)((*s) * scale + bias);
}

/*!
	\brief Convert a path coordinate from VG_PATH_DATATYPE_S_8 to VG_PATH_DATATYPE_S_8, applying scale and bias.
	\param dst pointer to the destination coordinate.
	\param src pointer to the source coordinate.
	\param scale scale to apply during the conversion.
	\param bias bias to apply during the conversion.
*/
void amPathCoordinates_S8ToS8(void *dst,
							  const void *src,
							  const AMfloat scale,
							  const AMfloat bias) {

	AMint8 *d = (AMint8 *)dst;
	AMint8 *s = (AMint8 *)src;

	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(scale != 0.0f);

	*d = (AMint8)((AMfloat)(*s) * scale + bias);
}

/*!
	\brief Convert a path coordinate from VG_PATH_DATATYPE_S_16 to VG_PATH_DATATYPE_S_8, applying scale and bias.
	\param dst pointer to the destination coordinate.
	\param src pointer to the source coordinate.
	\param scale scale to apply during the conversion.
	\param bias bias to apply during the conversion.
*/
void amPathCoordinates_S16ToS8(void *dst,
							   const void *src,
							   const AMfloat scale,
							   const AMfloat bias) {

	AMint8 *d = (AMint8 *)dst;
	AMint16 *s = (AMint16 *)src;

	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(scale != 0.0f);

	*d = (AMint8)((AMfloat)(*s) * scale + bias);
}

/*!
	\brief Convert a path coordinate from VG_PATH_DATATYPE_S_32 to VG_PATH_DATATYPE_S_8, applying scale and bias.
	\param dst pointer to the destination coordinate.
	\param src pointer to the source coordinate.
	\param scale scale to apply during the conversion.
	\param bias bias to apply during the conversion.
*/
void amPathCoordinates_S32ToS8(void *dst,
							   const void *src,
							   const AMfloat scale,
							   const AMfloat bias) {

	AMint8 *d = (AMint8 *)dst;
	AMint32 *s = (AMint32 *)src;

	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(scale != 0.0f);

	*d = (AMint8)((AMfloat)(*s) * scale + bias);
}

/*!
	\brief Convert a path coordinate from VG_PATH_DATATYPE_F to VG_PATH_DATATYPE_S_8, applying scale and bias.
	\param dst pointer to the destination coordinate.
	\param src pointer to the source coordinate.
	\param scale scale to apply during the conversion.
	\param bias bias to apply during the conversion.
*/
void amPathCoordinates_FToS8(void *dst,
							 const void *src,
							 const AMfloat scale,
							 const AMfloat bias) {

	AMint8 *d = (AMint8 *)dst;
	AMfloat *s = (AMfloat *)src;

	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(scale != 0.0f);

	*d = (AMint8)((*s) * scale + bias);
}


/*!
	\brief Calculate the length of a move command corresponding to a given path segment.
	\param segment input path segment.
	\return the requested length.
	\pre segment identifier must be AM_MOVE_TO_SEGMENT.
*/
AMfloat amPathSegmentMoveLength(AMPathSegment *segment) {

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_MOVE_TO_SEGMENT);

	segment->length = 0.0f;
	segment->flags |= AM_PATH_SEGMENT_LENGTH_VALID;
	return segment->length;
}

/*!
	\brief Calculate the length of a close command corresponding to a given path segment.
	\param segment input path segment.
	\return the requested length.
	\pre segment identifier must be AM_CLOSE_TO_SEGMENT.
*/
AMfloat amPathSegmentCloseLength(AMPathSegment *segment) {

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_CLOSE_SEGMENT);

	segment->length = amSqrtf(AM_VECT2_SQR_LENGTH(&segment->ctrlParams[0]));
	segment->flags |= AM_PATH_SEGMENT_LENGTH_VALID;
	return segment->length;
}

/*!
	\brief Calculate the length of a line command corresponding to a given path segment.
	\param segment input path segment.
	\return the requested length.
	\pre segment identifier must be AM_LINE_TO_SEGMENT.
*/
AMfloat amPathSegmentLineLength(AMPathSegment *segment) {

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_LINE_TO_SEGMENT);

	segment->length = amSqrtf(AM_VECT2_SQR_LENGTH(&segment->ctrlParams[0]));
	segment->flags |= AM_PATH_SEGMENT_LENGTH_VALID;
	return segment->length;
}

/*!
	\brief Calculate the length of a quadratic Bezier command corresponding to a given path segment.
	\param segment input path segment.
	\return the requested length.
	\pre segment identifier must be AM_BEZ2_TO_SEGMENT.
*/
AMfloat amPathSegmentBez2Length(AMPathSegment *segment) {

	AMBezier2f c;
	AMVect2f zero;

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_BEZ2_TO_SEGMENT);

	AM_VECT2_SET(&zero, 0.0f, 0.0f)
	amBezier2fSet(&c, &zero, &segment->ctrlParams[0], &segment->ctrlParams[1]);
	segment->length = amBezier2fLen(&c, 0.0f, 1.0f);
	segment->flags |= AM_PATH_SEGMENT_LENGTH_VALID;
	return segment->length;
}

/*!
	\brief Calculate the length of a cubic Bezier command corresponding to a given path segment.
	\param segment input path segment.
	\return the requested length.
	\pre segment identifier must be AM_BEZ3_TO_SEGMENT.
*/
AMfloat amPathSegmentBez3Length(AMPathSegment *segment) {

	AMBezier3f c;
	AMVect2f zero;

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_BEZ3_TO_SEGMENT);

	AM_VECT2_SET(&zero, 0.0f, 0.0f)
	amBezier3fSet(&c, &zero, &segment->ctrlParams[0], &segment->ctrlParams[1], &segment->ctrlParams[2]);
	segment->length = amBezier3fLen(&c, 0.0f, 1.0f);
	segment->flags |= AM_PATH_SEGMENT_LENGTH_VALID;
	return segment->length;
}

/*!
	\brief Calculate the length of an ellipse arc command corresponding to a given path segment.
	\param segment input path segment.
	\return the requested length.
	\pre segment identifier must be AM_ARC_TO_SEGMENT.
*/
AMfloat amPathSegmentEllipseLength(AMPathSegment *segment) {

	AMEllipsef e;

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_ARC_TO_SEGMENT);

	e.center = segment->ctrlParams[0];
	e.xSemiAxisLength = segment->ctrlParams[1].x;
	e.ySemiAxisLength = segment->ctrlParams[1].y;
	e.cosOfsRot = segment->ctrlParams[2].x;
	e.sinOfsRot = segment->ctrlParams[2].y;
	e.startAngle = segment->ctrlParams[3].x;
	e.endAngle = segment->ctrlParams[3].y;
	e.ccw = (segment->flags & AM_PATH_SEGMENT_DIRECTION_CCW) ? AM_TRUE : AM_FALSE;
	segment->length = amEllipsefLen(&e, 0.0f, 1.0f);
	segment->flags |= AM_PATH_SEGMENT_LENGTH_VALID;
	return segment->length;
}

/*!
	\brief Calculate the arc-length parametrization of a move command corresponding to a given path segment.
	\param segment input path segment.
	\param len length at which calculate the parameter.
	\return the arc-length parametrization.
	\pre segment identifier must be AM_MOVE_TO_SEGMENT.
*/
AMfloat amPathSegmentMoveParam(AMPathSegment *segment,
							   const AMfloat len) {

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_MOVE_TO_SEGMENT);

	(void)segment;
	(void)len;

	return 0.0f;
}

/*!
	\brief Calculate the arc-length parametrization of a close command corresponding to a given path segment.
	\param segment input path segment.
	\param len length at which calculate the parameter.
	\return the arc-length parametrization.
	\pre segment identifier must be AM_CLOSE_TO_SEGMENT.
*/
AMfloat amPathSegmentCloseParam(AMPathSegment *segment,
								const AMfloat len) {

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_CLOSE_SEGMENT);

	segment->length = amSqrtf(AM_VECT2_SQR_LENGTH(&segment->ctrlParams[0]));
	segment->flags |= AM_PATH_SEGMENT_LENGTH_VALID;
	return (len / segment->length);
}

/*!
	\brief Calculate the arc-length parametrization of a line command corresponding to a given path segment.
	\param segment input path segment.
	\param len length at which calculate the parameter.
	\return the arc-length parametrization.
	\pre segment identifier must be AM_LINE_TO_SEGMENT.
*/
AMfloat amPathSegmentLineParam(AMPathSegment *segment,
							   const AMfloat len) {

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_LINE_TO_SEGMENT);

	segment->length = amSqrtf(AM_VECT2_SQR_LENGTH(&segment->ctrlParams[0]));
	segment->flags |= AM_PATH_SEGMENT_LENGTH_VALID;
	return (len / segment->length);
}

/*!
	\brief Calculate the arc-length parametrization of a quadratic Bezier command corresponding to a given path segment.
	\param segment input path segment.
	\param len length at which calculate the parameter.
	\return the arc-length parametrization.
	\pre segment identifier must be AM_BEZ2_TO_SEGMENT.
*/
AMfloat amPathSegmentBez2Param(AMPathSegment *segment,
							   const AMfloat len) {

	AMBezier2f c;
	AMVect2f zero;
	AMfloat res;

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_BEZ2_TO_SEGMENT);

	AM_VECT2_SET(&zero, 0.0f, 0.0f)
	amBezier2fSet(&c, &zero, &segment->ctrlParams[0], &segment->ctrlParams[1]);
	amBezier2fParam(&res, &c, len);
	return res;
}

/*!
	\brief Calculate the arc-length parametrization of a cubic Bezier command corresponding to a given path segment.
	\param segment input path segment.
	\param len length at which calculate the parameter.
	\return the arc-length parametrization.
	\pre segment identifier must be AM_BEZ3_TO_SEGMENT.
*/
AMfloat amPathSegmentBez3Param(AMPathSegment *segment,
							   const AMfloat len) {

	AMBezier3f c;
	AMVect2f zero;
	AMfloat res;

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_BEZ3_TO_SEGMENT);

	AM_VECT2_SET(&zero, 0.0f, 0.0f)
	amBezier3fSet(&c, &zero, &segment->ctrlParams[0], &segment->ctrlParams[1], &segment->ctrlParams[2]);
	amBezier3fParam(&res, &c, len);
	return res;
}

/*!
	\brief Calculate the arc-length parametrization of an ellipse arc command corresponding to a given path segment.
	\param segment input path segment.
	\param len length at which calculate the parameter.
	\return the arc-length parametrization.
	\pre segment identifier must be AM_ARC_TO_SEGMENT.
*/
AMfloat amPathSegmentEllipseParam(AMPathSegment *segment,
								  const AMfloat len) {

	AMEllipsef e;
	AMfloat res;

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_ARC_TO_SEGMENT);

	e.center = segment->ctrlParams[0];
	e.xSemiAxisLength = segment->ctrlParams[1].x;
	e.ySemiAxisLength = segment->ctrlParams[1].y;
	e.cosOfsRot = segment->ctrlParams[2].x;
	e.sinOfsRot = segment->ctrlParams[2].y;
	e.startAngle = segment->ctrlParams[3].x;
	e.endAngle = segment->ctrlParams[3].y;
	e.ccw = (segment->flags & AM_PATH_SEGMENT_DIRECTION_CCW) ? AM_TRUE : AM_FALSE;
	amEllipsefParam(&res, &e, len);
	return res;
}

/*!
	\brief Evaluate a move command corresponding to a given path segment.
	\param x if not NULL, output evaluated x coordinate.
	\param y if not NULL, output evaluated y coordinate.
	\param tangentX if not NULL, output evaluated tangent x coordinate.
	\param tangentY if not NULL, output evaluated tangent y coordinate.
	\param segment input path segment.
	\param param input parameter at which evaluate the segment.
	\pre segment identifier must be AM_MOVE_TO_SEGMENT.
*/
void amPathSegmentMoveEval(AMfloat *x,
						   AMfloat *y,
						   AMfloat *tangentX,
						   AMfloat *tangentY,
						   const AMPathSegment *segment,
						   const AMfloat param) {

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_MOVE_TO_SEGMENT);

	(void)param;

	if (x)
		(*x) = segment->applicationPoint.x;
	if (y)
		(*y) = segment->applicationPoint.y;
	if (tangentX)
		(*tangentX) = 0.0f;
	if (tangentY)
		(*tangentY) = 0.0f;
}

/*!
	\brief Evaluate a close command corresponding to a given path segment.
	\param x if not NULL, output evaluated x coordinate.
	\param y if not NULL, output evaluated y coordinate.
	\param tangentX if not NULL, output evaluated tangent x coordinate.
	\param tangentY if not NULL, output evaluated tangent y coordinate.
	\param segment input path segment.
	\param param input parameter at which evaluate the segment.
	\pre segment identifier must be AM_CLOSE_TO_SEGMENT.
*/
void amPathSegmentCloseEval(AMfloat *x,
							AMfloat *y,
							AMfloat *tangentX,
							AMfloat *tangentY,
							const AMPathSegment *segment,
							const AMfloat param) {

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_CLOSE_SEGMENT);

	if (x)
		(*x) = (param * segment->ctrlParams[0].x) + segment->applicationPoint.x;
	if (y)
		(*y) = (param * segment->ctrlParams[0].y) + segment->applicationPoint.y;
	if (tangentX)
		(*tangentX) = segment->ctrlParams[0].x;
	if (tangentY)
		(*tangentY) = segment->ctrlParams[0].y;

}

/*!
	\brief Evaluate a line command corresponding to a given path segment.
	\param x if not NULL, output evaluated x coordinate.
	\param y if not NULL, output evaluated y coordinate.
	\param tangentX if not NULL, output evaluated tangent x coordinate.
	\param tangentY if not NULL, output evaluated tangent y coordinate.
	\param segment input path segment.
	\param param input parameter at which evaluate the segment.
	\pre segment identifier must be AM_LINE_TO_SEGMENT.
*/
void amPathSegmentLineEval(AMfloat *x,
						   AMfloat *y,
						   AMfloat *tangentX,
						   AMfloat *tangentY,
						   const AMPathSegment *segment,
						   const AMfloat param) {

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_LINE_TO_SEGMENT);

	if (x)
		(*x) = (param * segment->ctrlParams[0].x) + segment->applicationPoint.x;
	if (y)
		(*y) = (param * segment->ctrlParams[0].y) + segment->applicationPoint.y;
	if (tangentX)
		(*tangentX) = segment->ctrlParams[0].x;
	if (tangentY)
		(*tangentY) = segment->ctrlParams[0].y;
}

/*!
	\brief Evaluate a quadratic Bezier command corresponding to a given path segment.
	\param x if not NULL, output evaluated x coordinate.
	\param y if not NULL, output evaluated y coordinate.
	\param tangentX if not NULL, output evaluated tangent x coordinate.
	\param tangentY if not NULL, output evaluated tangent y coordinate.
	\param segment input path segment.
	\param param input parameter at which evaluate the segment.
	\pre segment identifier must be AM_BEZ2_TO_SEGMENT.
*/
void amPathSegmentBez2Eval(AMfloat *x,
						   AMfloat *y,
						   AMfloat *tangentX,
						   AMfloat *tangentY,
						   const AMPathSegment *segment,
						   const AMfloat param) {

	AMBezier2f c;
	AMVect2f zero, p;

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_BEZ2_TO_SEGMENT);

	AM_VECT2_SET(&zero, 0.0f, 0.0f)
	amBezier2fSet(&c, &zero, &segment->ctrlParams[0], &segment->ctrlParams[1]);

	if (x || y) {
		amBezier2fEval(&p, &c, param);
		if (x)
			(*x) = p.x + segment->applicationPoint.x;
		if (y)
			(*y) = p.y + segment->applicationPoint.y;
	}

	if (tangentX || tangentY) {
		amBezier2fTan(&p, &c, param);
		if (tangentX)
			(*tangentX) = p.x;
		if (tangentY)
			(*tangentY) = p.y;
	}
}


/*!
	\brief Evaluate a cubic Bezier command corresponding to a given path segment.
	\param x if not NULL, output evaluated x coordinate.
	\param y if not NULL, output evaluated y coordinate.
	\param tangentX if not NULL, output evaluated tangent x coordinate.
	\param tangentY if not NULL, output evaluated tangent y coordinate.
	\param segment input path segment.
	\param param input parameter at which evaluate the segment.
	\pre segment identifier must be AM_BEZ3_TO_SEGMENT.
*/
void amPathSegmentBez3Eval(AMfloat *x,
						   AMfloat *y,
						   AMfloat *tangentX,
						   AMfloat *tangentY,
						   const AMPathSegment *segment,
						   const AMfloat param) {

	AMBezier3f c;
	AMVect2f zero, p;

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_BEZ3_TO_SEGMENT);

	AM_VECT2_SET(&zero, 0.0f, 0.0f)
	amBezier3fSet(&c, &zero, &segment->ctrlParams[0], &segment->ctrlParams[1], &segment->ctrlParams[2]);

	if (x || y) {
		amBezier3fEval(&p, &c, param);
		if (x)
			(*x) = p.x + segment->applicationPoint.x;
		if (y)
			(*y) = p.y + segment->applicationPoint.y;
	}

	if (tangentX || tangentY) {
		amBezier3fTan(&p, &c, param);
		if (tangentX)
			(*tangentX) = p.x;
		if (tangentY)
			(*tangentY) = p.y;
	}
}

/*!
	\brief Evaluate an ellipse arc command corresponding to a given path segment.
	\param x if not NULL, output evaluated x coordinate.
	\param y if not NULL, output evaluated y coordinate.
	\param tangentX if not NULL, output evaluated tangent x coordinate.
	\param tangentY if not NULL, output evaluated tangent y coordinate.
	\param segment input path segment.
	\param param input parameter at which evaluate the segment.
	\pre segment identifier must be AM_ARC_TO_SEGMENT.
*/
void amPathSegmentEllipseEval(AMfloat *x,
							  AMfloat *y,
							  AMfloat *tangentX,
							  AMfloat *tangentY,
							  const AMPathSegment *segment,
							  const AMfloat param) {

	AMEllipsef e;
	AMVect2f p;

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_ARC_TO_SEGMENT);

	e.center = segment->ctrlParams[0];
	e.xSemiAxisLength = segment->ctrlParams[1].x;
	e.ySemiAxisLength = segment->ctrlParams[1].y;
	e.cosOfsRot = segment->ctrlParams[2].x;
	e.sinOfsRot = segment->ctrlParams[2].y;
	e.startAngle = segment->ctrlParams[3].x;
	e.endAngle = segment->ctrlParams[3].y;
	e.ccw = (segment->flags & AM_PATH_SEGMENT_DIRECTION_CCW) ? AM_TRUE : AM_FALSE;

	if (x || y) {
		amEllipsefEval(&p, &e, param);
		if (x)
			(*x) = p.x + segment->applicationPoint.x;
		if (y)
			(*y) = p.y + segment->applicationPoint.y;
	}

	if (tangentX || tangentY) {
		amEllipsefTan(&p, &e, param);
		if (tangentX)
			(*tangentX) = p.x;
		if (tangentY)
			(*tangentY) = p.y;
	}
}

/*!
	\brief Calculate the axes-aligned bounding box of a move command corresponding to a given path segment.
	\param box output bounding box.
	\param segment input path segment.
	\pre segment identifier must be AM_MOVE_TO_SEGMENT.
*/
void amPathSegmentMoveBox(AMAABox2f *box,
						  const AMPathSegment *segment) {

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_MOVE_TO_SEGMENT);

	box->minPoint = box->maxPoint = segment->applicationPoint;
}

/*!
	\brief Calculate the axes-aligned bounding box of a close command corresponding to a given path segment.
	\param box output bounding box.
	\param segment input path segment.
	\pre segment identifier must be AM_CLOSE_TO_SEGMENT.
*/
void amPathSegmentCloseBox(AMAABox2f *box,
						   const AMPathSegment *segment) {

	AMVect2f p;

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_CLOSE_SEGMENT);

	AM_VECT2_ADD(&p, &segment->applicationPoint, &segment->ctrlParams[0])
	AM_AABOX2_SET(box, &p, &segment->applicationPoint)
}

/*!
	\brief Calculate the axes-aligned bounding box of a line command corresponding to a given path segment.
	\param box output bounding box.
	\param segment input path segment.
	\pre segment identifier must be AM_LINE_TO_SEGMENT.
*/
void amPathSegmentLineBox(AMAABox2f *box,
						  const AMPathSegment *segment) {

	AMVect2f p;

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_LINE_TO_SEGMENT);

	AM_VECT2_ADD(&p, &segment->applicationPoint, &segment->ctrlParams[0])
	AM_AABOX2_SET(box, &p, &segment->applicationPoint)
}

/*!
	\brief Calculate the axes-aligned bounding box of a quadratic Bezier command corresponding to a given path segment.
	\param box output bounding box.
	\param segment input path segment.
	\pre segment identifier must be AM_BEZ2_TO_SEGMENT.
*/
void amPathSegmentBez2Box(AMAABox2f *box,
						  const AMPathSegment *segment) {

	AMfloat t, den;
	AMVect2f p, q;
	AMBezier2f c;

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_BEZ2_TO_SEGMENT);

	// calculate absolute control points
	AM_VECT2_ADD(&p, &segment->applicationPoint, &segment->ctrlParams[0])
	AM_VECT2_ADD(&q, &segment->applicationPoint, &segment->ctrlParams[1])
	amBezier2fSet(&c, &segment->applicationPoint, &p, &q);

	// initialize the box with first and last control points
	AM_AABOX2_SET(box, &segment->applicationPoint, &q)
	// now try to solve the problem to find "maximum" points; imposing the first derivative to 0 for x and y, the
	// result is t = (p0 - p1) / (p0 - 2p1 + p2). We can find the t also with 0-based control points, so p0 is
	// (0, 0); in the case of p2 == 2p1 (in x or y) den results 0.0f, and it's not required to extend the box
	den = segment->ctrlParams[1].x - 2.0f * segment->ctrlParams[0].x;
	if (den != 0.0f) {
		t = -segment->ctrlParams[0].x / den;
		if (t >= 0.0f && t <= 1.0f) {
			amBezier2fEval(&p, &c, t);
			AM_AABOX2_EXTEND_TO_INCLUDE(box, &p)
		}
	}
	den = segment->ctrlParams[1].y - 2.0f * segment->ctrlParams[0].y;
	if (den != 0.0f) {
		t = -segment->ctrlParams[0].y / den;
		if (t >= 0.0f && t <= 1.0f) {
			amBezier2fEval(&p, &c, t);
			AM_AABOX2_EXTEND_TO_INCLUDE(box, &p)
		}
	}
}

/*!
	\brief Calculate the axes-aligned bounding box of a cubic Bezier command corresponding to a given path segment.
	\param box output bounding box.
	\param segment input path segment.
	\pre segment identifier must be AM_BEZ3_TO_SEGMENT.
*/
void amPathSegmentBez3Box(AMAABox2f *box,
						  const AMPathSegment *segment) {

	AMfloat t0, t1, a, b, c;
	AMVect2f p, q, r;
	AMint32 n;
	AMBezier3f bez;

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_BEZ3_TO_SEGMENT);

	// calculate absolute control points
	AM_VECT2_ADD(&p, &segment->applicationPoint, &segment->ctrlParams[0])
	AM_VECT2_ADD(&q, &segment->applicationPoint, &segment->ctrlParams[1])
	AM_VECT2_ADD(&r, &segment->applicationPoint, &segment->ctrlParams[2])
	amBezier3fSet(&bez, &segment->applicationPoint, &p, &q, &r);

	// initialize the box with first and last control points
	AM_AABOX2_SET(box, &segment->applicationPoint, &r)
	// now try to solve the problem to find "maximum" points; imposing the first derivative to 0 for x and y, the
	// result is t^2(-p0 + 3p1 - 3p2 + p3) + 2t(p0 - 2p1 + p2) + (p1 - p0) = 0. We can find the t also with
	// 0-based control points, so p0 is (0, 0)
	a = 3.0f * (segment->ctrlParams[0].x - segment->ctrlParams[1].x) + segment->ctrlParams[2].x;
	b = 2.0f * (segment->ctrlParams[1].x - 2.0f * segment->ctrlParams[0].x);
	c = segment->ctrlParams[0].x;
	n = amQuadraticFormulaf(&t0, &t1, a, b, c);
	if (n == 1) {
		if (t0 >= 0.0f && t0 <= 1.0f) {
			amBezier3fEval(&p, &bez, t0);
			AM_AABOX2_EXTEND_TO_INCLUDE(box, &p)
		}
	}
	else
	if (n == 2) {
		if (t0 >= 0.0f && t0 <= 1.0f) {
			amBezier3fEval(&p, &bez, t0);
			AM_AABOX2_EXTEND_TO_INCLUDE(box, &p)
		}
		if (t1 >= 0.0f && t1 <= 1.0f) {
			amBezier3fEval(&p, &bez, t1);
			AM_AABOX2_EXTEND_TO_INCLUDE(box, &p)
		}
	}

	a = 3.0f * (segment->ctrlParams[0].y - segment->ctrlParams[1].y) + segment->ctrlParams[2].y;
	b = 2.0f * (segment->ctrlParams[1].y - 2.0f * segment->ctrlParams[0].y);
	c = segment->ctrlParams[0].y;
	n = amQuadraticFormulaf(&t0, &t1, a, b, c);
	if (n == 1) {
		if (t0 >= 0.0f && t0 <= 1.0f) {
			amBezier3fEval(&p, &bez, t0);
			AM_AABOX2_EXTEND_TO_INCLUDE(box, &p)
		}
	}
	else
	if (n == 2) {
		if (t0 >= 0.0f && t0 <= 1.0f) {
			amBezier3fEval(&p, &bez, t0);
			AM_AABOX2_EXTEND_TO_INCLUDE(box, &p)
		}
		if (t1 >= 0.0f && t1 <= 1.0f) {
			amBezier3fEval(&p, &bez, t1);
			AM_AABOX2_EXTEND_TO_INCLUDE(box, &p)
		}
	}
}

/*!
	\brief Calculate the axes-aligned bounding box of an ellipse arc command corresponding to a given path segment.
	\param box output bounding box.
	\param segment input path segment.
	\pre segment identifier must be AM_ARC_TO_SEGMENT.
*/
void amPathSegmentEllipseBox(AMAABox2f *box,
							 const AMPathSegment *segment) {

	AMfloat t0, t, tg;
	AMVect2f p, q;
	AMEllipsef e;

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_ARC_TO_SEGMENT);

	// calculate "absolute" ellipse (it's enough to have absolute center for the "center parametrization")
	AM_VECT2_ADD(&e.center, &segment->applicationPoint, &segment->ctrlParams[0])
	e.xSemiAxisLength = segment->ctrlParams[1].x;
	e.ySemiAxisLength = segment->ctrlParams[1].y;
	e.cosOfsRot = segment->ctrlParams[2].x;
	e.sinOfsRot = segment->ctrlParams[2].y;
	e.startAngle = segment->ctrlParams[3].x;
	e.endAngle = segment->ctrlParams[3].y;
	e.ccw  = (segment->flags & AM_PATH_SEGMENT_DIRECTION_CCW) ? AM_TRUE : AM_FALSE;
	// initialize the box with first and last ellipse ends
	amEllipsefEvalByAngle(&p, &e, e.startAngle);
	amEllipsefEvalByAngle(&q, &e, e.endAngle);
	AM_AABOX2_SET(box, &p, &q)
	// now try to solve the problem to find "maximum" points; imposing the first derivative to 0 for x and y; the
	// solution is:
	// tx = -ATAN(ySemiAxisLength * TAN(OfsRot) / xSemiAxisLength) +/- PI
	// ty = ATAN(ySemiAxisLength / (xSemiAxisLength * TAN(OfsRot))) +/- PI
	tg = (e.cosOfsRot == 0.0f) ? AM_MAX_FLOAT : (e.sinOfsRot / e.cosOfsRot);
	t0 = -amAtan2f(e.ySemiAxisLength * tg, e.xSemiAxisLength);
	t = t0;
	if (amEllipsefAngleIncluded(&e, t)) {
		amEllipsefEvalByAngle(&p, &e, t);
		AM_AABOX2_EXTEND_TO_INCLUDE(box, &p)
	}
	t = t0 + AM_PI;
	if (amEllipsefAngleIncluded(&e, t)) {
		amEllipsefEvalByAngle(&p, &e, t);
		AM_AABOX2_EXTEND_TO_INCLUDE(box, &p)
	}
	t = t0 - AM_PI;
	if (amEllipsefAngleIncluded(&e, t)) {
		amEllipsefEvalByAngle(&p, &e, t);
		AM_AABOX2_EXTEND_TO_INCLUDE(box, &p)
	}
	// repeat for y coordinate
	t0 = amAtan2f(e.ySemiAxisLength, e.xSemiAxisLength * tg);
	t = t0;
	if (amEllipsefAngleIncluded(&e, t)) {
		amEllipsefEvalByAngle(&p, &e, t);
		AM_AABOX2_EXTEND_TO_INCLUDE(box, &p)
	}
	t = t0 + AM_PI;
	if (amEllipsefAngleIncluded(&e, t)) {
		amEllipsefEvalByAngle(&p, &e, t);
		AM_AABOX2_EXTEND_TO_INCLUDE(box, &p)
	}
	t = t0 - AM_PI;
	if (amEllipsefAngleIncluded(&e, t)) {
		amEllipsefEvalByAngle(&p, &e, t);
		AM_AABOX2_EXTEND_TO_INCLUDE(box, &p)
	}
}

/*!
	\brief Flatten a move command corresponding to a given path segment.
	\param points output flatten points.
	\param segment input path segment.
	\param params input flattening parameters (chordal distance and derived values).
	\pre segment identifier must be AM_MOVE_TO_SEGMENT.
*/
void amPathSegmentMoveFlatten(AMVect2fDynArray *points,
							  const AMPathSegment *segment,
							  const AMFlattenParams *params) {

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_MOVE_TO_SEGMENT);
	AM_ASSERT(points);
	AM_ASSERT(params);

	(void)points;
	(void)segment;
	(void)params;
}

/*!
	\brief Flatten a close command corresponding to a given path segment.
	\param points output flatten points.
	\param segment input path segment.
	\param params input flattening parameters (chordal distance and derived values).
	\pre segment identifier must be AM_CLOSE_TO_SEGMENT.
*/
void amPathSegmentCloseFlatten(AMVect2fDynArray *points,
							   const AMPathSegment *segment,
							   const AMFlattenParams *params) {

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_CLOSE_SEGMENT);
	AM_ASSERT(params);
	AM_ASSERT(points);

	(void)points;
	(void)segment;
	(void)params;
}

/*!
	\brief Flatten a line command corresponding to a given path segment.
	\param points output flatten points.
	\param segment input path segment.
	\param params input flattening parameters (chordal distance and derived values).
	\pre segment identifier must be AM_LINE_TO_SEGMENT.
*/
void amPathSegmentLineFlatten(AMVect2fDynArray *points,
							  const AMPathSegment *segment,
							  const AMFlattenParams *params) {

	AMVect2f p;

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_LINE_TO_SEGMENT);
	AM_ASSERT(params);
	AM_ASSERT(points);

	(void)params;

	AM_VECT2_SET(&p, 0.0f, 0.0f)
	AM_DYNARRAY_PUSH_BACK(*points, AMVect2f, p)

	// we must avoid repeated points
	if ((amAbsf(segment->ctrlParams[0].x) > AM_EPSILON_FLOAT) || (amAbsf(segment->ctrlParams[0].y) > AM_EPSILON_FLOAT)) {
		AM_DYNARRAY_PUSH_BACK(*points, AMVect2f, segment->ctrlParams[0])
	}
}

/*!
	\brief Flatten a quadratic Bezier command corresponding to a given path segment.
	\param points output flatten points.
	\param segment input path segment.
	\param params input flattening parameters (chordal distance and derived values).
	\pre segment identifier must be AM_BEZ2_TO_SEGMENT.
*/
void amPathSegmentBez2Flatten(AMVect2fDynArray *points,
							  const AMPathSegment *segment,
							  const AMFlattenParams *params) {

	AMBezier2f c;
	AMVect2f zero;

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_BEZ2_TO_SEGMENT);
	AM_ASSERT(params);
	AM_ASSERT(points);

	AM_VECT2_SET(&zero, 0.0f, 0.0f)
	amBezier2fSet(&c, &zero, &segment->ctrlParams[0], &segment->ctrlParams[1]);
	amBezier2fFlatten(points, &c, params, AM_TRUE);
}

/*!
	\brief Flatten a cubic Bezier command corresponding to a given path segment.
	\param points output flatten points.
	\param segment input path segment.
	\param params input flattening parameters (chordal distance and derived values).
	\pre segment identifier must be AM_BEZ3_TO_SEGMENT.
*/
void amPathSegmentBez3Flatten(AMVect2fDynArray *points,
							  const AMPathSegment *segment,
							  const AMFlattenParams *params) {

	AMBezier3f c;
	AMVect2f zero;

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_BEZ3_TO_SEGMENT);
	AM_ASSERT(params);
	AM_ASSERT(points);

	AM_VECT2_SET(&zero, 0.0f, 0.0f)
	amBezier3fSet(&c, &zero, &segment->ctrlParams[0], &segment->ctrlParams[1], &segment->ctrlParams[2]);
	amBezier3fFlatten(points, &c, params, AM_TRUE);
}

/*!
	\brief Flatten an ellipse arc command corresponding to a given path segment.
	\param points output flatten points.
	\param segment input path segment.
	\param params input flattening parameters (chordal distance and derived values).
	\pre segment identifier must be AM_ARC_TO_SEGMENT.
*/
void amPathSegmentEllipseFlatten(AMVect2fDynArray *points,
								 const AMPathSegment *segment,
								 const AMFlattenParams *params) {

	AMEllipsef e;

	AM_ASSERT(segment);
	AM_ASSERT(segment->id == AM_ARC_TO_SEGMENT);
	AM_ASSERT(params);
	AM_ASSERT(points);

	e.center = segment->ctrlParams[0];
	e.xSemiAxisLength = segment->ctrlParams[1].x;
	e.ySemiAxisLength = segment->ctrlParams[1].y;
	e.cosOfsRot = segment->ctrlParams[2].x;
	e.sinOfsRot = segment->ctrlParams[2].y;
	e.startAngle = segment->ctrlParams[3].x;
	e.endAngle = segment->ctrlParams[3].y;
	e.ccw = (segment->flags & AM_PATH_SEGMENT_DIRECTION_CCW) ? AM_TRUE : AM_FALSE;
	amEllipsefFlatten(points, &e, params, AM_TRUE);
}

/*!
	\brief Initialize path segment function tables.
	\param _context pointer to a AMContext structure, containing path segment function tables.
*/
void amPathSegmentTablesInit(void *_context) {

	AMContext *context = (AMContext *)_context;

	context->lengthFunctions[0] = NULL;
	context->lengthFunctions[1] = amPathSegmentMoveLength;
	context->lengthFunctions[2] = amPathSegmentLineLength;
	context->lengthFunctions[3] = amPathSegmentBez2Length;
	context->lengthFunctions[4] = amPathSegmentBez3Length;
	context->lengthFunctions[5] = amPathSegmentEllipseLength;
	context->lengthFunctions[6] = amPathSegmentCloseLength;

	context->paramFunctions[0] = NULL;
	context->paramFunctions[1] = amPathSegmentMoveParam;
	context->paramFunctions[2] = amPathSegmentLineParam;
	context->paramFunctions[3] = amPathSegmentBez2Param;
	context->paramFunctions[4] = amPathSegmentBez3Param;
	context->paramFunctions[5] = amPathSegmentEllipseParam;
	context->paramFunctions[6] = amPathSegmentCloseParam;

	context->evalFunctions[0] = NULL;
	context->evalFunctions[1] = amPathSegmentMoveEval;
	context->evalFunctions[2] = amPathSegmentLineEval;
	context->evalFunctions[3] = amPathSegmentBez2Eval;
	context->evalFunctions[4] = amPathSegmentBez3Eval;
	context->evalFunctions[5] = amPathSegmentEllipseEval;
	context->evalFunctions[6] = amPathSegmentCloseEval;

	context->boxEvalFunctions[0] = NULL;
	context->boxEvalFunctions[1] = amPathSegmentMoveBox;
	context->boxEvalFunctions[2] = amPathSegmentLineBox;
	context->boxEvalFunctions[3] = amPathSegmentBez2Box;
	context->boxEvalFunctions[4] = amPathSegmentBez3Box;
	context->boxEvalFunctions[5] = amPathSegmentEllipseBox;
	context->boxEvalFunctions[6] = amPathSegmentCloseBox;

	context->flattenFunctions[0] = NULL;
	context->flattenFunctions[1] = amPathSegmentMoveFlatten;
	context->flattenFunctions[2] = amPathSegmentLineFlatten;
	context->flattenFunctions[3] = amPathSegmentBez2Flatten;
	context->flattenFunctions[4] = amPathSegmentBez3Flatten;
	context->flattenFunctions[5] = amPathSegmentEllipseFlatten;
	context->flattenFunctions[6] = amPathSegmentCloseFlatten;
	
	context->coordinatesConverters[0][0] = amPathCoordinates_S8ToS8;
	context->coordinatesConverters[0][1] = amPathCoordinates_S16ToS8;
	context->coordinatesConverters[0][2] = amPathCoordinates_S32ToS8;
	context->coordinatesConverters[0][3] = amPathCoordinates_FToS8;

	context->coordinatesConverters[1][0] = amPathCoordinates_S8ToS16;
	context->coordinatesConverters[1][1] = amPathCoordinates_S16ToS16;
	context->coordinatesConverters[1][2] = amPathCoordinates_S32ToS16;
	context->coordinatesConverters[1][3] = amPathCoordinates_FToS16;

	context->coordinatesConverters[2][0] = amPathCoordinates_S8ToS32;
	context->coordinatesConverters[2][1] = amPathCoordinates_S16ToS32;
	context->coordinatesConverters[2][2] = amPathCoordinates_S32ToS32;
	context->coordinatesConverters[2][3] = amPathCoordinates_FToS32;

	context->coordinatesConverters[3][0] = amPathCoordinates_S8ToF;
	context->coordinatesConverters[3][1] = amPathCoordinates_S16ToF;
	context->coordinatesConverters[3][2] = amPathCoordinates_S32ToF;
	context->coordinatesConverters[3][3] = amPathCoordinates_FToF;
}

/*!
	\brief Initialize a path segment.
	\param segment path segment to initialize.
*/
void amPathSegmentInit(AMPathSegment *segment) {

	AM_ASSERT(segment);

	segment->id = AM_UNKNOWN_SEGMENT;
	segment->flags = 0;
	AM_VECT2_SET(&segment->applicationPoint, 0.0f, 0.0f)
	AM_VECT2_SET(&segment->ctrlParams[0], 0.0f, 0.0f)
	AM_VECT2_SET(&segment->ctrlParams[1], 0.0f, 0.0f)
	AM_VECT2_SET(&segment->ctrlParams[2], 0.0f, 0.0f)
	AM_VECT2_SET(&segment->ctrlParams[3], 0.0f, 0.0f)
	segment->length = -1.0f;
}

/*!
	\brief Return the number of coordinates of the given path.
	\param path path whose coordinates count is to get.
*/
AMint32 amPathCoordinatesCount(const AMPath *path) {

	AM_ASSERT(path);
	AM_ASSERT((AMint32)path->dataType != (AMint32)VG_PATH_DATATYPE_INVALID);

	return (AMint32)path->coordinatesCount;
}

/*!
	\brief Build path segments, taking care to normalize commands (e.g. VG_HLINE_TO --> AM_LINE_TO_SEGMENT), converting coordinates in absolute float values.
	\param path input/output path.
*/
void amPathSegmentsBuild(AMPath *path) {

	AMint32 i;
	AMfloat floatCoord0, floatCoord1, floatCoord2, floatCoord3, floatCoord4, floatCoord5, *floatCoords;
	AMuint8 cmd, absRel;
	AMVect2f cursor, p, lastCP, moveToPoint;
	AMPathSegment *segment;
	AMEllipsef ellipse;
	AMbool pathMadeOfLine;

#define PROCESS_COMMAND(_vgCmd, _srcCoords) \
	switch (_vgCmd) { \
		case VG_CLOSE_PATH: \
			segment->id = AM_CLOSE_SEGMENT; \
			segment->applicationPoint = cursor; \
			AM_VECT2_SUB(&segment->ctrlParams[0], &moveToPoint, &segment->applicationPoint) \
			cursor = moveToPoint; \
			segment++; \
			break; \
		case VG_MOVE_TO: \
			segment->id = AM_MOVE_TO_SEGMENT; \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			floatCoord1 = (AMfloat)(*_srcCoords++); \
			if (absRel == VG_RELATIVE) { \
				cursor.x += floatCoord0; \
				cursor.y += floatCoord1; \
			} \
			else { \
				cursor.x = floatCoord0; \
				cursor.y = floatCoord1; \
			} \
			segment->applicationPoint = cursor; \
			lastCP = moveToPoint = cursor; \
			segment++; \
			break; \
		case VG_HLINE_TO: \
			segment->id = AM_LINE_TO_SEGMENT; \
			segment->applicationPoint = cursor; \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			if (absRel == VG_RELATIVE) { \
				cursor.x += floatCoord0; \
				AM_VECT2_SET(&segment->ctrlParams[0], floatCoord0, 0.0f) \
			} \
			else { \
				cursor.x = floatCoord0; \
				AM_VECT2_SET(&segment->ctrlParams[0], floatCoord0 - segment->applicationPoint.x, 0.0f) \
			} \
			lastCP = cursor; \
			segment++; \
			break; \
		case VG_VLINE_TO: \
			segment->id = AM_LINE_TO_SEGMENT; \
			segment->applicationPoint = cursor; \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			if (absRel == VG_RELATIVE) { \
				cursor.y += floatCoord0; \
				AM_VECT2_SET(&segment->ctrlParams[0], 0.0f, floatCoord0) \
			} \
			else { \
				cursor.y = floatCoord0; \
				AM_VECT2_SET(&segment->ctrlParams[0], 0.0f, floatCoord0 - segment->applicationPoint.y) \
			} \
			lastCP = cursor; \
			segment++; \
			break; \
		case VG_LINE_TO: \
			segment->id = AM_LINE_TO_SEGMENT; \
			segment->applicationPoint = cursor; \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			floatCoord1 = (AMfloat)(*_srcCoords++); \
			if (absRel == VG_RELATIVE) { \
				AM_VECT2_SET(&segment->ctrlParams[0], floatCoord0, floatCoord1) \
				cursor.x += floatCoord0; \
				cursor.y += floatCoord1; \
			} \
			else { \
				AM_VECT2_SET(&segment->ctrlParams[0], floatCoord0 - segment->applicationPoint.x, floatCoord1 - segment->applicationPoint.y) \
				AM_VECT2_SET(&cursor, floatCoord0, floatCoord1) \
			} \
			lastCP = cursor; \
			segment++; \
			break; \
		case VG_QUAD_TO: \
			pathMadeOfLine = AM_FALSE; \
			segment->id = AM_BEZ2_TO_SEGMENT; \
			segment->applicationPoint = cursor; \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			floatCoord1 = (AMfloat)(*_srcCoords++); \
			floatCoord2 = (AMfloat)(*_srcCoords++); \
			floatCoord3 = (AMfloat)(*_srcCoords++); \
			if (absRel == VG_RELATIVE) { \
				AM_VECT2_SET(&segment->ctrlParams[0], floatCoord0, floatCoord1) \
				AM_VECT2_SET(&lastCP, cursor.x + floatCoord0, cursor.y + floatCoord1) \
				AM_VECT2_SET(&segment->ctrlParams[1], floatCoord2, floatCoord3) \
				cursor.x += floatCoord2; \
				cursor.y += floatCoord3; \
			} \
			else { \
				AM_VECT2_SET(&segment->ctrlParams[0], floatCoord0 - segment->applicationPoint.x, floatCoord1 - segment->applicationPoint.y) \
				AM_VECT2_SET(&lastCP, floatCoord0, floatCoord1) \
				AM_VECT2_SET(&segment->ctrlParams[1], floatCoord2 - segment->applicationPoint.x, floatCoord3 - segment->applicationPoint.y) \
				AM_VECT2_SET(&cursor, floatCoord2, floatCoord3) \
			} \
			segment++; \
			break; \
		case VG_SQUAD_TO: \
			pathMadeOfLine = AM_FALSE; \
			segment->id = AM_BEZ2_TO_SEGMENT; \
			segment->applicationPoint = cursor; \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			floatCoord1 = (AMfloat)(*_srcCoords++); \
			AM_VECT2_SUB(&segment->ctrlParams[0], &cursor, &lastCP) \
			AM_VECT2_ADD(&lastCP, &segment->ctrlParams[0], &cursor) \
			if (absRel == VG_RELATIVE) { \
				AM_VECT2_SET(&segment->ctrlParams[1], floatCoord0, floatCoord1) \
				cursor.x += floatCoord0; \
				cursor.y += floatCoord1; \
			} \
			else { \
				AM_VECT2_SET(&segment->ctrlParams[1], floatCoord0 - segment->applicationPoint.x, floatCoord1 - segment->applicationPoint.y) \
				AM_VECT2_SET(&cursor, floatCoord0, floatCoord1) \
			} \
			segment++; \
			break; \
		case VG_SCUBIC_TO: \
			pathMadeOfLine = AM_FALSE; \
			segment->id = AM_BEZ3_TO_SEGMENT; \
			segment->applicationPoint = cursor; \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			floatCoord1 = (AMfloat)(*_srcCoords++); \
			floatCoord2 = (AMfloat)(*_srcCoords++); \
			floatCoord3 = (AMfloat)(*_srcCoords++); \
			AM_VECT2_SUB(&segment->ctrlParams[0], &cursor, &lastCP) \
			if (absRel == VG_RELATIVE) { \
				AM_VECT2_SET(&segment->ctrlParams[1], floatCoord0, floatCoord1) \
				AM_VECT2_SET(&lastCP, floatCoord0 + cursor.x, floatCoord1 + cursor.y) \
				AM_VECT2_SET(&segment->ctrlParams[2], floatCoord2, floatCoord3) \
				cursor.x += floatCoord2; \
				cursor.y += floatCoord3; \
			} \
			else { \
				AM_VECT2_SET(&segment->ctrlParams[1], floatCoord0 - segment->applicationPoint.x, floatCoord1 - segment->applicationPoint.y) \
				AM_VECT2_SET(&lastCP, floatCoord0, floatCoord1) \
				AM_VECT2_SET(&segment->ctrlParams[2], floatCoord2 - segment->applicationPoint.x, floatCoord3 - segment->applicationPoint.y) \
				AM_VECT2_SET(&cursor, floatCoord2, floatCoord3) \
			} \
			segment++; \
			break; \
		case VG_CUBIC_TO: \
			pathMadeOfLine = AM_FALSE; \
			segment->id = AM_BEZ3_TO_SEGMENT; \
			segment->applicationPoint = cursor; \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			floatCoord1 = (AMfloat)(*_srcCoords++); \
			floatCoord2 = (AMfloat)(*_srcCoords++); \
			floatCoord3 = (AMfloat)(*_srcCoords++); \
			floatCoord4 = (AMfloat)(*_srcCoords++); \
			floatCoord5 = (AMfloat)(*_srcCoords++); \
			if (absRel == VG_RELATIVE) { \
				AM_VECT2_SET(&segment->ctrlParams[0], floatCoord0, floatCoord1) \
				AM_VECT2_SET(&segment->ctrlParams[1], floatCoord2, floatCoord3) \
				AM_VECT2_SET(&lastCP, cursor.x + floatCoord2, cursor.y + floatCoord3) \
				AM_VECT2_SET(&segment->ctrlParams[2], floatCoord4, floatCoord5) \
				cursor.x += floatCoord4; \
				cursor.y += floatCoord5; \
			} \
			else { \
				AM_VECT2_SET(&segment->ctrlParams[0], floatCoord0 - segment->applicationPoint.x, floatCoord1 - segment->applicationPoint.y) \
				AM_VECT2_SET(&segment->ctrlParams[1], floatCoord2 - segment->applicationPoint.x, floatCoord3 - segment->applicationPoint.y) \
				AM_VECT2_SET(&lastCP, floatCoord2, floatCoord3) \
				AM_VECT2_SET(&segment->ctrlParams[2], floatCoord4 - segment->applicationPoint.x, floatCoord5 - segment->applicationPoint.y) \
				AM_VECT2_SET(&cursor, floatCoord4, floatCoord5) \
			} \
			segment++; \
			break; \
		case VG_SCCWARC_TO: \
			pathMadeOfLine = AM_FALSE; \
			segment->id = AM_ARC_TO_SEGMENT; \
			segment->applicationPoint = cursor; \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			floatCoord1 = (AMfloat)(*_srcCoords++); \
			floatCoord2 = (AMfloat)(*_srcCoords++); \
			floatCoord3 = (AMfloat)(*_srcCoords++); \
			floatCoord4 = (AMfloat)(*_srcCoords++); \
			if (absRel == VG_RELATIVE) { \
				AM_VECT2_SET(&segment->ctrlParams[0], floatCoord3, floatCoord4) \
				cursor.x += floatCoord3; \
				cursor.y += floatCoord4; \
			} \
			else { \
				AM_VECT2_SET(&segment->ctrlParams[0], floatCoord3 - segment->applicationPoint.x, floatCoord4 - segment->applicationPoint.y) \
				AM_VECT2_SET(&cursor, floatCoord3, floatCoord4) \
			} \
			AM_VECT2_SET(&p, 0.0f, 0.0f) \
			amEllipsefSetByPoints(&ellipse, &p, &segment->ctrlParams[0], amAbsf(floatCoord0), amAbsf(floatCoord1), amDeg2Radf(floatCoord2), AM_FALSE, AM_TRUE); \
			segment->ctrlParams[0] = ellipse.center; \
			AM_VECT2_SET(&segment->ctrlParams[1], ellipse.xSemiAxisLength, ellipse.ySemiAxisLength) \
			AM_VECT2_SET(&segment->ctrlParams[2], ellipse.cosOfsRot, ellipse.sinOfsRot) \
			AM_VECT2_SET(&segment->ctrlParams[3], ellipse.startAngle, ellipse.endAngle); \
			segment->flags |= AM_PATH_SEGMENT_DIRECTION_CCW; \
			lastCP = cursor; \
			segment++; \
			break; \
		case VG_SCWARC_TO: \
			pathMadeOfLine = AM_FALSE; \
			segment->id = AM_ARC_TO_SEGMENT; \
			segment->applicationPoint = cursor; \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			floatCoord1 = (AMfloat)(*_srcCoords++); \
			floatCoord2 = (AMfloat)(*_srcCoords++); \
			floatCoord3 = (AMfloat)(*_srcCoords++); \
			floatCoord4 = (AMfloat)(*_srcCoords++); \
			if (absRel == VG_RELATIVE) { \
				AM_VECT2_SET(&segment->ctrlParams[0], floatCoord3, floatCoord4) \
				cursor.x += floatCoord3; \
				cursor.y += floatCoord4; \
			} \
			else { \
				AM_VECT2_SET(&segment->ctrlParams[0], floatCoord3 - segment->applicationPoint.x, floatCoord4 - segment->applicationPoint.y) \
				AM_VECT2_SET(&cursor, floatCoord3, floatCoord4) \
			} \
			AM_VECT2_SET(&p, 0.0f, 0.0f) \
			amEllipsefSetByPoints(&ellipse, &p, &segment->ctrlParams[0], amAbsf(floatCoord0), amAbsf(floatCoord1), amDeg2Radf(floatCoord2), AM_FALSE, AM_FALSE); \
			segment->ctrlParams[0] = ellipse.center; \
			AM_VECT2_SET(&segment->ctrlParams[1], ellipse.xSemiAxisLength, ellipse.ySemiAxisLength) \
			AM_VECT2_SET(&segment->ctrlParams[2], ellipse.cosOfsRot, ellipse.sinOfsRot) \
			AM_VECT2_SET(&segment->ctrlParams[3], ellipse.startAngle, ellipse.endAngle); \
			segment->flags &= ((AMuint16)(~0) - AM_PATH_SEGMENT_DIRECTION_CCW); \
			lastCP = cursor; \
			segment++; \
			break; \
		case VG_LCCWARC_TO: \
			pathMadeOfLine = AM_FALSE; \
			segment->id = AM_ARC_TO_SEGMENT; \
			segment->applicationPoint = cursor; \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			floatCoord1 = (AMfloat)(*_srcCoords++); \
			floatCoord2 = (AMfloat)(*_srcCoords++); \
			floatCoord3 = (AMfloat)(*_srcCoords++); \
			floatCoord4 = (AMfloat)(*_srcCoords++); \
			if (absRel == VG_RELATIVE) { \
				AM_VECT2_SET(&segment->ctrlParams[0], floatCoord3, floatCoord4) \
				cursor.x += floatCoord3; \
				cursor.y += floatCoord4; \
			} \
			else { \
				AM_VECT2_SET(&segment->ctrlParams[0], floatCoord3 - segment->applicationPoint.x, floatCoord4 - segment->applicationPoint.y) \
				AM_VECT2_SET(&cursor, floatCoord3, floatCoord4) \
			} \
			AM_VECT2_SET(&p, 0.0f, 0.0f) \
			amEllipsefSetByPoints(&ellipse, &p, &segment->ctrlParams[0], amAbsf(floatCoord0), amAbsf(floatCoord1), amDeg2Radf(floatCoord2), AM_TRUE, AM_TRUE); \
			segment->ctrlParams[0] = ellipse.center; \
			AM_VECT2_SET(&segment->ctrlParams[1], ellipse.xSemiAxisLength, ellipse.ySemiAxisLength) \
			AM_VECT2_SET(&segment->ctrlParams[2], ellipse.cosOfsRot, ellipse.sinOfsRot) \
			AM_VECT2_SET(&segment->ctrlParams[3], ellipse.startAngle, ellipse.endAngle); \
			segment->flags |= AM_PATH_SEGMENT_DIRECTION_CCW; \
			lastCP = cursor; \
			segment++; \
			break; \
		case VG_LCWARC_TO: \
			pathMadeOfLine = AM_FALSE; \
			segment->id = AM_ARC_TO_SEGMENT; \
			segment->applicationPoint = cursor; \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			floatCoord1 = (AMfloat)(*_srcCoords++); \
			floatCoord2 = (AMfloat)(*_srcCoords++); \
			floatCoord3 = (AMfloat)(*_srcCoords++); \
			floatCoord4 = (AMfloat)(*_srcCoords++); \
			if (absRel == VG_RELATIVE) { \
				AM_VECT2_SET(&segment->ctrlParams[0], floatCoord3, floatCoord4) \
				cursor.x += floatCoord3; \
				cursor.y += floatCoord4; \
			} \
			else { \
				AM_VECT2_SET(&segment->ctrlParams[0], floatCoord3 - segment->applicationPoint.x, floatCoord4 - segment->applicationPoint.y) \
				AM_VECT2_SET(&cursor, floatCoord3, floatCoord4) \
			} \
			AM_VECT2_SET(&p, 0.0f, 0.0f) \
			amEllipsefSetByPoints(&ellipse, &p, &segment->ctrlParams[0], amAbsf(floatCoord0), amAbsf(floatCoord1), amDeg2Radf(floatCoord2), AM_TRUE, AM_FALSE); \
			segment->ctrlParams[0] = ellipse.center; \
			AM_VECT2_SET(&segment->ctrlParams[1], ellipse.xSemiAxisLength, ellipse.ySemiAxisLength) \
			AM_VECT2_SET(&segment->ctrlParams[2], ellipse.cosOfsRot, ellipse.sinOfsRot) \
			AM_VECT2_SET(&segment->ctrlParams[3], ellipse.startAngle, ellipse.endAngle); \
			segment->flags &= ((AMuint16)(~0) - AM_PATH_SEGMENT_DIRECTION_CCW); \
			lastCP = cursor; \
			segment++; \
			break; \
		default: \
			break; \
	}

	AM_ASSERT(path);
	pathMadeOfLine = AM_TRUE;
	AM_VECT2_SET(&cursor, 0.0f, 0.0f)
	AM_VECT2_SET(&moveToPoint, 0.0f, 0.0f)
	AM_VECT2_SET(&lastCP, 0.0f, 0.0f)
	segment = path->segments.data;

	floatCoords = path->coordinatesF.data;
	for (i = 0; i < (AMint32)path->commands.size; ++i) {
		cmd = AM_PATH_SEGMENT_COMMAND(path->commands.data[i]);
		absRel = AM_PATH_SEGMENT_ABSREL(path->commands.data[i]);
		PROCESS_COMMAND(cmd, floatCoords)
	}

	// sign the internal flag for a path made of only moveto/lineto/hlineto/vlineto/closeto commands
	if (pathMadeOfLine)
		path->flags |= AM_PATH_MADE_OF_LINES;
	else
		path->flags &= (((AMuint32)(~0)) - AM_PATH_MADE_OF_LINES);

	AM_ASSERT(path->commands.size == path->segments.size);

	#undef PROCESS_COMMAND
}

/*!
	\brief Normalize path commands (e.g. VG_HLINE_TO --> AM_LINE_TO_SEGMENT), converting coordinates in
	absolute float values.
	\param dstPath destination path.
	\param srcPath source path to normalize.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
*/
AMbool amPathNormalize(AMPath *dstPath,
					 const AMPath *srcPath) {

	AMint32 i;
	AMfloat floatCoord0, floatCoord1, floatCoord2, floatCoord3, floatCoord4, floatCoord5;
	AMfloat *floatCoords, *dstCoord;
	AMuint8 cmd, absRel, *dstCommand;
	AMVect2f cursor, lastCP, p;

#define PROCESS_COMMAND(_vgCmd, _srcCoords) \
	switch (_vgCmd) { \
		case VG_CLOSE_PATH: \
			*dstCommand++ = VG_CLOSE_PATH; \
			break; \
		case VG_MOVE_TO: \
			*dstCommand++ = VG_MOVE_TO_ABS; \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			floatCoord1 = (AMfloat)(*_srcCoords++); \
			if (absRel == VG_RELATIVE) { \
				cursor.x += floatCoord0; \
				cursor.y += floatCoord1; \
			} \
			else { \
				AM_VECT2_SET(&cursor, floatCoord0, floatCoord1) \
			} \
			*dstCoord++ = cursor.x; \
			*dstCoord++ = cursor.y; \
			lastCP = cursor; \
			break; \
		case VG_HLINE_TO: \
			*dstCommand++ = VG_LINE_TO_ABS; \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			if (absRel == VG_RELATIVE)\
				cursor.x += floatCoord0; \
			else \
				cursor.x = floatCoord0; \
			*dstCoord++ = cursor.x; \
			*dstCoord++ = cursor.y; \
			lastCP = cursor; \
			break; \
		case VG_VLINE_TO: \
			*dstCommand++ = VG_LINE_TO_ABS; \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			if (absRel == VG_RELATIVE) \
				cursor.y += floatCoord0; \
			else \
				cursor.y = floatCoord0; \
			*dstCoord++ = cursor.x; \
			*dstCoord++ = cursor.y; \
			lastCP = cursor; \
			break; \
		case VG_LINE_TO: \
			*dstCommand++ = VG_LINE_TO_ABS; \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			floatCoord1 = (AMfloat)(*_srcCoords++); \
			if (absRel == VG_RELATIVE) { \
				cursor.x += floatCoord0; \
				cursor.y += floatCoord1; \
			} \
			else { \
				AM_VECT2_SET(&cursor, floatCoord0, floatCoord1) \
			} \
			*dstCoord++ = cursor.x; \
			*dstCoord++ = cursor.y; \
			lastCP = cursor; \
			break; \
		case VG_QUAD_TO: \
			*dstCommand++ = VG_QUAD_TO_ABS; \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			floatCoord1 = (AMfloat)(*_srcCoords++); \
			floatCoord2 = (AMfloat)(*_srcCoords++); \
			floatCoord3 = (AMfloat)(*_srcCoords++); \
			if (absRel == VG_RELATIVE) { \
				AM_VECT2_SET(&lastCP, cursor.x + floatCoord0, cursor.y + floatCoord1) \
				cursor.x += floatCoord2; \
				cursor.y += floatCoord3; \
			} \
			else { \
				AM_VECT2_SET(&lastCP, floatCoord0, floatCoord1) \
				AM_VECT2_SET(&cursor, floatCoord2, floatCoord3) \
			} \
			*dstCoord++ = lastCP.x; \
			*dstCoord++ = lastCP.y; \
			*dstCoord++ = cursor.x; \
			*dstCoord++ = cursor.y; \
			break; \
		case VG_SQUAD_TO: \
			*dstCommand++ = VG_QUAD_TO_ABS; \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			floatCoord1 = (AMfloat)(*_srcCoords++); \
			AM_VECT2_REFLECT(&lastCP, &lastCP, &cursor) \
			*dstCoord++ = lastCP.x; \
			*dstCoord++ = lastCP.y; \
			if (absRel == VG_RELATIVE) { \
				cursor.x += floatCoord0; \
				cursor.y += floatCoord1; \
			} \
			else { \
				AM_VECT2_SET(&cursor, floatCoord0, floatCoord1) \
			} \
			*dstCoord++ = cursor.x; \
			*dstCoord++ = cursor.y; \
			break; \
		case VG_CUBIC_TO: \
			*dstCommand++ = VG_CUBIC_TO_ABS; \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			floatCoord1 = (AMfloat)(*_srcCoords++); \
			floatCoord2 = (AMfloat)(*_srcCoords++); \
			floatCoord3 = (AMfloat)(*_srcCoords++); \
			floatCoord4 = (AMfloat)(*_srcCoords++); \
			floatCoord5 = (AMfloat)(*_srcCoords++); \
			if (absRel == VG_RELATIVE) { \
				AM_VECT2_SET(&p, cursor.x + floatCoord0, cursor.y + floatCoord1) \
				AM_VECT2_SET(&lastCP, cursor.x + floatCoord2, cursor.y + floatCoord3) \
				cursor.x += floatCoord4; \
				cursor.y += floatCoord5; \
			} \
			else { \
				AM_VECT2_SET(&p, floatCoord0, floatCoord1) \
				AM_VECT2_SET(&lastCP, floatCoord2, floatCoord3) \
				AM_VECT2_SET(&cursor, floatCoord4, floatCoord5) \
			} \
			*dstCoord++ = p.x; \
			*dstCoord++ = p.y; \
			*dstCoord++ = lastCP.x; \
			*dstCoord++ = lastCP.y; \
			*dstCoord++ = cursor.x; \
			*dstCoord++ = cursor.y; \
			break; \
		case VG_SCUBIC_TO: \
			*dstCommand++ = VG_CUBIC_TO_ABS; \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			floatCoord1 = (AMfloat)(*_srcCoords++); \
			floatCoord2 = (AMfloat)(*_srcCoords++); \
			floatCoord3 = (AMfloat)(*_srcCoords++); \
			AM_VECT2_REFLECT(&p, &lastCP, &cursor) \
			*dstCoord++ = p.x; \
			*dstCoord++ = p.y; \
			if (absRel == VG_RELATIVE) { \
				AM_VECT2_SET(&lastCP, cursor.x + floatCoord0, cursor.y + floatCoord1) \
				cursor.x += floatCoord2; \
				cursor.y += floatCoord3; \
			} \
			else { \
				AM_VECT2_SET(&lastCP, floatCoord0, floatCoord1) \
				AM_VECT2_SET(&cursor, floatCoord2, floatCoord3) \
			} \
			*dstCoord++ = lastCP.x; \
			*dstCoord++ = lastCP.y; \
			*dstCoord++ = cursor.x; \
			*dstCoord++ = cursor.y; \
			break; \
		case VG_SCCWARC_TO: \
		case VG_SCWARC_TO: \
		case VG_LCCWARC_TO: \
		case VG_LCWARC_TO: \
			*dstCommand++ = (AMuint8)(((_vgCmd) & 0xFE) | VG_ABSOLUTE); \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			floatCoord1 = (AMfloat)(*_srcCoords++); \
			floatCoord2 = (AMfloat)(*_srcCoords++); \
			floatCoord3 = (AMfloat)(*_srcCoords++); \
			floatCoord4 = (AMfloat)(*_srcCoords++); \
			if (absRel == VG_RELATIVE) { \
				cursor.x += floatCoord3; \
				cursor.y += floatCoord4; \
			} \
			else { \
				AM_VECT2_SET(&cursor, floatCoord3, floatCoord4) \
			} \
			*dstCoord++ = floatCoord0; \
			*dstCoord++ = floatCoord1; \
			*dstCoord++ = floatCoord2; \
			*dstCoord++ = cursor.x; \
			*dstCoord++ = cursor.y; \
			lastCP = cursor; \
			break; \
		default: \
			break; \
	}

	AM_ASSERT(srcPath);
	AM_ASSERT(dstPath);
	AM_ASSERT(dstPath->commands.size == 0);
	AM_ASSERT(dstPath->coordinatesF.size == 0);
	AM_ASSERT(dstPath->dataType == VG_PATH_DATATYPE_F);

	// we must allocate, for each command, the maximum size for coordinates (6 coordinates for cubic Bezier)
	AM_DYNARRAY_RESERVE(dstPath->commands, AMuint8, srcPath->commands.size)
	if (dstPath->commands.error) {
		dstPath->commands.error = AM_DYNARRAY_NO_ERROR;
		return AM_FALSE;
	}
	AM_DYNARRAY_CLEAR_RESERVE(dstPath->coordinatesF, AMfloat, (srcPath->commands.size * 6))
	if (dstPath->coordinatesF.error) {
		dstPath->coordinatesF.error = AM_DYNARRAY_NO_ERROR;
		return AM_FALSE;
	}

	AM_VECT2_SET(&cursor, 0.0f, 0.0f)
	AM_VECT2_SET(&lastCP, 0.0f, 0.0f)
	dstCommand = dstPath->commands.data;
	dstCoord = dstPath->coordinatesF.data;
	floatCoords = srcPath->coordinatesF.data;
	for (i = 0; i < (AMint32)srcPath->commands.size; ++i) {
		cmd = AM_PATH_SEGMENT_COMMAND(srcPath->commands.data[i]);
		absRel = AM_PATH_SEGMENT_ABSREL(srcPath->commands.data[i]);
		PROCESS_COMMAND(cmd, floatCoords)
	}
	// We have used AM_DYNARRAY_CLEAR_RESERVE (with an upper bound for memory allocation) to avoid
	// to reallocate and to not use PUSH_BACK macro that is "slow" (it checks some bound conditions).
	// We have to know the exact number of coordinates, we can derive it from pointers (base and new)
	dstPath->coordinatesF.size = (AMuint32)(dstCoord - dstPath->coordinatesF.data);
	dstPath->coordinatesCount = dstPath->coordinatesF.size;
	return AM_TRUE;

	#undef PROCESS_COMMAND
}

/*!
	\brief Find a path cache slot valid for the specified deviation. If a valid slot is not found, the
	index of the farthest slot is returned.
	\param index output path cache slot index
	\param path input path.
	\param deviation input deviation.
	\return AM_TRUE if a valid path cache slot has been found, else AM_FALSE.
	\pre deviation > 0.0f.
*/
AMbool amPathCacheSlotIndex(AMuint32 *index,
							const AMPath *path,
							const AMfloat deviation) {

	AMint32 i, j;
	AMfloat maxDist;

	AM_ASSERT(index);
	AM_ASSERT(path);
	AM_ASSERT(deviation > 0.0f);
	AM_ASSERT((path->flags & AM_PATH_FLATTEN_VALID) != 0);

	j = -1;
	for (i = 0; i < AM_PATH_CACHE_SLOTS_COUNT; ++i) {
		if (deviation >= path->cache[i].deviationMin && deviation <= path->cache[i].deviationMax) {
			*index = i;
			return AM_TRUE;
		}
	}

	maxDist = -1.0f;
	for (i = 0; i < AM_PATH_CACHE_SLOTS_COUNT; ++i) {

		// if we found a free slot, return it
		if (path->cache[i].deviationMin < 0.0f) {
			AM_ASSERT(path->cache[i].deviationMax < 0.0f);
			*index = i;
			return AM_FALSE;
		}
		else {
			AMfloat minD = amAbsf(deviation - path->cache[i].deviationMin);
			AMfloat maxD = amAbsf(deviation - path->cache[i].deviationMax);
			AMfloat d;
			
			d = AM_MIN(minD, maxD);
			if (d > maxDist) {
				j = i;
				maxDist = d;
			}
		}
	}
	AM_ASSERT(j >= 0 && j < AM_PATH_CACHE_SLOTS_COUNT);
	*index = (AMuint32)j;
	return AM_FALSE;
}

void amPathDynResourcesInit(AMPath *path) {

	AMuint32 i;

	AM_ASSERT(path);

	AM_DYNARRAY_PREINIT(path->commands)
	AM_DYNARRAY_PREINIT(path->segments)
	AM_DYNARRAY_PREINIT(path->coordinatesF)

	for (i = 0; i < AM_PATH_CACHE_SLOTS_COUNT; ++i) {

		AM_DYNARRAY_PREINIT(path->cache[i].flattenPts)
	#if defined(AM_FIXED_POINT_PIPELINE)
		AM_DYNARRAY_PREINIT(path->cache[i].flattenPtsx)
	#endif
		AM_DYNARRAY_PREINIT(path->cache[i].ptsPerContour)
		AM_DYNARRAY_PREINIT(path->cache[i].subPathsClosed)
		AM_DYNARRAY_PREINIT(path->cache[i].ptsCountPerSegment)
	#if defined(AM_GLE) || defined(AM_GLS)
		AM_DYNARRAY_PREINIT(path->cache[i].fillTriangles.triangPoints)
		AM_DYNARRAY_PREINIT(path->cache[i].fillTriangles.ushortIndexes)
		AM_DYNARRAY_PREINIT(path->cache[i].strokeTriangles.triangPoints)
		AM_DYNARRAY_PREINIT(path->cache[i].strokeTriangles.ushortIndexes)
	#elif defined(AM_SRE)
	#if defined(AM_FIXED_POINT_PIPELINE)
		AM_DYNARRAY_PREINIT(path->cache[i].strokePts)
	#else
		AM_DYNARRAY_PREINIT(path->cache[i].strokePts)
	#endif
		AM_DYNARRAY_PREINIT(path->cache[i].strokePtsPerContour)
	#else
		#error Unreachable point.
	#endif
	}
}

void amPathDynResourcesDestroy(AMPath *path) {

	AMuint32 i;

	AM_ASSERT(path);

	AM_DYNARRAY_DESTROY(path->commands)
	AM_DYNARRAY_DESTROY(path->coordinatesF)
	AM_DYNARRAY_DESTROY(path->segments)

	for (i = 0; i < AM_PATH_CACHE_SLOTS_COUNT; ++i) {
		AM_DYNARRAY_DESTROY(path->cache[i].flattenPts)
	#if defined(AM_FIXED_POINT_PIPELINE)
		AM_DYNARRAY_DESTROY(path->cache[i].flattenPtsx)
	#endif
		AM_DYNARRAY_DESTROY(path->cache[i].ptsPerContour)
		AM_DYNARRAY_DESTROY(path->cache[i].subPathsClosed)
		AM_DYNARRAY_DESTROY(path->cache[i].ptsCountPerSegment)
	#if defined(AM_GLE) || defined(AM_GLS)
		// destroy fill software cache (triangles)
		AM_DYNARRAY_DESTROY(path->cache[i].fillTriangles.triangPoints)
		AM_DYNARRAY_DESTROY(path->cache[i].fillTriangles.ushortIndexes)
		// destroy stroke software cache (triangles)
		AM_DYNARRAY_DESTROY(path->cache[i].strokeTriangles.triangPoints)
		AM_DYNARRAY_DESTROY(path->cache[i].strokeTriangles.ushortIndexes)
	#elif defined(AM_SRE)
		// destroy stroke software cache (polygons)
		AM_DYNARRAY_DESTROY(path->cache[i].strokePts)
		AM_DYNARRAY_DESTROY(path->cache[i].strokePtsPerContour)
	#else
		#error Unreachable point.
	#endif
	}
}


/*!
	\brief Initialize a given path.
	\param path path to initialize.
	\param pathFormat path format = VG_PATH_FORMAT_STANDARD.
	\param dataType one of the following data types: VG_PATH_DATATYPE_S_8, VG_PATH_DATATYPE_S_16, VG_PATH_DATATYPE_S_32, VG_PATH_DATATYPE_F.
	\param scale scale factor to apply to path segment coordinates.
	\param bias bias value to apply to path segment coordinates.
	\param segmentCapacityHint a hint as to the total number of segments that will eventually be stored in the path.
	\param coordCapacityHint a hint as to the total number of specified coordinates that will eventually be stored in the path.
	\param capabilities a bitwise OR of the desired VGPathCapabilities values. Bits of capabilities that do not correspond to values from VGPathCapabilities have no effect.
	\param _context pointer to a AMContext structure, containing the GL context (used to destroy VBO handles, in AmanithVG GLE).
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
	\pre scale != 0.0f.
*/
AMbool amPathInit(AMPath *path,
				  const AMint32 pathFormat,
				  const VGPathDatatype dataType,
				  const AMfloat scale,
				  const AMfloat bias,
				  const AMint32 segmentCapacityHint,
				  const AMint32 coordCapacityHint,
				  const VGbitfield capabilities,
				  const void *_context) {

	AMuint32 i, segmentHint, coordHint;
#if defined(AM_GLE) || defined(AM_GLS)
	const AMContext *context = (const AMContext *)_context;
#elif defined(AM_SRE)
	(void)_context;
#endif

	AM_ASSERT(path);
	AM_ASSERT((AMint32)dataType != (AMint32)VG_PATH_DATATYPE_INVALID);
	AM_ASSERT(scale != 0.0f);

	amPathDynResourcesInit(path);

	// set path format, coordinates datatype, scale and bias
	path->id = AM_PATH_HANDLE_ID;
	path->type = AM_PATH_HANDLE_ID;
	path->referenceCounter = 1;
	path->format = pathFormat;
	path->dataType = dataType;
	path->scale = scale;
	path->bias = bias;

	// allocates initial space for commands
	segmentHint = (segmentCapacityHint <= 0) ? 1 : segmentCapacityHint;
	AM_DYNARRAY_INIT_RESERVE(path->commands, AMuint8, segmentHint)
	if (path->commands.error) {
		amPathDynResourcesDestroy(path);
		return AM_FALSE;
	}
	AM_DYNARRAY_INIT_RESERVE(path->segments, AMPathSegment, segmentHint)
	if (path->segments.error) {
		amPathDynResourcesDestroy(path);
		return AM_FALSE;
	}

	// allocates initial space for coordinates
	coordHint = (coordCapacityHint <= 0) ? 2 : coordCapacityHint;
	AM_DYNARRAY_INIT_RESERVE(path->coordinatesF, AMfloat, coordHint)
	if (path->coordinatesF.error) {
		amPathDynResourcesDestroy(path);
		return AM_FALSE;
	}
	path->coordinatesCount = 0;

	// set path capabilities
	path->capabilities = capabilities & VG_PATH_CAPABILITY_ALL;

	// set an empty bounding box
	AM_VECT2_SET(&path->box.minPoint, 0.0f, 0.0f)
	AM_VECT2_SET(&path->box.maxPoint, 0.0f, 0.0f)

	for (i = 0; i < AM_PATH_CACHE_SLOTS_COUNT; ++i) {
		// reset deviation range to an empty range
		path->cache[i].deviationMin = -1.0f;
		path->cache[i].deviationMax = -1.0f;
	#if defined(AM_GLE) || defined(AM_GLS)
		// initialize fill software cache (triangles)
		path->cache[i].fillRule = VG_EVEN_ODD;
		path->cache[i].fillTriangles.isIndexed = AM_TRUE;
		AM_DYNARRAY_INIT(path->cache[i].fillTriangles.triangPoints, GL_VERTEX_TYPE)
		if (path->cache[i].fillTriangles.triangPoints.error) {
			amPathDynResourcesDestroy(path);
			return AM_FALSE;
		}
		AM_DYNARRAY_INIT(path->cache[i].fillTriangles.ushortIndexes, AMuint16)
		if (path->cache[i].fillTriangles.ushortIndexes.error) {
			amPathDynResourcesDestroy(path);
			return AM_FALSE;
		}
		// initialize stroke software cache (triangles)
		path->cache[i].strokeTriangles.isIndexed = AM_TRUE;
		AM_DYNARRAY_INIT(path->cache[i].strokeTriangles.triangPoints, GL_VERTEX_TYPE)
		if (path->cache[i].strokeTriangles.triangPoints.error) {
			amPathDynResourcesDestroy(path);
			return AM_FALSE;
		}
		AM_DYNARRAY_INIT(path->cache[i].strokeTriangles.ushortIndexes, AMuint16)
		if (path->cache[i].strokeTriangles.ushortIndexes.error) {
			amPathDynResourcesDestroy(path);
			return AM_FALSE;
		}
		if (context->glContext.vboSupported) {
			// initialize VBO fill triangles
			amGlTriangulationInit(&path->cache[i].vboFillTriangles);
			// initialize VBO stroke triangles
			amGlTriangulationInit(&path->cache[i].vboStrokeTriangles);
		}
		// initialize triangles validity flags
		path->cache[i].trianglesFlag = AM_FILL_TRIANGLES_INVALID | AM_STROKE_TRIANGLES_INVALID;
	#endif
		// set an invalid hash, so the next time the stroke cache must be reconstructed
		path->cache[i].strokeDesc.dashPatternSize = 0;
		path->cache[i].strokeDesc.dashPatternHash = AM_HASH_INVALID;
	}

	path->flags = 0;
	path->cacheSlotBaseDeviation = -1.0f;
	return AM_TRUE;
}

void amPathRewind(AMPath *path,
				  const AMint32 pathFormat,
				  const VGPathDatatype dataType,
				  const AMfloat scale,
				  const AMfloat bias,
				  const VGbitfield capabilities,
				  const AMContext *context) {

	AM_ASSERT(path);

	// set path format, coordinates datatype, scale and bias
	path->id = AM_PATH_HANDLE_ID;
	path->type = AM_PATH_HANDLE_ID;
	path->referenceCounter = 1;
	path->format = pathFormat;
	path->dataType = dataType;
	path->scale = scale;
	path->bias = bias;

	path->commands.size = 0;
	path->coordinatesF.size = 0;
	path->coordinatesCount = 0;
	path->segments.size = 0;
	// set path capabilities
	path->capabilities = capabilities & VG_PATH_CAPABILITY_ALL;
	// set an empty bounding box
	AM_VECT2_SET(&path->box.minPoint, 0.0f, 0.0f)
	AM_VECT2_SET(&path->box.maxPoint, 0.0f, 0.0f)
	// rewind cache
	amPathCacheClear(path, AM_FALSE, context);
	path->flags = 0;
}

/*!
	\brief It removes all segment command and coordinate data associated with the specified path.
	\param path the path to clear.
	\param capabilities a bitwise OR of the desired VGPathCapabilities values. Bits of capabilities that do
	not correspond to values from VGPathCapabilities have no effect.
	\param _context pointer to a AMContext structure, containing the GL context.
*/
void amPathClear(AMPath *path,
				 const VGbitfield capabilities,
				 const void *_context) {

	AM_ASSERT(path);
	AM_ASSERT(_context);

	// rewind commands
	path->commands.size = 0;
	// rewind coordinates
	path->coordinatesF.size = 0;
	path->coordinatesCount = 0;
	// rewind segments
	path->segments.size = 0;
	// rewind cache
	amPathCacheClear(path, AM_FALSE, (const AMContext *)_context);
	path->flags = 0;
	// set new capabilities
	path->capabilities = capabilities & VG_PATH_CAPABILITY_ALL;
}

// Destroy path resources.
void amPathResourcesDestroy(AMPath *path,
							void *_context) {

	AMuint32 i;
#if defined(AM_GLE) || defined(AM_GLS)
	AMContext *context = (AMContext *)_context;
	AM_ASSERT(context);
#elif defined(AM_SRE)
	(void)_context;
#endif

	AM_ASSERT(path);

	amPathDynResourcesDestroy(path);
	path->coordinatesCount = 0;

	for (i = 0; i < AM_PATH_CACHE_SLOTS_COUNT; ++i) {
	#if defined(AM_GLE) || defined(AM_GLS)
		if (context->glContext.vboSupported) {
			// destroy VBO fill triangles
			amGlTriangulationDestroy(&path->cache[i].vboFillTriangles, &context->glContext);
			// destroy VBO stroke triangles
			amGlTriangulationDestroy(&path->cache[i].vboStrokeTriangles, &context->glContext);
		}
	#endif
	}
}

/*!
	\brief Destroy the specified path, releasing any resources associated with it.
	\param path the path to destroy.
	\param _context pointer to a AMContext structure, containing the list of created handles.
*/
void amPathDestroy(AMPath *path,
				   void *_context) {

	AMContext *context = (AMContext *)_context;

	AM_ASSERT(path);
	AM_ASSERT(context);

	// decrement reference counter, it possibly leads to resources deallocation
	amCtxHandleRefCounterDec(path, context);
	path->id = AM_INVALID_HANDLE_ID;
	path->dataType = (VGPathDatatype)VG_PATH_DATATYPE_INVALID;
	path->scale = 0.0f;
	path->bias = 0.0f;
}

/*!
	\brief It requests the set of capabilities specified in the capabilities argument to be disabled for the given path.\n
	\param path path whose capabilities are removed from.
	\param capabilities a bitwise OR of the VGPathCapabilities values whose removal is requested.
*/
void amPathCapabilitiesRemove(AMPath *path,
							  const VGbitfield capabilities) {

	VGbitfield tmpCapabilities = capabilities & VG_PATH_CAPABILITY_ALL;
	
	AM_ASSERT(path);

	path->capabilities = path->capabilities & (((VGbitfield)(~0)) - tmpCapabilities);
	// if the path does not contain the following capabilities, we can deallocate
	// coordinatesF and commands arrays, in order to free unused memory
	if (!(path->capabilities & (VG_PATH_CAPABILITY_APPEND_FROM | VG_PATH_CAPABILITY_APPEND_TO |
								VG_PATH_CAPABILITY_MODIFY | VG_PATH_CAPABILITY_TRANSFORM_FROM |
								VG_PATH_CAPABILITY_TRANSFORM_TO | VG_PATH_CAPABILITY_INTERPOLATE_FROM |
								VG_PATH_CAPABILITY_INTERPOLATE_TO))) {

		AM_DYNARRAY_CLEAR(path->commands, AMuint8)
		AM_DYNARRAY_CLEAR(path->coordinatesF, AMfloat)
	}
}

/*!
	\brief It appends specified data to the given destination path. The data are formatted using the destination
	path format of. Each incoming coordinate value, regardless of datatype, is transformed by the scale factor
	and bias of the destination path.
	\param dstPath the destination path.
	\param numCommands number of segments commands to append.
	\param pathCommands segments commands to append.
	\param pathCoordinates segments coordinates to append.
	\param pathDataType data type of input coordinates.
	\param _context pointer to a AMContext structure, containing coordinate converters.
	\param applyScaleBias AM_TRUE to apply scale and bias, else AM_FALSE.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
	\pre numCommands > 0.
*/
AMbool amPathDataAppend(AMPath *dstPath,
						const AMint32 numCommands,
						const AMuint8 *pathCommands,
						const void *pathCoordinates,
						const VGPathDatatype pathDataType,
						const void *_context,
						const AMbool applyScaleBias) {

	const AMContext *context = (const AMContext *)_context;
	AMint32 i, j, srcBytesPerCoord;
	AMuint8 cmd, *srcCoords;
	AMfloat *coordinatesF, scale, bias;
	AMPathCoordinatesConverterFunction converter;
	AMuint32 oldCommandsSize, oldSegmentsSize, oldCoordinatesSize;
	AMuint32 newCommandsSize, newSegmentsSize, newCoordinatesSize;

	AM_ASSERT(dstPath);
	AM_ASSERT(pathCommands);
	AM_ASSERT(pathCoordinates);
	AM_ASSERT(numCommands > 0);
	AM_ASSERT((AMint32)pathDataType != (AMint32)VG_PATH_DATATYPE_INVALID);
	AM_ASSERT(context);

	srcCoords = (AMuint8 *)pathCoordinates;
	srcBytesPerCoord = numBytesPerDatatype[pathDataType];
	converter = context->coordinatesConverters[VG_PATH_DATATYPE_F][pathDataType];

	oldCommandsSize = dstPath->commands.size;
	oldSegmentsSize = dstPath->segments.size;
	oldCoordinatesSize = dstPath->coordinatesF.size;
	AM_ASSERT(oldCommandsSize == oldSegmentsSize);
	newCommandsSize = newSegmentsSize = oldCommandsSize + numCommands;

	// reserve space for dstPath->segments array
	if (dstPath->segments.capacity < newSegmentsSize) {
		AM_DYNARRAY_CLEAR_RESERVE(dstPath->segments, AMPathSegment, newSegmentsSize)
		if (dstPath->segments.error) {
			dstPath->segments.error = AM_DYNARRAY_NO_ERROR;
			return AM_FALSE;
		}
	}
	
	// reserve space for dstPath->commands array
	if (dstPath->commands.capacity < newCommandsSize) {
		AM_DYNARRAY_CLEAR_RESERVE(dstPath->commands, AMuint8, newCommandsSize)
		if (dstPath->commands.error) {
			dstPath->commands.error = AM_DYNARRAY_NO_ERROR;
			dstPath->segments.size = oldSegmentsSize;
			return AM_FALSE;
		}
	}

	// calculate the total amount of additional float coordinates to expand dstPath->coordinatesF array
	newCoordinatesSize = oldCoordinatesSize;
	for (i = 0; i < numCommands; ++i) {
		cmd = AM_PATH_SEGMENT_COMMAND(pathCommands[i]);
		newCoordinatesSize += numCoordsPerCommand[cmd];
	}
	// reserve space for dstPath->coordinatesF array
	if (dstPath->coordinatesF.capacity < newCoordinatesSize) {
		AM_DYNARRAY_CLEAR_RESERVE(dstPath->coordinatesF, AMfloat, newCoordinatesSize)
		if (dstPath->coordinatesF.error) {
			dstPath->coordinatesF.error = AM_DYNARRAY_NO_ERROR;
			dstPath->commands.size = oldCommandsSize;
			dstPath->segments.size = oldSegmentsSize;
			return AM_FALSE;
		}
	}

	dstPath->segments.size = newSegmentsSize;
	dstPath->commands.size = newCommandsSize;
	dstPath->coordinatesF.size = newCoordinatesSize;
	coordinatesF = &dstPath->coordinatesF.data[oldCoordinatesSize];
	
	if (applyScaleBias) {
		scale = dstPath->scale;
		bias = dstPath->bias;
	}
	else {
		scale = 1.0f;
		bias = 0.0f;
	}

	for (i = 0; i < numCommands; ++i) {

		AMint32 numCoords;

		// push the command
		cmd = AM_PATH_SEGMENT_COMMAND(pathCommands[i]);
		numCoords = numCoordsPerCommand[cmd];
		AM_ASSERT(numCoords >= 0);

		dstPath->commands.data[oldCommandsSize + i] = pathCommands[i];
		// create and initialize (invalid) the associated segment
		amPathSegmentInit(&dstPath->segments.data[oldSegmentsSize + i]);

		// push coordinates
		if (dstPath->dataType != VG_PATH_DATATYPE_F) {

			for (j = 0; j < numCoords; ++j) {

				AMfloat tmpF;
				AMint32 tmpI;
				
				converter((void *)&tmpF, (void *)srcCoords, scale, bias);
				tmpI = (AMint32)amFloorf(tmpF + 0.5f);
				*coordinatesF++ = (AMfloat)tmpI;
				srcCoords += srcBytesPerCoord;
			}
		}
		else {
			for (j = 0; j < numCoords; ++j) {
				converter((void *)(coordinatesF), (void *)srcCoords, scale, bias);
				coordinatesF++;
				srcCoords += srcBytesPerCoord;
			}
		}
	}

	// build all segments, it calculates only applicationPoint, ctrlParams, ccwArc for each segment
	amPathSegmentsBuild(dstPath);
	dstPath->flags &= (((AMuint32)(~0)) - (AM_PATH_FLATTEN_VALID | AM_PATH_BOX_VALID));
	dstPath->coordinatesCount = dstPath->coordinatesF.size;
	return AM_TRUE;
}

/*!
	\brief It appends a copy of all path segments from source path onto the end of the existing data in
	the destination path. If the scale and bias of the destination path define a narrower range than that
	of the source path, overflow may occur silently.
	\param dstPath the destination path.
	\param srcPath the source path.
	\param context context containing coordinate converters.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
*/
AMbool amPathAppend(AMPath *dstPath,
					const AMPath *srcPath,
					const AMContext *context) {

	AM_ASSERT(dstPath);
	AM_ASSERT(srcPath);

	AM_ASSERT((AMint32)dstPath->dataType != (AMint32)VG_PATH_DATATYPE_INVALID);
	AM_ASSERT((AMint32)srcPath->dataType != (AMint32)VG_PATH_DATATYPE_INVALID);

	return amPathDataAppend(dstPath, (VGint)srcPath->commands.size, srcPath->commands.data, srcPath->coordinatesF.data,	VG_PATH_DATATYPE_F, context, AM_TRUE);
}

/*!
	\brief It appends a transformed copy of source path to the current contents of destination path.
	The appended path is equivalent to the results of applying the current path-user-to-surface transformation
	to the source path.
	\param dstPath the destination path.
	\param srcPath the source path.
	\param context context containing coordinate converters.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
*/
AMbool amPathTransform(AMPath *dstPath,
					   const AMPath *srcPath,
					   AMContext *context) {

	AMPath tmpPath;
	AMint32 i;
	AMfloat floatCoord0, floatCoord1, floatCoord2;
	AMfloat *floatCoords, *dstCoord;
	AMuint8 cmd, absRel, *dstCommand;
	AMVect2f cursor, prevCursor, p, tmp0, tmp1, tmp2;
	AMEllipsef ellipse, transfEllipse;
	const AMMatrix33f *transform;
	AMbool res;

#define PROCESS_COMMAND(_vgCmd, _srcCoords) \
	switch (_vgCmd & 0xFE) { \
		case VG_CLOSE_PATH: \
			*dstCommand++ = _vgCmd; \
			break; \
		case VG_MOVE_TO: \
		case VG_LINE_TO: \
		case VG_SQUAD_TO: \
			*dstCommand++ = _vgCmd; \
			tmp0.x = (AMfloat)(*_srcCoords++); \
			tmp0.y = (AMfloat)(*_srcCoords++); \
			if (absRel == VG_RELATIVE) { \
				/* update absolute cursor */ \
				AM_VECT2_SELF_ADD(&cursor, &tmp0) \
			} \
			else { \
				/* update absolute cursor */ \
				AM_VECT2_SET(&cursor, tmp0.x, tmp0.y) \
			} \
			AM_MATRIX33_VECT2_MUL(&p, transform, &tmp0) \
			*dstCoord++ = p.x; \
			*dstCoord++ = p.y; \
			break; \
		case VG_HLINE_TO: \
			*dstCommand++ = VG_LINE_TO | absRel; \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			if (absRel == VG_RELATIVE) { \
				/* update absolute cursor */ \
				cursor.x += floatCoord0; \
				AM_VECT2_SET(&tmp0, floatCoord0, 0.0f) \
			} \
			else { \
				/* update absolute cursor */ \
				cursor.x = floatCoord0; \
				AM_VECT2_SET(&tmp0, floatCoord0, cursor.y) \
			} \
			/* handle matrix transformation */ \
			AM_MATRIX33_VECT2_MUL(&p, transform, &tmp0) \
			*dstCoord++ = p.x; \
			*dstCoord++ = p.y; \
			break; \
		case VG_VLINE_TO: \
			*dstCommand++ = VG_LINE_TO | absRel; \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			if (absRel == VG_RELATIVE) { \
				/* update absolute cursor */ \
				cursor.y += floatCoord0; \
				AM_VECT2_SET(&tmp0, 0.0f, floatCoord0) \
			} \
			else { \
				/* update absolute cursor */ \
				cursor.y = floatCoord0; \
				AM_VECT2_SET(&tmp0, cursor.x, floatCoord0) \
			} \
			/* handle matrix transformation */ \
			AM_MATRIX33_VECT2_MUL(&p, transform, &tmp0) \
			*dstCoord++ = p.x; \
			*dstCoord++ = p.y; \
			break; \
		case VG_QUAD_TO: \
		case VG_SCUBIC_TO: \
			*dstCommand++ = _vgCmd; \
			tmp0.x = (AMfloat)(*_srcCoords++); \
			tmp0.y = (AMfloat)(*_srcCoords++); \
			tmp1.x = (AMfloat)(*_srcCoords++); \
			tmp1.y = (AMfloat)(*_srcCoords++); \
			if (absRel == VG_RELATIVE) { \
				/* update absolute cursor */ \
				AM_VECT2_SELF_ADD(&cursor, &tmp1) \
			} \
			else { \
				/* update absolute cursor */ \
				AM_VECT2_SET(&cursor, tmp1.x, tmp1.y) \
			} \
			/* handle matrix transformation */ \
			AM_MATRIX33_VECT2_MUL(&p, transform, &tmp0) \
			*dstCoord++ = p.x; \
			*dstCoord++ = p.y; \
			AM_MATRIX33_VECT2_MUL(&p, transform, &tmp1) \
			*dstCoord++ = p.x; \
			*dstCoord++ = p.y; \
			break; \
		case VG_CUBIC_TO: \
			*dstCommand++ = _vgCmd; \
			tmp0.x = (AMfloat)(*_srcCoords++); \
			tmp0.y = (AMfloat)(*_srcCoords++); \
			tmp1.x = (AMfloat)(*_srcCoords++); \
			tmp1.y = (AMfloat)(*_srcCoords++); \
			tmp2.x = (AMfloat)(*_srcCoords++); \
			tmp2.y = (AMfloat)(*_srcCoords++); \
			if (absRel == VG_RELATIVE) { \
				/* update absolute cursor */ \
				AM_VECT2_SELF_ADD(&cursor, &tmp2) \
			} \
			else { \
				/* update absolute cursor */ \
				AM_VECT2_SET(&cursor, tmp2.x, tmp2.y) \
			} \
			/* handle matrix transformation */ \
			AM_MATRIX33_VECT2_MUL(&p, transform, &tmp0) \
			*dstCoord++ = p.x; \
			*dstCoord++ = p.y; \
			AM_MATRIX33_VECT2_MUL(&p, transform, &tmp1) \
			*dstCoord++ = p.x; \
			*dstCoord++ = p.y; \
			AM_MATRIX33_VECT2_MUL(&p, transform, &tmp2) \
			*dstCoord++ = p.x; \
			*dstCoord++ = p.y; \
			break; \
		case VG_SCCWARC_TO: \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			floatCoord1 = (AMfloat)(*_srcCoords++); \
			floatCoord2 = (AMfloat)(*_srcCoords++); \
			tmp0.x = (AMfloat)(*_srcCoords++); \
			tmp0.y = (AMfloat)(*_srcCoords++); \
			AM_VECT2_SET(&prevCursor, cursor.x, cursor.y) \
			if (absRel == VG_RELATIVE) { \
				/* update absolute cursor */ \
				AM_VECT2_SELF_ADD(&cursor, &tmp0) \
			} \
			else { \
				/* update absolute cursor */ \
				AM_VECT2_SET(&cursor, tmp0.x, tmp0.y) \
			} \
			amEllipsefSetByPoints(&ellipse, &prevCursor, &cursor, amAbsf(floatCoord0), amAbsf(floatCoord1), amDeg2Radf(floatCoord2), AM_FALSE, AM_TRUE); \
			amEllipsefTransform(&transfEllipse, transform, &ellipse); \
			*dstCommand++ = (ellipse.ccw != transfEllipse.ccw) ? (VG_SCWARC_TO | absRel) : (VG_SCCWARC_TO | absRel); \
			*dstCoord++ = transfEllipse.xSemiAxisLength; \
			*dstCoord++ = transfEllipse.ySemiAxisLength; \
			*dstCoord++ = amRad2Degf(transfEllipse.offsetRotation); \
			amEllipsefEvalByAngle(&p, &transfEllipse, transfEllipse.endAngle); \
			if (absRel == VG_RELATIVE) { \
				AM_VECT2_SELF_SUB(&p, &prevCursor) \
			} \
			*dstCoord++ = p.x; \
			*dstCoord++ = p.y; \
			break; \
		case VG_SCWARC_TO: \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			floatCoord1 = (AMfloat)(*_srcCoords++); \
			floatCoord2 = (AMfloat)(*_srcCoords++); \
			tmp0.x = (AMfloat)(*_srcCoords++); \
			tmp0.y = (AMfloat)(*_srcCoords++); \
			AM_VECT2_SET(&prevCursor, cursor.x, cursor.y) \
			if (absRel == VG_RELATIVE) { \
				/* update absolute cursor */ \
				AM_VECT2_SELF_ADD(&cursor, &tmp0) \
			} \
			else { \
				/* update absolute cursor */ \
				AM_VECT2_SET(&cursor, tmp0.x, tmp0.y) \
			} \
			amEllipsefSetByPoints(&ellipse, &prevCursor, &cursor, amAbsf(floatCoord0), amAbsf(floatCoord1), amDeg2Radf(floatCoord2), AM_FALSE, AM_FALSE); \
			amEllipsefTransform(&transfEllipse, transform, &ellipse); \
			*dstCommand++ = (ellipse.ccw != transfEllipse.ccw) ? (VG_SCCWARC_TO | absRel) : (VG_SCWARC_TO | absRel); \
			*dstCoord++ = transfEllipse.xSemiAxisLength; \
			*dstCoord++ = transfEllipse.ySemiAxisLength; \
			*dstCoord++ = amRad2Degf(transfEllipse.offsetRotation); \
			amEllipsefEvalByAngle(&p, &transfEllipse, transfEllipse.endAngle); \
			if (absRel == VG_RELATIVE) { \
				AM_VECT2_SELF_SUB(&p, &prevCursor) \
			} \
			*dstCoord++ = p.x; \
			*dstCoord++ = p.y; \
			break; \
		case VG_LCCWARC_TO: \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			floatCoord1 = (AMfloat)(*_srcCoords++); \
			floatCoord2 = (AMfloat)(*_srcCoords++); \
			tmp0.x = (AMfloat)(*_srcCoords++); \
			tmp0.y = (AMfloat)(*_srcCoords++); \
			AM_VECT2_SET(&prevCursor, cursor.x, cursor.y) \
			if (absRel == VG_RELATIVE) { \
				/* update absolute cursor */ \
				AM_VECT2_SELF_ADD(&cursor, &tmp0) \
			} \
			else { \
				/* update absolute cursor */ \
				AM_VECT2_SET(&cursor, tmp0.x, tmp0.y) \
			} \
			amEllipsefSetByPoints(&ellipse, &prevCursor, &cursor, amAbsf(floatCoord0), amAbsf(floatCoord1), amDeg2Radf(floatCoord2), AM_TRUE, AM_TRUE); \
			amEllipsefTransform(&transfEllipse, transform, &ellipse); \
			*dstCommand++ = (ellipse.ccw != transfEllipse.ccw) ? (VG_LCWARC_TO | absRel) : (VG_LCCWARC_TO | absRel); \
			*dstCoord++ = transfEllipse.xSemiAxisLength; \
			*dstCoord++ = transfEllipse.ySemiAxisLength; \
			*dstCoord++ = amRad2Degf(transfEllipse.offsetRotation); \
			amEllipsefEvalByAngle(&p, &transfEllipse, transfEllipse.endAngle); \
			if (absRel == VG_RELATIVE) { \
				AM_VECT2_SELF_SUB(&p, &prevCursor) \
			} \
			*dstCoord++ = p.x; \
			*dstCoord++ = p.y; \
			break; \
		case VG_LCWARC_TO: \
			floatCoord0 = (AMfloat)(*_srcCoords++); \
			floatCoord1 = (AMfloat)(*_srcCoords++); \
			floatCoord2 = (AMfloat)(*_srcCoords++); \
			tmp0.x = (AMfloat)(*_srcCoords++); \
			tmp0.y = (AMfloat)(*_srcCoords++); \
			AM_VECT2_SET(&prevCursor, cursor.x, cursor.y) \
			if (absRel == VG_RELATIVE) { \
				/* update absolute cursor */ \
				AM_VECT2_SELF_ADD(&cursor, &tmp0) \
			} \
			else { \
				/* update absolute cursor */ \
				AM_VECT2_SET(&cursor, tmp0.x, tmp0.y) \
			} \
			amEllipsefSetByPoints(&ellipse, &prevCursor, &cursor, amAbsf(floatCoord0), amAbsf(floatCoord1), amDeg2Radf(floatCoord2), AM_TRUE, AM_FALSE); \
			amEllipsefTransform(&transfEllipse, transform, &ellipse); \
			*dstCommand++ = (ellipse.ccw != transfEllipse.ccw) ? (VG_LCCWARC_TO | absRel) : (VG_LCWARC_TO | absRel); \
			*dstCoord++ = transfEllipse.xSemiAxisLength; \
			*dstCoord++ = transfEllipse.ySemiAxisLength; \
			*dstCoord++ = amRad2Degf(transfEllipse.offsetRotation); \
			amEllipsefEvalByAngle(&p, &transfEllipse, transfEllipse.endAngle); \
			if (absRel == VG_RELATIVE) { \
				AM_VECT2_SELF_SUB(&p, &prevCursor) \
			} \
			*dstCoord++ = p.x; \
			*dstCoord++ = p.y; \
			break; \
		default: \
			break; \
	}

	AM_ASSERT(context);
	AM_ASSERT(dstPath);
	AM_ASSERT(srcPath);
	AM_ASSERT((AMint32)srcPath->dataType != (AMint32)VG_PATH_DATATYPE_INVALID);
	AM_ASSERT((AMint32)dstPath->dataType != (AMint32)VG_PATH_DATATYPE_INVALID);

	// initialize a temporary path
#if defined RIM_VG_SRC
    tmpPath.flags = 0;
#endif
	if (!amPathInit(&tmpPath, VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, (AMint32)srcPath->commands.size, amPathCoordinatesCount(srcPath), VG_PATH_CAPABILITY_ALL, context))
		return AM_FALSE;

	// we must allocate, for each command, the maximum size for coordinates (6 coordinates for cubic Bezier)
	AM_DYNARRAY_RESERVE(tmpPath.commands, AMuint8, srcPath->commands.size)
	if (tmpPath.commands.error) {
		amPathResourcesDestroy(&tmpPath, context);
		return AM_FALSE;
	}
	AM_DYNARRAY_CLEAR_RESERVE(tmpPath.coordinatesF, AMfloat, (srcPath->commands.size * 6))
	if (tmpPath.coordinatesF.error) {
		amPathResourcesDestroy(&tmpPath, context);
		return AM_FALSE;
	}

	AM_VECT2_SET(&cursor, 0.0f, 0.0f)
	dstCommand = tmpPath.commands.data;
	dstCoord = tmpPath.coordinatesF.data;
	floatCoords = srcPath->coordinatesF.data;
	transform = &context->pathUserToSurface;
	for (i = 0; i < (AMint32)srcPath->commands.size; ++i) {
		cmd = srcPath->commands.data[i];
		absRel = AM_PATH_SEGMENT_ABSREL(cmd);
		PROCESS_COMMAND(cmd, floatCoords)
	}
	// We have used AM_DYNARRAY_CLEAR_RESERVE (with an upper bound for memory allocation) to avoid
	// to reallocate and to not use PUSH_BACK macro that is "slow" (it checks some bound conditions).
	// We have to know the exact number of coordinates, we can derive it from pointers (base and new)
	tmpPath.coordinatesF.size = (AMuint32)(dstCoord - tmpPath.coordinatesF.data);
	tmpPath.coordinatesCount = tmpPath.coordinatesF.size;
	// append temporary path to the real destination path
	res = amPathAppend(dstPath, &tmpPath, context);
	// destroy the temporary path
	amPathResourcesDestroy(&tmpPath, context);
	return res;

	#undef PROCESS_COMMAND
}

/*!
	\brief It appends a path, defined by interpolation (or extrapolation) between a start path and an end
	path by the given amount, to the destination path.
	\param pathsInterpolable output flag: AM_TRUE if the paths had compatible segment types after normalization), AM_FALSE otherwise.
	\param dstPath the destination path.
	\param startPath input start path.
	\param endPath input end path.
	\param amount interpolation amount.
	\param context context containing coordinate converters.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
	\note if interpolation is unsuccessful, destination path is left unchanged.
*/
AMbool amPathInterpolate(AMbool *pathsInterpolable,
						 AMPath *dstPath,
						 const AMPath *startPath,
						 const AMPath *endPath,
						 const AMfloat amount,
						 AMContext *context) {

	AMPath normSrcPath;
	AMPath normDstPath;
	AMfloat *srcCoord, *dstCoord;
	AMint32 i;
	AMbool res;

	AM_ASSERT(dstPath);
	AM_ASSERT(startPath);
	AM_ASSERT(endPath);

	// initialize temporary paths
#if defined RIM_VG_SRC
    normSrcPath.flags = 0;
#endif
	if (!amPathInit(&normSrcPath, VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, (AMint32)startPath->commands.size, amPathCoordinatesCount(startPath), VG_PATH_CAPABILITY_ALL, context))
		return AM_FALSE;
#if defined RIM_VG_SRC
    normDstPath.flags = 0;
#endif
	if (!amPathInit(&normDstPath, VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1.0f, 0.0f, (AMint32)endPath->commands.size, amPathCoordinatesCount(endPath), VG_PATH_CAPABILITY_ALL, context)) {
		amPathResourcesDestroy(&normSrcPath, context);
		return AM_FALSE;
	}

	// normalize temporary paths, without matrix application
	if (!amPathNormalize(&normSrcPath, startPath)) {
		// destroy temporary paths
		amPathResourcesDestroy(&normSrcPath, context);
		amPathResourcesDestroy(&normDstPath, context);
		return AM_FALSE;
	}
	if (!amPathNormalize(&normDstPath, endPath)) {
		// destroy temporary paths
		amPathResourcesDestroy(&normSrcPath, context);
		amPathResourcesDestroy(&normDstPath, context);
		return AM_FALSE;
	}

	// check for the same sequence of commands, according to OpenVG specification
	if (normSrcPath.commands.size != normDstPath.commands.size) {
		*pathsInterpolable = AM_FALSE;
		return AM_TRUE;
	}

	for (i = 0; i < (AMint32)normSrcPath.commands.size; ++i) {
		// if the normSrcPath command is not an ellipse arc, we have to check for command equality
		if (normSrcPath.commands.data[i] < VG_SCCWARC_TO || normSrcPath.commands.data[i] > VG_LCWARC_TO) {
			if (normSrcPath.commands.data[i] != normDstPath.commands.data[i]) {
				// destroy temporary paths
				amPathResourcesDestroy(&normSrcPath, context);
				amPathResourcesDestroy(&normDstPath, context);
				*pathsInterpolable = AM_FALSE;
				return AM_TRUE;
			}
		}
		else {
			// the normSrcPath command is an ellipse arc: now check that also normDstPath command is an ellipse arc, if
			// not so exit with an error
			if (normDstPath.commands.data[i] < VG_SCCWARC_TO || normDstPath.commands.data[i] > VG_LCWARC_TO) {
				// destroy temporary paths
				amPathResourcesDestroy(&normSrcPath, context);
				amPathResourcesDestroy(&normDstPath, context);
				*pathsInterpolable = AM_FALSE;
				return AM_TRUE;
			}
			// both commands are granted to be an ellipse arc: now take care of ellipse arc interpolation
			if (amount >= 0.5f)
				normSrcPath.commands.data[i] = normDstPath.commands.data[i];
		}
	}

	AM_ASSERT(normSrcPath.coordinatesF.size == normDstPath.coordinatesF.size);

	// interpolate each coordinate
	srcCoord = normSrcPath.coordinatesF.data;
	dstCoord = normDstPath.coordinatesF.data;
	for (i = 0; i < (AMint32)normSrcPath.coordinatesF.size; ++i) {
		*srcCoord = (1.0f - amount) * (*srcCoord) + amount * (*dstCoord);
		srcCoord++;
		dstCoord++;
	}

	// append interpolated path
	res = amPathAppend(dstPath, &normSrcPath, context);
	// destroy temporary paths
	amPathResourcesDestroy(&normSrcPath, context);
	amPathResourcesDestroy(&normDstPath, context);
	*pathsInterpolable = AM_TRUE;
	return res;
}

/*!
	\brief It modifies the coordinate data for a contiguous range of segments of destination path, starting
	at the specified index (where 0 is the index of the first path segment). Each incoming coordinate
	value, regardless of data type, is transformed by the scale factor and bias of the destination path.
	\param dstPath the destination path.
	\param startIndex index of the first path segment to modify.
	\param numSegments number of path segments to modify.
	\param pathData pointer to input coordinates.
	\param context context containing coordinate converters.
*/
void amPathCoordinatesModify(AMPath *dstPath,
							 const AMint32 startIndex,
							 const AMint32 numSegments,
							 const void *pathData,
							 const AMContext *context) {

	AMint32 srcCoordsSize, dstCoordsSize, i, j, k;
	const AMuint8 *srcCoords = (const AMuint8 *)pathData;
	AMuint8 *dstCoords = NULL, cmd;
	AMPathCoordinatesConverterFunction converter;

	AM_ASSERT(dstPath);
	AM_ASSERT(numSegments > 0);
	AM_ASSERT(startIndex >= 0);
	AM_ASSERT(pathData);
	AM_ASSERT((startIndex + numSegments) <= (AMint32)dstPath->segments.size);

	srcCoordsSize = numBytesPerDatatype[dstPath->dataType];
	dstCoordsSize = numBytesPerDatatype[VG_PATH_DATATYPE_F];
	converter = context->coordinatesConverters[VG_PATH_DATATYPE_F][dstPath->dataType];

	// calculate the start index for the first coordinates to be modified
	j = 0;
	for (i = 0; i < startIndex; ++i) {
		cmd = AM_PATH_SEGMENT_COMMAND(dstPath->commands.data[i]);
		AM_ASSERT(numCoordsPerCommand[cmd] >= 0);
		j += numCoordsPerCommand[cmd];
	}

	dstCoords = (AMuint8 *)&dstPath->coordinatesF.data[j];
	AM_ASSERT(dstCoords != NULL);
	
	// loop over modified segments
	for (i = 0; i < numSegments; ++i) {

		// change coordinates relative to the i-th segment
		cmd = AM_PATH_SEGMENT_COMMAND(dstPath->commands.data[i + startIndex]);
		AM_ASSERT(numCoordsPerCommand[cmd] >= 0);
		j = numCoordsPerCommand[cmd];
		for (k = 0; k < j; ++k) {
			converter((void *)dstCoords, (const void *)srcCoords, dstPath->scale, dstPath->bias);
			srcCoords += srcCoordsSize;
			dstCoords += dstCoordsSize;
		}
		// invalidate i-th segment, for length calculation
		dstPath->segments.data[i + startIndex].flags &= ((AMuint16)(~0) - AM_PATH_SEGMENT_LENGTH_VALID);
	}

	// invalidate the next segment (if possible), up to the (startIndex + numSegments)-th
	if (i + startIndex < (AMint32)dstPath->segments.size) {
		dstPath->segments.data[i + startIndex].flags &= ((AMuint16)(~0) - AM_PATH_SEGMENT_LENGTH_VALID);
	}

	// build path segments array
	amPathSegmentsBuild(dstPath);
	dstPath->flags &= (((AMuint32)(~0)) - (AM_PATH_FLATTEN_VALID | AM_PATH_BOX_VALID));
}

/*!
	\brief It returns the length of a given portion of a path in the user coordinate system (that is, in
	the path's own coordinate system, disregarding any matrix settings).
	\param path input path whose length is to calculate.
	\param startSegment first path segment to consider during length calculation.
	\param numSegments number of path segments to consider during length calculation.
	\param context context containing path segment functions tables.
	\return the requested length.
*/
AMfloat amPathLength(AMPath *path,
					 const AMint32 startSegment,
					 const AMint32 numSegments,
					 const AMContext *context) {

	AMfloat len = 0.0f;
	AMint32 i;
	AMPathSegmentLengthFunction lengthFunc;

	AM_ASSERT(path);
	AM_ASSERT(startSegment >= 0);
	AM_ASSERT(numSegments > 0);
	AM_ASSERT(startSegment + numSegments <= (AMint32)path->segments.size);
	AM_ASSERT(path->segments.size == path->commands.size);

	for (i = 0; i < numSegments; ++i) {

		if (path->segments.data[startSegment + i].flags & AM_PATH_SEGMENT_LENGTH_VALID)
			len += path->segments.data[startSegment + i].length;
		else {
			lengthFunc = context->lengthFunctions[path->segments.data[startSegment + i].id];
			len += lengthFunc(&path->segments.data[startSegment + i]);
		}
	}
	return len;
}

/*!
	\brief It returns the point lying a given distance along a given portion of a path and the unit-length
	tangent vector at that point.
	\param x pointer to the evaluated x coordinate.
	\param y pointer to the evaluated y coordinate.
	\param tangentX pointer to the evaluated tangent x coordinate.
	\param tangentY pointer to the evaluated tangent y coordinate.
	\param path input path to evaluate.
	\param startSegment first path segment to consider during evaluation.
	\param numSegments number of path segments to consider during evaluation.
	\param distance distance at which evaluate the path.
	\param context context containing path segment functions tables.
*/
void amPathEvaluate(AMfloat *x,
					AMfloat *y,
					AMfloat *tangentX,
					AMfloat *tangentY,
					AMPath *path,
					const AMint32 startSegment,
					const AMint32 numSegments,
					const AMfloat distance,
					const AMContext *context) {

	AMfloat len, l = 0.0f, param;
	AMPathSegmentLengthFunction lengthFunc;
	AMPathSegmentParamFunction paramFunc;
	AMPathSegmentEvalFunction evalFunc;
	AMint32 i, j;

	#define NORMALIZE_TANGENT \
	    l = amSqrtf((*tangentX) * (*tangentX) + (*tangentY) * (*tangentY)); \
	    if (l > AM_EPSILON_FLOAT) { \
		   (*tangentX) /= l; \
		   (*tangentY) /= l; \
		}

	AM_ASSERT(x);
	AM_ASSERT(y);
	AM_ASSERT(tangentX);
	AM_ASSERT(tangentY);
	AM_ASSERT(path);

	i = startSegment;
	j = (AMint32)path->segments.size;
	len = distance;
	while (len > 0.0f && i < j) {

		if (path->segments.data[i].flags & AM_PATH_SEGMENT_LENGTH_VALID)
			l = path->segments.data[i].length;
		else {
			lengthFunc = context->lengthFunctions[path->segments.data[i].id];
			l = lengthFunc(&path->segments.data[i]);
		}
		len -= l;
		i++;
	}

	if (distance <= 0.0f) {
		i = startSegment;
		while (i < j && path->segments.data[i].id == AM_MOVE_TO_SEGMENT)
			i++;
		// if we have reached a valid segment evaluate it
		if (i < j) {
			evalFunc = context->evalFunctions[path->segments.data[i].id];
			param = 0.0f;
			evalFunc(x, y, tangentX, tangentY, &path->segments.data[i], param);
			NORMALIZE_TANGENT
		}
		// this is the case (i == j) of a path built of all MOVE_TO commands
		else {
			evalFunc = context->evalFunctions[path->segments.data[i - 1].id];
			param = 0.0f;
			evalFunc(x, y, NULL, NULL, &path->segments.data[i - 1], param);
			*tangentX = 1.0f;
			*tangentY = 0.0f;
		}
		return;
	}

	if (len > 0.0f) {
		evalFunc = context->evalFunctions[path->segments.data[startSegment + numSegments - 1].id];
		param = 1.0f;
		evalFunc(x, y, tangentX, tangentY, &path->segments.data[j - 1], param);
		NORMALIZE_TANGENT
		return;
	}

	i--;
	l += len;

	paramFunc = context->paramFunctions[path->segments.data[i].id];
	evalFunc = context->evalFunctions[path->segments.data[i].id];

	param = paramFunc(&path->segments.data[i], l);
	evalFunc(x, y, tangentX, tangentY, &path->segments.data[i], param);
    NORMALIZE_TANGENT

	#undef NORMALIZE_TANGENT
}

// DEBUG STUFF
/*
void dumpPath(AMPath *path,
			  const AMFlattenParams *flattenParams) {

	AMuint32 i, j;
	AMfloat x0, y0, x1, y1, x2, y2;
	FILE *f;

	f = fopen("dump_path.txt", "wt");
	j = 0;
	for (i = 0; i < path->commands.size; ++i) {
		switch (path->commands.data[i]) {
			case VG_MOVE_TO_ABS:
				x0 = path->coordinatesF.data[j++];
				y0 = path->coordinatesF.data[j++];
				fprintf(f, "MOVE_TO_ABS(%ff, %ff)\n", x0, y0);
				break;
			case VG_MOVE_TO_REL:
				x0 = path->coordinatesF.data[j++];
				y0 = path->coordinatesF.data[j++];
				fprintf(f, "MOVE_TO_REL(%ff, %ff)\n", x0, y0);
				break;
			case VG_LINE_TO_ABS:
				x0 = path->coordinatesF.data[j++];
				y0 = path->coordinatesF.data[j++];
				fprintf(f, "LINE_TO_ABS(%ff, %ff)\n", x0, y0);
				break;
			case VG_LINE_TO_REL:
				x0 = path->coordinatesF.data[j++];
				y0 = path->coordinatesF.data[j++];
				fprintf(f, "LINE_TO_REL(%ff, %ff)\n", x0, y0);
				break;
			case VG_HLINE_TO_ABS:
				x0 = path->coordinatesF.data[j++];
				fprintf(f, "HLINE_TO_ABS(%ff)\n", x0);
				break;
			case VG_HLINE_TO_REL:
				x0 = path->coordinatesF.data[j++];
				fprintf(f, "HLINE_TO_REL(%ff)\n", x0);
				break;
			case VG_VLINE_TO_ABS:
				y0 = path->coordinatesF.data[j++];
				fprintf(f, "VLINE_TO_ABS(%ff)\n", y0);
				break;
			case VG_VLINE_TO_REL:
				y0 = path->coordinatesF.data[j++];
				fprintf(f, "VLINE_TO_REL(%ff)\n", y0);
				break;
			case VG_QUAD_TO_ABS:
				x0 = path->coordinatesF.data[j++];
				y0 = path->coordinatesF.data[j++];
				x1 = path->coordinatesF.data[j++];
				y1 = path->coordinatesF.data[j++];
				fprintf(f, "QUAD_TO_ABS(%ff, %ff, %ff, %ff)\n", x0, y0, x1, y1);
				break;
			case VG_QUAD_TO_REL:
				x0 = path->coordinatesF.data[j++];
				y0 = path->coordinatesF.data[j++];
				x1 = path->coordinatesF.data[j++];
				y1 = path->coordinatesF.data[j++];
				fprintf(f, "QUAD_TO_REL(%ff, %ff, %ff, %ff)\n", x0, y0, x1, y1);
				break;
			case VG_CUBIC_TO_ABS:
				x0 = path->coordinatesF.data[j++];
				y0 = path->coordinatesF.data[j++];
				x1 = path->coordinatesF.data[j++];
				y1 = path->coordinatesF.data[j++];
				x2 = path->coordinatesF.data[j++];
				y2 = path->coordinatesF.data[j++];
				fprintf(f, "CUBIC_TO_ABS(%ff, %ff, %ff, %ff, %ff, %ff)\n", x0, y0, x1, y1, x2, y2);
				break;
			case VG_CUBIC_TO_REL:
				x0 = path->coordinatesF.data[j++];
				y0 = path->coordinatesF.data[j++];
				x1 = path->coordinatesF.data[j++];
				y1 = path->coordinatesF.data[j++];
				x2 = path->coordinatesF.data[j++];
				y2 = path->coordinatesF.data[j++];
				fprintf(f, "CUBIC_TO_REL(%ff, %ff, %ff, %ff, %ff, %ff)\n", x0, y0, x1, y1, x2, y2);
				break;
			case VG_SQUAD_TO_ABS:
				x0 = path->coordinatesF.data[j++];
				y0 = path->coordinatesF.data[j++];
				fprintf(f, "SQUAD_TO_ABS(%ff, %ff)\n", x0, y0);
				break;
			case VG_SQUAD_TO_REL:
				x0 = path->coordinatesF.data[j++];
				y0 = path->coordinatesF.data[j++];
				fprintf(f, "SQUAD_TO_REL(%ff, %ff)\n", x0, y0);
				break;
			case VG_SCUBIC_TO_ABS:
				x0 = path->coordinatesF.data[j++];
				y0 = path->coordinatesF.data[j++];
				x1 = path->coordinatesF.data[j++];
				y1 = path->coordinatesF.data[j++];
				fprintf(f, "SCUBIC_TO_ABS(%ff, %ff, %ff, %ff)\n", x0, y0, x1, y1);
				break;
			case VG_SCUBIC_TO_REL:
				x0 = path->coordinatesF.data[j++];
				y0 = path->coordinatesF.data[j++];
				x1 = path->coordinatesF.data[j++];
				y1 = path->coordinatesF.data[j++];
				fprintf(f, "SCUBIC_TO_REL(%ff, %ff, %ff, %ff)\n", x0, y0, x1, y1);
				break;
			case VG_CLOSE_PATH:
				fprintf(f, "CLOSE_PATH()\n");
				break;
		}
	}

	fprintf(f, "\n");
	fprintf(f, "context->flattenParams.deviation = %ff;\n", flattenParams->deviation);
	fprintf(f, "context->flattenParams.flatness = %ff;\n", flattenParams->flatness);
	fprintf(f, "context->flattenParams.two_sqrt_flatness = %ff;\n", flattenParams->two_sqrt_flatness);
	fprintf(f, "context->flattenParams.three_over_flatness = %ff;\n", flattenParams->three_over_flatness);
	fprintf(f, "context->flattenParams.two_sqrt_flatness_over_three = %ff;\n", flattenParams->two_sqrt_flatness_over_three);
	fprintf(f, "context->flattenParams.two_cuberoot_flatness_over_three = %ff;\n", flattenParams->two_cuberoot_flatness_over_three);
	fprintf(f, "context->flattenParams.sixtyfour_flatness = %ff;\n", flattenParams->sixtyfour_flatness);

	fclose(f);
}
*/

// Visual Studio requires precise floating point model to correctly compile this procedure
#if defined(AM_CC_MSVC)
#pragma float_control(precise, on)
#endif
/*!
	\brief Flatten a given path, and store the result in a cache slot.
	\param slotIndex path cache slot index where path flattening has been stored.
	\param path path to flatten.
	\param _context pointer to a AMContext structure, containing path segment functions tables.
	\param doneFlattening will be set to AM_FALSE if a cache slot containing an already valid flattening
	has been found, else AM_TRUE (in this case flattening occurs).
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
	\todo add a compile time definition to switch between different flattening caching approaches.
*/
AMbool amPathFlatten(AMuint32 *slotIndex,
					 AMPath *path,
					 void *_context,
					 AMbool *doneFlattening) {

	AMContext *context = (AMContext *)_context;
	AMint32 i, j, k, pushedCount;
	AMPathSegmentFlattenFunction flattenFunc;
	AMVect2f p, oldFinalPoint, lastPushed;
	AMbool oldFinalPointValid, pathSlotFound, firstTime;
	AMfloat deviationMin, deviationMax, boxX, boxY, boxW, boxH, dmax, cleanerPrecision;
	AMuint32 pathSlotIndex, oldPtsSize;
	AMPathCacheSlot *slot;
#if defined(AM_FIXED_POINT_PIPELINE)
	AMVect2x tmpVectX;
#endif
#if defined(VG_MZT_statistics)
	AMuint32 startMS, endMS;
#endif

	#define ALLOCATE_FLATTEN_ARRAY(_array, _itemTypeName, _capacity) \
	if (!_array.data) { \
		AM_DYNARRAY_INIT_RESERVE(_array, _itemTypeName, _capacity) \
	} \
	else { \
		if ((_capacity) > (_array.capacity)) { \
		AM_DYNARRAY_CLEAR_RESERVE(_array, _itemTypeName, _capacity) \
	} \
	} \
	_array.size = 0; \
	if (_array.error) { \
		_array.error = AM_DYNARRAY_NO_ERROR; \
		return AM_FALSE; \
	}

	AM_ASSERT(path);
	AM_ASSERT(doneFlattening);

	firstTime = AM_FALSE;
	if (path->flags & AM_PATH_FLATTEN_VALID) {
		pathSlotFound = amPathCacheSlotIndex(&pathSlotIndex, path, context->flattenParams.deviation);
		if (pathSlotFound) {
			*doneFlattening = AM_FALSE;
			*slotIndex = pathSlotIndex;
			return AM_TRUE;
		}
	}
	else {
		// clear the cache and use slot 0
		amPathCacheClear(path, AM_FALSE, context);
		pathSlotIndex = 0;
		firstTime = AM_TRUE;
	}

	if (firstTime) {
	#if defined(AM_GLE) || defined(AM_GLS)

		#define MIN_RANG_FACTOR 8.0f
		AMfloat dMin;

		dMin = context->flattenParams.deviation / MIN_RANG_FACTOR;
		deviationMin = ((AMfloat)rand() / (AMfloat)RAND_MAX) * (context->flattenParams.deviation - dMin) + dMin;
		path->cacheRangeFactor = MIN_RANG_FACTOR;
		deviationMax = deviationMin * path->cacheRangeFactor;
		path->cacheSlotBaseDeviation = deviationMin;
	#else // AmanithVG SRE
		path->cacheRangeFactor = 2.0f;
		deviationMin = context->flattenParams.deviation;
		deviationMax = deviationMin * path->cacheRangeFactor;
		path->cacheSlotBaseDeviation = deviationMin;
	#endif
	}
	else {
		AMfloat d;

		AM_ASSERT(path->cacheSlotBaseDeviation > 0.0f);

		if (context->flattenParams.deviation < path->cacheSlotBaseDeviation) {

			d = path->cacheSlotBaseDeviation / path->cacheRangeFactor;
			while (context->flattenParams.deviation < d)
				d /= path->cacheRangeFactor;

			deviationMin = d;
			deviationMax = d * path->cacheRangeFactor;
		}
		else {
			d = path->cacheSlotBaseDeviation * path->cacheRangeFactor;
			while (context->flattenParams.deviation > d)
				d *= path->cacheRangeFactor;
			
			deviationMin = d / path->cacheRangeFactor;
			deviationMax = d;
		}
	}

	// extract choosen slot pointer
	slot = &path->cache[pathSlotIndex];

	// if the path has no segments just set an empty box and exit
	j = (AMint32)path->segments.size;
	if (j <= 0) {
		slot->deviationMin = deviationMin;
		slot->deviationMax = deviationMax;
		path->flags |= AM_PATH_FLATTEN_VALID;
		doneFlattening = AM_FALSE;
		*slotIndex = pathSlotIndex;
		return AM_TRUE;
	}

#if defined(VG_MZT_statistics)
	startMS = amTimeGet();
#endif

	// initialize global flatten array, supposing that each segment generate 2 points
	ALLOCATE_FLATTEN_ARRAY(slot->flattenPts, AMVect2f, path->segments.size * 2)
#if defined(AM_FIXED_POINT_PIPELINE)
	ALLOCATE_FLATTEN_ARRAY(slot->flattenPtsx, AMVect2x, path->segments.size * 2)
#endif
	ALLOCATE_FLATTEN_ARRAY(slot->ptsPerContour, AMint32, 4)
	ALLOCATE_FLATTEN_ARRAY(slot->subPathsClosed, AMbool, 4)
	ALLOCATE_FLATTEN_ARRAY(slot->ptsCountPerSegment, AMint32, path->segments.size + 1)

	// threshold used to check for consecutive point equivalence
	cleanerPrecision = 2.0f * AM_EPSILON_FLOAT;

	// rewind temporary array used to store 0-based flattening point of each curve segment
	context->tmpFlatteningPts.size = 0;

	oldPtsSize = 0;
	flattenFunc = context->flattenFunctions[path->segments.data[0].id];

	// flatten first curve segment
	flattenFunc(&context->tmpFlatteningPts, &path->segments.data[0], &context->flattenParams);
	k = (AMint32)context->tmpFlatteningPts.size;
	// append all 0-based points to global path flattening, after their translation by application point
	lastPushed.x = AM_MIN_FLOAT;
	lastPushed.y = AM_MIN_FLOAT;
	pushedCount = 0;

	for (j = 0; j < k - 1; ++j) {
		AM_VECT2_ADD(&p, &path->segments.data[0].applicationPoint, &context->tmpFlatteningPts.data[j])
		if (amAbsf(p.x - lastPushed.x) > cleanerPrecision || amAbsf(p.y - lastPushed.y) > cleanerPrecision) {
			AM_DYNARRAY_PUSH_BACK(slot->flattenPts, AMVect2f, p)
		#if defined(AM_FIXED_POINT_PIPELINE)
			AM_VECT2_SET(&tmpVectX, amFloatToFixed1616(p.x), amFloatToFixed1616(p.y))
			AM_DYNARRAY_PUSH_BACK(slot->flattenPtsx, AMVect2x, tmpVectX)
		#endif
			lastPushed = p;
			pushedCount++;
		}
	}
	// this is the case of a path made of a single command (e.g. one VG_QUAD_TO_ABS), so we have to push
	// all points (now we are going to add also the last one) in the global flattening array
	if (path->segments.size == 1) {
		AM_VECT2_ADD(&p, &path->segments.data[0].applicationPoint, &context->tmpFlatteningPts.data[j])
		if (amAbsf(p.x - lastPushed.x) > cleanerPrecision || amAbsf(p.y - lastPushed.y) > cleanerPrecision) {
			AM_DYNARRAY_PUSH_BACK(slot->flattenPts, AMVect2f, p)
		#if defined(AM_FIXED_POINT_PIPELINE)
			AM_VECT2_SET(&tmpVectX, amFloatToFixed1616(p.x), amFloatToFixed1616(p.y))
			AM_DYNARRAY_PUSH_BACK(slot->flattenPtsx, AMVect2x, tmpVectX)
		#endif
			lastPushed = p;
		}
	}
	if (pushedCount > 0) {
		// NB: tangents are normalized inside stroking routines
		AM_DYNARRAY_PUSH_BACK_LIGHT(slot->ptsCountPerSegment, pushedCount)
	}
	
	// if the first segment is a 'moveto' or 'close' we must invalidate first oldFinalPoint
	if (path->segments.data[0].id == AM_MOVE_TO_SEGMENT || path->segments.data[0].id == AM_CLOSE_SEGMENT) {
		oldFinalPoint.x = oldFinalPoint.y = 0.0f;
		oldFinalPointValid = AM_FALSE;
	}
	else {
		if (k > 1) {
			oldFinalPoint.x = path->segments.data[0].applicationPoint.x + context->tmpFlatteningPts.data[k - 1].x;
			oldFinalPoint.y = path->segments.data[0].applicationPoint.y + context->tmpFlatteningPts.data[k - 1].y;
			oldFinalPointValid = AM_TRUE;
		}
		else {
			// this is the case of a command not equal to 'moveto' nor 'close' that has <= 1 flatten points
			// (for example an curve collapsed into one point
			oldFinalPoint.x = oldFinalPoint.y = 0.0f;
			oldFinalPointValid = AM_FALSE;
		}
	}
	
	for (i = 1; i < (VGint)path->segments.size; ++i) {

		flattenFunc = context->flattenFunctions[path->segments.data[i].id];
		context->tmpFlatteningPts.size = 0;
		flattenFunc(&context->tmpFlatteningPts, &path->segments.data[i], &context->flattenParams);
		k = (AMint32)context->tmpFlatteningPts.size;

		pushedCount = 0;
		// for non degenerated cases (collapsed in one point entities or 'moveto' or 'closeto' segments), we
		// have at least 2 points of flatten
		if (k > 1) {
			AM_ASSERT(path->segments.data[i].id != AM_MOVE_TO_SEGMENT);
			AM_ASSERT(path->segments.data[i].id != AM_CLOSE_SEGMENT);
			// append all 0-based points to global path flattening, after their translation by application point
			// NB: we skip last flatten point because all flattening routines push also the last control point
			for (j = 0; j < k - 1; ++j) {
				AM_VECT2_ADD(&p, &path->segments.data[i].applicationPoint, &context->tmpFlatteningPts.data[j])
				if (amAbsf(p.x - lastPushed.x) > cleanerPrecision || amAbsf(p.y - lastPushed.y) > cleanerPrecision) {
					AM_DYNARRAY_PUSH_BACK(slot->flattenPts, AMVect2f, p)
				#if defined(AM_FIXED_POINT_PIPELINE)
					AM_VECT2_SET(&tmpVectX, amFloatToFixed1616(p.x), amFloatToFixed1616(p.y))
					AM_DYNARRAY_PUSH_BACK(slot->flattenPtsx, AMVect2x, tmpVectX)
				#endif
					lastPushed = p;
					pushedCount++;
				}
			}

			if (pushedCount > 0) {
				// NB: tangents are normalized inside stroking routines
				AM_DYNARRAY_PUSH_BACK_LIGHT(slot->ptsCountPerSegment, pushedCount)
			}
			
			// keep track of last (absolute) flatten point
			oldFinalPoint.x = path->segments.data[i].applicationPoint.x + context->tmpFlatteningPts.data[k - 1].x;
			oldFinalPoint.y = path->segments.data[i].applicationPoint.y + context->tmpFlatteningPts.data[k - 1].y;
			oldFinalPointValid = AM_TRUE;
		}

		if (path->segments.data[i].id == AM_CLOSE_SEGMENT) {

			// if oldFinalPoint is valid we have to push it
			if (oldFinalPointValid) {
				if (amAbsf(oldFinalPoint.x - lastPushed.x) > cleanerPrecision || amAbsf(oldFinalPoint.y - lastPushed.y) > cleanerPrecision) {
					AM_DYNARRAY_PUSH_BACK(slot->flattenPts, AMVect2f, oldFinalPoint)
				#if defined(AM_FIXED_POINT_PIPELINE)
					AM_VECT2_SET(&tmpVectX, amFloatToFixed1616(oldFinalPoint.x), amFloatToFixed1616(oldFinalPoint.y))
					AM_DYNARRAY_PUSH_BACK(slot->flattenPtsx, AMVect2x, tmpVectX)
				#endif
					lastPushed = oldFinalPoint;
				}
				oldFinalPointValid = AM_FALSE;
			}

			// push a path closure flag inside subPathsClosed
			if (slot->flattenPts.size > oldPtsSize) {

				AMVect2f *firstPoint = &slot->flattenPts.data[oldPtsSize];
				AMVect2f *lastPoint = &slot->flattenPts.data[slot->flattenPts.size - 1];

				// remove last point (of the current sub-contour) in the case of path CLOSED, where the first and last points are coincident
					if (amAbsf(lastPoint->x - firstPoint->x) <= cleanerPrecision && amAbsf(lastPoint->y - firstPoint->y) <= cleanerPrecision) {
						slot->flattenPts.size--;
					#if defined(AM_FIXED_POINT_PIPELINE)
						slot->flattenPtsx.size--;
					#endif
					}
				else {
					// manage implicit/explicit closure
					AM_ASSERT(slot->ptsCountPerSegment.size > 0);
					AM_DYNARRAY_PUSH_BACK_LIGHT(slot->ptsCountPerSegment, 1)
				}

				if (slot->flattenPts.size > oldPtsSize) {
					// we are sure to are have a contour made of at least 2 points
					AM_DYNARRAY_PUSH_BACK(slot->ptsPerContour, AMint32, (slot->flattenPts.size - oldPtsSize))
					AM_DYNARRAY_PUSH_BACK(slot->subPathsClosed, AMbool, AM_TRUE)
					oldPtsSize = slot->flattenPts.size;
				}
				else {
					// contour is degenerated
					AM_ASSERT(slot->flattenPts.size == oldPtsSize);
					AM_ASSERT(slot->ptsCountPerSegment.size > 0);
					AM_ASSERT(slot->ptsCountPerSegment.data[slot->ptsCountPerSegment.size - 1] > 0);
					slot->ptsCountPerSegment.data[slot->ptsCountPerSegment.size - 1]--;
					if (slot->ptsCountPerSegment.data[slot->ptsCountPerSegment.size - 1] == 0)
						slot->ptsCountPerSegment.size--;
				}
			}
			// now the path is finished or a new sub-path is going to begin, so we must reset lastPushed point
			lastPushed.x = AM_MIN_FLOAT;
			lastPushed.y = AM_MIN_FLOAT;
		}
		else
		// 'moveto' (other than the first) and 'close' commands tell a path closure
		if (path->segments.data[i].id == AM_MOVE_TO_SEGMENT || i == (AMint32)path->segments.size - 1) {

			AMuint32 ptsCount;

			// if oldFinalPoint is valid we have to push it
			if (oldFinalPointValid) {
				if (amAbsf(oldFinalPoint.x - lastPushed.x) > cleanerPrecision || amAbsf(oldFinalPoint.y - lastPushed.y) > cleanerPrecision) {
					AM_DYNARRAY_PUSH_BACK(slot->flattenPts, AMVect2f, oldFinalPoint)
				#if defined(AM_FIXED_POINT_PIPELINE)
					AM_VECT2_SET(&tmpVectX, amFloatToFixed1616(oldFinalPoint.x), amFloatToFixed1616(oldFinalPoint.y))
					AM_DYNARRAY_PUSH_BACK(slot->flattenPtsx, AMVect2x, tmpVectX)
				#endif
					lastPushed = oldFinalPoint;
				}
				oldFinalPointValid = AM_FALSE;
			}

			AM_ASSERT(slot->flattenPts.size >= oldPtsSize);
			ptsCount = slot->flattenPts.size - oldPtsSize;
			// push a path closure flag inside subPathsClosed
			if (ptsCount > 1) {
				// we are sure to are have a contour made of at least 2 points
				AM_DYNARRAY_PUSH_BACK(slot->ptsPerContour, AMint32, ptsCount)
				AM_DYNARRAY_PUSH_BACK(slot->subPathsClosed, AMbool, AM_FALSE)
				oldPtsSize = slot->flattenPts.size;
			}
			else
			if (ptsCount == 1) {
				// contour is made of a single point, so discard it
				AM_ASSERT(slot->ptsCountPerSegment.size > 0);
				slot->ptsCountPerSegment.size--;
			}

			// now the path is finished or a new sub-path is going to begin, so we must reset lastPushed point
			lastPushed.x = AM_MIN_FLOAT;
			lastPushed.y = AM_MIN_FLOAT;
		}
	}

	// this is the case of a path made of a single command (e.g. one VG_QUAD_TO_ABS)
	if (path->segments.size == 1) {
		AM_DYNARRAY_PUSH_BACK(slot->ptsPerContour, AMint32, ((AMint32)slot->flattenPts.size))
		AM_DYNARRAY_PUSH_BACK(slot->subPathsClosed, AMbool, AM_FALSE)
	}

#if defined(VG_MZT_statistics)
	endMS = amTimeGet();
	context->statisticsInfo.flatteningTimeMS += (endMS - startMS);
	context->statisticsInfo.flatteningPerformedCount++;
#endif

	// check for memory errors
	if (context->tmpFlatteningPts.error ||
		slot->flattenPts.error ||
	#if defined(AM_FIXED_POINT_PIPELINE)
		slot->flattenPtsx.error ||
	#endif
		slot->ptsPerContour.error ||
		slot->subPathsClosed.error ||
		slot->ptsCountPerSegment.error) {

		context->tmpFlatteningPts.error = AM_DYNARRAY_NO_ERROR;
		slot->flattenPts.error = AM_DYNARRAY_NO_ERROR;
	#if defined(AM_FIXED_POINT_PIPELINE)
		slot->flattenPtsx.error = AM_DYNARRAY_NO_ERROR;
	#endif
		slot->ptsPerContour.error = AM_DYNARRAY_NO_ERROR;
		slot->subPathsClosed.error = AM_DYNARRAY_NO_ERROR;
		slot->ptsCountPerSegment.error = AM_DYNARRAY_NO_ERROR;
		*slotIndex = pathSlotIndex;
		return AM_FALSE;
	}

	// check for consecutive repeated points
	amPathBounds(&boxX, &boxY, &boxW, &boxH, path, context);
	dmax = AM_MAX(boxW, boxH);

	// discard the whole path if its bounding box max dimension is less than cleanerPrecision
	if (dmax < cleanerPrecision) {
		slot->ptsPerContour.size = 0;
		slot->flattenPts.size = 0;
	#if defined(AM_FIXED_POINT_PIPELINE)
		slot->flattenPtsx.size = 0;
	#endif
		// clear / rewind the tags array
		slot->ptsCountPerSegment.size = 0;
		// now the flatten is valid
		path->flags |= AM_PATH_FLATTEN_VALID;
		// set new deviation range
		slot->deviationMin = 0.0f;
		slot->deviationMax = AM_MAX_FLOAT;
		*doneFlattening = AM_TRUE;
		*slotIndex = pathSlotIndex;
		return AM_TRUE;
	}

#if defined(VG_MZT_statistics)
	context->statisticsInfo.flatteningPointsCount += slot->flattenPts.size;
#endif

	// now the flatten (and the box) is valid
	path->flags |= AM_PATH_FLATTEN_VALID;
#if defined(AM_GLE) || defined(AM_GLS)
	// fill and stroke triangles must be regenerated
	slot->trianglesFlag = AM_FILL_TRIANGLES_INVALID | AM_STROKE_TRIANGLES_INVALID;
#endif
	// set new deviation range
	if (path->flags & AM_PATH_MADE_OF_LINES) {
		slot->deviationMin = 0.0f;
		slot->deviationMax = AM_MAX_FLOAT;
	}
	else {
		slot->deviationMin = deviationMin;
		slot->deviationMax = deviationMax;
	}

	*doneFlattening = AM_TRUE;
	*slotIndex = pathSlotIndex;
	return AM_TRUE;

	#undef ALLOCATE_FLATTEN_ARRAY
}
#if defined(AM_CC_MSVC)
#pragma float_control(precise, off)
#endif

/*!
	\brief It returns an axis-aligned bounding box that tightly bounds the interior of the given path.
	Stroking parameters are ignored.
	\param minX pointer to the minimum x coordinate of the box.
	\param minY pointer to the minimum y coordinate of the box.
	\param width pointer to the width of the box.
	\param height pointer to the height of the box.
	\param path input path whose box is to calculate.
	\param _context pointer to a AMContext structure, containing path segment functions tables.
*/
void amPathBounds(AMfloat *minX,
				  AMfloat *minY,
				  AMfloat *width,
				  AMfloat *height,
				  AMPath *path,
				  const void *_context) {

	const AMContext *context = (const AMContext *)_context;
	AMAABox2f box, tmpBox;
	AMuint32 i, j;
	AMPathSegmentBoxEvalFunction boxEvalFunc;

	AM_ASSERT(path);
	AM_ASSERT(minX);
	AM_ASSERT(minY);
	AM_ASSERT(width);
	AM_ASSERT(height);
	AM_ASSERT(context);

	// if the box is already calculated and valid, return it
	if (path->flags & AM_PATH_BOX_VALID) {
		*minX = path->box.minPoint.x;
		*minY = path->box.minPoint.y;
		*width = path->box.maxPoint.x - path->box.minPoint.x;
		*height = path->box.maxPoint.y - path->box.minPoint.y;
		return;
	}

	j = (AMuint32)path->segments.size;
	if (j == 0) {
		*minX = *minY = *width = *height = 0.0f;
		return;
	}

	boxEvalFunc = context->boxEvalFunctions[path->segments.data[0].id];
	boxEvalFunc(&box, &path->segments.data[0]);

	for (i = 1; i < j; ++i) {

		boxEvalFunc = context->boxEvalFunctions[path->segments.data[i].id];
		boxEvalFunc(&tmpBox, &path->segments.data[i]);
		AM_AABOX2_EXTEND_TO_INCLUDE(&box, &tmpBox.minPoint)
		AM_AABOX2_EXTEND_TO_INCLUDE(&box, &tmpBox.maxPoint)
	}

	path->box = box;
	*minX = path->box.minPoint.x;
	*minY = path->box.minPoint.y;
	*width = path->box.maxPoint.x - path->box.minPoint.x;
	*height = path->box.maxPoint.y - path->box.minPoint.y;
	path->flags |= AM_PATH_BOX_VALID;
}

/*!
	\brief It returns an axis-aligned bounding box that is guaranteed to enclose the geometry of the given
	path following transformation by the current path-user-to-surface transform.
	\param minX pointer to the minimum x coordinate of the box.
	\param minY pointer to the minimum y coordinate of the box.
	\param width pointer to the width of the box.
	\param height pointer to the height of the box.
	\param path input path whose box is to calculate.
	\param context context containing path segment functions tables.
*/
void amPathTransformedBounds(AMfloat *minX,
							 AMfloat *minY,
							 AMfloat *width,
							 AMfloat *height,
							 AMPath *path,
							 const AMContext *context) {

	AMfloat localMinX, localMinY, localWidth, localHeight;
	AMVect2f p, q;
	AMAABox2f tmpBox;

	AM_ASSERT(context);
	AM_ASSERT(path);
	AM_ASSERT(minX);
	AM_ASSERT(minY);
	AM_ASSERT(width);
	AM_ASSERT(height);

	amPathBounds(&localMinX, &localMinY, &localWidth, &localHeight, path, context);

	// transform bounding box
	AM_MATRIX33_VECT2_MUL(&q, &context->pathUserToSurface, &path->box.minPoint)
	tmpBox.minPoint = tmpBox.maxPoint = q;

	AM_MATRIX33_VECT2_MUL(&q, &context->pathUserToSurface, &path->box.maxPoint)
	AM_AABOX2_EXTEND_TO_INCLUDE(&tmpBox, &q)

	AM_VECT2_SET(&p, path->box.maxPoint.x, path->box.minPoint.y)
	AM_MATRIX33_VECT2_MUL(&q, &context->pathUserToSurface, &p)
	AM_AABOX2_EXTEND_TO_INCLUDE(&tmpBox, &q)

	AM_VECT2_SET(&p, path->box.minPoint.x, path->box.maxPoint.y)
	AM_MATRIX33_VECT2_MUL(&q, &context->pathUserToSurface, &p)
	AM_AABOX2_EXTEND_TO_INCLUDE(&tmpBox, &q)

	// return the box
	*minX = tmpBox.minPoint.x;
	*minY = tmpBox.minPoint.y;
	*width = tmpBox.maxPoint.x - tmpBox.minPoint.x;
	*height = tmpBox.maxPoint.y - tmpBox.minPoint.y;
}

/*!
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
*/
AMbool amPathNew(AMPath **path,
				 VGPath *handle,
				 const AMint32 pathFormat,
				 const VGPathDatatype dataType,
				 const AMfloat scale,
				 const AMfloat bias,
				 const AMint32 segmentCapacityHint,
				 const AMint32 coordCapacityHint,
				 const VGbitfield capabilities,
				 void *_context) {

	AMContext *context = (AMContext *)_context;
	AMPathsPoolsManager *pathsPoolsManager = &context->handles->pathsPools;
	AMPathsPoolPtrDynArray *pools = &pathsPoolsManager->pools;
	AMbool mustBeInitialized;
	AMPathsPool *pool;
	AMPath *resPath;
	VGPath resHandle;

	AM_ASSERT(context);
	AM_ASSERT(pathsPoolsManager);

	// check if there are available handles
	if (pathsPoolsManager->availablePathsList.size > 0) {

		AMPathRef pathRef;

		mustBeInitialized = AM_FALSE;
		// extract an available handle descriptor (previously deleted)
		pathsPoolsManager->availablePathsList.size--;
		pathRef = pathsPoolsManager->availablePathsList.data[pathsPoolsManager->availablePathsList.size];
		// extract the relative pool
		pool = pathsPoolsManager->pools.data[pathRef.c.pool];
		resPath = &pool->data[pathRef.c.poolIdx];
	}
	else {
		mustBeInitialized = AM_TRUE;
		if (pools->size == 0 || pools->data[pools->size - 1]->size == AM_PATHS_POOL_CAPACITY) {

			// try to allocate a new paths pool
			pool = (AMPathsPool *)amMalloc(sizeof(AMPathsPool));
			if (!pool)
				return AM_FALSE;

			AM_DYNARRAY_PUSH_BACK(*pools, AMPathsPoolPtr, pool)
			if (pools->error) {
				pools->error = AM_DYNARRAY_NO_ERROR;
				// destroy the new allocated pool, if the "push back" operation failed
				amFree(pool);
				return AM_FALSE;
			}
			// initialize the pool as empty
			pool->size = 0;
		}
		else
			pool = pools->data[pools->size - 1];
		
		resPath = &pool->data[pool->size++];
	}

	// initialize / rewind the path
	if (mustBeInitialized) {
		// initialize path
		if (!amPathInit(resPath, pathFormat, dataType, scale, bias, segmentCapacityHint, coordCapacityHint, capabilities, context)) {
			// remove the last element from the pool, and if empty free it
			AM_ASSERT(pool->size > 0);
			pool->size--;
			if (pool->size == 0) {
				amFree(pool);
				pools->size--;

			}
			return AM_FALSE;
		}
	}
	else
		// rewind the path
		amPathRewind(resPath, pathFormat, dataType, scale, bias, capabilities, context);

	// create the handle
	resHandle = amCtxHandleNew(context, (AMhandle)resPath);
	if (resHandle == VG_INVALID_HANDLE) {
		// in order to avoid inconsistencies / assertion, reset the reference counter
		resPath->referenceCounter = 0;
		// in this case the path was taken from an hole
		if (!mustBeInitialized)
			pathsPoolsManager->availablePathsList.size++;
		else {
			// path was initialized, so its resources must be destroyed
			amPathResourcesDestroy(resPath, context);
			// remove the last element from the pool, and if empty free it
			AM_ASSERT(pool->size > 0);
			pool->size--;
			if (pool->size == 0) {
				amFree(pool);
				pools->size--;

			}
		}
		return AM_FALSE;
	}

	*path = resPath;
	*handle = resHandle;
	return AM_TRUE;
}

void amPathRemoveRecover(AMContext *context,
						 AMPath *path,
						 AMPathRef *pathRef) {

	AMContextHandlesList *handles = context->handles;
	AMHandleDynArray *createdHandlesList = &handles->createdHandlesList;
	AMPathsPoolPtrDynArray *pools = &handles->pathsPools.pools;
	AMPathRefDynArray *availablePathsList = &handles->pathsPools.availablePathsList;
	AMPathsPool *lastPool = pools->data[pools->size - 1];

	AM_ASSERT(context);
	AM_ASSERT(path && path->referenceCounter == 0);
	AM_ASSERT(pathRef);
	AM_ASSERT(createdHandlesList->data[path->vgHandle] == path);

	createdHandlesList->data[path->vgHandle] = NULL;
	AM_DYNARRAY_PUSH_BACK(handles->availableHandlesList, VGHandle, path->vgHandle)
	path->vgHandle = VG_INVALID_HANDLE;
	// if the was a memory error in the push back, we loose and handle entry, but we can do nothing
	if (handles->availableHandlesList.error)
		handles->availableHandlesList.error = AM_DYNARRAY_NO_ERROR;

	if (lastPool->data[lastPool->size - 1].referenceCounter > 0) {
		// link handle to the new position (in the pool)
		createdHandlesList->data[lastPool->data[lastPool->size - 1].vgHandle] = &pools->data[pathRef->c.pool]->data[pathRef->c.poolIdx];
		// if the last path is referenced, we can move it to fill the hole created by the path remotion and we are done
		pools->data[pathRef->c.pool]->data[pathRef->c.poolIdx] = lastPool->data[lastPool->size - 1];
		lastPool->size--;
	}
	else {

		AMuint32 i;

		for (i = 0; i < availablePathsList->size; ++i) {

			if (availablePathsList->data[i].c.pool == pools->size - 1 && availablePathsList->data[i].c.poolIdx == lastPool->size - 1) {
				lastPool->size--;
				availablePathsList->data[i].key = pathRef->key;
				break;
			}
		}
		// path must have been found inside the loop
		AM_ASSERT(i < availablePathsList->size);
	}

	// destroy the resources associated with path
	amPathResourcesDestroy(path, context);

	if (lastPool->size == 0) {
		amFree(lastPool);
		pools->size--;
	}
}

void amPathRemove(void *_context,
				  AMPath *path) {

	AMContext *context = (AMContext *)_context;
	AMPathsPoolsManager *pathsPoolsManager = &context->handles->pathsPools;
	AMuint32 i, j;

	AM_ASSERT(context);
	AM_ASSERT(pathsPoolsManager);
	AM_ASSERT(path);
	AM_ASSERT(path->referenceCounter == 0);
	AM_ASSERT(context->handles->createdHandlesList.data[path->vgHandle] == path);

	j = pathsPoolsManager->pools.size;
	for (i = 0; i < j; ++i) {

		AMPathsPool *pool = pathsPoolsManager->pools.data[i];
		AMPath *poolBase = pool->data;
		AMPath *poolEnd = poolBase + AM_PATHS_POOL_CAPACITY;

		// check if the handle belongs to the pool
		if (path >= poolBase && path < poolEnd) {

			AMPathRef pathRef;

			pathRef.c.pool = i;
			pathRef.c.poolIdx = (AMuint16)(path - poolBase);

			// copy the path descriptor inside the list of available path descriptors
			AM_DYNARRAY_PUSH_BACK(pathsPoolsManager->availablePathsList, AMPathRef, pathRef)
			if (pathsPoolsManager->availablePathsList.error) {
				pathsPoolsManager->availablePathsList.error = AM_DYNARRAY_NO_ERROR;
				amPathRemoveRecover(context, path, &pathRef);
			}
			else
				amCtxHandleRemove(context, path->vgHandle);
			return;
		}
	}
	AM_ASSERT(0 == 1);
}

// *********************************************************************
//                        Public implementations
// *********************************************************************

/*!
	\brief It creates a new path that is ready to accept segment data and returns a VGPath handle to it.
	The path data will be formatted in the specified format.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param pathFormat path format = VG_PATH_FORMAT_STANDARD.
	\param datatype one of the following data types: VG_PATH_DATATYPE_S_8, VG_PATH_DATATYPE_S_16, VG_PATH_DATATYPE_S_32, VG_PATH_DATATYPE_F.
	\param scale scale factor to apply to path segment coordinates.
	\param bias bias value to apply to path segment coordinates.
	\param segmentCapacityHint a hint as to the total number of segments that will eventually be stored in
	the path.
	\param coordCapacityHint a hint as to the total number of specified coordinates that will eventually
	be stored in the path.
	\param capabilities a bitwise OR of the desired VGPathCapabilities values. Bits of capabilities that
	do not correspond to values from VGPathCapabilities have no effect.
	\return a VGPath handle if the operation was successful, else VG_INVALID_HANDLE.
*/
VG_API_CALL VGPath VG_API_ENTRY vgCreatePath(VGint pathFormat,
                                             VGPathDatatype datatype,
                                             VGfloat scale,
                                             VGfloat bias,
                                             VGint segmentCapacityHint,
                                             VGint coordCapacityHint,
                                             VGbitfield capabilities) VG_API_EXIT {

	AMPath *pth;
	VGPath handle;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgCreatePath");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}

	// this implementation supports only standard format
	if (pathFormat != VG_PATH_FORMAT_STANDARD) {
		amCtxErrorSet(currentContext, VG_UNSUPPORTED_PATH_FORMAT_ERROR);
		AM_MEMORY_LOG("vgCreatePath");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}
	// check for supported datatype
	if (datatype > VG_PATH_DATATYPE_F) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgCreatePath");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}

	// handle NaN and Inf values
	scale = amNanInfFix(scale);
	bias = amNanInfFix(bias);

	// check for 0 scale
	if (scale == 0.0f) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgCreatePath");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}

	// allocate the path
	if (!amPathNew(&pth, &handle, pathFormat, datatype, scale, bias, segmentCapacityHint, coordCapacityHint, capabilities, currentContext)) {
		// try to retrieve as much memory as possbile
		amMemMngRetrieve(currentContext, AM_TRUE);
		// try to re-create the path
		if (!amPathNew(&pth, &handle, pathFormat, datatype, scale, bias, segmentCapacityHint, coordCapacityHint, capabilities, currentContext)) {	
		#if defined(AM_DEBUG_MEMORY)
			amCtxCheckConsistence(currentContext);
		#endif
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
			AM_MEMORY_LOG("vgCreatePath  (amPathNew fail)");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}
	}

#if defined(AM_DEBUG_MEMORY)
	amCtxCheckConsistence(currentContext);
#endif

	// decrement the counter of the memory manager
	amCtxMemMngCountDown(currentContext);
	// exit without errors
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgCreatePath");
	OPENVG_RETURN(handle)
}

/*!
	\brief It removes all segment command and coordinate data associated with the specified path.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param path the path to clear.
	\param capabilities a bitwise OR of the desired VGPathCapabilities values. Bits of capabilities that do
	not correspond to values from VGPathCapabilities have no effect.
*/
VG_API_CALL void VG_API_ENTRY vgClearPath(VGPath path,
                                          VGbitfield capabilities) VG_API_EXIT {

	AMPath *pth;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgClearPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check if the path handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, path) != AM_PATH_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgClearPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	
	pth = (AMPath *)currentContext->handles->createdHandlesList.data[path];
	AM_ASSERT(pth);

	amPathClear(pth, capabilities, (const void *)currentContext);
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgClearPath");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief Destroy the specified path, releasing any resources associated with it, and makes the handle
	invalid in all contexts that shared it.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param path the path to destroy.
*/
VG_API_CALL void VG_API_ENTRY vgDestroyPath(VGPath path) VG_API_EXIT {

	AMPath *pth;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgDestroyPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check if the path handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, path) != AM_PATH_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgDestroyPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	// release any resources associated with path
	pth = (AMPath *)currentContext->handles->createdHandlesList.data[path];
	AM_ASSERT(pth);

#if defined(AM_DEBUG_MEMORY)
	amCtxCheckConsistence(currentContext);
#endif
	amPathDestroy(pth, currentContext);

	if (pth->referenceCounter == 0)
		// remove path object from context internal list
		amPathRemove(currentContext, pth);

	// decrement the counter of the memory manager
	amCtxMemMngCountDown(currentContext);
	// exit without errors
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgDestroyPath");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief It requests the set of capabilities specified in the capabilities argument to be disabled
	for the given path.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param path path whose capabilities are removed from.
	\param capabilities a bitwise OR of the VGPathCapabilities values whose removal is requested.
*/
VG_API_CALL void VG_API_ENTRY vgRemovePathCapabilities(VGPath path,
                                                       VGbitfield capabilities) VG_API_EXIT {

	AMPath *pth;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgRemovePathCapabilities");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check if the path handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, path) != AM_PATH_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgRemovePathCapabilities");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	pth = (AMPath *)currentContext->handles->createdHandlesList.data[path];
	AM_ASSERT(pth);
	amPathCapabilitiesRemove(pth, capabilities);

	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgRemovePathCapabilities");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief It returns the current capabilities of the path, as a bitwise OR of VGPathCapabilities constants.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\return path capabilities if the operations was successful, else 0.
*/
VG_API_CALL VGbitfield VG_API_ENTRY vgGetPathCapabilities(VGPath path) VG_API_EXIT {

	AMPath *pth;
	VGbitfield res;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgGetPathCapabilities");
		OPENVG_RETURN(0)
	}

	// check if the path handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, path) != AM_PATH_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgGetPathCapabilities");
		OPENVG_RETURN(0)
	}
	pth = (AMPath *)currentContext->handles->createdHandlesList.data[path];
	AM_ASSERT(pth);
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgGetPathCapabilities");
	res = pth->capabilities;
	OPENVG_RETURN(res)
}

/*!
	\brief It appends a copy of all path segments from source path onto the end of the existing data in
	the destination path. If the scale and bias of the destination path define a narrower range than that
	of the source path, overflow may occur silently.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param dstPath the destination path.
	\param srcPath the source path.
*/
VG_API_CALL void VG_API_ENTRY vgAppendPath(VGPath dstPath,
                                           VGPath srcPath) VG_API_EXIT {

	AMPath *srcPth;
	AMPath *dstPth;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgAppendPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check if the dstPath handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, dstPath) != AM_PATH_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgAppendPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	// check if the srcPath handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, srcPath) != AM_PATH_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgAppendPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	dstPth = (AMPath *)currentContext->handles->createdHandlesList.data[dstPath];
	srcPth = (AMPath *)currentContext->handles->createdHandlesList.data[srcPath];
	AM_ASSERT(dstPth);
	AM_ASSERT(srcPth);

	// check if paths have different format
	if (srcPth->format != dstPth->format) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgAppendPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	// only standard format is supported in this implementation
	if (srcPth->format != VG_PATH_FORMAT_STANDARD) {
		amCtxErrorSet(currentContext, VG_UNSUPPORTED_PATH_FORMAT_ERROR);
		AM_MEMORY_LOG("vgAppendPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	if ((srcPth->capabilities & VG_PATH_CAPABILITY_APPEND_FROM) && (dstPth->capabilities & VG_PATH_CAPABILITY_APPEND_TO)) {
		// do the effective path append operation
		if (!amPathAppend(dstPth, srcPth, currentContext)) {
			// try to retrieve as much memory as possbile
			amMemMngRetrieve(currentContext, AM_TRUE);
			// re-extract the path pointers, because they could be changed by amMemMngRetrieve
			dstPth = (AMPath *)currentContext->handles->createdHandlesList.data[dstPath];
			srcPth = (AMPath *)currentContext->handles->createdHandlesList.data[srcPath];
			// try to re-append the path
		if (!amPathAppend(dstPth, srcPth, currentContext))
			amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		else
			amCtxErrorSet(currentContext, VG_NO_ERROR);
	}
	else
			amCtxErrorSet(currentContext, VG_NO_ERROR);
	}
	else
		amCtxErrorSet(currentContext, VG_PATH_CAPABILITY_ERROR);
	AM_MEMORY_LOG("vgAppendPath");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief It appends specified data to the given destination path. The data are formatted using the destination
	path format of. Each incoming coordinate value, regardless of datatype, is transformed by the scale factor
	and bias of the destination path.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param dstPath the destination path.
	\param numSegments number of segments commands to append.
	\param pathSegments segments commands to append.
	\param pathData segments coordinates to append.
*/
VG_API_CALL void VG_API_ENTRY vgAppendPathData(VGPath dstPath,
                                               VGint numSegments,
                                               const VGubyte *pathSegments,
                                               const void *pathData) VG_API_EXIT {

	AMint32 i;
	AMPath *dstPth;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgAppendPathData");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check if the dstPath handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, dstPath) != AM_PATH_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgAppendPathData");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	dstPth = (AMPath *)currentContext->handles->createdHandlesList.data[dstPath];
	AM_ASSERT(dstPth);

	// check capabilities for append operations
	if (!(dstPth->capabilities & VG_PATH_CAPABILITY_APPEND_TO)) {
		amCtxErrorSet(currentContext, VG_PATH_CAPABILITY_ERROR);
		AM_MEMORY_LOG("vgAppendPathData");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	// check for illegal arguments
	if (!pathSegments || !pathData || numSegments <= 0) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgAppendPathData");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if ((dstPth->dataType == VG_PATH_DATATYPE_S_16 && !amPointerIsAligned(pathData, 2)) ||
		((dstPth->dataType == VG_PATH_DATATYPE_S_32 || dstPth->dataType == VG_PATH_DATATYPE_F) && !amPointerIsAligned(pathData, 4))) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgAppendPathData");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for illegal commands
	for (i = 0; i < numSegments; ++i) {

		VGubyte cmd = AM_PATH_SEGMENT_COMMAND(pathSegments[i]);

		if (cmd > VG_LCWARC_TO) {
			amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
			AM_MEMORY_LOG("vgAppendPathData");
			OPENVG_RETURN(OPENVG_NO_RETVAL)
		}
	}
	// do the effective data append
	if (!amPathDataAppend(dstPth, numSegments, pathSegments, pathData, dstPth->dataType, currentContext, AM_TRUE)) {
		// try to retrieve as much memory as possbile
		amMemMngRetrieve(currentContext, AM_TRUE);
		// re-extract the path pointer, because it could be changed by amMemMngRetrieve
		dstPth = (AMPath *)currentContext->handles->createdHandlesList.data[dstPath];
		// try to re-append path data
	if (!amPathDataAppend(dstPth, numSegments, pathSegments, pathData, dstPth->dataType, currentContext, AM_TRUE))
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
	else
		amCtxErrorSet(currentContext, VG_NO_ERROR);
	}
	else
		amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgAppendPathData");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief It modifies the coordinate data for a contiguous range of segments of destination path, starting
	at the specified index (where 0 is the index of the first path segment). Each incoming coordinate
	value, regardless of data type, is transformed by the scale factor and bias of the destination path.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param dstPath the destination path.
	\param startIndex index of the first path segment to modify.
	\param numSegments number of path segments to modify.
	\param pathData pointer to input coordinates.
*/
VG_API_CALL void VG_API_ENTRY vgModifyPathCoords(VGPath dstPath,
                                                 VGint startIndex,
                                                 VGint numSegments,
                                                 const void *pathData) VG_API_EXIT {

	AMPath *dstPth;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgModifyPathCoords");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check if the dstPath handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, dstPath) != AM_PATH_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgModifyPathCoords");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	dstPth = (AMPath *)currentContext->handles->createdHandlesList.data[dstPath];
	AM_ASSERT(dstPth);

	// check capabilities for append operations
	if (!(dstPth->capabilities & VG_PATH_CAPABILITY_MODIFY)) {
		amCtxErrorSet(currentContext, VG_PATH_CAPABILITY_ERROR);
		AM_MEMORY_LOG("vgModifyPathCoords");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	// check for illegal arguments
	if (numSegments <= 0 || startIndex < 0 || !pathData || (startIndex + numSegments) > (VGint)dstPth->segments.size) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgModifyPathCoords");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if ((dstPth->dataType == VG_PATH_DATATYPE_S_16 && !amPointerIsAligned(pathData, 2)) ||
		((dstPth->dataType == VG_PATH_DATATYPE_S_32 || dstPth->dataType == VG_PATH_DATATYPE_F) && !amPointerIsAligned(pathData, 4))) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgModifyPathCoords");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// do the effective data modification
	amPathCoordinatesModify(dstPth, startIndex, numSegments, pathData, currentContext);
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgModifyPathCoords");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief It appends a transformed copy of source path to the current contents of destination path.
	The appended path is equivalent to the results of applying the current path-user-to-surface transformation
	to the source path.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param dstPath the destination path.
	\param srcPath the source path.
*/
VG_API_CALL void VG_API_ENTRY vgTransformPath(VGPath dstPath,
                                              VGPath srcPath) VG_API_EXIT {

	const AMPath *srcPth;
	AMPath *dstPth;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgTransformPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check if the dstPath handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, dstPath) != AM_PATH_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgTransformPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	// check if the srcPath handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, srcPath) != AM_PATH_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgTransformPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	dstPth = (AMPath *)currentContext->handles->createdHandlesList.data[dstPath];
	srcPth = (const AMPath *)currentContext->handles->createdHandlesList.data[srcPath];
	AM_ASSERT(dstPth);
	AM_ASSERT(srcPth);

	// check capabilities for transform operations
	if (!(srcPth->capabilities & VG_PATH_CAPABILITY_TRANSFORM_FROM)) {
		amCtxErrorSet(currentContext, VG_PATH_CAPABILITY_ERROR);
		AM_MEMORY_LOG("vgTransformPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if (!(dstPth->capabilities & VG_PATH_CAPABILITY_TRANSFORM_TO)) {
		amCtxErrorSet(currentContext, VG_PATH_CAPABILITY_ERROR);
		AM_MEMORY_LOG("vgTransformPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if (!amPathTransform(dstPth, srcPth, currentContext)) {
		// try to retrieve as much memory as possbile
		amMemMngRetrieve(currentContext, AM_TRUE);
		// re-extract the path pointers, because they could be changed by amMemMngRetrieve
		dstPth = (AMPath *)currentContext->handles->createdHandlesList.data[dstPath];
		srcPth = (const AMPath *)currentContext->handles->createdHandlesList.data[srcPath];
		// try to re-transform the path
	if (!amPathTransform(dstPth, srcPth, currentContext))
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
	else
		amCtxErrorSet(currentContext, VG_NO_ERROR);
	}
	else
		amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgTransformPath");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief It appends a path, defined by interpolation (or extrapolation) between a start path and an end
	path by the given amount, to the destination path.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param dstPath the destination path.
	\param startPath input start path.
	\param endPath input end path.
	\param amount interpolation amount.
	\return VG_TRUE if interpolation was successful (i.e., the paths had compatible segment types after
	normalization), VG_FALSE otherwise.
	\note if interpolation is unsuccessful, destination path is left unchanged.
*/
VG_API_CALL VGboolean VG_API_ENTRY vgInterpolatePath(VGPath dstPath,
                                                     VGPath startPath,
                                                     VGPath endPath,
                                                     VGfloat amount) VG_API_EXIT {

	AMPath *dstPth;
	const AMPath *startPth;
	const AMPath *endPth;
	VGboolean res;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;
	AMbool pathsInterpolable;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgInterpolatePath");
		OPENVG_RETURN(VG_FALSE)
	}

	// check if the dstPath handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, dstPath) != AM_PATH_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgInterpolatePath");
		OPENVG_RETURN(VG_FALSE)
	}
	// check if the startPath handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, startPath) != AM_PATH_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgInterpolatePath");
		OPENVG_RETURN(VG_FALSE)
	}
	// check if the endPath handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, endPath) != AM_PATH_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgInterpolatePath");
		OPENVG_RETURN(VG_FALSE)
	}

	dstPth = (AMPath *)currentContext->handles->createdHandlesList.data[dstPath];
	startPth = (const AMPath *)currentContext->handles->createdHandlesList.data[startPath];
	endPth = (const AMPath *)currentContext->handles->createdHandlesList.data[endPath];
	AM_ASSERT(dstPth);
	AM_ASSERT(startPth);
	AM_ASSERT(endPth);

	// check capabilities for transform operations
	if (!(startPth->capabilities & VG_PATH_CAPABILITY_INTERPOLATE_FROM)) {
		amCtxErrorSet(currentContext, VG_PATH_CAPABILITY_ERROR);
		AM_MEMORY_LOG("vgInterpolatePath");
		OPENVG_RETURN(VG_FALSE)
	}
	if (!(endPth->capabilities & VG_PATH_CAPABILITY_INTERPOLATE_FROM)) {
		amCtxErrorSet(currentContext, VG_PATH_CAPABILITY_ERROR);
		AM_MEMORY_LOG("vgInterpolatePath");
		OPENVG_RETURN(VG_FALSE)
	}
	if (!(dstPth->capabilities & VG_PATH_CAPABILITY_INTERPOLATE_TO)) {
		amCtxErrorSet(currentContext, VG_PATH_CAPABILITY_ERROR);
		AM_MEMORY_LOG("vgInterpolatePath");
		OPENVG_RETURN(VG_FALSE)
	}

	// handle NaN and Inf values
	amount = amNanInfFix(amount);

	if (!amPathInterpolate(&pathsInterpolable, dstPth, startPth, endPth, amount, currentContext)) {
		// try to retrieve as much memory as possbile
		amMemMngRetrieve(currentContext, AM_TRUE);
		// re-extract the path pointers, because they could be changed by amMemMngRetrieve
		dstPth = (AMPath *)currentContext->handles->createdHandlesList.data[dstPath];
		startPth = (const AMPath *)currentContext->handles->createdHandlesList.data[startPath];
		endPth = (const AMPath *)currentContext->handles->createdHandlesList.data[endPath];
		// try to re-interpolate the path
		if (!amPathInterpolate(&pathsInterpolable, dstPth, startPth, endPth, amount, currentContext)) {
			amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
			AM_MEMORY_LOG("vgInterpolatePath");
			OPENVG_RETURN(VG_FALSE)
		}
	}

	res = pathsInterpolable ? VG_TRUE : VG_FALSE;
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgInterpolatePath");
	OPENVG_RETURN(res)
}

/*!
	\brief It returns the length of a given portion of a path in the user coordinate system (that is, in
	the path's own coordinate system, disregarding any matrix settings).\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param path input path whose length is to calculate.
	\param startSegment first path segment to consider during length calculation.
	\param numSegments number of path segments to consider during length calculation.
	\return the requested length if the operation was successful, else -1.0f.
*/
VG_API_CALL VGfloat VG_API_ENTRY vgPathLength(VGPath path,
                                              VGint startSegment,
                                              VGint numSegments) VG_API_EXIT {

	AMPath *pth;
	VGfloat res;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgPathLength");
		OPENVG_RETURN(-1.0f)
	}

	// check if the path handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, path) != AM_PATH_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgPathLength");
		OPENVG_RETURN(-1.0f)
	}

	pth = (AMPath *)currentContext->handles->createdHandlesList.data[path];
	AM_ASSERT(pth);

	// check capabilities for length calculus
	if (!(pth->capabilities & VG_PATH_CAPABILITY_PATH_LENGTH)) {
		amCtxErrorSet(currentContext, VG_PATH_CAPABILITY_ERROR);
		AM_MEMORY_LOG("vgPathLength");
		OPENVG_RETURN(-1.0f)
	}
	// check for illegal arguments
	if (startSegment < 0 || startSegment >= (VGint)pth->segments.size || numSegments <= 0) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgPathLength");
		OPENVG_RETURN(-1.0f)
	}
	if ((numSegments + startSegment - 1) < 0 || (numSegments + startSegment) > (VGint)pth->segments.size) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgPathLength");
		OPENVG_RETURN(-1.0f)
	}
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgPathLength");
	res = amPathLength(pth, startSegment, numSegments, currentContext);
	OPENVG_RETURN(res)
}

/*!
	\brief It returns the point lying a given distance along a given portion of a path and the unit-length
	tangent vector at that point.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param path input path to evaluate.
	\param startSegment first path segment to consider during evaluation.
	\param numSegments number of path segments to consider during evaluation.
	\param distance distance at which evaluate the path.
	\param x pointer to the evaluated x coordinate.
	\param y pointer to the evaluated y coordinate.
	\param tangentX pointer to the evaluated tangent x coordinate.
	\param tangentY pointer to the evaluated tangent y coordinate.
*/
VG_API_CALL void VG_API_ENTRY vgPointAlongPath(VGPath path,
                                               VGint startSegment,
                                               VGint numSegments,
                                               VGfloat distance,
                                               VGfloat *x,
                                               VGfloat *y,
                                               VGfloat *tangentX,
                                               VGfloat *tangentY) VG_API_EXIT {

	#define ASSIGN_NULL_VALUES \
		if (x) \
			*x = 0.0f; \
		if (y) \
			*y = 0.0f; \
		if (tangentX) \
			*tangentX = 0.0f; \
		if (tangentY) \
			*tangentY = 0.0f;

	AMPath *pth;
	AMfloat tmpX, tmpY, tmpTangentX, tmpTangentY;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgPointAlongPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check if the path handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, path) != AM_PATH_HANDLE_ID) {
		ASSIGN_NULL_VALUES
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgPointAlongPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	pth = (AMPath *)currentContext->handles->createdHandlesList.data[path];
	AM_ASSERT(pth);

	// check for capabilities errors
	if (x && y && !(pth->capabilities & VG_PATH_CAPABILITY_POINT_ALONG_PATH)) {
		ASSIGN_NULL_VALUES
		amCtxErrorSet(currentContext, VG_PATH_CAPABILITY_ERROR);
		AM_MEMORY_LOG("vgPointAlongPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if (tangentX && tangentY && !(pth->capabilities & VG_PATH_CAPABILITY_TANGENT_ALONG_PATH)) {
		ASSIGN_NULL_VALUES
		amCtxErrorSet(currentContext, VG_PATH_CAPABILITY_ERROR);
		AM_MEMORY_LOG("vgPointAlongPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	// check for illegal arguments
	if (startSegment < 0 || startSegment >= (VGint)pth->segments.size || numSegments <= 0) {
		ASSIGN_NULL_VALUES
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgPointAlongPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if ((numSegments + startSegment - 1) < 0 || (numSegments + startSegment) > (VGint)pth->segments.size) {
		ASSIGN_NULL_VALUES
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgPointAlongPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if (!amPointerIsAligned(x, 4) || !amPointerIsAligned(y, 4) || !amPointerIsAligned(tangentX, 4) || !amPointerIsAligned(tangentY, 4)) {
		ASSIGN_NULL_VALUES
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgPointAlongPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	amCtxErrorSet(currentContext, VG_NO_ERROR);
	// evaluate the path
	amPathEvaluate(&tmpX, &tmpY, &tmpTangentX, &tmpTangentY, pth, startSegment, numSegments, distance, currentContext);
	// assign return values
	if (x)
		*x = tmpX;
	if (y)
		*y = tmpY;
	if (tangentX)
		*tangentX = tmpTangentX;
	if (tangentY)
		*tangentY = tmpTangentY;
	
	AM_MEMORY_LOG("vgPointAlongPath");
	OPENVG_RETURN(OPENVG_NO_RETVAL)

	#undef ASSIGN_NULL_VALUES
}

/*!
	\brief It returns an axis-aligned bounding box that tightly bounds the interior of the given path.
	Stroking parameters are ignored.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param path input path whose box is to calculate.
	\param minX pointer to the minimum x coordinate of the box.
	\param minY pointer to the minimum y coordinate of the box.
	\param width pointer to the width of the box.
	\param height pointer to the height of the box.
*/
VG_API_CALL void VG_API_ENTRY vgPathBounds(VGPath path,
                                           VGfloat *minX,
                                           VGfloat *minY,
                                           VGfloat *width,
                                           VGfloat *height) VG_API_EXIT {

	AMPath *pth;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgPathBounds");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check if the path handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, path) != AM_PATH_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgPathBounds");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	pth = (AMPath *)currentContext->handles->createdHandlesList.data[path];
	AM_ASSERT(pth);

	// check for capabilities errors
	if (!(pth->capabilities & VG_PATH_CAPABILITY_PATH_BOUNDS)) {
		amCtxErrorSet(currentContext, VG_PATH_CAPABILITY_ERROR);
		AM_MEMORY_LOG("vgPathBounds");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	// check for illegal arguments
	if (!minX || !minY || !width || !height) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgPathBounds");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if (!amPointerIsAligned(minX, 4) || !amPointerIsAligned(minY, 4) || !amPointerIsAligned(width, 4) || !amPointerIsAligned(height, 4)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgPathBounds");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	amCtxErrorSet(currentContext, VG_NO_ERROR);
	amPathBounds(minX, minY, width, height, pth, currentContext);
	AM_MEMORY_LOG("vgPathBounds");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief It returns an axis-aligned bounding box that is guaranteed to enclose the geometry of the given
	path following transformation by the current path-user-to-surface transform.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param path input path whose box is to calculate.
	\param minX pointer to the minimum x coordinate of the box.
	\param minY pointer to the minimum y coordinate of the box.
	\param width pointer to the width of the box.
	\param height pointer to the height of the box.
*/
VG_API_CALL void VG_API_ENTRY vgPathTransformedBounds(VGPath path,
                                                      VGfloat *minX,
                                                      VGfloat *minY,
                                                      VGfloat *width,
                                                      VGfloat *height) VG_API_EXIT {

	AMPath *pth;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgPathTransformedBounds");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check if the path handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, path) != AM_PATH_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgPathTransformedBounds");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	pth = (AMPath *)currentContext->handles->createdHandlesList.data[path];
	AM_ASSERT(pth);

	// check for capabilities errors
	if (!(pth->capabilities & VG_PATH_CAPABILITY_PATH_TRANSFORMED_BOUNDS)) {
		amCtxErrorSet(currentContext, VG_PATH_CAPABILITY_ERROR);
		AM_MEMORY_LOG("vgPathTransformedBounds");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	// check for illegal arguments
	if (!minX || !minY || !width || !height) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgPathTransformedBounds");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if (!amPointerIsAligned(minX, 4) || !amPointerIsAligned(minY, 4) || !amPointerIsAligned(width, 4) || !amPointerIsAligned(height, 4)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgPathTransformedBounds");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	amCtxErrorSet(currentContext, VG_NO_ERROR);
	amPathTransformedBounds(minX, minY, width, height, pth, currentContext);
	AM_MEMORY_LOG("vgPathTransformedBounds");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief Draw the given path.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param path input path to draw.
	\param paintModes a bitwise OR of values from the VGPaintMode enumeration, determining whether the path
	is to be filled (VG_FILL_PATH), stroked (VG_STROKE_PATH), or both (VG_FILL_PATH | VG_STROKE_PATH).
*/
VG_API_CALL void VG_API_ENTRY vgDrawPath(VGPath path,
                                         VGbitfield paintModes) VG_API_EXIT {

	AMPath *pth;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgDrawPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check if the path handle is valid and shared with the current context
	if (amCtxHandleValid(currentContext, path) != AM_PATH_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgDrawPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	// check for illegal arguments
	if (paintModes == 0 || paintModes > (VG_STROKE_PATH | VG_FILL_PATH)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgDrawPath");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	pth = (AMPath *)currentContext->handles->createdHandlesList.data[path];
	AM_ASSERT(pth);

	if (!amPathDraw(currentContext, currentSurface, pth, paintModes)) {
		AM_MEMORY_LOG("vgDrawPath   (amPathDraw fail, now try to recover memory)");
		// try to retrieve as much memory as possbile
		amMemMngRetrieve(currentContext, AM_TRUE);
		if (currentContext->beforePathRasterization) {
			// re-extract the path pointer, because it could be changed by amMemMngRetrieve
			pth = (AMPath *)currentContext->handles->createdHandlesList.data[path];
			// try to re-draw the path if the "out of memory" is not due to rasterization
			if (!amPathDraw(currentContext, currentSurface, pth, paintModes)) {
				#if defined(AM_DEBUG_MEMORY)
					amCtxCheckConsistence(currentContext);
				#endif
				amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
				AM_MEMORY_LOG("vgDrawPath   (amPathDraw fail, before rasterization)");
				OPENVG_RETURN(OPENVG_NO_RETVAL)
			}
		}
		else {
			// "out of memory" is due to rasterization, so exit with a memory error
			amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
			AM_MEMORY_LOG("vgDrawPath   (amPathDraw fail, after rasterization)");
			OPENVG_RETURN(OPENVG_NO_RETVAL)
		}
	}
	// decrement the counter of the memory manager
	amCtxMemMngCountDown(currentContext);

	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgDrawPath");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

#undef AM_PATH_SEGMENT_COMMAND
#undef AM_PATH_SEGMENT_ABSREL

#if defined (RIM_VG_SRC)
#pragma pop
#endif
