/*******************************************************
 *  egl_amanithqc.c
 *
 *  Copyright 2009, Research In Motion Ltd.
 *
 *  Contains the interface between AmanithVG, Qualcomm OpenGL ES and EGL
 *  Designed to conform with khronos EGL 1.2 standard
 *
 *  Philip Chrapka, April, 2009
 *
 *******************************************************/

#if defined (RIM_NDK)
    #include <egl_user_types.h>
#else
    #include "bugdispc.h"

    #define SRCFILE     FILE_EGL_AMQC
    #define SRCGROUP    GROUP_GRAPHICS

    #include "log_verbosity.h"
#endif

#include "egl_amanithqc.h"
#include "egl_amanithqc_globals.h"
#include "config_egl.h"
#include "bitmap.h"

#include "raster.h"
#include "i_raster.h"

#if !defined(RIM_NDK_DISABLE_DMOV_API)
#include "dmov_mem.h"
#include "vm_task.h" // What's this for?
#include "gl.h"      // What's this for?
#endif

#include "kd.h"

#ifdef RIM_WINDOW_MANAGER
#include "windowmanager.h"
#include "i_windowmanager.h"
#endif

EGLDisplay eglGetDisplayAmanithQC( NativeDisplayType display_id )
{
    // AmanithVG does not need a vendor display for now, I think
    // Just get a QC display
    EGLDisplay vndrDisplay = EGL_NO_DISPLAY;
    PRINT("eglGetDisplayAmanithQC");
    vndrDisplay = qceglGetDisplay( display_id );

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(qceglGetError()));

    return vndrDisplay;
}

EGLint eglGetErrorAmanithQC( void )
{
    PRINT("eglGetErrorAmanithQC");
    return (EGLint)(kdGetThreadStorageKHR(eglLastErrorKey));
}

EGLBoolean eglBindAPIAmanithQC( EGLenum api )
{
    PRINT("eglBindAPIAmanithQC");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    // Technically it shouldn't matter since QC will always be set to OpenGL ES
    return EGL_TRUE;
}

// Not used but implemented anyway
EGLenum eglQueryAPIAmanithQC( void )
{
    PRINT("eglQueryAPIAmanithQC");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    return (EGLenum)kdGetThreadStorageKHR(eglCurrentApiKey);
}

/**
* Matches a QC vendor config from the list of qcAllConfigs to the specified attribList and returns it as qcConfig
* For internal Amanith/QC use only
*/
EGLBoolean eglGetVendorConfigQC(EGLDisplay dpy, EGLConfig *qcConfig, EGLConfig *qcAllConfigs, EGLint qcNumConfigs, EGLint *attribList, EGLint localConfigID)
{
    DWORD i;
    EGLDisplayContext *dpyContext = NULL;
    EGLint qcRedVal, qcGreenVal, qcBlueVal, qcAlphaVal;
    EGLint redVal, greenVal, blueVal, alphaVal;
    EGLint qcConfigsReturned = 0;

    // Get the EGLDisplayContext, we already checked if it's valid
    dpyContext = eglGetDisplayContextInt( dpy );

    PRINT("eglGetVendorConfigQC");
    alphaVal = eglConfigDescriptors[localConfigID].ourConfig[CONFIG_ELEMENT(EGL_ALPHA_SIZE)];
    blueVal = eglConfigDescriptors[localConfigID].ourConfig[CONFIG_ELEMENT(EGL_BLUE_SIZE)];
    greenVal = eglConfigDescriptors[localConfigID].ourConfig[CONFIG_ELEMENT(EGL_GREEN_SIZE)];
    redVal = eglConfigDescriptors[localConfigID].ourConfig[CONFIG_ELEMENT(EGL_RED_SIZE)];
    if(qceglChooseConfig( dpyContext->vendorDisplay, attribList, qcAllConfigs, qcNumConfigs, &qcConfigsReturned ) == EGL_TRUE && qcConfigsReturned >= 1){
        for(i=0; i<qcConfigsReturned; i++){
            // Make sure that the buffer sizes match exactly
            qceglGetConfigAttrib( dpyContext->vendorDisplay, qcAllConfigs[i], EGL_RED_SIZE, &qcRedVal);
            qceglGetConfigAttrib( dpyContext->vendorDisplay, qcAllConfigs[i], EGL_GREEN_SIZE, &qcGreenVal);
            qceglGetConfigAttrib( dpyContext->vendorDisplay, qcAllConfigs[i], EGL_BLUE_SIZE, &qcBlueVal);
            qceglGetConfigAttrib( dpyContext->vendorDisplay, qcAllConfigs[i], EGL_ALPHA_SIZE, &qcAlphaVal);

            if( qcRedVal != redVal || qcGreenVal != greenVal || qcBlueVal != blueVal || qcAlphaVal != alphaVal){
                PRINTN("Vendor config index %d doesn't match the native config colourspace", i);
                PRINT2N("Native Red   %d, Vendor Red   %d",  redVal, qcRedVal);
                PRINT2N("Native Green %d, Vendor Green %d",  greenVal, qcGreenVal);
                PRINT2N("Native Blue  %d, Vendor Blue  %d",  blueVal, qcBlueVal);
                PRINT2N("Native Alpha %d, Vendor Alpha %d",  alphaVal, qcAlphaVal);
            }else{
                *qcConfig = qcAllConfigs[i];
                return EGL_TRUE;
            }
        }
    }
    WARN("No matching QC config found");
    return EGL_FALSE;
}

