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
	\file vgext.c
	\brief OpenVG extensions, implementation.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "vg_priv.h"
#include "vgmask.h"
#if defined(AM_GLE) || defined(AM_GLS)
	#include "glcontext.h"
	#include "gl_abstraction.h"
#endif

#if defined RIM_VG_SRC
    #include <KHR_thread_storage.h>
    #include "egl.h"
    #include "i_egl.h"

    #include "egl_amanith.h"
#endif

#if defined(AM_OS_WIN) || defined(AM_OS_WINCE)
	#include <Windows.h>
	// external DLL hinstance
	AM_EXTERN_C HINSTANCE dllInstance;
#endif

// support of _T function for unicode strings
#if !defined(_T)
	#if !defined(__TEXT)
		#define __TEXT(quote) L##quote
	#endif
	#define _T(x) __TEXT(x)
#endif

//! Maximum character length of a line in the external configuration file.
#define AM_CFG_FILE_MAX_LINE_LENGTH 256

void amSrfDynResourcesInit(AMDrawingSurface *surface) {

	AM_ASSERT(surface);

	AM_DYNARRAY_PREINIT(surface->clippedScissorRects)
	AM_DYNARRAY_PREINIT(surface->scissorEvents)
	AM_DYNARRAY_PREINIT(surface->curSpanList)
	AM_DYNARRAY_PREINIT(surface->mergedSpanList)
	AM_DYNARRAY_PREINIT(surface->curSpanEvents)
#if defined(AM_SRE)
	AM_DYNARRAY_PREINIT(surface->dirtyRegions)
	AM_DYNARRAY_PREINIT(surface->splitDirtyRegions)
#endif
}

void amSrfDynResourcesDestroy(AMDrawingSurface *surface) {

	AM_ASSERT(surface);

	AM_DYNARRAY_DESTROY(surface->clippedScissorRects)
	AM_DYNARRAY_DESTROY(surface->scissorEvents)
	AM_DYNARRAY_DESTROY(surface->curSpanList)
	AM_DYNARRAY_DESTROY(surface->mergedSpanList)
	AM_DYNARRAY_DESTROY(surface->curSpanEvents)
#if defined(AM_SRE)
	AM_DYNARRAY_DESTROY(surface->dirtyRegions)
	AM_DYNARRAY_DESTROY(surface->splitDirtyRegions);
#endif
}

AMbool amSrfInitByAddr(AMDrawingSurface *surface,
					   void *pixels,
					   void *alphaMaskPixels,
					   const AMint32 width,
					   const AMint32 height,
					   const AMbool linearColorSpace) {

	AM_ASSERT(surface);
	AM_ASSERT(width > 0);
	AM_ASSERT(height > 0);
 
	if (surface->initialized)
		return AM_TRUE;

	amSrfDynResourcesInit(surface);
	// initialize auxiliary arrays used to do rectangles decomposition
	AM_DYNARRAY_INIT(surface->clippedScissorRects, AMScissorRect)
	if (surface->clippedScissorRects.error) {
		amSrfDynResourcesDestroy(surface);
		return AM_FALSE;
	}
	AM_DYNARRAY_INIT(surface->scissorEvents, AMScissorEvent)
	if (surface->scissorEvents.error) {
		amSrfDynResourcesDestroy(surface);
		return AM_FALSE;
	}
	AM_DYNARRAY_INIT(surface->curSpanList, AMScissorYSpan)
	if (surface->curSpanList.error) {
		amSrfDynResourcesDestroy(surface);
		return AM_FALSE;
	}
	AM_DYNARRAY_INIT(surface->mergedSpanList, AMScissorYMergedSpan)
	if (surface->mergedSpanList.error) {
		amSrfDynResourcesDestroy(surface);
		return AM_FALSE;
	}
	AM_DYNARRAY_INIT(surface->curSpanEvents, AMScissorYSpanEvent)
	if (surface->curSpanEvents.error) {
		amSrfDynResourcesDestroy(surface);
		return AM_FALSE;
	}

#if defined(AM_SRE)
	AM_ASSERT(pixels);
	#if defined(AM_LITE_PROFILE)
		(void)linearColorSpace;
		surface->pixels = (AMuint16 *)pixels;
		// drawing surface is 16bit
		#if defined(AM_SURFACE_BYTE_ORDER_ARGB) || defined(AM_SURFACE_BYTE_ORDER_RGBA)
			surface->realFormat = VG_sRGB_565;
		#else
			surface->realFormat = VG_sBGR_565;
		#endif
		surface->dataStride = width * sizeof(AMuint16);
	#else
		surface->pixels = (AMuint32 *)pixels;
		// drawing surface is 32bit
		#if defined(AM_SURFACE_BYTE_ORDER_RGBA)
			surface->realFormat = (linearColorSpace) ? VG_lRGBA_8888_PRE : VG_sRGBA_8888_PRE;
		#elif defined(AM_SURFACE_BYTE_ORDER_ARGB)
			surface->realFormat = (linearColorSpace) ? VG_lARGB_8888_PRE : VG_sARGB_8888_PRE;
		#elif defined(AM_SURFACE_BYTE_ORDER_BGRA)
			surface->realFormat = (linearColorSpace) ? VG_lBGRA_8888_PRE : VG_sBGRA_8888_PRE;
		#else
			surface->realFormat = (linearColorSpace) ? VG_lABGR_8888_PRE : VG_sABGR_8888_PRE;
		#endif
		surface->dataStride = width * sizeof(AMuint32);
	#endif

	// initialize dirty regions arrays
	AM_DYNARRAY_INIT(surface->dirtyRegions, AMint32)
	if (surface->dirtyRegions.error) {
		amSrfDynResourcesDestroy(surface);
		return AM_FALSE;
	}
	AM_DYNARRAY_INIT(surface->splitDirtyRegions, AMScissorRect)
	if (surface->splitDirtyRegions.error) {
		amSrfDynResourcesDestroy(surface);
		return AM_FALSE;
	}
	surface->wholeCleared = AM_FALSE;
	surface->dirtyRegionsOverflow = AM_FALSE;
	surface->clearColor = 0;
#elif defined(AM_GLE) || defined(AM_GLS)
	(void)pixels;
	surface->pixels = NULL;
	// drawing surface is 32bit
	#if defined(AM_SURFACE_BYTE_ORDER_RGBA)
		surface->realFormat = (linearColorSpace) ? VG_lRGBA_8888_PRE : VG_sRGBA_8888_PRE;
	#elif defined(AM_SURFACE_BYTE_ORDER_ARGB)
		surface->realFormat = (linearColorSpace) ? VG_lARGB_8888_PRE : VG_sARGB_8888_PRE;
	#elif defined(AM_SURFACE_BYTE_ORDER_BGRA)
		surface->realFormat = (linearColorSpace) ? VG_lBGRA_8888_PRE : VG_sBGRA_8888_PRE;
	#else
		surface->realFormat = (linearColorSpace) ? VG_lABGR_8888_PRE : VG_sABGR_8888_PRE;
	#endif
	surface->dataStride = width * sizeof(AMuint32);

	// initialize alpha mask textures and flags
	amTextureInit(&surface->alphaMaskTexture);
	amTextureInit(&surface->invAlphaMaskTexture);
	surface->alphaMaskTextureValid = AM_FALSE;
	surface->invAlphaMaskTextureValid = AM_FALSE;
#else
	#error Undefined AmanithVG engine type.
#endif

	surface->alphaMaskPixels = alphaMaskPixels;
	surface->width = width;
	surface->height = height;
	surface->initialized = AM_TRUE;
	surface->initializedByAddr = AM_TRUE;
	return AM_TRUE;
}

AMbool amSrfInit(AMDrawingSurface *surface,
				 const AMint32 width,
				 const AMint32 height,
				 const AMbool linearColorSpace,
				 const AMbool alphaMask) {

#if defined(AM_SRE) && defined(AM_LITE_PROFILE)
	AMuint16 *pixels = NULL;
#else
	AMuint32 *pixels = NULL;
#endif
	AMuint8 *alphaMaskPixels = NULL;

	AM_ASSERT(surface);
	AM_ASSERT(width > 0);
	AM_ASSERT(height > 0);

	if (surface->initialized)
		return AM_TRUE;

#if defined(RIM_VG_SRC)
    if ( surface->isPixmapSurface ) {
        pixels = surface->pixels;
    } else {
#endif
#if defined(AM_SRE)
	#if defined(AM_LITE_PROFILE)
		pixels = (AMuint16 *)amMalloc(width * height * sizeof(AMuint16));
	#else
		pixels = (AMuint32 *)amMalloc(width * height * sizeof(AMuint32));
	#endif
	if (!pixels)
		return AM_FALSE;
#endif
#if defined(RIM_VG_SRC)
    }
#endif    
	// allocates alphaMask, if needed
	if (alphaMask) {
		alphaMaskPixels = (AMuint8 *)amMalloc(width * height * sizeof(AMuint8));
		if (!alphaMaskPixels) {
#if defined(RIM_VG_SRC)
            if (pixels && !surface->isPixmapSurface) 
#else
            if (pixels) 
#endif
				// free memory for pixels
				amFree(pixels);
			return AM_FALSE;
		}
	}
    
	if (!amSrfInitByAddr(surface, pixels, alphaMaskPixels, width, height, linearColorSpace)) {
		// free pixels and alphaMaskPixels
#if defined(RIM_VG_SRC)
		if (pixels && !surface->isPixmapSurface) 
#else
		if (pixels) 
#endif
			amFree(pixels);
		if (alphaMaskPixels)
			amFree(alphaMaskPixels);
		return AM_FALSE;
	}
	else {
		surface->initializedByAddr = AM_FALSE;
		return AM_TRUE;
	}
}

