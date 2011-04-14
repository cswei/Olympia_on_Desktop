/*******************************************************************************
 *  egl_vtmv.c
 *
 *  Copyright 2007, Research In Motion Ltd.
 *
 *  Contains the interface between rendering APIs such as
 *  openGL ES or openVG and underlying LCD
 *  Designed to conform with khronos EGL 1.2 standard
 *
 *  Russell Andrade, July 26, 2007
 *
 ******************************************************************************/
//#define EGL_EXTRA_CONFIG_DEBUG

#if defined(RIM_NDK)
    #include <egl_user_types.h>
    #include <egl_vtmv.h>
    #include <unistd.h>
    #include <string.h>
#else
    #include <bugdispc.h>
    #include <failure.h>
    #include <graphics_mempool.h>
    #include <i_graphics_mempool.h>
    #include <log_verbosity.h>
    #include <string.h>
#endif

#include <bitmap.h>
#include <egl.h>
#include <egl_vtmv.h>
#include <egl_native.h>
#include <egl_globals.h>
#include <config_egl.h>
#include <i_eglext.h>
#include <i_egl.h>
#include <kd.h>

#if defined(RIM_WINDOW_MANAGER)
    #include "windowmanager.h"
    #include "i_windowmanager.h"
#endif

#include "egl_vendor_ex.h"

#define SRCFILE     FILE_EGL_VTMV
#define SRCGROUP    GROUP_GRAPHICS

#if defined (EGL_EXTRA_CONFIG_DEBUG)
    void printAllVendorConfigs(EGLDisplay dpy);
#endif

/*******************************************************************************
 ******************************************************************************/
extern void initializeSurface( EGLDisplay               dpy,
                               EGLSurfaceDescriptor     *newEglSurface,
                               BYTE                     surfaceType );

/*******************************************************************************
 * Function Name:
 *
 * Description:
 *
 * Parameters:
 * @param
 *
 * @return
 ******************************************************************************/
