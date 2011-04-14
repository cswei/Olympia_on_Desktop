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
	\file vgimage.c
	\brief OpenVG images, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "vgdrawingsurface.h"
#include "vgprimitives.h"
#include "vgpaint.h"
#include "vgconversions.h"
#include "pixel_utils.h"

#if defined RIM_VG_SRC
#define VG_API_ENTRY 
#endif


// [0] = RED shift
// [1] = GREEN shift
// [2] = BLUE shift
// [3] = ALPHA shift
// [4] = BITS count
// [5] = RED bits
// [6] = GREEN bits
// [7] = BLUE bits
// [8] = ALPHA bits
// [9] = ORDER
// [10] = format FLAGS
const AMint32 pxlFormatTable[AM_IMAGE_FORMATS_COUNT][FMT_FLAGS - FMT_R_SH + 1] = {
	{ 24, 16,  8,  0, 32, 8, 8, 8, 8, AM_PIXEL_FMT_RGBA, 0 },							// VG_sRGBX_8888
	{ 24, 16,  8,  0, 32, 8, 8, 8, 8, AM_PIXEL_FMT_RGBA, FMT_ALPHA },					// VG_sRGBA_8888
	{ 24, 16,  8,  0, 32, 8, 8, 8, 8, AM_PIXEL_FMT_RGBA, FMT_PRE | FMT_ALPHA },			// VG_sRGBA_8888_PRE
	{ 11,  5,  0,  0, 16, 5, 6, 5, 0, AM_PIXEL_FMT_RGBA, 0 },							// VG_sRGB_565
	{ 11,  6,  1,  0, 16, 5, 5, 5, 1, AM_PIXEL_FMT_RGBA, FMT_ALPHA },					// VG_sRGBA_5551
	{ 12,  8,  4,  0, 16, 4, 4, 4, 4, AM_PIXEL_FMT_RGBA, FMT_ALPHA },					// VG_sRGBA_4444
	{  0,  0,  0,  0,  8, 0, 0, 0, 0, AM_PIXEL_FMT_RGBA, 0 },							// VG_sL_8
	{ 24, 16,  8,  0, 32, 8, 8, 8, 8, AM_PIXEL_FMT_RGBA, FMT_L },						// VG_lRGBX_8888
	{ 24, 16,  8,  0, 32, 8, 8, 8, 8, AM_PIXEL_FMT_RGBA, FMT_L | FMT_ALPHA },			// VG_lRGBA_8888
	{ 24, 16,  8,  0, 32, 8, 8, 8, 8, AM_PIXEL_FMT_RGBA, FMT_L | FMT_PRE | FMT_ALPHA },	// VG_lRGBA_8888_PRE
	{  0,  0,  0,  0,  8, 0, 0, 0, 0, AM_PIXEL_FMT_RGBA, FMT_L },						// VG_lL_8
	{  0,  0,  0,  0,  8, 0, 0, 0, 8, AM_PIXEL_FMT_RGBA, FMT_ALPHA },					// VG_A_8
	{  0,  0,  0,  0,  1, 0, 0, 0, 0, AM_PIXEL_FMT_RGBA, FMT_L },						// VG_BW_1
#if (AM_OPENVG_VERSION >= 110)
	{  0,  0,  0,  0,  1, 0, 0, 0, 1, AM_PIXEL_FMT_RGBA, FMT_ALPHA },					// VG_A_1
	{  0,  0,  0,  0,  4, 0, 0, 0, 4, AM_PIXEL_FMT_RGBA, FMT_ALPHA },					// VG_A_4
#endif
	// {A,X}RGB channel ordering
	{ 16,  8,  0, 24, 32, 8, 8, 8, 8, AM_PIXEL_FMT_ARGB, 0 },							// VG_sXRGB_8888
	{ 16,  8,  0, 24, 32, 8, 8, 8, 8, AM_PIXEL_FMT_ARGB, FMT_ALPHA },					// VG_sARGB_8888
	{ 16,  8,  0, 24, 32, 8, 8, 8, 8, AM_PIXEL_FMT_ARGB, FMT_PRE | FMT_ALPHA },			// VG_sARGB_8888_PRE
	{  0,  0,  0,  0,  0, 0, 0, 0, 0, AM_PIXEL_FMT_ARGB, 0 },							// ----
	{ 10,  5,  0, 15, 16, 5, 5, 5, 1, AM_PIXEL_FMT_ARGB, FMT_ALPHA },					// VG_sARGB_1555
	{  8,  4,  0, 12, 16, 4, 4, 4, 4, AM_PIXEL_FMT_ARGB, FMT_ALPHA },					// VG_sARGB_4444
	{  0,  0,  0,  0,  0, 0, 0, 0, 0, AM_PIXEL_FMT_ARGB, 0 },							// ----
	{ 16,  8,  0, 24, 32, 8, 8, 8, 8, AM_PIXEL_FMT_ARGB, FMT_L },						// VG_lXRGB_8888
	{ 16,  8,  0, 24, 32, 8, 8, 8, 8, AM_PIXEL_FMT_ARGB, FMT_L | FMT_ALPHA },			// VG_lARGB_8888
	{ 16,  8,  0, 24, 32, 8, 8, 8, 8, AM_PIXEL_FMT_ARGB, FMT_L | FMT_PRE | FMT_ALPHA },	// VG_lARGB_8888_PRE
	{  0,  0,  0,  0,  0, 0, 0, 0, 0, AM_PIXEL_FMT_ARGB, 0 },							// ----
	{  0,  0,  0,  0,  0, 0, 0, 0, 0, AM_PIXEL_FMT_ARGB, 0 },							// ----
	{  0,  0,  0,  0,  0, 0, 0, 0, 0, AM_PIXEL_FMT_ARGB, 0 },							// ----
#if (AM_OPENVG_VERSION >= 110)
	{  0,  0,  0,  0,  0, 0, 0, 0, 0, AM_PIXEL_FMT_ARGB, 0 },							// ----
	{  0,  0,  0,  0,  0, 0, 0, 0, 0, AM_PIXEL_FMT_ARGB, 0 },							// ----
#endif
	// BGR{A,X} channel ordering
	{  8, 16, 24,  0, 32, 8, 8, 8, 8, AM_PIXEL_FMT_BGRA, 0 },							// VG_sBGRX_8888
	{  8, 16, 24,  0, 32, 8, 8, 8, 8, AM_PIXEL_FMT_BGRA, FMT_ALPHA },					// VG_sBGRA_8888
	{  8, 16, 24,  0, 32, 8, 8, 8, 8, AM_PIXEL_FMT_BGRA, FMT_PRE | FMT_ALPHA },			// VG_sBGRA_8888_PRE
	{  0,  5, 11,  0, 16, 5, 6, 5, 0, AM_PIXEL_FMT_BGRA, 0 },							// VG_sBGR_565
	{  1,  6, 11,  0, 16, 5, 5, 5, 1, AM_PIXEL_FMT_BGRA, FMT_ALPHA },					// VG_sBGRA_5551
	{  4,  8, 12,  0, 16, 4, 4, 4, 4, AM_PIXEL_FMT_BGRA, FMT_ALPHA },					// VG_sBGRA_4444
	{  0,  0,  0,  0,  0, 0, 0, 0, 0, AM_PIXEL_FMT_BGRA, 0 },							// ----
	{  8, 16, 24,  0, 32, 8, 8, 8, 8, AM_PIXEL_FMT_BGRA, FMT_L },						// VG_lBGRX_8888
	{  8, 16, 24,  0, 32, 8, 8, 8, 8, AM_PIXEL_FMT_BGRA, FMT_L | FMT_ALPHA },			// VG_lBGRA_8888
	{  8, 16, 24,  0, 32, 8, 8, 8, 8, AM_PIXEL_FMT_BGRA, FMT_L | FMT_PRE | FMT_ALPHA },	// VG_lBGRA_8888_PRE
	{  0,  0,  0,  0,  0, 0, 0, 0, 0, AM_PIXEL_FMT_BGRA, 0 },							// ----
	{  0,  0,  0,  0,  0, 0, 0, 0, 0, AM_PIXEL_FMT_BGRA, 0 },							// ----
	{  0,  0,  0,  0,  0, 0, 0, 0, 0, AM_PIXEL_FMT_BGRA, 0 },							// ----
#if (AM_OPENVG_VERSION >= 110)
	{  0,  0,  0,  0,  0, 0, 0, 0, 0, AM_PIXEL_FMT_BGRA, 0 },							// ----
	{  0,  0,  0,  0,  0, 0, 0, 0, 0, AM_PIXEL_FMT_BGRA, 0 },							// ----
#endif
	// {A,X}BGR channel ordering
	{  0,  8, 16, 24, 32, 8, 8, 8, 8, AM_PIXEL_FMT_ABGR, 0 },							// VG_sXBGR_8888
	{  0,  8, 16, 24, 32, 8, 8, 8, 8, AM_PIXEL_FMT_ABGR, FMT_ALPHA },					// VG_sABGR_8888
	{  0,  8, 16, 24, 32, 8, 8, 8, 8, AM_PIXEL_FMT_ABGR, FMT_PRE | FMT_ALPHA },			// VG_sABGR_8888_PRE
	{  0,  0,  0,  0,  0, 0, 0, 0, 0, AM_PIXEL_FMT_ABGR, 0 },							// ----
	{  0,  5, 10, 11, 16, 5, 5, 5, 1, AM_PIXEL_FMT_ABGR, FMT_ALPHA },					// VG_sABGR_1555
	{  0,  4,  8, 12, 16, 4, 4, 4, 4, AM_PIXEL_FMT_ABGR, FMT_ALPHA },					// VG_sABGR_4444
	{  0,  0,  0,  0,  0, 0, 0, 0, 0, AM_PIXEL_FMT_ABGR, 0 },							// ----
	{  0,  8, 16, 24, 32, 8, 8, 8, 8, AM_PIXEL_FMT_ABGR, FMT_L },						// VG_lXBGR_8888
	{  0,  8, 16, 24, 32, 8, 8, 8, 8, AM_PIXEL_FMT_ABGR, FMT_L | FMT_ALPHA },			// VG_lABGR_8888
	{  0,  8, 16, 24, 32, 8, 8, 8, 8, AM_PIXEL_FMT_ABGR, FMT_L | FMT_PRE | FMT_ALPHA },	// VG_lABGR_8888_PRE
	{  0,  0,  0,  0,  0, 0, 0, 0, 0, AM_PIXEL_FMT_ABGR, 0 },							// ----
	{  0,  0,  0,  0,  0, 0, 0, 0, 0, AM_PIXEL_FMT_ABGR, 0 },							// ----
	{  0,  0,  0,  0,  0, 0, 0, 0, 0, AM_PIXEL_FMT_ABGR, 0 }							// ----
#if (AM_OPENVG_VERSION >= 110)
  , {  0,  0,  0,  0,  0, 0, 0, 0, 0, AM_PIXEL_FMT_ABGR, 0 },							// ----
	{  0,  0,  0,  0,  0, 0, 0, 0, 0, AM_PIXEL_FMT_ABGR, 0 }							// ----
#endif
};


//*************************************************************************************
//         Pixel samplers, they return always a 32bit color (low level functions)
//*************************************************************************************

//! Get a 32bit image sample at specified coordinates, according to the given tiling mode.
#define AM_SAMPLE32_GET(_res, _x, _y, _tilingMode, _pixels) \
	if ((((AMuint32)(_x)) < (AMuint32)image->width) && (((AMuint32)(_y)) < (AMuint32)image->height)) { \
		_res.value = (_pixels)[((_y) + childY) * stride + ((_x) + childX)]; \
	} \
	else { \
		if ((_tilingMode) == VG_TILE_FILL) \
			_res.value = params->tileFillColor; \
		else { \
			AMint32 tmpX, tmpY, cycles; \
			switch (_tilingMode) { \
				case VG_TILE_PAD: \
					tmpX = AM_CLAMP((_x), 0, image->width - 1); \
					tmpY = AM_CLAMP((_y), 0, image->height - 1); \
					break; \
				case VG_TILE_REPEAT: \
					tmpX = (_x); \
					if (((AMuint32)(tmpX)) >= (AMuint32)image->width) { \
						tmpX = tmpX % image->width; \
						if (tmpX < 0) tmpX += image->width; \
					} \
					tmpY = (_y); \
					if (((AMuint32)(tmpY)) >= (AMuint32)image->height) { \
						tmpY = tmpY % image->height; \
						if (tmpY < 0) tmpY += image->height; \
					} \
					break; \
				default: \
					AM_ASSERT(_tilingMode == VG_TILE_REFLECT); \
					tmpX = (_x); \
					if (((AMuint32)(tmpX)) >= (AMuint32)image->width) { \
						cycles = (tmpX < 0) ? (((-tmpX - 1) / image->width) + 1) : (tmpX / image->width); \
						tmpX = tmpX % image->width; \
						if (cycles & 1) { \
							tmpX = image->width - 1 - tmpX; \
							if (tmpX >= image->width) tmpX -= image->width; \
						} \
						else { \
							if (tmpX < 0) tmpX += image->width; \
						} \
					} \
					tmpY = (_y); \
					if (((AMuint32)(tmpY)) >= (AMuint32)image->height) { \
						cycles = (tmpY < 0) ? (((-tmpY - 1) / image->height) + 1) : (tmpY / image->height); \
						tmpY = tmpY % image->height; \
						if (cycles & 1) { \
							tmpY = image->height - 1 - tmpY; \
							if (tmpY >= image->height) tmpY -= image->height; \
						} \
						else { \
							if (tmpY < 0) tmpY += image->height; \
						} \
					} \
					break; \
			} \
			_res.value = (_pixels)[(tmpY + childY) * stride + (tmpX + childX)]; \
		} \
	}

//! Get a 32bit image sample at specified coordinates, according to the given tiling mode. Specific for power of two dimensions images.
#define AM_SAMPLE32_POW2_GET(_res, _x, _y, _tilingMode, _pixels) \
	if ((((AMuint32)(_x)) < (AMuint32)image->width) && (((AMuint32)(_y)) < (AMuint32)image->height)) { \
		_res.value = (_pixels)[((_y) + childY) * stride + ((_x) + childX)]; \
	} \
	else { \
		if ((_tilingMode) == VG_TILE_FILL) \
			_res.value = params->tileFillColor; \
		else { \
			AMint32 tmpX, tmpY, cycles; \
			switch (_tilingMode) { \
				case VG_TILE_PAD: \
					tmpX = AM_CLAMP((_x), 0, image->width - 1); \
					tmpY = AM_CLAMP((_y), 0, image->height - 1); \
					break; \
				case VG_TILE_REPEAT: \
					tmpX = (_x) & (image->width - 1); \
					tmpY = (_y) & (image->height - 1); \
					break; \
				default: \
					AM_ASSERT(_tilingMode == VG_TILE_REFLECT); \
					tmpX = (_x); \
					if (((AMuint32)(tmpX)) >= (AMuint32)image->width) { \
						cycles = (tmpX < 0) ? (((-tmpX - 1) >> image->widthShift) + 1) : (tmpX >> image->widthShift); \
						tmpX &= image->width - 1; \
						if (cycles & 1) tmpX = image->width - 1 - tmpX; \
					} \
					tmpY = (_y); \
					if (((AMuint32)(tmpY)) >= (AMuint32)image->height) { \
						cycles = (tmpY < 0) ? (((-tmpY - 1) >> image->heightShift) + 1) : (tmpY >> image->heightShift); \
						tmpY &= image->height - 1; \
						if (cycles & 1) tmpY = image->height - 1 - tmpY; \
					} \
					break; \
			} \
			_res.value = (_pixels)[(tmpY + childY) * stride + (tmpX + childX)]; \
		} \
	}

//! Get a 16bit image sample at specified coordinates, according to the given tiling mode.
#define AM_SAMPLE16_GET(_res, _x, _y, _tilingMode, _pixels) \
	if ((((AMuint32)(_x)) < (AMuint32)image->width) && (((AMuint32)(_y)) < (AMuint32)image->height)) { \
		rgba16 = (_pixels)[((_y) + childY) * stride + ((_x) + childX)]; \
		(_res).c.b = (bMap * ((rgba16 >> src_bSh) & src_bMask)) >> 8; \
		(_res).c.g = (gMap * ((rgba16 >> src_gSh) & src_gMask)) >> 8; \
		(_res).c.r = (rMap * ((rgba16 >> src_rSh) & src_rMask)) >> 8; \
		if (!(src_flags & FMT_ALPHA)) \
			(_res).c.a = 0xFF; \
		else \
			(_res).c.a = (aMap * ((rgba16 >> src_aSh) & src_aMask)) >> 8; \
	} \
	else { \
		if ((_tilingMode) == VG_TILE_FILL) \
			_res.value = params->tileFillColor; \
		else { \
			AMint32 tmpX, tmpY, cycles; \
			switch (_tilingMode) { \
				case VG_TILE_PAD: \
					tmpX = AM_CLAMP((_x), 0, image->width - 1); \
					tmpY = AM_CLAMP((_y), 0, image->height - 1); \
					break; \
				case VG_TILE_REPEAT: \
					tmpX = (_x); \
					if (((AMuint32)(tmpX)) >= (AMuint32)image->width) { \
						tmpX = tmpX % image->width; \
						if (tmpX < 0) tmpX += image->width; \
					} \
					tmpY = (_y); \
					if (((AMuint32)(tmpY)) >= (AMuint32)image->height) { \
						tmpY = tmpY % image->height; \
						if (tmpY < 0) tmpY += image->height; \
					} \
					break; \
				default: \
					AM_ASSERT(_tilingMode == VG_TILE_REFLECT); \
					tmpX = (_x); \
					if (((AMuint32)(tmpX)) >= (AMuint32)image->width) { \
						cycles = (tmpX < 0) ? (((-tmpX - 1) / image->width) + 1) : (tmpX / image->width); \
						tmpX = tmpX % image->width; \
						if (cycles & 1) { \
							tmpX = image->width - 1 - tmpX; \
							if (tmpX >= image->width) tmpX -= image->width; \
						} \
						else { \
							if (tmpX < 0) tmpX += image->width; \
						} \
					} \
					tmpY = (_y); \
					if (((AMuint32)(tmpY)) >= (AMuint32)image->height) { \
						cycles = (tmpY < 0) ? (((-tmpY - 1) / image->height) + 1) : (tmpY / image->height); \
						tmpY = tmpY % image->height; \
						if (cycles & 1) { \
							tmpY = image->height - 1 - tmpY; \
							if (tmpY >= image->height) tmpY -= image->height; \
						} \
						else { \
							if (tmpY < 0) tmpY += image->height; \
						} \
					} \
					break; \
			} \
			rgba16 = (_pixels)[((tmpY) + childY) * stride + ((tmpX) + childX)]; \
			(_res).c.b = (bMap * ((rgba16 >> src_bSh) & src_bMask)) >> 8; \
			(_res).c.g = (gMap * ((rgba16 >> src_gSh) & src_gMask)) >> 8; \
			(_res).c.r = (rMap * ((rgba16 >> src_rSh) & src_rMask)) >> 8; \
			if (!(src_flags & FMT_ALPHA)) \
				(_res).c.a = 0xFF; \
			else \
				(_res).c.a = (aMap * ((rgba16 >> src_aSh) & src_aMask)) >> 8; \
		} \
	}

//! Get a 16bit image sample at specified coordinates, according to the given tiling mode. Specific for power of two dimensions images.
#define AM_SAMPLE16_POW2_GET(_res, _x, _y, _tilingMode, _pixels) \
	if ((((AMuint32)(_x)) < (AMuint32)image->width) && (((AMuint32)(_y)) < (AMuint32)image->height)) { \
		rgba16 = (_pixels)[((_y) + childY) * stride + ((_x) + childX)]; \
		_res.c.b = (bMap * ((rgba16 >> src_bSh) & src_bMask)) >> 8; \
		_res.c.g = (gMap * ((rgba16 >> src_gSh) & src_gMask)) >> 8; \
		_res.c.r = (rMap * ((rgba16 >> src_rSh) & src_rMask)) >> 8; \
		if (!(src_flags & FMT_ALPHA)) \
			_res.c.a = 0xFF; \
		else \
			_res.c.a = (aMap * ((rgba16 >> src_aSh) & src_aMask)) >> 8; \
	} \
	else { \
		if ((_tilingMode) == VG_TILE_FILL) \
			_res.value = params->tileFillColor; \
		else { \
			AMint32 tmpX, tmpY, cycles; \
			switch (_tilingMode) { \
				case VG_TILE_PAD: \
					tmpX = AM_CLAMP((_x), 0, image->width - 1); \
					tmpY = AM_CLAMP((_y), 0, image->height - 1); \
					break; \
				case VG_TILE_REPEAT: \
					tmpX = (_x) & (image->width - 1); \
					tmpY = (_y) & (image->height - 1); \
					break; \
				default: \
					AM_ASSERT(_tilingMode == VG_TILE_REFLECT); \
					tmpX = (_x); \
					if (((AMuint32)(tmpX)) >= (AMuint32)image->width) { \
						cycles = (tmpX < 0) ? (((-tmpX - 1) >> image->widthShift) + 1) : (tmpX >> image->widthShift); \
						tmpX &= image->width - 1; \
						if (cycles & 1) tmpX = image->width - 1 - tmpX; \
					} \
					tmpY = (_y); \
					if (((AMuint32)(tmpY)) >= (AMuint32)image->height) { \
						cycles = (tmpY < 0) ? (((-tmpY - 1) >> image->heightShift) + 1) : (tmpY >> image->heightShift); \
						tmpY &= image->height - 1; \
						if (cycles & 1) tmpY = image->height - 1 - tmpY; \
					} \
					break; \
			} \
			rgba16 = (_pixels)[((tmpY) + childY) * stride + ((tmpX) + childX)]; \
			_res.c.b = (bMap * ((rgba16 >> src_bSh) & src_bMask)) >> 8; \
			_res.c.g = (gMap * ((rgba16 >> src_gSh) & src_gMask)) >> 8; \
			_res.c.r = (rMap * ((rgba16 >> src_rSh) & src_rMask)) >> 8; \
			if (!(src_flags & FMT_ALPHA)) \
				_res.c.a = 0xFF; \
			else \
				_res.c.a = (aMap * ((rgba16 >> src_aSh) & src_aMask)) >> 8; \
		} \
	}

