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
	\file fillers_Overlay.c
	\brief VG_BLEND_OVERLAY_MZT fillers, implementation ( \b generated \b file ).
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#if defined(AM_SRE) && !defined(AM_LITE_PROFILE)

#include "fillers.h"
#include "vggradients.h"
#include "pixel_utils.h"

#if defined(VG_MZT_advanced_blend_modes)

/*!
	\brief Filler: paint type VG_PAINT_TYPE_COLOR, blend mode VG_BLEND_OVERLAY_MZT, masking off ( \b generated \b function ).
	\param _surface pointer to a AMDrawingSurface structure, defining the destination drawing surface.
	\param _paintGen pointer to a AMPaintGen structure, containing paint information and derived values.
	\param y y coordinate of the scanline to fill.
	\param x0 x coordinate of the left-most pixel to fill.
	\param x1 x coordinate of the right-most pixel to fill.
	\note coverage line deltas are cleared (set to 0) during the loop, if needed.
*/
void amFilPath_ColorOverlay(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1) {

	AMDrawingSurface *surface = (AMDrawingSurface *)_surface;
	AMPaintGen *paintGen = (AMPaintGen *)_paintGen;
	AMuint32 *scrPixels;
	AMint32 *covLine;
	AMuint32 i, ofs0;
	AMint32 cov;
	AMPixel32 DcaDa, ScaSa;
	AMPixel32 tmp;
	AMint32 oldCov;
	AMPixel32 oldDcaDa;
	AMuint32 invSa;
	AMuint32 invDa;
	AMPixel32 t0, t1, t2, t3, t4, t5;
	AMint32 R, G, B;

	AM_ASSERT(surface);
	AM_ASSERT(paintGen);
	AM_ASSERT(paintGen->coverageLineDeltas);
	AM_ASSERT(y >= 0 && y < amSrfHeightGet(surface));
	AM_ASSERT(x0 <= x1);
	AM_ASSERT(x0 >= 0 && x0 < amSrfWidthGet(surface));
	AM_ASSERT(x1 >= 0 && x1 < amSrfWidthGet(surface));

	ofs0 = (amSrfHeightGet(surface) - y - 1) * amSrfWidthGet(surface) + x0;
	// go to the start screen pixel
	scrPixels = amSrfPixelsGet(surface) + ofs0;
	// extract coverage line
	covLine = paintGen->coverageLineDeltas + x0;
	ScaSa.value = paintGen->paintColor32;
	invSa = ScaSa.c.a ^ 0xFF;  // it's equal to 255 - Sa
	i = x1 - x0 + 1;
	cov = (*covLine);
	for (;;) {
		*covLine = 0;
		while (cov == 0) {
			scrPixels++;
			if (--i == 0) {
				covLine[1] = 0;
				return;
			}
			covLine++;
			cov += (*covLine);
		}
		*covLine = 0;
		DcaDa.value = oldDcaDa.value = *scrPixels;
		// apply blend equation
		invDa = DcaDa.c.a ^ 0xFF;
		t0.value = amPxlScl255(invDa, ScaSa.value);      // Sca.(1 - Da)
		t1.value = amPxlScl255(invSa, DcaDa.value);      // Dca.(1 - Sa)
		t2.value = amPxlMul(ScaSa.value, DcaDa.value); // Sca.Dca
		t3 = DcaDa;
		t4 = ScaSa;
		t5.value = t0.value + t1.value;

		B = (AMint32)t3.c.b * 2 - t3.c.a;
		B = (B < 0) ? (AMint32)t2.c.b * 2 + t5.c.b : ((((AMint32)t4.c.b * t3.c.a + (AMint32)t3.c.b * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.b * 2 + t5.c.b;
		G = (AMint32)t3.c.g * 2 - t3.c.a;
		G = (G < 0) ? (AMint32)t2.c.g * 2 + t5.c.g : ((((AMint32)t4.c.g * t3.c.a + (AMint32)t3.c.g * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.g * 2 + t5.c.g;
		R = (AMint32)t3.c.r * 2 - t3.c.a;
		R = (R < 0) ? (AMint32)t2.c.r * 2 + t5.c.r : ((((AMint32)t4.c.r * t3.c.a + (AMint32)t3.c.r * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.r * 2 + t5.c.r;
		tmp.c.a = t4.c.a + t1.c.a;
		tmp.c.b = AM_CLAMP(B, 0, 255);
		tmp.c.g = AM_CLAMP(G, 0, 255);
		tmp.c.r = AM_CLAMP(R, 0, 255);

		if (cov != AM_RAS_MAX_COVERAGE) {
			tmp.value = amPxlLerp(((AMuint32)cov) >> AM_RAS_COVERAGE_PRECISION, DcaDa.value, tmp.value);
			do {
				*scrPixels++ = tmp.value;
				if (--i == 0) {
					covLine[1] = 0;
					return;
				}
				oldCov = cov;
				oldDcaDa = DcaDa;
				covLine++;
				cov += *covLine;
				DcaDa.value = *scrPixels;
			} while ((cov == oldCov) && (DcaDa.value == oldDcaDa.value));
		}
		else {
			do {
				*scrPixels++ = tmp.value;
				if (--i == 0) {
					covLine[1] = 0;
					return;
				}
				oldDcaDa = DcaDa;
				covLine++;
				cov += *covLine;
				DcaDa.value = *scrPixels;
			} while ((cov == AM_RAS_MAX_COVERAGE) && (DcaDa.value == oldDcaDa.value));
		}
	}
}

/*!
	\brief Filler: paint type VG_PAINT_TYPE_LINEAR_GRADIENT, blend mode VG_BLEND_OVERLAY_MZT, masking off ( \b generated \b function ).
	\param _surface pointer to a AMDrawingSurface structure, defining the destination drawing surface.
	\param _paintGen pointer to a AMPaintGen structure, containing paint information and derived values.
	\param y y coordinate of the scanline to fill.
	\param x0 x coordinate of the left-most pixel to fill.
	\param x1 x coordinate of the right-most pixel to fill.
	\note coverage line deltas are cleared (set to 0) during the loop, if needed.
*/
void amFilPath_LinGradOverlay(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1) {

	AMDrawingSurface *surface = (AMDrawingSurface *)_surface;
	AMPaintGen *paintGen = (AMPaintGen *)_paintGen;
	AMuint32 *scrPixels;
	AMint32 *covLine;
	AMuint32 i, ofs0;
	AMuint32 j;
	AMint32 cov;
	AMPixel32 DcaDa, ScaSa;
	AMPixel32 tmp;
	AMint32 invCov;
	AMint32 tmpCov;
	AMuint32 invSa;
	AMuint32 invDa;
	AMPixel32 t0, t1, t2, t3, t4, t5;
	AMint32 R, G, B;
	AMint32 fixedU;
	const AMuint32 *gradientPixels;
	VGColorRampSpreadMode spreadMode;

	AM_ASSERT(surface);
	AM_ASSERT(paintGen);
	AM_ASSERT(paintGen->coverageLineDeltas);
	AM_ASSERT(y >= 0 && y < amSrfHeightGet(surface));
	AM_ASSERT(x0 <= x1);
	AM_ASSERT(x0 >= 0 && x0 < amSrfWidthGet(surface));
	AM_ASSERT(x1 >= 0 && x1 < amSrfWidthGet(surface));

	ofs0 = (amSrfHeightGet(surface) - y - 1) * amSrfWidthGet(surface) + x0;
	// go to the start screen pixel
	scrPixels = amSrfPixelsGet(surface) + ofs0;
	// extract coverage line
	covLine = paintGen->coverageLineDeltas + x0;
	// initialize the paint generation
	amPaintLinGradInit(paintGen, x0 - 1, y);
	spreadMode = paintGen->paintDesc->paint->colorRampSpreadMode;
	gradientPixels = paintGen->gradTexture;
	j = 0;
	i = x1 - x0 + 1;
	cov = 0;

loop_LinGradOverlay:
	// update paint generators
	paintGen->Ux += paintGen->DUx;
	cov += covLine[j];
	if (covLine[j] != 0)
		covLine[j] = 0;
	tmpCov = cov >> AM_RAS_COVERAGE_PRECISION;
	if (tmpCov != 0) {
		fixedU = paintGen->Ux;
		AM_GRADIENTS_LINEAR_PIXEL_GET(ScaSa.value, fixedU)
		// get the pixel on the buffer, if blendMode is different than Src
		DcaDa.value = scrPixels[j];
		// apply blend equation
		invSa = ScaSa.c.a ^ 0xFF;
		invDa = DcaDa.c.a ^ 0xFF;
		t0.value = amPxlScl255(invDa, ScaSa.value);      // Sca.(1 - Da)
		t1.value = amPxlScl255(invSa, DcaDa.value);      // Dca.(1 - Sa)
		t2.value = amPxlMul(ScaSa.value, DcaDa.value); // Sca.Dca
		t3 = DcaDa;
		t4 = ScaSa;
		t5.value = t0.value + t1.value;

		B = (AMint32)t3.c.b * 2 - t3.c.a;
		B = (B < 0) ? (AMint32)t2.c.b * 2 + t5.c.b : ((((AMint32)t4.c.b * t3.c.a + (AMint32)t3.c.b * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.b * 2 + t5.c.b;
		G = (AMint32)t3.c.g * 2 - t3.c.a;
		G = (G < 0) ? (AMint32)t2.c.g * 2 + t5.c.g : ((((AMint32)t4.c.g * t3.c.a + (AMint32)t3.c.g * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.g * 2 + t5.c.g;
		R = (AMint32)t3.c.r * 2 - t3.c.a;
		R = (R < 0) ? (AMint32)t2.c.r * 2 + t5.c.r : ((((AMint32)t4.c.r * t3.c.a + (AMint32)t3.c.r * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.r * 2 + t5.c.r;
		tmp.c.a = t4.c.a + t1.c.a;
		tmp.c.b = AM_CLAMP(B, 0, 255);
		tmp.c.g = AM_CLAMP(G, 0, 255);
		tmp.c.r = AM_CLAMP(R, 0, 255);

		// apply coverage
		invCov = 256 - tmpCov;
		if (invCov == 0) {
			// write pixel
			scrPixels[j++] = tmp.value;
			if (--i != 0)
				goto loop_LinGradOverlay;
			else {
				covLine[j] = 0;
				return;
			}
		}
		else {
			// write pixel
			scrPixels[j++] = amPxlLerp((AMuint32)tmpCov, DcaDa.value, tmp.value);
			if (--i != 0)
				goto loop_LinGradOverlay;
			else {
				covLine[j] = 0;
				return;
			}
		}
	}
	j++;
	if (--i != 0)
		goto loop_LinGradOverlay;
	else 
		covLine[j] = 0;
}

/*!
	\brief Filler: paint type VG_PAINT_TYPE_RADIAL_GRADIENT, blend mode VG_BLEND_OVERLAY_MZT, masking off ( \b generated \b function ).
	\param _surface pointer to a AMDrawingSurface structure, defining the destination drawing surface.
	\param _paintGen pointer to a AMPaintGen structure, containing paint information and derived values.
	\param y y coordinate of the scanline to fill.
	\param x0 x coordinate of the left-most pixel to fill.
	\param x1 x coordinate of the right-most pixel to fill.
	\note coverage line deltas are cleared (set to 0) during the loop, if needed.
*/
void amFilPath_RadGradOverlay(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1) {

	AMDrawingSurface *surface = (AMDrawingSurface *)_surface;
	AMPaintGen *paintGen = (AMPaintGen *)_paintGen;
	AMuint32 *scrPixels;
	AMint32 *covLine;
	AMuint32 i, ofs0;
	AMuint32 j;
	AMint32 cov;
	AMPixel32 DcaDa, ScaSa;
	AMPixel32 tmp;
	AMint32 invCov;
	AMint32 tmpCov;
	AMuint32 invSa;
	AMuint32 invDa;
	AMPixel32 t0, t1, t2, t3, t4, t5;
	AMint32 R, G, B;
	AMint32 fixedU;
	const AMuint32 *gradientPixels;
	VGColorRampSpreadMode spreadMode;

	AM_ASSERT(surface);
	AM_ASSERT(paintGen);
	AM_ASSERT(paintGen->coverageLineDeltas);
	AM_ASSERT(y >= 0 && y < amSrfHeightGet(surface));
	AM_ASSERT(x0 <= x1);
	AM_ASSERT(x0 >= 0 && x0 < amSrfWidthGet(surface));
	AM_ASSERT(x1 >= 0 && x1 < amSrfWidthGet(surface));

	ofs0 = (amSrfHeightGet(surface) - y - 1) * amSrfWidthGet(surface) + x0;
	// go to the start screen pixel
	scrPixels = amSrfPixelsGet(surface) + ofs0;
	// extract coverage line
	covLine = paintGen->coverageLineDeltas + x0;
	// initialize the paint generation
	amPaintRadGradInit(paintGen, x0 - 1, y);
	spreadMode = paintGen->paintDesc->paint->colorRampSpreadMode;
	gradientPixels = paintGen->gradTexture;
	j = 0;
	i = x1 - x0 + 1;
	cov = 0;

loop_RadGradOverlay:
	// update paint generators
	paintGen->Uf += paintGen->DUf;
	paintGen->Vf += paintGen->DVf;
	paintGen->A += paintGen->dA;
	paintGen->B += paintGen->dB;
	cov += covLine[j];
	if (covLine[j] != 0)
		covLine[j] = 0;
	tmpCov = cov >> AM_RAS_COVERAGE_PRECISION;
	if (tmpCov != 0) {
		AMfloat s = paintGen->radiusSqr * (paintGen->Uf * paintGen->Uf + paintGen->Vf * paintGen->Vf);
		AMfloat u = paintGen->A + amFastSqrtf(s - (paintGen->B *paintGen->B));
		fixedU = (AMint32)(u * paintGen->K);
		AM_GRADIENTS_RADIAL_PIXEL_GET(ScaSa.value, fixedU)
		// get the pixel on the buffer, if blendMode is different than Src
		DcaDa.value = scrPixels[j];
		// apply blend equation
		invSa = ScaSa.c.a ^ 0xFF;
		invDa = DcaDa.c.a ^ 0xFF;
		t0.value = amPxlScl255(invDa, ScaSa.value);      // Sca.(1 - Da)
		t1.value = amPxlScl255(invSa, DcaDa.value);      // Dca.(1 - Sa)
		t2.value = amPxlMul(ScaSa.value, DcaDa.value); // Sca.Dca
		t3 = DcaDa;
		t4 = ScaSa;
		t5.value = t0.value + t1.value;

		B = (AMint32)t3.c.b * 2 - t3.c.a;
		B = (B < 0) ? (AMint32)t2.c.b * 2 + t5.c.b : ((((AMint32)t4.c.b * t3.c.a + (AMint32)t3.c.b * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.b * 2 + t5.c.b;
		G = (AMint32)t3.c.g * 2 - t3.c.a;
		G = (G < 0) ? (AMint32)t2.c.g * 2 + t5.c.g : ((((AMint32)t4.c.g * t3.c.a + (AMint32)t3.c.g * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.g * 2 + t5.c.g;
		R = (AMint32)t3.c.r * 2 - t3.c.a;
		R = (R < 0) ? (AMint32)t2.c.r * 2 + t5.c.r : ((((AMint32)t4.c.r * t3.c.a + (AMint32)t3.c.r * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.r * 2 + t5.c.r;
		tmp.c.a = t4.c.a + t1.c.a;
		tmp.c.b = AM_CLAMP(B, 0, 255);
		tmp.c.g = AM_CLAMP(G, 0, 255);
		tmp.c.r = AM_CLAMP(R, 0, 255);

		// apply coverage
		invCov = 256 - tmpCov;
		if (invCov == 0) {
			// write pixel
			scrPixels[j++] = tmp.value;
			if (--i != 0)
				goto loop_RadGradOverlay;
			else {
				covLine[j] = 0;
				return;
			}
		}
		else {
			// write pixel
			scrPixels[j++] = amPxlLerp((AMuint32)tmpCov, DcaDa.value, tmp.value);
			if (--i != 0)
				goto loop_RadGradOverlay;
			else {
				covLine[j] = 0;
				return;
			}
		}
	}
	j++;
	if (--i != 0)
		goto loop_RadGradOverlay;
	else 
		covLine[j] = 0;
}

/*!
	\brief Filler: paint type VG_PAINT_TYPE_PATTERN, blend mode VG_BLEND_OVERLAY_MZT, masking off ( \b generated \b function ).
	\param _surface pointer to a AMDrawingSurface structure, defining the destination drawing surface.
	\param _paintGen pointer to a AMPaintGen structure, containing paint information and derived values.
	\param y y coordinate of the scanline to fill.
	\param x0 x coordinate of the left-most pixel to fill.
	\param x1 x coordinate of the right-most pixel to fill.
	\note coverage line deltas are cleared (set to 0) during the loop, if needed.
*/
void amFilPath_PatternOverlay(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1) {

	AMDrawingSurface *surface = (AMDrawingSurface *)_surface;
	AMPaintGen *paintGen = (AMPaintGen *)_paintGen;
	AMuint32 *scrPixels;
	AMint32 *covLine;
	AMuint32 i, ofs0;
	AMuint32 j;
	AMint32 cov;
	AMPixel32 DcaDa, ScaSa;
	AMPixel32 tmp;
	AMint32 invCov;
	AMint32 tmpCov;
	AMuint32 invSa;
	AMuint32 invDa;
	AMPixel32 t0, t1, t2, t3, t4, t5;
	AMint32 R, G, B;

	AM_ASSERT(surface);
	AM_ASSERT(paintGen);
	AM_ASSERT(paintGen->coverageLineDeltas);
	AM_ASSERT(y >= 0 && y < amSrfHeightGet(surface));
	AM_ASSERT(x0 <= x1);
	AM_ASSERT(x0 >= 0 && x0 < amSrfWidthGet(surface));
	AM_ASSERT(x1 >= 0 && x1 < amSrfWidthGet(surface));

	ofs0 = (amSrfHeightGet(surface) - y - 1) * amSrfWidthGet(surface) + x0;
	// go to the start screen pixel
	scrPixels = amSrfPixelsGet(surface) + ofs0;
	// extract coverage line
	covLine = paintGen->coverageLineDeltas + x0;
	// initialize the paint generation
	amPaintPatternInit(paintGen, x0 - 1, y);
	j = 0;
	i = x1 - x0 + 1;
	cov = 0;

loop_PatternOverlay:
	// update paint generators
	paintGen->Ux += paintGen->DUx;
	paintGen->Vx += paintGen->DVx;
	cov += covLine[j];
	if (covLine[j] != 0)
		covLine[j] = 0;
	tmpCov = cov >> AM_RAS_COVERAGE_PRECISION;
	if (tmpCov != 0) {
		paintGen->patternSamplerParams.x = paintGen->Ux;
		paintGen->patternSamplerParams.y = paintGen->Vx;
		ScaSa.value = paintGen->patternSampler(&paintGen->patternSamplerParams);
		// get the pixel on the buffer, if blendMode is different than Src
		DcaDa.value = scrPixels[j];
		// apply blend equation
		invSa = ScaSa.c.a ^ 0xFF;
		invDa = DcaDa.c.a ^ 0xFF;
		t0.value = amPxlScl255(invDa, ScaSa.value);      // Sca.(1 - Da)
		t1.value = amPxlScl255(invSa, DcaDa.value);      // Dca.(1 - Sa)
		t2.value = amPxlMul(ScaSa.value, DcaDa.value); // Sca.Dca
		t3 = DcaDa;
		t4 = ScaSa;
		t5.value = t0.value + t1.value;

		B = (AMint32)t3.c.b * 2 - t3.c.a;
		B = (B < 0) ? (AMint32)t2.c.b * 2 + t5.c.b : ((((AMint32)t4.c.b * t3.c.a + (AMint32)t3.c.b * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.b * 2 + t5.c.b;
		G = (AMint32)t3.c.g * 2 - t3.c.a;
		G = (G < 0) ? (AMint32)t2.c.g * 2 + t5.c.g : ((((AMint32)t4.c.g * t3.c.a + (AMint32)t3.c.g * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.g * 2 + t5.c.g;
		R = (AMint32)t3.c.r * 2 - t3.c.a;
		R = (R < 0) ? (AMint32)t2.c.r * 2 + t5.c.r : ((((AMint32)t4.c.r * t3.c.a + (AMint32)t3.c.r * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.r * 2 + t5.c.r;
		tmp.c.a = t4.c.a + t1.c.a;
		tmp.c.b = AM_CLAMP(B, 0, 255);
		tmp.c.g = AM_CLAMP(G, 0, 255);
		tmp.c.r = AM_CLAMP(R, 0, 255);

		// apply coverage
		invCov = 256 - tmpCov;
		if (invCov == 0) {
			// write pixel
			scrPixels[j++] = tmp.value;
			if (--i != 0)
				goto loop_PatternOverlay;
			else {
				covLine[j] = 0;
				return;
			}
		}
		else {
			// write pixel
			scrPixels[j++] = amPxlLerp((AMuint32)tmpCov, DcaDa.value, tmp.value);
			if (--i != 0)
				goto loop_PatternOverlay;
			else {
				covLine[j] = 0;
				return;
			}
		}
	}
	j++;
	if (--i != 0)
		goto loop_PatternOverlay;
	else 
		covLine[j] = 0;
}

#if defined(VG_MZT_conical_gradient)
/*!
	\brief Filler: paint type VG_PAINT_TYPE_CONICAL_GRADIENT_MZT, blend mode VG_BLEND_OVERLAY_MZT, masking off ( \b generated \b function ).
	\param _surface pointer to a AMDrawingSurface structure, defining the destination drawing surface.
	\param _paintGen pointer to a AMPaintGen structure, containing paint information and derived values.
	\param y y coordinate of the scanline to fill.
	\param x0 x coordinate of the left-most pixel to fill.
	\param x1 x coordinate of the right-most pixel to fill.
	\note coverage line deltas are cleared (set to 0) during the loop, if needed.
*/
void amFilPath_ConGradOverlay(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1) {

	AMDrawingSurface *surface = (AMDrawingSurface *)_surface;
	AMPaintGen *paintGen = (AMPaintGen *)_paintGen;
	AMuint32 *scrPixels;
	AMint32 *covLine;
	AMuint32 i, ofs0;
	AMuint32 j;
	AMint32 cov;
	AMPixel32 DcaDa, ScaSa;
	AMPixel32 tmp;
	AMint32 invCov;
	AMint32 tmpCov;
	AMuint32 invSa;
	AMuint32 invDa;
	AMPixel32 t0, t1, t2, t3, t4, t5;
	AMint32 R, G, B;
	AMint32 fixedU;
	AMint32 fixedV;
	const AMuint32 *gradientPixels;
	VGColorRampSpreadMode spreadMode;

	AM_ASSERT(surface);
	AM_ASSERT(paintGen);
	AM_ASSERT(paintGen->coverageLineDeltas);
	AM_ASSERT(y >= 0 && y < amSrfHeightGet(surface));
	AM_ASSERT(x0 <= x1);
	AM_ASSERT(x0 >= 0 && x0 < amSrfWidthGet(surface));
	AM_ASSERT(x1 >= 0 && x1 < amSrfWidthGet(surface));

	ofs0 = (amSrfHeightGet(surface) - y - 1) * amSrfWidthGet(surface) + x0;
	// go to the start screen pixel
	scrPixels = amSrfPixelsGet(surface) + ofs0;
	// extract coverage line
	covLine = paintGen->coverageLineDeltas + x0;
	// initialize the paint generation
	amPaintConGradInit(paintGen, x0 - 1, y);
	spreadMode = paintGen->paintDesc->paint->colorRampSpreadMode;
	gradientPixels = paintGen->gradTexture;
	j = 0;
	i = x1 - x0 + 1;
	cov = 0;

loop_ConGradOverlay:
	// update paint generators
	paintGen->Uf += paintGen->DUf;
	paintGen->Vf += paintGen->DVf;
	cov += covLine[j];
	if (covLine[j] != 0)
		covLine[j] = 0;
	tmpCov = cov >> AM_RAS_COVERAGE_PRECISION;
	if (tmpCov != 0) {
		AMfloat x = paintGen->Uf;
		AMfloat y = paintGen->Vf;
		AMfloat l = x * x + y * y;
		AMuint32 atan2Val;
		AMuint32 atan2U;
		if (l != 0) {
			AMfloat xhalf = 0.5f * l;
			AMint32 f = *(AMint32 *)&l;
			f = 0x5f3759df - (f >> 1);
			l = *(AMfloat*)&f;
			l = l * (1.5f - xhalf * l * l);
			x *= l;
			y *= l;
		}
		x = x * AM_ATAN2_TABLE_SIZE_K + AM_ATAN2_TABLE_SIZE_K;
		y = y * AM_ATAN2_TABLE_SIZE_K + AM_ATAN2_TABLE_SIZE_K;
		fixedU = (AMint32)x;
		fixedV = (AMint32)y;
		AM_ASSERT(fixedU >= 0 && fixedU < AM_GRADIENTS_CONICAL_TEXTURE_WIDTH);
		AM_ASSERT(fixedV >= 0 && fixedV < AM_GRADIENTS_CONICAL_TEXTURE_WIDTH);
		AM_GRADIENTS_CONICAL_PIXEL_GET(ScaSa.value, fixedU, fixedV)
		// get the pixel on the buffer, if blendMode is different than Src
		DcaDa.value = scrPixels[j];
		// apply blend equation
		invSa = ScaSa.c.a ^ 0xFF;
		invDa = DcaDa.c.a ^ 0xFF;
		t0.value = amPxlScl255(invDa, ScaSa.value);      // Sca.(1 - Da)
		t1.value = amPxlScl255(invSa, DcaDa.value);      // Dca.(1 - Sa)
		t2.value = amPxlMul(ScaSa.value, DcaDa.value); // Sca.Dca
		t3 = DcaDa;
		t4 = ScaSa;
		t5.value = t0.value + t1.value;

		B = (AMint32)t3.c.b * 2 - t3.c.a;
		B = (B < 0) ? (AMint32)t2.c.b * 2 + t5.c.b : ((((AMint32)t4.c.b * t3.c.a + (AMint32)t3.c.b * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.b * 2 + t5.c.b;
		G = (AMint32)t3.c.g * 2 - t3.c.a;
		G = (G < 0) ? (AMint32)t2.c.g * 2 + t5.c.g : ((((AMint32)t4.c.g * t3.c.a + (AMint32)t3.c.g * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.g * 2 + t5.c.g;
		R = (AMint32)t3.c.r * 2 - t3.c.a;
		R = (R < 0) ? (AMint32)t2.c.r * 2 + t5.c.r : ((((AMint32)t4.c.r * t3.c.a + (AMint32)t3.c.r * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.r * 2 + t5.c.r;
		tmp.c.a = t4.c.a + t1.c.a;
		tmp.c.b = AM_CLAMP(B, 0, 255);
		tmp.c.g = AM_CLAMP(G, 0, 255);
		tmp.c.r = AM_CLAMP(R, 0, 255);

		// apply coverage
		invCov = 256 - tmpCov;
		if (invCov == 0) {
			// write pixel
			scrPixels[j++] = tmp.value;
			if (--i != 0)
				goto loop_ConGradOverlay;
			else {
				covLine[j] = 0;
				return;
			}
		}
		else {
			// write pixel
			scrPixels[j++] = amPxlLerp((AMuint32)tmpCov, DcaDa.value, tmp.value);
			if (--i != 0)
				goto loop_ConGradOverlay;
			else {
				covLine[j] = 0;
				return;
			}
		}
	}
	j++;
	if (--i != 0)
		goto loop_ConGradOverlay;
	else 
		covLine[j] = 0;
}

#endif
/*!
	\brief Filler: paint type VG_PAINT_TYPE_COLOR, blend mode VG_BLEND_OVERLAY_MZT, masking on ( \b generated \b function ).
	\param _surface pointer to a AMDrawingSurface structure, defining the destination drawing surface.
	\param _paintGen pointer to a AMPaintGen structure, containing paint information and derived values.
	\param y y coordinate of the scanline to fill.
	\param x0 x coordinate of the left-most pixel to fill.
	\param x1 x coordinate of the right-most pixel to fill.
	\note coverage line deltas are cleared (set to 0) during the loop, if needed.
*/
void amFilPath_ColorOverlayMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1) {

	AMDrawingSurface *surface = (AMDrawingSurface *)_surface;
	AMPaintGen *paintGen = (AMPaintGen *)_paintGen;
	AMuint32 *scrPixels;
	AMint32 *covLine;
	AMuint32 i, ofs0;
	AMuint32 j;
	AMint32 cov;
	AMPixel32 DcaDa, ScaSa;
	AMPixel32 tmp;
	AMint32 invCov;
	AMint32 tmpCov;
	AMuint32 invSa;
	AMuint32 invDa;
	AMPixel32 t0, t1, t2, t3, t4, t5;
	AMint32 R, G, B;
	AMuint8 *alphaPixels;
	AMuint32 mask;

	AM_ASSERT(surface);
	AM_ASSERT(paintGen);
	AM_ASSERT(paintGen->coverageLineDeltas);
	AM_ASSERT(y >= 0 && y < amSrfHeightGet(surface));
	AM_ASSERT(x0 <= x1);
	AM_ASSERT(x0 >= 0 && x0 < amSrfWidthGet(surface));
	AM_ASSERT(x1 >= 0 && x1 < amSrfWidthGet(surface));

	ofs0 = (amSrfHeightGet(surface) - y - 1) * amSrfWidthGet(surface) + x0;
	// go to the start screen pixel
	scrPixels = amSrfPixelsGet(surface) + ofs0;
	// extract coverage line
	covLine = paintGen->coverageLineDeltas + x0;
	// go to the start alpha pixel
	alphaPixels = surface->alphaMaskPixels + ofs0;
	ScaSa.value = paintGen->paintColor32;
	invSa = ScaSa.c.a ^ 0xFF;  // it's equal to 255 - Sa
	j = 0;
	i = x1 - x0 + 1;
	cov = 0;

loop_ColorOverlayMask:
	// update paint generators
	cov += covLine[j];
	if (covLine[j] != 0)
		covLine[j] = 0;
	mask = alphaPixels[j];
	tmpCov = ((cov >> AM_RAS_COVERAGE_PRECISION) * mask) >> 8;
	if (tmpCov != 0) {
		// get the pixel on the buffer, if blendMode is different than Src
		DcaDa.value = scrPixels[j];
		// apply blend equation
		invDa = DcaDa.c.a ^ 0xFF;
		t0.value = amPxlScl255(invDa, ScaSa.value);      // Sca.(1 - Da)
		t1.value = amPxlScl255(invSa, DcaDa.value);      // Dca.(1 - Sa)
		t2.value = amPxlMul(ScaSa.value, DcaDa.value); // Sca.Dca
		t3 = DcaDa;
		t4 = ScaSa;
		t5.value = t0.value + t1.value;

		B = (AMint32)t3.c.b * 2 - t3.c.a;
		B = (B < 0) ? (AMint32)t2.c.b * 2 + t5.c.b : ((((AMint32)t4.c.b * t3.c.a + (AMint32)t3.c.b * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.b * 2 + t5.c.b;
		G = (AMint32)t3.c.g * 2 - t3.c.a;
		G = (G < 0) ? (AMint32)t2.c.g * 2 + t5.c.g : ((((AMint32)t4.c.g * t3.c.a + (AMint32)t3.c.g * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.g * 2 + t5.c.g;
		R = (AMint32)t3.c.r * 2 - t3.c.a;
		R = (R < 0) ? (AMint32)t2.c.r * 2 + t5.c.r : ((((AMint32)t4.c.r * t3.c.a + (AMint32)t3.c.r * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.r * 2 + t5.c.r;
		tmp.c.a = t4.c.a + t1.c.a;
		tmp.c.b = AM_CLAMP(B, 0, 255);
		tmp.c.g = AM_CLAMP(G, 0, 255);
		tmp.c.r = AM_CLAMP(R, 0, 255);

		// apply coverage
		invCov = 256 - tmpCov;
		if (invCov == 0) {
			// write pixel
			scrPixels[j++] = tmp.value;
			if (--i != 0)
				goto loop_ColorOverlayMask;
			else {
				covLine[j] = 0;
				return;
			}
		}
		else {
			// write pixel
			scrPixels[j++] = amPxlLerp((AMuint32)tmpCov, DcaDa.value, tmp.value);
			if (--i != 0)
				goto loop_ColorOverlayMask;
			else {
				covLine[j] = 0;
				return;
			}
		}
	}
	j++;
	if (--i != 0)
		goto loop_ColorOverlayMask;
	else 
		covLine[j] = 0;
}

/*!
	\brief Filler: paint type VG_PAINT_TYPE_LINEAR_GRADIENT, blend mode VG_BLEND_OVERLAY_MZT, masking on ( \b generated \b function ).
	\param _surface pointer to a AMDrawingSurface structure, defining the destination drawing surface.
	\param _paintGen pointer to a AMPaintGen structure, containing paint information and derived values.
	\param y y coordinate of the scanline to fill.
	\param x0 x coordinate of the left-most pixel to fill.
	\param x1 x coordinate of the right-most pixel to fill.
	\note coverage line deltas are cleared (set to 0) during the loop, if needed.
*/
void amFilPath_LinGradOverlayMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1) {

	AMDrawingSurface *surface = (AMDrawingSurface *)_surface;
	AMPaintGen *paintGen = (AMPaintGen *)_paintGen;
	AMuint32 *scrPixels;
	AMint32 *covLine;
	AMuint32 i, ofs0;
	AMuint32 j;
	AMint32 cov;
	AMPixel32 DcaDa, ScaSa;
	AMPixel32 tmp;
	AMint32 invCov;
	AMint32 tmpCov;
	AMuint32 invSa;
	AMuint32 invDa;
	AMPixel32 t0, t1, t2, t3, t4, t5;
	AMint32 R, G, B;
	AMint32 fixedU;
	const AMuint32 *gradientPixels;
	VGColorRampSpreadMode spreadMode;
	AMuint8 *alphaPixels;
	AMuint32 mask;

	AM_ASSERT(surface);
	AM_ASSERT(paintGen);
	AM_ASSERT(paintGen->coverageLineDeltas);
	AM_ASSERT(y >= 0 && y < amSrfHeightGet(surface));
	AM_ASSERT(x0 <= x1);
	AM_ASSERT(x0 >= 0 && x0 < amSrfWidthGet(surface));
	AM_ASSERT(x1 >= 0 && x1 < amSrfWidthGet(surface));

	ofs0 = (amSrfHeightGet(surface) - y - 1) * amSrfWidthGet(surface) + x0;
	// go to the start screen pixel
	scrPixels = amSrfPixelsGet(surface) + ofs0;
	// extract coverage line
	covLine = paintGen->coverageLineDeltas + x0;
	// go to the start alpha pixel
	alphaPixels = surface->alphaMaskPixels + ofs0;
	// initialize the paint generation
	amPaintLinGradInit(paintGen, x0 - 1, y);
	spreadMode = paintGen->paintDesc->paint->colorRampSpreadMode;
	gradientPixels = paintGen->gradTexture;
	j = 0;
	i = x1 - x0 + 1;
	cov = 0;

loop_LinGradOverlayMask:
	// update paint generators
	paintGen->Ux += paintGen->DUx;
	cov += covLine[j];
	if (covLine[j] != 0)
		covLine[j] = 0;
	mask = alphaPixels[j];
	tmpCov = ((cov >> AM_RAS_COVERAGE_PRECISION) * mask) >> 8;
	if (tmpCov != 0) {
		fixedU = paintGen->Ux;
		AM_GRADIENTS_LINEAR_PIXEL_GET(ScaSa.value, fixedU)
		// get the pixel on the buffer, if blendMode is different than Src
		DcaDa.value = scrPixels[j];
		// apply blend equation
		invSa = ScaSa.c.a ^ 0xFF;
		invDa = DcaDa.c.a ^ 0xFF;
		t0.value = amPxlScl255(invDa, ScaSa.value);      // Sca.(1 - Da)
		t1.value = amPxlScl255(invSa, DcaDa.value);      // Dca.(1 - Sa)
		t2.value = amPxlMul(ScaSa.value, DcaDa.value); // Sca.Dca
		t3 = DcaDa;
		t4 = ScaSa;
		t5.value = t0.value + t1.value;

		B = (AMint32)t3.c.b * 2 - t3.c.a;
		B = (B < 0) ? (AMint32)t2.c.b * 2 + t5.c.b : ((((AMint32)t4.c.b * t3.c.a + (AMint32)t3.c.b * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.b * 2 + t5.c.b;
		G = (AMint32)t3.c.g * 2 - t3.c.a;
		G = (G < 0) ? (AMint32)t2.c.g * 2 + t5.c.g : ((((AMint32)t4.c.g * t3.c.a + (AMint32)t3.c.g * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.g * 2 + t5.c.g;
		R = (AMint32)t3.c.r * 2 - t3.c.a;
		R = (R < 0) ? (AMint32)t2.c.r * 2 + t5.c.r : ((((AMint32)t4.c.r * t3.c.a + (AMint32)t3.c.r * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.r * 2 + t5.c.r;
		tmp.c.a = t4.c.a + t1.c.a;
		tmp.c.b = AM_CLAMP(B, 0, 255);
		tmp.c.g = AM_CLAMP(G, 0, 255);
		tmp.c.r = AM_CLAMP(R, 0, 255);

		// apply coverage
		invCov = 256 - tmpCov;
		if (invCov == 0) {
			// write pixel
			scrPixels[j++] = tmp.value;
			if (--i != 0)
				goto loop_LinGradOverlayMask;
			else {
				covLine[j] = 0;
				return;
			}
		}
		else {
			// write pixel
			scrPixels[j++] = amPxlLerp((AMuint32)tmpCov, DcaDa.value, tmp.value);
			if (--i != 0)
				goto loop_LinGradOverlayMask;
			else {
				covLine[j] = 0;
				return;
			}
		}
	}
	j++;
	if (--i != 0)
		goto loop_LinGradOverlayMask;
	else 
		covLine[j] = 0;
}

/*!
	\brief Filler: paint type VG_PAINT_TYPE_RADIAL_GRADIENT, blend mode VG_BLEND_OVERLAY_MZT, masking on ( \b generated \b function ).
	\param _surface pointer to a AMDrawingSurface structure, defining the destination drawing surface.
	\param _paintGen pointer to a AMPaintGen structure, containing paint information and derived values.
	\param y y coordinate of the scanline to fill.
	\param x0 x coordinate of the left-most pixel to fill.
	\param x1 x coordinate of the right-most pixel to fill.
	\note coverage line deltas are cleared (set to 0) during the loop, if needed.
*/
void amFilPath_RadGradOverlayMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1) {

	AMDrawingSurface *surface = (AMDrawingSurface *)_surface;
	AMPaintGen *paintGen = (AMPaintGen *)_paintGen;
	AMuint32 *scrPixels;
	AMint32 *covLine;
	AMuint32 i, ofs0;
	AMuint32 j;
	AMint32 cov;
	AMPixel32 DcaDa, ScaSa;
	AMPixel32 tmp;
	AMint32 invCov;
	AMint32 tmpCov;
	AMuint32 invSa;
	AMuint32 invDa;
	AMPixel32 t0, t1, t2, t3, t4, t5;
	AMint32 R, G, B;
	AMint32 fixedU;
	const AMuint32 *gradientPixels;
	VGColorRampSpreadMode spreadMode;
	AMuint8 *alphaPixels;
	AMuint32 mask;

	AM_ASSERT(surface);
	AM_ASSERT(paintGen);
	AM_ASSERT(paintGen->coverageLineDeltas);
	AM_ASSERT(y >= 0 && y < amSrfHeightGet(surface));
	AM_ASSERT(x0 <= x1);
	AM_ASSERT(x0 >= 0 && x0 < amSrfWidthGet(surface));
	AM_ASSERT(x1 >= 0 && x1 < amSrfWidthGet(surface));

	ofs0 = (amSrfHeightGet(surface) - y - 1) * amSrfWidthGet(surface) + x0;
	// go to the start screen pixel
	scrPixels = amSrfPixelsGet(surface) + ofs0;
	// extract coverage line
	covLine = paintGen->coverageLineDeltas + x0;
	// go to the start alpha pixel
	alphaPixels = surface->alphaMaskPixels + ofs0;
	// initialize the paint generation
	amPaintRadGradInit(paintGen, x0 - 1, y);
	spreadMode = paintGen->paintDesc->paint->colorRampSpreadMode;
	gradientPixels = paintGen->gradTexture;
	j = 0;
	i = x1 - x0 + 1;
	cov = 0;

loop_RadGradOverlayMask:
	// update paint generators
	paintGen->Uf += paintGen->DUf;
	paintGen->Vf += paintGen->DVf;
	paintGen->A += paintGen->dA;
	paintGen->B += paintGen->dB;
	cov += covLine[j];
	if (covLine[j] != 0)
		covLine[j] = 0;
	mask = alphaPixels[j];
	tmpCov = ((cov >> AM_RAS_COVERAGE_PRECISION) * mask) >> 8;
	if (tmpCov != 0) {
		AMfloat s = paintGen->radiusSqr * (paintGen->Uf * paintGen->Uf + paintGen->Vf * paintGen->Vf);
		AMfloat u = paintGen->A + amFastSqrtf(s - (paintGen->B *paintGen->B));
		fixedU = (AMint32)(u * paintGen->K);
		AM_GRADIENTS_RADIAL_PIXEL_GET(ScaSa.value, fixedU)
		// get the pixel on the buffer, if blendMode is different than Src
		DcaDa.value = scrPixels[j];
		// apply blend equation
		invSa = ScaSa.c.a ^ 0xFF;
		invDa = DcaDa.c.a ^ 0xFF;
		t0.value = amPxlScl255(invDa, ScaSa.value);      // Sca.(1 - Da)
		t1.value = amPxlScl255(invSa, DcaDa.value);      // Dca.(1 - Sa)
		t2.value = amPxlMul(ScaSa.value, DcaDa.value); // Sca.Dca
		t3 = DcaDa;
		t4 = ScaSa;
		t5.value = t0.value + t1.value;

		B = (AMint32)t3.c.b * 2 - t3.c.a;
		B = (B < 0) ? (AMint32)t2.c.b * 2 + t5.c.b : ((((AMint32)t4.c.b * t3.c.a + (AMint32)t3.c.b * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.b * 2 + t5.c.b;
		G = (AMint32)t3.c.g * 2 - t3.c.a;
		G = (G < 0) ? (AMint32)t2.c.g * 2 + t5.c.g : ((((AMint32)t4.c.g * t3.c.a + (AMint32)t3.c.g * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.g * 2 + t5.c.g;
		R = (AMint32)t3.c.r * 2 - t3.c.a;
		R = (R < 0) ? (AMint32)t2.c.r * 2 + t5.c.r : ((((AMint32)t4.c.r * t3.c.a + (AMint32)t3.c.r * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.r * 2 + t5.c.r;
		tmp.c.a = t4.c.a + t1.c.a;
		tmp.c.b = AM_CLAMP(B, 0, 255);
		tmp.c.g = AM_CLAMP(G, 0, 255);
		tmp.c.r = AM_CLAMP(R, 0, 255);

		// apply coverage
		invCov = 256 - tmpCov;
		if (invCov == 0) {
			// write pixel
			scrPixels[j++] = tmp.value;
			if (--i != 0)
				goto loop_RadGradOverlayMask;
			else {
				covLine[j] = 0;
				return;
			}
		}
		else {
			// write pixel
			scrPixels[j++] = amPxlLerp((AMuint32)tmpCov, DcaDa.value, tmp.value);
			if (--i != 0)
				goto loop_RadGradOverlayMask;
			else {
				covLine[j] = 0;
				return;
			}
		}
	}
	j++;
	if (--i != 0)
		goto loop_RadGradOverlayMask;
	else 
		covLine[j] = 0;
}

/*!
	\brief Filler: paint type VG_PAINT_TYPE_PATTERN, blend mode VG_BLEND_OVERLAY_MZT, masking on ( \b generated \b function ).
	\param _surface pointer to a AMDrawingSurface structure, defining the destination drawing surface.
	\param _paintGen pointer to a AMPaintGen structure, containing paint information and derived values.
	\param y y coordinate of the scanline to fill.
	\param x0 x coordinate of the left-most pixel to fill.
	\param x1 x coordinate of the right-most pixel to fill.
	\note coverage line deltas are cleared (set to 0) during the loop, if needed.
*/
void amFilPath_PatternOverlayMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1) {

	AMDrawingSurface *surface = (AMDrawingSurface *)_surface;
	AMPaintGen *paintGen = (AMPaintGen *)_paintGen;
	AMuint32 *scrPixels;
	AMint32 *covLine;
	AMuint32 i, ofs0;
	AMuint32 j;
	AMint32 cov;
	AMPixel32 DcaDa, ScaSa;
	AMPixel32 tmp;
	AMint32 invCov;
	AMint32 tmpCov;
	AMuint32 invSa;
	AMuint32 invDa;
	AMPixel32 t0, t1, t2, t3, t4, t5;
	AMint32 R, G, B;
	AMuint8 *alphaPixels;
	AMuint32 mask;

	AM_ASSERT(surface);
	AM_ASSERT(paintGen);
	AM_ASSERT(paintGen->coverageLineDeltas);
	AM_ASSERT(y >= 0 && y < amSrfHeightGet(surface));
	AM_ASSERT(x0 <= x1);
	AM_ASSERT(x0 >= 0 && x0 < amSrfWidthGet(surface));
	AM_ASSERT(x1 >= 0 && x1 < amSrfWidthGet(surface));

	ofs0 = (amSrfHeightGet(surface) - y - 1) * amSrfWidthGet(surface) + x0;
	// go to the start screen pixel
	scrPixels = amSrfPixelsGet(surface) + ofs0;
	// extract coverage line
	covLine = paintGen->coverageLineDeltas + x0;
	// go to the start alpha pixel
	alphaPixels = surface->alphaMaskPixels + ofs0;
	// initialize the paint generation
	amPaintPatternInit(paintGen, x0 - 1, y);
	j = 0;
	i = x1 - x0 + 1;
	cov = 0;

loop_PatternOverlayMask:
	// update paint generators
	paintGen->Ux += paintGen->DUx;
	paintGen->Vx += paintGen->DVx;
	cov += covLine[j];
	if (covLine[j] != 0)
		covLine[j] = 0;
	mask = alphaPixels[j];
	tmpCov = ((cov >> AM_RAS_COVERAGE_PRECISION) * mask) >> 8;
	if (tmpCov != 0) {
		paintGen->patternSamplerParams.x = paintGen->Ux;
		paintGen->patternSamplerParams.y = paintGen->Vx;
		ScaSa.value = paintGen->patternSampler(&paintGen->patternSamplerParams);
		// get the pixel on the buffer, if blendMode is different than Src
		DcaDa.value = scrPixels[j];
		// apply blend equation
		invSa = ScaSa.c.a ^ 0xFF;
		invDa = DcaDa.c.a ^ 0xFF;
		t0.value = amPxlScl255(invDa, ScaSa.value);      // Sca.(1 - Da)
		t1.value = amPxlScl255(invSa, DcaDa.value);      // Dca.(1 - Sa)
		t2.value = amPxlMul(ScaSa.value, DcaDa.value); // Sca.Dca
		t3 = DcaDa;
		t4 = ScaSa;
		t5.value = t0.value + t1.value;

		B = (AMint32)t3.c.b * 2 - t3.c.a;
		B = (B < 0) ? (AMint32)t2.c.b * 2 + t5.c.b : ((((AMint32)t4.c.b * t3.c.a + (AMint32)t3.c.b * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.b * 2 + t5.c.b;
		G = (AMint32)t3.c.g * 2 - t3.c.a;
		G = (G < 0) ? (AMint32)t2.c.g * 2 + t5.c.g : ((((AMint32)t4.c.g * t3.c.a + (AMint32)t3.c.g * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.g * 2 + t5.c.g;
		R = (AMint32)t3.c.r * 2 - t3.c.a;
		R = (R < 0) ? (AMint32)t2.c.r * 2 + t5.c.r : ((((AMint32)t4.c.r * t3.c.a + (AMint32)t3.c.r * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.r * 2 + t5.c.r;
		tmp.c.a = t4.c.a + t1.c.a;
		tmp.c.b = AM_CLAMP(B, 0, 255);
		tmp.c.g = AM_CLAMP(G, 0, 255);
		tmp.c.r = AM_CLAMP(R, 0, 255);

		// apply coverage
		invCov = 256 - tmpCov;
		if (invCov == 0) {
			// write pixel
			scrPixels[j++] = tmp.value;
			if (--i != 0)
				goto loop_PatternOverlayMask;
			else {
				covLine[j] = 0;
				return;
			}
		}
		else {
			// write pixel
			scrPixels[j++] = amPxlLerp((AMuint32)tmpCov, DcaDa.value, tmp.value);
			if (--i != 0)
				goto loop_PatternOverlayMask;
			else {
				covLine[j] = 0;
				return;
			}
		}
	}
	j++;
	if (--i != 0)
		goto loop_PatternOverlayMask;
	else 
		covLine[j] = 0;
}

#if defined(VG_MZT_conical_gradient)
/*!
	\brief Filler: paint type VG_PAINT_TYPE_CONICAL_GRADIENT_MZT, blend mode VG_BLEND_OVERLAY_MZT, masking on ( \b generated \b function ).
	\param _surface pointer to a AMDrawingSurface structure, defining the destination drawing surface.
	\param _paintGen pointer to a AMPaintGen structure, containing paint information and derived values.
	\param y y coordinate of the scanline to fill.
	\param x0 x coordinate of the left-most pixel to fill.
	\param x1 x coordinate of the right-most pixel to fill.
	\note coverage line deltas are cleared (set to 0) during the loop, if needed.
*/
void amFilPath_ConGradOverlayMask(void *_surface, void *_paintGen, AMint32 y, AMint32 x0, AMint32 x1) {

	AMDrawingSurface *surface = (AMDrawingSurface *)_surface;
	AMPaintGen *paintGen = (AMPaintGen *)_paintGen;
	AMuint32 *scrPixels;
	AMint32 *covLine;
	AMuint32 i, ofs0;
	AMuint32 j;
	AMint32 cov;
	AMPixel32 DcaDa, ScaSa;
	AMPixel32 tmp;
	AMint32 invCov;
	AMint32 tmpCov;
	AMuint32 invSa;
	AMuint32 invDa;
	AMPixel32 t0, t1, t2, t3, t4, t5;
	AMint32 R, G, B;
	AMint32 fixedU;
	AMint32 fixedV;
	const AMuint32 *gradientPixels;
	VGColorRampSpreadMode spreadMode;
	AMuint8 *alphaPixels;
	AMuint32 mask;

	AM_ASSERT(surface);
	AM_ASSERT(paintGen);
	AM_ASSERT(paintGen->coverageLineDeltas);
	AM_ASSERT(y >= 0 && y < amSrfHeightGet(surface));
	AM_ASSERT(x0 <= x1);
	AM_ASSERT(x0 >= 0 && x0 < amSrfWidthGet(surface));
	AM_ASSERT(x1 >= 0 && x1 < amSrfWidthGet(surface));

	ofs0 = (amSrfHeightGet(surface) - y - 1) * amSrfWidthGet(surface) + x0;
	// go to the start screen pixel
	scrPixels = amSrfPixelsGet(surface) + ofs0;
	// extract coverage line
	covLine = paintGen->coverageLineDeltas + x0;
	// go to the start alpha pixel
	alphaPixels = surface->alphaMaskPixels + ofs0;
	// initialize the paint generation
	amPaintConGradInit(paintGen, x0 - 1, y);
	spreadMode = paintGen->paintDesc->paint->colorRampSpreadMode;
	gradientPixels = paintGen->gradTexture;
	j = 0;
	i = x1 - x0 + 1;
	cov = 0;

loop_ConGradOverlayMask:
	// update paint generators
	paintGen->Uf += paintGen->DUf;
	paintGen->Vf += paintGen->DVf;
	cov += covLine[j];
	if (covLine[j] != 0)
		covLine[j] = 0;
	mask = alphaPixels[j];
	tmpCov = ((cov >> AM_RAS_COVERAGE_PRECISION) * mask) >> 8;
	if (tmpCov != 0) {
		AMfloat x = paintGen->Uf;
		AMfloat y = paintGen->Vf;
		AMfloat l = x * x + y * y;
		AMuint32 atan2Val;
		AMuint32 atan2U;
		if (l != 0) {
			AMfloat xhalf = 0.5f * l;
			AMint32 f = *(AMint32 *)&l;
			f = 0x5f3759df - (f >> 1);
			l = *(AMfloat*)&f;
			l = l * (1.5f - xhalf * l * l);
			x *= l;
			y *= l;
		}
		x = x * AM_ATAN2_TABLE_SIZE_K + AM_ATAN2_TABLE_SIZE_K;
		y = y * AM_ATAN2_TABLE_SIZE_K + AM_ATAN2_TABLE_SIZE_K;
		fixedU = (AMint32)x;
		fixedV = (AMint32)y;
		AM_ASSERT(fixedU >= 0 && fixedU < AM_GRADIENTS_CONICAL_TEXTURE_WIDTH);
		AM_ASSERT(fixedV >= 0 && fixedV < AM_GRADIENTS_CONICAL_TEXTURE_WIDTH);
		AM_GRADIENTS_CONICAL_PIXEL_GET(ScaSa.value, fixedU, fixedV)
		// get the pixel on the buffer, if blendMode is different than Src
		DcaDa.value = scrPixels[j];
		// apply blend equation
		invSa = ScaSa.c.a ^ 0xFF;
		invDa = DcaDa.c.a ^ 0xFF;
		t0.value = amPxlScl255(invDa, ScaSa.value);      // Sca.(1 - Da)
		t1.value = amPxlScl255(invSa, DcaDa.value);      // Dca.(1 - Sa)
		t2.value = amPxlMul(ScaSa.value, DcaDa.value); // Sca.Dca
		t3 = DcaDa;
		t4 = ScaSa;
		t5.value = t0.value + t1.value;

		B = (AMint32)t3.c.b * 2 - t3.c.a;
		B = (B < 0) ? (AMint32)t2.c.b * 2 + t5.c.b : ((((AMint32)t4.c.b * t3.c.a + (AMint32)t3.c.b * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.b * 2 + t5.c.b;
		G = (AMint32)t3.c.g * 2 - t3.c.a;
		G = (G < 0) ? (AMint32)t2.c.g * 2 + t5.c.g : ((((AMint32)t4.c.g * t3.c.a + (AMint32)t3.c.g * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.g * 2 + t5.c.g;
		R = (AMint32)t3.c.r * 2 - t3.c.a;
		R = (R < 0) ? (AMint32)t2.c.r * 2 + t5.c.r : ((((AMint32)t4.c.r * t3.c.a + (AMint32)t3.c.r * t4.c.a) * 2) >> 8) - t2.c.a - t2.c.r * 2 + t5.c.r;
		tmp.c.a = t4.c.a + t1.c.a;
		tmp.c.b = AM_CLAMP(B, 0, 255);
		tmp.c.g = AM_CLAMP(G, 0, 255);
		tmp.c.r = AM_CLAMP(R, 0, 255);

		// apply coverage
		invCov = 256 - tmpCov;
		if (invCov == 0) {
			// write pixel
			scrPixels[j++] = tmp.value;
			if (--i != 0)
				goto loop_ConGradOverlayMask;
			else {
				covLine[j] = 0;
				return;
			}
		}
		else {
			// write pixel
			scrPixels[j++] = amPxlLerp((AMuint32)tmpCov, DcaDa.value, tmp.value);
			if (--i != 0)
				goto loop_ConGradOverlayMask;
			else {
				covLine[j] = 0;
				return;
			}
		}
	}
	j++;
	if (--i != 0)
		goto loop_ConGradOverlayMask;
	else 
		covLine[j] = 0;
}

#endif
#endif
#endif

#if defined (RIM_VG_SRC)
#pragma pop
#endif

