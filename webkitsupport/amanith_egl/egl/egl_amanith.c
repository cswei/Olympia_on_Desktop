/*******************************************************
 *  egl_amanith.c
 *
 *  Copyright 2007, Research In Motion Ltd.
 *
 *  Contains the specific interface between EGL and AmanithVG
 *  Designed to conform with Khronos EGL 1.2 standard
 *
 *  Russell Andrade, July 26, 2007
 *
 *******************************************************/
//#define EGL_EXTRA_CONFIG_DEBUG

#if defined(RIM_NDK)
    #include <egl_user_types.h>
#else
    #include "bugdispc.h"
    #include "failure.h"

    #define SRCFILE     FILE_EGL_AMANITH
    #define SRCGROUP    GROUP_GRAPHICS

    #include <log_verbosity.h>

    #include <graphics_mempool.h>
    #include <i_graphics_mempool.h>
    #include <i_lcd.h>
#endif

#include <KHR_thread_storage.h>
#include <basetype.h>
#include <Raster.h>
#include <i_raster.h>
#include <string.h>
#include "bitmap.h"

#include "egl_amanith.h"
#include "egl_amanith_globals.h"
#include "config_egl.h"
#include "i_egl.h" // for eglInitExtension()
#include "vg_priv.h"

#include "kd.h"

#ifdef RIM_WINDOW_MANAGER
#include "windowmanager.h"
#include "i_windowmanager.h"
#endif

//This is the TLS key used to store the current vg context for each thread.
KDThreadStorageKeyKHR vgCurrentContextKey = 0;

// Initialize the Amanith thread to context map to NULL.
void eglInitAmanith(void) {

    kdLogMessage("eglInitAmanith");
    // Create TLS key
    vgCurrentContextKey = kdMapThreadStorageKHR(NULL);
    kdSetThreadStorageKHR(vgCurrentContextKey, (void *)(NULL));

#if defined RIM_VG_SRC
    vgInitAmanith();
#endif
}
 
EGLint eglGetErrorAmanith( void )
{
    kdLogMessage("eglGetErrorAmanith");

    if((EGLint)kdGetThreadStorageKHR(eglLastErrorKey) != EGL_SUCCESS)
        WARNN("  error: 0x%04x",(EGLint)kdGetThreadStorageKHR(eglLastErrorKey));
    return (EGLint)kdGetThreadStorageKHR(eglLastErrorKey);
}

EGLDisplay eglGetDisplayAmanith( NativeDisplayType display_id )
{
    DWORD i;
    EGLDisplay newDisplay = EGL_NO_DISPLAY;
    kdLogMessage("eglGetDisplayAmanith");
    // Search through the EGLDisplayContext table to find a matching display id
    // We should not generate an error condition here, because this code is only run if this has already been successful
    for(i=0; i < RIM_LCD_NUM_DISPLAYS; i++){
        if(EglDpyContext[i].id == display_id){
            // Grab the EGLDisplay handle
            newDisplay = EglDpyContext[i].handle;
            break;
        }
    }
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_SUCCESS);
    return newDisplay;
}

