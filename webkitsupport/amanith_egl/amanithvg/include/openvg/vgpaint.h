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

#ifndef _VGPAINT_H
#define _VGPAINT_H

/*!
	\file vgpaint.h
	\brief OpenVG paint, header.
	\author Matteo Muratori
	\author Michele Fabbri
*/

#include "vg_priv.h"
#include "vgimage.h"
#include "vgmatrix.h"

// *********************************************************************
//                                 AMPaint
// *********************************************************************

/*!
	\brief Color ramp stop structure.
*/
typedef struct _AMColorStop {
	//! Position, in the range [0.0f; 1.0f].
	AMfloat position;
	//! Color components, float values in the range [0.0f; 1.0f].
	AMfloat color[4];
} AMColorStop;

AM_DYNARRAY_DECLARE(AMColorStopDynArray, AMColorStop, _AMColorStopDynArray)

/*!
	\brief Paint structure, used to implement OpenVG paints.
*/
typedef struct _AMPaint {
	//! VGHandle type identifier, it can be AM_PAINT_HANDLE_ID or AM_INVALID_HANDLE_ID.
	AMuint16 id;
	//! It's always AM_PAINT_HANDLE_ID, never changed.
	AMuint16 type;
	//! VG handle (index inside the context createdHandlesList).
	VGHandle vgHandle;
	//! Reference counter.
	AMuint32 referenceCounter;
	//!	\name Original (unpatched) OpenVG values.
	//@{
	//! Paint type.
	VGPaintType paintType;
	//! Paint color (RGBA), in non-premultiplied non-linear color space.
	VGfloat sColor[4];
	//! Color ramp spread mode.
	VGColorRampSpreadMode colorRampSpreadMode;
	/*!
		If VG_TRUE color and alpha values at each gradient stop are multiplied together to form premultiplied
		sRGBA values prior to interpolation. Otherwise, color and alpha values are processed independently.
	*/
	VGboolean colorRampPremultiplied;
	//! Color stops (RGBA), in non-premultiplied non-linear color space.
	AMColorStopDynArray sColorStops;
	//! Linear gradient start point.
	VGfloat linGradPt0[2];
	//! Linear gradient end point.
	VGfloat linGradPt1[2];
	//! Radial gradient center point.
	VGfloat radGradCenter[2];
	//! Radial gradient focus point.
	VGfloat radGradFocus[2];
	//! Radial gradient radius.
	VGfloat radGradRadius;
	//! Tiling mode.
	VGTilingMode tilingMode;
#if defined(VG_MZT_conical_gradient)
	//! Conical gradient center point.
	VGfloat conGradPt0[2];
	//! Conical gradient target point.
	VGfloat conGradPt1[2];
	//! Conical gradient repeats.
	VGfloat conGradRepeats;
#endif
#if defined(VG_MZT_color_ramp_interpolation)
	//! Color ramp interpolation type.
	VGColorRampInterpolationTypeMzt colorRampInterpolationType;
#endif
	//! Image handle for pattern paint type.
	VGImage pattern;
	//@}
	//! Minimum alpha value of gradients color keys.
	AMfloat gradientMinAlpha;
	//! Maximum alpha value of gradients color keys.
	AMfloat gradientMaxAlpha;
	//! Gradient main texture (used by linear and radial gradients, to draw paths and images in stencil mode).
	AMTexture gradTexture;
	//! Gradient reflected texture (used by linear and radial gradients, to draw paths and images in stencil mode).
	AMTexture reflectGradTexture;
	//! Gradient main texture (used by linear and radial gradients, to draw images in multiply mode).
	AMTexture gradTextureImgMultiply;
	//! Gradient reflected texture (used by linear and radial gradients, to draw images multiply mode).
	AMTexture reflectGradTextureImgMultiply;
	//! Validity flag for linear/radial/conical gradients texture.
	AMbool gradTexturesValid;
#if (AM_OPENVG_VERSION >= 110)
	//!	Hash value corresponding to a color transform configuration. When color transform changes, hash	changes, and textures must be recalculated.
	AMuint32 ctTexturesHash;
#endif
	//! Patched internal linear gradients target.
	AMVect2f patchedLinGradTarget;
	//! Patched internal radial gradients focus.
	AMVect2f patchedRadGradFocus;
	//! Patched internal radial gradients radius.
	AMfloat patchedRadGradRadius;
#if defined(VG_MZT_conical_gradient)
	//! Patched internal conical gradients target.
	AMVect2f patchedConGradTarget;
	//! Patched internal conical gradients repeats.
	AMfloat patchedConGradRepeats;
#endif
	//@}
} AMPaint;