EGLBoolean eglInitializeVTMVWrap( EGLDisplay    dpy,
                                  EGLint        *major,
                                  EGLint        *minor )
{
    EGLDisplayContext   *pDpyCtx = NULL;
    EGLBoolean          retBool = EGL_TRUE;
    DWORD               i;
    DWORD               j;
    EGLint              numCfg;
    EGLConfig           tmpCfg;
    EGLint              renderableType = 0;
    EGLint              attribList[]={ //EGL_BUFFER_SIZE,                    0,
                                       EGL_ALPHA_SIZE,                     0,
                                       EGL_BLUE_SIZE,                      0,
                                       EGL_GREEN_SIZE,                     0,
                                       EGL_RED_SIZE,                       0,
                                       EGL_DEPTH_SIZE,                     0,
                                       EGL_STENCIL_SIZE,                   0,
                                       //EGL_CONFIG_CAVEAT,                  0,
                                       //EGL_CONFIG_ID,                      0,
                                       //EGL_LEVEL,                          0,
                                       //EGL_MAX_PBUFFER_HEIGHT,             0,
                                       //EGL_MAX_PBUFFER_PIXELS,             0,
                                       //EGL_MAX_PBUFFER_WIDTH,              0,
                                       //EGL_NATIVE_RENDERABLE,              0,
                                       //EGL_NATIVE_VISUAL_ID,               0,
                                       //EGL_NATIVE_VISUAL_TYPE,             0,
                                       //EGL_PRESERVED_RESOURCES,            0,
                                       EGL_SAMPLES,                        0,
                                       EGL_SAMPLE_BUFFERS,                 0,
                                       //EGL_SURFACE_TYPE,                   0,
                                       //EGL_TRANSPARENT_TYPE,               0,
                                       //EGL_TRANSPARENT_BLUE_VALUE,         0,
                                       //EGL_TRANSPARENT_GREEN_VALUE,        0,
                                       //EGL_TRANSPARENT_RED_VALUE,          0,
                                       //EGL_BIND_TO_TEXTURE_RGB,            0,
                                       //EGL_BIND_TO_TEXTURE_RGBA,           0,
                                       //EGL_MIN_SWAP_INTERVAL,              0,
                                       //EGL_MAX_SWAP_INTERVAL,              0,
                                       //EGL_LUMINANCE_SIZE,                 0,
                                       //EGL_ALPHA_MASK_SIZE,                0,
                                       //EGL_COLOR_BUFFER_TYPE,              0,
                                       EGL_RENDERABLE_TYPE,                0,
                                       //EGL_MATCH_NATIVE_PIXMAP,            0,
                                       //EGL_CONFORMANT,                     0,
                                       //EGL_CONFORMANT_KHR,                 0,
                                       EGL_NONE };

    LV1( LOG_EGL, PRINT("eglInitializeVTMVWrap"));

    pDpyCtx = eglGetDisplayContextInt( dpy );
    if( EGL_NO_CONTEXT == pDpyCtx ) {
        WARN( "eglInitializeVTMVWrap : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    retBool = eglInitializeVTMV( pDpyCtx->vendorDisplay, major, minor );
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVTMV()));

    if( EGL_FALSE == retBool ) {
        return EGL_FALSE;
    }

    //Initialize VTMV Extensions to match our function pointers
    eglInitExtension();

    //need to populate our configs with all the configs awailable thru the
    //vendor implementation
    #if defined (EGL_EXTRA_CONFIG_DEBUG)
        printAllVendorConfigs(pDpyCtx->vendorDisplay);
    #endif

    for( i = 0; i < TOTAL_NUMBER_OF_CONFIGS; i++ ) {
        //generate the attribute list
        for( j = 0; attribList[ 2*j ] != EGL_NONE; j++ ) {
             attribList[ 2*j + 1 ] = eglConfigDescriptors[i].ourConfig[CONFIG_ELEMENT(attribList[2*j])];
             // When the renderable type is 0, don't bother looking for a vendor config
             if(attribList[2*j] == EGL_RENDERABLE_TYPE){
                renderableType = attribList[2*j+1];
             }
         }

        //find equivalent vendor config
        if( (renderableType != 0) && 
            (eglChooseConfigVTMV( pDpyCtx->vendorDisplay,
                                  attribList, &tmpCfg, 1, &numCfg )) && 
            (numCfg == 1) ) {
            PRINT2N("Found native config 0x%x to match our %d",
                                                    (DWORD)tmpCfg, i+1);
            pDpyCtx->vendorConfigMap[i].localConfig =
                                (EGLConfigDescriptor*)&eglConfigDescriptors[i];
            pDpyCtx->vendorConfigMap[i].vendorConfigAvail = TRUE;
            pDpyCtx->vendorConfigMap[i].vendorConfig = tmpCfg;
            {
                EGLint value;
                eglGetConfigAttribVTMV(pDpyCtx->vendorDisplay,tmpCfg,EGL_CONFIG_ID, &value);
                PRINT2N("Found native config id %d to match our %d", value, i+1);
            }
        }
        else{
            PRINT2N("No config to match our %d ( 0x%04x )",
                                                     i+1, eglGetErrorVTMV() );
            pDpyCtx->vendorConfigMap[i].localConfig =
                                (EGLConfigDescriptor*)&eglConfigDescriptors[i];
            pDpyCtx->vendorConfigMap[i].vendorConfigAvail = FALSE;
            pDpyCtx->vendorConfigMap[i].vendorConfig = EGL_NO_CONTEXT;
        }
    }

    return retBool;
}


/*******************************************************************************
 * Function Name:
 *
 * Description:
 *
 * Parameters:
 * @param
 *
 * @return
 ******************************************************************************/
EGLDisplay eglGetDisplayVTMVWrap( NativeDisplayType display_id )
{
    EGLDisplay vndrDisplay = EGL_NO_DISPLAY;

    LV1( LOG_EGL, PRINT("eglGetDisplayVTMVWrap"));

    // TEMPORARY! The first call into the egl library hangs fledge in many cases,
    // due to thread suspending while it holds a system lock.
    #if defined( RIM_FLEDGE )
        OsDisableInterrupts( );
    #endif

    vndrDisplay = eglGetDisplayVTMV( display_id );
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVTMV()));

    #if defined( RIM_FLEDGE )
        OsEnableInterrupts( );
    #endif
    return vndrDisplay;
}

/*******************************************************************************
 * Function Name:
 *
 * Description: NOTE: Takes in RIM EGL parameters
 *
 * Parameters:
 * @param
 *
 * @return
 ******************************************************************************/