EGLBoolean eglInitializeAmanithQC( EGLDisplay dpy, EGLint *major, EGLint *minor )
{
    DWORD i,j;
    EGLint maj, min;
    EGLDisplayContext *dpyContext = NULL;
    EGLint qcNumConfigs = 0;
    EGLConfig *qcAllConfigs = NULL;
    EGLBoolean retBool = EGL_TRUE;
    EGLint renderableType = 0;
    EGLint      attribList[]={
        EGL_ALPHA_SIZE,                     0,
            EGL_BLUE_SIZE,                      0,
            EGL_GREEN_SIZE,                     0,
            EGL_RED_SIZE,                       0,
            EGL_SURFACE_TYPE,                   0,
            EGL_RENDERABLE_TYPE,                0,
            EGL_BUFFER_SIZE,                    0,
            EGL_DEPTH_SIZE,                     0,
            EGL_STENCIL_SIZE,                   0,
            //EGL_CONFIG_CAVEAT,                  0,
            //EGL_NATIVE_RENDERABLE,              0,
            //EGL_NATIVE_VISUAL_TYPE,             0,
            //EGL_SAMPLES,                        0,
            //EGL_LEVEL,                          0,
            //EGL_SAMPLE_BUFFERS,                 0,
            //EGL_BIND_TO_TEXTURE_RGB,            0,
            //EGL_BIND_TO_TEXTURE_RGBA,           0,
            //EGL_MIN_SWAP_INTERVAL,              0,
            //EGL_MAX_SWAP_INTERVAL,              0,
            //EGL_LUMINANCE_SIZE,                 0,
            //EGL_ALPHA_MASK_SIZE,                0,
            //EGL_COLOR_BUFFER_TYPE,              0,
            EGL_NONE
    };

    PRINT("eglInitializeAmanithQC");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglInitializeAmanithQC : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    // Check should be added to not reinitialize a display -> change EGLDisplayContext architeture

    // PC Not sure if i can initialize the extensions if only eglGetDisplay has been called
    // or if it should be done after eglInitialize -> check with marvell
    if( eglInitExtension() != EGL_TRUE ){
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_FALSE;
    }

    // vendorDisplay will always be a QC Display, no need to update major and minor
    retBool = qceglInitialize( dpyContext->vendorDisplay, &maj, &min);

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(qceglGetError()));

    // Check if QC initialization was successful
    if(retBool == EGL_FALSE){
        return retBool;
    }

    // Get the total number of QC EGLConfigs and make an array that size
    qceglGetConfigs(dpyContext->vendorDisplay, NULL, 0, &qcNumConfigs);
    PRINTN("QC Vendor has %d configs total", qcNumConfigs);
    qcAllConfigs = (EGLConfig *)kdMalloc(sizeof(EGLConfig)*qcNumConfigs);
    if(!qcAllConfigs){
        //TODO uninitialize display ???
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_FALSE;
    }

    // We need to populate our configs with all the configs available thru the vendor implementation
    for(i=0; i<TOTAL_NUMBER_OF_CONFIGS; i++){
        // Generate the attribute list
        for(j=0;  attribList[2*j] != EGL_NONE; j++){
            attribList[2*j+1] = eglConfigDescriptors[i].ourConfig[CONFIG_ELEMENT(attribList[2*j])];
            // Use the renderable type to decide which set of vendor functions to use
            if(attribList[2*j] == EGL_RENDERABLE_TYPE){
                renderableType = attribList[2*j+1];
            }
        }

        // Intialize the vendor config map to its default values
        dpyContext->vendorConfigMap[i].localConfig = (EGLConfigDescriptor*)&eglConfigDescriptors[i];
        dpyContext->vendorConfigMap[i].vendorConfigAvail = FALSE;
        dpyContext->vendorConfigMap[i].vendorConfig = NULL;

        // Initialize different vendor configs based on the supported rendering api
        if(renderableType == EGL_OPENVG_BIT){
            // Grab Amanith Configs only
            EGLVendorConfigAmanithQC *tempConfig = (EGLVendorConfigAmanithQC *)kdMalloc(sizeof(EGLVendorConfigAmanithQC));
            PRINT2N("Found Amanith config 0x%08x to match our %d", &eglConfigDescriptors[i], i+1);
            if(!tempConfig){
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));
                break;
            }
            tempConfig->vendorID = EGL_VENDOR_AMANITH;
            tempConfig->vendorConfig = (EGLConfig)&eglConfigDescriptors[i];

            dpyContext->vendorConfigMap[i].localConfig = (EGLConfigDescriptor*)&eglConfigDescriptors[i];
            dpyContext->vendorConfigMap[i].vendorConfigAvail = TRUE;
            dpyContext->vendorConfigMap[i].vendorConfig = (EGLConfig)tempConfig;
        }else if(renderableType == EGL_OPENGL_ES_BIT){
            EGLConfig qcConfig = 0;
            // Grab QC Configs only
            if(eglGetVendorConfigQC(dpy, &qcConfig, qcAllConfigs, qcNumConfigs, attribList, i) == EGL_TRUE){
                EGLVendorConfigAmanithQC *tempConfig = (EGLVendorConfigAmanithQC *)kdMalloc(sizeof(EGLVendorConfigAmanithQC));
                PRINT2N("Found QC config 0x%08x to match our %d", qcConfig, i+1);
                if(!tempConfig){
                    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));
                    break;
                }
                tempConfig->vendorID = EGL_VENDOR_QC;
                tempConfig->vendorConfig = qcConfig;

                dpyContext->vendorConfigMap[i].localConfig = (EGLConfigDescriptor*)&eglConfigDescriptors[i];
                dpyContext->vendorConfigMap[i].vendorConfigAvail = TRUE;
                dpyContext->vendorConfigMap[i].vendorConfig = (EGLConfig)tempConfig;
            }else{
                WARNN("No QC config to match our %d", i+1);
            }
        }else if(renderableType == (EGL_OPENGL_ES_BIT|EGL_OPENVG_BIT)){
            WARN("This implementation does not support OpenGL ES and OpenVG configurations");
            kdCatfailRim("Invalid config");
        }else{
            // Grab no vendor configs - default to dummy RIM implementation
            PRINTN("No vendor config to match our %d", i+1);
        }
    }

    // Destroy the list of all QC vendor configs
    if(qcAllConfigs){
        kdFree(qcAllConfigs);
        qcAllConfigs = NULL;
    }

    // Destroy all other allocated resources if there is not enough memory
    if((EGLint)(kdGetThreadStorageKHR(eglLastErrorKey)) == EGL_BAD_ALLOC){
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        for(i=0; i<TOTAL_NUMBER_OF_CONFIGS; i++){
            if(dpyContext->vendorConfigMap[i].vendorConfig){
                kdFree(dpyContext->vendorConfigMap[i].vendorConfig);
            }
        }
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

EGLBoolean eglMakeCurrentAmanithQC( EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx )
{
    EGLBoolean retBool = EGL_TRUE;
    EGLDisplayContext *dpyContext = NULL;
    EGLSurfaceDescriptor *readSurface = (EGLSurfaceDescriptor *)read;
    EGLSurfaceDescriptor *drawSurface = (EGLSurfaceDescriptor *)draw;
    EGLRenderingContext *context = (EGLRenderingContext *)ctx;
    PRINT("eglMakeCurrentAmanithQC");
    PRINT3N("read 0x%08x draw 0x%08x ctx 0x%08x",readSurface, drawSurface, context);

    // Get the EGLDisplayContext, we already checked if it's valid
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglMakeCurrentAmanithQC : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    if(draw == EGL_NO_SURFACE && read == EGL_NO_SURFACE && ctx == EGL_NO_CONTEXT){
        PRINT("Making NULL");
        // Determine which client context to make null
        switch ((EGLenum)kdGetThreadStorageKHR(eglCurrentApiKey)){
        case EGL_OPENVG_API:
            PRINT("Making OpenVG NULL");
            retBool = eglMakeCurrentAmanithInt(dpyContext->vendorDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorAmanith()));
            return retBool;
        case EGL_OPENGL_ES_API:
            PRINT("Making OpenGL ES NULL");
            retBool = qceglMakeCurrent(dpyContext->vendorDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(qceglGetError()));
            return retBool;
        default:
            WARNN("Bad current api %x",(EGLenum)kdGetThreadStorageKHR(eglCurrentApiKey));
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
            return EGL_FALSE;
        }
    }else if(draw == EGL_NO_SURFACE || read == EGL_NO_SURFACE || ctx == EGL_NO_CONTEXT){
        WARN("Bad mix of parameters");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
        return EGL_FALSE;
    }else{
        EGLVendorSurfaceAmanithQC *vendorRead = (EGLVendorSurfaceAmanithQC *)(readSurface->vendorSurface);
        EGLVendorSurfaceAmanithQC *vendorDraw = (EGLVendorSurfaceAmanithQC *)(drawSurface->vendorSurface);
        EGLVendorContextAmanithQC *vendorContext = (EGLVendorContextAmanithQC *)(context->vendorContext);
        PRINT3N("Vendor read 0x%08x draw 0x%08x ctx 0x%08x",vendorRead,vendorDraw,vendorContext);

        if(vendorRead != EGL_NO_SURFACE && vendorDraw != EGL_NO_SURFACE && vendorContext != EGL_NO_CONTEXT){

            // Check for surface and context compatibility
            switch(vendorContext->vendorID){
            case EGL_VENDOR_QC:
                // Make sure that the surfaces and the context have the same vendor
                if(vendorDraw->vendorID == EGL_VENDOR_QC && vendorRead->vendorID == EGL_VENDOR_QC){
                    retBool = qceglMakeCurrent(dpyContext->vendorDisplay, vendorDraw->vendorSurface, vendorRead->vendorSurface, vendorContext->vendorContext);
                    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(qceglGetError()));
                }else{
                    retBool = EGL_FALSE;
                    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
                }
                break;
            case EGL_VENDOR_AMANITH:
                // Make sure that the surfaces and the context have the same vendor
                if(vendorDraw->vendorID == EGL_VENDOR_AMANITH && vendorRead->vendorID == EGL_VENDOR_AMANITH){ 
                    retBool = eglMakeCurrentAmanithInt(dpyContext->vendorDisplay, vendorDraw->vendorSurface, vendorRead->vendorSurface, vendorContext->vendorContext);
                    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorAmanith()));
                }else{
                    retBool = EGL_FALSE;
                    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
                }
                break;
            default:
                WARNN("Unknown VendorID 0x%02x",vendorContext->vendorID);
                retBool = EGL_FALSE;
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONTEXT));
                break;
            }
        }else if(vendorRead != EGL_NO_SURFACE || vendorDraw != EGL_NO_SURFACE || vendorContext != EGL_NO_CONTEXT){
            //Can't mix our implementation and vendor implementation
            WARN("Can't mix surfaces and context from different EGL implementations");
            PRINT3N("draw 0x%08x, read 0x%08x, ctx 0x%08x", vendorDraw, vendorRead, vendorContext);
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
            return EGL_FALSE;
        }else{
            // All of the parameters are RIM Implementation
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
            retBool = EGL_TRUE;
        }
    }
    return retBool;
}