EGLBoolean eglInitializeAmanith( EGLDisplay dpy, EGLint *major, EGLint *minor )
{
    DWORD i;
    EGLDisplayContext *dpyContext = NULL;

    kdLogMessage("eglInitializeAmanith");

    // Get the EGLDisplayContext, we already checked if it's valid
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglInitializeAmanith : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_DISPLAY);
        return EGL_FALSE;
    }

    for(i=0; i<TOTAL_NUMBER_OF_CONFIGS; i++){
        dpyContext->vendorConfigMap[i].localConfig = (EGLConfigDescriptor*)&eglConfigDescriptors[i];
        // Only make vendor configs available for OpenVG only configs
        if(eglConfigDescriptors[i].ourConfig[CONFIG_ELEMENT(EGL_RENDERABLE_TYPE)] & EGL_OPENVG_BIT){
            PRINTN("Found vendor config %d",i+1);
            // Make a vendor config available and set it to our own
            dpyContext->vendorConfigMap[i].vendorConfigAvail = TRUE;
            dpyContext->vendorConfigMap[i].vendorConfig = (EGLConfig*)&eglConfigDescriptors[i];
        }else{
            PRINTN("No matching vendor config %d",i+1);
            dpyContext->vendorConfigMap[i].vendorConfigAvail = FALSE;
            dpyContext->vendorConfigMap[i].vendorConfig = NULL;
        }
    }

    //Initialize Amanith Extensions to match our function pointers
    eglInitExtension();
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean eglTerminateAmanith( EGLDisplay dpy )
{
    kdLogMessage("eglTerminateAmanith");
    eglInitAmanith();
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean eglGetConfigsAmanith( EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config )
{
    kdLogMessage("eglGetConfigsAmanith");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean eglGetConfigAttribAmanith( EGLDisplay dpy, EGLConfig config,
                                     EGLint attribute, EGLint *value )
{
    EGLConfigContextPtr currentConfigContext = NULL;

    kdLogMessage("eglGetConfigAttribAmanith");

    if(config){
        currentConfigContext = ((EGLConfigDescriptor*)config)->ourConfig;
    }else{
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONFIG));
        return EGL_FALSE;
    }

    *value = currentConfigContext[CONFIG_ELEMENT(attribute)];
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean eglChooseConfigAmanith( EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config )
{
    kdLogMessage("eglChooseConfigAmanith");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean eglBindAPIAmanith( EGLenum api )
{
    kdLogMessage("eglBindAPIAmanith");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLenum eglQueryAPIAmanith( void )
{
    kdLogMessage("eglQueryAPIAmanith");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    return (EGLenum)kdGetThreadStorageKHR(eglCurrentApiKey);
}

EGLBoolean eglMakeCurrentAmanith( EGLDisplay dpy, EGLSurface draw,
                                  EGLSurface read, EGLContext ctx )
{
    EGLDisplayContext *dpyContext = NULL;
    EGLSurfaceDescriptor *drawSurface = (EGLSurfaceDescriptor *)draw;
    EGLSurfaceDescriptor *readSurface = (EGLSurfaceDescriptor *)read;
    EGLRenderingContext *context = (EGLRenderingContext *)ctx;
    EGLBoolean retValue = EGL_TRUE;
    
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglMakeCurrentAmanith : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    // Special case when called with all paraeters NULL
    if( ctx == EGL_NO_CONTEXT && draw == EGL_NO_SURFACE && read == EGL_NO_SURFACE ){
        return eglMakeCurrentAmanithInt( dpyContext->vendorDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );

    } else if( ctx != EGL_NO_CONTEXT && draw != EGL_NO_SURFACE && read != EGL_NO_SURFACE ){
        if( drawSurface->vendorSurface != EGL_NO_SURFACE && readSurface->vendorSurface != EGL_NO_SURFACE && context->vendorContext != EGL_NO_CONTEXT){
            return eglMakeCurrentAmanithInt( dpyContext->vendorDisplay, drawSurface->vendorSurface, readSurface->vendorSurface, context->vendorContext );

        }else if( drawSurface->vendorSurface!=EGL_NO_SURFACE || readSurface->vendorSurface!=EGL_NO_SURFACE || context->vendorContext!=EGL_NO_CONTEXT){
            //Can't mix our implementation and vendor implementation
            WARN("Can't mix surfaces and context from different EGL implementations");
            PRINT3N("draw 0x%08x, read 0x%08x, ctx 0x%08x", drawSurface->vendorSurface, readSurface->vendorSurface, context->vendorContext );
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
            retValue = EGL_FALSE;
        }
        // Otherwise all of the parameters are from the RIM implementaion
    } else {
        kdCatfailRim( "Broken promise in eglMakeCurrentAmanith");
    }
    return retValue;

}

/**
 * Accepts Amanith Vendor Surfaces Only
 */
EGLBoolean eglMakeCurrentAmanithInt( EGLDisplay dpy, EGLSurface draw,
                                     EGLSurface read, EGLContext ctx )
{
    VGSurfaceDescriptor *readVGSurfaceDesc = (VGSurfaceDescriptor *)read;
    VGSurfaceDescriptor *drawVGSurfaceDesc = (VGSurfaceDescriptor *)draw;
    VGContextDescriptor *newVGContextDesc = (VGContextDescriptor *)ctx;
    VGContextDescriptor *currentVGContextDesc = NULL;

    kdLogMessage("eglMakeCurrentAmanithInt");
    //PRINT3N("read 0x%08x draw 0x%08x ctx 0x%08x",read,draw,ctx);

    currentVGContextDesc = kdGetThreadStorageKHR(vgCurrentContextKey);

    // Section 1: Check if the current surfaces and context needs to be destroyed
    if(currentVGContextDesc){
        // Check if the current surface needs to be cleaned up
        if(currentVGContextDesc->currentSurface){
            if(currentVGContextDesc->currentSurface->markForDelete){
                currentVGContextDesc->currentSurface->current = FALSE;
                PRINTN( "eglMakeCurrentAmanithInt: destroy surface 0x%08x", currentVGContextDesc->currentSurface );
                if(eglDestroySurfaceAmanith( dpy, (EGLSurface)(currentVGContextDesc->currentSurface) ) == EGL_TRUE){
                    kdLogMessage("Destroyed current surface");
                    currentVGContextDesc->currentSurface = NULL;
                    //eglDestroySurfaceAmanith will make the current AmanithVG surface NULL
                }else{
                    WARN("Failed destroying the surface marked for delete");
                    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
                    return EGL_FALSE;
                }
            }
        }

        // Check if the current context needs to be cleaned up
        if(currentVGContextDesc->markForDelete){
            currentVGContextDesc->current = FALSE;
            PRINTN( "eglMakeCurrentAmanithInt: destroy context 0x%08x", currentVGContextDesc );
            if(eglDestroyContextAmanith( dpy, (EGLContext)(currentVGContextDesc) ) == EGL_TRUE){
                kdLogMessage("Destroyed current context");
                kdSetThreadStorageKHR(vgCurrentContextKey, NULL);
                //eglDestroyContextAmanith will make the current AmanithVG context NULL
            }else{
                WARN("Failed destroying the context marked for delete");
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONTEXT));
                return EGL_FALSE;
            }
        }
    }

    // Section 2: Reset the current context and surface (if applicable) and set the new current context and surface (if applicable)
    if(drawVGSurfaceDesc == readVGSurfaceDesc){
        if(newVGContextDesc == NULL && drawVGSurfaceDesc == NULL && readVGSurfaceDesc == NULL){
            // Reset the current context's and surface's values
            if(currentVGContextDesc && (EGLenum)kdGetThreadStorageKHR(eglCurrentApiKey) == EGL_OPENVG_API){
                if(currentVGContextDesc->currentSurface){
                    // Check if the surface is using a client buffer
                    if(currentVGContextDesc->currentSurface->isClientBufferLocked){
                        if(currentVGContextDesc->currentSurface->clientBuffer && currentVGContextDesc->currentSurface->clientBufferContext){
                            // Sync the contents of the surface and the image
                            vgPrivImageUnlock(currentVGContextDesc->currentSurface->clientBufferContext,
                                currentVGContextDesc->currentSurface->clientBuffer,
                                currentVGContextDesc->currentSurface->vgSurface);
                            currentVGContextDesc->currentSurface->isClientBufferLocked = FALSE;
                        }
                    }
                    currentVGContextDesc->currentSurface->current = FALSE;
                    currentVGContextDesc->currentSurface = NULL;
                }
                currentVGContextDesc->current = FALSE;
                kdSetThreadStorageKHR(vgCurrentContextKey, NULL);

                // Make the current AmanithVG context and surface NULL
                // Should always return true
                vgMakeCurrent(NULL, NULL);
            }
        }else if(newVGContextDesc && drawVGSurfaceDesc && readVGSurfaceDesc){
            // Check if the surface is using a client buffer
            if(drawVGSurfaceDesc->clientBufferContext && drawVGSurfaceDesc->clientBuffer){
                // Check if EGL can use that client buffer
                if(vgPrivImageInUseByOpenVG(drawVGSurfaceDesc->clientBufferContext, drawVGSurfaceDesc->clientBuffer)){
                    WARN("Resource is in use");
                    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ACCESS));                    
                    return EGL_FALSE;
                }
            }
            // Reset the current context's and surface's values
            if(currentVGContextDesc){
                if(currentVGContextDesc->currentSurface){
                    if(currentVGContextDesc->currentSurface->isClientBufferLocked){
                        if(currentVGContextDesc->currentSurface->clientBuffer && currentVGContextDesc->currentSurface->clientBufferContext){
                            // Sync the contents of the surface and the image
                            vgPrivImageUnlock(currentVGContextDesc->currentSurface->clientBufferContext,
                                currentVGContextDesc->currentSurface->clientBuffer,
                                currentVGContextDesc->currentSurface->vgSurface);
                            currentVGContextDesc->currentSurface->isClientBufferLocked = FALSE;
                        }
                    }
                    currentVGContextDesc->currentSurface->current = FALSE;
                    currentVGContextDesc->currentSurface = NULL;
                }
                currentVGContextDesc->current = FALSE;
                kdSetThreadStorageKHR(vgCurrentContextKey, NULL);
            }

            // Make the new context and surface current
            if(newVGContextDesc->vgContext){
                if(drawVGSurfaceDesc->vgSurface){
                    // Check if we're using a client buffer surface
                    if(drawVGSurfaceDesc->clientBuffer && drawVGSurfaceDesc->clientBufferContext){
                        // Sync the contents of the surface and the image
                        vgPrivImageLock(drawVGSurfaceDesc->clientBufferContext,
                            drawVGSurfaceDesc->clientBuffer,
                            drawVGSurfaceDesc->vgSurface);
                        drawVGSurfaceDesc->isClientBufferLocked = TRUE;
                    }
                    // Should always return true
                    vgMakeCurrent(newVGContextDesc->vgContext, drawVGSurfaceDesc->vgSurface);
                    // Update the current context with new current information

                    kdSetThreadStorageKHR(vgCurrentContextKey, newVGContextDesc);
                    currentVGContextDesc = newVGContextDesc;
                    currentVGContextDesc->current = TRUE;
                    currentVGContextDesc->currentSurface = drawVGSurfaceDesc;
                    currentVGContextDesc->currentSurface->current = TRUE;
                }else{
                    WARN("Invalid EGL Surface");
                    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
                    return EGL_FALSE;
                }
            }else{
                WARN("Invalid EGL Context");
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONTEXT));
                return EGL_FALSE;
            }
        }else{
            WARN("Draw and read are not compatible with ctx");
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
            return EGL_FALSE;
        }
    }else{
        WARN("Invalid draw and read surfaces");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
        return EGL_FALSE;
    } 
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLDisplay eglGetCurrentDisplayAmanith( void )
{
    kdLogMessage("eglGetCurrentDisplayAmanith"); 
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    return (EGLDisplay)&EglDpyContext[0];
}