/*!
	\brief Internal paint descriptor.
*/
typedef struct _AMPaintDesc {
	//! Pointer to the original OpenVG paint.
	AMPaint *paint;
	//!	Derived paint type; in some specific cases it can be different from the original one (e.g. clear blend mode, paint type is forced to color).
	AMuint32 paintType;
	//!	Derived blend mode; in some specific cases it can be different from the original one (e.g. src_over	blend mode can be substituted by src blend mode for opaque paints).
	AMuint32 blendMode;
	//!	Derived image mode; in some specific cases it can be different from the original one (e.g. draw image with perspective matrix, multiply/stencil image mode is forced to normal image mode.
	AMuint32 imageMode;
	//! Patched paint color: non-linear and non-premultiplied color space, components in [0.0f; 1.0f] range.
	AMfloat paintColor[4];
	//! Patched tile fill color: non-linear and non-premultiplied color space, components in [0.0f; 1.0f] range.
	AMfloat tileFillColor[4];
	//!	Derived fill rule; in some specific cases it can be different from the original one (e.g. stroke drawings are always performed using non-zero fill rule).
	VGFillRule fillRule;
	//!	Derived masking flag; in some specific cases it can be different from the original one (e.g. masking is always disabled for vgSetPixels operations).
	AMbool masking;
	//!	Derived rendering quality flag; in some specific cases it can be different from the original one.
	VGRenderingQuality renderingQuality;
	//! Derived image quality used by vgDrawImage fillers.
	VGImageQuality imageQuality;
	//! Pointer to the image to draw, used by vgDrawImage fillers.
	const AMImage *image;
	//! Image quality used for pattern paints.
	VGImageQuality patternQuality;
	//! User to surface descriptor.
	AMUserToSurfaceDesc *userToSurfaceDesc;
	//! Paint-to-user matrix, used during paint generation.
	AMMatrix33f *paintToUser;
	//! Inverse paint-to-user matrix, used during paint generation.
	AMMatrix33f *invPaintToUser;
#if (AM_OPENVG_VERSION >= 110)
	//! AM_TRUE if the color transformation must take place, else AM_FALSE.
	AMbool colorTransform;
	#if defined(AM_GLE) || defined(AM_GLS)
		AMfloat ctGlColor[4];
		AMbool ctUseGlColor;
	#endif
#endif
#if defined(AM_GLE) || defined(AM_GLS)
	//! AM_TRUE if a multiple pass technique (geometry mask using depth/stencil buffers) is required, else AM_FALSE.
	AMbool multiplePass;
	//! AM_TRUE if framebuffer grab is required to realize DARKEN / LIGHTEN blend modes, else AM_FALSE.
	AMbool minmaxBlendGrab;
	//! AM_TRUE if the alpha mask replace pass is required, else AM_FALSE.
	AMbool alphaMaskReplace;
	//! AM_TRUE if a software workaround to realize image MULTIPLY is required, else AM_FALSE.
	AMbool imgMultiplySoftware;
#endif
} AMPaintDesc;

// Given a float color in non-linear non-premultiplied color space (components in [0;1] range), it returns the corresponding 32bit color according to the specified format index.
AMuint32 amColorPackByFormat(const AMfloat *col,
							 const AMuint32 formatIndex);
// Given a float color transformation, it returns its fixed point equivalent.
void amColorTransformFToI(AMint32 *dst,
						  const AMfloat *src);
// Given a paint descriptor, it returns AM_TRUE if the paint is opaque (taking care of color transform, alpha masking and so on), else AM_FALSE.
AMbool amPaintIsOpaque(const AMPaintDesc *paintDesc,
					   const AMbool includeColorTransform,
					   const void *_context);
// Set a paint inside a context.
void amPaintSet(void *_context,
				VGHandle paint,
				const VGbitfield paintModes);
// Set the paint color of a given paint.
void amPaintColorSet(AMPaint *paint,
					 const AMfloat sColor[4]);
// Set the paint type of a given paint.
void amPaintTypeSet(AMPaint *paint,
					const VGPaintType paintType);
// Set the color ramp spread mode of a given paint.
void amPaintColorRampSpreadModeSet(AMPaint *paint,
								   const VGColorRampSpreadMode spreadMode);
// Set the color ramp premultiplied flag of a given paint.
void amPaintColorRampPremultipliedSet(AMPaint *paint,
									  const VGboolean premultiplied);
#if defined(VG_MZT_color_ramp_interpolation)
// Set the color ramp interpolation type of a given paint.
void amPaintColorRampInterpolationTypeSet(AMPaint *paint,
										  const VGColorRampInterpolationTypeMzt interpolationType);