// NOTE: Takes in RIM EGL parameters
EGLBoolean eglCopyBuffersAmanithQC( EGLDisplay dpy, EGLSurface surface, NativePixmapType target )
{
    EGLBoolean retBool = EGL_FALSE;
    EGLDisplayContext *dpyContext = NULL;
    EGLSurfaceDescriptor *surfaceDesc = (EGLSurfaceDescriptor *)surface;
    // We already checked if a vendorSurface exists in eglCopyBuffers()
    EGLVendorSurfaceAmanithQC *vendorSurface = (EGLVendorSurfaceAmanithQC *)(surfaceDesc->vendorSurface);

    PRINT("eglCopyBuffersAmanithQC");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
    // We validated the display in eglCopyBuffers()
    dpyContext = eglGetDisplayContextInt( dpy );

    switch(vendorSurface->vendorID){
    case EGL_VENDOR_QC:
        // NOTE: Takes in RIM EGL parameters
        glFinish();
        retBool = qceglCopyBuffersWrap(dpy, surface, target);
        // EglLastError is updated in qceglCopyBuffersWrap
        break;
    case EGL_VENDOR_AMANITH:
        // NOTE: Takes in RIM EGL parameters
        retBool = eglCopyBuffersAmanithInt(dpyContext->vendorDisplay, vendorSurface->vendorSurface, target);
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorAmanith()));
        break;
    default:
        WARNN("Unknown VendorID 0x%02x",vendorSurface->vendorID);
        break;
    }
    return retBool;
}