void amSrfDestroy(AMDrawingSurface *surface) {

	AM_ASSERT(surface);

	if (!surface->initialized)
		return;

	amSrfDynResourcesDestroy(surface);

#if defined(AM_SRE)
#if defined(RIM_VG_SRC)
	if (surface->pixels && !surface->initializedByAddr && !surface->isPixmapSurface)
#else
	if (surface->pixels && !surface->initializedByAddr)
#endif
		amFree(surface->pixels);
	surface->pixels = NULL;
	surface->wholeCleared = AM_FALSE;
	surface->dirtyRegionsOverflow = AM_FALSE;
	surface->clearColor = 0;
#elif defined(AM_GLE) || defined(AM_GLS)
	surface->alphaMaskTextureValid = AM_FALSE;
	surface->invAlphaMaskTextureValid = AM_FALSE;
	amTextureDestroy(&surface->alphaMaskTexture);
	amTextureDestroy(&surface->invAlphaMaskTexture);
#else
	#error Undefined AmanithVG engine type.
#endif

	surface->width = 0;
	surface->height = 0;
	surface->realFormat = (AMuint32)VG_IMAGE_FORMAT_INVALID;
	surface->dataStride = 0;
	if (surface->alphaMaskPixels && !surface->initializedByAddr)
		amFree(surface->alphaMaskPixels);
	surface->alphaMaskPixels = NULL;
	surface->initialized = AM_FALSE;
	surface->initializedByAddr = AM_FALSE;
}

void amSrfResizeByAddr(AMDrawingSurface *surface,
					   void *pixels,
					   void *alphaMaskPixels,
					   const AMint32 newWidth,
					   const AMint32 newHeight) {

#if defined(AM_SRE) && defined(AM_LITE_PROFILE)
	#define PIXEL_TYPE AMuint16
#else
	#define PIXEL_TYPE AMuint32
#endif

	AM_ASSERT(surface && surface->initialized);
	AM_ASSERT(newWidth > 0);
	AM_ASSERT(newHeight > 0);

	surface->width = newWidth;
	surface->height = newHeight;
	surface->dataStride = newWidth * sizeof(PIXEL_TYPE);
	surface->pixels = pixels;
	surface->alphaMaskPixels = alphaMaskPixels;

#if defined(AM_SRE)
	AM_ASSERT(pixels);
	surface->wholeCleared = AM_FALSE;
	surface->dirtyRegionsOverflow = AM_FALSE;
	surface->dirtyRegions.size = 0;
#else
	surface->alphaMaskTextureValid = AM_FALSE;
	surface->invAlphaMaskTextureValid = AM_FALSE;
	amTextureDestroy(&surface->alphaMaskTexture);
	amTextureDestroy(&surface->invAlphaMaskTexture);
#endif

	#undef PIXEL_TYPE
}

/*!
	\brief Realloc memory for pixels (and alpha mask) of a given drawing surface.
	\param surface input surface whose pixels are to be reallocated.
	\param newWidth new surface width, in pixels.
	\param newHeight new surface height, in pixels.
	\return AM_FALSE if a memory allocation error occurred, else AM_TRUE.
	\pre newWidth, newHeight > 0
*/
AMbool amSrfResize(AMDrawingSurface *surface,
				   const AMint32 newWidth,
				   const AMint32 newHeight) {

#if defined(AM_SRE) && defined(AM_LITE_PROFILE)
	#define PIXEL_TYPE AMuint16
#else
	#define PIXEL_TYPE AMuint32
#endif
	PIXEL_TYPE *newPixels;
	AMuint8 *newAlphaMaskPixels;
	AMuint32 n = (AMuint32)newWidth * (AMuint32)newHeight;
	AMbool dimensionsChanged = (amSrfWidthGet(surface) != newWidth || amSrfHeightGet(surface) != newHeight);

	AM_ASSERT(surface && surface->initialized);
	AM_ASSERT(surface->initializedByAddr == AM_FALSE);
	AM_ASSERT(newWidth > 0);
	AM_ASSERT(newHeight > 0);

	if (surface->alphaMaskPixels) {
		// real allocation occurs only when new dimensions are really different than previous ones
		newAlphaMaskPixels = (dimensionsChanged) ? (AMuint8 *)amMalloc(n * sizeof(AMuint8)) : surface->alphaMaskPixels;
		if (!newAlphaMaskPixels)
			return AM_FALSE;
	}
	else
		newAlphaMaskPixels = NULL;

#if defined(AM_SRE)
	// real allocation occurs only when new dimensions are really different than previous ones
	newPixels = (dimensionsChanged) ? (PIXEL_TYPE *)amRealloc(surface->pixels, n * sizeof(PIXEL_TYPE)) : surface->pixels;
	if (!newPixels) {
		if (newAlphaMaskPixels && newAlphaMaskPixels != surface->alphaMaskPixels)
			amFree(newAlphaMaskPixels);
		return AM_FALSE;
	}
	else {
		if (newAlphaMaskPixels) {
			// reset alpha mask pixels (TO DO: copy instead of clear)
			amMemset(newAlphaMaskPixels, 0xFF, n);
			// free "old" alphaMaskPixels
			if (newAlphaMaskPixels != surface->alphaMaskPixels)
				amFree(surface->alphaMaskPixels);
		}
	}
#elif defined(AM_GLE) || defined(AM_GLS)
	newPixels = NULL;
	if (newAlphaMaskPixels) {
		// reset alpha mask pixels (TO DO: copy instead of clear)
		amMemset(newAlphaMaskPixels, 0xFF, n);
		// free "old" alphaMaskPixels
		if (newAlphaMaskPixels != surface->alphaMaskPixels)
			amFree(surface->alphaMaskPixels);
	}
#else
	#error Undefined AmanithVG engine type.
#endif

	amSrfResizeByAddr(surface, newPixels, newAlphaMaskPixels, newWidth, newHeight);
	return AM_TRUE;

	#undef PIXEL_TYPE
}


/*!
	\brief Set default values for configuration parameters.
	\param confParams output configuration parameters structure.
*/
void amCtxConfigParametersDefault(AMContextConfParams *confParams) {

	AMint32 intValue;

	AM_ASSERT(confParams);

	intValue = 100 - AM_CLAMP(AM_FLATTENING_DEFAULT_QUALITY, 0, 100);
	// map the range [0; 100] to [0.01; 1.5]
	confParams->curvesQuality = ((AMfloat)intValue / 100.0f);
	confParams->curvesQuality = confParams->curvesQuality * confParams->curvesQuality;
	confParams->curvesQuality = confParams->curvesQuality * (1.5f - 0.01f) + 0.01f;

#if defined(AM_GLE) || defined(AM_GLS)
	confParams->forceRectTexturesDisabled = AM_FALSE;
	confParams->forceMirroredRepeatDisabled = AM_FALSE;
	confParams->forceClampToBorderDisabled = AM_FALSE;
	confParams->forceBlendMinMaxDisabled = AM_FALSE;
	confParams->forceDot3Disabled = AM_FALSE;
	confParams->forceVBODisabled = AM_FALSE;
	confParams->maxPermittedTextureUnits = 4;
	confParams->maxTextureSize = 0;
	confParams->forceBufferDisabled = AM_NO_BUFFER_DISABLED;
	#if defined(AM_OPENGL)
		confParams->supposePersistentBuffer = AM_TRUE;
	#elif defined(AM_OPENGL_ES)
		confParams->supposePersistentBuffer = AM_FALSE;
	#else
		#error Unreachable point.
	#endif
	confParams->forceScissorDisabled = AM_FALSE;
	confParams->forceColorMaskingDisabled = AM_FALSE;
	confParams->forceMimMapsOnGradients = AM_FALSE;
	confParams->forceDitheringOnGradients = AM_FALSE;
	confParams->forceDitheringOnImages = AM_FALSE;
	confParams->forceRGBATextures = AM_TRUE;
#endif

#if defined(AM_GLE)
	// map the range [0; 100] to [0.01; 1.5]
	intValue = 100 - AM_CLAMP(AM_GLE_RADIAL_GRADIENTS_DEFAULT_QUALITY, 0, 100);
	confParams->radialGradientsQuality = ((AMfloat)intValue / 100.0f);
	confParams->radialGradientsQuality = confParams->radialGradientsQuality * confParams->radialGradientsQuality;
	confParams->radialGradientsQuality = confParams->radialGradientsQuality * (1.5f - 0.01f) + 0.01f;
	// map the range [0; 100] to [0.01; 1.5]
	intValue = 100 - AM_CLAMP(AM_GLE_CONICAL_GRADIENTS_DEFAULT_QUALITY, 0, 100);
	confParams->conicalGradientsQuality = ((AMfloat)intValue / 100.0f);
	confParams->conicalGradientsQuality = confParams->conicalGradientsQuality * confParams->conicalGradientsQuality;
	confParams->conicalGradientsQuality = confParams->conicalGradientsQuality * (1.5f - 0.01f) + 0.01f;
#endif
}

