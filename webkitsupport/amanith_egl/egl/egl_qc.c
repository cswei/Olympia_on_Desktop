/*******************************************************
 *  egl_qc.c
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
#include <unistd.h>
#include <graphicsinit.h>
#include <sys/mman.h>
#include <string.h>
#include <basetype.h>
#include <Raster.h>
#include <i_raster.h>
#include "kd.h"
#include <egl.h>
#include <egl_globals.h>
#include <config_egl.h>
#include <gl.h>
#include "bitmap.h"

typedef struct _BoundingRect {
    SWORD xPos;
    SWORD yPos;
    SWORD wide;
    SWORD high;
    BOOL created;
} BoundingRect;

#else
#include "bugdispc.h"
#include <graphics_mempool.h>
#include <i_graphics_mempool.h>
#include <Raster.h>
#include <i_raster.h>
#include <log_verbosity.h>
#include <lcd.h>
#include <i_lcd.h>
#include <raster.h>
#include <string.h>

#include <egl.h>
#include <egl_globals.h>
#include <config_egl.h>
#include <gl.h>
#include "bitmap.h"

#include <dmov_mem.h>
#include <vm_task.h>

#include "kd.h"
#include "windowmanager.h"
#include "i_windowmanager.h"
#endif

#if defined ( RIM_USES_QCOMM_GRAPHICS )
#include "egl_entry.h"
#endif


#define SRCFILE     FILE_EGL_QC
#define SRCGROUP    GROUP_GRAPHICS

#if defined ( RIM_USES_QCOMM_GRAPHICS )

#ifdef RIM_ELTRON6
/* DMOV is not working on ELTRON6. Do not use it. */
#undef USE_DMOV_MEMCPY
#else
#define USE_DMOV_MEMCPY
#endif

void * qceglGetColorBuffer(EGLDisplay venDisplay, EGLSurface venSurf);
    
extern void * eglGetColorBufferQUALCOMM(void);

extern void initializeSurface( EGLDisplay dpy, EGLSurfaceDescriptor *newEglSurface,
    BYTE surfaceType );