EGLBoolean eglSwapBuffersAmanithQC( EGLDisplay dpy, EGLSurface surface )
{
    EGLDisplayContext *dpyContext = NULL;
    EGLSurfaceDescriptor * surfaceDesc = (EGLSurfaceDescriptor *)surface;
    LcdConfig *lcdCfg = NULL;

    PRINT("eglSwapBuffersAmanithQC");

    // Get the EGLDisplayContext, we already checked if it's valid
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglSwapBuffersAmanithQC : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    lcdCfg = LcdGetLcdConfigPtr( dpyContext->id );

    if(surfaceDesc != EGL_NO_SURFACE && surfaceDesc->surfaceType == EGL_WINDOW_BIT){
        EGLVendorSurfaceAmanithQC *vendorSurface = (EGLVendorSurfaceAmanithQC *)surfaceDesc->vendorSurface;

#ifdef RIM_WINDOW_MANAGER
        WMError_t       result;
        DWORD           backPixelStride;
        DWORD           frontBitDepth;
        DWORD           frontPixelStride;
        SDWORD          temp[4];
        Rect            dirtyRect;
        DWORD          *backBuffer;
        DWORD          *frontBuffer;

        // get bit depth of front buffer
        frontBitDepth = BitMapGetProperty(surfaceDesc->frontBuffer.bType, BMP_BPP);
        if (frontBitDepth == 0) {
            kdCatfailRim("No bit depth for EGL front buffer");
            return EGL_FALSE;
        }

        // get front buffer and calculate pixels per row
        frontBuffer = (DWORD*)surfaceDesc->frontBuffer.data;
        frontPixelStride = surfaceDesc->frontBuffer.stride / (frontBitDepth / 8);

        // get native window dirty region
        result = WMGetWindowPropertyiv(surfaceDesc->nativeWindow, WM_PROPERTY_DIRTY_REGION, temp);
        if (result != WM_E_OK) {
            WARN("eglSwapBuffersAmanithQC: bad native window");
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_NATIVE_WINDOW));
            return EGL_FALSE;
        }
        dirtyRect.x = temp[0];
        dirtyRect.y = temp[1];
        dirtyRect.width = temp[2];
        dirtyRect.height = temp[3];