#if defined( RIM_NDK ) && !defined( RIM_WINDOW_MANAGER )
EGLBoolean eglSwapBuffersAmanith( EGLDisplay dpy, EGLSurface surface )
{
    kdLogMessage("eglSwapBuffersAmanith() is not implemented."); 
    return EGL_FALSE;
}
#else

EGLBoolean eglSwapBuffersAmanith( EGLDisplay dpy, EGLSurface surface )
{
    EGLDisplayContext *dpyContext = NULL;
    EGLSurfaceDescriptor * surfaceDesc = (EGLSurfaceDescriptor *)surface;
    VGSurfaceDescriptor *vgSurfaceDesc = NULL;
 
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    kdLogMessage("eglSwapBuffersAmanith");

    // Get the EGLDisplayContext, we already checked if it's valid
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglSwapBuffersAmanith : bad display" ); 
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    if(!surfaceDesc){
        WARN("Invalid EGL surface"); 
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    if(surfaceDesc->vendorSurface){
        vgSurfaceDesc = (VGSurfaceDescriptor*)(surfaceDesc->vendorSurface);
    }else{
        WARN("Invalid EGL surface"); 
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    // Only swap the surface if it is a window surface
    if (surfaceDesc->surfaceType == EGL_WINDOW_BIT){
#ifdef RIM_WINDOW_MANAGER
        WMError_t       result;
        DWORD           frontBitDepth;
        DWORD           frontPixelStride;
        SDWORD          temp[4];
        Rect            dirtyRect;
        DWORD          *backBuffer;
        DWORD          *frontBuffer;
        VGImageFormat   surfaceFormat;

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
            WARN("eglSwapBuffersAmanith: bad native window"); 
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_NATIVE_WINDOW);
            return EGL_FALSE;
        }
        dirtyRect.x = temp[0];
        dirtyRect.y = temp[1];
        dirtyRect.width = temp[2];
        dirtyRect.height = temp[3];

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
            WARN("eglSwapBuffersAmanith: bad native window"); 
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_NATIVE_WINDOW);
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
        DWORD          *contextBuff;
        WORD           *frontBuff;
        WORD           *frontBuffTemp;
        VGImageFormat   surfaceFormat;
        DWORD           frontBuffStride;
        DWORD           frontBuffFormat;
        LcdConfig       *lcdCfg = LcdGetLcdConfigPtr( dpyContext->id );
        DWORD lcdXPos = surfaceDesc->nativeWindow->windowSize.x + surfaceDesc->nativeWindow->clipRect.x;
        DWORD lcdYPos = lcdCfg->height - surfaceDesc->nativeWindow->windowSize.y - surfaceDesc->nativeWindow->clipRect.y
                - surfaceDesc->nativeWindow->clipRect.height;
        DWORD windowXPos = surfaceDesc->nativeWindow->clipRect.x;
        DWORD windowYPos = surfaceDesc->nativeWindow->windowSize.height - surfaceDesc->nativeWindow->clipRect.y - surfaceDesc->nativeWindow->clipRect.height;

        contextBuff = (DWORD*)vgGetSurface(vgSurfaceDesc->vgSurface, &surfaceFormat);
        if(!contextBuff){
            WARN("Could not get the surface"); 
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
            return EGL_FALSE;
        }

        // Adjust the surface buffer to the correct y position
        contextBuff += windowYPos * surfaceDesc->nativeWindow->windowSize.width;

        LcdGetBuffer( dpyContext->id, LCD_FRONT_BUFFER, &frontBuff, &frontBuffFormat, &frontBuffStride );
        frontBuffTemp = frontBuff;

        // Adjust the back buffer to the correct y position
        frontBuffTemp += lcdYPos * lcdCfg->width;
        RasterARGB8888ToRGB565Copy( contextBuff, surfaceDesc->nativeWindow->windowSize.width, windowXPos,
                            frontBuffTemp, lcdCfg->width, lcdXPos,
                            surfaceDesc->nativeWindow->clipRect.width, surfaceDesc->nativeWindow->clipRect.height );
        

        LV1(LOG_EGL, PRINT4N( "Swap buffers updating LCD %d %d %d %d", lcdXPos, lcdYPos, 
            surfaceDesc->nativeWindow->clipRect.width,
            surfaceDesc->nativeWindow->clipRect.height));


        LcdUpdateInt(dpyContext->id, lcdXPos, lcdYPos, surfaceDesc->nativeWindow->clipRect.width,
            surfaceDesc->nativeWindow->clipRect.height);
#endif // RIM_WINDOW_MANAGER
    }
    return EGL_TRUE;
}
#endif // #if !defined(RIM_NDK) && !defined( RIM_WINDOW_MANAGER )