EGLBoolean qceglInitializeWrap( EGLDisplay dpy, EGLint *major, EGLint *minor )
{
    EGLDisplayContext *dpyContext = NULL;
    EGLBoolean retBool = EGL_TRUE;
    DWORD       i,j,k;
    EGLint      num_config;
    EGLint      numOfVendorConfigs;
    EGLConfig*  vendorConfigList;
    EGLint      renderableType = 0;
    EGLint      attribList[]={
        EGL_ALPHA_SIZE,                     0,
            EGL_BLUE_SIZE,                      0,
            EGL_GREEN_SIZE,                     0,
            EGL_RED_SIZE,                       0,
            EGL_SURFACE_TYPE,                   0,
            EGL_RENDERABLE_TYPE,                0,
            EGL_DEPTH_SIZE,                     0,
            
            /*
            EGL_BUFFER_SIZE,                    0,
            EGL_STENCIL_SIZE,                   0,
            EGL_CONFIG_CAVEAT,                  0,
            EGL_NATIVE_RENDERABLE,              0,
            EGL_NATIVE_VISUAL_TYPE,             0,
            EGL_SAMPLES,                        0,
            //EGL_LEVEL,                          0,
            EGL_SAMPLE_BUFFERS,                 0,
            EGL_BIND_TO_TEXTURE_RGB,            0,
            EGL_BIND_TO_TEXTURE_RGBA,           0,
            EGL_MIN_SWAP_INTERVAL,              0,
            EGL_MAX_SWAP_INTERVAL,              0,
            EGL_LUMINANCE_SIZE,                 0,
            //EGL_ALPHA_MASK_SIZE,                0,
            EGL_COLOR_BUFFER_TYPE,              0,
            */
            EGL_NONE
    };
    
    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "qceglInitializeWrap : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_DISPLAY);       
        return EGL_FALSE;
    }
    
    retBool = qceglInitialize( dpyContext->vendorDisplay, major, minor );
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)qceglGetError());           
    
    //need to populate our configs with all the configs awailable thru the
    //vendor implementation
    //figure out how many configs vendor has in total and allocate enough memory
    //for that
    qceglGetConfigs(dpyContext->vendorDisplay, NULL, 0, &numOfVendorConfigs);
    PRINTN("Vendor has %d configs total", numOfVendorConfigs);
    
    vendorConfigList = (EGLConfig *)kdMalloc(sizeof(EGLConfig)*numOfVendorConfigs);
    for(i=0; i<TOTAL_NUMBER_OF_CONFIGS;i++){
        // Generate the attribute list
        for(j=0;  attribList[2*j]!=EGL_NONE; j++){
            attribList[2*j+1] = eglConfigDescriptors[i].ourConfig[CONFIG_ELEMENT(attribList[2*j])];
            // When the renderable type is 0, don't bother looking for a vendor config
            if(attribList[2*j] == EGL_RENDERABLE_TYPE){
                renderableType = attribList[2*j+1];
            }
        }
        
        // Since we only use pbuffers internally, we need to make sure that
        // we get config that supports the PBUFFER surfaces
        if( (eglConfigDescriptors[i].ourConfig[CONFIG_ELEMENT(EGL_SURFACE_TYPE)] & (EGL_WINDOW_BIT)) ||
            (eglConfigDescriptors[i].ourConfig[CONFIG_ELEMENT(EGL_SURFACE_TYPE)] & (EGL_PBUFFER_BIT|EGL_WINDOW_BIT))){
            attribList[4*2+1] = EGL_PBUFFER_BIT; //set the EGL_SURFACE_TYPE    = EGL_PBUFFER_BIT
        }
        
        // Intialize the vendor config map
        dpyContext->vendorConfigMap[i].localConfig = (EGLConfigDescriptor*)&eglConfigDescriptors[i];
        dpyContext->vendorConfigMap[i].vendorConfigAvail = FALSE;
        dpyContext->vendorConfigMap[i].vendorConfig = EGL_NO_CONTEXT;
        
        if(renderableType != 0) {
            // Try choosing the vendor config
            if(qceglChooseConfig( dpyContext->vendorDisplay, attribList, vendorConfigList, numOfVendorConfigs, &num_config ) && num_config >=1){
                EGLint redVal, greenVal, blueVal, alphaVal, depth;
                PRINT2N("Found %d vendor config to match our %d", num_config, i+1);
                for(k=0; k<num_config;k++){
                    //manually check the colorspace of the config that we got
                    qceglGetConfigAttrib( dpyContext->vendorDisplay, vendorConfigList[k], EGL_RED_SIZE, &redVal);
                    qceglGetConfigAttrib( dpyContext->vendorDisplay, vendorConfigList[k], EGL_GREEN_SIZE, &greenVal);
                    qceglGetConfigAttrib( dpyContext->vendorDisplay, vendorConfigList[k], EGL_BLUE_SIZE, &blueVal);
                    qceglGetConfigAttrib( dpyContext->vendorDisplay, vendorConfigList[k], EGL_ALPHA_SIZE, &alphaVal);
                    qceglGetConfigAttrib( dpyContext->vendorDisplay, vendorConfigList[k], EGL_DEPTH_SIZE, &depth);
                    if( redVal != eglConfigDescriptors[i].ourConfig[CONFIG_ELEMENT(EGL_RED_SIZE)] ||
                        greenVal != eglConfigDescriptors[i].ourConfig[CONFIG_ELEMENT(EGL_GREEN_SIZE)] ||
                        blueVal != eglConfigDescriptors[i].ourConfig[CONFIG_ELEMENT(EGL_BLUE_SIZE)] ||
                        alphaVal != eglConfigDescriptors[i].ourConfig[CONFIG_ELEMENT(EGL_ALPHA_SIZE)] ||
                        depth != eglConfigDescriptors[i].ourConfig[CONFIG_ELEMENT(EGL_DEPTH_SIZE)]){
                        
                        PRINTN("Vendor config index %d doesn't match the native config colourspace", k);
                        PRINT2N("Native Red %d, Vendor Red %d",  eglConfigDescriptors[i].ourConfig[CONFIG_ELEMENT(EGL_RED_SIZE)], redVal);
                        PRINT2N("Native Green %d, Vendor Green %d",  eglConfigDescriptors[i].ourConfig[CONFIG_ELEMENT(EGL_GREEN_SIZE)], greenVal);
                        PRINT2N("Native Blue %d, Vendor Blue %d",  eglConfigDescriptors[i].ourConfig[CONFIG_ELEMENT(EGL_BLUE_SIZE)], blueVal);
                        PRINT2N("Native Alpha %d, Vendor Alpha %d",  eglConfigDescriptors[i].ourConfig[CONFIG_ELEMENT(EGL_ALPHA_SIZE)], alphaVal);
                        PRINT2N("Native Depth %d, Vendor Depth %d",  eglConfigDescriptors[i].ourConfig[CONFIG_ELEMENT(EGL_DEPTH_SIZE)], depth);
                    }else{
                        //found matching config
                        PRINT3N("Found native config 0x%08x (index %d) to match our %d", (DWORD)vendorConfigList[k], k, i+1);
                        dpyContext->vendorConfigMap[i].vendorConfigAvail = TRUE;
                        dpyContext->vendorConfigMap[i].vendorConfig = vendorConfigList[k];
                        break;
                    }
                }
            }else{
                PRINT2N("No config to match our %d ( 0x%04x )", i+1,qceglGetError() );
            }
        }
    }
    kdFree(vendorConfigList);
    return retBool;
}