#else
        WORD           *frontBuffer = NULL;
        WORD           *frontBufferTemp = NULL;
        WORD           *surfaceBuffer = NULL;
        DWORD          *surfaceBuffer32 = NULL;
        DWORD           frontBufferStride;
        DWORD           frontBufferFormat;

        DWORD lcdXPos = surfaceDesc->nativeWindow->windowSize.x + surfaceDesc->nativeWindow->clipRect.x;
        DWORD lcdYPos = lcdCfg->height - surfaceDesc->nativeWindow->windowSize.y - surfaceDesc->nativeWindow->clipRect.y
            - surfaceDesc->nativeWindow->clipRect.height;
        DWORD windowXPos = surfaceDesc->nativeWindow->clipRect.x;
        DWORD windowYPos = surfaceDesc->nativeWindow->windowSize.height - surfaceDesc->nativeWindow->clipRect.y - surfaceDesc->nativeWindow->clipRect.height;
#endif // RIM_WINDOW_MANAGER

        if (vendorSurface->vendorID == EGL_VENDOR_QC){
            PRINT("QC Vendor Surface");

            glFinish();

#ifdef RIM_WINDOW_MANAGER
            // get vendor back buffer
            backBuffer = (DWORD*) eglGetColorBufferQUALCOMM();
            if(!backBuffer){
                WARN("Could not get the surface");
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
                return EGL_FALSE;
            }
            PRINTN("Color buffer returned 0x%08x", (DWORD)backBuffer);

            // calculate stride of back buffer (must be 8-pixel aligned regardless of bit depth)
            backPixelStride = (surfaceDesc->frontBuffer.wide + 7) & ~7;

            // lock window front buffer
            result = WMIntLockWindowBuffer(surfaceDesc->nativeWindow, NULL);
            if (result != WM_E_OK) {
                WARN("eglSwapBuffersAmanithQC: bad native window");
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_NATIVE_WINDOW));
                return EGL_FALSE;
            }

            if (surfaceDesc->bitDepth == 16){

                    RasterCopyC16( ((WORD*)backBuffer) + dirtyRect.y * backPixelStride, backPixelStride, dirtyRect.x,
                        ((WORD*)frontBuffer) + dirtyRect.y * frontPixelStride, frontPixelStride, dirtyRect.x,
                        dirtyRect.width, dirtyRect.height );

            }else if (surfaceDesc->bitDepth == 32){

                    RasterCopyC32( backBuffer + dirtyRect.y * backPixelStride, backPixelStride, dirtyRect.x,
                        frontBuffer + dirtyRect.y * frontPixelStride, frontPixelStride, dirtyRect.x,
                        dirtyRect.width, dirtyRect.height );

            }

            // unlock window front buffer
            WMIntUnlockWindowBuffer(surfaceDesc->nativeWindow);