#endif
// Set the tiling mode of a given paint.
void amPaintTilingModeSet(AMPaint *paint,
						  const VGTilingMode tilingMode);
// Set color stops of a given paint, float values.
AMbool amPaintColorStopsSetf(AMPaint *paint,
							 const AMfloat *stops,
							 const AMint32 stopsCount);
// Set color stops of a given paint, integer values.
AMbool amPaintColorStopsSeti(AMPaint *paint,
							 const AMint32 *stops,
							 const AMint32 stopsCount);
// Set linear gradient parameters of a given paint.
void amPaintLinGradParametersSet(AMPaint *paint,
								 const AMfloat start[2],
								 const AMfloat end[2]);
// Set radial gradient parameters of a given paint.
void amPaintRadGradParametersSet(AMPaint *paint,
								 const AMfloat center[2],
								 const AMfloat focus[2],
								 const AMfloat radius);
#if defined(VG_MZT_conical_gradient)
// Set conical gradient parameters of a given paint.
void amPaintConGradParametersSet(AMPaint *paint,
								 const AMfloat center[2],
								 const AMfloat target[2],
								 const AMfloat repeats);
#endif
// Replace any previous pattern image defined on the given paint object.
void amPaintPatternSet(AMPaint *paint,
					   VGImage pattern,
					   void *_context);
// Clamp the given image quality against a set of allowed qualities.
VGImageQuality amImageQualityClamp(const VGImageQuality quality,
								   const VGbitfield allowedQuality);

// Update a paint descriptor, used by vgDrawImage function.
AMbool amPaintDescImageUpdate(AMPaintDesc *paintDesc,
							  void *_context,
							  AMDrawingSurface *surface);

// Get a paint descriptor for a path fill.
AMbool amPaintDescPathFillGet(AMPaintDesc *paintDesc,
							  void *_context,
							  AMDrawingSurface *surface,
							  AMUserToSurfaceDesc *userToSurfaceDesc,
							  const VGbitfield paintModes);
// Get a paint descriptor for a path stroke.
AMbool amPaintDescPathStrokeGet(AMPaintDesc *paintDesc,
								void *_context,
								AMDrawingSurface *surface,
								AMUserToSurfaceDesc *userToSurfaceDesc);
// Get a paint descriptor for an image.
AMbool amPaintDescImageFillGet(AMPaintDesc *paintDesc,
							   void *_context,
							   AMDrawingSurface *surface,
							   AMUserToSurfaceDesc *userToSurfaceDesc,
							   AMImage *image);
// Initialize a paint structure.
AMbool amPaintInit(AMPaint *paint);
// Destroy paint resources.
void amPaintResourcesDestroy(AMPaint *paint,
							 void *_context);
// Destroy a paint structure.
void amPaintDestroy(AMPaint *paint,
					void *_context);

void amPaintMemoryRetrieve(AMPaint *paint,
						   const AMbool preserveData,
						   const void *_context);

// *********************************************************************
//                         Paints pools management
// *********************************************************************
typedef union _AMPaintRef {
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
} AMPaintRef;

AM_DYNARRAY_DECLARE(AMPaintRefDynArray, AMPaintRef, _AMPaintRefDynArray)

typedef struct _AMPaintsPool {
	//! Paints pool.
	AMPaint data[AM_PAINTS_POOL_CAPACITY];
	//! Number of paints currently stored in the pool.
	AMuint32 size;
} AMPaintsPool, *AMPaintsPoolPtr;

AM_DYNARRAY_DECLARE(AMPaintsPoolPtrDynArray, AMPaintsPoolPtr, _AMPaintsPoolPtrDynArray)

typedef struct _AMPaintsPoolsManager {
	//! Paints pools (pointers).
	AMPaintsPoolPtrDynArray pools;
	//! List of available handles, to be reused.
	AMPaintRefDynArray availablePaintsList;
	//! AM_TRUE if correctly initialized, else AM_FALSE.
	AMbool initialized;
} AMPaintsPoolsManager;

AMbool amPaintsPoolsManagerInit(AMPaintsPoolsManager *paintsPoolsManager);

void amPaintsPoolsManagerDestroy(AMPaintsPoolsManager *paintsPoolsManager);

void amPaintsPoolsAvailablesSort(AMPaintsPoolsManager *paintsPoolsManager);

AMbool amPaintNew(AMPaint **paint,
				  VGPaint *handle,
				  void *_context);

void amPaintRemove(void *_context,
				   AMPaint *paint);

#endif
