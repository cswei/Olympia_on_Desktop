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
    \file vg_priv.c
    \brief AmanithVG private data structures and functions, implementation.
    \author Matteo Muratori
    \author Michele Fabbri
*/

#if defined (RIM_VG_SRC)
#pragma push
#pragma arm
#endif

#include "vg_priv.h"
#include "vgcontext.h"

#if defined RIM_VG_SRC
    #include <KHR_thread_storage.h>
    #include "egl.h"
    #include "i_egl.h"
    #include "egl_amanith.h"
    #include "kd.h"
#endif

#if defined TORCH_VG_SRC
#include "vgconversions.h"
#endif

#if defined(AM_STANDALONE)  || defined(RIM_VG_SRC)
    #if defined(AM_OS_SYMBIAN) && !defined(AM_CC_MSVC)
        #include "symbiancontext.h"
    #elif defined(AM_OS_BREW)
        #include "brewcontext.h"
    #else
        // all other platforms support global writable static variables
        #if defined(RIM_VG_SRC)
            #include "vgconversions.h"
            //These are the TLS keys used to store thread specific data
            KDThreadStorageKeyKHR currentContextKey = 0;
            KDThreadStorageKeyKHR currentSurfaceKey = 0;
        #else
            AMContext *_current_context = NULL;
            AMDrawingSurface *_current_drawing_surface = NULL;
        #endif
    #endif
#else
    #include "vgconversions.h"
#endif

#if defined RIM_VG_SRC

/*!
    \brief Get the pointer to the current context.
    \return the pointer to the current context.
*/
void *amCtxCurrentGet(void) {

#if defined(AM_STANDALONE)
    return (void *)kdGetThreadStorageKHR(currentContextKey);
#else
    amMutexAcquire();
    return amOpenVGContextGet();
#endif
}

/*!
    \brief Get the pointer to the current surface.
    \return the pointer to the current surface.
*/
void *amSrfCurrentGet(void) {

    return (void *)kdGetThreadStorageKHR(currentSurfaceKey);
}

/*!
    \brief Set the current context to a new context.
    \return the pointer to the current context.
*/
AMContext *amCtxCurrentSet(AMContext *context) {

    kdSetThreadStorageKHR(currentContextKey, (void *)(context));
    return context;
}

/*!
    \brief Set the current surface to a new surface.
    \return the pointer to the current surface.
*/
AMDrawingSurface *amSrfCurrentSet(AMDrawingSurface *surface) {

    kdSetThreadStorageKHR(currentSurfaceKey, (void *)(surface));
    return surface;
}

/*!
    \brief Initializes the thread to VGContext map for Amanith
*/
void vgInitAmanith(void) {

    kdLogMessage("vgInitAmanith");
    // Create Thread Local Storage keys.
    currentContextKey = kdMapThreadStorageKHR(NULL);
    currentSurfaceKey = kdMapThreadStorageKHR(NULL);
}
#endif


/*!
    \brief Get the pointer to the current context and drawing surface. AM_FALSE is returned if the operation failed.
*/
void amCtxSrfCurrentGet(void **_context,
                        void **_surface) {

    AMContext *ctx = NULL;
    AMDrawingSurface *srf = NULL;

#if defined(AM_STANDALONE) || defined(RIM_VG_SRC)
    #if defined(AM_OS_SYMBIAN) && !defined(AM_CC_MSVC)
        // TO DO
        amSymbianCtxSrfCurrentGet(&ctx, &srf);
    #elif defined(AM_OS_BREW)
        // TO DO
        amBrewCtxSrfCurrentGet(&ctx, &srf);
    #else
        #if defined(RIM_VG_SRC)
            ctx = amCtxCurrentGet();
            srf = amSrfCurrentGet();
        #else
            ctx = _current_context;
            srf = _current_drawing_surface;
        #endif
    #endif
#else
    amMutexAcquire();
    amOpenVGContextSurfaceGet((void **)&ctx, (void **)&srf);
#endif

    if (ctx && srf) {
        *_context = ctx;
        *_surface = srf;
    }
    else {
        *_context = NULL;
        *_surface = NULL;
    }
}