EGLBoolean eglCopyBuffersVTMVWrap( EGLDisplay           dpy,
                                   EGLSurface           surface,
                                   NativePixmapType     target )
{
    EGLDisplayContext *pDpyCtx = NULL;
    EGLSurfaceDescriptor * pSurfDesc = (EGLSurfaceDescriptor *)surface;
    EGLBoolean retBool = EGL_TRUE;
    EGLDisplay vendorDisplay = EGL_NO_DISPLAY;
    EGLSurface vendorSurface = EGL_NO_SURFACE;

    LV1( LOG_EGL, PRINT("eglCopyBuffersVTMVWrap"));

    // We validated the display in eglCopyBuffers()
    pDpyCtx = eglGetDisplayContextInt( dpy );
    if( EGL_NO_CONTEXT == pDpyCtx ) {
        WARN("eglCopyBuffersVTMVWrap: bad display");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    eglWaitClientVTMV(); //synchronization

    vendorDisplay = pDpyCtx->vendorDisplay;
    vendorSurface = pSurfDesc->vendorSurface;

#if defined RIM_EGL_VENDOR_EX_ENABLED
    {
        EGLVndrExSurface * vSurface = (EGLVndrExSurface *)vendorSurface;
        EGLVndrExDisplay * vDisp = (EGLVndrExDisplay *)vendorDisplay;

        switch (vSurface->vndrApiId) {
            case EGL_VENDOREX_API_GLES:
                vendorDisplay = vDisp->vndrDisplay[vSurface->vndrApiId];
                vendorSurface = vSurface->vndrSurface;
                break;
            default:
                WARNN("eglCopyBuffersVTMVWrap: Unexpected apiId %(_eglVndrExApiId)", vSurface->vndrApiId);
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
                return EGL_FALSE;
        }
    }
#endif //  defined RIM_EGL_VENDOR_EX_ENABLED && defined RIM_OPENVG_MRVL
    
    #if 0
    {
        BitMap vtmvTarget;
        vtmvTarget.data = (void *)(target->data + ((target->high - 1) * target->stride));   
        vtmvTarget.bType = target->bType;
        vtmvTarget.high = target->high;
        vtmvTarget.wide = target->wide;
        vtmvTarget.stride = -(target->stride);
        
        LV1(LOG_EGL, PRINT2N("Copying 0x%x 0x%x", (DWORD)vtmvTarget.data, (DWORD)target->data));
        PRINT2N("Copying 0x%x 0x%x", (DWORD)vtmvTarget.data, (DWORD)target->data);
        retBool = eglCopyBuffersVTMV( vendorDisplay,
                                      vendorSurface,
                                      &vtmvTarget );
    }
    #else
        retBool = eglCopyBuffersVTMV( vendorDisplay, vendorSurface, target );    
    #endif

    LV1( LOG_EGL, PRINTN("eglCopyBuffersVTMVWrap result %d", retBool));
    PRINTN("eglCopyBuffersVTMVWrap result %d", retBool);

    return retBool;
}


/******************************************************************************
 * Function Name:
 *
 * Description:
 *
 * Parameters:
 * @param
 *
 * @return
 *****************************************************************************/
#ifndef RIM_WINDOW_MANAGER
EGLBoolean eglSwapBuffersVTMVWrap( EGLDisplay dpy, EGLSurface surface )
{
    EGLDisplayContext       *pDpyCtx = NULL;
    EGLBoolean              retVal = EGL_TRUE;
    EGLSurfaceDescriptor    *pSurfDesc = (EGLSurfaceDescriptor *)surface;
    LcdConfig               *lcdCfg = NULL;

    LV1( LOG_EGL, PRINT("eglSwapBuffersVTMVWrap"));

    // Get the EGLDisplayContext
    pDpyCtx = eglGetDisplayContextInt( dpy );
    if( EGL_NO_CONTEXT == pDpyCtx ) {
        WARN( "eglSwapBuffersVTMVWrap : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    lcdCfg = LcdGetLcdConfigPtr( pDpyCtx->id );
    LV1( LOG_EGL, PRINTN("pSurfDesc 0x%08x", pSurfDesc));
    if(pSurfDesc) {
        LV1( LOG_EGL, PRINTN("pSurfDesc->surfaceType 0x%08x",
                                                pSurfDesc->surfaceType) );
    }

    if( (EGL_NO_SURFACE != pSurfDesc) &&
        (EGL_WINDOW_BIT == pSurfDesc->surfaceType) ) {

        const NativeWindow      *pWin = pSurfDesc->nativeWindow;
        const NativeWinRect_t   *pClip = &pWin->clipRect;
        const DWORD             lcdXPos = pWin->windowSize.x + pClip->x;
        const DWORD             lcdYPos = lcdCfg->height - pWin->windowSize.y
                                        - pClip->y - pClip->height;

        LV2( LOG_EGL, PRINT("VTMV eglSwapBuffers"));

        // Swap the surface
        retVal = eglSwapBuffersVTMV( pDpyCtx->vendorDisplay,
                                     pSurfDesc->vendorSurface );

        kdSetThreadStorageKHR( eglLastErrorKey, (void *)(eglGetErrorVTMV()) );

        LV2(LOG_EGL, PRINT4N( "Swap buffers updating LCD %d %d %d %d",
                               lcdXPos, lcdYPos,
                               pClip->width,
                               pClip->height));

        LcdUpdateInt(pDpyCtx->id,lcdXPos,lcdYPos,pClip->width,pClip->height);
    }

    return retVal;
}

/*******************************************************************************
 ******************************************************************************/
#else  //#ifndef RIM_WINDOW_MANAGER
EGLBoolean eglSwapBuffersVTMVWrap( EGLDisplay dpy, EGLSurface surface )
{
    EGLDisplayContext       *pDpyCtx = NULL;
    EGLBoolean              retVal = EGL_TRUE;
    EGLSurfaceDescriptor    *pSurfDesc = (EGLSurfaceDescriptor *)surface;

    LV1( LOG_EGL, PRINT("eglSwapBuffersVTMVWrap"));

    // Get the EGLDisplayContext
    pDpyCtx = eglGetDisplayContextInt( dpy );
    if( EGL_NO_CONTEXT == pDpyCtx ){
        WARN( "eglSwapBuffersVTMVWrap : bad display" );

        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));

        return EGL_FALSE;
    }

    if( (EGL_NO_SURFACE != pSurfDesc) && 
        (EGL_WINDOW_BIT == pSurfDesc->surfaceType) ) {

        LV1( LOG_EGL, PRINT("VTMV eglSwapBuffers"));
        // Swap the surface
        retVal = eglSwapBuffersVTMV( pDpyCtx->vendorDisplay,
                                     pSurfDesc->vendorSurface );

        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVTMV()));
    }

    return retVal;
}
#endif // #ifndef RIM_WINDOW_MANAGER

