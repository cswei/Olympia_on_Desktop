/*******************************************************
 *  EGL.c
 *
 *  Copyright 2007, Research In Motion Ltd.
 *
 *  Contains the EGL extensions
 *
 *  Russell Andrade, Feb 19, 2008
 *
 *******************************************************/

#if defined(RIM_NDK)
#include <egl_user_types.h>
#else
#include "bugdispc.h"
#include <log_verbosity.h>
#endif

#include <egl.h>
#include <egl_hw.h>
#include <eglext.h>
#include <egl_globals.h>
#include <i_eglext.h>
#include <KHR_thread_storage.h>

#if defined VECTOR_FRAMEWORK_AMANITH && defined RIM_OPENGLES_QC
#include "egl_amanithqc.h"
#elif defined RIM_EGL_MRVL
#include <egl_native.h>
#include <egl_mrvl.h>
#include <eglext_mrvl.h>
#elif defined ( RIM_EGL_VTMV )
#include <vtmv_eglext.h>
#elif defined VECTOR_FRAMEWORK_AMANITH
#include "egl_amanith.h"
#endif

#define SRCFILE     FILE_EGL_EXTENSIONS
#define SRCGROUP    GROUP_GRAPHICS

EGLBoolean      bSupportLockSurface = EGL_FALSE;
EGLBoolean      bSupportRegionHint  = EGL_FALSE;

PFNEGLLOCKSURFACEKHRPROC    pfnEGLLockSurfaceKHR = NULL;
PFNEGLUNLOCKSURFACEKHRPROC  pfnEGLUnlockSurfaceKHR = NULL;

#if defined( RIM_EGL_VTMV )

PFNEGLCREATEIMAGEKHRPROC pfnEGLCreateImageKHR = NULL;
PFNEGLDESTROYIMAGEKHRPROC pfnEGLDestroyImageKHR = NULL;

// These functions aren't yet being used in the driver
#if 0
PFNEGLCREATESYNCKHRPROC pfnEGLCreateSyncKHR = NULL;
PFNEGLDESTROYSYNCKHRPROC pfnEGLDestroySyncKHR = NULL;
PFNEGLCLIENTWAITSYNCKHRPROC pfnEGLClientWaitSyncKHR = NULL;
PFNEGLSIGNALSYNCKHRPROC pfnEGLSignalSyncKHR = NULL;
PFNEGLGETSYNCATTRIBKHRPROC pfnEGLGetSyncAttribKHR = NULL;
#endif // 0

#endif

//*****************************************************************************
// Name:            eglLockSurfaceKHR
// Description:     wrapper for eglLockSurfaceKHR
//*****************************************************************************
EGLBoolean eglLockSurfaceKHR (EGLDisplay display, EGLSurface surface, const EGLint *attrib_list)
{
    EGLDisplayContext *dpyContext = NULL;
    EGLSurfaceDescriptor * surfaceDesc = (EGLSurfaceDescriptor *)surface;
    EGLBoolean ret = EGL_FALSE;
    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( display );
    if(!dpyContext){
        WARN( "eglLockSurfaceKHR : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }
    if(!dpyContext->eglInitialized){
        WARN( "eglLockSurfaceKHR : display not initialized" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_FALSE;
    }

    PRINTN("Lock EGL surface KHR 0x%x", (DWORD)surface);

    //open up the front buffer
    if ( surfaceDesc->surfaceLocked ) {
        WARN("Surface already locked");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ACCESS));
        return EGL_FALSE;
    }

    if ((pfnEGLLockSurfaceKHR) && !(surfaceDesc->nativeRenderable) )
    {
        //calling vendor lock surface
        PRINT("Calling vendor eglLockSurfaceKHR");

        ret = pfnEGLLockSurfaceKHR(dpyContext->vendorDisplay, surfaceDesc->vendorSurface,
            attrib_list);
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVndr()));

        if ( ret == EGL_TRUE ) {
            surfaceDesc->surfaceLocked = TRUE;
        } else {
            PRINT("eglLockSurfaceKHR: egl lock failed");
        }
    } else if ( !(surfaceDesc->nativeRenderable) ) {
        // Should never get here.
        PRINT("Can't lock surface: no vendor function!");
        ret = FALSE;
    } else {
        surfaceDesc->surfaceLocked = TRUE;
        ret = TRUE;
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    }

    return ret;
}

