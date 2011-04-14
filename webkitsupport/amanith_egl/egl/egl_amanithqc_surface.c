/*******************************************************
 *  egl_amanithqc_surface.c
 *
 *  Copyright 2009, Research In Motion Ltd.
 *
 *  Contains the interface between AmanithVG, Qualcomm OpenGL ES and EGL
 *  Designed to conform with khronos EGL 1.2 standard
 *
 *  Philip Chrapka, April, 2009
 *
 *******************************************************/

#include "bugdispc.h"

#define SRCFILE     FILE_EGL_AMQC_SURFACE
#define SRCGROUP    GROUP_GRAPHICS

#include "egl_amanithqc.h"
#include "egl_amanithqc_globals.h"
#include "gl.h" // For glFinish() prototype.

EGLSurface eglCreatePbufferFromClientBufferAmanithQC( EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, 
                                                      const EGLint *attrib_list )
{
    EGLVendorSurfaceAmanithQC *vendorSurface = NULL;
    EGLVendorConfigAmanithQC *vendorConfig = (EGLVendorConfigAmanithQC *)config;
    PRINT("eglCreatePbufferFromClientBufferAmanithQC");

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_CONFIG);

    if(vendorConfig){
        switch (vendorConfig->vendorID){
            case EGL_VENDOR_QC:
                WARN("OpenGL ES rendering is not supported on client buffers");
                break;
            case EGL_VENDOR_AMANITH:
                {
                    vendorSurface = (EGLVendorSurfaceAmanithQC *)kdMalloc(sizeof(EGLVendorSurfaceAmanithQC));
                    if(!vendorSurface){
                        kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_ALLOC);
                        return EGL_NO_SURFACE;
                    }

                    vendorSurface->vendorID = EGL_VENDOR_AMANITH;
                    vendorSurface->parentSurface = NULL;
                    vendorSurface->vendorSurface = eglCreatePbufferFromClientBufferAmanith(dpy, buftype, buffer, vendorConfig->vendorConfig, attrib_list);
                    kdSetThreadStorageKHR(eglLastErrorKey, (void *)eglGetErrorAmanith());

                    if(vendorSurface->vendorSurface != EGL_NO_SURFACE){
                        // Return the new surface
                        return (EGLSurface)vendorSurface;
                    }else{
                        // Free the EGLVendorSurfaceAmanithQC if we end up here
                        kdFree(vendorSurface);
                    }
                }
                break;
            default:
                WARNN("Unknown VendorID 0x%02x",vendorConfig->vendorID);
                break;
        }
    }

    return EGL_NO_SURFACE;
}

EGLSurface eglCreatePbufferSurfaceAmanithQC( EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list )
{
    EGLVendorSurfaceAmanithQC *vendorSurface = NULL;
    EGLVendorConfigAmanithQC *vendorConfig = (EGLVendorConfigAmanithQC *)config;
    PRINT("eglCreatePbufferSurfaceAmanithQC");

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_SUCCESS);

    if(vendorConfig){
        vendorSurface = (EGLVendorSurfaceAmanithQC *)kdMalloc(sizeof(EGLVendorSurfaceAmanithQC));
        if(!vendorSurface){
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_ALLOC);
            return EGL_NO_SURFACE;
        }
        vendorSurface->vendorID = EGL_VENDOR_NONE;
        vendorSurface->parentSurface = NULL;
        vendorSurface->vendorSurface = EGL_NO_SURFACE;

        switch (vendorConfig->vendorID){
            case EGL_VENDOR_QC:
                // Create QC Surface Only
                vendorSurface->vendorID = EGL_VENDOR_QC;
                vendorSurface->vendorSurface = qceglCreatePbufferSurface(dpy, vendorConfig->vendorConfig, attrib_list);
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)qceglGetError());
                break;
            case EGL_VENDOR_AMANITH:
                // Create Amanith Surface Only
                vendorSurface->vendorID = EGL_VENDOR_AMANITH;
                vendorSurface->vendorSurface = eglCreatePbufferSurfaceAmanith(dpy, vendorConfig->vendorConfig, attrib_list);
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)eglGetErrorAmanith());
                break;
            default:
                WARNN("Unknown VendorID 0x%02",vendorConfig->vendorID);
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_CONFIG);
                break;
        }

        if(vendorSurface->vendorSurface != EGL_NO_SURFACE){
            // Return the new surface
            return (EGLSurface)vendorSurface;
        }else{
            // Free the EGLVendorSurfaceAmanithQC if we end up here
            kdFree(vendorSurface);
            return EGL_NO_SURFACE;
        }
    }
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_CONFIG);
    return EGL_NO_SURFACE;
}