EGLBoolean qceglSwapBuffersWrap(EGLDisplay dpy, EGLSurface surface)
{
    EGLDisplayContext *dpyContext = NULL;
    EGLSurfaceDescriptor * surfaceDesc = (EGLSurfaceDescriptor *)surface;
    LcdConfig *lcdCfg = NULL;
    
    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "qceglSwapBuffersWrap : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_DISPLAY);
        return EGL_FALSE;
    }
    
    lcdCfg = LcdGetLcdConfigPtr( dpyContext->id );
    
    if(surfaceDesc != EGL_NO_SURFACE && surfaceDesc->surfaceType == EGL_WINDOW_BIT){
#ifdef RIM_WINDOW_MANAGER
        WMError_t       result;
        DWORD           backPixelStride;
        DWORD           frontBitDepth;
        DWORD           frontPixelStride;
        SDWORD          temp[4];
        Rect            dirtyRect;
        DWORD          *backBuffer;
        DWORD          *frontBuffer;

        //PRINT2N("qceglSwapBuffersWrap surface=0x%08x v0x%08x", surface, surfaceDesc->vendorSurface);
        
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
            WARN("qceglSwapBuffersWrap: bad native window"); 
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_NATIVE_WINDOW);
            return EGL_FALSE;
        }
        dirtyRect.x = temp[0];
        dirtyRect.y = temp[1];
        dirtyRect.width = temp[2];
        dirtyRect.height = temp[3];

        glFinish();

        // get vendor back buffer
        backBuffer = (DWORD *) qceglGetColorBuffer(dpyContext->vendorDisplay,surfaceDesc->vendorSurface);
        if(!backBuffer){
            WARN("Could not get the surface");
 
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_SURFACE);
            return EGL_FALSE;
        }
        PRINTN("Color buffer returned 0x%08x", (DWORD)backBuffer);

        // calculate stride of back buffer (must be 8-pixel aligned regardless of bit depth)
        backPixelStride = (surfaceDesc->frontBuffer.wide + 7) & ~7;

        // lock window front buffer
        result = WMIntLockWindowBuffer(surfaceDesc->nativeWindow, NULL);
        if (result != WM_E_OK) {
            WARN("qceglSwapBuffersWrap: bad native window"); 
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_NATIVE_WINDOW);
            return EGL_FALSE;
        }

        if (surfaceDesc->bitDepth == 16){
            // If the stride is the same as the lcd width we can use the faster dmov_memcpy
            /*if( dirtyRect.width == lcdCfg->width && lcdCfg->width == backPixelStride && backPixelStride == frontPixelStride ){
                LV1( LOG_EGL, PRINT("QCOMM dmov_memcpy"));
                dmov_memcpy( ((WORD*)backBuffer) + dirtyRect.y * frontPixelStride, 
                    ((WORD*)frontBuffer) + dirtyRect.y * frontPixelStride, 
                    dirtyRect.width * dirtyRect.height * 2,
                    DMOV_MEM_CACHE_DEST_ONLY, VM_FINISH_DMOV_MEMCPY_SIG );
            }else{*/
                RasterCopyC16( ((WORD*)backBuffer) + dirtyRect.y * backPixelStride, backPixelStride, dirtyRect.x,
                    ((WORD*)frontBuffer) + dirtyRect.y * frontPixelStride, frontPixelStride, dirtyRect.x,
                    dirtyRect.width, dirtyRect.height );
            //}
        }else if (surfaceDesc->bitDepth == 32){
            // If the stride is the same as the lcd width we can use the faster dmov_memcpy
            /*if( dirtyRect.width == lcdCfg->width && lcdCfg->width == backPixelStride && backPixelStride == frontPixelStride ){
                LV1( LOG_EGL, PRINT("QCOMM dmov_memcpy"));
                dmov_memcpy( backBuffer + dirtyRect.y * frontPixelStride,
                    frontBuffer + dirtyRect.y * frontPixelStride,
                    dirtyRect.width * dirtyRect.height * 4,
                    DMOV_MEM_CACHE_DEST_ONLY, VM_FINISH_DMOV_MEMCPY_SIG );
            }else{*/
                RasterCopyC32( backBuffer + dirtyRect.y * backPixelStride, backPixelStride, dirtyRect.x,
                    frontBuffer + dirtyRect.y * frontPixelStride, frontPixelStride, dirtyRect.x,
                    dirtyRect.width, dirtyRect.height );
            //}
        }

        // unlock window front buffer
        WMIntUnlockWindowBuffer(surfaceDesc->nativeWindow);

        //PRINT2N("qceglSwapBuffersWrap surface=0x%08x v0x%08x done", surface, surfaceDesc->vendorSurface);
        
