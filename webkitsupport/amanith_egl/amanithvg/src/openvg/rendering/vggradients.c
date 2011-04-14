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
	\file vggradients.c
	\brief Gradients utilities, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#if defined(AM_GLE) || defined(AM_GLS)
	#include "glgradients.h"
#endif
#include "vggradients.h"
#include "vgconversions.h"

/*!
	\brief Calculate gradient color stop tangents, using Catmull-Rom schema.
	\param inTangents output array containing incoming tangents.
	\param outTangents output array containing outcoming tangents.
	\param keys input color stops.
*/
void amGradientsTangentsGenerate(AMColorStopDynArray *inTangents,
								 AMColorStopDynArray *outTangents,
								 const AMColorStopDynArray *keys) {

	AMColorStop key;
	AMfloat csi, cso;
	AMint32 index0, index1, i, j, i0, i1;
	AMfloat v1[4], v2[4];

	AM_ASSERT(inTangents);
	AM_ASSERT(outTangents);
	AM_ASSERT(keys);
	AM_ASSERT(keys->size >= 2);
	AM_ASSERT(inTangents->size == 0);
	AM_ASSERT(outTangents->size == 0);
	AM_ASSERT(inTangents->capacity == keys->size);
	AM_ASSERT(outTangents->capacity == keys->size);

	index0 = 0;
	index1 = (AMint32)keys->size - 1;

	// 2 keys case
	if (keys->size == 2) {
		
		key.position = keys->data[index0].position;
		key.color[AM_R] = 0.5f * (keys->data[index1].color[AM_R] - keys->data[index0].color[AM_R]);
		key.color[AM_G] = 0.5f * (keys->data[index1].color[AM_G] - keys->data[index0].color[AM_G]);
		key.color[AM_B] = 0.5f * (keys->data[index1].color[AM_B] - keys->data[index0].color[AM_B]);
		key.color[AM_A] = 0.5f * (keys->data[index1].color[AM_A] - keys->data[index0].color[AM_A]);
		inTangents->data[index0] = key;
		outTangents->data[index0] = key;

		key.position = keys->data[index0 + 1].position;
		inTangents->data[index1] = key;
		outTangents->data[index1] = key;

		inTangents->size = inTangents->capacity;
		outTangents->size = outTangents->capacity;
		return;
	}

	// full Catmull-Rom schema (3 or more keys)
	j = (AMint32)keys->size;

	// first tangent
	csi = keys->data[2].position - keys->data[0].position;
	cso = keys->data[1].position - keys->data[0].position;
	v1[AM_R] = (-cso / (2.0f * csi)) * (keys->data[2].color[AM_R] - keys->data[0].color[AM_R]);
	v1[AM_G] = (-cso / (2.0f * csi)) * (keys->data[2].color[AM_G] - keys->data[0].color[AM_G]);
	v1[AM_B] = (-cso / (2.0f * csi)) * (keys->data[2].color[AM_B] - keys->data[0].color[AM_B]);
	v1[AM_A] = (-cso / (2.0f * csi)) * (keys->data[2].color[AM_A] - keys->data[0].color[AM_A]);
	v2[AM_R] = (1.5f) * (keys->data[1].color[AM_R] - keys->data[0].color[AM_R]);
	v2[AM_G] = (1.5f) * (keys->data[1].color[AM_G] - keys->data[0].color[AM_G]);
	v2[AM_B] = (1.5f) * (keys->data[1].color[AM_B] - keys->data[0].color[AM_B]);
	v2[AM_A] = (1.5f) * (keys->data[1].color[AM_A] - keys->data[0].color[AM_A]);
	key.position = keys->data[index0].position;
	key.color[AM_R] = v1[AM_R] + v2[AM_R];
	key.color[AM_G] = v1[AM_G] + v2[AM_G];
	key.color[AM_B] = v1[AM_B] + v2[AM_B];
	key.color[AM_A] = v1[AM_A] + v2[AM_A];
	inTangents->data[index0] = key;
	outTangents->data[index0] = key;
	i0 = index0 + 1;
	
	// last tangent
	csi = keys->data[j - 1].position - keys->data[j - 3].position;
	cso = keys->data[j - 1].position - keys->data[j - 2].position;

	v1[AM_R] = (-cso / (2.0f * csi)) * (keys->data[j - 1].color[AM_R] - keys->data[j - 3].color[AM_R]);
	v1[AM_G] = (-cso / (2.0f * csi)) * (keys->data[j - 1].color[AM_G] - keys->data[j - 3].color[AM_G]);
	v1[AM_B] = (-cso / (2.0f * csi)) * (keys->data[j - 1].color[AM_B] - keys->data[j - 3].color[AM_B]);
	v1[AM_A] = (-cso / (2.0f * csi)) * (keys->data[j - 1].color[AM_A] - keys->data[j - 3].color[AM_A]);
	v2[AM_R] = (1.5f) * (keys->data[j - 1].color[AM_R] - keys->data[j - 2].color[AM_R]);
	v2[AM_G] = (1.5f) * (keys->data[j - 1].color[AM_G] - keys->data[j - 2].color[AM_G]);
	v2[AM_B] = (1.5f) * (keys->data[j - 1].color[AM_B] - keys->data[j - 2].color[AM_B]);
	v2[AM_A] = (1.5f) * (keys->data[j - 1].color[AM_A] - keys->data[j - 2].color[AM_A]);
	key.color[AM_R] = v1[AM_R] + v2[AM_R];
	key.color[AM_G] = v1[AM_G] + v2[AM_G];
	key.color[AM_B] = v1[AM_B] + v2[AM_B];
	key.color[AM_A] = v1[AM_A] + v2[AM_A];
	key.position = keys->data[index1].position;
	inTangents->data[index1] = key;
	outTangents->data[index1] = key;
	i1 = index1 - 1;

	for (i = i0; i <= i1; i++) {
		cso = (keys->data[i + 1].position - keys->data[i].position) / (keys->data[i + 1].position - keys->data[i - 1].position);
		csi = (keys->data[i].position - keys->data[i - 1].position) / (keys->data[i + 1].position - keys->data[i - 1].position);

		key.position = keys->data[i].position;
		key.color[AM_R] = cso * (keys->data[i + 1].color[AM_R] - keys->data[i - 1].color[AM_R]);
		key.color[AM_G] = cso * (keys->data[i + 1].color[AM_G] - keys->data[i - 1].color[AM_G]);
		key.color[AM_B] = cso * (keys->data[i + 1].color[AM_B] - keys->data[i - 1].color[AM_B]);
		key.color[AM_A] = cso * (keys->data[i + 1].color[AM_A] - keys->data[i - 1].color[AM_A]);
		outTangents->data[i] = key;

		key.color[AM_R] = csi * (keys->data[i + 1].color[AM_R] - keys->data[i - 1].color[AM_R]);
		key.color[AM_G] = csi * (keys->data[i + 1].color[AM_G] - keys->data[i - 1].color[AM_G]);
		key.color[AM_B] = csi * (keys->data[i + 1].color[AM_B] - keys->data[i - 1].color[AM_B]);
		key.color[AM_A] = csi * (keys->data[i + 1].color[AM_A] - keys->data[i - 1].color[AM_A]);
		inTangents->data[i] = key;
	}

	// if first and last key values (colors) are equal, set equal tangents
	if ((keys->data[index0].color[AM_R] == keys->data[index1].color[AM_R]) &&
		(keys->data[index0].color[AM_G] == keys->data[index1].color[AM_G]) &&
		(keys->data[index0].color[AM_B] == keys->data[index1].color[AM_B]) &&
		(keys->data[index0].color[AM_A] == keys->data[index1].color[AM_A])) {

		key.position = keys->data[index0].position;
		key.color[AM_R] = (outTangents->data[index0].color[AM_R] + inTangents->data[index1].color[AM_R]) * 0.5f;
		key.color[AM_G] = (outTangents->data[index0].color[AM_G] + inTangents->data[index1].color[AM_G]) * 0.5f;
		key.color[AM_B] = (outTangents->data[index0].color[AM_B] + inTangents->data[index1].color[AM_B]) * 0.5f;
		key.color[AM_A] = (outTangents->data[index0].color[AM_A] + inTangents->data[index1].color[AM_A]) * 0.5f;
		inTangents->data[index0] = key;
		outTangents->data[index0] = key;

		key.position = keys->data[index1].position;
		inTangents->data[index1] = key;
		outTangents->data[index1] = key;
	}

	inTangents->size = inTangents->capacity;
	outTangents->size = outTangents->capacity;
}