#else
            surfaceBuffer = (WORD *) eglGetColorBufferQUALCOMM();
            if(!surfaceBuffer){
                WARN("Could not get the surface");
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
                return EGL_FALSE;
            }
            PRINTN("Color buffer returned 0x%08x", (DWORD)surfaceBuffer);

            // Adjust the surface buffer to the correct y position
            surfaceBuffer += windowYPos * surfaceDesc->nativeWindow->windowSize.width;


            LcdGetBuffer( dpyContext->id, LCD_FRONT_BUFFER, &frontBuffer, &frontBufferFormat, &frontBufferStride );
            frontBufferTemp = frontBuffer;
            // Adjust the back buffer to the correct y position
            frontBufferTemp += lcdYPos*lcdCfg->width;

            // If the stride is the same as the lcd width we can use the faster dmov_memcpy
            if (surfaceDesc->bitDepth == 16){

#if !defined(RIM_NDK_DISABLE_DMOV_API)
                if( surfaceDesc->nativeWindow->clipRect.width == lcdCfg->width ){
                    LV1( LOG_EGL, PRINT("QCOMM dmov_memcpy"));
                    dmov_memcpy(frontBufferTemp, surfaceBuffer, surfaceDesc->nativeWindow->clipRect.width * surfaceDesc->nativeWindow->clipRect.height * 2,
                        DMOV_MEM_CACHE_DEST_ONLY, VM_FINISH_DMOV_MEMCPY_SIG);
                }else{
                    RasterCopyC16( surfaceBuffer, surfaceDesc->nativeWindow->windowSize.width, windowXPos,
                        frontBufferTemp, lcdCfg->width, lcdXPos,
                        surfaceDesc->nativeWindow->clipRect.width, surfaceDesc->nativeWindow->clipRect.height );
                }
#else
                RasterCopyC16( surfaceBuffer, surfaceDesc->nativeWindow->windowSize.width, windowXPos,
                    frontBufferTemp, lcdCfg->width, lcdXPos,
                    surfaceDesc->nativeWindow->clipRect.width, surfaceDesc->nativeWindow->clipRect.height );
#endif // #if !defined(RIM_DISABLE_DMOV_API)
            }else if (surfaceDesc->bitDepth == 32){
                RasterARGB8888ToRGB565Copy( (DWORD*)surfaceBuffer, surfaceDesc->nativeWindow->windowSize.width, windowXPos,
                   frontBufferTemp, lcdCfg->width, lcdXPos,
                    surfaceDesc->nativeWindow->clipRect.width, surfaceDesc->nativeWindow->clipRect.height );

            }