#else
        BoundingRect    frameWindow;
        WORD           *backBuffer;
        WORD           *backBufferTemp;
        DWORD           backBufferStride;
        DWORD           backBufferFormat;
        WORD           *surfaceBuffer;
        
        DWORD lcdXPos = surfaceDesc->nativeWindow->windowSize.x + surfaceDesc->nativeWindow->clipRect.x;
        DWORD lcdYPos = lcdCfg->height - surfaceDesc->nativeWindow->windowSize.y - surfaceDesc->nativeWindow->clipRect.y 
            - surfaceDesc->nativeWindow->clipRect.height;
        DWORD windowXPos = surfaceDesc->nativeWindow->clipRect.x;
        DWORD windowYPos = surfaceDesc->nativeWindow->windowSize.height - surfaceDesc->nativeWindow->clipRect.y - surfaceDesc->nativeWindow->clipRect.height;
        
        glFinish();
        
#if defined(RIM_ELTRON6) || defined(AMSS_ON_NESSUS)
        /* In ELTRON6 there is no support for eglGetColorBufferQUALCOMM. There will be */
        /* standard extenion we can use, but they are not available yet. Until then    */
        /* directly use QC's swap buffer which will update the LCD via update manager  */
        return qceglSwapBuffers(dpyContext->vendorDisplay, surfaceDesc->vendorSurface);
#else
        surfaceBuffer = (WORD *) eglGetColorBufferQUALCOMM();

        if(!surfaceBuffer){
            WARN("Could not get the surface"); 
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_SURFACE);
            return EGL_FALSE;
        }
        PRINTN("Color buffer returned 0x%08x", (DWORD)surfaceBuffer);
        // Adjust the surface buffer to the correct y position
        surfaceBuffer += windowYPos * surfaceDesc->nativeWindow->windowSize.width;
        
        LcdSyncBegin();
        LcdGetBuffer( dpyContext->id, LCD_BACK_BUFFER, &backBuffer, &backBufferFormat, &backBufferStride );
        backBufferTemp = backBuffer;
        // Adjust the back buffer to the correct y position
        backBufferTemp += lcdYPos*lcdCfg->width;
        
        // If the stride is the same as the lcd width we can use the faster dmov_memcpy
        if (surfaceDesc->bitDepth == 16){
#ifdef USE_DMOV_MEMCPY
            if( surfaceDesc->nativeWindow->clipRect.width == lcdCfg->width ){
                LV1( LOG_EGL, PRINT("QCOMM dmov_memcpy"));
                dmov_memcpy(backBufferTemp, surfaceBuffer, surfaceDesc->nativeWindow->clipRect.width * surfaceDesc->nativeWindow->clipRect.height * 2,
                    DMOV_MEM_CACHE_DEST_ONLY, VM_FINISH_DMOV_MEMCPY_SIG);
            }else
#endif /* USE_DMOV_MEMCPY */
            {
                RasterCopyC16( surfaceBuffer, surfaceDesc->nativeWindow->windowSize.width, windowXPos,
                    backBufferTemp, lcdCfg->width, lcdXPos,
                    surfaceDesc->nativeWindow->clipRect.width, surfaceDesc->nativeWindow->clipRect.height );
            }
        }else if (surfaceDesc->bitDepth == 32){
            RasterARGB8888ToRGB565Copy( (DWORD*)surfaceBuffer, surfaceDesc->nativeWindow->windowSize.width, windowXPos,
                backBufferTemp, lcdCfg->width, lcdXPos,
                surfaceDesc->nativeWindow->clipRect.width, surfaceDesc->nativeWindow->clipRect.height );
            
        }
        LcdSyncEnd();
        
        frameWindow.xPos = lcdXPos;
        frameWindow.yPos = lcdYPos;
        frameWindow.wide = surfaceDesc->nativeWindow->clipRect.width;
        frameWindow.high = surfaceDesc->nativeWindow->clipRect.height;
        
        LcdUpdateFrame(dpyContext->id, (BYTE*)backBuffer, ROWWISE_16BIT_COLOUR_BITMAP, &frameWindow);
