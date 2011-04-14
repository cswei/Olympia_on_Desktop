/*******************************************************
 *  egl_vendor.c
 *
 *  Copyright 2007, Research In Motion Ltd.
 *
 *  Contains the interface between rendering APIs such as
 *  openGL ES or openVG and underlying LCD
 *  Designed to conform with khronos EGL 1.2 standard
 *
 *  Russell Andrade, July 26, 2007
 *
 *******************************************************/
#if defined(RIM_NDK)
#include <egl_user_types.h>
#endif

#include <basetype.h>
#include <lcd.h>

#if !defined(RIM_NDK)
#include <i_lcd.h>
#include <device.h>
#include <lcd_if.h>
#endif

#include <egl.h>
#include <egl_hw.h>
#include <egl_globals.h>
#include <kd.h>

#if defined VECTOR_FRAMEWORK_AMANITH && defined RIM_OPENGLES_QC
#include "egl_amanithqc.h"
#elif defined RIM_EGL_MRVL && defined RIM_EGL_VTMV
    #include "egl_vendor_ex.h"
    #include "egl_vtmv.h"
    #include <egl_mrvl.h>
#elif defined RIM_EGL_MRVL
#include <egl_mrvl.h>
#elif defined RIM_USES_QCOMM_GRAPHICS
#include "egl_entry.h"
#elif defined VECTOR_FRAMEWORK_AMANITH
#include "egl_amanith.h"
#elif defined RIM_EGL_VTMV
#include "egl_vtmv.h"
#endif

#define EGL_FUNC_DEFINE_WRAPPER(_func_)  _func_##Wrap

#if defined VECTOR_FRAMEWORK_AMANITH && defined RIM_OPENGLES_QC
// Nothing needed

#elif defined RIM_EGL_MRVL

EGLBoolean eglInitializeMRVLWrap( EGLDisplay dpy, EGLint *major, EGLint *minor );
EGLDisplay eglGetDisplayMRVLWrap( NativeDisplayType display_id);
EGLBoolean eglCopyBuffersMRVLWrap( EGLDisplay dpy, EGLSurface surface,
                                   NativePixmapType target );
EGLBoolean eglSwapBuffersMRVLWrap( EGLDisplay dpy, EGLSurface surface );
EGLSurface eglCreatePixmapSurfaceMRVLWrap( EGLDisplay dpy, EGLConfig config,
                                           NativePixmapType pixmap,
                                           const EGLint *attrib_list );
EGLBoolean eglMakeCurrentMRVLWrap( EGLDisplay dpy, EGLSurface draw,
                                   EGLSurface read, EGLContext ctx );

#elif defined RIM_USES_QCOMM_GRAPHICS

EGLBoolean qceglInitializeWrap( EGLDisplay dpy, EGLint *major, EGLint *minor );
EGLBoolean qceglSwapBuffersWrap( EGLDisplay dpy, EGLSurface surface );
EGLBoolean qceglCopyBuffersWrap( EGLDisplay dpy, EGLSurface surface,
                                 NativePixmapType target );
EGLSurface qceglCreatePixmapSurfaceWrap( EGLDisplay dpy, EGLConfig config,
                                         NativePixmapType pixmap,
                                         const EGLint *attrib_list );
EGLBoolean qceglMakeCurrentWrap( EGLDisplay dpy, EGLSurface draw,
                                 EGLSurface read, EGLContext ctx);
#elif defined VECTOR_FRAMEWORK_AMANITH

// Nothing needed

#endif

EGLint (*eglGetErrorVndr)( void );
EGLDisplay (*eglGetDisplayVndr)(NativeDisplayType display_id);
EGLBoolean (*eglInitializeVndr)(EGLDisplay dpy, EGLint *major, EGLint *minor);
EGLBoolean (*eglTerminateVndr)(EGLDisplay dpy);
EglStr (*eglQueryStringVndr)(EGLDisplay dpy, EGLint name);
EGLBoolean (*eglGetConfigsVndr)(EGLDisplay dpy, EGLConfig *configs,
        EGLint config_size, EGLint *num_config);
EGLBoolean (*eglChooseConfigVndr)(EGLDisplay dpy, const EGLint *attrib_list,
        EGLConfig *configs, EGLint config_size,
        EGLint *num_config);
EGLBoolean (*eglGetConfigAttribVndr)(EGLDisplay dpy, EGLConfig config,
        EGLint attribute, EGLint *value);