void amCtxSrfCurrentSet(void *_context,
                        void *_surface) {

#if defined(AM_STANDALONE)  || defined(RIM_VG_SRC)
    #if defined(AM_OS_SYMBIAN) && !defined(AM_CC_MSVC)
        // TO DO
        amSymbianCtxSrfCurrentSet(_context, _surface);
    #elif defined(AM_OS_BREW)
        // TO DO
        amBrewCtxSrfCurrentSet(_context, _surface);
    #else
        #if defined RIM_VG_SRC
            amCtxCurrentSet((AMContext *)_context);
            amSrfCurrentSet((AMDrawingSurface *)_surface);
        #else
            _current_context = _context;
            _current_drawing_surface = _surface;
        #endif
    #endif
#else
    amMutexAcquire();
    amOpenVGContextSurfaceSet(_context, _surface);
#endif
}

#if !defined(AM_STANDALONE)

VG_API_CALL signed int VG_API_ENTRY vgPrivCtxSizeGet(void) VG_API_EXIT {

    return (sizeof(AMContext));
}

VG_API_CALL unsigned int VG_API_ENTRY vgPrivCtxInit(void *_context) VG_API_EXIT {

    AMContext *context = (AMContext *)_context;
    AMContextConfParams confParams;
    unsigned int res;

    AM_ASSERT(context);

    // load external configuration file options
    amCtxConfigParametersLoad(&confParams);

    // initialize the OpenVG context
    res = amCtxInit(context, &confParams);
    if (!res)
        return AM_FALSE;

    // patch capabilities according to configuration parameters
#if defined(AM_GLE) || defined(AM_GLS)
    amGlCtxSetConfigurationParams(&context->glContext, surfaceWidth, surfaceHeight, &confParams);
    if (context->glContext.stencilBits <= 0 && context->glContext.depthBits <= 0)
        return AM_FALSE;
#endif
    return AM_TRUE;
}

VG_API_CALL void VG_API_ENTRY vgPrivCtxDestroy(void *_context) VG_API_EXIT {

    AMContext *context = (AMContext *)_context;

    AM_ASSERT(context);
    // destroy the OpenVG context
    amCtxDestroy(context);
}

VG_API_CALL signed int VG_API_ENTRY vgPrivSrfMaxSizeGet(void) VG_API_EXIT {

    return AM_SURFACE_MAX_DIMENSION;
}

VG_API_CALL VGImageFormat VG_API_ENTRY vgPrivSrfFormat(const unsigned int linearColorSpace,
                                                       const unsigned int alphaFormatPre) VG_API_EXIT {

    VGImageFormat res;

#if defined(AM_SURFACE_BYTE_ORDER_RGBA)
    if (linearColorSpace)
        res = (alphaFormatPre) ? VG_lRGBA_8888_PRE : VG_lRGBA_8888;
    else
        res = (alphaFormatPre) ? VG_sRGBA_8888_PRE : VG_sRGBA_8888;
#elif defined(AM_SURFACE_BYTE_ORDER_ARGB)
    if (linearColorSpace)
        res = (alphaFormatPre) ? VG_lARGB_8888_PRE : VG_lARGB_8888;
    else
        res = (alphaFormatPre) ? VG_sARGB_8888_PRE : VG_sARGB_8888;
#elif defined(AM_SURFACE_BYTE_ORDER_BGRA)
    if (linearColorSpace)
        res = (alphaFormatPre) ? VG_lBGRA_8888_PRE : VG_lBGRA_8888;
    else
        res = (alphaFormatPre) ? VG_sBGRA_8888_PRE : VG_sBGRA_8888;
#else
    if (linearColorSpace)
        res = (alphaFormatPre) ? VG_lABGR_8888_PRE : VG_lABGR_8888;
    else
        res = (alphaFormatPre) ? VG_sABGR_8888_PRE : VG_sABGR_8888;
#endif
    return res;
}