#endif /* RIM_ELTRON6 */
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_SUCCESS);

        return EGL_TRUE;
#endif // RIM_WINDOW_MANAGER 
    }
    return EGL_TRUE;
}

// NOTE: Takes in RIM EGL parameters
EGLBoolean qceglCopyBuffersWrap(EGLDisplay dpy, EGLSurface surface,
                                NativePixmapType target)
{
    EGLSurfaceDescriptor *surfaceDesc = (EGLSurfaceDescriptor *)surface;
    WORD *surfaceBuffer16 = NULL;
    DWORD *surfaceBuffer32 = NULL;
    
    //call glfinish() to flush the pipeline
    //NOTE : We are assuming here that qualcomm surfaces are gl surfaces
    //which would need to be changed if we decide to use their vg solution
    // in the future
    glFinish();

#if defined(RIM_ELTRON6) || defined(AMSS_ON_NESSUS)
    {
        EGLDisplayContext *dpyContext = NULL;
        
        // Get the EGLDisplayContext
        dpyContext = eglGetDisplayContextInt( dpy );

        PRINT("Calling qceglCopyBuffers()!!");
        /* In ELTRON6 there is no support for eglGetColorBufferQUALCOMM. There will be */
        /* standard extenion we can use, but they are not available yet. Until then    */
        /* directly use QC's copy buffer which will update the LCD via update manager  */
        return qceglCopyBuffers( dpyContext->vendorDisplay, surfaceDesc->vendorSurface, target);
    }
#else

    
    if(surfaceDesc->bitDepth == 32){
        surfaceBuffer32 = (DWORD *) eglGetColorBufferQUALCOMM();
        if(!surfaceBuffer32){ 
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_SURFACE);
            return EGL_FALSE;
        }
        // 32 bit surface is ARGB8888
        switch( target->bType ) {
            case BMT_16BPP_RGB565:
                RasterARGB8888ToRGB565Copy( (DWORD *)surfaceBuffer32, surfaceDesc->width, 0,
                    (WORD*)target->data, target->stride/sizeof(WORD), 0,
                    (target->wide > surfaceDesc->width) ? surfaceDesc->width : target->wide,
                    (target->high > surfaceDesc->height) ? surfaceDesc->height : target->high );
                break;
            case BMT_32BPP_XRGB8888:
            case BMT_32BPP_ARGB8888:
            case BMT_32BPP_ARGB8888_PMA:
                RasterCopyC32( (DWORD *)surfaceBuffer32, surfaceDesc->width, 0,
                    (DWORD*)target->data, target->stride/sizeof(DWORD), 0,
                    (target->wide > surfaceDesc->width) ? surfaceDesc->width : target->wide,
                    (target->high > surfaceDesc->height) ? surfaceDesc->height : target->high );
                break;
            default:
                WARNN("Unsupported target pixmap type: %x", target->bType);
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_MATCH);
                return EGL_FALSE;
        
        }
    }else if(surfaceDesc->bitDepth == 16){
        surfaceBuffer16 = (WORD *) eglGetColorBufferQUALCOMM();
        if(!surfaceBuffer16){ 
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_SURFACE);            
            return EGL_FALSE;
        }
#ifdef USE_DMOV_MEMCPY
        if((surfaceDesc->width * sizeof(WORD)) == target->stride){
            DWORD copyHeight;
            
            // If the source and destination formats are both 16 bit and their strides are the same, use the faster memcpy
            copyHeight = (target->high > surfaceDesc->height) ? surfaceDesc->height : target->high;
            dmov_memcpy(target->data, surfaceBuffer16, target->wide * copyHeight * sizeof(WORD), DMOV_MEM_CACHE_DEST_ONLY, VM_FINISH_DMOV_MEMCPY_SIG);
        }else
