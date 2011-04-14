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

#ifndef _DYNARRAY_H
#define _DYNARRAY_H

/*!
	\file dynarray.h
	\brief Dynamic arrays, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "stdlib_abstraction.h"
#include "vector.h"

#define AM_DYNARRAY_NO_ERROR 0
#define AM_DYNARRAY_OUT_OF_MEMORY 1

#define AM_DYNARRAY_EXPANSION 64

//! Dynamic array declaration.
#define AM_DYNARRAY_DECLARE(_typeName, _itemTypeName, _tmpStructName) \
	typedef struct _tmpStructName { \
		_itemTypeName *tmpData; \
		_itemTypeName *data; \
		AMuint32 size; \
		AMuint32 capacity; \
		AMuint32 error; \
	} _typeName;

//! Simply set all fields to "empty"
#define AM_DYNARRAY_PREINIT(_dynArray) \
	(_dynArray).tmpData = NULL; \
	(_dynArray).data = NULL; \
	(_dynArray).size = 0; \
	(_dynArray).capacity = 0; \
	(_dynArray).error = AM_DYNARRAY_NO_ERROR;

//! Dynamic array initialization.
#define AM_DYNARRAY_INIT(_dynArray, _itemTypeName) \
	(_dynArray).tmpData = NULL; \
	(_dynArray).data = (_itemTypeName *)amMalloc(sizeof(_itemTypeName)); \
	if ((_dynArray).data) { \
		(_dynArray).capacity = 1; \
		(_dynArray).error = AM_DYNARRAY_NO_ERROR; \
	} \
	else { \
		(_dynArray).capacity = 0; \
		(_dynArray).error = AM_DYNARRAY_OUT_OF_MEMORY; \
	} \
	(_dynArray).size = 0;

//! Dynamic array initialization, reserving memory space.
#define AM_DYNARRAY_INIT_RESERVE(_dynArray, _itemTypeName, _capacity) \
	(_dynArray).tmpData = NULL; \
	(_dynArray).data = (_itemTypeName *)amMalloc((_capacity) * sizeof(_itemTypeName)); \
	if ((_dynArray).data) { \
		(_dynArray).capacity = (_capacity); \
		(_dynArray).error = AM_DYNARRAY_NO_ERROR; \
	} \
	else { \
		(_dynArray).capacity = 0; \
		(_dynArray).error = AM_DYNARRAY_OUT_OF_MEMORY; \
	} \
	(_dynArray).size = 0;

//! Dynamic array destruction.
#define AM_DYNARRAY_DESTROY(_dynArray) \
	(_dynArray).tmpData = NULL; \
	if ((_dynArray).data) \
		amFree((_dynArray).data); \
	(_dynArray).data = NULL; \
	(_dynArray).size = 0; \
	(_dynArray).capacity = 0; \
	(_dynArray).error = AM_DYNARRAY_NO_ERROR;


//! Dynamic array clear (empty).
#define AM_DYNARRAY_CLEAR(_dynArray, _itemTypeName) \
	(_dynArray).tmpData = (_itemTypeName *)amRealloc((_dynArray).data, sizeof(_itemTypeName)); \
	if ((_dynArray).tmpData) { \
		(_dynArray).size = 0; \
		(_dynArray).capacity = 1; \
		(_dynArray).data = (_dynArray).tmpData; \
	} \
	else { \
		(_dynArray).error = AM_DYNARRAY_OUT_OF_MEMORY; \
	}

//! Dynamic array clear (empty), reserving memory space.
#define AM_DYNARRAY_CLEAR_RESERVE(_dynArray, _itemTypeName, _capacity) \
	(_dynArray).tmpData = (_itemTypeName *)amRealloc((_dynArray).data, (_capacity) * sizeof(_itemTypeName)); \
	if ((_dynArray).tmpData) { \
		(_dynArray).size = 0; \
		(_dynArray).capacity = (_capacity); \
		(_dynArray).data = (_dynArray).tmpData; \
	} \
	else { \
		(_dynArray).error = AM_DYNARRAY_OUT_OF_MEMORY; \
	} \

//! Get dynamic array capacity.
#define AM_DYNARRAY_CAPACITY(_dynArray) ((_dynArray).capacity)

//! Get dynamic array size (number of elements).
#define AM_DYNARRAY_SIZE(_dynArray) ((_dynArray).size)

//! Dynamic array reserve memory space.
#define AM_DYNARRAY_RESERVE(_dynArray, _itemTypeName, _newSize) \
	(_dynArray).tmpData = (_itemTypeName *)amRealloc((_dynArray).data, (_newSize) * sizeof(_itemTypeName)); \
	if ((_dynArray).tmpData) { \
		(_dynArray).size = (_newSize); \
		(_dynArray).capacity = (_newSize); \
		(_dynArray).data = (_dynArray).tmpData; \
	} \
	else { \
		(_dynArray).error = AM_DYNARRAY_OUT_OF_MEMORY; \
	}
	
//! Push an element at the end of a dynamic array; if space is not enought, the array is expanded by additional AM_DYNARRAY_EXPANSION elememts memory space.
#define AM_DYNARRAY_PUSH_BACK(_dynArray, _itemTypeName, _val) \
	if ((_dynArray).size >= (_dynArray).capacity) { \
		(_dynArray).tmpData = (_itemTypeName *)amRealloc((_dynArray).data, ((_dynArray).capacity + AM_DYNARRAY_EXPANSION) * sizeof(_itemTypeName)); \
		if ((_dynArray).tmpData) { \
			(_dynArray).data = (_dynArray).tmpData; \
			(_dynArray).capacity += AM_DYNARRAY_EXPANSION; \
			(_dynArray).data[(_dynArray).size++] = (_val); \
		} \
		else { \
			(_dynArray).error = AM_DYNARRAY_OUT_OF_MEMORY; \
		} \
	} \
	else { \
		(_dynArray).data[(_dynArray).size++] = (_val); \
	}

//! Push an element at the end of a dynamic array, without to check for available space; this function is not safe, use with care.
#define AM_DYNARRAY_PUSH_BACK_LIGHT(_dynArray, _val) \
	(_dynArray).data[(_dynArray).size++] = (_val);

//! Remove the last element from the dynamic array; it doesn't resize the memory.
#define AM_DYNARRAY_POP_BACK(_dynArray) \
	if ((_dynArray).size > 0) \
		(_dynArray).size--;

//! Get an element from a dynamic array.
#define AM_DYNARRAY_GET(_dynArray, _index) (((_index) < (_dynArray).size) ? ((_dynArray).data[(_index)]) : (NULL))

//! Set an element inside a dynamic array.
#define AM_DYNARRAY_SET(_dynArray, _index, _val) \
	if ((_index) >= 0 && (_index) < (_dynArray).size) \
		(_dynArray).data[_index] = (_val);

//! Return the first dynamic array element.
#define AM_DYNARRAY_FRONT(_dynArray) ((_dynArray).data[0])

//! Return the last dynamic array element.
#define AM_DYNARRAY_BACK(_dynArray) ((((_dynArray).size) > (0)) ? ((_dynArray).data[(_dynArray).size - 1]) : (NULL))

//! Insert a new element inside a dynamic array.
#define AM_DYNARRAY_INSERT(_dynArray, _itemTypeName, _index, _val) \
	if ((_index) >= 0 && (_index) <= (AMint32)((_dynArray).size)) { \
		if ((_dynArray).size >= (_dynArray).capacity) { \
			(_dynArray).tmpData = (_itemTypeName *)amRealloc((_dynArray).data, ((_dynArray).capacity + AM_DYNARRAY_EXPANSION) * sizeof(_itemTypeName)); \
			if ((_dynArray).tmpData) { \
				AMint32 _i; \
				(_dynArray).data = (_dynArray).tmpData; \
				(_dynArray).capacity += AM_DYNARRAY_EXPANSION; \
				for (_i = (AMint32)((_dynArray).size) - 1; _i >= (AMint32)(_index); _i--) \
					(_dynArray).data[_i + 1] = (_dynArray).data[_i]; \
				(_dynArray).data[_index] = (_val); \
				(_dynArray).size++; \
			} \
			else { \
				(_dynArray).error = AM_DYNARRAY_OUT_OF_MEMORY; \
			} \
		} \
		else { \
			AMint32 _i; \
			for (_i = (AMint32)((_dynArray).size) - 1; _i >= (AMint32)(_index); _i--) \
				(_dynArray).data[_i + 1] = (_dynArray).data[_i]; \
			(_dynArray).data[_index] = (_val); \
			(_dynArray).size++; \
		} \
	}

//! Remove an element from a dynamic array.
#define AM_DYNARRAY_ERASE(_dynArray, _index) \
	if ((_index) < (_dynArray).size) { \
		AMint32 _i; \
		for (_i = (AMint32)(_index); _i <= (AMint32)((_dynArray).size) - 2; ++_i) \
			(_dynArray).data[_i] = (_dynArray).data[_i + 1]; \
		(_dynArray).size--; \
	}

//! Retrieve memory from a dynamic array, unpreserving data
#define AM_DYNARRAY_UNPRESERVING_DATA_MEMORY_RETRIEVE(_dynArray, _itemTypeName) \
	if ((_dynArray).data && ((_dynArray).capacity > 1)) { \
		AM_DYNARRAY_CLEAR(_dynArray, _itemTypeName) \
	} \
	(_dynArray).size = 0;

//! Retrieve memory from a dynamic array, preserving data
#define AM_DYNARRAY_PRESERVING_DATA_MEMORY_RETRIEVE(_dynArray, _itemTypeName) \
	if ((_dynArray).data && ((_dynArray).capacity - (_dynArray).size > 1)) { \
		AMuint32 _curSize = (_dynArray).size; \
		AMuint32 _newSize = AM_MAX(1, _curSize); \
		AM_DYNARRAY_RESERVE(_dynArray, _itemTypeName, _newSize) \
		(_dynArray).size = _curSize; \
	}

AM_DYNARRAY_DECLARE(AMBoolDynArray, AMbool, _AMBoolDynArray)
AM_DYNARRAY_DECLARE(AMUint8DynArray, AMuint8, _AMUint8DynArray)
AM_DYNARRAY_DECLARE(AMUint16DynArray, AMuint16, _AMUint16DynArray)
AM_DYNARRAY_DECLARE(AMUint32DynArray, AMuint32, _AMUint32DynArray)
AM_DYNARRAY_DECLARE(AMInt8DynArray, AMint8, _AMInt8DynArray)
AM_DYNARRAY_DECLARE(AMInt16DynArray, AMint16, _AMInt16DynArray)
AM_DYNARRAY_DECLARE(AMInt32DynArray, AMint32, _AMInt32DynArray)
AM_DYNARRAY_DECLARE(AMFloatDynArray, AMfloat, _AMFloatDynArray)
AM_DYNARRAY_DECLARE(AMVect2fDynArray, AMVect2f, _AMVect2fDynArray)
AM_DYNARRAY_DECLARE(AMVect2xDynArray, AMVect2x, _AMVect2xDynArray)
AM_DYNARRAY_DECLARE(AMHandleDynArray, AMhandle, _AMHandleDynArray)

#endif