EGLSurface (*eglCreateWindowSurfaceVndr)(EGLDisplay dpy, EGLConfig config,
        NativeWindowType win,
        const EGLint *attrib_list);
EGLSurface (*eglCreatePbufferSurfaceVndr)(EGLDisplay dpy, EGLConfig config,
        const EGLint *attrib_list);
EGLSurface (*eglCreatePixmapSurfaceVndr)(EGLDisplay dpy, EGLConfig config,
        NativePixmapType pixmap,
        const EGLint *attrib_list);
EGLBoolean (*eglDestroySurfaceVndr)(EGLDisplay dpy, EGLSurface surface);
EGLBoolean (*eglQuerySurfaceVndr)(EGLDisplay dpy, EGLSurface surface,
        EGLint attribute, EGLint *value);
EGLBoolean (*eglBindAPIVndr)(EGLenum api);
EGLenum (*eglQueryAPIVndr)(void);
EGLBoolean (*eglWaitClientVndr)(void);
EGLBoolean (*eglReleaseThreadVndr)(void);
EGLSurface (*eglCreatePbufferFromClientBufferVndr)(
        EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer,
        EGLConfig config, const EGLint *attrib_list);
EGLBoolean (*eglSurfaceAttribVndr)(EGLDisplay dpy, EGLSurface surface,
        EGLint attribute, EGLint value);