/**
 * Accepts Amanith Vendor Surfaces Only
 */
EGLBoolean eglCopyBuffersAmanithInt(EGLDisplay dpy, EGLSurface surface, NativePixmapType target)
{
    void *contextBuff = NULL;
    // We already checked if a vendorSurface exists in eglCopyBuffers()
    VGSurfaceDescriptor *vgSurfaceDesc = (VGSurfaceDescriptor*)surface;

    kdLogMessage("eglCopyBuffersAmanithInt");
    PRINT("eglCopyBuffersAmanithInt");

    // NOTE: We're assuming that only 16 bit bitmaps are used
    if(vgSurfaceDesc->surfaceType == EGL_WINDOW_BIT || vgSurfaceDesc->surfaceType == EGL_PBUFFER_BIT ||
        vgSurfaceDesc->surfaceType == EGL_PIXMAP_BIT ){ 
        
        contextBuff = vgGetSurface(vgSurfaceDesc->vgSurface, NULL);
        if(!contextBuff){
            WARN("Could not get the surface");
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
            return EGL_FALSE;
        }
        // Amanith surface is ARGB8888
        switch(target->bType) {
            case BMT_16BPP_RGB565:
                RasterARGB8888ToRGB565Copy( (DWORD *)contextBuff, vgSurfaceDesc->width, 0,
                    (WORD*)target->data, target->stride/sizeof(WORD), 0,
                    (target->wide > vgSurfaceDesc->width) ? vgSurfaceDesc->width : target->wide,
                    (target->high > vgSurfaceDesc->height) ? vgSurfaceDesc->height : target->high );                
                break;
            case BMT_32BPP_XRGB8888:
            case BMT_32BPP_ARGB8888:                        
            case BMT_32BPP_ARGB8888_PMA:
                RasterCopyC32( (DWORD*)contextBuff, vgSurfaceDesc->width, 0,
                    (DWORD*)target->data, target->stride/sizeof(DWORD),0,
                    (target->wide > vgSurfaceDesc->width) ? vgSurfaceDesc->width : target->wide,
                    (target->high > vgSurfaceDesc->height) ? vgSurfaceDesc->height : target->high );
                break;
            default:
                 WARNN("eglCopyBuffersAmanith doesn't support target NativePixmapType %x", target->bType);
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_MATCH);
                return EGL_FALSE;

        }
         
    }else{
        WARN("eglCopyBuffersAmanith does not support the surface type");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_SURFACE);
        return EGL_FALSE;
    }
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    return EGL_TRUE;
}