/*******************************************************************************
 * Function Name:
 *
 * Description:
 *
 * Parameters:
 * @param
 *
 * @return
 ******************************************************************************/
EGLBoolean eglMakeCurrentVTMVWrap( EGLDisplay   dpy,
                                   EGLSurface   draw,
                                   EGLSurface   read,
                                   EGLContext   ctx )
{
    EGLBoolean              retValue = EGL_TRUE;
    EGLDisplayContext       *pDpyCtx = NULL;

    LV1( LOG_EGL, PRINT("eglMakeCurrentVTMVWrap"));

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    // Get the EGLDisplayContext
    pDpyCtx = eglGetDisplayContextInt( dpy );
    if( EGL_NO_CONTEXT == pDpyCtx ){
        WARN( "eglMakeCurrentVTMVWrap : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    LV1( LOG_EGL, PRINT("eglMakeCurrentVTMV Check params.") );

    //special case when called with all paraeters NULL
    if( (EGL_NO_CONTEXT == ctx) &&
        (EGL_NO_SURFACE == draw) && 
        (EGL_NO_SURFACE == read) ) {
        LV1( LOG_EGL, PRINT("eglMakeCurrentVTMV No Nothing") );
        retValue = eglMakeCurrentVTMV( pDpyCtx->vendorDisplay, NULL,NULL,NULL );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVTMV()));
        retValue = retValue;
    }
    else if( (EGL_NO_CONTEXT != ctx) &&
             (EGL_NO_SURFACE != draw) && 
             (EGL_NO_SURFACE != read) ) {
        EGLSurface  *pDrawSurf = ((EGLSurfaceDescriptor *)draw)->vendorSurface;
        EGLSurface  *pReadSurf = ((EGLSurfaceDescriptor *)read)->vendorSurface;
        EGLContext  *pVndrCtx = ((EGLRenderingContext *)ctx)->vendorContext;

        if( (EGL_NO_SURFACE != pDrawSurf) &&
            (EGL_NO_SURFACE != pReadSurf) &&
            (EGL_NO_CONTEXT != pVndrCtx) ) {
            LV1( LOG_EGL, PRINT("eglMakeCurrentVTMV everything present") );
            retValue = eglMakeCurrentVTMV( pDpyCtx->vendorDisplay,
                                           pDrawSurf, pReadSurf, pVndrCtx );
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVTMV()));
        }else if( (EGL_NO_SURFACE != pDrawSurf) ||
                  (EGL_NO_SURFACE != pReadSurf) ||
                  (EGL_NO_CONTEXT != pVndrCtx) ) {
            //can't mix our implementation and vendor implementation
            WARN("VTMV Can't mix surfs and ctx from diff EGL implements.");
            PRINT3N("Draw 0x%08x, read 0x%08x, ctx 0x%08x",
                    (DWORD)pDrawSurf, (DWORD)pReadSurf, (DWORD)pVndrCtx );
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
            retValue = EGL_FALSE;
        }
        // Otherwise all of the parameters are RIM Implementation
    } else {
        #if !defined(RIM_NDK)
            CATFAIL_WITH_CODE(
                "broken promise in eglMakeCurrentVTMVWrap", FAILURE_DESCRIBED );

        #else
            log_printf(LOG_CRITICAL,"broken promise in eglMakeCurrentVTMVWrap");
            _exit(0);

        #endif
    }
    return retValue;
}