EGLBoolean (*eglBindTexImageVndr)(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
EGLBoolean (*eglReleaseTexImageVndr)(EGLDisplay dpy, EGLSurface surface, EGLint buffer);

EGLBoolean (*eglSwapIntervalVndr)(EGLDisplay dpy, EGLint interval);
EGLContext (*eglCreateContextVndr)(EGLDisplay dpy, EGLConfig config,
        EGLContext share_context,
        const EGLint *attrib_list);
EGLBoolean (*eglDestroyContextVndr)(EGLDisplay dpy, EGLContext ctx);
EGLBoolean (*eglMakeCurrentVndr)(EGLDisplay dpy, EGLSurface draw,
        EGLSurface read, EGLContext ctx);
EGLContext (*eglGetCurrentContextVndr)(void);
EGLSurface (*eglGetCurrentSurfaceVndr)(EGLint readdraw);
EGLDisplay (*eglGetCurrentDisplayVndr)(void);
EGLBoolean (*eglQueryContextVndr)(EGLDisplay dpy, EGLContext ctx,
        EGLint attribute, EGLint *value);
EGLBoolean (*eglWaitGLVndr)(void);
EGLBoolean (*eglWaitNativeVndr)(EGLint engine);
EGLBoolean (*eglSwapBuffersVndr)(EGLDisplay dpy, EGLSurface surface);
EGLBoolean (*eglCopyBuffersVndr)(EGLDisplay dpy, EGLSurface surface,
        NativePixmapType target);

void EglSetupFunctions( void )
{
#if defined VECTOR_FRAMEWORK_AMANITH && defined RIM_OPENGLES_QC

    eglBindAPIVndr = eglBindAPIAmanithQC;
    eglBindTexImageVndr = eglBindTexImageAmanithQC;
    eglChooseConfigVndr = eglChooseConfigAmanithQC;
    eglCopyBuffersVndr = eglCopyBuffersAmanithQC;
    eglCreateContextVndr = eglCreateContextAmanithQC;
    eglCreatePbufferFromClientBufferVndr = eglCreatePbufferFromClientBufferAmanithQC;
    eglCreatePbufferSurfaceVndr = eglCreatePbufferSurfaceAmanithQC;
    eglCreatePixmapSurfaceVndr = eglCreatePixmapSurfaceAmanithQC;
    eglCreateWindowSurfaceVndr = eglCreateWindowSurfaceAmanithQC;
    eglDestroyContextVndr = eglDestroyContextAmanithQC;
    eglDestroySurfaceVndr = eglDestroySurfaceAmanithQC;
    eglGetConfigAttribVndr = eglGetConfigAttribAmanithQC;
    eglGetConfigsVndr = eglGetConfigsAmanithQC;
    eglGetCurrentContextVndr = (EGLContext(*) (void))eglFuncNotInitialized;
    eglGetCurrentDisplayVndr = (EGLDisplay(*) (void)) eglFuncNotInitialized;
    eglGetCurrentSurfaceVndr = (EGLSurface(*) (EGLint))eglFuncNotInitialized;
    eglGetDisplayVndr = eglGetDisplayAmanithQC;
    eglGetErrorVndr = eglGetErrorAmanithQC;
    eglInitializeVndr = eglInitializeAmanithQC;
    eglMakeCurrentVndr = eglMakeCurrentAmanithQC;
    eglQueryAPIVndr = eglQueryAPIAmanithQC;
    eglQueryContextVndr = eglQueryContextAmanithQC;
    eglQueryStringVndr = eglQueryStringAmanithQC;
    eglQuerySurfaceVndr = eglQuerySurfaceAmanithQC;
    eglReleaseTexImageVndr = eglReleaseTexImageAmanithQC;
    eglReleaseThreadVndr = eglReleaseThreadAmanithQC;
    eglSurfaceAttribVndr = eglSurfaceAttribAmanithQC;
    eglSwapBuffersVndr = eglSwapBuffersAmanithQC;
    eglSwapIntervalVndr = (EGLBoolean(*) (EGLDisplay, EGLint))eglFuncNotInitialized;
    eglTerminateVndr = eglTerminateAmanithQC;
    eglWaitClientVndr = (EGLBoolean(*) (void))eglFuncNotInitialized;
    eglWaitGLVndr = (EGLBoolean(*) (void))eglFuncNotInitialized;
    eglWaitNativeVndr = (EGLBoolean(*) (EGLint))eglFuncNotInitialized;

#elif defined RIM_EGL_MRVL && defined RIM_EGL_VTMV
    #if defined RIM_OPENVG_VTMV
        #error "Can't have both MRVL and VTMV OpenVG"
    #endif
    #if !defined RIM_OPENVG_MRVL
        #error "expecting MRVL VG"
    #endif

    EglInitVndrEx();

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_GET_ERROR, eglGetErrorVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_GET_ERROR, eglGetErrorVTMV, "eglGetErrorVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_GET_ERROR, eglGetErrorMRVL, "eglGetErrorMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_GET_DISPLAY, eglGetDisplayVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_GET_DISPLAY, eglGetDisplayVTMVWrap, "eglGetDisplayVTMVWrap" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_GET_DISPLAY, eglGetDisplayMRVLWrap, "eglGetDisplayMRVLWrap" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_INITIALIZE, eglInitializeVndrMrvlVtmv );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_TERMINATE, eglTerminateVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_TERMINATE, eglTerminateVTMV, "eglTerminateVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_TERMINATE, eglTerminateMRVL, "eglTerminateMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_QUERY_STRING, eglQueryStringVndrMrvlVtmv );
    //EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_QUERY_STRING, eglQueryStringVndrEx );
    //EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_QUERY_STRING, eglQueryStringVTMV, "eglQueryStringVTMV" );
    //EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_QUERY_STRING, eglQueryStringMRVL, "eglQueryStringMRVL" );

    // do nothing funcs
    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_GET_CONFIGS, eglGetConfigsVndrEx );
    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_CHOOSE_CONFIG, eglChooseConfigVndrEx );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_GET_CONFIG_ATTRIB, eglGetConfigAttribVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_GET_CONFIG_ATTRIB, eglGetConfigAttribVTMV, "eglGetConfigAttribVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_GET_CONFIG_ATTRIB, eglGetConfigAttribMRVL, "eglGetConfigAttribMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_CREATE_WINDOW_SURFACE, eglCreateWindowSurfaceVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_CREATE_WINDOW_SURFACE, eglCreateWindowSurfaceVTMV, "eglCreateWindowSurfaceVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_CREATE_WINDOW_SURFACE, eglCreateWindowSurfaceMRVL, "eglCreateWindowSurfaceMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_CREATE_PBUFFER_SURFACE, eglCreatePbufferSurfaceVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_CREATE_PBUFFER_SURFACE, eglCreatePbufferSurfaceVTMV, "eglCreatePbufferSurfaceVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_CREATE_PBUFFER_SURFACE, eglCreatePbufferSurfaceMRVL, "eglCreatePbufferSurfaceMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_CREATE_PIXMAP_SURFACE, eglCreatePixmapSurfaceVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_CREATE_PIXMAP_SURFACE, eglCreatePixmapSurfaceVTMV, "eglCreatePixmapSurfaceVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_CREATE_PIXMAP_SURFACE, eglCreatePixmapSurfaceMRVLWrap, "eglCreatePixmapSurfaceMRVLWrap" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_DESTROY_SURFACE, eglDestroySurfaceVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_DESTROY_SURFACE, eglDestroySurfaceVTMV, "eglDestroySurfaceVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_DESTROY_SURFACE, eglDestroySurfaceMRVL, "eglDestroySurfaceMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_QUERY_SURFACE, eglQuerySurfaceVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_QUERY_SURFACE, eglQuerySurfaceVTMV, "eglQuerySurfaceVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_QUERY_SURFACE, eglQuerySurfaceMRVL, "eglQuerySurfaceMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_BIND_API, eglBindAPIVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_BIND_API, eglBindAPIVTMV, "eglBindAPIVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_BIND_API, eglBindAPIMRVL, "eglBindAPIMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_QUERY_API, eglQueryAPIVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_QUERY_API, eglQueryAPIVTMV, "eglQueryAPIVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_QUERY_API, eglQueryAPIMRVL, "eglQueryAPIMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_WAIT_CLIENT, eglWaitClientVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_WAIT_CLIENT, eglWaitClientVTMV, "eglWaitClientVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_WAIT_CLIENT, eglWaitClientMRVL, "eglWaitClientMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_RELEASE_THREAD, eglReleaseThreadVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_RELEASE_THREAD, eglReleaseThreadVTMV, "eglReleaseThreadVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_RELEASE_THREAD, eglReleaseThreadMRVL, "eglReleaseThreadMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_CREATE_PBUFFER_FROM_CLIENT_BUFFER, eglCreatePbufferFromClientBufferVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_CREATE_PBUFFER_FROM_CLIENT_BUFFER, eglCreatePbufferFromClientBufferVTMV, "eglCreatePbufferFromClientBufferVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_CREATE_PBUFFER_FROM_CLIENT_BUFFER, eglCreatePbufferFromClientBufferMRVL, "eglCreatePbufferFromClientBufferMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_SURFACE_ATTRIB, eglSurfaceAttribVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_SURFACE_ATTRIB, eglSurfaceAttribVTMV, "eglSurfaceAttribVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_SURFACE_ATTRIB, eglSurfaceAttribMRVL, "eglSurfaceAttribMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_BIND_TEX_IMAGE, eglBindTexImageVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_BIND_TEX_IMAGE, eglBindTexImageVTMV, "eglBindTexImageVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_BIND_TEX_IMAGE, eglBindTexImageMRVL, "eglBindTexImageMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_RELEASE_TEX_IMAGE, eglReleaseTexImageVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_RELEASE_TEX_IMAGE, eglReleaseTexImageVTMV, "eglReleaseTexImageVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_RELEASE_TEX_IMAGE, eglReleaseTexImageMRVL, "eglReleaseTexImageMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_SWAP_INTERVAL, eglSwapIntervalVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_SWAP_INTERVAL, eglSwapIntervalVTMV, "eglSwapIntervalVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_SWAP_INTERVAL, eglSwapIntervalMRVL, "eglSwapIntervalMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_CREATE_CONTEXT, eglCreateContextVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_CREATE_CONTEXT, eglCreateContextVTMV, "eglCreateContextVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_CREATE_CONTEXT, eglCreateContextMRVL, "eglCreateContextMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_DESTROY_CONTEXT, eglDestroyContextVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_DESTROY_CONTEXT, eglDestroyContextVTMV, "eglDestroyContextVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_DESTROY_CONTEXT, eglDestroyContextMRVL, "eglDestroyContextMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_MAKE_CURRENT, eglMakeCurrentVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_MAKE_CURRENT, eglMakeCurrentVTMV, "eglMakeCurrentVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_MAKE_CURRENT, eglMakeCurrentMRVL, "eglMakeCurrentMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_GET_CURRENT_CONTEXT, eglGetCurrentContextVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_GET_CURRENT_CONTEXT, eglGetCurrentContextVTMV, "eglGetCurrentContextVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_GET_CURRENT_CONTEXT, eglGetCurrentContextMRVL, "eglGetCurrentContextMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_GET_CURRENT_SURFACE, eglGetCurrentSurfaceVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_GET_CURRENT_SURFACE, eglGetCurrentSurfaceVTMV, "eglGetCurrentSurfaceVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_GET_CURRENT_SURFACE, eglGetCurrentSurfaceMRVL, "eglGetCurrentSurfaceMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_GET_CURRENT_DISPLAY, eglGetCurrentDisplayVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_GET_CURRENT_DISPLAY, eglGetCurrentDisplayVTMV, "eglGetCurrentDisplayVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_GET_CURRENT_DISPLAY, eglGetCurrentDisplayMRVL, "eglGetCurrentDisplayMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_QUERY_CONTEXT, eglQueryContextVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_QUERY_CONTEXT, eglQueryContextVTMV, "eglQueryContextVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_QUERY_CONTEXT, eglQueryContextMRVL, "eglQueryContextMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_WAIT_GL, eglWaitGLVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_WAIT_GL, eglWaitGLVTMV, "eglWaitGLVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_WAIT_GL, eglWaitGLMRVL, "eglWaitGLMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_WAIT_NATIVE, eglWaitNativeVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_WAIT_NATIVE, eglWaitNativeVTMV, "eglWaitNativeVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_WAIT_NATIVE, eglWaitNativeMRVL, "eglWaitNativeMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_SWAP_BUFFERS, eglSwapBuffersVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_SWAP_BUFFERS, eglSwapBuffersVTMV, "eglSwapBuffersVTMV" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_SWAP_BUFFERS, eglSwapBuffersMRVL, "eglSwapBuffersMRVL" );

    EGL_REGISTER_VNDREX_ROOTFUNC( EGL_VENDOREX_COPY_BUFFERS, eglCopyBuffersVndrEx );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_GLES, EGL_VENDOREX_COPY_BUFFERS, eglCopyBuffersVTMVWrap, "eglCopyBuffersVTMVWrap" );
    EGL_REGISTER_VNDREX_APIFUNC( EGL_VENDOREX_API_VG, EGL_VENDOREX_COPY_BUFFERS, eglCopyBuffersMRVLWrap, "eglCopyBuffersMRVLWrap" );
    