#endif // RIM_WINDOW_MANAGER
        }
        else if(vendorSurface->vendorID == EGL_VENDOR_AMANITH){
            VGImageFormat surfaceFormat;
            VGSurfaceDescriptor *vgSurfaceDesc = (VGSurfaceDescriptor*)vendorSurface->vendorSurface;
            PRINT("Amanith Vendor Surface");

#ifdef RIM_WINDOW_MANAGER
            // get vendor back buffer
            backBuffer = (DWORD*)vgGetSurface(vgSurfaceDesc->vgSurface, &surfaceFormat);
            if(!backBuffer){
                WARN("Could not get the surface");
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
                return EGL_FALSE;
            }

            // lock window front buffer
            result = WMIntLockWindowBuffer(surfaceDesc->nativeWindow, NULL);
            if (result != WM_E_OK) {
                WARN("eglSwapBuffersAmanithQC: bad native window");
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_NATIVE_WINDOW));
                return EGL_FALSE;
            }

            if (surfaceDesc->bitDepth == 16){
                // downsample 32-bit back buffer to 16-bit front buffer
                RasterARGB8888ToRGB565Copy( backBuffer + dirtyRect.y * vgSurfaceDesc->width, vgSurfaceDesc->width, dirtyRect.x,
                    ((WORD*)frontBuffer) + dirtyRect.y * frontPixelStride, frontPixelStride, dirtyRect.x,
                    dirtyRect.width, dirtyRect.height );
            } else {

                    RasterCopyC32( backBuffer + dirtyRect.y * vgSurfaceDesc->width, vgSurfaceDesc->width, dirtyRect.x,
                        frontBuffer + dirtyRect.y * frontPixelStride, frontPixelStride, dirtyRect.x,
                        dirtyRect.width, dirtyRect.height );

            }

            // unlock window front buffer
            WMIntUnlockWindowBuffer(surfaceDesc->nativeWindow);
#else
            surfaceBuffer32 = (DWORD*)vgGetSurface(vgSurfaceDesc->vgSurface, &surfaceFormat);
            if(!surfaceBuffer32){
                WARN("Could not get the surface");
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
                return EGL_FALSE;
            }

            // Adjust the surface buffer to the correct y position
            surfaceBuffer32 += windowYPos * surfaceDesc->nativeWindow->windowSize.width;



            LcdGetBuffer( dpyContext->id, LCD_FRONT_BUFFER, &frontBuffer, &frontBufferFormat, &frontBufferStride );

            frontBufferTemp = frontBuffer;


            // Adjust the back buffer to the correct y position
            frontBufferTemp += lcdYPos * lcdCfg->width;
            RasterARGB8888ToRGB565Copy( surfaceBuffer32, surfaceDesc->nativeWindow->windowSize.width, windowXPos,
                frontBufferTemp, lcdCfg->width, lcdXPos,
                surfaceDesc->nativeWindow->clipRect.width, surfaceDesc->nativeWindow->clipRect.height );

#endif // RIM_WINDOW_MANAGER
        }
        else{

            WARN("Bad surface don't know what to do");
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
            return EGL_FALSE;
        }