#endif /* USE_DMOV_MEMCPY */
        {
            switch( target->bType ) {
                case BMT_16BPP_RGB565: 
                    RasterCopyC16( (WORD *)surfaceBuffer16, surfaceDesc->width, 0,
                        (WORD*)target->data, target->stride/sizeof(WORD), 0,
                        (target->wide > surfaceDesc->width) ? surfaceDesc->width : target->wide,
                        (target->high > surfaceDesc->height) ? surfaceDesc->height : target->high );
                    break;
                case BMT_32BPP_XRGB8888:
                case BMT_32BPP_ARGB8888:                        
                case BMT_32BPP_ARGB8888_PMA:                        
                     RasterRGB565A8PreToARGB8888Copy( (WORD *)surfaceDesc->backBuffer.data, surfaceDesc->width, 
                        NULL, 0, (DWORD *)target->data, target->stride/sizeof(DWORD),
                        (target->wide > surfaceDesc->width) ? surfaceDesc->width : target->wide,
                        (target->high > surfaceDesc->height) ? surfaceDesc->height : target->high );     
                     break;
                case BMT_32BPP_RGBX8888:
                case BMT_32BPP_RGBA8888:                   
                case BMT_32BPP_RGBA8888_PMA:
                     RasterRGB565A8PreToRGBA8888Copy( (WORD *)surfaceDesc->backBuffer.data, surfaceDesc->width, 
                        NULL, 0, (DWORD *)target->data, target->stride/sizeof(DWORD),
                        (target->wide > surfaceDesc->width) ? surfaceDesc->width : target->wide,
                        (target->high > surfaceDesc->height) ? surfaceDesc->height : target->high ); 
                    break;
                default:
                WARNN("Unsupported target pixmap type: %x", target->bType);
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_MATCH);
                return EGL_FALSE;

            }

        }
    }else{
        WARNN("Unknown bit depth %d",surfaceDesc->bitDepth); 
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_SURFACE);
        return EGL_FALSE;
    }
#endif
     
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_SUCCESS);
    return EGL_TRUE;
}


EGLSurface qceglCreatePixmapSurfaceWrap( EGLDisplay dpy, EGLConfig config,
                                        NativePixmapType pixmap,
                                        const EGLint *attrib_list )
{
    EGLDisplayContext *dpyContext = NULL;
    EGLSurface retSurf;
    EGLConfigContextPtr currentConfigContext;
    DWORD config_id = (DWORD)config;
    EGLSurfaceDescriptor * newEglSurface = EGL_NO_SURFACE;
    
    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "qceglCreatePixmapSurfaceWrap : bad display" ); 
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_DISPLAY);
        return EGL_FALSE;
    }
    
    currentConfigContext = dpyContext->vendorConfigMap[config_id-1].localConfig->ourConfig;
    PRINT("QCOMM eglCreatePixmapSurface");
    retSurf = qceglCreatePixmapSurface(dpyContext->vendorDisplay, dpyContext->vendorConfigMap[config_id-1].vendorConfig,
        NULL, attrib_list); 
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(qceglGetError()));
    
    if(retSurf!=EGL_NO_SURFACE){
        newEglSurface = (EGLSurfaceDescriptor*)kdMalloc(sizeof(EGLSurfaceDescriptor));
        if(!newEglSurface){
            qceglDestroySurface(dpyContext->vendorDisplay, retSurf); 
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_ALLOC);
            return EGL_NO_SURFACE;
        }
        initializeSurface(dpyContext->vendorDisplay, newEglSurface, EGL_PIXMAP_BIT);
        
        // Querying the bit depth of the config
        newEglSurface->bitDepth = currentConfigContext[CONFIG_ELEMENT(EGL_BUFFER_SIZE)];
        
        newEglSurface->vendorSurface = retSurf;
        newEglSurface->ownsBackBuffer = FALSE;
        memcpy(&newEglSurface->backBuffer, pixmap, sizeof(BitMap));
        newEglSurface->ourConfigId = config_id;
    }
    
    return newEglSurface;
}