#elif defined RIM_EGL_MRVL

    eglBindAPIVndr = eglBindAPIMRVL;
    eglBindTexImageVndr = eglBindTexImageMRVL;
    eglChooseConfigVndr = eglChooseConfigMRVL;
    eglCopyBuffersVndr = EGL_FUNC_DEFINE_WRAPPER(eglCopyBuffersMRVL);
    eglCreateContextVndr = eglCreateContextMRVL;
    eglCreatePbufferFromClientBufferVndr = eglCreatePbufferFromClientBufferMRVL;
    eglCreatePbufferSurfaceVndr = eglCreatePbufferSurfaceMRVL;
    eglCreatePixmapSurfaceVndr = EGL_FUNC_DEFINE_WRAPPER(eglCreatePixmapSurfaceMRVL);
    eglCreateWindowSurfaceVndr = (EGLSurface(*)(EGLDisplay, EGLConfig, NativeWindowType,
        const EGLint *))eglCreateWindowSurfaceMRVL;
    eglDestroyContextVndr = eglDestroyContextMRVL;
    eglDestroySurfaceVndr = eglDestroySurfaceMRVL;
    eglGetConfigAttribVndr = eglGetConfigAttribMRVL;
    eglGetConfigsVndr = eglGetConfigsMRVL;
    eglGetCurrentContextVndr = eglGetCurrentContextMRVL;
    eglGetCurrentDisplayVndr = eglGetCurrentDisplayMRVL;
    eglGetCurrentSurfaceVndr = eglGetCurrentSurfaceMRVL;
    eglGetDisplayVndr = eglGetDisplayMRVLWrap;
    eglGetErrorVndr = eglGetErrorMRVL;
    eglInitializeVndr = EGL_FUNC_DEFINE_WRAPPER(eglInitializeMRVL);
    eglMakeCurrentVndr = EGL_FUNC_DEFINE_WRAPPER(eglMakeCurrentMRVL);
    eglQueryAPIVndr = eglQueryAPIMRVL;
    eglQueryContextVndr = eglQueryContextMRVL;
    eglQueryStringVndr = eglQueryStringMRVL;
    eglQuerySurfaceVndr = eglQuerySurfaceMRVL;
    eglReleaseTexImageVndr = eglReleaseTexImageMRVL;
    eglReleaseThreadVndr = eglReleaseThreadMRVL;
    eglSurfaceAttribVndr = eglSurfaceAttribMRVL;
    eglSwapBuffersVndr = EGL_FUNC_DEFINE_WRAPPER(eglSwapBuffersMRVL);
    eglSwapIntervalVndr = eglSwapIntervalMRVL;
    eglTerminateVndr = eglTerminateMRVL;
    eglWaitClientVndr = eglWaitClientMRVL;
    eglWaitGLVndr = eglWaitGLMRVL;
    eglWaitNativeVndr = eglWaitNativeMRVL;