#ifndef RIM_WINDOW_MANAGER


        LcdUpdateInt(dpyContext->id, lcdXPos, lcdYPos, surfaceDesc->nativeWindow->clipRect.width,
            surfaceDesc->nativeWindow->clipRect.height);
#endif // RIM_WINDOW_MANAGER
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
        return EGL_TRUE;

    }

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean eglTerminateAmanithQC( EGLDisplay dpy )
{
    EGLBoolean retBool;
    PRINT("eglTerminateAmanithQC");
    // Nothing to do for Amanith
    retBool = qceglTerminate(dpy);
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(qceglGetError()));
    return retBool;
}

EGLBoolean eglBindTexImageAmanithQC( EGLDisplay dpy, EGLSurface surface, EGLint buffer )
{
    EGLBoolean retBool = EGL_FALSE;
    EGLVendorSurfaceAmanithQC *vendorSurface = (EGLVendorSurfaceAmanithQC *)surface;

    PRINT("eglBindTexImageAmanithQC");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));

    if(vendorSurface){
        if(vendorSurface->vendorID == EGL_VENDOR_QC){
            PRINT("Binding QC Surface");
            WARN("Function not implemented");
        }else if(vendorSurface->vendorID == EGL_VENDOR_AMANITH){
            PRINT("Binding Amanith surface");
            WARN("Function not implemented");
        }
    }

    return retBool;
}

EGLBoolean eglReleaseTexImageAmanithQC( EGLDisplay dpy, EGLSurface surface, EGLint buffer )
{
    EGLBoolean retBool = EGL_FALSE;
    EGLVendorSurfaceAmanithQC *vendorSurface = (EGLVendorSurfaceAmanithQC *)surface;

    PRINT("eglReleaseTexImageAmanithQC");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));

    if(vendorSurface){
        if(vendorSurface->vendorID == EGL_VENDOR_QC){
            PRINT("Releasing QC Surface");
            WARN("Function not implemented");
        }else if(vendorSurface->vendorID == EGL_VENDOR_AMANITH){
            PRINT("Releasing Amanith surface");
            WARN("Function not implemented");
        }
    }

    return retBool;
}

EGLBoolean eglChooseConfigAmanithQC( EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config )
{
    PRINT("eglChooseConfigAmanithQC");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean eglGetConfigsAmanithQC( EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config )
{
    PRINT("eglGetConfigsAmanithQC");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean eglGetConfigAttribAmanithQC( EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value )
{
    EGLBoolean retBool = EGL_FALSE;
    EGLVendorConfigAmanithQC *vendorConfig = (EGLVendorConfigAmanithQC *)config;

    PRINT("eglGetConfigAttribAmanithQC");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONFIG));
    if(vendorConfig){
        switch (vendorConfig->vendorID){
        case EGL_VENDOR_QC:
            retBool = qceglGetConfigAttrib( dpy, vendorConfig->vendorConfig, attribute, value );
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(qceglGetError()));
            break;
        case EGL_VENDOR_AMANITH:
            retBool = eglGetConfigAttribAmanith( dpy, vendorConfig->vendorConfig, attribute, value );
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorAmanith()));
            break;
        default:
            WARNN("Unknown VendorID 0x%02x",vendorConfig->vendorID);
            break;
        }
    }

    return retBool;
}
EGLBoolean eglReleaseThreadAmanithQC()
{
    WARN("This function is not supported in Amanith QC architechture");
    return EGL_FALSE;
}

const char *eglQueryStringAmanithQC( EGLDisplay dpy, EGLint name )
{
    static char strQueried[45];

    switch( name ) {
    case EGL_CLIENT_APIS:
        strcpy(strQueried, "OpenVG OpenGL_ES");
        break;
    case EGL_EXTENSIONS:
        strcpy(strQueried, "eglLockSurfaceKHR eglUnlockSurfaceKHR");
        break;
    case EGL_VENDOR:
        strcpy(strQueried, "Qualcomm");
        break;
    case EGL_VERSION:
        strcpy(strQueried, "1.2");
        break;
    default:
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_PARAMETER));
        return NULL;
    }

    return strQueried;
}