#if defined(AM_CONFIG_FILE) && !defined(AM_OS_BREW)
AMbool amCtxConfigFileParseLine(char *key,
								char *value,
								const char *line) {

	char tmpLine[AM_CFG_FILE_MAX_LINE_LENGTH];
	AMint32 i, j, k;

	AM_ASSERT(line);
	AM_ASSERT(key);
	AM_ASSERT(value);

	amMemset(tmpLine, 0, AM_CFG_FILE_MAX_LINE_LENGTH);

	i = 0;
	j = (AMint32)amStrlen(line);
	if (j > AM_CFG_FILE_MAX_LINE_LENGTH)
		j = AM_CFG_FILE_MAX_LINE_LENGTH;

	// discard initial white spaces
	while ((i < j) && ((line[i] == ' ') || (line[i] == '\t') || (line[i] == '\r')) && (line[i] != '\n'))
		i++;

	// extract key name
	k = 0;
	while ((i < j) && ((line[i] != ' ') && (line[i] != '\t') && (line[i] != '\r')) && (line[i] != '\n')) {
		tmpLine[k++] = (char)amToupper(line[i++]);
	}
	if (k == 0)
		return AM_FALSE;
	if (tmpLine[0] < 'A' || tmpLine[0] > 'Z')
		return AM_FALSE;
	// copy the key
	amStrcpy(key, tmpLine);

	amMemset(tmpLine, 0, AM_CFG_FILE_MAX_LINE_LENGTH);

	// discard intermediate white spaces
	while ((i < j) && ((line[i] == ' ') || (line[i] == '\t') || (line[i] == '\r')) && (line[i] != '\n'))
		i++;

	k = 0;
	while ((i < j) && ((line[i] != ' ') && (line[i] != '\t') && (line[i] != '\r')) && (line[i] != '\n')) {
		tmpLine[k++] = (char)amToupper(line[i++]);
	}
	if (k == 0)
		return AM_FALSE;

	// copy the value and exit
	amStrcpy(value, tmpLine);
	return AM_TRUE;
}

/*!
	\brief Load configuration parameters from an external file.
	\param confParms output configuration parameters structure.
	\param fullFileName name (path included) of the external file to load.
	\return AM_TRUE if the operation was successful, else AM_FALSE.
*/
AMbool amCtxConfigParametersLoadFromFile(AMContextConfParams *confParams,
										 const char *fullFileName) {

	FILE *f;
	char line[AM_CFG_FILE_MAX_LINE_LENGTH];
	char key[AM_CFG_FILE_MAX_LINE_LENGTH];
	char value[AM_CFG_FILE_MAX_LINE_LENGTH];
	AMbool parseOk;

	AM_ASSERT(fullFileName);
	AM_ASSERT(confParams);

	// set defaults
	amCtxConfigParametersDefault(confParams);

	if (!fullFileName)
		// return default configuration
		return AM_FALSE;

	f = fopen(fullFileName, "rt");
	if (!f)
		// return default configuration
		return AM_FALSE;

	while (!feof(f)) {

		amMemset(line, 0, AM_CFG_FILE_MAX_LINE_LENGTH);

		if (fgets(line, AM_CFG_FILE_MAX_LINE_LENGTH - 1, f)) {

			amMemset(key, 0, AM_CFG_FILE_MAX_LINE_LENGTH);
			amMemset(value, 0, AM_CFG_FILE_MAX_LINE_LENGTH);
			parseOk = amCtxConfigFileParseLine(key, value, line);
			// valid line
			if (parseOk && amStrlen(key) > 0 && amStrlen(value) > 0) {

				// [OpenGL] section
			#if defined(AM_GLE) || defined(AM_GLS)
				if (amStrcmp(key, "FORCERECTTEXTURESDISABLED") == 0) {
					if (amStrcmp(value, "TRUE") == 0)
						confParams->forceRectTexturesDisabled = AM_TRUE;
					else
					if (amStrcmp(value, "FALSE") == 0)
						confParams->forceRectTexturesDisabled = AM_FALSE;
				}
				else
				if (amStrcmp(key, "FORCEMIRROREDREPEATDISABLED") == 0) {
					if (amStrcmp(value, "TRUE") == 0)
						confParams->forceMirroredRepeatDisabled = AM_TRUE;
					else
					if (amStrcmp(value, "FALSE") == 0)
						confParams->forceMirroredRepeatDisabled = AM_FALSE;
				}
				else
				if (amStrcmp(key, "FORCECLAMPTOBORDERDISABLED") == 0) {
					if (amStrcmp(value, "TRUE") == 0)
						confParams->forceClampToBorderDisabled = AM_TRUE;
					else
					if (amStrcmp(value, "FALSE") == 0)
						confParams->forceClampToBorderDisabled = AM_FALSE;
				}
				else
				if (amStrcmp(key, "FORCEBLENDMINMAXDISABLED") == 0) {
					if (amStrcmp(value, "TRUE") == 0)
						confParams->forceBlendMinMaxDisabled = AM_TRUE;
					else
					if (amStrcmp(value, "FALSE") == 0)
						confParams->forceBlendMinMaxDisabled = AM_FALSE;
				}
				else
				if (amStrcmp(key, "FORCEDOT3DISABLED") == 0) {
					if (amStrcmp(value, "TRUE") == 0)
						confParams->forceDot3Disabled = AM_TRUE;
					else
					if (amStrcmp(value, "FALSE") == 0)
						confParams->forceDot3Disabled = AM_FALSE;
				}
				else
				if (amStrcmp(key, "FORCEVBODISABLED") == 0) {
					if (amStrcmp(value, "TRUE") == 0)
						confParams->forceVBODisabled = AM_TRUE;
					else
					if (amStrcmp(value, "FALSE") == 0)
						confParams->forceVBODisabled = AM_FALSE;
				}
				else
				if (amStrcmp(key, "MAXPERMITTEDTEXTUREUNITS") == 0) {

					AMint32 intValue = 0;
					char strValue[3];

					amStrncpy(strValue, value, 3);
					strValue[2] = 0;
					intValue = amAtoi(strValue);
					if (intValue > 0)
						confParams->maxPermittedTextureUnits = intValue;
				}
				else
				if (amStrcmp(key, "MAXTEXTURESIZE") == 0) {

					AMint32 intValue = 0;
					char strValue[5];

					amStrncpy(strValue, value, 5);
					strValue[4] = 0;
					intValue = amAtoi(strValue);
					if ((intValue == 64) || (intValue == 128) || (intValue == 256) || (intValue == 512) ||
						(intValue == 1024) || (intValue == 2048) || (intValue == 4096) || (intValue == 8192))
						confParams->maxTextureSize = intValue;
				}
				else
				if (amStrcmp(key, "FORCEBUFFERDISABLED") == 0) {
					if (amStrcmp(value, "STENCIL") == 0) 
						confParams->forceBufferDisabled = AM_STENCIL_BUFFER_DISABLED;
					else
					if (amStrcmp(value, "DEPTH") == 0) 
						confParams->forceBufferDisabled = AM_DEPTH_BUFFER_DISABLED;
					else
						confParams->forceBufferDisabled = AM_NO_BUFFER_DISABLED;
				}
				else
				if (amStrcmp(key, "SUPPOSEPERSISTENTBUFFERS") == 0) {
					if (amStrcmp(value, "TRUE") == 0)
						confParams->supposePersistentBuffer = AM_TRUE;
					else
					if (amStrcmp(value, "FALSE") == 0)
						confParams->supposePersistentBuffer = AM_FALSE;
				}
				else
				if (amStrcmp(key, "FORCESCISSORDISABLED") == 0) {
					if (amStrcmp(value, "TRUE") == 0)
						confParams->forceScissorDisabled = AM_TRUE;
					else
					if (amStrcmp(value, "FALSE") == 0)
						confParams->forceScissorDisabled = AM_FALSE;
				}
				else
				if (amStrcmp(key, "FORCECOLORMASKINGDISABLED") == 0) {
					if (amStrcmp(value, "TRUE") == 0)
						confParams->forceColorMaskingDisabled = AM_TRUE;
					else
					if (amStrcmp(value, "FALSE") == 0)
						confParams->forceColorMaskingDisabled = AM_FALSE;
				}
				else
				if (amStrcmp(key, "FORCEMIMMAPSONGRADIENTS") == 0) {
					if (amStrcmp(value, "TRUE") == 0)
						confParams->forceMimMapsOnGradients = AM_TRUE;
					else
					if (amStrcmp(value, "FALSE") == 0)
						confParams->forceMimMapsOnGradients = AM_FALSE;
				}
				else
				if (amStrcmp(key, "FORCEDITHERINGONGRADIENTS") == 0) {
					if (amStrcmp(value, "TRUE") == 0)
						confParams->forceDitheringOnGradients = AM_TRUE;
					else
					if (amStrcmp(value, "FALSE") == 0)
						confParams->forceDitheringOnGradients = AM_FALSE;
				}
				else
				if (amStrcmp(key, "FORCEDITHERINGONIMAGES") == 0) {
					if (amStrcmp(value, "TRUE") == 0)
						confParams->forceDitheringOnImages = AM_TRUE;
					else
					if (amStrcmp(value, "FALSE") == 0)
						confParams->forceDitheringOnImages = AM_FALSE;
				}
				else
				if (amStrcmp(key, "FORCERGBATEXTURES") == 0) {
					if (amStrcmp(value, "TRUE") == 0)
						confParams->forceRGBATextures = AM_TRUE;
					else
					if (amStrcmp(value, "FALSE") == 0)
						confParams->forceRGBATextures = AM_FALSE;
				}
				else
			#endif
				// [Geometry] section
				if (amStrcmp(key, "CURVESQUALITY") == 0) {

					AMint32 intValue = 0;
					char strValue[4];

					amStrncpy(strValue, value, 4);
					strValue[3] = 0;
					intValue = amAtoi(strValue);
					intValue = 100 - AM_CLAMP(intValue, 0, 100);
					// map the range [0; 100] to [0.01; 1.5]
					confParams->curvesQuality = ((AMfloat)intValue / 100.0f);
					confParams->curvesQuality = confParams->curvesQuality * confParams->curvesQuality;
					confParams->curvesQuality = confParams->curvesQuality * (1.5f - 0.01f) + 0.01f;
				}
			#if defined(AM_GLE)
				else
				if (strcmp(key, "RADIALGRADIENTSQUALITY") == 0) {

					AMint32 intValue = 0;
					char strValue[4];

					amStrncpy(strValue, value, 4);
					strValue[3] = 0;
					intValue = amAtoi(strValue);
					intValue = 100 - AM_CLAMP(intValue, 0, 100);
					// map the range [0; 100] to [0.01; 1.5]
					confParams->radialGradientsQuality = ((AMfloat)intValue / 100.0f);
					confParams->radialGradientsQuality = confParams->radialGradientsQuality * confParams->radialGradientsQuality;
					confParams->radialGradientsQuality = confParams->radialGradientsQuality * (1.5f - 0.01f) + 0.01f;
				}
				else
				if (strcmp(key, "CONICALGRADIENTSQUALITY") == 0) {

					AMint32 intValue = 0;
					char strValue[4];

					amStrncpy(strValue, value, 4);
					strValue[3] = 0;
					intValue = amAtoi(strValue);
					intValue = 100 - AM_CLAMP(intValue, 0, 100);
					// map the range [0; 100] to [0.01; 1.5]
					confParams->conicalGradientsQuality = ((AMfloat)intValue / 100.0f);
					confParams->conicalGradientsQuality = confParams->conicalGradientsQuality * confParams->conicalGradientsQuality;
					confParams->conicalGradientsQuality = confParams->conicalGradientsQuality * (1.5f - 0.01f) + 0.01f;
				}
			#endif
			}
		}
	}
	fclose(f);
	return AM_TRUE;
}
#endif

