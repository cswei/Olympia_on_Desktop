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

#ifndef _VGFONT_H
#define _VGFONT_H

/*!
	\file vgfont.h
	\brief OpenVG fonts, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "vgcontext.h"

#if (AM_OPENVG_VERSION >= 110)

// *********************************************************************
//                               AMFont
// *********************************************************************

#define AM_GLYPH_IS_PATH	1
#define AM_GLYPH_IS_HINTED	2

//! Structure used to store glyph information.
typedef struct _AMGlyph {
	//! Glyph handle, it could be a VGPath or a VGImage.
	VGHandle handle;
	//! Glyph origin coordinates.
	AMVect2f origin;
	//! Glyph escapement coordinates.
	AMVect2f escapement;
	//! Glyph flags (path/image | hinted/unhinted).
	AMuint32 flags;
} AMGlyph, *AMGlyphPtr;

//! Structure used to retrieve a glyph (a AMGlyph structure) from glyph memory pools.
typedef struct _AMGlyphDesc {
	//! Glyph index.
	AMuint32 index;
	//! Index in a pool array.
	AMuint16 pool;
	//! Position inside the pool.
	AMuint16 poolIdx;
} AMGlyphDesc;

AM_DYNARRAY_DECLARE(AMGlyphDescDynArray, AMGlyphDesc, _AMGlyphDescDynArray)

/*!
	\brief A pool of font glyphs, used to store glyphs structures.
*/
typedef struct _AMGlyphPool {
	//! Glyphs pool.
	AMGlyph data[AM_FONT_GLYPH_POOL_CAPACITY];
	//! Number of glyphs currently stored in the pool.
	AMuint32 size;
} AMGlyphPool, *AMGlyphPoolPtr;

AM_DYNARRAY_DECLARE(AMGlyphPoolPtrDynArray, AMGlyphPoolPtr, _AMGlyphPoolPtrDynArray)


/*!
	\brief Font structure, used to implement OpenVG fonts.
*/
typedef struct _AMFont {
	//! VGHandle type identifier, it can be AM_FONT_HANDLE_ID or AM_INVALID_HANDLE_ID.
	AMuint16 id;
	//! It's always AM_FONT_HANDLE_ID, never changed.
	AMuint16 type;
	//! VG handle (index inside the context createdHandlesList).
	VGHandle vgHandle;
	//! Reference counter.
	AMuint32 referenceCounter;
	//! List of created glyphs descriptors.
	AMGlyphDescDynArray createdGlyphsDesc;
	//! List of available glyph descriptors, to be reused.
	AMGlyphDescDynArray availableGlyphDesc;
	//! Font glyph pools
	AMGlyphPoolPtrDynArray glyphPools;
} AMFont;

// Initialize a given font structure.
AMbool amFontInit(AMFont *font,
				  const AMint32 glyphCapacityHint);
// Destroy font resources.
void amFontResourcesDestroy(AMFont *font,
							AMContext *context);
// Destroy the specified font.
void amFontDestroy(AMFont *font,
				   AMContext *context);
// Get the number of created glyphs by the specified font.
AMint32 amFontGlyphsCount(const AMFont *font);

#endif
#endif
