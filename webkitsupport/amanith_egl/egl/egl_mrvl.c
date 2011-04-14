/*******************************************************
 *  egl_mrvl.c
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
//#define EGL_EXTRA_CONFIG_DEBUG

#if defined(RIM_NDK)
#include <egl_user_types.h>
#include <egl_mrvl.h>
#include <unistd.h>
#include <string.h>
#else
#include <bugdispc.h>
#include <failure.h>
#include <graphics_mempool.h>
#include <i_graphics_mempool.h>
#include <log_verbosity.h>
#include <string.h>
#include <i_lcd.h>
#endif

#include <bitmap.h>

#include <egl.h>
#include <egl_native.h>
#include <egl_globals.h>
#include <config_egl.h>
#include <eglext_mrvl.h>
#include <egl_mrvl.h>
#include <i_eglext.h>
#include <i_egl.h> 
#include <kd.h>

#if defined(RIM_WINDOW_MANAGER)
#include "windowmanager.h"
#include "i_windowmanager.h"
#endif

#include "egl_vendor_ex.h"
#include <egl_hw.h>

#define SRCFILE     FILE_EGL_MRVL
#define SRCGROUP    GROUP_GRAPHICS

extern void initializeSurface( EGLDisplay dpy, EGLSurfaceDescriptor *newEglSurface,
    BYTE surfaceType );

EGLBoolean eglInitializeMRVLWrap( EGLDisplay dpy, EGLint *major, EGLint *minor )
{
    EGLDisplayContext *dpyContext = NULL;
    EGLBoolean retBool = EGL_TRUE;
    DWORD       i;
    DWORD       j;
    EGLint      num_config;
    EGLConfig   tempConfig[40];
    EGLint      renderableType = 0;

    EGLint      attribList[]={
        EGL_BUFFER_SIZE,                    0,
        EGL_ALPHA_SIZE,                     0,
        EGL_BLUE_SIZE,                      0,
        EGL_GREEN_SIZE,                     0,
        EGL_RED_SIZE,                       0,
        EGL_DEPTH_SIZE,                     0,
        EGL_STENCIL_SIZE,                   0,
        EGL_CONFIG_CAVEAT,                  0,
        EGL_NATIVE_VISUAL_TYPE,             0,
        EGL_SAMPLES,                        0,
        EGL_SAMPLE_BUFFERS,                 0,
        EGL_SURFACE_TYPE,                   0,
        EGL_TRANSPARENT_TYPE,               0,
        EGL_TRANSPARENT_BLUE_VALUE,         0,
        EGL_TRANSPARENT_GREEN_VALUE,        0,
        EGL_TRANSPARENT_RED_VALUE,          0,
        EGL_MIN_SWAP_INTERVAL,              0,
        EGL_MAX_SWAP_INTERVAL,              0,
        EGL_LUMINANCE_SIZE,                 0,
        EGL_ALPHA_MASK_SIZE,                0,
        EGL_COLOR_BUFFER_TYPE,              0,
        EGL_RENDERABLE_TYPE,                EGL_OPENGL_ES_BIT | EGL_OPENVG_BIT | EGL_OPENGL_ES2_BIT,
        //EGL_CONFORMANT,                      EGL_OPENGL_ES_BIT | EGL_OPENVG_BIT | EGL_OPENGL_ES2_BIT,
        EGL_NONE
        };

    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglInitializeMRVLWrap : bad display" ); 
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    retBool = eglInitializeMRVL( dpyContext->vendorDisplay, major, minor );
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorMRVL()));
    

    if(retBool){
        //Initialize Marvell Extensions to match our function pointers
        eglInitExtension();
    }else{
        return retBool;
    }

    //get all of them
#if defined (EGL_EXTRA_CONFIG_DEBUG)
    printAllVendorConfigs(dpyContext->vendorDisplay);
#endif

    if ( eglUseNewChooseConfigVendor ) {
        retBool = eglGenericChooseConfigVendor( dpy, dpyContext->vendorDisplay, attribList, eglGetConfigsVndr, eglGetConfigAttribVndr, NULL, NULL );
    } else {
        // clear 
        for(i=0; i<TOTAL_NUMBER_OF_CONFIGS;i++){
            //generate the attribute list
            for(j=0;  attribList[2*j]!=EGL_NONE; j++){
                 attribList[2*j+1] = eglConfigDescriptors[i].ourConfig[CONFIG_ELEMENT(attribList[2*j])];
                 // When the renderable type is 0, don't bother looking for a vendor config
                 if(attribList[2*j] == EGL_RENDERABLE_TYPE){
                    renderableType = attribList[2*j+1];
                 }
             }

            //find equivalent vendor config
            if(renderableType != 0 && eglChooseConfigMRVL( dpyContext->vendorDisplay, attribList, &tempConfig[0], 40, &num_config ) && num_config >= 1){
                PRINT2N("Found native config 0x%x to match our %d", (DWORD)tempConfig[0], i+1);
                dpyContext->vendorConfigMap[i].localConfig = (EGLConfigDescriptor*)&eglConfigDescriptors[i];
                dpyContext->vendorConfigMap[i].vendorConfigAvail = TRUE;
                dpyContext->vendorConfigMap[i].vendorConfig = tempConfig[0];
            }
            else{
                PRINT2N("No config to match our %d ( 0x%04x )", i+1, eglGetErrorMRVL() );
                dpyContext->vendorConfigMap[i].localConfig = (EGLConfigDescriptor*)&eglConfigDescriptors[i];
                dpyContext->vendorConfigMap[i].vendorConfigAvail = FALSE;
                dpyContext->vendorConfigMap[i].vendorConfig = EGL_NO_CONTEXT;
            }
        }
    }

    return retBool;
}


EGLDisplay eglGetDisplayMRVLWrap(NativeDisplayType display_id)
{
    EGLDisplay vndrDisplay = EGL_NO_DISPLAY;
    MarvellNativeDisplayType displayType = (MarvellNativeDisplayType) display_id;
    PRINT("eglGetDisplay marvell");
// TEMPORARY! The first call into the egl library hangs fledge in many cases,
// due to thread suspending while it holds a system lock.
#if defined( RIM_FLEDGE )
    OsDisableInterrupts( );
#endif
    vndrDisplay = eglGetDisplayMRVL( displayType );
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorMRVL()));
#if defined( RIM_FLEDGE )
    OsEnableInterrupts( );
#endif
    return vndrDisplay;
}

// NOTE: Takes in RIM EGL parameters
EGLBoolean eglCopyBuffersMRVLWrap( EGLDisplay dpy, EGLSurface surface,
                                   NativePixmapType target)
{
    EGLDisplayContext *dpyContext = NULL;
    EGLSurfaceDescriptor * surfaceDesc = (EGLSurfaceDescriptor *)surface;
    EGLBoolean retBool = EGL_TRUE;
    EGLDisplay vendorDisplay = EGL_NO_DISPLAY;
    EGLSurface vendorSurface = EGL_NO_SURFACE;
    NativePixmap_t mrvlTrget;
    LV1(LOG_EGL, PRINT("eglCopyBuffersMRVLWrap"));

    // We validated the display in eglCopyBuffers()
    dpyContext = eglGetDisplayContextInt( dpy );
    if( !dpyContext ) {
        WARN("eglCopyBuffersMRVLWrap: bad display");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    vendorDisplay = dpyContext->vendorDisplay;
    vendorSurface = surfaceDesc->vendorSurface;

#if defined RIM_EGL_VENDOR_EX_ENABLED && defined RIM_OPENVG_MRVL
    {
        EGLVndrExSurface * vSurface = (EGLVndrExSurface *)(surfaceDesc->vendorSurface);
        EGLVndrExDisplay * vDisp = (EGLVndrExDisplay *)dpyContext->vendorDisplay;

        switch (vSurface->vndrApiId) {
            case EGL_VENDOREX_API_VG:
                vendorDisplay = vDisp->vndrDisplay[vSurface->vndrApiId];
                vendorSurface = vSurface->vndrSurface;
                break;
            default:
                WARNN("eglCopyBuffersMRVLWrap: Unexpected apiId %(_eglVndrExApiId)", vSurface->vndrApiId);
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
                return EGL_FALSE;
        }
    }
#endif //  defined RIM_EGL_VENDOR_EX_ENABLED && defined RIM_OPENVG_MRVL

    eglWaitClientMRVL();
    mrvlTrget.data = (void *)(target->data + ((target->high - 1) * target->stride));
    if( BitMapGetProperty(target->bType, BMP_BPP) == 32 ) {
        mrvlTrget.format = NATIVE_PIXFMT_ARGB8888;
    } else {
        mrvlTrget.format = NATIVE_PIXFMT_RGB565;
    }
    mrvlTrget.height = target->high;
    mrvlTrget.width = target->wide;
    mrvlTrget.stride = -(target->stride);
    
    LV1(LOG_EGL, PRINT2N("Copying 0x%x 0x%x", (DWORD)mrvlTrget.data, (DWORD)target->data));

    retBool = eglCopyBuffersMRVL( vendorDisplay, vendorSurface, (MarvellNativePixmapType)&mrvlTrget );
    return retBool;
}


EGLBoolean eglSwapBuffersMRVLWrap( EGLDisplay dpy, EGLSurface surface )
{
    EGLDisplayContext *dpyContext = NULL;
    EGLBoolean retVal = EGL_TRUE;
    EGLSurfaceDescriptor * surfaceDesc = (EGLSurfaceDescriptor *)surface;
#ifndef RIM_WINDOW_MANAGER
    LcdConfig *lcdCfg = NULL;
#endif // RIM_WINDOW_MANAGER

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglSwapBuffersMRVLWrap : bad display" ); 
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

#ifndef RIM_WINDOW_MANAGER
    lcdCfg = LcdGetLcdConfigPtr( dpyContext->id );
#endif // RIM_WINDOW_MANAGER

    if(surfaceDesc != EGL_NO_SURFACE && surfaceDesc->surfaceType == EGL_WINDOW_BIT){

#ifndef RIM_WINDOW_MANAGER
        DWORD lcdXPos = surfaceDesc->nativeWindow->windowSize.x + surfaceDesc->nativeWindow->clipRect.x;
        DWORD lcdYPos = lcdCfg->height - surfaceDesc->nativeWindow->windowSize.y - surfaceDesc->nativeWindow->clipRect.y
                - surfaceDesc->nativeWindow->clipRect.height;
#endif // RIM_KD_WINDOW_MANAGER
        LV1( LOG_EGL, PRINT("marvell eglSwapBuffers"));
        // Swap the surface
        retVal = eglSwapBuffersMRVL( dpyContext->vendorDisplay, surfaceDesc->vendorSurface );
       
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorMRVL()));

#ifndef RIM_WINDOW_MANAGER
         LV1(LOG_EGL, PRINT4N( "Swap buffers updating LCD %d %d %d %d", lcdXPos, lcdYPos,
            surfaceDesc->nativeWindow->clipRect.width,
            surfaceDesc->nativeWindow->clipRect.height));

        LcdUpdateInt(dpyContext->id, lcdXPos, lcdYPos, surfaceDesc->nativeWindow->clipRect.width,
            surfaceDesc->nativeWindow->clipRect.height);
#endif // RIM_WINDOW_MANAGER
    }
    return retVal;
}



EGLSurface eglCreatePixmapSurfaceMRVLWrap( EGLDisplay dpy, EGLConfig config,
                                           NativePixmapType pixmap,
                                           const EGLint *attrib_list )
{
    EGLDisplayContext *dpyContext = NULL;

    EGLSurface retSurf;
    EGLSurfaceDescriptor * newEglSurface = EGL_NO_SURFACE;
    EGLConfigContextPtr currentConfigContext;
    DWORD config_id = (DWORD)config;

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglCreatePixmapSurfaceMRVLWrap : bad display" ); 
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        
        return EGL_FALSE;
    }

    // Arguments have been checked in eglCreatePixmapSurface, no need to check them again
    currentConfigContext = dpyContext->vendorConfigMap[config_id-1].localConfig->ourConfig;

    newEglSurface = (EGLSurfaceDescriptor*)kdMalloc(sizeof(EGLSurfaceDescriptor));
    
    if (!newEglSurface) {
        WARN("Out of memory");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));
        return EGL_NO_SURFACE;
    }

    initializeSurface(dpy, newEglSurface, EGL_PIXMAP_BIT);

    PRINT("marvell eglCreatePixmapSurface");
    if ( currentConfigContext[CONFIG_ELEMENT(EGL_BUFFER_SIZE)] == 32 ) {
        PRINT("Creating RGBA8888 Pixmap Surface");
        newEglSurface->nativePixmap.format = NATIVE_PIXFMT_RGBA8888;
    }else{
        PRINT("Creating RGB565 Pixmap Surface");
        newEglSurface->nativePixmap.format = NATIVE_PIXFMT_RGB565;
    }


    newEglSurface->nativePixmap.width = pixmap->wide;
    newEglSurface->nativePixmap.height = pixmap->high;
    newEglSurface->nativePixmap.stride = pixmap->stride;
    newEglSurface->nativePixmap.data = pixmap->data;


    PRINT3N("wid: %d, heig: %d, data 0x%08x", pixmap->wide, pixmap->high,(int)(newEglSurface->nativePixmap.data));
    retSurf = eglCreatePixmapSurfaceMRVL(dpyContext->vendorDisplay, dpyContext->vendorConfigMap[config_id-1].vendorConfig,
        (MarvellNativePixmapType)(&(newEglSurface->nativePixmap)), attrib_list);
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorMRVL()));

    if(retSurf!=EGL_NO_SURFACE){

        // Querying the bit depth of the config
        newEglSurface->bitDepth = currentConfigContext[CONFIG_ELEMENT(EGL_BUFFER_SIZE)];

        newEglSurface->vendorSurface = retSurf;
        newEglSurface->ownsBackBuffer = FALSE;
        memcpy(&newEglSurface->backBuffer, pixmap, sizeof(BitMap));
        newEglSurface->ourConfigId = config_id;
    }
    else {
        kdFree(newEglSurface);
        newEglSurface = EGL_NO_SURFACE;

    }

    return newEglSurface;
}

EGLBoolean eglMakeCurrentMRVLWrap(EGLDisplay dpy, EGLSurface draw,
                                  EGLSurface read, EGLContext ctx)
{
    EGLBoolean retValue = EGL_TRUE;
    EGLDisplayContext *dpyContext = NULL;
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglMakeCurrentMRVLWrap : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    //special case when called with all paraeters NULL
    if( ctx == EGL_NO_CONTEXT && draw == EGL_NO_SURFACE && read == EGL_NO_SURFACE ){
        retValue = eglMakeCurrentMRVL( dpyContext->vendorDisplay, NULL, NULL, NULL );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorMRVL()));
        retValue = retValue;
    } else if( ctx != EGL_NO_CONTEXT && draw != EGL_NO_SURFACE && read != EGL_NO_SURFACE ){
        if( ((EGLSurfaceDescriptor *)draw)->vendorSurface!=EGL_NO_SURFACE &&
            ((EGLSurfaceDescriptor *)read)->vendorSurface!=EGL_NO_SURFACE &&
            ((EGLRenderingContext *)ctx)->vendorContext!=EGL_NO_CONTEXT){
            LV1( LOG_EGL, PRINT("eglMakeCurrentMRVL") );

            retValue = eglMakeCurrentMRVL( dpyContext->vendorDisplay, ((EGLSurfaceDescriptor *)draw)->vendorSurface,
                ((EGLSurfaceDescriptor *)read)->vendorSurface, ((EGLRenderingContext *)ctx)->vendorContext );
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorMRVL()));
            
        }else if( ((EGLSurfaceDescriptor *)draw)->vendorSurface!=EGL_NO_SURFACE ||
                  ((EGLSurfaceDescriptor *)read)->vendorSurface!=EGL_NO_SURFACE ||
                  ((EGLRenderingContext *)ctx)->vendorContext!=EGL_NO_CONTEXT){
            //can't mix our implementation and vendor implementation
            WARN("MRVL Can't mix surfaces and context from different EGL implementations");
            PRINT3N("Draw 0x%08x, read 0x%08x, ctx 0x%08x", (DWORD)((EGLSurfaceDescriptor *)draw)->vendorSurface, (DWORD)((EGLSurfaceDescriptor *)read)->vendorSurface, (DWORD)((EGLRenderingContext *)ctx)->vendorContext );
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
            retValue = EGL_FALSE;
        }
        // Otherwise all of the parameters are RIM Implementation
    } else {
#if !defined(RIM_NDK)    
        CATFAIL_WITH_CODE( "broken promise in eglMakeCurrentMRVLWrap", FAILURE_DESCRIBED );
#else
        log_printf(LOG_CRITICAL, "broken promise in eglMakeCurrentMRVLWrap");
        _exit(0);
#endif
    }
    return retValue;
}