/*!
	\brief generate a 1D pixel buffer to use for linear and radial gradients; pixels will be in the same format
	of the current drawing surface.
	\param pixels output (32bit) pixel buffer.
	\param width number of pixels to generate.
	\param sColStops input color stops, in the non-linear unpremultiplied color format.
	\param reflectKeys if AM_TRUE generate half width pixels according to color stops, and the remaining half
	width pixels reflecting color stops.
	\param linearInterpolation if AM_TRUE use linear interpolation between color stops, else use smooth (Hermite)
	color interpolation.
	\param colorRampPremultiplied if AM_TRUE color and alpha values at each gradient stop are multiplied
	together to	form premultiplied sRGBA values prior to interpolation; otherwise, color and
	alpha values are processed independently.
	\param srfFormatIdx drawing surface format index.
	\param colorTransformation an optional color transformation; NULL if none.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
	\pre at least two color stops must be present in the sColStops array.
*/
AMbool amGradientPixelsGenerate(AMuint32 *pixels,
							  const AMuint32 width,
							  const AMColorStopDynArray *sColStops,
							  const AMbool reflectKeys,
							  const AMbool linearInterpolation,
							  const VGboolean colorRampPremultiplied,
							  const AMuint32 srfFormatIdx,
							  const AMfloat *colorTransformation) {

	AMColorStopDynArray tmpKeys;
	AMColorStopDynArray inTangents, outTangents;
	AMColorStop key;
	AMint32 i, j, k0, k1;
	AMuint32 r, g, b, a;
	AMfloat domainLen, step, u, u1, t, colR, colG, colB, colA;
	AMuint32 surfaceLinearColorSpace = pxlFormatTable[srfFormatIdx][FMT_FLAGS] & FMT_L;
	AMuint32 rSh = pxlFormatTable[srfFormatIdx][FMT_R_SH];
	AMuint32 gSh = pxlFormatTable[srfFormatIdx][FMT_G_SH];
	AMuint32 bSh = pxlFormatTable[srfFormatIdx][FMT_B_SH];
	AMuint32 aSh = pxlFormatTable[srfFormatIdx][FMT_A_SH];

	#define PIXEL_PIPELINE() \
		if (colorTransformation) { \
			if (colorRampPremultiplied) { \
				if (colA != 0.0f) { \
					AMfloat invA = 1.0f / colA; \
					/* apply color transform in unpremultiplied format */ \
					colR = (colR * invA) * colorTransformation[0] + colorTransformation[4]; \
					colG = (colG * invA) * colorTransformation[1] + colorTransformation[5]; \
					colB = (colB * invA) * colorTransformation[2] + colorTransformation[6]; \
					colA = colA * colorTransformation[3] + colorTransformation[7]; \
					/* clamp result in the [0; 1] range */ \
					colR = AM_CLAMP(colR, 0.0f, 1.0f); \
					colG = AM_CLAMP(colG, 0.0f, 1.0f); \
					colB = AM_CLAMP(colB, 0.0f, 1.0f); \
					colA = AM_CLAMP(colA, 0.0f, 1.0f); \
					/* premultiply format again */ \
					colR *= colA; \
					colG *= colA; \
					colB *= colA; \
				} \
				else { \
					AM_ASSERT(colR == 0.0f); \
					AM_ASSERT(colG == 0.0f); \
					AM_ASSERT(colB == 0.0f); \
				} \
			} \
			else { \
				/* apply color transform */ \
				colR = colR * colorTransformation[0] + colorTransformation[4]; \
				colG = colG * colorTransformation[1] + colorTransformation[5]; \
				colB = colB * colorTransformation[2] + colorTransformation[6]; \
				colA = colA * colorTransformation[3] + colorTransformation[7]; \
				/* clamp result in the [0; 1] range */ \
				colR = AM_CLAMP(colR, 0.0f, 1.0f); \
				colG = AM_CLAMP(colG, 0.0f, 1.0f); \
				colB = AM_CLAMP(colB, 0.0f, 1.0f); \
				colA = AM_CLAMP(colA, 0.0f, 1.0f); \
			} \
		} \
		AM_ASSERT(colR >= 0.0f && colR <= 1.0f); \
		AM_ASSERT(colG >= 0.0f && colG <= 1.0f); \
		AM_ASSERT(colB >= 0.0f && colB <= 1.0f); \
		AM_ASSERT(colA >= 0.0f && colA <= 1.0f); \
		if (colorRampPremultiplied) { \
			/* in this case color are always in sRGBAPre, so for linear color space surfaces we have to transform sRGBAPre -> lRGBAPre */ \
			if (surfaceLinearColorSpace) { \
				if (colA != 0.0f) { \
					AMfloat invA = 1.0f / colA; \
					colR = amGammaInvConversion(colR * invA) * colA; \
					colG = amGammaInvConversion(colG * invA) * colA; \
					colB = amGammaInvConversion(colB * invA) * colA; \
				} \
				else { \
					AM_ASSERT(colR == 0.0f); \
					AM_ASSERT(colG == 0.0f); \
					AM_ASSERT(colB == 0.0f); \
				} \
			} \
			/* check that colors are correctly premultiplied (after the interpolation) */ \
			AM_ASSERT(colR <= colA); \
			AM_ASSERT(colG <= colA); \
			AM_ASSERT(colB <= colA); \
		} \
		else { \
			/* generate pixel always in premultiplied format, but take in care the color space */ \
			if (!surfaceLinearColorSpace) { \
				colR *= colA; \
				colG *= colA; \
				colB *= colA; \
			} \
			else { \
				colR = amGammaInvConversion(colR) * colA; \
				colG = amGammaInvConversion(colG) * colA; \
				colB = amGammaInvConversion(colB) * colA; \
			} \
		} \
		AM_ASSERT(colR >= 0.0f && colR <= 1.0f); \
		AM_ASSERT(colG >= 0.0f && colG <= 1.0f); \
		AM_ASSERT(colB >= 0.0f && colB <= 1.0f); \
		AM_ASSERT(colA >= 0.0f && colA <= 1.0f); \
		/* convert color component in the range [0; 255] */ \
		r = (AMuint32)amFloorf(colR * 255.0f + 0.5f); \
		g = (AMuint32)amFloorf(colG * 255.0f + 0.5f); \
		b = (AMuint32)amFloorf(colB * 255.0f + 0.5f); \
		a = (AMuint32)amFloorf(colA * 255.0f + 0.5f); \
		AM_ASSERT(r <= 255 && g <= 255 && b <= 255 && a <= 255); \
		/* write the pixel */ \
		pixels[i] = (r << rSh) | (g << gSh) | (b << bSh) | (a << aSh); \
		t += step;

	#define MANAGE_MEMORY_ERRORS \
		if (tmpKeys.error || inTangents.error || outTangents.error) { \
			AM_DYNARRAY_DESTROY(tmpKeys) \
			AM_DYNARRAY_DESTROY(inTangents) \
			AM_DYNARRAY_DESTROY(outTangents) \
			return AM_FALSE; \
		}

	AM_ASSERT(pixels);
	AM_ASSERT(width > 0);
	AM_ASSERT(sColStops && sColStops->size >= 2);
	AM_ASSERT(pxlFormatTable[srfFormatIdx][FMT_BITS] == 32);

	AM_DYNARRAY_PREINIT(tmpKeys)
	AM_DYNARRAY_PREINIT(inTangents)
	AM_DYNARRAY_PREINIT(outTangents)

	j = (AMint32)sColStops->size;
	if (reflectKeys) {
		AM_DYNARRAY_INIT_RESERVE(tmpKeys, AMColorStop, ((j * 2) - 1))
		// for smooth interpolation we have to generate also tangents
		if (!linearInterpolation) {
			AM_DYNARRAY_INIT_RESERVE(inTangents, AMColorStop, ((j * 2) - 1))
			AM_DYNARRAY_INIT_RESERVE(outTangents, AMColorStop, ((j * 2) - 1))
		}
	}
	else {
		AM_DYNARRAY_INIT_RESERVE(tmpKeys, AMColorStop, sColStops->size)
		// for smooth interpolation we have to generate also tangents
		if (!linearInterpolation) {
			AM_DYNARRAY_INIT_RESERVE(inTangents, AMColorStop, sColStops->size)
			AM_DYNARRAY_INIT_RESERVE(outTangents, AMColorStop, sColStops->size)
		}
	}
	MANAGE_MEMORY_ERRORS

	// push all the keys
	for (i = 0; i < j; ++i) {
		// make sure that original keys array is valid (color values between 0.0f and 1.0f)
		AM_ASSERT(sColStops->data[i].color[AM_R] >= 0.0f && sColStops->data[i].color[AM_R] <= 1.0f);
		AM_ASSERT(sColStops->data[i].color[AM_G] >= 0.0f && sColStops->data[i].color[AM_G] <= 1.0f);
		AM_ASSERT(sColStops->data[i].color[AM_B] >= 0.0f && sColStops->data[i].color[AM_B] <= 1.0f);
		AM_ASSERT(sColStops->data[i].color[AM_A] >= 0.0f && sColStops->data[i].color[AM_B] <= 1.0f);

		if (colorRampPremultiplied) {

			AMColorStop tmp = sColStops->data[i];

			tmp.color[AM_R] *= tmp.color[AM_A];
			tmp.color[AM_G] *= tmp.color[AM_A];
			tmp.color[AM_B] *= tmp.color[AM_A];
			AM_DYNARRAY_PUSH_BACK(tmpKeys, AMColorStop, tmp)
		}
		else {
			AM_DYNARRAY_PUSH_BACK(tmpKeys, AMColorStop, sColStops->data[i])
		}
	}

	// reflect keys, is specified
	if (reflectKeys) {
		for (i = j - 2; i >= 0; --i) {
			key = tmpKeys.data[i];
			// reflect the time position
			key.position = 2.0f - key.position;
			// push the reflected key
			AM_DYNARRAY_PUSH_BACK(tmpKeys, AMColorStop, key)
		}
		domainLen = 2.0f;
	}
	else
		domainLen = 1.0f;

	MANAGE_MEMORY_ERRORS

	step = domainLen / (AMfloat)(width - 1);
	t = 0.0f;

	if (linearInterpolation) {

		for (i = 0; i < (AMint32)width; ++i) {
			// find the couple of key that bound u
			k1 = 0;
			while (k1 < (AMint32)tmpKeys.size && tmpKeys.data[k1].position <= t)
				k1++;

			AM_ASSERT(k1 >= 1);
			if (k1 >= (AMint32)tmpKeys.size)
				k1 = (AMint32)tmpKeys.size - 1;

			k0 = k1 - 1;

			AM_ASSERT(tmpKeys.data[k1].position > tmpKeys.data[k0].position);
			u = (t - tmpKeys.data[k0].position) / (tmpKeys.data[k1].position - tmpKeys.data[k0].position);
			AM_ASSERT(u >= 0.0f && u <= 1.0f);
			u1 = 1.0f - u;
			// linear color interpolation
			colR = (u1 * tmpKeys.data[k0].color[AM_R]) + (u * tmpKeys.data[k1].color[AM_R]);
			colG = (u1 * tmpKeys.data[k0].color[AM_G]) + (u * tmpKeys.data[k1].color[AM_G]);
			colB = (u1 * tmpKeys.data[k0].color[AM_B]) + (u * tmpKeys.data[k1].color[AM_B]);
			colA = (u1 * tmpKeys.data[k0].color[AM_A]) + (u * tmpKeys.data[k1].color[AM_A]);
			PIXEL_PIPELINE()
		}
	}
	// smooth interpolation
	else {
		AMfloat h1, h2, h3, h4, u2, u3;

		// initialize tangents
		amGradientsTangentsGenerate(&inTangents, &outTangents, &tmpKeys);

		for (i = 0; i < (AMint32)width; ++i) {
			// find the couple of key that bound u
			k1 = 0;
			while (k1 < (AMint32)tmpKeys.size && tmpKeys.data[k1].position <= t)
				k1++;

			AM_ASSERT(k1 >= 1 && k1 < (AMint32)tmpKeys.size);
			k0 = k1 - 1;

			AM_ASSERT(tmpKeys.data[k1].position > tmpKeys.data[k0].position);
			u = (t - tmpKeys.data[k0].position) / (tmpKeys.data[k1].position - tmpKeys.data[k0].position);
			AM_ASSERT(u >= 0.0f && u <= 1.0f);
			
			u2 = u * u;
			u3 = u2 * u;
			// Hermite basis functions
			h1 =  2.0f * u3 - 3.0f * u2 + 1.0f;
			h2 = -2.0f * u3 + 3.0f * u2;
			h3 = u3 - 2.0f * u2 + u;
			h4 = u3 - u2;
			// Hermite color interpolation
			colR = h1 * tmpKeys.data[k0].color[AM_R] + h2 * tmpKeys.data[k1].color[AM_R] +
				   h3 * outTangents.data[k0].color[AM_R] + h4 * inTangents.data[k1].color[AM_R];
			colG = h1 * tmpKeys.data[k0].color[AM_G] + h2 * tmpKeys.data[k1].color[AM_G] +
				   h3 * outTangents.data[k0].color[AM_G] + h4 * inTangents.data[k1].color[AM_G];
			colB = h1 * tmpKeys.data[k0].color[AM_B] + h2 * tmpKeys.data[k1].color[AM_B] +
				   h3 * outTangents.data[k0].color[AM_B] + h4 * inTangents.data[k1].color[AM_B];
			colA = h1 * tmpKeys.data[k0].color[AM_A] + h2 * tmpKeys.data[k1].color[AM_A] +
				   h3 * outTangents.data[k0].color[AM_A] + h4 * inTangents.data[k1].color[AM_A];

			colR = AM_CLAMP(colR, 0.0f, 1.0f);
			colG = AM_CLAMP(colG, 0.0f, 1.0f);
			colB = AM_CLAMP(colB, 0.0f, 1.0f);
			colA = AM_CLAMP(colA, 0.0f, 1.0f);
			PIXEL_PIPELINE()
		}
	}
	// destroy temporary array
	AM_DYNARRAY_DESTROY(tmpKeys)
		AM_DYNARRAY_DESTROY(inTangents)
		AM_DYNARRAY_DESTROY(outTangents)
	return AM_TRUE;

	#undef PIXEL_PIPELINE
	#undef MANAGE_MEMORY_ERRORS
}