EGLSurface eglCreatePixmapSurfaceAmanithQC( EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list )
{
    EGLVendorSurfaceAmanithQC *vendorSurface = NULL;
    DWORD config_id = (DWORD)config;
    EGLDisplayContext *dpyContext = NULL;
    EGLVendorConfigAmanithQC *vendorConfig = (EGLVendorConfigAmanithQC*)config; 
    PRINT("eglCreatePixmapSurfaceAmanithQC");

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_SUCCESS);

    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglCreatePixmapSurface : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_NO_SURFACE;
    }
        
    if(vendorConfig){
        vendorSurface = (EGLVendorSurfaceAmanithQC *)kdMalloc(sizeof(EGLVendorSurfaceAmanithQC));
        if(!vendorSurface){
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_ALLOC);
            return EGL_NO_SURFACE;
        }
        vendorSurface->vendorID = EGL_VENDOR_NONE;
        vendorSurface->parentSurface = NULL;
        vendorSurface->vendorSurface = EGL_NO_SURFACE;

        switch (vendorConfig->vendorID){
            case EGL_VENDOR_QC:
                // Create QC Surface Only
                //vendorSurface->vendorID = EGL_VENDOR_QC;
                //vendorSurface->vendorSurface = qceglCreatePbufferSurface(dpy, vendorConfig->vendorConfig, attrib_list);
                //kdSetThreadStorageKHR(eglLastErrorKey, (void *)qceglGetError());
                break;
            case EGL_VENDOR_AMANITH:
                // Create Amanith Surface Only
                vendorSurface->vendorID = EGL_VENDOR_AMANITH;
                vendorSurface->vendorSurface = eglCreatePixmapSurfaceAmanith(dpy, vendorConfig->vendorConfig, pixmap, attrib_list);
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)eglGetErrorAmanith());
                break;
            default:
                WARNN("Unknown VendorID 0x%X",vendorConfig->vendorID);
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_CONFIG);
                break;
        }

        if(vendorSurface->vendorSurface != EGL_NO_SURFACE){
            // Return the new surface
            return (EGLSurface)vendorSurface;
        }else{
            // Free the EGLVendorSurfaceAmanithQC if we end up here
            kdFree(vendorSurface);
            return EGL_NO_SURFACE;
        }
    }
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_CONFIG);
    return EGL_NO_SURFACE;
}


EGLSurface eglCreateWindowSurfaceAmanithQC( EGLDisplay dpy, EGLConfig config, NativeWindowType win, const EGLint *attrib_list )
{
    EGLVendorSurfaceAmanithQC *vendorSurface = NULL;
    EGLVendorConfigAmanithQC *vendorConfig = (EGLVendorConfigAmanithQC *)config;
    PRINT("eglCreateWindowSurfaceAmanithQC");

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_SUCCESS);

    if(vendorConfig){
        vendorSurface = (EGLVendorSurfaceAmanithQC *)kdMalloc(sizeof(EGLVendorSurfaceAmanithQC));
        if(!vendorSurface){
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_ALLOC);
            
            return EGL_NO_SURFACE;
        }
        vendorSurface->vendorID = EGL_VENDOR_NONE;
        vendorSurface->parentSurface = NULL;
        vendorSurface->vendorSurface = EGL_NO_SURFACE;

        switch (vendorConfig->vendorID){
            case EGL_VENDOR_QC:
                // Create QC Surface Only
                vendorSurface->vendorID = EGL_VENDOR_QC;
                vendorSurface->vendorSurface = qceglCreateWindowSurface(dpy, vendorConfig->vendorConfig, win, attrib_list);
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)qceglGetError());                
                break;
            case EGL_VENDOR_AMANITH:
                // Create Amanith Surface Only
                vendorSurface->vendorID = EGL_VENDOR_AMANITH;
                vendorSurface->vendorSurface = eglCreateWindowSurfaceAmanith(dpy, vendorConfig->vendorConfig, win, attrib_list);
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)eglGetErrorAmanith());                
                break;
            default:
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_CONFIG);                
                WARNN("Unknown VendorID 0x%02",vendorConfig->vendorID);
                break;
        }

        if(vendorSurface->vendorSurface != EGL_NO_SURFACE){
            // Return the new surface
            return (EGLSurface)vendorSurface;
        }else{
            // Free the EGLVendorSurfaceAmanithQC if we end up here
            kdFree(vendorSurface);
            return EGL_NO_SURFACE;
        }
    }
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_CONFIG);
    
    return EGL_NO_SURFACE;
}