void amCtxConfigParametersLoad(AMContextConfParams *confParams) {

	AM_ASSERT(confParams);

#if defined(AM_CONFIG_FILE)
	#if defined(AM_OS_WIN) && !defined(AM_MAKE_STATIC_LIBRARY)
		if (dllInstance != 0) {

			AMchar8 fullName[_MAX_PATH], drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];

			amMemset(fullName, 0, _MAX_PATH);
			GetModuleFileName(dllInstance, fullName, _MAX_PATH);
			_splitpath(fullName, drive, dir, fname, ext);
			amStrcpy(fullName, drive);
			amStrcat(fullName, dir);
			amStrcat(fullName, "amanithvg.ini");
			amCtxConfigParametersLoadFromFile(confParams, fullName);
		}
	#elif defined(AM_OS_WIN) && defined(AM_MAKE_STATIC_LIBRARY)
		amCtxConfigParametersLoadFromFile(confParams, "c:\\windows\\amanithvg.ini");
	#elif defined(AM_OS_WINCE)
		amCtxConfigParametersLoadFromFile(confParams, "\\Windows\\amanithvg.ini");
	#elif defined(AM_OS_SYMBIAN)
		amCtxConfigParametersLoadFromFile(confParams, "e:\\openvg\\amanithvg.conf");
	#elif defined(AM_OS_BREW)
		amCtxConfigParametersDefault(confParams);
	#elif defined(AM_OS_UNIXLIKE) || defined(AM_OS_MAC)
		amCtxConfigParametersLoadFromFile(confParams, "/etc/amanithvg.conf");
	#elif defined(AM_OS_AMIGAOS)
		amCtxConfigParametersLoadFromFile(confParams, "/env/amanithvg.conf");
	#else
		amCtxConfigParametersDefault(confParams);
	#endif
#else // without AM_CONFIG_FILE just set default parameters
	amCtxConfigParametersDefault(confParams);
#endif
}

// *********************************************************************
//                        Public implementations
// *********************************************************************

/*!
	\brief Create and initialize an OpenVG context.

	\param _sharedContext a pointer to a previously created context; all shareable data (OpenVG handles) will be shared by _sharedContext, all other contexts
	_sharedContext already shares with, and	the newly created context.
	\return NULL if a memory allocation error occurred, else a valid pointer.
*/
VG_API_CALL void* VG_API_ENTRY vgPrivContextCreateAM(void *_sharedContext) VG_API_EXIT {

#if defined(AM_STANDALONE)
	AMContext *context = (AMContext *)amMalloc(sizeof(AMContext));
	AMContext *sharedContext = (AMContext *)_sharedContext;

	if (!context)
		return NULL;
	
	// load external configuration file options
	amCtxConfigParametersLoad(&context->confParams);

	// initialize the OpenVG context
	context->initialized = AM_FALSE;
	if (!amCtxInit(context, sharedContext, &context->confParams)) {
		amFree(context);
		return NULL;
	}
	return context;
#else
	(void)_sharedContext;
	return NULL;
#endif
}

/*!
	\brief Destroy a previously created OpenVG context.

	\param _context pointer to a (valid) previously created context.
	\note if the specified context is the currently bound one, the function simply exits without doing nothing.
*/
VG_API_CALL void VG_API_ENTRY vgPrivContextDestroyAM(void *_context) VG_API_EXIT {

#if defined(AM_STANDALONE)
	AMContext *context = (AMContext *)_context;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	if (!context || !context->initialized)
		return;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (context != currentContext) {
		// destroy the OpenVG context
		amCtxDestroy(context);
		amFree(context);
	}
#else
	(void)_context;
#endif
}