/*!
	\brief Generate textures to use for linear and radial gradients.
	\param paintDesc output paint descriptor, containing gradient textures.
	\param context context containing the color transform values.
	\param surface drawing surface used to read its pixel format.
	\param ctReferenceHash hash of the desired color transform to use for the texture pixels generation.
	\return AM_FALSE if memory allocation fails, else AM_TRUE.
	\note please note that this function set paint->gradTexturesValid flag to AM_TRUE.
*/
AMbool amGradientsTexturesUpdate(AMPaintDesc *paintDesc,
							   const AMContext *context,
							   const AMDrawingSurface *surface,
							   const AMuint32 ctReferenceHash) {

	AMfloat t, minAlpha, maxAlpha, oldT;
	AMuint32 srfFormatIdx = AM_FMT_GET_INDEX(amSrfFormat32Get(surface));
	AMuint32 textureWidth;
	AMPaint *paint = paintDesc->paint;
	AMColorStopDynArray patchedColorStops;
	const AMfloat *keys;
	AMColorStop key;
	AMint32 i, keysCount;
	AMbool linearInterpolation;
	const AMfloat *colorTransformation;

	AM_ASSERT(paint && paint->referenceCounter > 0);
	AM_ASSERT(context);
#if (AM_OPENVG_VERSION >= 110)
	AM_ASSERT(!paint->gradTexturesValid || paint->ctTexturesHash != ctReferenceHash);
#else
	AM_ASSERT(!paint->gradTexturesValid);
#endif

	// allocate structures for a purged (made of at least two ordered keys) keys array
	keys = (const AMfloat *)paint->sColorStops.data;
	keysCount = (AMint32)paint->sColorStops.size;

	if (keysCount > 0) {
		AM_DYNARRAY_INIT_RESERVE(patchedColorStops, AMColorStop, keysCount)
		t = oldT = keys[0];
	}
	else {
		AM_DYNARRAY_INIT_RESERVE(patchedColorStops, AMColorStop, 2)
	}

	if (patchedColorStops.error)
		return AM_FALSE;

	minAlpha = 1.0f;
	maxAlpha = 0.0f;
	oldT = 0.0f;
	// generate a valid keys array
	for (i = 0; i < keysCount; ++i, keys += 5) {

		t = keys[0];
		// for unordered sequences, we have to throw away the entire sequence
		if (t < oldT) {
			patchedColorStops.size = 0;
			break;
		}
		// stops outside [0; 1] range must be discarded
		if (t < 0.0f || t > 1.0f)
			continue;

		// for stops that fall on the same offset we have to keep only first and last entries
		if (amAbsf(t - oldT) <= AM_EPSILON_FLOAT) {
			if (i < keysCount - 1) {
				if (amAbsf(keys[5] - t) <= AM_EPSILON_FLOAT)
					continue;
				else
					// to avoid numerical imprecision we force 't' to 'oldT'
					t = oldT;
			}
		}
		// push the color stop
		key.position = t;
		key.color[AM_R] = AM_CLAMP(keys[1], 0.0f, 1.0f);
		key.color[AM_G] = AM_CLAMP(keys[2], 0.0f, 1.0f);
		key.color[AM_B] = AM_CLAMP(keys[3], 0.0f, 1.0f);
		key.color[AM_A] = AM_CLAMP(keys[4], 0.0f, 1.0f);
		AM_DYNARRAY_PUSH_BACK(patchedColorStops, AMColorStop, key)
		// update min/max values of alpha
		if (key.color[AM_A] < minAlpha)
			minAlpha = key.color[AM_A];
		if (key.color[AM_A] > maxAlpha)
			maxAlpha = key.color[AM_A];
		// update old timeline position
		oldT = t;
	}

	// if array is empty we have to insert 2 keys at time 0 and 1
	if (patchedColorStops.size == 0) {
		// push a black key
		key.position = 0.0f;
		key.color[AM_R] = 0.0f;
		key.color[AM_G] = 0.0f;
		key.color[AM_B] = 0.0f;
		key.color[AM_A] = 1.0f;
		AM_DYNARRAY_PUSH_BACK(patchedColorStops, AMColorStop, key)
		// push a white key
		key.position = 1.0f;
		key.color[AM_R] = 1.0f;
		key.color[AM_G] = 1.0f;
		key.color[AM_B] = 1.0f;
		key.color[AM_A] = 1.0f;
		AM_DYNARRAY_PUSH_BACK(patchedColorStops, AMColorStop, key)
		minAlpha = maxAlpha = 1.0f;
	}
	else {
		// if first key is not at time 0, we add a key at time 0 with the same value
		key = patchedColorStops.data[0];
		if (key.position > 0.0f) {
			key.position = 0.0f;
			AM_DYNARRAY_INSERT(patchedColorStops, AMColorStop, 0, key)
		}
		// if last key is not at time 1, we add a key at time 1 with the same value
		i = (AMint32)patchedColorStops.size - 1;
		key = patchedColorStops.data[i];
		if (key.position < 1.0f) {
			key.position = 1.0f;
			AM_DYNARRAY_PUSH_BACK(patchedColorStops, AMColorStop, key)
		}
	}
	// check memory errors
	if (patchedColorStops.error) {
		AM_DYNARRAY_DESTROY(patchedColorStops)
		return AM_FALSE;
	}
	
	AM_ASSERT(patchedColorStops.data[0].position == 0.0f);
	if (patchedColorStops.data[0].position == patchedColorStops.data[1].position) {
		AM_DYNARRAY_ERASE(patchedColorStops, 0);
	}

	AM_ASSERT(patchedColorStops.size >= 2);
	i = (AMint32)patchedColorStops.size - 2;
	AM_ASSERT(patchedColorStops.data[i + 1].position == 1.0f);
	if (patchedColorStops.data[i].position == patchedColorStops.data[i + 1].position) {
		AM_DYNARRAY_POP_BACK(patchedColorStops);
	}
	AM_ASSERT(patchedColorStops.size >= 2);

#if defined(VG_MZT_color_ramp_interpolation)
	linearInterpolation = (paint->colorRampInterpolationType == VG_COLOR_RAMP_INTERPOLATION_LINEAR_MZT) ? AM_TRUE : AM_FALSE;
#else
	linearInterpolation = AM_TRUE;
#endif

#if (AM_OPENVG_VERSION >= 110)
	colorTransformation = (paintDesc->colorTransform && ctReferenceHash != AM_COLOR_TRANSFORM_IDENTITY_HASH) ? context->clampedColorTransformValues : NULL;
#else
	colorTransformation = NULL;
#endif

	// assign minimum and maximum alpha key values
	paint->gradientMinAlpha = minAlpha;
	paint->gradientMaxAlpha = maxAlpha;

#if defined(AM_SRE)
	textureWidth = AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH;
#elif defined(AM_GLE) || defined(AM_GLS)
	textureWidth = AM_MIN(AM_GRADIENTS_LINEAR_RADIAL_TEXTURE_WIDTH, context->glContext.maxTextureSize);
#else
	#error Undefined engine!
#endif
	
	// allocate pixels for the non-reflected texture, if needed (the first time)
	if (!paint->gradTexture.pixels) {
		paint->gradTexture.pixels = (AMuint32 *)amMalloc(textureWidth * sizeof(AMuint32));
		if (!paint->gradTexture.pixels) {
			AM_DYNARRAY_DESTROY(patchedColorStops)
			return AM_FALSE;
                    }              
		    paint->gradTexture.width = textureWidth;
		    paint->gradTexture.height = 1;
	}
	AM_ASSERT(paint->gradTexture.pixels);
	// create the non-reflected 1D texture
	if (!amGradientPixelsGenerate(paint->gradTexture.pixels, paint->gradTexture.width, &patchedColorStops,
								  AM_FALSE, linearInterpolation, paint->colorRampPremultiplied, srfFormatIdx, colorTransformation)) {
		AM_DYNARRAY_DESTROY(patchedColorStops)
		return AM_FALSE;
	}

	// allocate pixels for the reflected texture, if needed (the first time)
	if (!paint->reflectGradTexture.pixels) {
		paint->reflectGradTexture.pixels = (AMuint32 *)amMalloc(textureWidth * sizeof(AMuint32));
		if (!paint->reflectGradTexture.pixels) {
			AM_DYNARRAY_DESTROY(patchedColorStops)
			return AM_FALSE;
                    }
		    paint->reflectGradTexture.width = textureWidth;
		    paint->reflectGradTexture.height = 1;
	}
	AM_ASSERT(paint->reflectGradTexture.pixels);
	// create the reflected 1D texture
	if (!amGradientPixelsGenerate(paint->reflectGradTexture.pixels, paint->reflectGradTexture.width, &patchedColorStops,
								  AM_TRUE, linearInterpolation, paint->colorRampPremultiplied, srfFormatIdx, colorTransformation)) {
		AM_DYNARRAY_DESTROY(patchedColorStops)
		return AM_FALSE;
	}

	// gradient textures used to realize draw images in mutiply mode, are not influenced by the color transform because, in that case, color transform must be
	// applied to the multiplication of image * paint (and not on paint)
	if (!paint->gradTexturesValid) {

		AMuint32 dstIdx;

		// update texture used to draw images in mutiply mode (pixels must be in the same byteorder used for GL textures)
		switch (pxlFormatTable[srfFormatIdx][FMT_ORDER]) {
			case AM_PIXEL_FMT_RGBA:
				dstIdx = AM_FMT_GET_INDEX(VG_sRGBA_8888_PRE);
				break;
			case AM_PIXEL_FMT_ARGB:
				dstIdx = AM_FMT_GET_INDEX(VG_sARGB_8888_PRE);
				break;
			case AM_PIXEL_FMT_BGRA:
				dstIdx = AM_FMT_GET_INDEX(VG_sBGRA_8888_PRE);
				break;
			default:
				dstIdx = AM_FMT_GET_INDEX(VG_sABGR_8888_PRE);
				break;
		}
		// allocate pixels for the non-reflected texture (images in multiply mode), if needed (the first time)
		if (!paint->gradTextureImgMultiply.pixels) {
			paint->gradTextureImgMultiply.pixels = (AMuint32 *)amMalloc(textureWidth * sizeof(AMuint32));
			if (!paint->gradTextureImgMultiply.pixels) {
				AM_DYNARRAY_DESTROY(patchedColorStops)
				return AM_FALSE;
                        }
			paint->gradTextureImgMultiply.width = textureWidth;
			paint->gradTextureImgMultiply.height = 1;
		}
		AM_ASSERT(paint->gradTextureImgMultiply.pixels);
		// create the non-reflected 1D texture, for images in multiply mode
		if (!amGradientPixelsGenerate(paint->gradTextureImgMultiply.pixels, paint->gradTextureImgMultiply.width, &patchedColorStops,
									  AM_FALSE, linearInterpolation, paint->colorRampPremultiplied, dstIdx, NULL)) {
			AM_DYNARRAY_DESTROY(patchedColorStops)
			return AM_FALSE;
		}

		// allocate pixels for the reflected texture (images in multiply mode), if needed (the first time)
		if (!paint->reflectGradTextureImgMultiply.pixels) {
			paint->reflectGradTextureImgMultiply.pixels = (AMuint32 *)amMalloc(textureWidth * sizeof(AMuint32));
			if (!paint->reflectGradTextureImgMultiply.pixels) {
				AM_DYNARRAY_DESTROY(patchedColorStops)
				return AM_FALSE;
                        }
			paint->reflectGradTextureImgMultiply.width = textureWidth;
			paint->reflectGradTextureImgMultiply.height = 1;
		}
		AM_ASSERT(paint->reflectGradTextureImgMultiply.pixels);
		// create the reflected 1D texture, for images in multiply mode
		if (!amGradientPixelsGenerate(paint->reflectGradTextureImgMultiply.pixels, paint->reflectGradTextureImgMultiply.width, &patchedColorStops,
									  AM_TRUE, linearInterpolation, paint->colorRampPremultiplied, dstIdx, NULL)) {
			AM_DYNARRAY_DESTROY(patchedColorStops)
			return AM_FALSE;
		}
	}

#if defined(AM_GLE) || defined(AM_GLS)
	amGlGradientsTexturesUpload(paintDesc, context, ctReferenceHash, colorTransformation);
#endif

	// now gradient textures are valid
#if (AM_OPENVG_VERSION >= 110)
	paint->ctTexturesHash = ctReferenceHash;
#endif
	paint->gradTexturesValid = AM_TRUE;
	// free temporary patched color stops array
	AM_DYNARRAY_DESTROY(patchedColorStops)
	return AM_TRUE;
}

#if defined (RIM_VG_SRC)
#pragma pop
#endif