EGLBoolean eglDestroySurfaceAmanithQC( EGLDisplay dpy, EGLSurface surface )
{
    EGLBoolean retBool = EGL_FALSE;
    EGLVendorSurfaceAmanithQC *vendorSurface = (EGLVendorSurfaceAmanithQC *)surface;
    PRINT("eglDestroySurfaceAmanithQC");

    if(vendorSurface){
        if(vendorSurface->vendorSurface != EGL_NO_SURFACE){
            switch (vendorSurface->vendorID){
                case EGL_VENDOR_QC:
                    retBool = qceglDestroySurface(dpy, vendorSurface->vendorSurface);
                    kdSetThreadStorageKHR(eglLastErrorKey, (void *)qceglGetError());
                    break;
                case EGL_VENDOR_AMANITH:
                    retBool = eglDestroySurfaceAmanith(dpy, vendorSurface->vendorSurface);
                    kdSetThreadStorageKHR(eglLastErrorKey, (void *)eglGetErrorAmanith());
                    break;
                default:
                    WARNN("Unknown VendorID 0x%02",vendorSurface->vendorID);
                    break;
            }

            // If everything went well destroy the vendor surface struct
            if(retBool == EGL_TRUE){
                vendorSurface->vendorID = EGL_VENDOR_NONE;
                vendorSurface->parentSurface = NULL;
                vendorSurface->vendorSurface = EGL_NO_SURFACE;
                kdFree(vendorSurface);
                return retBool;
            }
        }
    }
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_SURFACE);
    
    return EGL_FALSE;
}

EGLBoolean eglQuerySurfaceAmanithQC( EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value )
{
    EGLBoolean retBool = EGL_TRUE;
    EGLVendorSurfaceAmanithQC *vendorSurface = (EGLVendorSurfaceAmanithQC *)surface;

    PRINT("eglQuerySurfaceAmanithQC");
    //lLastError = EGL_SUCCESS;
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_SUCCESS);

    //check to make sure that vendor surface has a parent
    if ( ! vendorSurface->parentSurface ) {
        WARN ("Surface does not have valid parent");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_SURFACE);
        return EGL_FALSE;   
    }
    
    if(vendorSurface){
        switch (vendorSurface->vendorID){ 
            case EGL_VENDOR_QC:
                //lock and unlock are not supported on qualcomm platforms currently
                //so we implement it ourselves
                switch(attribute) {
                    case EGL_BITMAP_POINTER_KHR:
                        glFinish();
                        *value = qceglGetColorBuffer(dpy, vendorSurface->vendorSurface);
                        PRINTN("Returning pointer to colour buffer 0x%x", *value);
                        break;
                    case EGL_BITMAP_PITCH_KHR:
                        // Number of bytes between successive rows
                        *value = vendorSurface->parentSurface->width * 
                                 vendorSurface->parentSurface->bitDepth >> 3;
                        break;
                    case EGL_BITMAP_ORIGIN_KHR:
                        //where the first pixel resides
                        *value = EGL_UPPER_LEFT_KHR;
                        break;
                    case EGL_BITMAP_PIXEL_RED_OFFSET_KHR:
                        if ( vendorSurface->parentSurface->bitDepth == 32) {
                            *value = 16;      
                        }
                        else {
                            *value = 11;
                        }
                    break;
                    case EGL_BITMAP_PIXEL_GREEN_OFFSET_KHR:
                        if ( vendorSurface->parentSurface->bitDepth == 32) {
                            *value = 8;      
                        }
                        else {
                            *value = 5;
                        }
                    break;
                    case EGL_BITMAP_PIXEL_BLUE_OFFSET_KHR:
                        *value = 0;                        
                        break;
                    case EGL_BITMAP_PIXEL_ALPHA_OFFSET_KHR:
                        if ( vendorSurface->parentSurface->bitDepth == 32) {
                            *value = 24;      
                        }
                        else {
                            *value = 0;
                        }                       
                        break;
                   case EGL_ALPHA_FORMAT:
                        *value = vendorSurface->parentSurface->alphaFormat;
                        break;
                   default:
                       retBool = qceglQuerySurface( dpy, vendorSurface->vendorSurface, attribute, value );
                       kdSetThreadStorageKHR(eglLastErrorKey, (void *)qceglGetError());
                       break;
                }
                break;
            case EGL_VENDOR_AMANITH:
                retBool = eglQuerySurfaceAmanith( dpy, vendorSurface->vendorSurface, attribute, value );
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)eglGetErrorAmanith());
                break;
            default:
                WARNN("Unknown VendorID 0x%02x",vendorSurface->vendorID);
                break;
        }
    }

    return retBool;
}