VG_API_CALL void VG_API_ENTRY vgPrivImageInfoGet(void *_context,
                                                 VGImage handle,
                                                 unsigned int *isOpenVGImage,
                                                 unsigned int *isInUse,
                                                 signed int *width,
                                                 signed int *height,
                                                 unsigned int *colorSpaceLinear,
                                                 unsigned int *alphaFormatPre,
                                                 signed int *redSize,
                                                 signed int *greenSize,
                                                 signed int *blueSize,
                                                 signed int *alphaSize,
                                                 signed int *luminanceSize) VG_API_EXIT {

    AMContext *context = (AMContext *)_context;

    AM_ASSERT(context);

    if (!context || !context->initialized || amCtxHandleValid(context, handle) != AM_IMAGE_HANDLE_ID) {
        *isOpenVGImage = AM_FALSE;
        *isInUse = AM_FALSE;
        *width = 0;
        *height = 0;
        *colorSpaceLinear = AM_FALSE;
        *alphaFormatPre = AM_FALSE;
        *redSize = 0;
        *greenSize = 0;
        *blueSize = 0;
        *alphaSize = 0;
        *luminanceSize = 0;
    }
    else {
        AMImage *img = (AMImage *)context->handles->createdHandlesList.data[handle];
        AMuint32 idx = AM_FMT_GET_INDEX(img->format);
        AMuint32 flags = pxlFormatTable[idx][FMT_FLAGS];

        *isOpenVGImage = AM_TRUE;
        // if reference counter > 1 or image has a father, the image is in use
        *isInUse = (img->referenceCounter > 1 || img->parent) ? AM_TRUE : AM_FALSE;
        *width = img->width;
        *height = img->height;
        *colorSpaceLinear = (flags & FMT_L) ? AM_TRUE : AM_FALSE;
        *alphaFormatPre = (flags & FMT_PRE) ? AM_TRUE : AM_FALSE;
        *redSize = pxlFormatTable[idx][FMT_R_BITS];
        *greenSize = pxlFormatTable[idx][FMT_G_BITS];
        *blueSize = pxlFormatTable[idx][FMT_B_BITS];
        *alphaSize = pxlFormatTable[idx][FMT_A_BITS];
        switch (img->format) {
            case VG_sL_8:
            case VG_lL_8:
                *luminanceSize = 8;
                break;
            case VG_BW_1:
                *luminanceSize = 1;
                break;
            default:
                *luminanceSize = 0;
                break;
        }
    }
}

VG_API_CALL unsigned int VG_API_ENTRY vgPrivImageInUseByOpenVG(void *_context,
                                                               VGImage handle) VG_API_EXIT {

    AMContext *context = (AMContext *)_context;

    AM_ASSERT(context);

    if (!context || !context->initialized || amCtxHandleValid(context, handle) != AM_IMAGE_HANDLE_ID)
        return AM_FALSE;
    else {
        AMImage *img = (AMImage *)context->handles->createdHandlesList.data[handle];
        return (img->referenceCounter > 1 || img->parent) ? AM_TRUE : AM_FALSE;
    }
}

VG_API_CALL unsigned int VG_API_ENTRY vgPrivImageRefCounterInc(void *_context,
                                                               VGImage handle) {

    AMContext *context;
    AMImage *image;

    context = (AMContext *)_context;
    AM_ASSERT(context && context->initialized);

    if (!handle || handle >= context->handles->createdHandlesList.size)
        return AM_FALSE;

    image = (AMImage *)context->handles->createdHandlesList.data[handle];
    AM_ASSERT(image && image->type ==AM_IMAGE_HANDLE_ID);
    amCtxHandleRefCounterInc(image);
    return AM_TRUE;
}