#elif defined RIM_USES_QCOMM_GRAPHICS

    eglBindAPIVndr = qceglBindAPI;
    eglBindTexImageVndr = qceglBindTexImage;
    eglChooseConfigVndr = qceglChooseConfig;
    eglCopyBuffersVndr = EGL_FUNC_DEFINE_WRAPPER(qceglCopyBuffers);
    eglCreateContextVndr = qceglCreateContext;
    eglCreatePbufferFromClientBufferVndr = qceglCreatePbufferFromClientBuffer;
    eglCreatePbufferSurfaceVndr = qceglCreatePbufferSurface;
    eglCreatePixmapSurfaceVndr = EGL_FUNC_DEFINE_WRAPPER(qceglCreatePixmapSurface);
    eglCreateWindowSurfaceVndr = qceglCreateWindowSurface;
    eglDestroyContextVndr = qceglDestroyContext;
    eglDestroySurfaceVndr = qceglDestroySurface;
    eglGetConfigAttribVndr = qceglGetConfigAttrib;
    eglGetConfigsVndr = qceglGetConfigs;
    eglGetCurrentContextVndr = qceglGetCurrentContext;
    eglGetCurrentDisplayVndr = qceglGetCurrentDisplay;
    eglGetCurrentSurfaceVndr = qceglGetCurrentSurface;
    eglGetDisplayVndr = qceglGetDisplay;
    eglGetErrorVndr = qceglGetError;
    eglInitializeVndr = EGL_FUNC_DEFINE_WRAPPER(qceglInitialize);
    eglMakeCurrentVndr = EGL_FUNC_DEFINE_WRAPPER(qceglMakeCurrent);
    eglQueryAPIVndr = qceglQueryAPI;
    eglQueryContextVndr = qceglQueryContext;
    eglQueryStringVndr = qceglQueryString;
    eglQuerySurfaceVndr = qceglQuerySurface;
    eglReleaseTexImageVndr = qceglReleaseTexImage;
    eglReleaseThreadVndr = qceglReleaseThread;
    eglSurfaceAttribVndr = qceglSurfaceAttrib;
    eglSwapBuffersVndr = EGL_FUNC_DEFINE_WRAPPER(qceglSwapBuffers);
    eglSwapIntervalVndr = qceglSwapInterval;
    eglTerminateVndr = qceglTerminate;
    eglWaitClientVndr = qceglWaitClient;
    eglWaitGLVndr = qceglWaitGL;
    eglWaitNativeVndr = qceglWaitNative;