/*!
	\brief Create and initialize a drawing surface. In the detail this function allocates:
	- a 32bit drawing surface (AmanithVG SRE)
	- a 16bit drawing surface (AmanithVG SRE Lite)
	- an 8bit alphaMask buffer, if alphaMask parameter is VG_TRUE (AmanithVG SRE / SRE Lite / GLE / GLS)

	\param width desired width, in pixels.
	\param height desired height, in pixels.
	\param linearColorSpace VG_TRUE for linear color space, VG_FALSE for sRGB color space. This parameter is
	ignored in AmanithVG SRE Lite, since it	always uses non-linear color space.
	\param alphaMask VG_TRUE if the drawing surface must support/contain OpenVG alpha mask, else VG_FALSE.
	\return NULL if a memory allocation error occurred or if width or height are less than or equal zero, else a valid pointer.
	\note for best performance use non-linear color space.
*/
VG_API_CALL void* VG_API_ENTRY vgPrivSurfaceCreateAM(VGint width,
													 VGint height,
													 VGboolean linearColorSpace,
													 VGboolean alphaMask) VG_API_EXIT {

#if defined(AM_STANDALONE)
	AMDrawingSurface *surface;

	if (width <= 0 || height <= 0)
		return NULL;

	if (width > AM_SURFACE_MAX_DIMENSION)
		width = AM_SURFACE_MAX_DIMENSION;
	if (height > AM_SURFACE_MAX_DIMENSION)
		height = AM_SURFACE_MAX_DIMENSION;

	surface = (AMDrawingSurface *)amMalloc(sizeof(AMDrawingSurface));
	if (!surface)
		return NULL;

	// initialize the drawing surface
	surface->initialized = AM_FALSE;
	if (!amSrfInit(surface, width, height, (linearColorSpace == VG_TRUE) ? AM_TRUE : AM_FALSE, (alphaMask == VG_TRUE) ? AM_TRUE : AM_FALSE)) {
		amFree(surface);
		surface = NULL;
	}
	return surface;
#else
	(void)width;
	(void)height;
	(void)linearColorSpace;
	(void)alphaMask;
	return NULL;
#endif
}

/*!
	\brief Destroy a previously created drawing surface.

	\param _surface pointer to a (valid) previously created drawing surface.
	\note if the specified surface is the currently bound one, the function simply exits without doing nothing.
*/
VG_API_CALL void VG_API_ENTRY vgPrivSurfaceDestroyAM(void *_surface) VG_API_EXIT {

#if defined(AM_STANDALONE)
	AMDrawingSurface *surface = (AMDrawingSurface *)_surface;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	if (!surface || !surface->initialized)
		return;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (surface != currentSurface) {
		amSrfDestroy(surface);
		amFree(surface);
	}
#else
	(void)_surface;
#endif
}

/*
	\brief Resize/update the dimensions of the specified drawing surface. This function:
	- reallocates the drawing surface pixels buffer, according to new specified dimensions (AmanithVG SRE / SRE Lite).
	- if the surface contains the alpha mask buffer, it reallocates that 8bit buffer according to new specified dimensions (AmanithVG SRE / SRE Lite / GLE / GLS).

	\param _surface pointer to a (valid) previously created drawing surface.
	\param width the new desired width, in pixels.
	\param height the new desired width, in pixels.
	\return VG_FALSE if a memory allocation error occurred or if width or height are less than or equal zero, else VG_TRUE.
*/
VG_API_CALL VGboolean VG_API_ENTRY vgPrivSurfaceResizeAM(void *_surface,
														 VGint width,
														 VGint height) VG_API_EXIT {

#if defined(AM_STANDALONE)
	AMDrawingSurface *surface = (AMDrawingSurface *)_surface;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	if (!surface || !surface->initialized || width <= 0 || height <= 0)
		return VG_FALSE;

	if (width > AM_SURFACE_MAX_DIMENSION)
		width = AM_SURFACE_MAX_DIMENSION;
	if (height > AM_SURFACE_MAX_DIMENSION)
		height = AM_SURFACE_MAX_DIMENSION;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (amSrfResize(surface, width, height)) {
		// if the surface that we want to resize is the currently bound one, we have to...
		if (surface == currentSurface) {
			// ...invalidate scissor rectangles (even in the SRE, because the new resized surface could make some scissor rects invalid, so they must be re-decomposed)
			currentContext->scissorRectsModified = AM_TRUE;
		#if defined(AM_GLE) || defined(AM_GLS)
			currentContext->scissorRectsNeedUpload = AM_TRUE;
			// set viewport and projection matrix
			amGlViewport(0, 0, width, height);
			amGlScissor(0, 0, width, height, &currentContext->glContext);
			amGlMatrixProjectionLoad(&currentContext->glContext, 0.0f, (AMfloat)width, 0.0f, (AMfloat)height, 1.0f);
		#endif
		}
		return VG_TRUE;
	}
	else
		return VG_FALSE;
#else
	(void)_surface;
	(void)width;
	(void)height;
	return VG_TRUE;
#endif
}

/*
	\brief Get the drawing surface width, in pixels.

	\param _surface pointer to a (valid) previously created drawing surface.
	\return the drawing surface width, in pixels.
*/
VG_API_CALL VGint VG_API_ENTRY vgPrivGetSurfaceWidthAM(const void *_surface) VG_API_EXIT {

#if defined(AM_STANDALONE)
	AMDrawingSurface *surface = (AMDrawingSurface *)_surface;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;
	VGint res;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !surface || !surface->initialized) {
		//AM_MEMORY_LOG("vgGetSurfaceWidthAM");
		OPENVG_RETURN(0)
	}

	//AM_MEMORY_LOG("vgGetSurfaceWidthAM");
	res = amSrfWidthGet(surface);
	OPENVG_RETURN(res);
#else
	return 0;
#endif
}

/*
	\brief Get the drawing surface height, in pixels.

	\param _surface pointer to a (valid) previously created drawing surface.
	\return the drawing surface height, in pixels.
*/
VG_API_CALL VGint VG_API_ENTRY vgPrivGetSurfaceHeightAM(const void *_surface) VG_API_EXIT {

#if defined(AM_STANDALONE)
	AMDrawingSurface *surface = (AMDrawingSurface *)_surface;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;
	VGint res;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !surface || !surface->initialized) {
		//AM_MEMORY_LOG("vgGetSurfaceHeightAM");
		OPENVG_RETURN(0)
	}

	//AM_MEMORY_LOG("vgGetSurfaceHeightAM");
	res = amSrfHeightGet(surface);
	OPENVG_RETURN(res);
#else
	return 0;
#endif
}

/*!
	\brief Bind the specified context to the given drawing surface.

	\param _context NULL or a pointer to a (valid) previously created OpenVG context.
	\param _surface NULL or a pointer to a (valid) previously created drawing surface.
	\return VG_FALSE if one parameter (context or surface) is NULL and the other is not NULL, else VG_TRUE.
	\note use vgPrivMakeCurrentAM(NULL, NULL) in order to allow surface and context destruction.
	\note in AmanithVG GLE / GLS this function returns VG_FALSE if GL preconditions (e.g. the presence of depth or stencil buffer) are not satisfied.
*/
VG_API_CALL VGboolean VG_API_ENTRY vgPrivMakeCurrentAM(void *_context,
													   void *_surface) VG_API_EXIT {

#if defined(AM_STANDALONE)
	AMContext *context = (AMContext *)_context;
	AMDrawingSurface *surface = (AMDrawingSurface *)_surface;

	// if only one of the two parameter is NULL, the makecurrent is not valid
	if ((!context && surface) || (context && !surface))
		return VG_FALSE;

	if (context && surface) {
		// context and surface, if not NULL, must be already initialized
		if (!context->initialized || !surface->initialized)
			return VG_FALSE;
#if defined(AM_GLE) || defined(AM_GLS)
	if (!amGlCtxMakeCurrent(context, surface))
		return VG_FALSE;
		context->scissorRectsNeedUpload = AM_TRUE;
#endif
	// invalidate scissor rectangles (even in the SRE, because the new surface could make some scissor rects invalid, so they must be re-decomposed)
	context->scissorRectsModified = AM_TRUE;
	}
	amCtxSrfCurrentSet(_context, _surface);
	return VG_TRUE;
#else
	(void)_context;
	(void)_surface;
	return VG_TRUE;
#endif
}