VG_API_CALL unsigned int VG_API_ENTRY vgPrivImageRefCounterDec(void *_context,
                                                               VGImage handle) {

    AMContext *context;
    AMImage *image;

    context = (AMContext *)_context;
    AM_ASSERT(context && context->initialized);

    if (!handle || handle >= context->handles->createdHandlesList.size)
        return AM_FALSE;

    image = (AMImage *)context->handles->createdHandlesList.data[handle];
    AM_ASSERT(image && image->type ==AM_IMAGE_HANDLE_ID);
    // if resources has been totally released, we can remove image pointer from the internal list of current context
    if (amCtxHandleRefCounterDec(image, context) == 0) {
        amCtxHandleRemove(context, handle);
        amFree(image);
    }

    return AM_TRUE;
}

VG_API_CALL void VG_API_ENTRY vgPrivImageLock(void *_context,
                                              VGImage handle,
                                              AMDrawingSurface *srf) VG_API_EXIT {

    AMContext *context = (AMContext *)_context;
#if defined ( RIM_VG_SRC )
    AMImage *image;
#else
	AMImage *image = (AMImage *)context->handles->createdHandlesList.data[handle];
#endif
    AMint32 w = srf->mipmaps.data[srf->mipmapLevel].width;
    AMint32 h = srf->mipmaps.data[srf->mipmapLevel].height;

#if defined ( RIM_VG_SRC )
    if( handle < context->handles->createdHandlesList.size ) {
        image = (AMImage *)context->handles->createdHandlesList.data[handle];
    } else {
        return;
    }
#endif

    AM_ASSERT(context);
    AM_ASSERT(image);
    AM_ASSERT(!image->inUseByEgl);
    AM_ASSERT(image->width == w);
    AM_ASSERT(image->height == h);

    amPxlMapConvert(&srf->mipmaps.data[srf->mipmapLevel].pixels[w * (h - 1)], srf->vgFormat, -srf->mipmaps.data[srf->mipmapLevel].dataStride, 0, 0,
                    image->pixels, image->format, image->dataStride, 0, 0,
                    w, h, AM_FALSE, AM_TRUE);

    // NB: image cannot be parented with anyone, nor it can have children
    //amCtxHandleRefCounterInc(image);
    image->inUseByEgl = AM_TRUE;
}

VG_API_CALL void VG_API_ENTRY vgPrivImageUnlock(void *_context,
                                                VGImage handle,
                                                AMDrawingSurface *srf) VG_API_EXIT {

    AMContext *context = (AMContext *)_context;
    AMImage *image = (AMImage *)context->handles->createdHandlesList.data[handle];
    AMint32 w = srf->mipmaps.data[srf->mipmapLevel].width;
    AMint32 h = srf->mipmaps.data[srf->mipmapLevel].height;

    AM_ASSERT(context);
    AM_ASSERT(image);
    AM_ASSERT(image->inUseByEgl);
    AM_ASSERT(image->width == w);
    AM_ASSERT(image->height == h);

    amPxlMapConvert(image->pixels, image->format, image->dataStride, 0, 0,
                    &srf->mipmaps.data[srf->mipmapLevel].pixels[w * (h - 1)], srf->vgFormat, -srf->mipmaps.data[srf->mipmapLevel].dataStride, 0, 0,
                    w, h, AM_FALSE, AM_TRUE);
    //amCtxHandleRefCounterDec(image, context);
    image->inUseByEgl = AM_FALSE;
}

VG_API_CALL void VG_API_ENTRY vgPrivDirtyRegionsInvalidate(AMDrawingSurface *srf) VG_API_EXIT {

    AM_ASSERT(srf && srf->initialized);

    srf->wholeCleared = AM_FALSE;
    srf->dirtyRegions.size = 0;
    srf->dirtyRegionsOverflow = AM_FALSE;
}

#endif // !AM_STANDALONE