#elif defined VECTOR_FRAMEWORK_AMANITH

    eglBindAPIVndr = eglBindAPIAmanith;
    eglBindTexImageVndr = eglBindTexImageAmanith;
    eglChooseConfigVndr = eglChooseConfigAmanith;
    eglCopyBuffersVndr = eglCopyBuffersAmanith;
    eglCreateContextVndr = eglCreateContextAmanith;
    eglCreatePbufferFromClientBufferVndr = eglCreatePbufferFromClientBufferAmanith;
    eglCreatePbufferSurfaceVndr = eglCreatePbufferSurfaceAmanith;
    eglCreatePixmapSurfaceVndr = eglCreatePixmapSurfaceAmanith;
    eglCreateWindowSurfaceVndr = eglCreateWindowSurfaceAmanith;
    eglDestroyContextVndr = eglDestroyContextAmanith;
    eglDestroySurfaceVndr = eglDestroySurfaceAmanith;
    eglGetConfigAttribVndr = eglGetConfigAttribAmanith;
    eglGetConfigsVndr = eglGetConfigsAmanith;
    eglGetCurrentContextVndr = eglGetCurrentContextAmanith;
    eglGetCurrentDisplayVndr = eglGetCurrentDisplayAmanith;
    eglGetCurrentSurfaceVndr = eglGetCurrentSurfaceAmanith;
    eglGetDisplayVndr = eglGetDisplayAmanith;
    eglGetErrorVndr = eglGetErrorAmanith;
    eglInitializeVndr = eglInitializeAmanith;
    eglMakeCurrentVndr = eglMakeCurrentAmanith;
    eglQueryAPIVndr = eglQueryAPIAmanith;
    eglQueryContextVndr = eglQueryContextAmanith;
    eglQueryStringVndr = eglQueryStringAmanith;
    eglQuerySurfaceVndr = eglQuerySurfaceAmanith;
    eglReleaseTexImageVndr = eglReleaseTexImageAmanith;
    eglReleaseThreadVndr = eglReleaseThreadAmanith;
    eglSurfaceAttribVndr = eglSurfaceAttribAmanith;
    eglSwapBuffersVndr = eglSwapBuffersAmanith;
    eglSwapIntervalVndr = eglSwapIntervalAmanith;
    eglTerminateVndr = eglTerminateAmanith;
    eglWaitClientVndr = eglWaitClientAmanith;
    eglWaitGLVndr = eglWaitGLAmanith;
    eglWaitNativeVndr = eglWaitNativeAmanith;