/*!
	\brief Initialize the AmanithVG driver; it internally allocates:
		- a 32bit drawing surface (AmanithVG SRE)
		- a 16bit drawing surface (AmanithVG SRE Lite)
		- nothing (AManithVG GLE / GLS)
	with specified dimensions (in pixels).

	It must be also specified linear (VG_TRUE) or non-linear (VG_FALSE) color space, but
	this parameter is ignored in AmanithVG SRE Lite that always uses non-linear color space.

	\param surfaceWidth requested drawing surface width, in pixels.
	\param surfaceHeight requested drawing surface height, in pixels.
	\param surfaceLinearColorSpace VG_TRUE to request a linear color space drawing surface.
	\return VG_FALSE, if surfaceWidth or surfaceHeight are less or equal 0 or if memory allocation errors occur during initialization, else VG_TRUE.
	\note for best performance use non-linear color space.
	\note surfaceWidth and surfaceHeight are silently clamped to AM_SURFACE_MAX_DIMENSION (configurable
	through the building system); the user should call vgGetSurfaceWidthAM, vgGetSurfaceHeightAM after
	vgInitContextAM in order to check real drawing surface dimensions.
	\todo support umpremultiplied drawing surfaces.
*/
VG_API_CALL VGboolean VG_API_ENTRY vgInitContextAM(VGint surfaceWidth,
                                                   VGint surfaceHeight,
                                                   VGboolean surfaceLinearColorSpace) VG_API_EXIT {

#if defined(AM_STANDALONE)
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	if (surfaceWidth <= 0 || surfaceHeight <= 0)
		return VG_FALSE;

	if (surfaceWidth > AM_SURFACE_MAX_DIMENSION)
		surfaceWidth = AM_SURFACE_MAX_DIMENSION;
	if (surfaceHeight > AM_SURFACE_MAX_DIMENSION)
		surfaceHeight = AM_SURFACE_MAX_DIMENSION;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	// if context and surface were already allocated and initialized, simply return
	if (currentContext && currentSurface) {
		AM_ASSERT(currentContext->initialized);
		AM_ASSERT(currentSurface->initialized);
		return VG_TRUE;
	}
	AM_ASSERT(currentContext == NULL);
	AM_ASSERT(currentSurface == NULL);

	// create and initialize a new context
	currentContext = (AMContext *)vgPrivContextCreateAM(NULL);
	if (!currentContext)
		return VG_FALSE;

	// create and initialize a new surface
	currentSurface = (AMDrawingSurface *)vgPrivSurfaceCreateAM(surfaceWidth, surfaceHeight, surfaceLinearColorSpace, VG_TRUE);
	if (!currentSurface) {
		vgPrivContextDestroyAM(currentContext);
		return VG_FALSE;
	}

	// make current the created context and surface
	if (vgPrivMakeCurrentAM(currentContext, currentSurface) == VG_FALSE) {
		// destroy surface
		vgPrivSurfaceDestroyAM(currentSurface);
		// destroy context
		vgPrivContextDestroyAM(currentContext);
		return VG_FALSE;
	}

	// clear the drawing surface
	amDrawingSurfaceClear(currentContext, currentSurface, 0, 0, currentSurface->width, currentSurface->height);

	AM_MEMORY_LOG("vgInitContextAM successful");
	return VG_TRUE;
#else
	(void)surfaceWidth;
	(void)surfaceHeight;
	(void)surfaceLinearColorSpace;
	return VG_FALSE;
#endif
}

/*!
	\brief Destroy the AmanithVG driver, it frees all allocated memory and created entities (images, paths, paints).
*/
VG_API_CALL void VG_API_ENTRY vgDestroyContextAM(void) VG_API_EXIT {

#if defined(AM_STANDALONE)
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentSurface) {
		AM_MEMORY_LOG("vgInitContextAM failed to get context or surface pointers (issue due to global variables or TLS");
		return;
	}

	AM_ASSERT(currentContext && currentContext->initialized);
	AM_ASSERT(currentSurface && currentSurface->initialized);

	// set NULL global pointer to current context and current surface
	vgPrivMakeCurrentAM(NULL, NULL);
	// destroy the OpenVG context
	vgPrivContextDestroyAM(currentContext);
	// destroy the drawing surface
	vgPrivSurfaceDestroyAM(currentSurface);
	AM_MEMORY_LOG("vgDestroyContextAM successful");
#else
#endif
}

/*!
	\brief Update the dimensions of the drawing surface, since the last call of vgInitContextAM or vgResizeSurfaceAM. The function:
		- reallocates the internal drawing surface, according to new specified dimensions (AmanithVG SRE, AmanithVG SRE Lite).
		- set a new GL viewport and other GL states, according to new specified dimensions (AmanithVG GLE / GLS).

	\param surfaceWidth requested drawing surface width, in pixels.
	\param surfaceHeight requested drawing surface height, in pixels.
	\note Errors: VG_ILLEGAL_ARGUMENT_ERROR if surfaceWidth or surfaceHeight are less or equal 0, VG_OUT_OF_MEMORY_ERROR if memory allocation
	errors occur during the resize operations.
	\note in AmanithVG SRE / SRE Lite, this function simply returns without doing nothing if the context has been initialized with vgInitContextByAddrAM.
	\note surfaceWidth and surfaceHeight are silently clamped to AM_SURFACE_MAX_DIMENSION (configurable
	through the building system); the user should call vgGetSurfaceWidthAM, vgGetSurfaceHeightAM after
	vgResizeSurfaceAM in order to check real drawing surface dimensions.
*/
VG_API_CALL VGboolean VG_API_ENTRY vgResizeSurfaceAM(VGint surfaceWidth,
													 VGint surfaceHeight) VG_API_EXIT {

#if defined(AM_STANDALONE)
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;
	
	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgResizeSurfaceAM failed");
		return VG_FALSE;
	}

	// check for invalid arguments
	if (surfaceWidth <= 0 || surfaceHeight <= 0) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgResizeSurfaceAM failed due to invalid arguments");
		return VG_FALSE;
	}

	if (surfaceWidth > AM_SURFACE_MAX_DIMENSION)
		surfaceWidth = AM_SURFACE_MAX_DIMENSION;
	if (surfaceHeight > AM_SURFACE_MAX_DIMENSION)
		surfaceHeight = AM_SURFACE_MAX_DIMENSION;

	if (amSrfResize(currentSurface, surfaceWidth, surfaceHeight)) {
		// invalidate scissor rectangles (even in the SRE, because the new resized surface could make some scissor rects invalid, so they must be re-decomposed)
		currentContext->scissorRectsModified = AM_TRUE;
	#if defined(AM_GLE) || defined(AM_GLS)
		currentContext->scissorRectsNeedUpload = AM_TRUE;
		// set viewport and projection matrix
		amGlViewport(0, 0, surfaceWidth, surfaceHeight);
		amGlScissor(0, 0, surfaceWidth, surfaceHeight, &currentContext->glContext);
		amGlMatrixProjectionLoad(&currentContext->glContext, 0.0f, (AMfloat)surfaceWidth, 0.0f, (AMfloat)surfaceHeight, 1.0f);
	#endif
		amCtxErrorSet(currentContext, VG_NO_ERROR);
		AM_MEMORY_LOG("vgResizeSurfaceAM successful");
		return VG_TRUE;
	}
	else {
		amCtxErrorSet(currentContext, VG_OUT_OF_MEMORY_ERROR);
		AM_MEMORY_LOG("vgResizeSurfaceAM failed due to out of memory");
		return VG_FALSE;
	}
#else
	(void)surfaceWidth;
	(void)surfaceHeight;
	return VG_FALSE;
#endif
}

/*!
	\brief Get the drawing surface width, in pixels.
	\return the drawing surface width, in pixels.
*/
VG_API_CALL VGint VG_API_ENTRY vgGetSurfaceWidthAM(void) VG_API_EXIT {

#if defined(AM_STANDALONE)
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;
	VGint res;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		//AM_MEMORY_LOG("vgGetSurfaceWidthAM");
		OPENVG_RETURN(0)
	}
	//AM_MEMORY_LOG("vgGetSurfaceWidthAM");
	res = amSrfWidthGet(currentSurface);
	OPENVG_RETURN(res)
#else
	return 0;
#endif
}

/*!
	\brief Get the drawing surface height, in pixels.
	\return the drawing surface height, in pixels.
*/
VG_API_CALL VGint VG_API_ENTRY vgGetSurfaceHeightAM(void) VG_API_EXIT {

#if defined(AM_STANDALONE)
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;
	VGint res;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		//AM_MEMORY_LOG("vgGetSurfaceHeightAM");
		OPENVG_RETURN(0)
	}
	//AM_MEMORY_LOG("vgGetSurfaceHeightAM");
	res = amSrfHeightGet(currentSurface);
	OPENVG_RETURN(res);
#else
	return 0;
#endif
}

/*!
	\brief Get the drawing surface format.
	\return the drawing surface format.
*/
VG_API_CALL VGImageFormat VG_API_ENTRY vgGetSurfaceFormatAM(void) VG_API_EXIT {

#if defined(AM_STANDALONE)
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;
	VGImageFormat res;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgGetSurfaceFormatAM");
		return (VGImageFormat)VG_IMAGE_FORMAT_INVALID;
	}
	AM_MEMORY_LOG("vgGetSurfaceFormatAM");
	res = (VGImageFormat)amSrfRealFormatGet(currentSurface);
	OPENVG_RETURN(res)
#else
	return (VGImageFormat)VG_IMAGE_FORMAT_INVALID;
#endif
}

/*!
	\brief Get the direct access to the drawing surface pixels. It must be used only to blit the surface
	on the screen, according to the platform graphic subsystem.
	\return pointer to the drawing surface pixels.
*/
VG_API_CALL VGubyte * VG_API_ENTRY vgGetSurfacePixelsAM(void) VG_API_EXIT {

#if defined(AM_STANDALONE)
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;
	VGubyte *res;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		//AM_MEMORY_LOG("vgGetSurfaceFormatAM");
		return NULL;
	}
	//AM_MEMORY_LOG("vgGetSurfacePixelsAM");
	res = (VGubyte *)amSrfPixelsGet(currentSurface);
	OPENVG_RETURN(res)