// NOTE: Takes in RIM EGL parameters
EGLBoolean eglCopyBuffersAmanith(EGLDisplay dpy, EGLSurface surface, NativePixmapType target)
{
    EGLSurfaceDescriptor *surfaceDesc = (EGLSurfaceDescriptor *)surface;
    // We already checked if a vendorSurface exists in eglCopyBuffers()
    VGSurfaceDescriptor *vgSurfaceDesc = (VGSurfaceDescriptor*)(surfaceDesc->vendorSurface);

    kdLogMessage("eglCopyBuffersAmanith");

    return eglCopyBuffersAmanithInt(dpy, vgSurfaceDesc, target);
}

const char *eglQueryStringAmanith( EGLDisplay dpy, EGLint name )
{
    EGLDisplayContext *dpyContext = NULL;

    kdLogMessage("eglQueryStringAmanith");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    // Get the EGLDisplayContext, we already checked if it's valid
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglQueryStringAmanith : bad display" ); 
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    switch( name ) {
    case EGL_CLIENT_APIS:
        kdStrcpy_s(dpyContext->strQueried, EGL_STR_QUERIED_SZ_BYTES, "OpenVG");
        break;
    case EGL_EXTENSIONS:
        kdStrcpy_s(dpyContext->strQueried, EGL_STR_QUERIED_SZ_BYTES,
                                      "eglLockSurfaceKHR eglUnlockSurfaceKHR");
        break;
    case EGL_VENDOR:
        kdStrcpy_s(dpyContext->strQueried, EGL_STR_QUERIED_SZ_BYTES, "RIM");
        break;
    case EGL_VERSION:
        kdStrcpy_s(dpyContext->strQueried, EGL_STR_QUERIED_SZ_BYTES, "1.2");
        break;
    default: 
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_PARAMETER));
        return NULL;
    }
    return dpyContext->strQueried;
}