EGLBoolean qceglMakeCurrentWrap(EGLDisplay dpy, EGLSurface draw,
                                EGLSurface read, EGLContext ctx)
{
    EGLBoolean retValue = EGL_TRUE;
    EGLDisplayContext *dpyContext = NULL; 
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_SUCCESS);
        
    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "qceglMakeCurrentWrap : bad display" ); 
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_DISPLAY);
        return EGL_FALSE;
    }
    
    //special case when called with all paraeters NULL
    if( ctx == EGL_NO_CONTEXT && draw == EGL_NO_SURFACE && read == EGL_NO_SURFACE ){
        retValue = qceglMakeCurrent( dpyContext->vendorDisplay, NULL, NULL, NULL ); 
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)qceglGetError());
        retValue = retValue;
    }else if( ((EGLSurfaceDescriptor *)draw)->vendorSurface!=EGL_NO_SURFACE &&
        ((EGLSurfaceDescriptor *)read)->vendorSurface!=EGL_NO_SURFACE &&
        ((EGLRenderingContext *)ctx)->vendorContext!=EGL_NO_CONTEXT){
        LV1( LOG_EGL, PRINT("QC eglMakeCurrent"));
        retValue = qceglMakeCurrent( dpyContext->vendorDisplay, ((EGLSurfaceDescriptor *)draw)->vendorSurface, ((EGLSurfaceDescriptor *)read)->vendorSurface, ((EGLRenderingContext *)ctx)->vendorContext ); 
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)qceglGetError());
    }else if( ((EGLSurfaceDescriptor *)draw)->vendorSurface!=EGL_NO_SURFACE ||
        ((EGLSurfaceDescriptor *)read)->vendorSurface!=EGL_NO_SURFACE ||
        ((EGLRenderingContext *)ctx)->vendorContext!=EGL_NO_CONTEXT){
        //can't mix our implementation and vendor implementation
        WARN(" Can't mix surfaces and context from different EGL implementations");
        PRINT3N("Draw 0x%08x, read 0x%08x, ctx 0x%08x", (DWORD)((EGLSurfaceDescriptor *)draw)->vendorSurface, (DWORD)((EGLSurfaceDescriptor *)read)->vendorSurface, (DWORD)((EGLRenderingContext *)ctx)->vendorContext ); 
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_MATCH);
        retValue = EGL_FALSE;
    }
    return retValue;
}


#if defined(RIM_ELTRON5)

extern EGLBoolean qceglLockSurfaceKHR(EGLDisplay venDisplay, EGLSurface venSurf, EGLint *lock_attrib_list);
extern EGLBoolean qceglUnlockSurfaceKHR(EGLDisplay venDisplay, EGLSurface venSurf);

void * qceglGetColorBuffer(EGLDisplay venDisplay, EGLSurface venSurf)
{
   DWORD       *buff = NULL;
   EGLint      lock_attrib_list[] = {
       EGL_MAP_PRESERVE_PIXELS_KHR,    EGL_TRUE,
       EGL_LOCK_USAGE_HINT_KHR,        EGL_READ_SURFACE_BIT_KHR | EGL_WRITE_SURFACE_BIT_KHR,
       EGL_NONE
   };

   if (qceglLockSurfaceKHR(venDisplay, venSurf, lock_attrib_list) != EGL_TRUE) {
      WARNN("qceglLockSurceKHR failed, rc=0x%08x", qceglGetError());
      return NULL;
   }

   if (qceglQuerySurface(venDisplay, venSurf, EGL_BITMAP_POINTER_KHR, (EGLint *)&buff ) != EGL_TRUE) {
      WARNN("eglQuerySurface failed, rc=0x%08x", qceglGetError());
      buff = NULL;
   }

   qceglUnlockSurfaceKHR(venDisplay, venSurf);

   return ((void*)buff);
} /* qceglGetColorBuffer */

#elif defined(RIM_ELTRON6) || defined(AMSS_ON_NESSUS)

extern EGLBoolean qeglShimAPI_eglLockSurfaceKHR(EGLDisplay venDisplay, EGLSurface venSurf, EGLint *lock_attrib_list);
extern EGLBoolean qeglShimAPI_eglUnlockSurfaceKHR(EGLDisplay venDisplay, EGLSurface venSurf);