#else
	return NULL;
#endif
}

VG_API_CALL void VG_API_ENTRY vgPostSwapBuffersAM(void) VG_API_EXIT {

#if defined(AM_STANDALONE)
#if defined(AM_GLE) || defined(AM_GLS)
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgPostSwapBuffersAM");
		return;
	}
	amGlPostSwapBuffers(currentContext);
#endif
	AM_MEMORY_LOG("vgPostSwapBuffersAM");
#else
#endif
}

#if defined(VG_MZT_statistics)
VG_API_CALL void VG_API_ENTRY vgResetStatisticsAM(const VGbitfield statistics) VG_API_EXIT {

	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgResetStatisticsAM");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	// check for illegal arguments
	if (statistics > VG_STATISTIC_ALL_MZT) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgResetStatisticsAM");
		OPENVG_RETURN(OPENVG_NO_RETVAL)
	}

	if (statistics & VG_STAT_FLATTENING_POINTS_COUNT_MZT)
		currentContext->statisticsInfo.flatteningPointsCount = 0;
	if (statistics & VG_STAT_FLATTENING_TIME_MS_MZT)
		currentContext->statisticsInfo.flatteningTimeMS = 0;
	if (statistics & VG_STAT_FLATTENING_PERFORMED_COUNT_MZT)
		currentContext->statisticsInfo.flatteningPerformedCount = 0;
	if (statistics & VG_STAT_RASTERIZER_TOTAL_TIME_MS_MZT)
		currentContext->statisticsInfo.rasterizerTotalTimeMS = 0;
	if (statistics & VG_STAT_TRIANGULATION_TRIANGLES_COUNT_MZT)
		currentContext->statisticsInfo.triangulationTrianglesCount = 0;
	if (statistics & VG_STAT_TRIANGULATION_TIME_MS_MZT)
		currentContext->statisticsInfo.triangulationTimeMS = 0;
	if (statistics & VG_STAT_STROKER_POINTS_COUNT_MZT)
		currentContext->statisticsInfo.strokerPointsCount = 0;
	if (statistics & VG_STAT_STROKER_TIME_MS_MZT)
		currentContext->statisticsInfo.strokerTimeMS = 0;
	if (statistics & VG_STAT_GL_DRAWELEMENTS_COUNT_MZT)
		currentContext->statisticsInfo.glDrawElementsCount = 0;
	if (statistics & VG_STAT_GL_DRAWARRAYS_COUNT_MZT)
		currentContext->statisticsInfo.glDrawArraysCount = 0;
	amCtxErrorSet(currentContext, VG_NO_ERROR);
	AM_MEMORY_LOG("vgResetStatisticsAM");
	OPENVG_RETURN(OPENVG_NO_RETVAL)
}

VG_API_CALL VGint VG_API_ENTRY vgGetStatisticiAM(const VGStatisticInfoMzt statistic) VG_API_EXIT {

	VGint res;
	AMContext *currentContext;
	AMDrawingSurface *currentSurface;

	amCtxSrfCurrentGet((void **)&currentContext, (void **)&currentSurface);
	if (!currentContext || !currentContext->initialized || !currentSurface || !currentSurface->initialized) {
		AM_MEMORY_LOG("vgGetStatisticiAM");
		OPENVG_RETURN(0)
	}

	// check for illegal arguments
	if (statistic != VG_STAT_FLATTENING_POINTS_COUNT_MZT &&
		statistic != VG_STAT_FLATTENING_TIME_MS_MZT &&
		statistic != VG_STAT_FLATTENING_PERFORMED_COUNT_MZT &&
		statistic != VG_STAT_RASTERIZER_TOTAL_TIME_MS_MZT &&
		statistic != VG_STAT_TRIANGULATION_TRIANGLES_COUNT_MZT &&
		statistic != VG_STAT_TRIANGULATION_TIME_MS_MZT &&
		statistic != VG_STAT_STROKER_POINTS_COUNT_MZT &&
		statistic != VG_STAT_STROKER_TIME_MS_MZT &&
		statistic != VG_STAT_GL_DRAWELEMENTS_COUNT_MZT &&
		statistic != VG_STAT_GL_DRAWARRAYS_COUNT_MZT) {
		amCtxErrorSet(currentContext, VG_ILLEGAL_ARGUMENT_ERROR);
		AM_MEMORY_LOG("vgGetStatisticiAM");
		OPENVG_RETURN(0)
	}

	amCtxErrorSet(currentContext, VG_NO_ERROR);

	switch (statistic) {
		case VG_STAT_FLATTENING_POINTS_COUNT_MZT:
			res = currentContext->statisticsInfo.flatteningPointsCount;
			break;
		case VG_STAT_FLATTENING_TIME_MS_MZT:
			res = currentContext->statisticsInfo.flatteningTimeMS;
			break;
		case VG_STAT_FLATTENING_PERFORMED_COUNT_MZT:
			res = currentContext->statisticsInfo.flatteningPerformedCount;
			break;
		case VG_STAT_RASTERIZER_TOTAL_TIME_MS_MZT:
			res = currentContext->statisticsInfo.rasterizerTotalTimeMS;
			break;
			break;
		case VG_STAT_TRIANGULATION_TRIANGLES_COUNT_MZT:
			res = currentContext->statisticsInfo.triangulationTrianglesCount;
			break;
		case VG_STAT_TRIANGULATION_TIME_MS_MZT:
			res = currentContext->statisticsInfo.triangulationTimeMS;
			break;
		case VG_STAT_STROKER_POINTS_COUNT_MZT:
			res = currentContext->statisticsInfo.strokerPointsCount;
			break;
		case VG_STAT_STROKER_TIME_MS_MZT:
			res = currentContext->statisticsInfo.strokerTimeMS;
			break;
		case VG_STAT_GL_DRAWELEMENTS_COUNT_MZT:
			res = currentContext->statisticsInfo.glDrawElementsCount;
			break;
		case VG_STAT_GL_DRAWARRAYS_COUNT_MZT:
			res = currentContext->statisticsInfo.glDrawArraysCount;
			break;
		default:
			AM_ASSERT(0 == 1);
			res = 0;
			break;
	}
	AM_MEMORY_LOG("vgGetStatisticiAM");
	OPENVG_RETURN(res)
}
#endif

#if defined RIM_VG_SRC
/*!
	\brief Get the direct access to the drawing surface pixels. It must be used only to blit the surface
	on the screen, according to the platform graphic subsystem.
    \note Does not have to be the current drawing surface
	\return pointer to the drawing surface pixels.
*/
VGubyte *vgGetSurface(VGSurface surface, VGImageFormat *format) {

    AMDrawingSurface *amDrawingSurface = (AMDrawingSurface *)surface;

    if(!amDrawingSurface || !amDrawingSurface->pixels){
        return NULL;
    }

    if(format){
        *format = amDrawingSurface->realFormat;
    }

	return (BYTE*)(amDrawingSurface->pixels);
}

/*!
	\brief Creates and initializes a VGContext
    \note LinearColorSpace is hardcoded to AM_FALSE and AlphaPremultiplied is hardcoded to AM_FALSE
    \return a valid VGContext or NULL
*/
VGContext vgCreateContext(void) {

    //Create the AmanithVG context
    AMContext *newAMContext = NULL;
    AMContextConfParams confParams;

    AM_DEBUG("vgCreateContext");

    //Create the AmanithVG context
    newAMContext = (AMContext *)amMalloc(sizeof(AMContext));
    if(newAMContext){
        newAMContext->initialized = AM_FALSE;
        // load external configuration file options
        amCtxConfigParametersLoad(&confParams);

        // initialize the OpenVG context
        if( amCtxInit(newAMContext, NULL, &confParams) == AM_FALSE ) {
            AM_MEMORY_LOG("vgInitContextAM failed to initialize context");
            vgDestroyContext((VGContext)newAMContext);
        }
    }

    return (VGContext)newAMContext;
}

/*!
	\brief Destroy the specified VGContext. 
    \note This function should only be called through eglDestroyContextAmanith in our EGL layer. When it is called 
    this function assumes that it is valid to release all resources associated with the specified context
    and that it is no longer current.
	\param context VGContext to be destroyed
    \return EGL_TRUE if the context was destroyed, EGL_FALSE otherwise
*/
EGLBoolean vgDestroyContext(VGContext context){

    AMContext *amContext = (AMContext*)context;
    AMContext *amContextCurrent = amCtxCurrentGet();

    AM_DEBUG("vgDestroyContext");

    if(amContext){
        if(amContext == amContextCurrent){
            //Release the current context
            amContextCurrent = amCtxCurrentSet(NULL);
        }
        if(amContext == amContextCurrent){
            AM_DEBUG("vgDestroyContext: This context is still current, bug");
        }
        amCtxDestroy(amContext);
        amFree(amContext);
    }
    return EGL_TRUE;
}

