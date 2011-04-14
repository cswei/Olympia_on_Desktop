/*******************************************************
 *  egl_amanith_context.c
 *
 *  Copyright 2009, Research In Motion Ltd.
 *
 *  Contains the specific interface between EGL and AmanithVG
 *  Designed to conform with Khronos EGL 1.2 standard
 *
 *  Philip Chrapka, April, 2009
 *
 *******************************************************/

#if defined (RIM_NDK)
    #include <egl_user_types.h>
#else
    #include "bugdispc.h"

    #define SRCFILE     FILE_EGL_AMANITH_CONTEXT
    #define SRCGROUP    GROUP_GRAPHICS
#endif

#include "egl_amanith.h"
#include "egl_amanith_globals.h"
#include <KHR_thread_storage.h>

EGLContext eglCreateContextAmanith( EGLDisplay dpy, EGLConfig config,
                                   EGLContext share_context,
                                   const EGLint *attrib_list )
{
    VGContextDescriptor *newVGContext = NULL;
    EGLConfigContextPtr currentConfigContext = NULL;

    kdLogMessage("eglCreateContextAmanith");


    if(config){
        currentConfigContext = ((EGLConfigDescriptor*)config)->ourConfig;
    }else{ 
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONFIG));        
        return EGL_NO_SURFACE;
    }

    // If the current api is EGL_OPENVG_API
    if((EGLenum)kdGetThreadStorageKHR(eglCurrentApiKey) == EGL_OPENVG_API){
        //If the context is a VG context create and initialize an AmanithVG context
        if(currentConfigContext[CONFIG_ELEMENT(EGL_RENDERABLE_TYPE)] & EGL_OPENVG_BIT){
            // Allocate a VGContextDescriptor
            newVGContext = (VGContextDescriptor *)kdMalloc(sizeof(VGContextDescriptor));
            if(!newVGContext){ 
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));                                
                return EGL_NO_SURFACE;
            }
            newVGContext->configID = currentConfigContext[CONFIG_ELEMENT(EGL_CONFIG_ID)];
            newVGContext->current = FALSE;
            newVGContext->markForDelete = FALSE;
            newVGContext->currentSurface = NULL;
            newVGContext->shared = FALSE;

            if ( share_context ) {
                PRINT2N("Shared with context 0x%x 0x%x", share_context, ((VGContextDescriptor *)share_context)->vgContext);
                newVGContext->vgContext = ((VGContextDescriptor *)share_context)->vgContext;
                newVGContext->shared = TRUE;
            }
            else {
                // Create a new OpenVG context
                newVGContext->vgContext = vgCreateContext();
                PRINTN("Created a new vg context 0x%x", newVGContext->vgContext);
                
            }
            if(!newVGContext->vgContext){
                WARN("vgCreateContext : failed to allocate");
                vgDestroyContext(newVGContext->vgContext);
                kdFree(newVGContext); 
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));
                
                return EGL_NO_CONTEXT;
            }
        }else{ 
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONFIG));
            
            return EGL_NO_CONTEXT;
        }
    }else{
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));        
        return EGL_NO_CONTEXT;
    } 
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    return (EGLContext)newVGContext;
}

EGLBoolean eglDestroyContextAmanith( EGLDisplay dpy, EGLContext ctx )
{
    VGContextDescriptor *vgContextDesc = (VGContextDescriptor *)ctx;

    PRINTN( "eglDestroyContextAmanith: 0x%08x", ctx );
    kdLogMessage("eglDestroyContextAmanith");

    if(vgContextDesc){
        // If the context is current, it will be destroyed on next valid call to eglMakeCurrent
        if(vgContextDesc->current){
            vgContextDesc->markForDelete = TRUE; 
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
            return EGL_TRUE;
        }

        // Destroy the VGContext
        if(vgContextDesc->vgContext){
            if ( vgContextDesc->shared) {
                //DO NOT DESTROY SHARED CONTEXT as original context may still be active
                //INSTEAD MAKE IT NULL
                vgContextDesc->shared = FALSE;
                vgContextDesc->vgContext = NULL;

                }
            else if(vgDestroyContext(vgContextDesc->vgContext) != EGL_TRUE){
                WARNN("Amanith Context 0x%08x was not destroyed",(DWORD)vgContextDesc->vgContext); 
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONTEXT));
                return EGL_FALSE;
            }
        }

        // Destroy the VGContextDescriptor
        kdFree(vgContextDesc);
    }else{ 
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONTEXT));
        return EGL_FALSE;
    } 
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLContext eglGetCurrentContextAmanith( void )
{
    kdLogMessage("eglGetCurrentContextAmanith"); 
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    if((EGLenum)kdGetThreadStorageKHR(eglCurrentApiKey) == EGL_OPENVG_API){
        VGContextDescriptor *currentVGContextDesc = NULL;

        currentVGContextDesc = kdGetThreadStorageKHR(vgCurrentContextKey);
        if(currentVGContextDesc){
            return (EGLContext)currentVGContextDesc;
        }
    }
    return EGL_NO_CONTEXT;
}

EGLBoolean eglQueryContextAmanith( EGLDisplay dpy, EGLContext ctx,
                                    EGLint attribute, EGLint *value )
{
    VGContextDescriptor *vgContextDesc = (VGContextDescriptor *)ctx;
    kdLogMessage("eglQueryContextAmanith");
    if(vgContextDesc){ 
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
        switch(attribute){
        case EGL_CONFIG_ID:
            *value = (EGLint)(vgContextDesc->configID);
            return EGL_TRUE;
        case EGL_CONTEXT_CLIENT_TYPE:
            *value = EGL_OPENVG_API;
            return EGL_TRUE;
        case EGL_RENDER_BUFFER:
            if (vgContextDesc->current){
                if(vgContextDesc->currentSurface){
                    *value = vgContextDesc->currentSurface->rBuffer;
                }
            }else{
                *value = EGL_NONE;
            }
            return EGL_TRUE;
        default: 
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ATTRIBUTE));
            return EGL_FALSE;
        }
    } 
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONTEXT));
    return EGL_FALSE;
}

