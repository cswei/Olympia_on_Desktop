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
	\file vgtexture.c
	\brief Texture functions, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "vgtexture.h"
#include "stdlib_abstraction.h"
#if defined(AM_GLE) || defined(AM_GLS)
	#include "gl_abstraction.h"
#endif

/*!
	\brief Initialize a given texture structure, clearing all fields to 0.
	\param texture texture to initialize.
*/
void amTextureInit(AMTexture *texture) {

	AM_ASSERT(texture);

	texture->width = 0;
	texture->height = 0;
	texture->pixels = NULL;
#if defined(AM_GLE) || defined(AM_GLS)
	texture->target = GL_TEXTURE_2D;
	texture->glHandle = 0;
	texture->subWidth = 0;
	texture->subHeight = 0;
#endif
}

/*!
	\brief Destroy the specified texture.
	\param texture texture to destroy.
*/
void amTextureDestroy(AMTexture *texture) {

	AM_ASSERT(texture);

	texture->width = 0;
	texture->height = 0;
	if (texture->pixels) {
		amFree(texture->pixels);
		texture->pixels = NULL;
	}
#if defined(AM_GLE) || defined(AM_GLS)
	if (texture->glHandle) {
		amGlDeleteTextures(1, &texture->glHandle);
		texture->glHandle = 0;
	}
	texture->subWidth = 0;
	texture->subHeight = 0;
#endif
}

/*!
	\brief Give the power of two value greater (or equal) to a specified value.
	\param value input value.
	\return the power of two value greater (or equal) to \a value.
*/
AMuint32 amPow2Get(const AMuint32 value) {

	AMuint32 v = 1;

	if (value >= (AMuint32)((AMuint32)1 << 31))
		return (AMuint32)((AMuint32)1 << 31);
	
	while (v < value)
		v <<= 1;

	return v;
}

/*!
	\brief Given a power of two value, it returns the corresponding shift such as (1 << amPow2Get(value)) = value.
	\param pow2Value input value.
	\return the corresponding shift.
	\pre pow2Value must be a power of two.
*/
AMuint32 amPow2Shift(const AMuint32 pow2Value) {

	AMuint32 v = pow2Value;
	AMuint32 res = 0;

	AM_ASSERT(amPow2Check(pow2Value));

	while (res < 32 && (v & 1) == 0) {
		res++;
		v >>= 1;
	}
	return res;
}

#if defined(AM_GLE) || defined(AM_GLS)
// Map an image point (x, y) into the corresponding texture coordinate uv.
void amTextureUVMap(AMfloat *u,
					AMfloat *v,
					const AMTexture *texture,
					const AMuint32 imageWidth,
					const AMuint32 imageHeight,
					const AMfloat x,
					const AMfloat y) {

	AM_ASSERT(u);
	AM_ASSERT(v);
	AM_ASSERT(texture);
	AM_ASSERT(texture->glHandle > 0);
	AM_ASSERT(texture->width > 0 && texture->height > 0);
	AM_ASSERT(texture->subWidth > 0 && texture->subHeight > 0);
	AM_ASSERT(texture->width >= texture->subWidth && texture->height >= texture->subHeight);
	AM_ASSERT(imageWidth > 0 && imageHeight > 0);
	AM_ASSERT(x >= 0.0f && x <= (AMfloat)imageWidth);
	AM_ASSERT(y >= 0.0f && y <= (AMfloat)imageHeight);

	if (texture->target == GL_TEXTURE_2D) {
		// in this case texture was not resized in x direction, so sx is already good
		if (imageWidth == texture->subWidth)
			*u = x / (AMfloat)texture->width;
		// in this case texture was resized in x direction, so sx must be rescaled to match current texture width
		else {
			AM_ASSERT(imageWidth > texture->subWidth);
			*u = x / (AMfloat)imageWidth;
			*u = ((*u) * (AMfloat)texture->subWidth) / (AMfloat)texture->width;
		}
		// in this case texture was not resized in y direction, so sy is already good
		if (imageHeight == texture->subHeight)
			*v = y / (AMfloat)texture->height;
		// in this case texture was resized in y direction, so sy must be rescaled to match current texture height
		else {
			AM_ASSERT(imageHeight > texture->subHeight);
			*v = y / (AMfloat)imageHeight;
			*v = ((*v) * (AMfloat)texture->subHeight) / (AMfloat)texture->height;
		}
	}
	else {
		// in this case texture was not resized in x direction, so sx is already good
		if (imageWidth == texture->subWidth)
			*u = x;
		// in this case texture was resized in x direction, so sx must be rescaled to match current texture width
		else {
			AM_ASSERT(imageWidth > texture->subWidth);
			*u = x / (AMfloat)imageWidth;
			*u = ((*u) * (AMfloat)texture->subWidth);
		}
		// in this case texture was not resized in y direction, so sy is already good
		if (imageHeight == texture->subHeight)
			*v = y;
		// in this case texture was resized in y direction, so sy must be rescaled to match current texture height
		else {
			AM_ASSERT(imageHeight > texture->subHeight);
			*v = y / (AMfloat)imageHeight;
			*v = ((*v) * (AMfloat)texture->subHeight);
		}
	}
}
#endif

#if defined (RIM_VG_SRC)
#pragma pop
#endif