#if defined RIM_VG_SRC || defined TORCH_VG_SRC
#if defined TORCH_VG_SRC
VG_API_CALL void VG_API_ENTRY vgPrivImageInfoGet(void *_context,
                                                 VGImage handle,
                                                 unsigned int *isOpenVGImage,
                                                 unsigned int *isInUse,
                                                 signed int *width,
                                                 signed int *height,
                                                 unsigned int *colorSpaceLinear,
                                                 unsigned int *alphaFormatPre,
                                                 signed int *redSize,
                                                 signed int *greenSize,
                                                 signed int *blueSize,
                                                 signed int *alphaSize,
                                                 signed int *luminanceSize) VG_API_EXIT {
    AMContext *context = (AMContext *) _context;

    AM_ASSERT(context);
#else
void vgPrivImageInfoGet(VGContext *clientContext,
                        VGImage handle,
                        BOOL *isOpenVGImage,
                        BOOL *isInUse,
                        signed int *width,
                        signed int *height,
                        BOOL *colorSpaceLinear,
                        BOOL *alphaFormatPre,
                        signed int *redSize,
                        signed int *greenSize,
                        signed int *blueSize,
                        signed int *alphaSize,
                        signed int *luminanceSize) {

    AMContext *context = (AMContext *) amCtxCurrentGet();

    AM_ASSERT(context);
    kdLogMessage("vgPrivImageInfoGet");
#endif

    if (!context || !context->initialized || amCtxHandleValid(context, handle) != AM_IMAGE_HANDLE_ID) {
        *isOpenVGImage = AM_FALSE;
        *isInUse = AM_FALSE;
        *width = 0;
        *height = 0;
        *colorSpaceLinear = AM_FALSE;
        *alphaFormatPre = AM_FALSE;
        *redSize = 0;
        *greenSize = 0;
        *blueSize = 0;
        *alphaSize = 0;
        *luminanceSize = 0;
#if !defined (TORCH_VG_SRC)
        *clientContext = NULL;
#endif
    }
    else {
        AMImage *img = (AMImage *)context->handles->createdHandlesList.data[handle];
        AMuint32 idx = AM_FMT_GET_INDEX(img->format);
        AMuint32 flags = pxlFormatTable[idx][FMT_FLAGS];

        *isOpenVGImage = AM_TRUE;
        // if reference counter > 1 or image has a father, the image is in use
        *isInUse = (img->referenceCounter > 1 || img->parent) ? AM_TRUE : AM_FALSE;
        *width = img->width;
        *height = img->height;
        *colorSpaceLinear = (flags & FMT_L) ? AM_TRUE : AM_FALSE;
        *alphaFormatPre = (flags & FMT_PRE) ? AM_TRUE : AM_FALSE;
        *redSize = pxlFormatTable[idx][FMT_R_BITS];
        *greenSize = pxlFormatTable[idx][FMT_G_BITS];
        *blueSize = pxlFormatTable[idx][FMT_B_BITS];
        *alphaSize = pxlFormatTable[idx][FMT_A_BITS];
        switch (img->format) {
            case VG_sL_8:
            case VG_lL_8:
                *luminanceSize = 8;
                break;
            case VG_BW_1:
                *luminanceSize = 1;
                break;
            default:
                *luminanceSize = 0;
                break;
        }
#if !defined (TORCH_VG_SRC)
        *clientContext = context;
#endif
    }
}

#if defined TORCH_VG_SRC
VG_API_CALL unsigned int VG_API_ENTRY vgPrivImageInUseByOpenVG(void *_context,
                                                               VGImage handle) VG_API_EXIT {
    AMContext *context = (AMContext *)_context;
    AM_ASSERT(context);
#else
BOOL vgPrivImageInUseByOpenVG(VGContext clientContext, VGImage handle) {

    AMContext *context = (AMContext *)clientContext;

    AM_ASSERT(context);
    kdLogMessage("vgPrivImageInUseByOpenVG");
#endif

    if (!context || !context->initialized || amCtxHandleValid(context, handle) != AM_IMAGE_HANDLE_ID)
        return AM_FALSE;
    else {
        AMImage *img = (AMImage *)context->handles->createdHandlesList.data[handle];
        return (img->referenceCounter > 1 || img->parent) ? AM_TRUE : AM_FALSE;
    }
}

#if defined TORCH_VG_SRC
VG_API_CALL void VG_API_ENTRY vgPrivImageLock(void *_context,
                                              VGImage handle,
                                              void *_surface) VG_API_EXIT {
    AMDrawingSurface *srf = (AMDrawingSurface *)_surface;
    AMContext *context = (AMContext *)_context;
#else
void vgPrivImageLock(VGContext clientContext,
                     VGImage handle,
                     VGSurface surface) {

    AMDrawingSurface *srf = (AMDrawingSurface *)surface;
    AMContext *context = (AMContext *)clientContext;
#endif
    AMImage *image;
    AMint32 w = srf->width;
    AMint32 h = srf->height;

    AM_ASSERT(context);
#if !defined (TORCH_VG_SRC)
    kdLogMessage("vgPrivImageLock");
#endif

    // Check if the handle is valid and if the image exists
    // NOTE: If the handle does not exist at this point that is ok since it was probably destroyed using vgDestroyImage
    if (!context || !context->initialized || amCtxHandleValid(context, handle) != AM_IMAGE_HANDLE_ID){
        return;
    }else{
        image = (AMImage *)context->handles->createdHandlesList.data[handle];
    }
    AM_ASSERT(image);
    AM_ASSERT(!image->inUseByEgl);
    AM_ASSERT(image->width == w);
    AM_ASSERT(image->height == h);

    amPxlMapConvert(&srf->pixels[w * (h - 1)], srf->realFormat, -srf->dataStride, 0, 0,
                    image->pixels, image->format, image->dataStride, 0, 0,
                    w, h, AM_FALSE, AM_TRUE);

    // NB: image cannot be parented with anyone, nor it can have children
    //amCtxHandleRefCounterInc(image);
    image->inUseByEgl = AM_TRUE;
}

#if defined TORCH_VG_SRC
VG_API_CALL void VG_API_ENTRY vgPrivImageUnlock(void *_context,
                                                VGImage handle,
                                                void *_surface) VG_API_EXIT {
    AMDrawingSurface *srf = (AMDrawingSurface *)_surface;
    AMContext *context = (AMContext *)_context;
#else
void vgPrivImageUnlock(VGContext clientContext,
                       VGImage handle,
                       VGSurface surface) {

    AMDrawingSurface *srf = (AMDrawingSurface *)surface;
    AMContext *context = (AMContext *)clientContext;
#endif
    AMImage *image;
    AMint32 w = srf->width;
    AMint32 h = srf->height;

    AM_ASSERT(context);
#if !defined (TORCH_VG_SRC)
    kdLogMessage("vgPrivImageUnlock");
#endif

    // Check if the handle is valid and if the image exists
    // NOTE: If the handle does not exist at this point that is ok since it was probably destroyed using vgDestroyImage
    if (!context || !context->initialized || amCtxHandleValid(context, handle) != AM_IMAGE_HANDLE_ID){
        return;
    }else{
        image = (AMImage *)context->handles->createdHandlesList.data[handle];
    }
    AM_ASSERT(image);
    AM_ASSERT(image->inUseByEgl);
    AM_ASSERT(image->width == w);
    AM_ASSERT(image->height == h);

    amPxlMapConvert(image->pixels, image->format, image->dataStride, 0, 0,
                    &srf->pixels[w * (h - 1)], srf->realFormat, -srf->dataStride, 0, 0,
                    w, h, AM_FALSE, AM_TRUE);
    //amCtxHandleRefCounterDec(image, context);
    image->inUseByEgl = AM_FALSE;
}

#endif

#if defined (RIM_VG_SRC)
#pragma pop
#endif