#elif defined RIM_EGL_VTMV

    eglBindAPIVndr = eglBindAPIVTMV;
    eglBindTexImageVndr = eglBindTexImageVTMV;
    eglChooseConfigVndr = eglChooseConfigVTMV;
    eglCopyBuffersVndr = eglCopyBuffersVTMVWrap;
    eglCreateContextVndr = eglCreateContextVTMV;
    eglCreatePbufferFromClientBufferVndr = eglCreatePbufferFromClientBufferVTMV;
    eglCreatePbufferSurfaceVndr = eglCreatePbufferSurfaceVTMV;
    eglCreatePixmapSurfaceVndr = eglCreatePixmapSurfaceVTMV;
    eglCreateWindowSurfaceVndr = eglCreateWindowSurfaceVTMV;
    eglDestroyContextVndr = eglDestroyContextVTMV;
    eglDestroySurfaceVndr = eglDestroySurfaceVTMV;
    eglGetConfigAttribVndr = eglGetConfigAttribVTMV;
    eglGetConfigsVndr = eglGetConfigsVTMV;
    eglGetCurrentContextVndr = eglGetCurrentContextVTMV;
    eglGetCurrentDisplayVndr = eglGetCurrentDisplayVTMV;
    eglGetCurrentSurfaceVndr = eglGetCurrentSurfaceVTMV;
    eglGetDisplayVndr = eglGetDisplayVTMVWrap;
    eglGetErrorVndr = eglGetErrorVTMV;
    eglInitializeVndr = eglInitializeVTMVWrap;
    eglMakeCurrentVndr = eglMakeCurrentVTMVWrap;
    eglQueryAPIVndr = eglQueryAPIVTMV;
    eglQueryContextVndr = eglQueryContextVTMV;
    eglQueryStringVndr = eglQueryStringVTMV;
    eglQuerySurfaceVndr = eglQuerySurfaceVTMV;
    eglReleaseTexImageVndr = eglReleaseTexImageVTMV;
    eglReleaseThreadVndr = eglReleaseThreadVTMV;
    eglSurfaceAttribVndr = eglSurfaceAttribVTMV;
    eglSwapBuffersVndr = eglSwapBuffersVTMVWrap;
    eglSwapIntervalVndr = eglSwapIntervalVTMV;
    eglTerminateVndr = eglTerminateVTMV;
    eglWaitClientVndr = eglWaitClientVTMV;
    eglWaitGLVndr = eglWaitGLVTMV;
    eglWaitNativeVndr = eglWaitNativeVTMV;