EGLBoolean eglSurfaceAttribAmanithQC( EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value )
{
    EGLBoolean retBool = EGL_FALSE;
    EGLVendorSurfaceAmanithQC *vendorSurface = (EGLVendorSurfaceAmanithQC *)surface;

    PRINT("eglSurfaceAttribAmanithQC");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_SURFACE);
    if(vendorSurface){
        switch (vendorSurface->vendorID){
            case EGL_VENDOR_QC:
                WARN("Function not implemented");
                //retBool = qceglSurfaceAttrib( dpy, vendorSurface->vendorSurface, attribute, value );
                //SetThreadStorageKHR(eglLastErrorKey, (void *)qceglGetError());
                break;
            case EGL_VENDOR_AMANITH:
                retBool = eglSurfaceAttribAmanith( dpy, vendorSurface->vendorSurface, attribute, value );
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)eglGetErrorAmanith());
                break;
            default:
                WARNN("Unknown VendorID 0x%02x",vendorSurface->vendorID);
                break;
        }
    }

    return retBool;
}

EGLBoolean eglLockSurfaceKHRAmanithQC( EGLDisplay display, EGLSurface surface, const EGLint *attrib_list )
{
    EGLBoolean retBool = EGL_FALSE;
    EGLVendorSurfaceAmanithQC *vendorSurface = (EGLVendorSurfaceAmanithQC *)surface;

    PRINT("eglLockSurfaceKHRAmanithQC"); 
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_SURFACE);

    if(vendorSurface){
        switch (vendorSurface->vendorID){
            case EGL_VENDOR_QC:
                // Nothing to do
                retBool = EGL_TRUE; 
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_SUCCESS);
                break;
            case EGL_VENDOR_AMANITH:
                retBool = eglLockSurfaceKHRAmanith( display, vendorSurface->vendorSurface, attrib_list );
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)eglGetErrorAmanith());
                break;
            default:
                WARNN("Unknown VendorID 0x%02x",vendorSurface->vendorID);
                break;
        }
    }

    return retBool;
}

EGLBoolean eglUnlockSurfaceKHRAmanithQC( EGLDisplay display, EGLSurface surface )
{
    EGLBoolean retBool = EGL_FALSE;
    EGLVendorSurfaceAmanithQC *vendorSurface = (EGLVendorSurfaceAmanithQC *)surface;

    PRINT("eglUnlockSurfaceKHRAmanithQC");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_SURFACE);

    if(vendorSurface){
        switch (vendorSurface->vendorID){
            case EGL_VENDOR_QC:
                // Nothing to do
                retBool = EGL_TRUE;
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_SUCCESS);
                break;
            case EGL_VENDOR_AMANITH:
                retBool = eglUnlockSurfaceKHRAmanith( display, vendorSurface->vendorSurface );
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)eglGetErrorAmanith());
                break;
            default:
                WARNN("Unknown VendorID 0x%02x",vendorSurface->vendorID);
                break;
        }
    }
    return retBool;
}