EGLBoolean eglLockSurfaceKHRAmanith(EGLDisplay display, EGLSurface surface, const EGLint *attrib_list)
{
    VGSurfaceDescriptor *vgSurfaceDesc = (VGSurfaceDescriptor*)surface;

    kdLogMessage("eglLockSurfaceKHRAmanith");

    if(!vgSurfaceDesc){ 
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    if(vgSurfaceDesc->locked || vgSurfaceDesc->current){ 
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ACCESS));
        return EGL_FALSE;
    }

    vgSurfaceDesc->locked = TRUE; 
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean eglUnlockSurfaceKHRAmanith(EGLDisplay display, EGLSurface surface)
{
    VGSurfaceDescriptor *vgSurfaceDesc = (VGSurfaceDescriptor*)surface;

    kdLogMessage("eglUnlockSurfaceKHRAmanith");

    if(!vgSurfaceDesc){
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    if(!vgSurfaceDesc->locked){ 
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ACCESS));
        return EGL_FALSE;
    }

    vgSurfaceDesc->locked = FALSE; 
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean eglBindTexImageAmanith(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    kdLogMessage("eglBindTexImageAmanith");

    // Return EGL_BAD_SURFACE, since Amanith does not implement OpenGL ES 
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
    return EGL_FALSE;
}

EGLBoolean eglReleaseTexImageAmanith(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    kdLogMessage("eglReleaseTexImageAmanith");

    // Return EGL_BAD_SURFACE, since Amanith does not implement OpenGL ES 
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
    return EGL_FALSE;
}

EGLBoolean eglSwapIntervalAmanith(EGLDisplay dpy, EGLint interval)
{
    kdLogMessage("eglSwapIntervalAmanith"); 
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));       
    return EGL_TRUE;
}

EGLBoolean eglReleaseThreadAmanith(void)
{
    // Needs to be implemented
    kdLogMessage("eglReleaseThreadAmanith"); 
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));       
    return EGL_TRUE;
}

EGLBoolean eglWaitClientAmanith( void )
{
    // Nothing to do
    kdLogMessage("eglWaitClientAmanith"); 
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));       
    return EGL_TRUE;
}

EGLBoolean eglWaitGLAmanith(void)
{
    // Nothing to do
    kdLogMessage("eglWaitGLAmanith"); 
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));       
    return EGL_TRUE;
}

EGLBoolean eglWaitNativeAmanith(EGLint engine)
{
    // Nothing to do
    kdLogMessage("eglWaitNativeAmanith"); 
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));       
    return EGL_TRUE;
}