//! Get an 8bit image sample at specified coordinates, according to the given tiling mode.
#define AM_SAMPLE8_GET(_res, _x, _y, _tilingMode, _pixels) \
	if ((((AMuint32)(_x)) < (AMuint32)image->width) && (((AMuint32)(_y)) < (AMuint32)image->height)) { \
		_res = (_pixels)[((_y) + childY) * stride + ((_x) + childX)]; \
	} \
	else { \
		if ((_tilingMode) == VG_TILE_FILL) \
			_res = params->tileFillColor; \
		else { \
			AMint32 tmpX, tmpY, cycles; \
			switch (_tilingMode) { \
				case VG_TILE_PAD: \
					tmpX = AM_CLAMP((_x), 0, image->width - 1); \
					tmpY = AM_CLAMP((_y), 0, image->height - 1); \
					break; \
				case VG_TILE_REPEAT: \
					tmpX = (_x); \
					if (((AMuint32)(tmpX)) >= (AMuint32)image->width) { \
						tmpX = tmpX % image->width; \
						if (tmpX < 0) tmpX += image->width; \
					} \
					tmpY = (_y); \
					if (((AMuint32)(tmpY)) >= (AMuint32)image->height) { \
						tmpY = tmpY % image->height; \
						if (tmpY < 0) tmpY += image->height; \
					} \
					break; \
				default: \
					AM_ASSERT(_tilingMode == VG_TILE_REFLECT); \
					tmpX = (_x); \
					if (((AMuint32)(tmpX)) >= (AMuint32)image->width) { \
						cycles = (tmpX < 0) ? (((-tmpX - 1) / image->width) + 1) : (tmpX / image->width); \
						tmpX = tmpX % image->width; \
						if (cycles & 1) { \
							tmpX = image->width - 1 - tmpX; \
							if (tmpX >= image->width) tmpX -= image->width; \
						} \
						else { \
							if (tmpX < 0) tmpX += image->width; \
						} \
					} \
					tmpY = (_y); \
					if (((AMuint32)(tmpY)) >= (AMuint32)image->height) { \
						cycles = (tmpY < 0) ? (((-tmpY - 1) / image->height) + 1) : (tmpY / image->height); \
						tmpY = tmpY % image->height; \
						if (cycles & 1) { \
							tmpY = image->height - 1 - tmpY; \
							if (tmpY >= image->height) tmpY -= image->height; \
						} \
						else { \
							if (tmpY < 0) tmpY += image->height; \
						} \
					} \
					break; \
			} \
			_res = (_pixels)[(tmpY + childY) * stride + (tmpX + childX)]; \
		} \
	}

//! Get an 8bit image sample at specified coordinates, according to the given tiling mode. Specific for power of two dimensions images.
#define AM_SAMPLE8_POW2_GET(_res, _x, _y, _tilingMode, _pixels) \
	if ((((AMuint32)(_x)) < (AMuint32)image->width) && (((AMuint32)(_y)) < (AMuint32)image->height)) { \
		_res = (_pixels)[((_y) + childY) * stride + ((_x) + childX)]; \
	} \
	else { \
		if ((_tilingMode) == VG_TILE_FILL) \
			_res = params->tileFillColor; \
		else { \
			AMint32 tmpX, tmpY, cycles; \
			switch (_tilingMode) { \
				case VG_TILE_PAD: \
					tmpX = AM_CLAMP((_x), 0, image->width - 1); \
					tmpY = AM_CLAMP((_y), 0, image->height - 1); \
					break; \
				case VG_TILE_REPEAT: \
					tmpX = (_x) & (image->width - 1); \
					tmpY = (_y) & (image->height - 1); \
					break; \
				default: \
					AM_ASSERT(_tilingMode == VG_TILE_REFLECT); \
					tmpX = (_x); \
					if (((AMuint32)(tmpX)) >= (AMuint32)image->width) { \
						cycles = (tmpX < 0) ? (((-tmpX - 1) >> image->widthShift) + 1) : (tmpX >> image->widthShift); \
						tmpX &= image->width - 1; \
						if (cycles & 1) tmpX = image->width - 1 - tmpX; \
					} \
					tmpY = (_y); \
					if (((AMuint32)(tmpY)) >= (AMuint32)image->height) { \
						cycles = (tmpY < 0) ? (((-tmpY - 1) >> image->heightShift) + 1) : (tmpY >> image->heightShift); \
						tmpY &= image->height - 1; \
						if (cycles & 1) tmpY = image->height - 1 - tmpY; \
					} \
					break; \
			} \
			_res = (_pixels)[(tmpY + childY) * stride + (tmpX + childX)]; \
		} \
	}

//! Get a 4bit image sample at specified coordinates, according to the given tiling mode.
#define AM_SAMPLE4_GET(_res, _x, _y, _tilingMode, _pixels) \
	if ((((AMuint32)(_x)) < (AMuint32)image->width) && (((AMuint32)(_y)) < (AMuint32)image->height)) { \
		_res = (_pixels)[((_y) + childY) * stride + (((_x) + childX) >> 1)]; \
	} \
	else { \
		if ((_tilingMode) == VG_TILE_FILL) \
			_res = params->tileFillColor; \
		else { \
			AMint32 tmpX, tmpY, cycles; \
			switch (_tilingMode) { \
				case VG_TILE_PAD: \
					tmpX = AM_CLAMP((_x), 0, image->width - 1); \
					tmpY = AM_CLAMP((_y), 0, image->height - 1); \
					break; \
				case VG_TILE_REPEAT: \
					tmpX = (_x); \
					if (((AMuint32)(tmpX)) >= (AMuint32)image->width) { \
						tmpX = tmpX % image->width; \
						if (tmpX < 0) tmpX += image->width; \
					} \
					tmpY = (_y); \
					if (((AMuint32)(tmpY)) >= (AMuint32)image->height) { \
						tmpY = tmpY % image->height; \
						if (tmpY < 0) tmpY += image->height; \
					} \
					break; \
				default: \
					AM_ASSERT(_tilingMode == VG_TILE_REFLECT); \
					tmpX = (_x); \
					if (((AMuint32)(tmpX)) >= (AMuint32)image->width) { \
						cycles = (tmpX < 0) ? (((-tmpX - 1) / image->width) + 1) : (tmpX / image->width); \
						tmpX = tmpX % image->width; \
						if (cycles & 1) { \
							tmpX = image->width - 1 - tmpX; \
							if (tmpX >= image->width) tmpX -= image->width; \
						} \
						else { \
							if (tmpX < 0) tmpX += image->width; \
						} \
					} \
					tmpY = (_y); \
					if (((AMuint32)(tmpY)) >= (AMuint32)image->height) { \
						cycles = (tmpY < 0) ? (((-tmpY - 1) / image->height) + 1) : (tmpY / image->height); \
						tmpY = tmpY % image->height; \
						if (cycles & 1) { \
							tmpY = image->height - 1 - tmpY; \
							if (tmpY >= image->height) tmpY -= image->height; \
						} \
						else { \
							if (tmpY < 0) tmpY += image->height; \
						} \
					} \
					break; \
			} \
			_res = (_pixels)[(tmpY + childY) * stride + ((tmpX + childX) >> 1)]; \
		} \
	}

//! Get a 4bit image sample at specified coordinates, according to the given tiling mode. Specific for power of two dimensions images.
#define AM_SAMPLE4_POW2_GET(_res, _x, _y, _tilingMode, _pixels) \
	if ((((AMuint32)(_x)) < (AMuint32)image->width) && (((AMuint32)(_y)) < (AMuint32)image->height)) { \
		_res = (_pixels)[((_y) + childY) * stride + (((_x) + childX) >> 1)]; \
	} \
	else { \
		if ((_tilingMode) == VG_TILE_FILL) \
			_res = params->tileFillColor; \
		else { \
			AMint32 tmpX, tmpY, cycles; \
			switch (_tilingMode) { \
				case VG_TILE_PAD: \
					tmpX = AM_CLAMP((_x), 0, image->width - 1); \
					tmpY = AM_CLAMP((_y), 0, image->height - 1); \
					break; \
				case VG_TILE_REPEAT: \
					tmpX = (_x) & (image->width - 1); \
					tmpY = (_y) & (image->height - 1); \
					break; \
				default: \
					AM_ASSERT(_tilingMode == VG_TILE_REFLECT); \
					tmpX = (_x); \
					if (((AMuint32)(tmpX)) >= (AMuint32)image->width) { \
						cycles = (tmpX < 0) ? (((-tmpX - 1) >> image->widthShift) + 1) : (tmpX >> image->widthShift); \
						tmpX &= image->width - 1; \
						if (cycles & 1) tmpX = image->width - 1 - tmpX; \
					} \
					tmpY = (_y); \
					if (((AMuint32)(tmpY)) >= (AMuint32)image->height) { \
						cycles = (tmpY < 0) ? (((-tmpY - 1) >> image->heightShift) + 1) : (tmpY >> image->heightShift); \
						tmpY &= image->height - 1; \
						if (cycles & 1) tmpY = image->height - 1 - tmpY; \
					} \
					break; \
			} \
			_res = (_pixels)[(tmpY + childY) * stride + ((tmpX + childX) >> 1)]; \
		} \
	}

//! Get a 1bit image sample at specified coordinates, according to the given tiling mode.
#define AM_SAMPLE1_GET(_res, _x, _y, _tilingMode, _pixels) \
	if ((((AMuint32)(_x)) < (AMuint32)image->width) && (((AMuint32)(_y)) < (AMuint32)image->height)) { \
		_res = (_pixels)[((_y) + childY) * stride + (((_x) + childX) >> 3)]; \
	} \
	else { \
		if ((_tilingMode) == VG_TILE_FILL) \
			_res = params->tileFillColor; \
		else { \
			AMint32 tmpX, tmpY, cycles; \
			switch (_tilingMode) { \
				case VG_TILE_PAD: \
					tmpX = AM_CLAMP((_x), 0, image->width - 1); \
					tmpY = AM_CLAMP((_y), 0, image->height - 1); \
					break; \
				case VG_TILE_REPEAT: \
					tmpX = (_x); \
					if (((AMuint32)(tmpX)) >= (AMuint32)image->width) { \
						tmpX = tmpX % image->width; \
						if (tmpX < 0) tmpX += image->width; \
					} \
					tmpY = (_y); \
					if (((AMuint32)(tmpY)) >= (AMuint32)image->height) { \
						tmpY = tmpY % image->height; \
						if (tmpY < 0) tmpY += image->height; \
					} \
					break; \
				default: \
					AM_ASSERT(_tilingMode == VG_TILE_REFLECT); \
					tmpX = (_x); \
					if (((AMuint32)(tmpX)) >= (AMuint32)image->width) { \
						cycles = (tmpX < 0) ? (((-tmpX - 1) / image->width) + 1) : (tmpX / image->width); \
						tmpX = tmpX % image->width; \
						if (cycles & 1) { \
							tmpX = image->width - 1 - tmpX; \
							if (tmpX >= image->width) tmpX -= image->width; \
						} \
						else { \
							if (tmpX < 0) tmpX += image->width; \
						} \
					} \
					tmpY = (_y); \
					if (((AMuint32)(tmpY)) >= (AMuint32)image->height) { \
						cycles = (tmpY < 0) ? (((-tmpY - 1) / image->height) + 1) : (tmpY / image->height); \
						tmpY = tmpY % image->height; \
						if (cycles & 1) { \
							tmpY = image->height - 1 - tmpY; \
							if (tmpY >= image->height) tmpY -= image->height; \
						} \
						else { \
							if (tmpY < 0) tmpY += image->height; \
						} \
					} \
					break; \
			} \
			_res = (_pixels)[(tmpY + childY) * stride + ((tmpX + childX) >> 3)]; \
		} \
	}

//! Get a 1bit image sample at specified coordinates, according to the given tiling mode. Specific for power of two dimensions images.
#define AM_SAMPLE1_POW2_GET(_res, _x, _y, _tilingMode, _pixels) \
	if ((((AMuint32)(_x)) < (AMuint32)image->width) && (((AMuint32)(_y)) < (AMuint32)image->height)) { \
		_res = (_pixels)[((_y) + childY) * stride + (((_x) + childX) >> 3)]; \
	} \
	else { \
		if ((_tilingMode) == VG_TILE_FILL) \
			_res = params->tileFillColor; \
		else { \
			AMint32 tmpX, tmpY, cycles; \
			switch (_tilingMode) { \
				case VG_TILE_PAD: \
					tmpX = AM_CLAMP((_x), 0, image->width - 1); \
					tmpY = AM_CLAMP((_y), 0, image->height - 1); \
					break; \
				case VG_TILE_REPEAT: \
					tmpX = (_x) & (image->width - 1); \
					tmpY = (_y) & (image->height - 1); \
					break; \
				default: \
					AM_ASSERT(_tilingMode == VG_TILE_REFLECT); \
					tmpX = (_x); \
					if (((AMuint32)(tmpX)) >= (AMuint32)image->width) { \
						cycles = (tmpX < 0) ? (((-tmpX - 1) >> image->widthShift) + 1) : (tmpX >> image->widthShift); \
						tmpX &= image->width - 1; \
						if (cycles & 1) tmpX = image->width - 1 - tmpX; \
					} \
					tmpY = (_y); \
					if (((AMuint32)(tmpY)) >= (AMuint32)image->height) { \
						cycles = (tmpY < 0) ? (((-tmpY - 1) >> image->heightShift) + 1) : (tmpY >> image->heightShift); \
						tmpY &= image->height - 1; \
						if (cycles & 1) tmpY = image->height - 1 - tmpY; \
					} \
					break; \
			} \
			_res = (_pixels)[(tmpY + childY) * stride + ((tmpX + childX) >> 3)]; \
		} \
	}