#else
    eglBindAPIVndr = (EGLBoolean(*) ( EGLenum))eglFuncNotInitialized;
    eglBindTexImageVndr = (EGLBoolean(*) (EGLDisplay, EGLSurface, EGLint))eglFuncNotInitialized;
    eglChooseConfigVndr = (EGLBoolean(*) (EGLDisplay, const EGLint *,
        EGLConfig *, EGLint, EGLint *))eglFuncNotInitialized;
    eglCopyBuffersVndr = (EGLBoolean(*) (EGLDisplay dpy, EGLSurface surface,
        NativePixmapType target))eglFuncNotInitialized;
    eglCreateContextVndr = (EGLContext(*) (EGLDisplay, EGLConfig, EGLContext,
        const EGLint *attrib_list)) eglFuncNotInitialized;
    eglCreatePbufferFromClientBufferVndr = (EGLSurface(*) ( EGLDisplay, EGLenum,
        EGLClientBuffer, EGLConfig, const EGLint *))eglFuncNotInitialized;
    eglCreatePbufferSurfaceVndr = (EGLSurface(*) (EGLDisplay, EGLConfig,
        const EGLint *))eglFuncNotInitialized;
    eglCreatePixmapSurfaceVndr = (EGLSurface(*) (EGLDisplay, EGLConfig,
        NativePixmapType, const EGLint *))eglFuncNotInitialized;
    eglCreateWindowSurfaceVndr = (EGLSurface(*) (EGLDisplay, EGLConfig,
        NativeWindowType, const EGLint *))eglFuncNotInitialized;
    eglDestroyContextVndr = (EGLBoolean(*) (EGLDisplay, EGLContext))
        eglFuncNotInitialized;
    eglDestroySurfaceVndr = (EGLBoolean(*) (EGLDisplay, EGLSurface))
        eglFuncNotInitialized;
    eglGetConfigAttribVndr = (EGLBoolean(*) (EGLDisplay, EGLConfig,
        EGLint, EGLint *))eglFuncNotInitialized;
    eglGetConfigsVndr = (EGLBoolean(*) (EGLDisplay, EGLConfig *,
        EGLint, EGLint *))eglFuncNotInitialized;
    eglGetCurrentContextVndr = (EGLContext(*) (void))eglFuncNotInitialized;
    eglGetCurrentDisplayVndr = (EGLDisplay(*) (void)) eglFuncNotInitialized;
    eglGetCurrentSurfaceVndr = (EGLSurface(*) (EGLint))eglFuncNotInitialized;
    eglGetDisplayVndr = (EGLDisplay(*) (NativeDisplayType))eglFuncNotInitialized;
    eglGetErrorVndr = (EGLint(*) (void)) eglFuncNotInitialized;
    eglInitializeVndr = (EGLBoolean(*) (EGLDisplay, EGLint *, EGLint *))
        eglFuncNotInitialized;
    eglMakeCurrentVndr = (EGLBoolean(*) (EGLDisplay, EGLSurface,
        EGLSurface, EGLContext))eglFuncNotInitialized;
    eglQueryAPIVndr = (EGLenum(*) (void))eglFuncNotInitialized;
    eglQueryContextVndr = (EGLBoolean(*) (EGLDisplay, EGLContext,
        EGLint, EGLint *))eglFuncNotInitialized;
    eglQuerySurfaceVndr = (EGLBoolean(*) (EGLDisplay, EGLSurface,
        EGLint, EGLint *))eglFuncNotInitialized;
    eglReleaseTexImageVndr = (EGLBoolean(*) (EGLDisplay, EGLSurface, EGLint))
        eglFuncNotInitialized;
    eglReleaseThreadVndr = (EGLBoolean(*) (void))eglFuncNotInitialized;
    eglSurfaceAttribVndr = (EGLBoolean(*) (EGLDisplay, EGLSurface,
        EGLint, EGLint))eglFuncNotInitialized;
    eglSwapBuffersVndr = (EGLBoolean(*) (EGLDisplay, EGLSurface))eglFuncNotInitialized;
    eglSwapIntervalVndr = (EGLBoolean(*) (EGLDisplay, EGLint))eglFuncNotInitialized;
    eglTerminateVndr = (EGLBoolean(*) (EGLDisplay))eglFuncNotInitialized;
    eglWaitClientVndr = (EGLBoolean(*) (void))eglFuncNotInitialized;
    eglWaitGLVndr = (EGLBoolean(*) (void))eglFuncNotInitialized;
    eglWaitNativeVndr = (EGLBoolean(*) (EGLint))eglFuncNotInitialized;
    eglReleaseThreadVndr = (EGLBoolean(*) (void))eglFuncNotInitialized;

#endif

}

void eglUpdateVendorSurface(EGLSurfaceDescriptor *surface)
{
#if defined ( VECTOR_FRAMEWORK_AMANITH ) && defined ( RIM_OPENGLES_QC )
    ((EGLVendorSurfaceAmanithQC *)surface->vendorSurface)->parentSurface = surface;
#endif
}


void eglFuncNotInitialized( void )
{
    //WARN("Invoking uninitialized egl function");
#if defined RIM_USES_QCOMM_GRAPHICS
    kdCatfailRim("Invoking uninitialized egl function in EGL");
#endif
}



