/*******************************************************
 *  egl_amanithqc_context.c
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

#define SRCFILE     FILE_EGL_AMQC_CONTEXT
#define SRCGROUP    GROUP_GRAPHICS

#include "egl_amanithqc.h"
#include "egl_amanithqc_globals.h"

EGLContext eglCreateContextAmanithQC( EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list )
{
    EGLVendorContextAmanithQC *newContext = NULL;
    EGLVendorConfigAmanithQC *vendorConfig = (EGLVendorConfigAmanithQC *)config;
    EGLContext qc_amanith_share_ctx;
    PRINT("eglCreateContextAmanithQC");
    PRINTN("config 0x%08x",config);
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_SUCCESS);

    if(vendorConfig){
        if(vendorConfig->vendorConfig){
            switch ((EGLenum)kdGetThreadStorageKHR(eglCurrentApiKey)){
                case EGL_OPENVG_API:
                    if(vendorConfig->vendorID == EGL_VENDOR_AMANITH){
                        PRINT("Creating Amanith context");
                        // Allocate memory for a EGLVendorContextAmanithQC
                        newContext = (EGLVendorContextAmanithQC *)kdMalloc(sizeof(EGLVendorContextAmanithQC));
                        if(!newContext){
                            kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_ALLOC);
                            return EGL_NO_CONTEXT;
                        }
                        newContext->vendorID = EGL_VENDOR_AMANITH;
                        // Create the Amanith Context
                        qc_amanith_share_ctx = (share_context != EGL_NO_CONTEXT) ? (((EGLVendorContextAmanithQC *)share_context)->vendorContext) : (share_context);
                        newContext->vendorContext = eglCreateContextAmanith( dpy, vendorConfig->vendorConfig, 
                            qc_amanith_share_ctx, attrib_list );
                        PRINTN("QC amanith vendor context is 0x%x", (DWORD)newContext->vendorContext);
                        kdSetThreadStorageKHR(eglLastErrorKey, (void *)eglGetErrorAmanith());
                        // Check if the returned context
                        if(newContext->vendorContext == EGL_NO_CONTEXT){
                            kdFree(newContext);
                            return EGL_NO_CONTEXT;
                        }
                        return (EGLContext)newContext;
                    }else{
                        kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_CONFIG);
                        break;
                    }
                case EGL_OPENGL_ES_API:
                    if(vendorConfig->vendorID == EGL_VENDOR_QC){
                        PRINT("Creating QC context");
                        // Allocate memory for a EGLVendorContextAmanithQC
                        newContext = (EGLVendorContextAmanithQC *)kdMalloc(sizeof(EGLVendorContextAmanithQC));
                        if(!newContext){
                            kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_ALLOC);
                            
                            return EGL_NO_CONTEXT;
                        }
                        newContext->vendorID = EGL_VENDOR_QC;
                        // Create the QC Context
                        newContext->vendorContext = qceglCreateContext( dpy, vendorConfig->vendorConfig, share_context, attrib_list );
                        kdSetThreadStorageKHR(eglLastErrorKey, (void *)qceglGetError());
                        
                        if(newContext->vendorContext == EGL_NO_CONTEXT){
                            kdFree(newContext);
                            return EGL_NO_CONTEXT;
                        }
                        return (EGLContext)newContext;
                    }else{
                        kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_CONFIG);
                        break;
                    }
                default:
                    WARNN("Unknown current API 0x%04x",(EGLenum)kdGetThreadStorageKHR(eglCurrentApiKey));
                    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_MATCH);
                    break;
            }
        }
    }

    return EGL_NO_CONTEXT;
}

EGLBoolean eglDestroyContextAmanithQC( EGLDisplay dpy, EGLContext ctx )
{
    EGLBoolean retBool = EGL_FALSE;
    EGLVendorContextAmanithQC *oldContext = (EGLVendorContextAmanithQC *)ctx;
    PRINT("eglDestroyContextAmanithQC");

    if(oldContext){
        if(oldContext->vendorContext != EGL_NO_CONTEXT){
            switch (oldContext->vendorID){
                case EGL_VENDOR_QC:
                    retBool = qceglDestroyContext(dpy, oldContext->vendorContext);
                    kdSetThreadStorageKHR(eglLastErrorKey, (void *)qceglGetError());
                    break;
                case EGL_VENDOR_AMANITH:
                    retBool = eglDestroyContextAmanith(dpy, oldContext->vendorContext);
                    kdSetThreadStorageKHR(eglLastErrorKey, (void *) eglGetErrorAmanith());
                    break;
                default:
                    WARNN("Unknown VendorID 0x%02x",oldContext->vendorID);
                    break;
            }
        }

        if(retBool == EGL_TRUE){
            oldContext->vendorID = EGL_VENDOR_NONE;
            oldContext->vendorContext = EGL_NO_CONTEXT;
            kdFree(oldContext);
            return retBool;
        }
    }

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_CONTEXT);
    return retBool;
}

EGLBoolean eglQueryContextAmanithQC( EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value )
{
    EGLBoolean retBool = EGL_FALSE;
    EGLVendorContextAmanithQC *vendorContext = (EGLVendorContextAmanithQC *)ctx;

    PRINT("eglQuerySurfaceAmanithQC");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_CONTEXT);
    if(vendorContext){
        switch (vendorContext->vendorID){
            case EGL_VENDOR_QC:
                retBool = qceglQueryContext( dpy, vendorContext->vendorContext, attribute, value );
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)qceglGetError());
                break;
            case EGL_VENDOR_AMANITH:
                retBool = eglQueryContextAmanith( dpy, vendorContext->vendorContext, attribute, value );
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)eglGetErrorAmanith());
                break;
            default:
                WARNN("Unknown VendorID 0x%02x",vendorContext->vendorID);
                break;
        }
    }

    return retBool;
}