void * qceglGetColorBuffer(EGLDisplay venDisplay, EGLSurface venSurf)
{
   DWORD       *buff = NULL;
   EGLint      lock_attrib_list[] = {
       EGL_MAP_PRESERVE_PIXELS_KHR,    EGL_TRUE,
       EGL_LOCK_USAGE_HINT_KHR,        EGL_READ_SURFACE_BIT_KHR | EGL_WRITE_SURFACE_BIT_KHR,
       EGL_NONE
   };

   //PRINT2N("qceglGetColorBuffer venDisplay=0x%08x v0x%08x", venDisplay, venSurf);

   if (qeglShimAPI_eglLockSurfaceKHR(venDisplay, venSurf, lock_attrib_list) != EGL_TRUE) {
      WARNN("qceglLockSurceKHR failed, rc=0x%08x", qceglGetError());
      return NULL;
   }

   if (qceglQuerySurface(venDisplay, venSurf, EGL_BITMAP_POINTER_KHR, (EGLint *)&buff ) != EGL_TRUE) {
      WARNN("eglQuerySurface failed, rc=0x%08x", qceglGetError());
      buff = NULL;
   }

   qeglShimAPI_eglUnlockSurfaceKHR(venDisplay, venSurf);

   //PRINT3N("qceglGetColorBuffer venDisplay=0x%08x v0x%08x buffer=0x%08x done", venDisplay, venSurf, buff);

   return ((void*)buff);
} /* qceglGetColorBuffer */

#else
extern void * eglGetColorBufferQUALCOMM(void);
void * qceglGetColorBuffer(EGLDisplay vendorDisplay, EGLSurface vendorSurface)
{
   return eglGetColorBufferQUALCOMM();
} /* qceglGetColorBuffer */
#endif

#ifdef RIM_USES_AMANITH_OVG

/* In this case, we disable QC's OpenVG. This requires the following stubs */
/* to be provided for the OpenGL lib.                                      */

void vgiSwapInterval(void) {WARN("Unimplemented...");};
void vgiProcessAllQueuedPrimitiveLists(void) {WARN("Unimplemented...");};

void vgiSwapBuffers(void) {WARN("Unimplemented...");};
void vgiMakeCurrent(void) {WARN("Unimplemented...");};
void vgiWaitVG(void) {WARN("Unimplemented...");};
void vgiInitContext(void) {WARN("Unimplemented...");};
void vgiWipeContext(void) {WARN("Unimplemented...");};
void vgiCloseDisplay(void) {WARN("Unimplemented...");};
EGLBoolean vgiInitDisplay(void) {WARN("Unimplemented..."); return EGL_SUCCESS;};
void QVG_vgReadPixels(void) {WARN("Unimplemented...");};
void vgiGetSurfaceDimensions(void) {WARN("Unimplemented...");};
void vgiAllocPBufferSurface(void) {WARN("Unimplemented...");};
void vgiDeleteSurfaceMem(void) {WARN("Unimplemented...");};
void vgiAllocNewSurface(void) {WARN("Unimplemented...");};
void vgiFreeCompositeSurface(void) {WARN("Unimplemented...");};
void vgiSurfaceOverlayBind(void) {WARN("Unimplemented...");};
void vgiSurfaceOverlayLayerEnable(void) {WARN("Unimplemented...");};
void vgiSurfaceOverlayEnable(void) {WARN("Unimplemented...");};
void vgiCreateCompositeSurface(void) {WARN("Unimplemented...");};
void vgiCreateNewCompositeSurface(void) {WARN("Unimplemented...");};
void vgiSetSurfaceColorKey(void) {WARN("Unimplemented...");};
void vgiSurfaceColorKeyEnable(void) {WARN("Unimplemented...");};
void vgiSetSurfaceTransparencyMap(void) {WARN("Unimplemented...");};
void vgiSetSurfaceTransparency(void) {WARN("Unimplemented...");};
void vgiSurfaceTransparencyEnable(void) {WARN("Unimplemented...");};
void vgiSetSurfaceRotate(void) {WARN("Unimplemented...");};
void vgiSurfaceRotateEnable(void) {WARN("Unimplemented...");};
void vgiSetSurfaceScale(void) {WARN("Unimplemented...");};
void vgiSurfaceScaleEnable(void) {WARN("Unimplemented...");};
void vgiSetWindowSurfaceClipRegion(void) {WARN("Unimplemented...");};
void vgiPreserveWindowSurfaceContents(void) {WARN("Unimplemented...");};
void vgiAllocPixmapSurface(void) {WARN("Unimplemented...");};
void vgiMapWindowSurface(void) {WARN("Unimplemented...");};
void vgiGetWindowSurfaceDimensions(void) {WARN("Unimplemented...");};
void vgiAllocWindowSurface(void) {WARN("Unimplemented...");};

void vgiGetProcAddress(void) {WARN("Unimplemented...");};

vgiSetScissorRect(void) {WARN("Unimplemented...");};
#endif

#endif