/*!
	\brief Creates and initializes a VGSurface
    \param surfaceWidth requested drawing surface width, in pixels.
	\param surfaceHeight requested drawing surface height, in pixels.
    \note surfaceWidth and surfaceHeight are silently clamped to AM_SURFACE_MAX_DIMENSION (configurable
	through the building system)
    \return a valid VGSurface or NULL
*/
VGSurface vgCreateSurface(const int surfaceWidth, 
                          const int surfaceHeight,
                          const int colourSpace,
                          VGboolean alphaMask)
{

    VGSurface newVGSurface = NULL;
    AMDrawingSurface *newAMDrawingSurface = NULL;
    AMint32 w = AM_CLAMP(surfaceWidth, 1, AM_SURFACE_MAX_DIMENSION);
	AMint32 h = AM_CLAMP(surfaceHeight, 1, AM_SURFACE_MAX_DIMENSION);
    AMbool linearColorSpace = (colourSpace == EGL_COLORSPACE_LINEAR) ? AM_TRUE : AM_FALSE;

    AM_DEBUG("vgCreateSurface");

    newAMDrawingSurface = (AMDrawingSurface *)amMalloc(sizeof(AMDrawingSurface));
    if(newAMDrawingSurface){
        memset(newAMDrawingSurface, 0, sizeof(AMDrawingSurface)); 
        if(amSrfInit(newAMDrawingSurface, w, h, linearColorSpace, 
            ((alphaMask == VG_TRUE) ? AM_TRUE : AM_FALSE)) == VG_TRUE){
            newVGSurface = (VGSurface)newAMDrawingSurface;
            amInternalDrawingSurfaceClear(newAMDrawingSurface, 0, 0, w, h);
        }else{
            vgDestroySurface((VGSurface)newAMDrawingSurface);
        }
    }

    return newVGSurface;
}

/*!
    \brief Initializes a VGSurface from an existing NativePixmap buffer
    \brief rendering is done directly to the pixmap's buffer so be careful
    \brief about synchronization.
    \param pixmapBuffer specifies the pixmap buffer.
    \param pixmapWidth specifies the pixmap width, in pixels.
    \param pixmapHeight specifies the pixmap height, in pixels.
    \param colourSpace specifies whether the color rendering is linear or not.
    \param alphaFormat returns the alpha format of the drawing surface.
    \return a valid VGSurface or NULL
*/
VGSurface vgCreatePixmapSurface(void* pixmapBuffer, 
                          const int pixmapWidth, 
                          const int pixmapHeight,
                          const int colourSpace,
                          VGboolean alphaMask)
{
    VGSurface newVGSurface = NULL;
    AMDrawingSurface *newAMDrawingSurface = NULL;
    AMint32 w = AM_CLAMP(pixmapWidth, 1, AM_SURFACE_MAX_DIMENSION);
    AMint32 h = AM_CLAMP(pixmapHeight, 1, AM_SURFACE_MAX_DIMENSION);
    AMbool linearColorSpace = (colourSpace == EGL_VG_COLORSPACE_LINEAR) ? AM_TRUE : AM_FALSE;

    AM_DEBUG("vgCreatePixmapSurface");

    newAMDrawingSurface = (AMDrawingSurface *)amMalloc(sizeof(AMDrawingSurface)); 
    if(newAMDrawingSurface){
        memset(newAMDrawingSurface, 0, sizeof(AMDrawingSurface));
        newAMDrawingSurface->pixels = pixmapBuffer; 
        newAMDrawingSurface->isPixmapSurface = AM_TRUE; 
        if(amSrfInit(newAMDrawingSurface, w, h, linearColorSpace, 
            ((alphaMask == VG_TRUE) ? AM_TRUE : AM_FALSE)) == VG_TRUE){
            newVGSurface = (VGSurface)newAMDrawingSurface;
            amInternalDrawingSurfaceClear(newAMDrawingSurface, 0, 0, w, h);
        }else{
            vgDestroySurface((VGSurface)newAMDrawingSurface);
        }
    }

    return newVGSurface;
}

/*!
	\brief Creates and initializes a VGSurface from an existing VGImage
    \param surfaceWidth returns drawing surface width, in pixels.
	\param surfaceHeight returns drawing surface height, in pixels.
    \param colourSpace returns the colour space of the drawing surface.
	\param alphaFormat returns the alpha format of the drawing surface.
    \param clientBuffer VGImage that is to be used as the VGSurface
    \return a valid VGSurface or NULL
*/
VGSurface vgCreateSurfaceFromClient(const VGImage clientBuffer, 
                                    const int surfaceWidth, 
                                    const int surfaceHeight,
                                    const int colourSpace,
                                    VGboolean alphaMask){

    VGSurface newVGSurface = NULL;
    AMDrawingSurface *newAMDrawingSurface = NULL;
    AMint32 w = AM_CLAMP(surfaceWidth, 1, AM_SURFACE_MAX_DIMENSION);
	AMint32 h = AM_CLAMP(surfaceHeight, 1, AM_SURFACE_MAX_DIMENSION);
    AMbool linearColorSpace = (colourSpace == EGL_COLORSPACE_LINEAR) ? AM_TRUE : AM_FALSE;

    AM_DEBUG("vgCreateSurfaceFromClient");
    newAMDrawingSurface = (AMDrawingSurface *)amMalloc(sizeof(AMDrawingSurface));
    if(newAMDrawingSurface){
        memset(newAMDrawingSurface, 0, sizeof(AMDrawingSurface)); 
        if(amSrfInit(newAMDrawingSurface, w, h, linearColorSpace, 
            ((alphaMask == VG_TRUE) ? AM_TRUE : AM_FALSE)) == VG_TRUE) {
            newVGSurface = (VGSurface)newAMDrawingSurface;
            amInternalDrawingSurfaceClear(newAMDrawingSurface, 0, 0, w, h);
        }else{
            vgDestroySurface((VGSurface)newAMDrawingSurface);
        }
    }

    return newVGSurface;
}

/*!
	\brief Destroys the specified VGSurface
    \param surface VGSurface to be destroyed
    \note This function should only be called through eglDestroySurfaceAmanith in our EGL layer. When it is called 
    this function assumes that it is valid to release all resources associated with the specified surface
    and that it is no longer current.
    \return EGL_TRUE if the surface was destroyed, EGL_FALSE otherwise
*/
EGLBoolean vgDestroySurface(VGSurface surface){

    AMDrawingSurface *amDrawingSurface = (AMDrawingSurface *)surface;

    AM_DEBUG("vgDestroySurface");

    //Assuming that the drawing surface is not current
    if(amDrawingSurface){
        amSrfDestroy(amDrawingSurface);
        amFree(amDrawingSurface);
    }
    return EGL_TRUE;
}

/*!
	\brief Makes the specified context and surface current
    \param context VGContext to make current
    \param surface VGSurface to make current
    \note Assumes that context and surface are valid inputs
    \return EGL_TRUE if the operation was succesful, EGL_FALSE otherwise
*/
EGLBoolean vgMakeCurrent(VGContext context, VGSurface surface){

    AMDrawingSurface *amSurfaceNew = (AMDrawingSurface *)surface;
    AMContext *amContextNew = (AMContext *)context;
    AMDrawingSurface *amSurfaceCurrent = NULL;
    AMContext *amContextCurrent = NULL;

    //amContextCurrent = amCtxCurrentGet();
    //amSurfaceCurrent = amSrfCurrentGet();
    amCtxSrfCurrentGet((void **)&amContextCurrent, (void **)&amSurfaceCurrent);

    AM_DEBUG("vgMakeCurrent");

    if(amContextNew == NULL && amSurfaceNew == NULL){
        //Check if we have anything to release
        if( amContextCurrent != NULL && amSurfaceCurrent != NULL ) {
            //Release the current surface and context
            amSrfCurrentSet(NULL);
            amCtxCurrentSet(NULL);
        }
    }else if(amContextNew != NULL && amSurfaceNew != NULL){
        //Bind the new surface and context
        amSrfCurrentSet(amSurfaceNew);
        amCtxCurrentSet(amContextNew);
    }else{
        AM_DEBUG("Invalid parameters for vgMakeCurrent");
        return EGL_FALSE;
    }

    return EGL_TRUE;
}
#endif


#undef AM_CFG_FILE_MAX_LINE_LENGTH

#if defined (RIM_VG_SRC)
#pragma pop
#endif