#define AM_COLOR_TRANSFORM32() \
	if (params->colorTransformation) { \
		AMint32 tmpR, tmpG, tmpB, tmpA; \
		if (src_flags & FMT_PRE) { \
			if (t0.c.a != 0) { \
				/* remove premultiplication */ \
				AM_UNPREMULTIPLY(tmpR, tmpG, tmpB, t0.c.r, t0.c.g, t0.c.b, t0.c.a) \
				tmpA = (AMuint32)t0.c.a; \
				/* apply color transform */ \
				tmpR = (tmpR * params->colorTransformation[0] + params->colorTransformation[4]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION; \
				tmpG = (tmpG * params->colorTransformation[1] + params->colorTransformation[5]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION; \
				tmpB = (tmpB * params->colorTransformation[2] + params->colorTransformation[6]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION; \
				tmpA = (tmpA * params->colorTransformation[3] + params->colorTransformation[7]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION; \
			} \
			else { \
				/* apply (simplified) color transform */ \
				tmpR = params->colorTransformation[0] >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION; \
				tmpG = params->colorTransformation[1] >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION; \
				tmpB = params->colorTransformation[2] >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION; \
				tmpA = params->colorTransformation[3] >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION; \
			} \
			/*  add premultiplication */ \
			tmpA = AM_CLAMP(tmpA, 0, 255); \
			MULT_DIV_255(tmpR, tmpR, tmpA) \
			MULT_DIV_255(tmpG, tmpG, tmpA) \
			MULT_DIV_255(tmpB, tmpB, tmpA) \
			tmpR = AM_CLAMP(tmpR, 0, tmpA); \
			tmpG = AM_CLAMP(tmpG, 0, tmpA); \
			tmpB = AM_CLAMP(tmpB, 0, tmpA); \
			t0.c.a = (AMuint8)tmpA; \
			t0.c.r = (AMuint8)tmpR; \
			t0.c.g = (AMuint8)tmpG; \
			t0.c.b = (AMuint8)tmpB; \
		} \
		else { \
			/* apply color transform */ \
			tmpR = ((AMint32)t0.c.r * params->colorTransformation[0] + params->colorTransformation[4]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION; \
			tmpG = ((AMint32)t0.c.g * params->colorTransformation[1] + params->colorTransformation[5]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION; \
			tmpB = ((AMint32)t0.c.b * params->colorTransformation[2] + params->colorTransformation[6]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION; \
			tmpA = ((AMint32)t0.c.a * params->colorTransformation[3] + params->colorTransformation[7]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION; \
			t0.c.r = (AMuint8)(AM_CLAMP(tmpR, 0, 255)); \
			t0.c.g = (AMuint8)(AM_CLAMP(tmpG, 0, 255)); \
			t0.c.b = (AMuint8)(AM_CLAMP(tmpB, 0, 255)); \
			t0.c.a = (AMuint8)(AM_CLAMP(tmpA, 0, 255)); \
		} \
	}

/*!
	\brief Get a sample from a given 32bit RGBA image at (x, y) coordinates, according to the specified sampler
	parameters.
	\param params input sampler parameters.
	\return the 32bit color value of the sample.
*/
AMuint32 amImageSample_RGBA32_Get(const AMImageSamplerParams *params) {

	AMPixel32RGBA t0;
	const AMImage *image = params->image;
	// take care of child images
	AMint32 childX = image->x;
	AMint32 childY = image->y;
	AMint32 stride = image->root->width;
	const AMuint32 *src32 = (const AMuint32 *)image->pixels;
	AMuint32 src_flags = pxlFormatTable[params->srcIdx][FMT_FLAGS];
	AMuint32 dst_rSh = pxlFormatTable[params->dstIdx][FMT_R_SH];
	AMuint32 dst_gSh = pxlFormatTable[params->dstIdx][FMT_G_SH];
	AMuint32 dst_bSh = pxlFormatTable[params->dstIdx][FMT_B_SH];
	AMuint32 dst_aSh = pxlFormatTable[params->dstIdx][FMT_A_SH];
	AMuint32 dst_flags = pxlFormatTable[params->dstIdx][FMT_FLAGS];

	AM_ASSERT(pxlFormatTable[params->srcIdx][FMT_BITS] == 32);
	AM_ASSERT(pxlFormatTable[params->dstIdx][FMT_BITS] == 32);

	// params->x and params->y are in 16.16 fixed point format
	if (image->isPow2) {
		if (params->bilinear) {
			AMint32 u = (params->x - 0x8000) >> 16;
			AMint32 v = (params->y - 0x8000) >> 16;
			AMint32 fru = ((params->x - 0x8000) >> 8) & 0xFF;
			AMint32 frv = ((params->y - 0x8000) >> 8) & 0xFF;
			AMPixel32RGBA t1, t2, t3;

			AM_SAMPLE32_POW2_GET(t0, u, v, params->tilingMode, src32)
			AM_SAMPLE32_POW2_GET(t1, u + 1, v, params->tilingMode, src32)
			AM_SAMPLE32_POW2_GET(t2, u, v + 1, params->tilingMode, src32)
			AM_SAMPLE32_POW2_GET(t3, u + 1, v + 1, params->tilingMode, src32)
			t0.rgba = amPxlLerp(fru, t0.rgba, t1.rgba);
			t2.rgba = amPxlLerp(fru, t2.rgba, t3.rgba);
			t0.rgba = amPxlLerp(frv, t0.rgba, t2.rgba);
		}
		else {
			AMint32 u = params->x >> 16;
			AMint32 v = params->y >> 16;
			AM_SAMPLE32_POW2_GET(t0, u, v, params->tilingMode, src32)
		}
	}
	else {
		if (params->bilinear) {
			AMint32 u = (params->x - 0x8000) >> 16;
			AMint32 v = (params->y - 0x8000) >> 16;
			AMint32 fru = ((params->x - 0x8000) >> 8) & 0xFF;
			AMint32 frv = ((params->y - 0x8000) >> 8) & 0xFF;
			AMPixel32RGBA t1, t2, t3;

			AM_SAMPLE32_GET(t0, u, v, params->tilingMode, src32)
			AM_SAMPLE32_GET(t1, u + 1, v, params->tilingMode, src32)
			AM_SAMPLE32_GET(t2, u, v + 1, params->tilingMode, src32)
			AM_SAMPLE32_GET(t3, u + 1, v + 1, params->tilingMode, src32)
			t0.rgba = amPxlLerp(fru, t0.rgba, t1.rgba);
			t2.rgba = amPxlLerp(fru, t2.rgba, t3.rgba);
			t0.rgba = amPxlLerp(frv, t0.rgba, t2.rgba);
		}
		else {
			AMint32 u = params->x >> 16;
			AMint32 v = params->y >> 16;
			AM_SAMPLE32_GET(t0, u, v, params->tilingMode, src32)
		}
	}

	if (!(src_flags & FMT_ALPHA))
		t0.c.a = 0xFF;

#if (AM_OPENVG_VERSION >= 110)
	AM_COLOR_TRANSFORM32()
#endif

	if ((src_flags & (FMT_PRE | FMT_L)) == (dst_flags & (FMT_PRE | FMT_L)))
		return ((AMuint32)t0.c.r << dst_rSh) | ((AMuint32)t0.c.g << dst_gSh) | ((AMuint32)t0.c.b << dst_bSh) | ((AMuint32)t0.c.a << dst_aSh);
	else {
		// remove premultiplication, if source format was premultiplied
		if (src_flags & FMT_PRE) {
			if (t0.c.a == 0)
				return 0;
			else {
				AM_ASSERT(t0.c.r <= t0.c.a && t0.c.g <= t0.c.a && t0.c.b <= t0.c.a);
				// remove premultiplication
				AM_UNPREMULTIPLY(t0.c.r, t0.c.g, t0.c.b, t0.c.r, t0.c.g, t0.c.b, t0.c.a)
			}
		}
		// l --> s conversion
		if ((src_flags & FMT_L) && !(dst_flags & FMT_L)) {
			t0.c.r = AM_GAMMA_TABLE(t0.c.r);
			t0.c.g = AM_GAMMA_TABLE(t0.c.g);
			t0.c.b = AM_GAMMA_TABLE(t0.c.b);
		}
		else
		// s --> l conversion
		if (!(src_flags & FMT_L) && (dst_flags & FMT_L)) {
			t0.c.r = AM_GAMMA_INV_TABLE(t0.c.r);
			t0.c.g = AM_GAMMA_INV_TABLE(t0.c.g);
			t0.c.b = AM_GAMMA_INV_TABLE(t0.c.b);
		}

		// premultiply if requested by the destination format
		if (dst_flags & FMT_PRE)
			t0.rgba = amPxl_RGBA_Scl255PreserveA(t0.c.a, t0.rgba);

		return ((AMuint32)t0.c.r << dst_rSh) | ((AMuint32)t0.c.g << dst_gSh) | ((AMuint32)t0.c.b << dst_bSh) | ((AMuint32)t0.c.a << dst_aSh);
	}
}

/*!
	\brief Get a sample from a given 32bit ARGB image at (x, y) coordinates, according to the specified sampler
	parameters.
	\param params input sampler parameters.
	\return the 32bit color value of the sample.
*/
AMuint32 amImageSample_ARGB32_Get(const AMImageSamplerParams *params) {

	AMPixel32ARGB t0;
	const AMImage *image = params->image;
	// take care of child images
	AMint32 childX = image->x;
	AMint32 childY = image->y;
	AMint32 stride = image->root->width;
	const AMuint32 *src32 = (const AMuint32 *)image->pixels;
	AMuint32 src_flags = pxlFormatTable[params->srcIdx][FMT_FLAGS];
	AMuint32 dst_rSh = pxlFormatTable[params->dstIdx][FMT_R_SH];
	AMuint32 dst_gSh = pxlFormatTable[params->dstIdx][FMT_G_SH];
	AMuint32 dst_bSh = pxlFormatTable[params->dstIdx][FMT_B_SH];
	AMuint32 dst_aSh = pxlFormatTable[params->dstIdx][FMT_A_SH];
	AMuint32 dst_flags = pxlFormatTable[params->dstIdx][FMT_FLAGS];

	AM_ASSERT(pxlFormatTable[params->srcIdx][FMT_BITS] == 32);
	AM_ASSERT(pxlFormatTable[params->dstIdx][FMT_BITS] == 32);

	// params->x and params->y are in 16.16 fixed point format
	if (image->isPow2) {
		if (params->bilinear) {
		
			AMint32 u = (params->x - 0x8000) >> 16;
			AMint32 v = (params->y - 0x8000) >> 16;
			AMint32 fru = ((params->x - 0x8000) >> 8) & 0xFF;
			AMint32 frv = ((params->y - 0x8000) >> 8) & 0xFF;
			AMPixel32ARGB t1, t2, t3;

			AM_SAMPLE32_POW2_GET(t0, u, v, params->tilingMode, src32)
			AM_SAMPLE32_POW2_GET(t1, u + 1, v, params->tilingMode, src32)
			AM_SAMPLE32_POW2_GET(t2, u, v + 1, params->tilingMode, src32)
			AM_SAMPLE32_POW2_GET(t3, u + 1, v + 1, params->tilingMode, src32)
			t0.argb = amPxlLerp(fru, t0.argb, t1.argb);
			t2.argb = amPxlLerp(fru, t2.argb, t3.argb);
			t0.argb = amPxlLerp(frv, t0.argb, t2.argb);
		}
		else {
			AMint32 u = params->x >> 16;
			AMint32 v = params->y >> 16;
			AM_SAMPLE32_POW2_GET(t0, u, v, params->tilingMode, src32)
		}
	}
	else {
		if (params->bilinear) {
			AMint32 u = (params->x - 0x8000) >> 16;
			AMint32 v = (params->y - 0x8000) >> 16;
			AMint32 fru = ((params->x - 0x8000) >> 8) & 0xFF;
			AMint32 frv = ((params->y - 0x8000) >> 8) & 0xFF;
			AMPixel32ARGB t1, t2, t3;

			AM_SAMPLE32_GET(t0, u, v, params->tilingMode, src32)
			AM_SAMPLE32_GET(t1, u + 1, v, params->tilingMode, src32)
			AM_SAMPLE32_GET(t2, u, v + 1, params->tilingMode, src32)
			AM_SAMPLE32_GET(t3, u + 1, v + 1, params->tilingMode, src32)
			t0.argb = amPxlLerp(fru, t0.argb, t1.argb);
			t2.argb = amPxlLerp(fru, t2.argb, t3.argb);
			t0.argb = amPxlLerp(frv, t0.argb, t2.argb);
		}
		else {
			AMint32 u = params->x >> 16;
			AMint32 v = params->y >> 16;
			AM_SAMPLE32_GET(t0, u, v, params->tilingMode, src32)
		}
	}

	if (!(src_flags & FMT_ALPHA))
		t0.c.a = 0xFF;

#if (AM_OPENVG_VERSION >= 110)
	AM_COLOR_TRANSFORM32()
#endif

	if ((src_flags & (FMT_PRE | FMT_L)) == (dst_flags & (FMT_PRE | FMT_L)))
		return ((AMuint32)t0.c.r << dst_rSh) | ((AMuint32)t0.c.g << dst_gSh) | ((AMuint32)t0.c.b << dst_bSh) | ((AMuint32)t0.c.a << dst_aSh);
	else {
		// remove premultiplication, if source format was premultiplied
		if (src_flags & FMT_PRE) {
			if (t0.c.a == 0)
				return 0;
			else {
				AM_ASSERT(t0.c.r <= t0.c.a && t0.c.g <= t0.c.a && t0.c.b <= t0.c.a);
				// remove premultiplication
				AM_UNPREMULTIPLY(t0.c.r, t0.c.g, t0.c.b, t0.c.r, t0.c.g, t0.c.b, t0.c.a)
			}
		}
		// l --> s conversion
		if ((src_flags & FMT_L) && !(dst_flags & FMT_L)) {
			t0.c.r = AM_GAMMA_TABLE(t0.c.r);
			t0.c.g = AM_GAMMA_TABLE(t0.c.g);
			t0.c.b = AM_GAMMA_TABLE(t0.c.b);
		}
		else
		// s --> l conversion
		if (!(src_flags & FMT_L) && (dst_flags & FMT_L)) {
			t0.c.r = AM_GAMMA_INV_TABLE(t0.c.r);
			t0.c.g = AM_GAMMA_INV_TABLE(t0.c.g);
			t0.c.b = AM_GAMMA_INV_TABLE(t0.c.b);
		}

		// premultiply if requested by the destination format
		if (dst_flags & FMT_PRE)
			t0.argb = amPxl_ARGB_Scl255PreserveA(t0.c.a, t0.argb);

		return ((AMuint32)t0.c.r << dst_rSh) | ((AMuint32)t0.c.g << dst_gSh) | ((AMuint32)t0.c.b << dst_bSh) | ((AMuint32)t0.c.a << dst_aSh);
	}
}

/*!
	\brief Get a sample from a given 16bit RGBA image at (x, y) coordinates, according to the specified sampler
	parameters.
	\param params input sampler parameters.
	\return the 32bit color value of the sample.
*/
AMuint32 amImageSample_RGBA16_Get(const AMImageSamplerParams *params) {

	AMPixel32RGBA t0;
	const AMImage *image = params->image;
	// take care of child images
	AMint32 childX = image->x;
	AMint32 childY = image->y;
	AMint32 stride = image->root->width;
	const AMuint16 *src16 = (const AMuint16 *)image->pixels;
	AMuint16 rgba16;
	AMuint32 src_rSh = pxlFormatTable[params->srcIdx][FMT_R_SH];
	AMuint32 src_gSh = pxlFormatTable[params->srcIdx][FMT_G_SH];
	AMuint32 src_bSh = pxlFormatTable[params->srcIdx][FMT_B_SH];
	AMuint32 src_aSh = pxlFormatTable[params->srcIdx][FMT_A_SH];
	AMuint16 src_rMask = (1 << pxlFormatTable[params->srcIdx][FMT_R_BITS]) - 1;
	AMuint16 src_gMask = (1 << pxlFormatTable[params->srcIdx][FMT_G_BITS]) - 1;
	AMuint16 src_bMask = (1 << pxlFormatTable[params->srcIdx][FMT_B_BITS]) - 1;
	AMuint16 src_aMask = (1 << pxlFormatTable[params->srcIdx][FMT_A_BITS]) - 1;
	AMuint32 src_flags = pxlFormatTable[params->srcIdx][FMT_FLAGS];
	AMuint32 dst_rSh = pxlFormatTable[params->dstIdx][FMT_R_SH];
	AMuint32 dst_gSh = pxlFormatTable[params->dstIdx][FMT_G_SH];
	AMuint32 dst_bSh = pxlFormatTable[params->dstIdx][FMT_B_SH];
	AMuint32 dst_aSh = pxlFormatTable[params->dstIdx][FMT_A_SH];
	AMuint32 dst_flags = pxlFormatTable[params->dstIdx][FMT_FLAGS];
	AMuint32 rMap = amBitConversionTable[8 - 1][pxlFormatTable[params->srcIdx][FMT_R_BITS] - 1];
	AMuint32 gMap = amBitConversionTable[8 - 1][pxlFormatTable[params->srcIdx][FMT_G_BITS] - 1];
	AMuint32 bMap = amBitConversionTable[8 - 1][pxlFormatTable[params->srcIdx][FMT_B_BITS] - 1];
	AMuint32 aMap = amBitConversionTable[8 - 1][pxlFormatTable[params->srcIdx][FMT_A_BITS] - 1];

	AM_ASSERT(pxlFormatTable[params->srcIdx][FMT_BITS] == 16);
	AM_ASSERT(pxlFormatTable[params->dstIdx][FMT_BITS] == 32);

	// params->x and params->y are in 16.16 fixed point format
	if (image->isPow2) {
		if (params->bilinear) {
			AMint32 u = (params->x - 0x8000) >> 16;
			AMint32 v = (params->y - 0x8000) >> 16;
			AMint32 fru = ((params->x - 0x8000) >> 8) & 0xFF;
			AMint32 frv = ((params->y - 0x8000) >> 8) & 0xFF;
			AMPixel32RGBA t1, t2, t3;

			AM_SAMPLE16_POW2_GET(t0, u, v, params->tilingMode, src16)
			AM_SAMPLE16_POW2_GET(t1, u + 1, v, params->tilingMode, src16)
			AM_SAMPLE16_POW2_GET(t2, u, v + 1, params->tilingMode, src16)
			AM_SAMPLE16_POW2_GET(t3, u + 1, v + 1, params->tilingMode, src16)
			t0.rgba = amPxlLerp(fru, t0.rgba, t1.rgba);
			t2.rgba = amPxlLerp(fru, t2.rgba, t3.rgba);
			t0.rgba = amPxlLerp(frv, t0.rgba, t2.rgba);
		}
		else {
			AMint32 u = params->x >> 16;
			AMint32 v = params->y >> 16;
			AM_SAMPLE16_POW2_GET(t0, u, v, params->tilingMode, src16)
		}
	}
	else {
		if (params->bilinear) {
			AMint32 u = (params->x - 0x8000) >> 16;
			AMint32 v = (params->y - 0x8000) >> 16;
			AMint32 fru = ((params->x - 0x8000) >> 8) & 0xFF;
			AMint32 frv = ((params->y - 0x8000) >> 8) & 0xFF;
			AMPixel32RGBA t1, t2, t3;

			AM_SAMPLE16_GET(t0, u, v, params->tilingMode, src16)
			AM_SAMPLE16_GET(t1, u + 1, v, params->tilingMode, src16)
			AM_SAMPLE16_GET(t2, u, v + 1, params->tilingMode, src16)
			AM_SAMPLE16_GET(t3, u + 1, v + 1, params->tilingMode, src16)
			t0.rgba = amPxlLerp(fru, t0.rgba, t1.rgba);
			t2.rgba = amPxlLerp(fru, t2.rgba, t3.rgba);
			t0.rgba = amPxlLerp(frv, t0.rgba, t2.rgba);
		}
		else {
			AMint32 u = params->x >> 16;
			AMint32 v = params->y >> 16;
			AM_SAMPLE16_GET(t0, u, v, params->tilingMode, src16)
		}
	}

	if (!(src_flags & FMT_ALPHA))
		t0.c.a = 0xFF;

#if (AM_OPENVG_VERSION >= 110)
	AM_COLOR_TRANSFORM32()
#endif

	// s --> l conversion
	if (!(src_flags & FMT_L) && (dst_flags & FMT_L)) {
		t0.c.r = AM_GAMMA_INV_TABLE(t0.c.r);
		t0.c.g = AM_GAMMA_INV_TABLE(t0.c.g);
		t0.c.b = AM_GAMMA_INV_TABLE(t0.c.b);
	}

	// premultiply if requested by the destination format
	if (dst_flags & FMT_PRE)
		t0.rgba = amPxl_RGBA_Scl255PreserveA(t0.c.a, t0.rgba);

	return ((AMuint32)t0.c.r << dst_rSh) | ((AMuint32)t0.c.g << dst_gSh) | ((AMuint32)t0.c.b << dst_bSh) | ((AMuint32)t0.c.a << dst_aSh);
}

/*!
	\brief Get a sample from a given 16bit ARGB image at (x, y) coordinates, according to the specified sampler
	parameters.
	\param params input sampler parameters.
	\return the 32bit color value of the sample.
*/
AMuint32 amImageSample_ARGB16_Get(const AMImageSamplerParams *params) {

	AMPixel32ARGB t0;
	const AMImage *image = params->image;
	// take care of child images
	AMint32 childX = image->x;
	AMint32 childY = image->y;
	AMint32 stride = image->root->width;
	const AMuint16 *src16 = (const AMuint16 *)image->pixels;
	AMuint16 rgba16;
	AMuint32 src_rSh = pxlFormatTable[params->srcIdx][FMT_R_SH];
	AMuint32 src_gSh = pxlFormatTable[params->srcIdx][FMT_G_SH];
	AMuint32 src_bSh = pxlFormatTable[params->srcIdx][FMT_B_SH];
	AMuint32 src_aSh = pxlFormatTable[params->srcIdx][FMT_A_SH];
	AMuint16 src_rMask = (1 << pxlFormatTable[params->srcIdx][FMT_R_BITS]) - 1;
	AMuint16 src_gMask = (1 << pxlFormatTable[params->srcIdx][FMT_G_BITS]) - 1;
	AMuint16 src_bMask = (1 << pxlFormatTable[params->srcIdx][FMT_B_BITS]) - 1;
	AMuint16 src_aMask = (1 << pxlFormatTable[params->srcIdx][FMT_A_BITS]) - 1;
	AMuint32 src_flags = pxlFormatTable[params->srcIdx][FMT_FLAGS];
	AMuint32 dst_rSh = pxlFormatTable[params->dstIdx][FMT_R_SH];
	AMuint32 dst_gSh = pxlFormatTable[params->dstIdx][FMT_G_SH];
	AMuint32 dst_bSh = pxlFormatTable[params->dstIdx][FMT_B_SH];
	AMuint32 dst_aSh = pxlFormatTable[params->dstIdx][FMT_A_SH];
	AMuint32 dst_flags = pxlFormatTable[params->dstIdx][FMT_FLAGS];
	AMuint32 rMap = amBitConversionTable[8 - 1][pxlFormatTable[params->srcIdx][FMT_R_BITS] - 1];
	AMuint32 gMap = amBitConversionTable[8 - 1][pxlFormatTable[params->srcIdx][FMT_G_BITS] - 1];
	AMuint32 bMap = amBitConversionTable[8 - 1][pxlFormatTable[params->srcIdx][FMT_B_BITS] - 1];
	AMuint32 aMap = amBitConversionTable[8 - 1][pxlFormatTable[params->srcIdx][FMT_A_BITS] - 1];

	AM_ASSERT(pxlFormatTable[params->srcIdx][FMT_BITS] == 16);
	AM_ASSERT(pxlFormatTable[params->dstIdx][FMT_BITS] == 32);

	// params->x and params->y are in 16.16 fixed point format
	if (image->isPow2) {
		if (params->bilinear) {
			AMint32 u = (params->x - 0x8000) >> 16;
			AMint32 v = (params->y - 0x8000) >> 16;
			AMint32 fru = ((params->x - 0x8000) >> 8) & 0xFF;
			AMint32 frv = ((params->y - 0x8000) >> 8) & 0xFF;
			AMPixel32ARGB t1, t2, t3;

			AM_SAMPLE16_POW2_GET(t0, u, v, params->tilingMode, src16)
			AM_SAMPLE16_POW2_GET(t1, u + 1, v, params->tilingMode, src16)
			AM_SAMPLE16_POW2_GET(t2, u, v + 1, params->tilingMode, src16)
			AM_SAMPLE16_POW2_GET(t3, u + 1, v + 1, params->tilingMode, src16)
			t0.argb = amPxlLerp(fru, t0.argb, t1.argb);
			t2.argb = amPxlLerp(fru, t2.argb, t3.argb);
			t0.argb = amPxlLerp(frv, t0.argb, t2.argb);
		}
		else {
			AMint32 u = params->x >> 16;
			AMint32 v = params->y >> 16;
			AM_SAMPLE16_POW2_GET(t0, u, v, params->tilingMode, src16)
		}
	}
	else {
		if (params->bilinear) {
			AMint32 u = (params->x - 0x8000) >> 16;
			AMint32 v = (params->y - 0x8000) >> 16;
			AMint32 fru = ((params->x - 0x8000) >> 8) & 0xFF;
			AMint32 frv = ((params->y - 0x8000) >> 8) & 0xFF;
			AMPixel32ARGB t1, t2, t3;

			AM_SAMPLE16_GET(t0, u, v, params->tilingMode, src16)
			AM_SAMPLE16_GET(t1, u + 1, v, params->tilingMode, src16)
			AM_SAMPLE16_GET(t2, u, v + 1, params->tilingMode, src16)
			AM_SAMPLE16_GET(t3, u + 1, v + 1, params->tilingMode, src16)
			t0.argb = amPxlLerp(fru, t0.argb, t1.argb);
			t2.argb = amPxlLerp(fru, t2.argb, t3.argb);
			t0.argb = amPxlLerp(frv, t0.argb, t2.argb);
		}
		else {
			AMint32 u = params->x >> 16;
			AMint32 v = params->y >> 16;
			AM_SAMPLE16_GET(t0, u, v, params->tilingMode, src16)
		}
	}

	if (!(src_flags & FMT_ALPHA))
		t0.c.a = 0xFF;

#if (AM_OPENVG_VERSION >= 110)
	AM_COLOR_TRANSFORM32()
#endif

	// s --> l conversion
	if (!(src_flags & FMT_L) && (dst_flags & FMT_L)) {
		t0.c.r = AM_GAMMA_INV_TABLE(t0.c.r);
		t0.c.g = AM_GAMMA_INV_TABLE(t0.c.g);
		t0.c.b = AM_GAMMA_INV_TABLE(t0.c.b);
	}

	// premultiply if requested by the destination format
	if (dst_flags & FMT_PRE)
		t0.argb = amPxl_ARGB_Scl255PreserveA(t0.c.a, t0.argb);

	return ((AMuint32)t0.c.r << dst_rSh) | ((AMuint32)t0.c.g << dst_gSh) | ((AMuint32)t0.c.b << dst_bSh) | ((AMuint32)t0.c.a << dst_aSh);
}

/*!
	\brief Get a sample from a given 8bit image at (x, y) coordinates, according to the specified sampler
	parameters.
	\param params input sampler parameters.
	\return the 32bit color value of the sample.
*/
AMuint32 amImageSample_8_Get(const AMImageSamplerParams *params) {

	AMuint8 a0;
	const AMImage *image = params->image;
	// take care of child images
	AMint32 childX = image->x;
	AMint32 childY = image->y;
	AMint32 stride = image->root->width;
	AMuint8 *src8 = (AMuint8 *)image->pixels;
	AMuint32 src_flags = pxlFormatTable[params->srcIdx][FMT_FLAGS];
	AMuint32 dst_rSh = pxlFormatTable[params->dstIdx][FMT_R_SH];
	AMuint32 dst_gSh = pxlFormatTable[params->dstIdx][FMT_G_SH];
	AMuint32 dst_bSh = pxlFormatTable[params->dstIdx][FMT_B_SH];
	AMuint32 dst_aSh = pxlFormatTable[params->dstIdx][FMT_A_SH];
	AMuint32 dst_flags = pxlFormatTable[params->dstIdx][FMT_FLAGS];

	AM_ASSERT(pxlFormatTable[params->srcIdx][FMT_BITS] == 8);
	AM_ASSERT(pxlFormatTable[params->dstIdx][FMT_BITS] == 32);

	// params->x and params->y are in 16.16 fixed point format
	if (image->isPow2) {
		if (params->bilinear) {
			AMint32 u = (params->x - 0x8000) >> 16;
			AMint32 v = (params->y - 0x8000) >> 16;
			AMint32 fru = ((params->x - 0x8000) >> 8) & 0xFF;
			AMint32 frv = ((params->y - 0x8000) >> 8) & 0xFF;
			AMuint8 a1, a2, a3;

			AM_SAMPLE8_POW2_GET(a0, u, v, params->tilingMode, src8)
			AM_SAMPLE8_POW2_GET(a1, u + 1, v, params->tilingMode, src8)
			AM_SAMPLE8_POW2_GET(a2, u, v + 1, params->tilingMode, src8)
			AM_SAMPLE8_POW2_GET(a3, u + 1, v + 1, params->tilingMode, src8)
			
			a0 = (AMuint8)(((AMuint32)(255 - fru) * a0 + (AMuint32)fru * a1) >> 8);
			a2 = (AMuint8)(((AMuint32)(255 - fru) * a2 + (AMuint32)fru * a3) >> 8);
			a0 = (AMuint8)(((AMuint32)(255 - frv) * a0 + (AMuint32)frv * a2) >> 8);
		}
		else {
			AMint32 u = params->x >> 16;
			AMint32 v = params->y >> 16;
			AM_SAMPLE8_POW2_GET(a0, u, v, params->tilingMode, src8)
		}
	}
	else {
		if (params->bilinear) {
			AMint32 u = (params->x - 0x8000) >> 16;
			AMint32 v = (params->y - 0x8000) >> 16;
			AMint32 fru = ((params->x - 0x8000) >> 8) & 0xFF;
			AMint32 frv = ((params->y - 0x8000) >> 8) & 0xFF;
			AMuint8 a1, a2, a3;

			AM_SAMPLE8_GET(a0, u, v, params->tilingMode, src8)
			AM_SAMPLE8_GET(a1, u + 1, v, params->tilingMode, src8)
			AM_SAMPLE8_GET(a2, u, v + 1, params->tilingMode, src8)
			AM_SAMPLE8_GET(a3, u + 1, v + 1, params->tilingMode, src8)
			
			a0 = (AMuint8)(((AMuint32)(255 - fru) * a0 + (AMuint32)fru * a1) >> 8);
			a2 = (AMuint8)(((AMuint32)(255 - fru) * a2 + (AMuint32)fru * a3) >> 8);
			a0 = (AMuint8)(((AMuint32)(255 - frv) * a0 + (AMuint32)frv * a2) >> 8);
		}
		else {
			AMint32 u = params->x >> 16;
			AMint32 v = params->y >> 16;
			AM_SAMPLE8_GET(a0, u, v, params->tilingMode, src8)
		}
	}

	if (image->format == VG_A_8) {

	#if (AM_OPENVG_VERSION >= 110)
		if (params->colorTransformation) {

			AMPixel32RGBA t0;
			AMint32 tmpR = ((AMint32)0xFF * params->colorTransformation[0] + params->colorTransformation[4]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION;
			AMint32 tmpG = ((AMint32)0xFF * params->colorTransformation[1] + params->colorTransformation[5]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION;
			AMint32 tmpB = ((AMint32)0xFF * params->colorTransformation[2] + params->colorTransformation[6]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION;
			AMint32 tmpA = ((AMint32)a0 * params->colorTransformation[3] + params->colorTransformation[7]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION;

			t0.c.r = (AMuint8)(AM_CLAMP(tmpR, 0, 255));
			t0.c.g = (AMuint8)(AM_CLAMP(tmpG, 0, 255));
			t0.c.b = (AMuint8)(AM_CLAMP(tmpB, 0, 255));
			t0.c.a = (AMuint8)(AM_CLAMP(tmpA, 0, 255));

			AM_ASSERT((src_flags & FMT_PRE) == 0);

			// l --> s conversion
			if (!(dst_flags & FMT_L)) {
				t0.c.r = AM_GAMMA_TABLE(t0.c.r);
				t0.c.g = AM_GAMMA_TABLE(t0.c.g);
				t0.c.b = AM_GAMMA_TABLE(t0.c.b);
			}
			// premultiply if requested by the destination format
			if (dst_flags & FMT_PRE)
				t0.rgba = amPxl_RGBA_Scl255PreserveA(t0.c.a, t0.rgba);

			return ((AMuint32)t0.c.r << dst_rSh) | ((AMuint32)t0.c.g << dst_gSh) | ((AMuint32)t0.c.b << dst_bSh) | ((AMuint32)t0.c.a << dst_aSh);
		}
		else
	#endif
		if (dst_flags & FMT_PRE)
			return ((AMuint32)a0 << 24) | ((AMuint32)a0 << 16) | ((AMuint32)a0 << 8) | (AMuint32)a0;
		else
			return ((AMuint32)0xFF << dst_rSh) | ((AMuint32)0xFF << dst_gSh) | ((AMuint32)0xFF << dst_bSh) | ((AMuint32)a0 << dst_aSh);
	}
	else {
		AM_ASSERT(image->format == VG_sL_8 || image->format == VG_lL_8);

	#if (AM_OPENVG_VERSION >= 110)
		if (params->colorTransformation) {

			AMPixel32RGBA t0;
			AMint32 tmpR = ((AMint32)a0 * params->colorTransformation[0] + params->colorTransformation[4]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION;
			AMint32 tmpG = ((AMint32)a0 * params->colorTransformation[1] + params->colorTransformation[5]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION;
			AMint32 tmpB = ((AMint32)a0 * params->colorTransformation[2] + params->colorTransformation[6]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION;
			AMint32 tmpA = ((AMint32)255 * params->colorTransformation[3] + params->colorTransformation[7]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION;

			t0.c.r = (AMuint8)(AM_CLAMP(tmpR, 0, 255));
			t0.c.g = (AMuint8)(AM_CLAMP(tmpG, 0, 255));
			t0.c.b = (AMuint8)(AM_CLAMP(tmpB, 0, 255));
			t0.c.a = (AMuint8)(AM_CLAMP(tmpA, 0, 255));

			AM_ASSERT((src_flags & FMT_PRE) == 0);

			if ((src_flags & (FMT_PRE | FMT_L)) == (dst_flags & (FMT_PRE | FMT_L))) {
				if (!(src_flags & FMT_ALPHA))
					t0.c.a = 0xFF;
				return ((AMuint32)t0.c.r << dst_rSh) | ((AMuint32)t0.c.g << dst_gSh) | ((AMuint32)t0.c.b << dst_bSh) | ((AMuint32)t0.c.a << dst_aSh);
			}
			else {
				// l --> s conversion
				if ((src_flags & FMT_L) && !(dst_flags & FMT_L)) {
					t0.c.r = AM_GAMMA_TABLE(t0.c.r);
					t0.c.g = AM_GAMMA_TABLE(t0.c.g);
					t0.c.b = AM_GAMMA_TABLE(t0.c.b);
				}
				else
				// s --> l conversion
				if (!(src_flags & FMT_L) && (dst_flags & FMT_L)) {
					t0.c.r = AM_GAMMA_INV_TABLE(t0.c.r);
					t0.c.g = AM_GAMMA_INV_TABLE(t0.c.g);
					t0.c.b = AM_GAMMA_INV_TABLE(t0.c.b);
				}

				// premultiply if requested by the destination format
				if (dst_flags & FMT_PRE)
					t0.rgba = amPxl_RGBA_Scl255PreserveA(t0.c.a, t0.rgba);

				return ((AMuint32)t0.c.r << dst_rSh) | ((AMuint32)t0.c.g << dst_gSh) | ((AMuint32)t0.c.b << dst_bSh) | ((AMuint32)t0.c.a << dst_aSh);
			}
		}
		else {
	#endif
			// l --> s conversion
			if ((src_flags & FMT_L) && !(dst_flags & FMT_L))
				a0 = AM_GAMMA_TABLE(a0);
			else
			// s --> l conversion
			if (!(src_flags & FMT_L) && (dst_flags & FMT_L))
				a0 = AM_GAMMA_INV_TABLE(a0);

			return ((AMuint32)a0 << dst_rSh) | ((AMuint32)a0 << dst_gSh) | ((AMuint32)a0 << dst_bSh) | ((AMuint32)0xFF << dst_aSh);
	#if (AM_OPENVG_VERSION >= 110)
		}
	#endif
	}
}

#if (AM_OPENVG_VERSION >= 110)
/*!
	\brief Get a sample from a given 4bit image at (x, y) coordinates, according to the specified sampler
	parameters.
	\param params input sampler parameters.
	\return the 32bit color value of the sample.
*/
AMuint32 amImageSample_4_Get(const AMImageSamplerParams *params) {

	AMuint8 a0;
	const AMImage *image = params->image;
	// take care of child images
	AMint32 childX = image->x;
	AMint32 childY = image->y;
	AMint32 stride = image->root->dataStride;
	AMuint8 *src8 = (AMuint8 *)image->pixels;
	AMuint32 dst_rSh = pxlFormatTable[params->dstIdx][FMT_R_SH];
	AMuint32 dst_gSh = pxlFormatTable[params->dstIdx][FMT_G_SH];
	AMuint32 dst_bSh = pxlFormatTable[params->dstIdx][FMT_B_SH];
	AMuint32 dst_aSh = pxlFormatTable[params->dstIdx][FMT_A_SH];
	AMuint32 dst_flags = pxlFormatTable[params->dstIdx][FMT_FLAGS];

	AM_ASSERT(pxlFormatTable[params->srcIdx][FMT_BITS] == 4);
	AM_ASSERT(image->format == VG_A_4);
	AM_ASSERT(pxlFormatTable[params->dstIdx][FMT_BITS] == 32);

	// params->x and params->y are in 16.16 fixed point format
	if (image->isPow2) {
		if (params->bilinear) {
			AMint32 u = (params->x - 0x8000) >> 16;
			AMint32 v = (params->y - 0x8000) >> 16;
			AMint32 fru = ((params->x - 0x8000) >> 8) & 0xFF;
			AMint32 frv = ((params->y - 0x8000) >> 8) & 0xFF;
			AMuint8 a1, a2, a3;

			AM_SAMPLE4_POW2_GET(a0, u, v, params->tilingMode, src8)
			a0 = ((a0 >> ((u & 1) << 2)) & 0x0F) * 17;
			AM_SAMPLE4_POW2_GET(a1, u + 1, v, params->tilingMode, src8)
			a1 = ((a1 >> (((u + 1) & 1) << 2)) & 0x0F) * 17;
			AM_SAMPLE4_POW2_GET(a2, u, v + 1, params->tilingMode, src8)
			a2 = ((a2 >> ((u & 1) << 2)) & 0x0F) * 17;
			AM_SAMPLE4_POW2_GET(a3, u + 1, v + 1, params->tilingMode, src8)
			a3 = ((a3 >> (((u + 1) & 1) << 2)) & 0x0F) * 17;
			
			a0 = (AMuint8)(((AMuint32)(255 - fru) * a0 + (AMuint32)fru * a1) >> 8);
			a2 = (AMuint8)(((AMuint32)(255 - fru) * a2 + (AMuint32)fru * a3) >> 8);
			a0 = (AMuint8)(((AMuint32)(255 - frv) * a0 + (AMuint32)frv * a2) >> 8);
		}
		else {
			AMint32 u = params->x >> 16;
			AMint32 v = params->y >> 16;
			AM_SAMPLE4_POW2_GET(a0, u, v, params->tilingMode, src8)
			a0 = ((a0 >> ((u & 1) << 2)) & 0x0F) * 17;
		}
	}
	else {
		if (params->bilinear) {
			AMint32 u = (params->x - 0x8000) >> 16;
			AMint32 v = (params->y - 0x8000) >> 16;
			AMint32 fru = ((params->x - 0x8000) >> 8) & 0xFF;
			AMint32 frv = ((params->y - 0x8000) >> 8) & 0xFF;
			AMuint8 a1, a2, a3;

			AM_SAMPLE4_GET(a0, u, v, params->tilingMode, src8)
			a0 = ((a0 >> ((u & 1) << 2)) & 0x0F) * 17;
			AM_SAMPLE4_GET(a1, u + 1, v, params->tilingMode, src8)
			a1 = ((a1 >> (((u + 1) & 1) << 2)) & 0x0F) * 17;
			AM_SAMPLE4_GET(a2, u, v + 1, params->tilingMode, src8)
			a2 = ((a2 >> ((u & 1) << 2)) & 0x0F) * 17;
			AM_SAMPLE4_GET(a3, u + 1, v + 1, params->tilingMode, src8)
			a3 = ((a3 >> (((u + 1) & 1) << 2)) & 0x0F) * 17;
			
			a0 = (AMuint8)(((AMuint32)(255 - fru) * a0 + (AMuint32)fru * a1) >> 8);
			a2 = (AMuint8)(((AMuint32)(255 - fru) * a2 + (AMuint32)fru * a3) >> 8);
			a0 = (AMuint8)(((AMuint32)(255 - frv) * a0 + (AMuint32)frv * a2) >> 8);
		}
		else {
			AMint32 u = params->x >> 16;
			AMint32 v = params->y >> 16;
			AM_SAMPLE4_GET(a0, u, v, params->tilingMode, src8)
			a0 = ((a0 >> ((u & 1) << 2)) & 0x0F) * 17;
		}
	}

	if (params->colorTransformation) {

		AMPixel32RGBA t0;
		AMint32 tmpR = ((AMint32)0xFF * params->colorTransformation[0] + params->colorTransformation[4]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION;
		AMint32 tmpG = ((AMint32)0xFF * params->colorTransformation[1] + params->colorTransformation[5]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION;
		AMint32 tmpB = ((AMint32)0xFF * params->colorTransformation[2] + params->colorTransformation[6]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION;
		AMint32 tmpA = ((AMint32)a0 * params->colorTransformation[3] + params->colorTransformation[7]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION;

		t0.c.r = (AMuint8)(AM_CLAMP(tmpR, 0, 255));
		t0.c.g = (AMuint8)(AM_CLAMP(tmpG, 0, 255));
		t0.c.b = (AMuint8)(AM_CLAMP(tmpB, 0, 255));
		t0.c.a = (AMuint8)(AM_CLAMP(tmpA, 0, 255));

		// l --> s conversion
		if (!(dst_flags & FMT_L)) {
			t0.c.r = AM_GAMMA_TABLE(t0.c.r);
			t0.c.g = AM_GAMMA_TABLE(t0.c.g);
			t0.c.b = AM_GAMMA_TABLE(t0.c.b);
		}
		// premultiply if requested by the destination format
		if (dst_flags & FMT_PRE)
			t0.rgba = amPxl_RGBA_Scl255PreserveA(t0.c.a, t0.rgba);

		return ((AMuint32)t0.c.r << dst_rSh) | ((AMuint32)t0.c.g << dst_gSh) | ((AMuint32)t0.c.b << dst_bSh) | ((AMuint32)t0.c.a << dst_aSh);
	}
	else
	if (dst_flags & FMT_PRE)
		return ((AMuint32)a0 << 24) | ((AMuint32)a0 << 16) | ((AMuint32)a0 << 8) | (AMuint32)a0;
	else
		return ((AMuint32)0xFF << dst_rSh) | ((AMuint32)0xFF << dst_gSh) | ((AMuint32)0xFF << dst_bSh) | ((AMuint32)a0 << dst_aSh);
}
#endif

/*!
	\brief Get a sample from a given 1bit image at (x, y) coordinates, according to the specified sampler
	parameters.
	\param params input sampler parameters.
	\return the 32bit color value of the sample.
*/
AMuint32 amImageSample_1_Get(const AMImageSamplerParams *params) {

	AMuint8 bw0;
	const AMImage *image = params->image;
	// take care of child images
	AMint32 childX = image->x;
	AMint32 childY = image->y;
	AMint32 stride = image->root->dataStride;
	AMuint8 *src8 = (AMuint8 *)image->pixels;
	AMuint32 dst_rSh = pxlFormatTable[params->dstIdx][FMT_R_SH];
	AMuint32 dst_gSh = pxlFormatTable[params->dstIdx][FMT_G_SH];
	AMuint32 dst_bSh = pxlFormatTable[params->dstIdx][FMT_B_SH];
	AMuint32 dst_aSh = pxlFormatTable[params->dstIdx][FMT_A_SH];
#if defined RIM_VG_SRC || defined TORCH_VG_SRC
#if (AM_OPENVG_VERSION >= 110)
	AMuint32 dst_flags = pxlFormatTable[params->dstIdx][FMT_FLAGS];
#endif
#endif

	AM_ASSERT(pxlFormatTable[params->srcIdx][FMT_BITS] == 1);
	AM_ASSERT(pxlFormatTable[params->dstIdx][FMT_BITS] == 32);

	// params->x and params->y are in 16.16 fixed point format
	if (image->isPow2) {
		if (params->bilinear) {
			AMint32 u = (params->x - 0x8000) >> 16;
			AMint32 v = (params->y - 0x8000) >> 16;
			AMint32 fru = ((params->x - 0x8000) >> 8) & 0xFF;
			AMint32 frv = ((params->y - 0x8000) >> 8) & 0xFF;
			AMuint8 bw1, bw2, bw3;

			AM_SAMPLE1_POW2_GET(bw0, u, v, params->tilingMode, src8)
			bw0 = ((bw0 >> (u & 7)) & 1) * 255;
			AM_SAMPLE1_POW2_GET(bw1, u + 1, v, params->tilingMode, src8)
			bw1 = ((bw1 >> ((u + 1) & 7)) & 1) * 255;
			AM_SAMPLE1_POW2_GET(bw2, u, v + 1, params->tilingMode, src8)
			bw2 = ((bw2 >> (u & 7)) & 1) * 255;
			AM_SAMPLE1_POW2_GET(bw3, u + 1, v + 1, params->tilingMode, src8)
			bw3 = ((bw3 >> ((u + 1) & 7)) & 1) * 255;
			
			bw0 = (AMuint8)(((AMuint32)(255 - fru) * bw0 + (AMuint32)fru * bw1) >> 8);
			bw2 = (AMuint8)(((AMuint32)(255 - fru) * bw2 + (AMuint32)fru * bw3) >> 8);
			bw0 = (AMuint8)(((AMuint32)(255 - frv) * bw0 + (AMuint32)frv * bw2) >> 8);
		}
		else {
			AMint32 u = params->x >> 16;
			AMint32 v = params->y >> 16;
			AM_SAMPLE1_POW2_GET(bw0, u, v, params->tilingMode, src8)
			bw0 = ((bw0 >> (u & 7)) & 1) * 255;
		}
	}
	else {
		if (params->bilinear) {
			AMint32 u = (params->x - 0x8000) >> 16;
			AMint32 v = (params->y - 0x8000) >> 16;
			AMint32 fru = ((params->x - 0x8000) >> 8) & 0xFF;
			AMint32 frv = ((params->y - 0x8000) >> 8) & 0xFF;
			AMuint8 bw1, bw2, bw3;

			AM_SAMPLE1_GET(bw0, u, v, params->tilingMode, src8)
			bw0 = ((bw0 >> (u & 7)) & 1) * 255;
			AM_SAMPLE1_GET(bw1, u + 1, v, params->tilingMode, src8)
			bw1 = ((bw1 >> ((u + 1) & 7)) & 1) * 255;
			AM_SAMPLE1_GET(bw2, u, v + 1, params->tilingMode, src8)
			bw2 = ((bw2 >> (u & 7)) & 1) * 255;
			AM_SAMPLE1_GET(bw3, u + 1, v + 1, params->tilingMode, src8)
			bw3 = ((bw3 >> ((u + 1) & 7)) & 1) * 255;

			bw0 = (AMuint8)(((AMuint32)(255 - fru) * bw0 + (AMuint32)fru * bw1) >> 8);
			bw2 = (AMuint8)(((AMuint32)(255 - fru) * bw2 + (AMuint32)fru * bw3) >> 8);
			bw0 = (AMuint8)(((AMuint32)(255 - frv) * bw0 + (AMuint32)frv * bw2) >> 8);
		}
		else {
			AMint32 u = params->x >> 16;
			AMint32 v = params->y >> 16;
			AM_SAMPLE1_GET(bw0, u, v, params->tilingMode, src8)
			bw0 = ((bw0 >> (u & 7)) & 1) * 255;
		}
	}

#if (AM_OPENVG_VERSION >= 110)
	if (image->format == VG_A_1) {

		if (params->colorTransformation) {

			AMPixel32RGBA t0;
			AMint32 tmpR = ((AMint32)0xFF * params->colorTransformation[0] + params->colorTransformation[4]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION;
			AMint32 tmpG = ((AMint32)0xFF * params->colorTransformation[1] + params->colorTransformation[5]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION;
			AMint32 tmpB = ((AMint32)0xFF * params->colorTransformation[2] + params->colorTransformation[6]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION;
			AMint32 tmpA = ((AMint32)bw0 * params->colorTransformation[3] + params->colorTransformation[7]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION;

			t0.c.r = (AMuint8)(AM_CLAMP(tmpR, 0, 255));
			t0.c.g = (AMuint8)(AM_CLAMP(tmpG, 0, 255));
			t0.c.b = (AMuint8)(AM_CLAMP(tmpB, 0, 255));
			t0.c.a = (AMuint8)(AM_CLAMP(tmpA, 0, 255));

			// l --> s conversion
			if (!(dst_flags & FMT_L)) {
				t0.c.r = AM_GAMMA_TABLE(t0.c.r);
				t0.c.g = AM_GAMMA_TABLE(t0.c.g);
				t0.c.b = AM_GAMMA_TABLE(t0.c.b);
			}
			// premultiply if requested by the destination format
			if (dst_flags & FMT_PRE)
				t0.rgba = amPxl_RGBA_Scl255PreserveA(t0.c.a, t0.rgba);

			return ((AMuint32)t0.c.r << dst_rSh) | ((AMuint32)t0.c.g << dst_gSh) | ((AMuint32)t0.c.b << dst_bSh) | ((AMuint32)t0.c.a << dst_aSh);
		}
		else
		if (dst_flags & FMT_PRE)
			return ((AMuint32)bw0 << 24) | ((AMuint32)bw0 << 16) | ((AMuint32)bw0 << 8) | (AMuint32)bw0;
		else
			return ((AMuint32)0xFF << dst_rSh) | ((AMuint32)0xFF << dst_gSh) | ((AMuint32)0xFF << dst_bSh) | ((AMuint32)bw0 << dst_aSh);
	}
	else {
		AM_ASSERT(image->format == VG_BW_1);

		if (params->colorTransformation) {

			AMPixel32RGBA t0;
			AMint32 tmpR = ((AMint32)bw0 * params->colorTransformation[0] + params->colorTransformation[4]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION;
			AMint32 tmpG = ((AMint32)bw0 * params->colorTransformation[1] + params->colorTransformation[5]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION;
			AMint32 tmpB = ((AMint32)bw0 * params->colorTransformation[2] + params->colorTransformation[6]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION;
			AMint32 tmpA = ((AMint32)255 * params->colorTransformation[3] + params->colorTransformation[7]) >> AM_COLOR_TRANSFORM_SCALE_BIAS_PRECISION;

			t0.c.r = (AMuint8)(AM_CLAMP(tmpR, 0, 255));
			t0.c.g = (AMuint8)(AM_CLAMP(tmpG, 0, 255));
			t0.c.b = (AMuint8)(AM_CLAMP(tmpB, 0, 255));
			t0.c.a = (AMuint8)(AM_CLAMP(tmpA, 0, 255));

			// l --> s conversion
			if (!(dst_flags & FMT_L)) {
				t0.c.r = AM_GAMMA_TABLE(t0.c.r);
				t0.c.g = AM_GAMMA_TABLE(t0.c.g);
				t0.c.b = AM_GAMMA_TABLE(t0.c.b);
			}
			// premultiply if requested by the destination format
			if (dst_flags & FMT_PRE)
				t0.rgba = amPxl_RGBA_Scl255PreserveA(t0.c.a, t0.rgba);

			return ((AMuint32)t0.c.r << dst_rSh) | ((AMuint32)t0.c.g << dst_gSh) | ((AMuint32)t0.c.b << dst_bSh) | ((AMuint32)t0.c.a << dst_aSh);
		}
	}
#endif
	return ((AMuint32)bw0 << dst_rSh) | ((AMuint32)bw0 << dst_gSh) | ((AMuint32)bw0 << dst_bSh) | ((AMuint32)0xFF << dst_aSh);
}


// {A,X}RGB channel ordering
// BGR{A,X} channel ordering

/*!
	\brief Get a sample from a given 32bit BGRA image at (x, y) coordinates, according to the specified sampler
	parameters.
	\param params input sampler parameters.
	\return the 32bit color value of the sample.
*/
AMuint32 amImageSample_BGRA32_Get(const AMImageSamplerParams *params) {

	AMPixel32BGRA t0;
	const AMImage *image = params->image;
	// take care of child images
	AMint32 childX = image->x;
	AMint32 childY = image->y;
	AMint32 stride = image->root->width;
	const AMuint32 *src32 = (const AMuint32 *)image->pixels;
	AMuint32 src_flags = pxlFormatTable[params->srcIdx][FMT_FLAGS];
	AMuint32 dst_rSh = pxlFormatTable[params->dstIdx][FMT_R_SH];
	AMuint32 dst_gSh = pxlFormatTable[params->dstIdx][FMT_G_SH];
	AMuint32 dst_bSh = pxlFormatTable[params->dstIdx][FMT_B_SH];
	AMuint32 dst_aSh = pxlFormatTable[params->dstIdx][FMT_A_SH];
	AMuint32 dst_flags = pxlFormatTable[params->dstIdx][FMT_FLAGS];

	AM_ASSERT(pxlFormatTable[params->srcIdx][FMT_BITS] == 32);
	AM_ASSERT(pxlFormatTable[params->dstIdx][FMT_BITS] == 32);

	// params->x and params->y are in 16.16 fixed point format
	if (image->isPow2) {
		if (params->bilinear) {
			AMint32 u = (params->x - 0x8000) >> 16;
			AMint32 v = (params->y - 0x8000) >> 16;
			AMint32 fru = ((params->x - 0x8000) >> 8) & 0xFF;
			AMint32 frv = ((params->y - 0x8000) >> 8) & 0xFF;
			AMPixel32BGRA t1, t2, t3;

			AM_SAMPLE32_POW2_GET(t0, u, v, params->tilingMode, src32)
			AM_SAMPLE32_POW2_GET(t1, u + 1, v, params->tilingMode, src32)
			AM_SAMPLE32_POW2_GET(t2, u, v + 1, params->tilingMode, src32)
			AM_SAMPLE32_POW2_GET(t3, u + 1, v + 1, params->tilingMode, src32)
			t0.bgra = amPxlLerp(fru, t0.bgra, t1.bgra);
			t2.bgra = amPxlLerp(fru, t2.bgra, t3.bgra);
			t0.bgra = amPxlLerp(frv, t0.bgra, t2.bgra);
		}
		else {
			AMint32 u = params->x >> 16;
			AMint32 v = params->y >> 16;
			AM_SAMPLE32_POW2_GET(t0, u, v, params->tilingMode, src32)
		}
	}
	else {
		if (params->bilinear) {
			AMint32 u = (params->x - 0x8000) >> 16;
			AMint32 v = (params->y - 0x8000) >> 16;
			AMint32 fru = ((params->x - 0x8000) >> 8) & 0xFF;
			AMint32 frv = ((params->y - 0x8000) >> 8) & 0xFF;
			AMPixel32BGRA t1, t2, t3;

			AM_SAMPLE32_GET(t0, u, v, params->tilingMode, src32)
			AM_SAMPLE32_GET(t1, u + 1, v, params->tilingMode, src32)
			AM_SAMPLE32_GET(t2, u, v + 1, params->tilingMode, src32)
			AM_SAMPLE32_GET(t3, u + 1, v + 1, params->tilingMode, src32)
			t0.bgra = amPxlLerp(fru, t0.bgra, t1.bgra);
			t2.bgra = amPxlLerp(fru, t2.bgra, t3.bgra);
			t0.bgra = amPxlLerp(frv, t0.bgra, t2.bgra);
		}
		else {
			AMint32 u = params->x >> 16;
			AMint32 v = params->y >> 16;
			AM_SAMPLE32_GET(t0, u, v, params->tilingMode, src32)
		}
	}

	if (!(src_flags & FMT_ALPHA))
		t0.c.a = 0xFF;

#if (AM_OPENVG_VERSION >= 110)
	AM_COLOR_TRANSFORM32()
#endif

	if ((src_flags & (FMT_PRE | FMT_L)) == (dst_flags & (FMT_PRE | FMT_L)))
		return ((AMuint32)t0.c.r << dst_rSh) | ((AMuint32)t0.c.g << dst_gSh) | ((AMuint32)t0.c.b << dst_bSh) | ((AMuint32)t0.c.a << dst_aSh);
	else {
		// remove premultiplication, if source format was premultiplied
		if (src_flags & FMT_PRE) {
			if (t0.c.a == 0)
				return 0;
			else {
				AM_ASSERT(t0.c.r <= t0.c.a && t0.c.g <= t0.c.a && t0.c.b <= t0.c.a);
				// remove premultiplication
				AM_UNPREMULTIPLY(t0.c.r, t0.c.g, t0.c.b, t0.c.r, t0.c.g, t0.c.b, t0.c.a)
			}
		}
		// l --> s conversion
		if ((src_flags & FMT_L) && !(dst_flags & FMT_L)) {
			t0.c.r = AM_GAMMA_TABLE(t0.c.r);
			t0.c.g = AM_GAMMA_TABLE(t0.c.g);
			t0.c.b = AM_GAMMA_TABLE(t0.c.b);
		}
		else
		// s --> l conversion
		if (!(src_flags & FMT_L) && (dst_flags & FMT_L)) {
			t0.c.r = AM_GAMMA_INV_TABLE(t0.c.r);
			t0.c.g = AM_GAMMA_INV_TABLE(t0.c.g);
			t0.c.b = AM_GAMMA_INV_TABLE(t0.c.b);
		}

		// premultiply if requested by the destination format
		if (dst_flags & FMT_PRE)
			t0.bgra = amPxl_BGRA_Scl255PreserveA(t0.c.a, t0.bgra);

		return ((AMuint32)t0.c.r << dst_rSh) | ((AMuint32)t0.c.g << dst_gSh) | ((AMuint32)t0.c.b << dst_bSh) | ((AMuint32)t0.c.a << dst_aSh);
	}
}

// {A,X}BGR channel ordering
/*!
	\brief Get a sample from a given 32bit ABGR image at (x, y) coordinates, according to the specified sampler
	parameters.
	\param params input sampler parameters.
	\return the 32bit color value of the sample.
*/
AMuint32 amImageSample_ABGR32_Get(const AMImageSamplerParams *params) {

	AMPixel32ABGR t0;
	const AMImage *image = params->image;
	// take care of child images
	AMint32 childX = image->x;
	AMint32 childY = image->y;
	AMint32 stride = image->root->width;
	const AMuint32 *src32 = (const AMuint32 *)image->pixels;
	AMuint32 src_flags = pxlFormatTable[params->srcIdx][FMT_FLAGS];
	AMuint32 dst_rSh = pxlFormatTable[params->dstIdx][FMT_R_SH];
	AMuint32 dst_gSh = pxlFormatTable[params->dstIdx][FMT_G_SH];
	AMuint32 dst_bSh = pxlFormatTable[params->dstIdx][FMT_B_SH];
	AMuint32 dst_aSh = pxlFormatTable[params->dstIdx][FMT_A_SH];
	AMuint32 dst_flags = pxlFormatTable[params->dstIdx][FMT_FLAGS];

	AM_ASSERT(pxlFormatTable[params->srcIdx][FMT_BITS] == 32);
	AM_ASSERT(pxlFormatTable[params->dstIdx][FMT_BITS] == 32);

	// params->x and params->y are in 16.16 fixed point format
	if (image->isPow2) {
		if (params->bilinear) {
			AMint32 u = (params->x - 0x8000) >> 16;
			AMint32 v = (params->y - 0x8000) >> 16;
			AMint32 fru = ((params->x - 0x8000) >> 8) & 0xFF;
			AMint32 frv = ((params->y - 0x8000) >> 8) & 0xFF;
			AMPixel32ABGR t1, t2, t3;

			AM_SAMPLE32_POW2_GET(t0, u, v, params->tilingMode, src32)
			AM_SAMPLE32_POW2_GET(t1, u + 1, v, params->tilingMode, src32)
			AM_SAMPLE32_POW2_GET(t2, u, v + 1, params->tilingMode, src32)
			AM_SAMPLE32_POW2_GET(t3, u + 1, v + 1, params->tilingMode, src32)
			t0.abgr = amPxlLerp(fru, t0.abgr, t1.abgr);
			t2.abgr = amPxlLerp(fru, t2.abgr, t3.abgr);
			t0.abgr = amPxlLerp(frv, t0.abgr, t2.abgr);
		}
		else {
			AMint32 u = params->x >> 16;
			AMint32 v = params->y >> 16;
			AM_SAMPLE32_POW2_GET(t0, u, v, params->tilingMode, src32)
		}
	}
	else {
		if (params->bilinear) {
			AMint32 u = (params->x - 0x8000) >> 16;
			AMint32 v = (params->y - 0x8000) >> 16;
			AMint32 fru = ((params->x - 0x8000) >> 8) & 0xFF;
			AMint32 frv = ((params->y - 0x8000) >> 8) & 0xFF;
			AMPixel32ABGR t1, t2, t3;

			AM_SAMPLE32_GET(t0, u, v, params->tilingMode, src32)
			AM_SAMPLE32_GET(t1, u + 1, v, params->tilingMode, src32)
			AM_SAMPLE32_GET(t2, u, v + 1, params->tilingMode, src32)
			AM_SAMPLE32_GET(t3, u + 1, v + 1, params->tilingMode, src32)
			t0.abgr = amPxlLerp(fru, t0.abgr, t1.abgr);
			t2.abgr = amPxlLerp(fru, t2.abgr, t3.abgr);
			t0.abgr = amPxlLerp(frv, t0.abgr, t2.abgr);
		}
		else {
			AMint32 u = params->x >> 16;
			AMint32 v = params->y >> 16;
			AM_SAMPLE32_GET(t0, u, v, params->tilingMode, src32)
		}
	}

	if (!(src_flags & FMT_ALPHA))
		t0.c.a = 0xFF;

#if (AM_OPENVG_VERSION >= 110)
	AM_COLOR_TRANSFORM32()
#endif

	if ((src_flags & (FMT_PRE | FMT_L)) == (dst_flags & (FMT_PRE | FMT_L)))
		return ((AMuint32)t0.c.r << dst_rSh) | ((AMuint32)t0.c.g << dst_gSh) | ((AMuint32)t0.c.b << dst_bSh) | ((AMuint32)t0.c.a << dst_aSh);
	else {
		// remove premultiplication, if source format was premultiplied
		if (src_flags & FMT_PRE) {
			if (t0.c.a == 0)
				return 0;
			else {
				AM_ASSERT(t0.c.r <= t0.c.a && t0.c.g <= t0.c.a && t0.c.b <= t0.c.a);
				// remove premultiplication
				AM_UNPREMULTIPLY(t0.c.r, t0.c.g, t0.c.b, t0.c.r, t0.c.g, t0.c.b, t0.c.a)
			}
		}
		// l --> s conversion
		if ((src_flags & FMT_L) && !(dst_flags & FMT_L)) {
			t0.c.r = AM_GAMMA_TABLE(t0.c.r);
			t0.c.g = AM_GAMMA_TABLE(t0.c.g);
			t0.c.b = AM_GAMMA_TABLE(t0.c.b);
		}
		else
		// s --> l conversion
		if (!(src_flags & FMT_L) && (dst_flags & FMT_L)) {
			t0.c.r = AM_GAMMA_INV_TABLE(t0.c.r);
			t0.c.g = AM_GAMMA_INV_TABLE(t0.c.g);
			t0.c.b = AM_GAMMA_INV_TABLE(t0.c.b);
		}

		// premultiply if requested by the destination format
		if (dst_flags & FMT_PRE)
			t0.abgr = amPxl_ABGR_Scl255PreserveA(t0.c.a, t0.abgr);

		return ((AMuint32)t0.c.r << dst_rSh) | ((AMuint32)t0.c.g << dst_gSh) | ((AMuint32)t0.c.b << dst_bSh) | ((AMuint32)t0.c.a << dst_aSh);
	}
}

/*!
	\brief Get an image sampler corresponding to the given image format.
	\param format input image format.
	\return image sampler.
*/
AMImageSampler amImageSamplerGet(const VGImageFormat format) {

	AMuint32 srcIdx = AM_FMT_GET_INDEX(format);
	AMPixelFormat fmt;
	AMImageSampler res;

	switch ((format >> 6) & 3) {
		case 0:
			fmt = AM_PIXEL_FMT_RGBA;
			break;
		case 1:
			fmt = AM_PIXEL_FMT_ARGB;
			break;
		case 2:
			fmt = AM_PIXEL_FMT_BGRA;
			break;
		default:
			AM_ASSERT(((format >> 6) & 3) == 3);
			fmt = AM_PIXEL_FMT_ABGR;
			break;
	}

	switch (pxlFormatTable[srcIdx][FMT_BITS]) {
		case 32:
			switch (fmt) {
				case AM_PIXEL_FMT_RGBA:
					res = amImageSample_RGBA32_Get;
					break;
				case AM_PIXEL_FMT_ABGR:
					res = amImageSample_ABGR32_Get;
					break;
				case AM_PIXEL_FMT_ARGB:
					res = amImageSample_ARGB32_Get;
					break;
				case AM_PIXEL_FMT_BGRA:
					res = amImageSample_BGRA32_Get;
					break;
				default:
					AM_ASSERT(0 == 1);
					res = NULL;
					break;
			}
			break;
		case 16:
			switch (fmt) {
				case AM_PIXEL_FMT_RGBA:
					res = amImageSample_RGBA16_Get;
					break;
				case AM_PIXEL_FMT_ABGR:
					res = amImageSample_ARGB16_Get;
					break;
				case AM_PIXEL_FMT_ARGB:
					res = amImageSample_ARGB16_Get;
					break;
				case AM_PIXEL_FMT_BGRA:
					res = amImageSample_RGBA16_Get;
					break;
				default:
					AM_ASSERT(0 == 1);
					res = NULL;
					break;
			}
			break;
		case 8:
			res = amImageSample_8_Get;
			break;
	#if (AM_OPENVG_VERSION >= 110)
		case 4:
			res = amImageSample_4_Get;
			break;
	#endif
		case 1:
			res = amImageSample_1_Get;
			break;
		default:
			AM_ASSERT(0 == 1);
			res = NULL;
			break;
	}
	return res;
}

// *********************************************************************
//                        Private implementations
// *********************************************************************

/*!
	\brief Check if the given image format is a valid OpenVG image format.
	\param format image format to check.
	\return AM_TRUE if specified format is a valid OpenVG image format, else AM_FALSE.
*/
AMbool amImageFormatValid(const VGImageFormat format) {

	AM_ASSERT(VG_sRGBX_8888 == 0);
	
#if (AM_OPENVG_VERSION >= 110)
	return ((format <= VG_A_4) ||
			(format >= VG_sXRGB_8888 && format <= VG_lARGB_8888_PRE) ||
			(format >= VG_sBGRX_8888 && format <= VG_lBGRA_8888_PRE) ||
			(format >= VG_sXBGR_8888 && format <= VG_lABGR_8888_PRE)) ? AM_TRUE : AM_FALSE;
#else
	return (format <= VG_BW_1) ? AM_TRUE : AM_FALSE;
#endif
}

/*!
	\brief Check if two images overlap.
	\param img0 first input image.
	\param img1 second input image.
	\return AM_TRUE if images overlap, else AM_FALSE.
*/
AMbool amImagesOverlap(const AMImage *img0,
					   const AMImage *img1) {

	AMAABox2i box0, box1;

	AM_ASSERT(img0);
	AM_ASSERT(img1);

	if (img0 == img1)
		return AM_TRUE;

	if (img0->pixels != img1->pixels)
		return AM_FALSE;

	AM_VECT2_SET(&box0.minPoint, img0->x, img0->y)
	AM_VECT2_SET(&box0.maxPoint, img0->x + img0->width, img0->y + img0->height)
	AM_VECT2_SET(&box1.minPoint, img1->x, img1->y)
	AM_VECT2_SET(&box1.maxPoint, img1->x + img1->width, img1->y + img1->height)
	// the two images are parented (they share pixels storage); let's check overlapping
	return amAABox2iOverlap(&box0, &box1);
}

/*!
	\brief Return bytes required to store a pixel of a given image format.
	\param format image format.
	\return bytes per pixel.
*/
AMint32 amImageBytesPerPixel(const VGImageFormat format) {

	AMint32 res;

	switch (format) {
		case VG_sRGBX_8888:
		case VG_sRGBA_8888:
		case VG_sRGBA_8888_PRE:
		case VG_lRGBX_8888:
		case VG_lRGBA_8888:
		case VG_lRGBA_8888_PRE:
		case VG_sXRGB_8888:
		case VG_sARGB_8888:
		case VG_sARGB_8888_PRE:
		case VG_lXRGB_8888:
		case VG_lARGB_8888:
		case VG_lARGB_8888_PRE:
		case VG_sBGRX_8888:
		case VG_sBGRA_8888:
		case VG_sBGRA_8888_PRE:
		case VG_lBGRX_8888:
		case VG_lBGRA_8888:
		case VG_lBGRA_8888_PRE:
		case VG_sXBGR_8888:
		case VG_sABGR_8888:
		case VG_sABGR_8888_PRE:  
		case VG_lXBGR_8888:
		case VG_lABGR_8888:
		case VG_lABGR_8888_PRE:
			res = 4;
			break;
		case VG_sRGB_565:
		case VG_sRGBA_5551:
		case VG_sRGBA_4444:
		case VG_sARGB_1555:
		case VG_sARGB_4444:
		case VG_sBGR_565:
		case VG_sBGRA_5551:
		case VG_sBGRA_4444:
		case VG_sABGR_1555:
		case VG_sABGR_4444:
			res = 2;
			break;
		case VG_sL_8:
		case VG_lL_8:
		case VG_A_8:
			res = 1;
			break;
		default:
			res = 0;
			break;
	}
	return res;
}

/*!
	\brief Return bytes required to store an horizontal pixels line, of a given image format.
	\param format image format.
	\param width number of pixels in the horizontal line.
	\return bytes per line.
*/
AMint32 amBytesPerLine(const VGImageFormat format,
					   const AMint32 width) {

	AMint32 res;

	AM_ASSERT(width > 0);

	switch (format) {

		case VG_BW_1:
	#if (AM_OPENVG_VERSION >= 110)
		case VG_A_1:
	#endif
			res = (width & 0x07) ? ((width >> 3) + 1) : (width >> 3);
			break;
#if defined RIM_VG_SRC
	#if (AM_OPENVG_VERSION >= 110)
		case VG_A_4:
			res = (width & 0x01) ? ((width >> 1) + 1) : (width >> 1);
			break;
	#endif
#else
	#if (AM_OPENVG_VERSION >= 110)
		case VG_A_4:
	#endif
			res = (width & 0x01) ? ((width >> 1) + 1) : (width >> 1);
			break;
#endif
		default:
			res = width * amImageBytesPerPixel(format);
	}
	return res;
}

#if defined(AM_GLE) || defined(AM_GLS)

/*!
	\brief Resize the given source image, using bilinear filter (and applying an optional color trasformation), and write the output to the destination 32bit image.
	\param dst output 32bit destination image.
	\param src input source image.
	\param colorTransformation an optional color transformation; if NULL no color transformation is applied.
	\param halfScaleBias if AM_TRUE it applies a scale and a bias equal to 0.5 for each component (used to draw images in stencil mode).
	\pre src != dst, dst must be a 32bit image.
*/
void amImageBilinearResize(AMImage *dst,
						   const AMImage *src,
						   const AMfloat *colorTransformation,
						   const AMbool halfScaleBias) {

	AMuint32 stepX, stepY, i, j;
	AMImageSamplerParams samplerParams;
	AMImageSampler sampler = amImageSamplerGet(src->format);
	AMuint32 *dstPixels = (AMuint32 *)dst->pixels;
	AMuint32 dstIdx = AM_FMT_GET_INDEX(dst->format);
	AMint32 colorTransformationi[8];

	AM_ASSERT(dst);
	AM_ASSERT(src);
	AM_ASSERT(dst != src);
	AM_ASSERT(dst->pixels != src->pixels);

	// use a faster pixelmap conversion if images dimensions are equal
	if (!colorTransformation && !halfScaleBias && dst->width == src->width && dst->height == src->height) {
		amPxlMapConvert(dst->pixels, dst->format, dst->dataStride, dst->x, dst->y, src->pixels, src->format, src->dataStride, src->x, src->y, src->width, src->height, AM_FALSE, AM_TRUE);
		return;
	}

	if (colorTransformation)
		amColorTransformFToI(colorTransformationi, colorTransformation);

	stepX = (((AMuint32)src->width) << 16) / ((AMuint32)dst->width);
	stepY = (((AMuint32)src->height) << 16) / ((AMuint32)dst->height);

	samplerParams.image = src;
	samplerParams.srcIdx = AM_FMT_GET_INDEX(src->format);
	samplerParams.y = 0x8000;

	samplerParams.tilingMode = VG_TILE_PAD;
	samplerParams.dstIdx = dstIdx;
	samplerParams.bilinear = AM_TRUE;
#if (AM_OPENVG_VERSION >= 110)
	samplerParams.colorTransformation = (colorTransformation) ? colorTransformationi : NULL;
#endif

	AM_ASSERT(pxlFormatTable[samplerParams.dstIdx][FMT_BITS] == 32);

	if (halfScaleBias) {

		AMuint32 aSh = pxlFormatTable[dstIdx][FMT_A_SH];
		AMuint32 aMask = 0xFFFFFFFF ^ (0xFF << aSh);

		for (i = (AMuint32)dst->height; i != 0; --i) {
			samplerParams.x = 0x8000;
			for (j = (AMuint32)dst->width; j != 0; --j) {

				AMuint32 sample = sampler(&samplerParams);
				AMuint32 a = (sample >> aSh) & 0xFF;
				AMuint32 lerp = amPxlLerp(128, sample, 0xFFFFFFFF);
				
				*dstPixels++ = (lerp & aMask) | (a << aSh);
				samplerParams.x += stepX;
			}
			samplerParams.y += stepY;
		}
	}
	else {
		for (i = (AMuint32)dst->height; i != 0; --i) {
			samplerParams.x = 0x8000;
			for (j = (AMuint32)dst->width; j != 0; --j) {
				*dstPixels++ = sampler(&samplerParams);
				samplerParams.x += stepX;
			}
			samplerParams.y += stepY;
		}
	}
}

void amImageTexturesTreeInvalidate(AMImage *current,
								   const AMImage *reference) {

	AM_ASSERT(current);
	AM_ASSERT(reference);

	if (amImagesOverlap(current, reference)) {

		AMuint32 i, j;

		current->patternTexturesValid = AM_FALSE;
		current->drawImageTextureValid = AM_FALSE;

		j = current->children.size;
		for (i = 0; i < j; ++i)
			amImageTexturesTreeInvalidate(current->children.data[i], reference);
	}
}

/*!
	\brief It invalidates texture flags, taking care of children.
	\param image image whose textures are to invalidate.
	\param _context pointer to a AMContext structure, containing the handle list.
*/
void amImageTexturesInvalidate(AMImage *image,
							   const void *_context) {

	const AMContext *context = (const AMContext *)_context;
	const AMContextHandlesList *handles = context->handles;
	AMImage *tmpImg;
	VGImage handle;

	AM_ASSERT(image);
	AM_ASSERT(context);

	// set root
	tmpImg = image;
	while (tmpImg->parent) {
		handle = tmpImg->parent;
		tmpImg = handles->createdHandlesList.data[handle];
	}
	// tmpImg is the tree root
	amImageTexturesTreeInvalidate(tmpImg, image);
}
#endif

// Given an image, it returns AM_TRUE if the image is opaque, else AM_FALSE.
AMbool amImageIsOpaque(const AMImage *img) {

	AMuint32 idx = AM_FMT_GET_INDEX(img->format);

	AM_ASSERT(img);

	return (pxlFormatTable[idx][FMT_FLAGS] & FMT_ALPHA) ? AM_FALSE : AM_TRUE;
}

/*!
	\brief Image constructor, it initializes a given image structure.
	\param image image to initialize.
	\param format pixel format use.
	\param allowedQuality bitwise OR of values from the VGImageQuality enumeration, indicating which levels of resampling quality may be used to draw the image.
	\param x for child images, x-coordinate (respect to the parent) of the first pixel.
	\param y for child images, y-coordinate (respect to the parent) of the first pixel.
	\param width width of the image.
	\param height height of the image.
	\param parent handle to an image parent, 0 if the image is not a child image.
	\param _context pointer to a AMContext structure, containing the list of all created handles.
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
	\pre \a format must be a valid OpenVG image format.
	\pre width, height > 0
	\pre \a image must be different than \a parent.
*/
AMbool amImageInit(AMImage *image,
				   const VGImageFormat format,
				   const VGbitfield allowedQuality,
				   const AMint32 x,
				   const AMint32 y,
				   const AMint32 width,
				   const AMint32 height,
				   VGImage parent,
				   void *_context) {

	AMContext *context = (AMContext *)_context;
	AMContextHandlesList *handles = context->handles;
	AMImage *parentImg, *tmpImg;
	VGImage handle;

	AM_ASSERT(image);
	AM_ASSERT(width > 0 && height > 0);
	AM_ASSERT((AMint32)format != (AMint32)VG_IMAGE_FORMAT_INVALID);
	AM_ASSERT(context);
	AM_ASSERT(parent < handles->createdHandlesList.size);

	// NULL values are allowed
	parentImg = handles->createdHandlesList.data[parent];
	AM_ASSERT(image != parentImg);

	image->id = AM_IMAGE_HANDLE_ID;
	image->type = AM_IMAGE_HANDLE_ID;
	image->referenceCounter = 1;
	image->format = format;
	image->allowedQuality = allowedQuality;
	image->width = width;
	image->height = height;
#if !defined(AM_STANDALONE) || defined(RIM_VG_SRC) || defined(TORCH_VG_SRC)
	image->inUseByEgl = AM_FALSE;
#endif

	if (parentImg) {

		AM_ASSERT(format == parentImg->format);
		AM_ASSERT(x >= 0 && x < parentImg->width);
		AM_ASSERT(y >= 0 && y < parentImg->height);
		AM_ASSERT(x + width <= parentImg->width);
		AM_ASSERT(y + height <= parentImg->height);

		image->pixels = parentImg->root->pixels;
		image->dataStride = parentImg->root->dataStride;
		image->parent = parent;
		// set root
		handle = parent;
		tmpImg = handles->createdHandlesList.data[handle];
		while (tmpImg->parent) {
			handle = tmpImg->parent;
			tmpImg = handles->createdHandlesList.data[handle];
		}
		image->root = tmpImg;
		// initialize children array
		AM_DYNARRAY_INIT(image->children, AMImagePtr);
		// check for memory errors
		if (image->children.error)
			return AM_FALSE;
		// now the father has a new child
		AM_DYNARRAY_PUSH_BACK(parentImg->children, AMImagePtr, image)
		// check for memory errors
		if (parentImg->children.error) {
			parentImg->children.error = AM_DYNARRAY_NO_ERROR;
			AM_DYNARRAY_DESTROY(image->children)
			return AM_FALSE;
		}
		// increment the reference counter of parent and root images
		amCtxHandleRefCounterInc(parentImg);
		amCtxHandleRefCounterInc(image->root);
		// calculates offsets respect to the "root" image
		image->x = x + parentImg->x;
		image->y = y + parentImg->y;
	}
	else {
		AMint32 bytesPerLine = amBytesPerLine(format, width);

		AM_ASSERT(bytesPerLine > 0);
		image->dataStride = bytesPerLine;
		// allocate pixels
		image->pixels = (AMuint8 *)amMalloc(bytesPerLine * height * sizeof(AMuint8));
		// check for memory errors
		if (!image->pixels)
			return AM_FALSE;
			// according to OpenVG specification images must be created filled with 0 / black
			amMemset(image->pixels, 0, bytesPerLine * height);
		// initialize children array
		AM_DYNARRAY_INIT(image->children, AMImagePtr)
		// check for memory errors
		if (image->children.error) {
			amFree(image->pixels);
			return AM_FALSE;
		}
			// this is a "root" image, so offsets must be set to 0
			image->x = 0;
			image->y = 0;
			// set root
			image->root = image;
			// set parent and children
		image->parent = VG_INVALID_HANDLE;
	}

	// check if pow2
	if (amPow2Check(image->width) && amPow2Check(image->height)) {
		image->isPow2 = AM_TRUE;
		image->widthShift = amPow2Shift(image->width);
		image->heightShift = amPow2Shift(image->height);
	}
	else {
		image->isPow2 = AM_FALSE;
		image->widthShift = 0;
		image->heightShift = 0;
	}
#if defined(AM_GLE) || defined(AM_GLS)
	// initialize textures
	amTextureInit(&image->patternMainTexture);
	amTextureInit(&image->patternReflectTexture);
	amTextureInit(&image->patternFillTexture);
	amTextureInit(&image->drawImageTexture);
	image->patternTexturesValid = AM_FALSE;
	image->patternTexturePixelsFormat = (VGImageFormat)-1;
	image->drawImageTextureValid = AM_FALSE;
	image->drawImageTexturePixelsAreStencil = AM_FALSE;
	image->drawImageTexturePixelsFormat = (VGImageFormat)-1;
#if (AM_OPENVG_VERSION >= 110)
	image->ctPatternTexturesHash = AM_COLOR_TRANSFORM_IDENTITY_HASH;
	image->ctDrawImageTextureHash = AM_COLOR_TRANSFORM_IDENTITY_HASH;
#endif
#endif
	return AM_TRUE;
}

// Destroy image resources.
void amImageResourcesDestroy(AMImage *image,
							 void *_context) {

	AMContext *context = (AMContext *)_context;

	AM_ASSERT(image);
	AM_ASSERT(context);

	(void)context;

	// destroy children array
	AM_DYNARRAY_DESTROY(image->children)

	// destroy pixels for root images
	if (image == image->root) {
		AM_ASSERT(image->pixels);
		amFree(image->pixels);
		image->pixels = NULL;
	}

#if defined(AM_GLE) || defined(AM_GLS)
	// destroy textures
	amTextureDestroy(&image->patternMainTexture);
	amTextureDestroy(&image->patternReflectTexture);
	amTextureDestroy(&image->patternFillTexture);
	amTextureDestroy(&image->drawImageTexture);
#endif
}

/*!
	\brief Destroy the specified image, releasing associated resources.
	\param image image to destroy.
	\param _context pointer to a AMContext structure, containing the list of all created handles.
*/
void amImageDestroy(AMImage *image,
					void *_context) {

	AMContext *context = (AMContext *)_context;
	AMuint32 i;

	AM_ASSERT(image);
	AM_ASSERT(context);

	// keep hierarchy updated
	if (image->parent) {

		AMImage *parentImg = (AMImage *)context->handles->createdHandlesList.data[image->parent];

		// child image
		AM_ASSERT(parentImg);

		// remove image from the children list of image->parent
		for (i = 0; i < parentImg->children.size; ++i) {
			if (parentImg->children.data[i] == image) {
				AM_DYNARRAY_ERASE(parentImg->children, i)
				break;
			}
		}

		for (i = 0; i < image->children.size; ++i) {
			// update child parent
			image->children.data[i]->parent = image->parent;
			// add the child to the children list of image->parent
			AM_DYNARRAY_PUSH_BACK(parentImg->children, AMImagePtr, image->children.data[i])
			amCtxHandleRefCounterDec(image, context);
			amCtxHandleRefCounterInc(parentImg);
		}

		amCtxHandleRefCounterDec(parentImg, context);
	}
	else {
		// root (child) image
		for (i = 0; i < image->children.size; ++i) {
			image->children.data[i]->parent = 0;
			amCtxHandleRefCounterDec(image, context);
		}
	}

	// decrement reference counter for root image
	if (image->root != image)
		amCtxHandleRefCounterDec(image->root, context);

	// decrement reference counter, it possibly leads to resources deallocation
	amCtxHandleRefCounterDec(image, context);

	// sign that this image is not more valid
	image->id = AM_INVALID_HANDLE_ID;
}

/*!
	\brief Fill a given rectangle of an image with the specified color.
	\param image image to clear.
	\param x x-coordinate of the first pixel to clear.
	\param y y-coordinate of the first pixel to clear.
	\param width width of the area to clear.
	\param height height of the area to clear.
	\param sRGBAcolor clear color to use in non-linear color space, float components ([0;1] range).
	\param _context pointer to a AMContext structure, containing the GL context (used to invalidate image textures, in AmanithVG GLE).
	\pre width, height > 0.
*/
void amImageClear(AMImage *image,
				  AMint32 x,
				  AMint32 y,
				  AMint32 width,
				  AMint32 height,
				  const AMfloat *sRGBAcolor,
				  const void *_context) {

	AMuint32 rgba32, r32, g32, b32, a32;
	AMuint16 rgba16, r16, g16, b16, a16;
	AMuint8 *dst8, rgba8;
	AMint32 i, j;
	AMfloat color[4], luminance;
	AMuint32 tableIdx, rShift, gShift, bShift, aShift, rBits, gBits, bBits, aBits, flags;

	AM_ASSERT(image && (AMint32)image->format != (AMint32)VG_IMAGE_FORMAT_INVALID && image->pixels);
	AM_ASSERT(width > 0);
	AM_ASSERT(height > 0);

	// clip specified bound
	if (x < 0) {
		width += x;
		if (width <= 0)
			return;
		x = 0;
	}
	if (y < 0) {
		height += y;
		if (height <= 0)
			return;
		y = 0;
	}
	if (x + width > image->width) {
		width = image->width - x;
		if (width <= 0)
			return;
	}
	if (y + height > image->height) {
		height = image->height - y;
		if (height <= 0)
			return;
	}

	// take into account child images
	x += image->x;
	y += image->y;

	AM_CLAMP4(color, sRGBAcolor, 0.0f, 1.0f)

	// convert clear color into image format
	tableIdx = AM_FMT_GET_INDEX(image->format);
	rShift = pxlFormatTable[tableIdx][FMT_R_SH];
	gShift = pxlFormatTable[tableIdx][FMT_G_SH];
	bShift = pxlFormatTable[tableIdx][FMT_B_SH];
	aShift = pxlFormatTable[tableIdx][FMT_A_SH];
	rBits = pxlFormatTable[tableIdx][FMT_R_BITS];
	gBits = pxlFormatTable[tableIdx][FMT_G_BITS];
	bBits = pxlFormatTable[tableIdx][FMT_B_BITS];
	aBits = pxlFormatTable[tableIdx][FMT_A_BITS];
	flags = pxlFormatTable[tableIdx][FMT_FLAGS];

	switch (pxlFormatTable[tableIdx][FMT_BITS]) {
		case 32:
			if (flags & FMT_L) {
				color[AM_R] = amGammaInvConversion(color[AM_R]);
				color[AM_G] = amGammaInvConversion(color[AM_G]);
				color[AM_B] = amGammaInvConversion(color[AM_B]);
			}
			if (flags & FMT_PRE) {
				color[AM_R] *= color[AM_A];
				color[AM_G] *= color[AM_A];
				color[AM_B] *= color[AM_A];
			}
			r32 = (AMuint32)amFloorf(color[AM_R] * 255.0f + 0.5f);
			g32 = (AMuint32)amFloorf(color[AM_G] * 255.0f + 0.5f);
			b32 = (AMuint32)amFloorf(color[AM_B] * 255.0f + 0.5f);
			a32 = (AMuint32)amFloorf(color[AM_A] * 255.0f + 0.5f);
			rgba32 = (r32 << rShift) | (g32 << gShift) | (b32 << bShift) | (a32 << aShift);
			dst8 = &image->pixels[y * image->dataStride + x * 4];
			for (i = 0; i < height; ++i) {
				AMuint32 *dst32 = (AMuint32 *)dst8;
				for (j = 0; j < width; ++j)
					*dst32++ = rgba32;
				dst8 += image->dataStride;
			}
			break;
		case 16:
			r16 = (AMuint16)amFloorf(color[AM_R] * (AMfloat)((1 << rBits) - 1) + 0.5f);
			g16 = (AMuint16)amFloorf(color[AM_G] * (AMfloat)((1 << gBits) - 1) + 0.5f);
			b16 = (AMuint16)amFloorf(color[AM_B] * (AMfloat)((1 << bBits) - 1) + 0.5f);
			a16 = (AMuint16)amFloorf(color[AM_A] * (AMfloat)((1 << aBits) - 1) + 0.5f);
			rgba16 = (r16 << rShift) | (g16 << gShift) | (b16 << bShift) | (a16 << aShift);
			dst8 = &image->pixels[y * image->dataStride + x * 2];
			for (i = 0; i < height; ++i) {
				AMuint16 *dst16 = (AMuint16 *)dst8;
				for (j = 0; j < width; ++j)
					*dst16++ = rgba16;
				dst8 += image->dataStride;
			}
			break;
		case 8:
			if (image->format == VG_A_8)
				rgba8 = (AMuint8)amFloorf(color[AM_A] * 255.0f + 0.5f);
			else {
				// transform the color in the linear color space
				color[AM_R] = amGammaInvConversion(color[AM_R]);
				color[AM_G] = amGammaInvConversion(color[AM_G]);
				color[AM_B] = amGammaInvConversion(color[AM_B]);
				luminance = color[AM_R] * 0.2126f + color[AM_G] * 0.7152f + color[AM_B] * 0.0722f;
				AM_ASSERT(luminance >= 0.0f && luminance <= 1.0f);
				if (image->format == VG_lL_8)
					rgba8 = (AMuint8)amFloorf(255.0f * luminance + 0.5f);
				else {
					AM_ASSERT(image->format == VG_sL_8);
					rgba8 = (AMuint8)amFloorf(255.0f * amGammaConversion(luminance) + 0.5f);
				}
			}
			dst8 = &image->pixels[y * image->dataStride + x];
			for (i = 0; i < height; ++i) {
				AMuint8 *tmpDst8 = dst8;
				for (j = 0; j < width; ++j)
					*tmpDst8++ = rgba8;
				dst8 += image->dataStride;
			}
			break;
	#if (AM_OPENVG_VERSION >= 110)
		case 4:
			AM_ASSERT(image->format == VG_A_4);

			dst8 = &image->pixels[y * image->dataStride];
			rgba8 = (AMuint8)amFloorf(color[AM_A] * 15.0f + 0.5f);
			AM_ASSERT(rgba8 <= 15);
			for (i = 0; i < height; ++i) {
				for (j = 0; j < width; ++j) {

					AMuint32 xByte = (AMuint32)(x + j);
					AMuint8 dst = dst8[xByte >> 1];

					dst8[xByte >> 1] = (xByte & 1) ? (dst & 0x0F) | (rgba8 << 4) : (dst & 0xF0) | (rgba8);
				}
				dst8 += image->dataStride;
			}
			break;
	#endif
		case 1:
		#if (AM_OPENVG_VERSION >= 110)
			if (image->format == VG_BW_1) {
		#endif
				// transform the color in the linear color space
				color[AM_R] = amGammaInvConversion(color[AM_R]);
				color[AM_G] = amGammaInvConversion(color[AM_G]);
				color[AM_B] = amGammaInvConversion(color[AM_B]);
				luminance = color[AM_R] * 0.2126f + color[AM_G] * 0.7152f + color[AM_B] * 0.0722f;
				AM_ASSERT(luminance >= 0.0f && luminance <= 1.0f);
				rgba8 = (AMuint8)amFloorf(255.0f * luminance + 0.5f);
				rgba8 >>= 7;
				dst8 = &image->pixels[y * image->dataStride];
				for (i = 0; i < height; ++i) {
					for (j = 0; j < width; ++j) {
						AMuint32 xByte = (AMuint32)(x + j);
						AMuint8 xBit = (AMuint8)(xByte & 7);
						AMuint8 mask = (AMuint8)(1 << xBit);
						if (rgba8 == 0)
							dst8[xByte >> 3] &= mask ^ 0xFF;
						else
							dst8[xByte >> 3] |= mask;
					}
					dst8 += image->dataStride;
				}
		#if (AM_OPENVG_VERSION >= 110)
			}
			else {
				AM_ASSERT(image->format == VG_A_1);

				dst8 = &image->pixels[y * image->dataStride];
				rgba8 = (AMuint8)amFloorf(color[AM_A] + 0.5f);
				AM_ASSERT(rgba8 <= 1);
				for (i = 0; i < height; ++i) {
					for (j = 0; j < width; ++j) {
						AMuint32 xByte = (AMuint32)(x + j);
						AMuint8 xBit = (AMuint8)(xByte & 7);
						AMuint8 mask = (AMuint8)(1 << xBit);
						if (rgba8 == 0)
							dst8[xByte >> 3] &= mask ^ 0xFF;
						else
							dst8[xByte >> 3] |= mask;
					}
					dst8 += image->dataStride;
				}
			}
		#endif
			break;
		default:
			AM_ASSERT(0 == 1);
			break;
	}
#if defined(AM_GLE) || defined(AM_GLS)
	// invalidate image textures
	amImageTexturesInvalidate(image, _context);
#else
	(void)_context;
#endif
}

/*!
	\brief It reads pixel values from memory, performs format conversion if necessary, and stores the
	resulting pixels into a rectangular portion of an image.
	\param image destination image.
	\param data pointer to source pixels.
	\param dataFormat data format of source pixels.
	\param dataStride data stride of source pixels.
	\param x x-coordinate of the rectangular region to write.
	\param y y-coordinate of the rectangular region to write.
	\param width width of the rectangular region to write, in pixels.
	\param height height of the rectangular region to write, in pixels.
	\param _context pointer to a AMContext structure, containing the GL context (used to invalidate image textures, in AmanithVG GLE).
	\pre width, height > 0.
*/
void amImageSubDataSet(AMImage *image,
					   const void *data,
					   const VGImageFormat dataFormat,
					   const AMint32 dataStride,
					   AMint32 x,
					   AMint32 y,
					   AMint32 width,
					   AMint32 height,
					   const void *_context) {

	AMint32 sx, sy, dx, dy;

	AM_ASSERT(image && image->pixels);
	AM_ASSERT(width > 0 && height > 0);
	AM_ASSERT((AMint32)dataFormat != (AMint32)VG_IMAGE_FORMAT_INVALID && (AMint32)image->format != (AMint32)VG_IMAGE_FORMAT_INVALID);

	// clip specified bound
	dx = x;
	dy = y;
	sx = 0;
	sy = 0;
	if (dx < 0) {
		width += dx;
		if (width <= 0)
			return;
		sx -= dx;
		dx = 0;
	}
	if (dy < 0) {
		height += dy;
		if (height <= 0)
			return;
		sy -= dy;
		dy = 0;
	}
	if (dx + width > image->width) {
		width = image->width - dx;
		if (width <= 0)
			return;
	}
	if (dy + height > image->height) {
		height = image->height - dy;
		if (height <= 0)
			return;
	}

	// take into account child images
	dx += image->x;
	dy += image->y;
	amPxlMapConvert(image->pixels, image->format, image->dataStride, dx, dy, data, dataFormat, dataStride, sx, sy, width, height, AM_FALSE, AM_FALSE);

#if defined(AM_GLE) || defined(AM_GLS)
	// invalidate image textures
	amImageTexturesInvalidate(image, _context);
#else
	(void)_context;
#endif
}

/*!
	\brief It reads pixel values from a rectangular portion of an image, performs format conversion if
	necessary, and stores the resulting pixels into memory.
	\param data pointer to the destination pixels.
	\param dataFormat data format of destination pixels. 
	\param dataStride data stride of destination pixels.
	\param dx x-coordinate of the rectangular region to write.
	\param dy y-coordinate of the rectangular region to write.
	\param image source image.
	\param sx x-coordinate of the rectangular region to read.
	\param sy y-coordinate of the rectangular region to read.
	\param width width of the rectangular region to read, in pixels.
	\param height height of the rectangular region to read, in pixels.
	\pre width, height > 0.
	\pre dx, dy >= 0.
*/
void amImageSubDataGet(void *data,
					   const VGImageFormat dataFormat,
					   const AMint32 dataStride,
					   AMint32 dx,
					   AMint32 dy,
					   const AMImage *image,
					   AMint32 sx,
					   AMint32 sy,
					   AMint32 width,
					   AMint32 height) {

	AM_ASSERT(image && image->pixels);
	AM_ASSERT(width > 0 && height > 0);
	AM_ASSERT((AMint32)dataFormat != (AMint32)VG_IMAGE_FORMAT_INVALID && (AMint32)image->format != (AMint32)VG_IMAGE_FORMAT_INVALID);
	AM_ASSERT(dx >= 0 && dy >= 0);

	// clip specified bound
	if (sx < 0) {
		width += sx;
		if (width <= 0)
			return;
		dx -= sx;
		sx = 0;
	}
	if (sy < 0) {
		height += sy;
		if (height <= 0)
			return;
		dy -= sy;
		sy = 0;
	}
	if (sx + width > image->width) {
		width = image->width - sx;
		if (width <= 0)
			return;
	}
	if (sy + height > image->height) {
		height = image->height - sy;
		if (height <= 0)
			return;
	}
	// take into account child images
	sx += image->x;
	sy += image->y;

	// keep the extract converter, according to source and destination formats
	amPxlMapConvert(data, dataFormat, dataStride, dx, dy, image->pixels, image->format, image->dataStride, sx, sy, width, height, AM_FALSE, AM_TRUE);
}

/*!
	\brief Copy a rectangular region from a source image to a destination image. Pixels whose source or
	destination lie outside of the bounds of the respective image are ignored. Pixel format conversion is applied as needed.
	\param dst destination image.
	\param dx x-coordinate of the rectangular region to write.
	\param dy y-coordinate of the rectangular region to write.
	\param src source image.
	\param sx x-coordinate of the rectangular region to read.
	\param sy y-coordinate of the rectangular region to read.
	\param width width of the rectangular region to copy, in pixels.
	\param height height of the rectangular region to copy, in pixels.
	\param dither AM_TRUE to use dithering in the conversion process, else AM_FALSE.
	\param _context pointer to a AMContext structure, containing the list of all created handles.
	\return VG_OUT_OF_MEMORY_ERROR if a memory allocation error occurred, else VG_NO_ERROR.
	\pre width, height > 0.
*/
AMbool amImageCopy(AMImage *dst,
				   AMint32 dx,
				   AMint32 dy,
				   const AMImage *src,
				   AMint32 sx,
				   AMint32 sy,
				   AMint32 width,
				   AMint32 height,
				   const AMbool dither,
				   void *_context) {

	AMbool res;
	AMContext *context = (AMContext *)_context;

	AM_ASSERT(src && src->pixels);
	AM_ASSERT(dst && dst->pixels);
	AM_ASSERT(width > 0 && height > 0);
	AM_ASSERT((AMint32)dst->format != (AMint32)VG_IMAGE_FORMAT_INVALID && (AMint32)dst->format != (AMint32)VG_IMAGE_FORMAT_INVALID);
	AM_ASSERT(context);

	// clip specified bound
	if (sx < 0) {
		width += sx;
		if (width <= 0)
			return AM_TRUE;
		dx -= sx;
		sx = 0;
	}
	if (sy < 0) {
		height += sy;
		if (height <= 0)
			return AM_TRUE;
		dy -= sy;
		sy = 0;
	}
	if (sx + width > src->width) {
		width = src->width - sx;
		if (width <= 0)
			return AM_TRUE;
	}
	if (sy + height > src->height) {
		height = src->height - sy;
		if (height <= 0)
			return AM_TRUE;
	}

	if (dx < 0) {
		width += dx;
		if (width <= 0)
			return AM_TRUE;
		sx -= dx;
		dx = 0;
	}
	if (dy < 0) {
		height += dy;
		if (height <= 0)
			return AM_TRUE;
		sy -= dy;
		dy = 0;
	}
	if (dx + width > dst->width) {
		width = dst->width - dx;
		if (width <= 0)
			return AM_TRUE;
	}
	if (dy + height > dst->height) {
		height = dst->height - dy;
		if (height <= 0)
			return AM_TRUE;
	}

	AM_ASSERT(width > 0 && height > 0);
	AM_ASSERT(sx >= 0 && sx < src->width);
	AM_ASSERT(sy >= 0 && sy < src->height);
	AM_ASSERT(dx >= 0 && dx < dst->width);
	AM_ASSERT(dy >= 0 && dy < dst->height);

	// take into account child images
	sx += src->x;
	sy += src->y;
	dx += dst->x;
	dy += dst->y;

	// if images overlap we have to pass through a temporary image
	if (amImagesOverlap(src, dst)) {

		AMImage tmpImage;

		// create and initialize temporary image
		res = amImageInit(&tmpImage, dst->format, dst->allowedQuality, 0, 0, width, height, 0, context);
		if (res) {
			// first convert from src to tmpImage
			amPxlMapConvert(tmpImage.pixels, dst->format, tmpImage.dataStride, 0, 0, src->pixels, src->format, src->dataStride, sx, sy, width, height, dither, AM_TRUE);
			// now convert from tmpImage to dst
			amPxlMapConvert(dst->pixels, dst->format, dst->dataStride, dx, dy, tmpImage.pixels, tmpImage.format, tmpImage.dataStride, 0, 0, width, height, AM_FALSE, AM_TRUE);
		// destroy temporary image
		amImageDestroy(&tmpImage, context);
	}
	}
	else {
		amPxlMapConvert(dst->pixels, dst->format, dst->dataStride, dx, dy, src->pixels, src->format, src->dataStride, sx, sy, width, height, dither, AM_TRUE);
		res = AM_TRUE;
	}

#if defined(AM_GLE) || defined(AM_GLS)
	if (res) {
		// invalidate image textures
		amImageTexturesInvalidate(dst, context);
	}
#endif
	return res;
}

// *********************************************************************
//                        Public implementations
// *********************************************************************

/*!
	\brief It creates an image with the given width, height, and pixel format and returns a VGImage handle to it.
	All color and alpha channel values are initially set to zero.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param format image format.
	\param width width of the image, in pixels.
	\param height height of the image, in pixels.
	\param allowedQuality bitwise OR of values from the VGImageQuality enumeration, indicating which
	levels of resampling quality may be used to draw the image.
	\return a VGImage handle, or VG_INVALID_HANDLE if an error occurs.
*/
VG_API_CALL VGImage VG_API_ENTRY vgCreateImage(VGImageFormat format,
                                               VGint width,
                                               VGint height,
                                               VGbitfield allowedQuality) VG_API_EXIT {

	AMImage *img;
	VGImage handle;
	AMint32 wh = width * height;
	AMint32 bytesPerLine;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgCreateImage");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}

	// check for unsupported formats
	if (!amImageFormatValid(format)) {
		amCtxErrorSet(currentContext, VG_UNSUPPORTED_IMAGE_FORMAT_ERROR);
		AM_MEMORY_LOG("vgCreateImage");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}
	// check for illegal arguments
	if (width <= 0 || height <= 0 ||
		width > currentContext->maxImageWidth || height > currentContext->maxImageHeight) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgCreateImage");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}

	bytesPerLine = amBytesPerLine(format, width);
	if (wh > currentContext->maxImagePixels || width * bytesPerLine > currentContext->maxImageBytes) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgCreateImage");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}
	if (allowedQuality == 0 || allowedQuality > (VG_IMAGE_QUALITY_NONANTIALIASED | VG_IMAGE_QUALITY_FASTER | VG_IMAGE_QUALITY_BETTER)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgCreateImage");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}

	// allocate the image
	img = (AMImage *)amMalloc(sizeof(AMImage));
	if (!img) {
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgCreateImage");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}
	// initialize image
	if (!amImageInit(img, format, allowedQuality, 0, 0, width, height, 0, currentContext)) {
		AM_MEMORY_LOG("vgCreateImage (amImageInit fail, now try to recover memory)");
		// try to retrieve as much memory as possbile
		amMemMngRetrieve(currentContext, AM_TRUE);
		// try to re-initialize the image
		if (!amImageInit(img, format, allowedQuality, 0, 0, width, height, 0, currentContext)) {
		amFree(img);
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgCreateImage");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}
	}
	// add the new path to the internal handles list of the context
	handle = amCtxHandleNew(currentContext, (AMhandle)img);
	if (handle == VG_INVALID_HANDLE) {
		amImageDestroy(img, currentContext);
		amFree(img);
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgCreateImage");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}
	// decrement the counter of the memory manager
	amCtxMemMngCountDown(currentContext);
	// exit without errors
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgCreateImage");
	OPENVG_RETURN(handle)
}

/*!
	\brief The resources associated with an image may be deallocated by calling	vgDestroyImage.
	Following the call, the image handle is no longer valid in any context that shared it.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param image the image to destroy.
*/
VG_API_CALL void VG_API_ENTRY vgDestroyImage(VGImage image) VG_API_EXIT {

	AMImage *img;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgDestroyImage");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for bad handle
	if (amCtxHandleValid(currentContext, image) != AM_IMAGE_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgDestroyImage");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	img = (AMImage *)currentContext->handles->createdHandlesList.data[image];
	AM_ASSERT(img);

	// invalidate image handle and decrement reference counters
	amImageDestroy(img, currentContext);

	// if resources has been totally released, we can remove image pointer from the internal list of current context
	if (img->referenceCounter == 0) {
		amCtxHandleRemove(currentContext, image);
		amFree(img);
	}
	// decrement the counter of the memory manager
	amCtxMemMngCountDown(currentContext);
	// exit without errors
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgDestroyImage");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief Fill a given rectangle of an image with the with the color specified by the VG_CLEAR_COLOR parameter.
	The rectangle is clipped to the bounds of the image.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param image image to clear.
	\param x x-coordinate of the first pixel to clear.
	\param y y-coordinate of the first pixel to clear.
	\param width width of the area to clear.
	\param height height of the area to clear.
*/
VG_API_CALL void VG_API_ENTRY vgClearImage(VGImage image,
                                           VGint x,
                                           VGint y,
                                           VGint width,
                                           VGint height) VG_API_EXIT {

	AMImage *img;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgClearImage");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for bad handle
	if (amCtxHandleValid(currentContext, image) != AM_IMAGE_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgClearImage");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for illegal arguments
	if (width <= 0 || height <= 0) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgClearImage");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	img = (AMImage *)currentContext->handles->createdHandlesList.data[image];
	AM_ASSERT(img && img->type == AM_IMAGE_HANDLE_ID);

#if !defined(AM_STANDALONE) || defined(RIM_VG_SRC) || defined(TORCH_VG_SRC)
	if (img->inUseByEgl) {
		amCtxErrorSet(currentContext, VG_IMAGE_IN_USE_ERROR);
		AM_MEMORY_LOG("vgClearImage");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
#endif

	amImageClear(img, x, y, width, height, currentContext->clearColor, currentContext);
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgClearImage");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief It reads pixel values from memory, performs format conversion if necessary, and stores the
	resulting pixels into a rectangular portion of an image.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param image destination image.
	\param data pointer to source pixels.
	\param dataStride data stride of source pixels.
	\param dataFormat data format of source pixels.
	\param x x-coordinate of the rectangular region to write.
	\param y y-coordinate of the rectangular region to write.
	\param width width of the rectangular region to write, in pixels.
	\param height height of the rectangular region to write, in pixels.
*/
VG_API_CALL void VG_API_ENTRY vgImageSubData(VGImage image,
                                             const void *data,
                                             VGint dataStride,
                                             VGImageFormat dataFormat,
                                             VGint x,
                                             VGint y,
                                             VGint width,
                                             VGint height) VG_API_EXIT {

	AMImage *img;
	AMint32 bytesPerPixel = amImageBytesPerPixel(dataFormat);
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgImageSubData");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for bad handle
	if (amCtxHandleValid(currentContext, image) != AM_IMAGE_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgImageSubData");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for illegal arguments
	if (width <= 0 || height <= 0 || !data) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgImageSubData");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if (bytesPerPixel > 1 && !amPointerIsAligned(data, bytesPerPixel)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgImageSubData");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for unsupported data format
	if (!amImageFormatValid(dataFormat)) {
		amCtxErrorSet(currentContext, VG_UNSUPPORTED_IMAGE_FORMAT_ERROR);
		AM_MEMORY_LOG("vgImageSubData");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	img = (AMImage *)currentContext->handles->createdHandlesList.data[image];
	AM_ASSERT(img);

#if !defined(AM_STANDALONE) || defined(RIM_VG_SRC) || defined(TORCH_VG_SRC)
	if (img->inUseByEgl) {
		amCtxErrorSet(currentContext, VG_IMAGE_IN_USE_ERROR);
		AM_MEMORY_LOG("vgImageSubData");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
#endif

	amImageSubDataSet(img, data, dataFormat, dataStride, x, y, width, height, currentContext);
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgImageSubData");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief It reads pixel values from a rectangular portion of an image, performs format conversion if
	necessary, and stores the resulting pixels into memory.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param image source image.
	\param data pointer to the destination pixels.
	\param dataStride data stride of destination pixels.
	\param dataFormat data format of destination pixels. 
	\param x x-coordinate of the rectangular region to read.
	\param y y-coordinate of the rectangular region to read.
	\param width width of the rectangular region to read, in pixels.
	\param height height of the rectangular region to read, in pixels.
*/
VG_API_CALL void VG_API_ENTRY vgGetImageSubData(VGImage image,
                                                void *data,
                                                VGint dataStride,
                                                VGImageFormat dataFormat,
                                                VGint x,
                                                VGint y,
                                                VGint width,
                                                VGint height) VG_API_EXIT {

	const AMImage *img;
	AMint32 bytesPerPixel = amImageBytesPerPixel(dataFormat);
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgGetImageSubData");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for bad handle
	if (amCtxHandleValid(currentContext, image) != AM_IMAGE_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgGetImageSubData");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for illegal arguments
	if (width <= 0 || height <= 0 || !data) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgGetImageSubData");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if (bytesPerPixel > 1 && !amPointerIsAligned(data, bytesPerPixel)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgGetImageSubData");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for unsupported data format
	if (!amImageFormatValid(dataFormat)) {
		amCtxErrorSet(currentContext, VG_UNSUPPORTED_IMAGE_FORMAT_ERROR);
		AM_MEMORY_LOG("vgGetImageSubData");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	img = (const AMImage *)currentContext->handles->createdHandlesList.data[image];
	AM_ASSERT(img);
#if !defined(AM_STANDALONE) || defined(RIM_VG_SRC) || defined(TORCH_VG_SRC)
	if (img->inUseByEgl) {
		amCtxErrorSet(currentContext, VG_IMAGE_IN_USE_ERROR);
		AM_MEMORY_LOG("vgGetImageSubData");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
#endif

	amImageSubDataGet(data, dataFormat, dataStride, 0, 0, img, x, y, width, height);
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgGetImageSubData");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief It returns a new VGImage handle that refers to a portion of the parent image. The region is
	given by the intersection of the bounds of the parent image with the specified rectangle, which must
	define a positive region contained entirely within parent.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param parent parent image.
	\param x x-coordinate (respect to the parent) of the first pixel.
	\param y y-coordinate (respect to the parent) of the first pixel.
	\param width width of the child image, in pixels.
	\param height height of the child image, in pixels.
	\return new VGImage handle, or VG_INVALID_HANDLE if an error occurs.
*/
VG_API_CALL VGImage VG_API_ENTRY vgChildImage(VGImage parent,
                                              VGint x,
                                              VGint y,
                                              VGint width,
                                              VGint height) VG_API_EXIT {

	AMImage *img, *parentImg;
	VGImage handle;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgChildImage");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}

	// check for bad handle
	if (amCtxHandleValid(currentContext, parent) != AM_IMAGE_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgChildImage");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}

	parentImg = (AMImage *)currentContext->handles->createdHandlesList.data[parent];
	AM_ASSERT(parentImg);
#if !defined(AM_STANDALONE) || defined(RIM_VG_SRC) || defined(TORCH_VG_SRC)
	if (parentImg->inUseByEgl) {
		amCtxErrorSet(currentContext, VG_IMAGE_IN_USE_ERROR);
		AM_MEMORY_LOG("vgChildImage");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}
#endif

	// check for illegal arguments
	if (x < 0 || x >= parentImg->width || y < 0 || y >= parentImg->height || width <= 0 || height <= 0) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgChildImage");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}
	if (x + width > parentImg->width || y + height > parentImg->height) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgChildImage");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}

	// allocate the image
	img = (AMImage *)amMalloc(sizeof(AMImage));
	if (!img) {
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgChildImage");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}
	// initialize image
	if (!amImageInit(img, parentImg->format, parentImg->allowedQuality, x, y, width, height, parent, currentContext)) {
		AM_MEMORY_LOG("vgChildImage (amImageInit fail, now try to recover memory)");
		// try to retrieve as much memory as possbile
		amMemMngRetrieve(currentContext, AM_TRUE);
		// try to re-initialize the image
		if (!amImageInit(img, parentImg->format, parentImg->allowedQuality, x, y, width, height, parent, currentContext)) {
		amFree(img);
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgChildImage");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}
	}

	// add the new path to the internal handles list of the context
	handle = amCtxHandleNew(currentContext, (AMhandle)img);
	if (handle == VG_INVALID_HANDLE) {
		amImageDestroy(img, currentContext);
		amFree(img);
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgChildImage");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}
	// decrement the counter of the memory manager
	amCtxMemMngCountDown(currentContext);
	// exit without errors
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgChildImage");
	OPENVG_RETURN(handle)
}

/*!
	\brief It returns the closest valid ancestor (i.e., one that has not been the target of a vgDestroyImage call)
	of the given image. If image has no	ancestors, image itself is returned.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param image the image whose parent is to get.
	\return the closest valid ancestor VGImage handle.
*/
VG_API_CALL VGImage VG_API_ENTRY vgGetParent(VGImage image) VG_API_EXIT {

	AMImage *img;
	VGImage res;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgGetParent");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}

	// check for bad handle
	if (amCtxHandleValid(currentContext, image) != AM_IMAGE_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgGetParent");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}

	img = (AMImage *)currentContext->handles->createdHandlesList.data[image];
	AM_ASSERT(img);
#if !defined(AM_STANDALONE) || defined(RIM_VG_SRC) || defined(TORCH_VG_SRC)
	if (img->inUseByEgl) {
		amCtxErrorSet(currentContext, VG_IMAGE_IN_USE_ERROR);
		AM_MEMORY_LOG("vgGetParent");
		OPENVG_RETURN(VG_INVALID_HANDLE)
	}
#endif

	AM_MEMORY_LOG("vgGetParent");
	res = (img->parent) ? img->parent : image;
	OPENVG_RETURN(res)
}

/*!
	\brief Copy a rectangular region from a source image to a destination image. Pixels whose source or
	destination lie outside of the bounds of the respective image are ignored. Pixel format conversion
	is applied as needed.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param dst destination image.
	\param dx x-coordinate of the rectangular region to write.
	\param dy y-coordinate of the rectangular region to write.
	\param src source image.
	\param sx x-coordinate of the rectangular region to read.
	\param sy y-coordinate of the rectangular region to read.
	\param width width of the rectangular region to copy, in pixels.
	\param height height of the rectangular region to copy, in pixels.
	\param dither VG_TRUE to use dithering in the conversion process, else VG_FALSE.
*/
VG_API_CALL void VG_API_ENTRY vgCopyImage(VGImage dst,
                                          VGint dx,
                                          VGint dy,
                                          VGImage src,
                                          VGint sx,
                                          VGint sy,
                                          VGint width,
                                          VGint height,
                                          VGboolean dither) VG_API_EXIT {

	AMImage *dstImg;
	const AMImage *srcImg;
	AMbool dth = (dither == VG_TRUE) ? AM_TRUE : AM_FALSE;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgCopyImage");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for bad handles
	if (amCtxHandleValid(currentContext, dst) != AM_IMAGE_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgCopyImage");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	if (amCtxHandleValid(currentContext, src) != AM_IMAGE_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgCopyImage");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for illegal arguments
	if (width <= 0 || height <= 0) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgCopyImage");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	dstImg = (AMImage *)currentContext->handles->createdHandlesList.data[dst];
	srcImg = (const AMImage *)currentContext->handles->createdHandlesList.data[src];
	AM_ASSERT(dstImg);
	AM_ASSERT(srcImg);
#if !defined(AM_STANDALONE) || defined(RIM_VG_SRC) || defined(TORCH_VG_SRC)
	if (dstImg->inUseByEgl && srcImg->inUseByEgl) {
		amCtxErrorSet(currentContext, VG_IMAGE_IN_USE_ERROR);
		AM_MEMORY_LOG("vgCopyImage");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
#endif

	if (!amImageCopy(dstImg, dx, dy, srcImg, sx, sy, width, height, dth, currentContext)) {
		AM_MEMORY_LOG("vgCopyImage (amImageCopy fail, now try to recover memory)");
		// try to retrieve as much memory as possbile
		amMemMngRetrieve(currentContext, AM_TRUE);
		// try to re-copy the image
		if (!amImageCopy(dstImg, dx, dy, srcImg, sx, sy, width, height, dth, currentContext)) {
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgCopyImage");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	}

	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgCopyImage");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief It copies pixel data from the source image onto the drawing surface. Pixels whose source lies outside
	of the bounds of source image or whose destination lies outside the bounds of the drawing
	surface are ignored. Pixel format conversion is applied as needed. Scissoring takes place normally.
	Transformations, masking, and blending are not applied.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param dx x-coordinate of the rectangular region to write on the drawing surface.
	\param dy y-coordinate of the rectangular region to write on the drawing surface.
	\param src source image.
	\param sx x-coordinate of the rectangular region to read.
	\param sy y-coordinate of the rectangular region to read.
	\param width width of the rectangular region to set, in pixels.
	\param height height of the rectangular region to set, in pixels.
*/
VG_API_CALL void VG_API_ENTRY vgSetPixels(VGint dx,
                                          VGint dy,
                                          VGImage src,
                                          VGint sx,
                                          VGint sy,
                                          VGint width,
                                          VGint height) VG_API_EXIT {

	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgSetPixels");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for bad handle
	if (amCtxHandleValid(currentContext, src) != AM_IMAGE_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgSetPixels");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for illegal arguments
	if (width <= 0 || height <= 0) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgSetPixels");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

#if !defined(AM_STANDALONE) || defined(RIM_VG_SRC) || defined(TORCH_VG_SRC)
{
	AMImage *img = (AMImage *)currentContext->handles->createdHandlesList.data[src];

	if (img->inUseByEgl) {
		amCtxErrorSet(currentContext, VG_IMAGE_IN_USE_ERROR);
		AM_MEMORY_LOG("vgSetPixels");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
}
#endif

	if (!amDrawingSurfacePixelsSet(currentContext, currentSurface, dx, dy, src, sx, sy, width, height)) {
		AM_MEMORY_LOG("vgSetPixels (amDrawingSurfacePixelsSet fail, now try to recover memory)");
		// try to retrieve as much memory as possbile
		amMemMngRetrieve(currentContext, AM_TRUE);
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgSetPixels");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgSetPixels");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief It copies pixel data onto the drawing surface, without the creation of a VGImage object.
	The pixel values to be drawn are taken from the data pointer at the time of the vgWritePixels
	call, so future changes to the data have no effect. Pixels whose destination coordinate lies outside the
	bounds of the drawing surface are ignored. Pixel format conversion is applied as needed. Scissoring takes
	place normally. Transformations, masking, and blending are not applied.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param data source pixels.
	\param dataStride data stride of the source pixels.
	\param dataFormat format of the source pixels.
	\param dx x-coordinate of the rectangular region to write on the drawing surface.
	\param dy y-coordinate of the rectangular region to write on the drawing surface.
	\param width width of the rectangular region to write, in pixels.
	\param height height of the rectangular region to write, in pixels.
*/
VG_API_CALL void VG_API_ENTRY vgWritePixels(const void *data,
                                            VGint dataStride,
                                            VGImageFormat dataFormat,
                                            VGint dx,
                                            VGint dy,
                                            VGint width,
                                            VGint height) VG_API_EXIT {

	AMContext *currentContext;
	AMDrawingSurface *currentSurface;
	AMint32 bytesPerPixel;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgWritePixels");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for illegal arguments
	if (!data || width <= 0 || height <= 0) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgWritePixels");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	bytesPerPixel = amImageBytesPerPixel(dataFormat);
	if (bytesPerPixel > 1 && !amPointerIsAligned(data, bytesPerPixel)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgWritePixels");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	// check for unsupported image formats
	if (!amImageFormatValid(dataFormat)) {
		amCtxErrorSet(currentContext, VG_UNSUPPORTED_IMAGE_FORMAT_ERROR);
		AM_MEMORY_LOG("vgWritePixels");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	if (!amDrawingSurfacePixelsWrite(currentContext, currentSurface, dx, dy, data, dataFormat, dataStride, width, height)) {
		AM_MEMORY_LOG("vgWritePixels (amDrawingSurfacePixelsWrite fail, now try to recover memory)");
		// try to retrieve as much memory as possbile
		amMemMngRetrieve(currentContext, AM_TRUE);
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgWritePixels");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgWritePixels");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief It retrieves pixel data from the drawing surface into the destination image. Pixels whose source
	lies outside of the bounds of the drawing surface or whose destination lies outside
	the bounds of destination image are ignored. Pixel format conversion is applied as needed. The
	scissoring region does not affect the reading of pixels.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param dst destination image.
	\param dx x-coordinate of the rectangular region to write on the destination image.
	\param dy y-coordinate of the rectangular region to write on the destination image.
	\param sx x-coordinate of the rectangular region to read from the drawing surface.
	\param sy y-coordinate of the rectangular region to read from the drawing surface.
	\param width width of the rectangular region to get, in pixels.
	\param height height of the rectangular region to get, in pixels.
*/
VG_API_CALL void VG_API_ENTRY vgGetPixels(VGImage dst,
                                          VGint dx,
                                          VGint dy,
                                          VGint sx,
                                          VGint sy,
                                          VGint width,
                                          VGint height) VG_API_EXIT {

	AMImage *dstImg;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgGetPixels");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for bad handle
	if (amCtxHandleValid(currentContext, dst) != AM_IMAGE_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgGetPixels");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for illegal arguments
	if (width <= 0 || height <= 0) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgGetPixels");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	dstImg = (AMImage *)currentContext->handles->createdHandlesList.data[dst];
	AM_ASSERT(dstImg);
#if !defined(AM_STANDALONE) || defined(RIM_VG_SRC) || defined(TORCH_VG_SRC)
	if (dstImg->inUseByEgl) {
		amCtxErrorSet(currentContext, VG_IMAGE_IN_USE_ERROR);
		AM_MEMORY_LOG("vgGetPixels");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
#endif

	if (!amDrawingSurfacePixelsGet(dstImg, dx, dy, currentContext, currentSurface, sx, sy, width, height)) {
		AM_MEMORY_LOG("vgGetPixels (amDrawingSurfacePixelsGet fail, now try to recover memory)");
		// try to retrieve as much memory as possbile
		amMemMngRetrieve(currentContext, AM_TRUE);
		// try to re-get pixels
		if (!amDrawingSurfacePixelsGet(dstImg, dx, dy, currentContext, currentSurface, sx, sy, width, height)) {
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgGetPixels");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	}

	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgGetPixels");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief It allows pixel data to be copied from the drawing surface without the creation of a VGImage object.
	Pixels whose source lies outside of the bounds of the drawing surface are ignored. Pixel format conversion
	is applied as needed. The scissoring region does not affect the reading of pixels.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param data destination pixels.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param dataStride data stride of destination pixels.
	\param dataFormat data format of destination pixels.
	\param sx x-coordinate of the rectangular region to read from the drawing surface.
	\param sy y-coordinate of the rectangular region to read from the drawing surface.
	\param width width of the rectangular region to read, in pixels.
	\param height height of the rectangular region to read, in pixels.
*/
VG_API_CALL void VG_API_ENTRY vgReadPixels(void *data,
                                           VGint dataStride,
                                           VGImageFormat dataFormat,
                                           VGint sx,
                                           VGint sy,
                                           VGint width,
                                           VGint height) VG_API_EXIT {

	AMContext *currentContext;
	AMDrawingSurface *currentSurface;
	AMint32 bytesPerPixel;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgReadPixels");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for illegal arguments
	if (!data || width <= 0 || height <= 0) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgReadPixels");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	bytesPerPixel = amImageBytesPerPixel(dataFormat);
	if (bytesPerPixel > 1 && !amPointerIsAligned(data, bytesPerPixel)) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgReadPixels");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	// check for unsupported image formats
	if (!amImageFormatValid(dataFormat)) {
		amCtxErrorSet(currentContext, VG_UNSUPPORTED_IMAGE_FORMAT_ERROR);
		AM_MEMORY_LOG("vgReadPixels");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	if (!amDrawingSurfacePixelsRead(data, dataFormat, dataStride, currentContext, currentSurface, sx, sy, width, height)) {
		AM_MEMORY_LOG("vgReadPixels (amDrawingSurfacePixelsRead fail, now try to recover memory)");
		// try to retrieve as much memory as possbile
		amMemMngRetrieve(currentContext, AM_TRUE);
		// try to re-read pixels
		if (!amDrawingSurfacePixelsRead(data, dataFormat, dataStride, currentContext, currentSurface, sx, sy, width, height)) {
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgReadPixels");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	}

	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgReadPixels");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief It copies pixels from one region of the drawing surface to another. Copies between overlapping
	regions are allowed and always produce consistent results identical to copying the entire source region
	to a scratch buffer followed by copying the scratch buffer into the destination region.
	Pixels whose source or destination lies outside of the bounds of the drawing surface are ignored.
	Transformations, masking, and blending are not applied. Scissoring is applied to the destination, but
	does not affect the reading of pixels.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param dx x-coordinate of the rectangular region to write onto the drawing surface.
	\param dy y-coordinate of the rectangular region to write onto the drawing surface.
	\param sx x-coordinate of the rectangular region to read from the drawing surface.
	\param sy y-coordinate of the rectangular region to read from the drawing surface.
	\param width width of the rectangular region to copy, in pixels.
	\param height height of the rectangular region to copy, in pixels.
*/
VG_API_CALL void VG_API_ENTRY vgCopyPixels(VGint dx,
                                           VGint dy,
                                           VGint sx,
                                           VGint sy,
                                           VGint width,
                                           VGint height) VG_API_EXIT {

	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgCopyPixels");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for illegal arguments
	if (width <= 0 || height <= 0) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgCopyPixels");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	if (!amDrawingSurfacePixelsCopy(currentContext, currentSurface, dx, dy, sx, sy, width, height)) {
		AM_MEMORY_LOG("vgCopyPixels (amDrawingSurfacePixelsCopy fail, now try to recover memory)");
		// try to retrieve as much memory as possbile
		amMemMngRetrieve(currentContext, AM_TRUE);
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgCopyPixels");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgCopyPixels");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

/*!
	\brief It draws the specified image to the current drawing surface. Interpolation is done in the color
	space of the image. Image color values are processed in premultiplied alpha format during interpolation.
	Color channel values greater than their corresponding alpha value are clamped to the alpha value.\n
	For more information, please refer to the official \b OpenVG \b specifications document.
	\param image image to draw.
*/
VG_API_CALL void VG_API_ENTRY vgDrawImage(VGImage image) VG_API_EXIT {

	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgDrawImage");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for bad handle
	if (amCtxHandleValid(currentContext, image) != AM_IMAGE_HANDLE_ID) {
		amCtxErrorSet(currentContext, VG_BAD_HANDLE_ERROR);
		AM_MEMORY_LOG("vgDrawImage");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

#if !defined(AM_STANDALONE) || defined(RIM_VG_SRC) || defined(TORCH_VG_SRC)
{
	AMImage *img = (AMImage *)currentContext->handles->createdHandlesList.data[image];

	if (img->inUseByEgl) {
		amCtxErrorSet(currentContext, VG_IMAGE_IN_USE_ERROR);
		AM_MEMORY_LOG("vgDrawImage");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
}
#endif

	if (!amImageDraw(currentContext, currentSurface, image)) {
		AM_MEMORY_LOG("vgDrawImage (amImageDraw fail, now try to recover memory)");
		// try to retrieve as much memory as possbile
		amMemMngRetrieve(currentContext, AM_TRUE);
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgDrawImage");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}
	// decrement the counter of the memory manager
	amCtxMemMngCountDown(currentContext);
	// exit without errors
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgDrawImage");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

#undef AM_SAMPLE32_GET
#undef AM_SAMPLE32_POW2_GET
#undef AM_SAMPLE16_GET
#undef AM_SAMPLE16_POW2_GET
#undef AM_SAMPLE8_GET
#undef AM_SAMPLE8_POW2_GET
#undef AM_SAMPLE1_GET
#undef AM_SAMPLE1_POW2_GET
#undef AM_COLOR_TRANSFORM32

#if defined (RIM_VG_SRC)
#pragma pop
#endif