//*****************************************************************************
// Name:            eglUnlockSurfaceKHR
// Description:     wrapper for eglLockSurfaceKHR
//*****************************************************************************
EGLBoolean eglUnlockSurfaceKHR (EGLDisplay display, EGLSurface surface)
{
    EGLBoolean ret = EGL_FALSE;
    EGLSurfaceDescriptor * surfaceDesc = (EGLSurfaceDescriptor *)surface;
    EGLDisplayContext *dpyContext = NULL;

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( display );
    if(!dpyContext){
        WARN( "eglUnlockSurfaceKHR : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    if(!dpyContext->eglInitialized){
        WARN( "eglUnlockSurfaceKHR : display not initialized" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_FALSE;
    }

    PRINT("Unlocking EGL surface KHR");

    if ( !surfaceDesc->surfaceLocked ) {
        PRINT("Surface not locked : cannot unlock");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ACCESS));
        return EGL_FALSE;
    }
    else {
        surfaceDesc->surfaceLocked = FALSE;
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
        ret = EGL_TRUE;
    }

    if ((pfnEGLUnlockSurfaceKHR) && !(surfaceDesc->nativeRenderable))
    {
        PRINT("Calling vendor eglUnlockSurfaceKHR");
        ret = pfnEGLUnlockSurfaceKHR(dpyContext->vendorDisplay, surfaceDesc->vendorSurface);
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVndr()));

        if ( ret == EGL_TRUE ) {
            surfaceDesc->surfaceLocked = FALSE;
        } else {
            PRINT("eglLockSurfaceKHR: egl unlock failed");
        }
    }

    return ret;
}


//*****************************************************************************
// Name:            InitExtension
// Description:     Initialization of EGL extensions
//*****************************************************************************
EGLBoolean eglInitExtension( void )
{
    PRINT("Initializing EGL extensions");
#if defined VECTOR_FRAMEWORK_AMANITH && defined RIM_OPENGLES_QC
    PRINT("Initializing AmanithQC extensions");
    pfnEGLLockSurfaceKHR    = (PFNEGLLOCKSURFACEKHRPROC)eglLockSurfaceKHRAmanithQC;
    pfnEGLUnlockSurfaceKHR  = (PFNEGLUNLOCKSURFACEKHRPROC)eglUnlockSurfaceKHRAmanithQC;
#elif defined ( RIM_EGL_MRVL )
    PRINT("Initializing MRVL extensions");
    pfnEGLLockSurfaceKHR    = (PFNEGLLOCKSURFACEKHRPROC)eglGetProcAddressMRVL("eglLockSurfaceKHR");
    pfnEGLUnlockSurfaceKHR  = (PFNEGLUNLOCKSURFACEKHRPROC)eglGetProcAddressMRVL("eglUnlockSurfaceKHR");
#elif defined ( VECTOR_FRAMEWORK_AMANITH )
    PRINT("Initializing Amanith extensions");
    pfnEGLLockSurfaceKHR    = (PFNEGLLOCKSURFACEKHRPROC)eglLockSurfaceKHRAmanith;
    pfnEGLUnlockSurfaceKHR  = (PFNEGLUNLOCKSURFACEKHRPROC)eglUnlockSurfaceKHRAmanith;
#elif defined ( RIM_EGL_VTMV )
    PRINT("Initializing VTMV extensions");
    pfnEGLLockSurfaceKHR    = (PFNEGLLOCKSURFACEKHRPROC)eglLockSurfaceKHRVTMV;
    pfnEGLUnlockSurfaceKHR  = (PFNEGLUNLOCKSURFACEKHRPROC)eglUnlockSurfaceKHRVTMV;

// These functions aren't yet being used in the driver
#if 0
    pfnEGLCreateSyncKHR = (PFNEGLCREATESYNCKHRPROC) eglCreateSyncKHRVTMV;
    pfnEGLDestroySyncKHR = (PFNEGLDESTROYSYNCKHRPROC) eglDestroySyncKHRVTMV;
    pfnEGLClientWaitSyncKHR = (PFNEGLCLIENTWAITSYNCKHRPROC)eglClientWaitSyncKHRVTMV;
    pfnEGLSignalSyncKHR = (PFNEGLSIGNALSYNCKHRPROC) eglSignalSyncKHRVTMV;
    pfnEGLGetSyncAttribKHR = (PFNEGLGETSYNCATTRIBKHRPROC) eglGetSyncAttribKHRVTMV;
#endif // 0

#endif
    return EGL_TRUE;
}

EGLBoolean eglSetDirtyRegionRim(EGLDisplay dpy, EGLSurface surface,
    NativeWinRect_t *dirtyRegion )
{
    EGLDisplayContext *dpyContext = NULL;

    if ( surface == EGL_NO_SURFACE ) {
        WARN("Current EGL Surface is EGL_NO_SURFACE");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglSetDirtyRegionRim : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    if(!dpyContext->eglInitialized){
        WARN( "eglSetDirtyRegionRim : display not initialized" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_FALSE;
    }

    // if surface is a valid pBuffer surface or a pixmap, then  set the lock region in there
    if ( ((EGLSurfaceDescriptor *)surface)->surfaceType == EGL_PIXMAP_BIT ||
        pBufferFrontSurface[dpyContext->id] == surface) {

        LV1( LOG_EGL,
            PRINT4N( "Setting dirty region to %d %d %d %d",
                dirtyRegion->x, dirtyRegion->y,
                dirtyRegion->width, dirtyRegion->height ) );
        ( ( EGLSurfaceDescriptor * )surface )->dirtyRegion = *dirtyRegion;
    }

    return EGL_TRUE;
}







