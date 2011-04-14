/*******************************************************
 *  egl_vendor_ex.c
 *
 *  Copyright 2010, Research In Motion Ltd.
 *
 *  Bashar Taha: October 15, 2010
 *
 *  New EGL redirection layer to deal w/ different
 *  client APIs provided by different vendors in a 
 *  generic manner.
 *
 *******************************************************/
#include "egl_vendor_ex.h"

#if defined(RIM_NDK)
    #include <egl_user_types.h>
#else // !RIM_NDK
    #include "bugdispc.h"

    #define SRCFILE     FILE_EGL_VNDREX
    #define SRCGROUP    GROUP_GRAPHICS
#endif // RIM_NDK

#if defined(RIM_EGL_VENDOR_EX_ENABLED)

#include <string.h>
#include <config_egl.h>
#include <kd.h>
#include <kd_usermode.h>
#include <KHR_thread_storage.h>

EGLVndrExFuncList EglVndrExFuncs[ EGL_NUM_VENDOREX_APIS ];  // 0-th represents root function which may call GLES and/or VG functions

// TODO(BT) - debug only...
char EglVndrExFuncNames[ EGL_NUM_VENDOREX_APIS ][ EGL_VENDOREX_NAME_LENGTH ];

typedef void (*VOID_FUNC_PTR)();

EGLVndrExDisplay EglVndrExDisplay[ RIM_LCD_NUM_DISPLAYS ];


void EglInitVndrEx() {

    int i;
    int j;

    PRINT( "EglInitVndrEx" );

    for ( i = 0; i < EGL_NUM_VENDOREX_APIS; ++i ) {
        EGLVndrExFuncList * list = &EglVndrExFuncs[ i ];
    
        for ( j = 0; j < EGL_NUM_VENDOREX_FUNCS; ++j ) {
            // generic nullify of all func ptrs
            VOID_FUNC_PTR * funcPtr = (VOID_FUNC_PTR *)list;
            funcPtr += j;
            *funcPtr = (VOID_FUNC_PTR)NULL;
        }

        memset( &EglVndrExFuncNames[i][0], '\0', EGL_VENDOREX_NAME_LENGTH );
    }
}

EGLBoolean EglRegisterVndrExRootFunc( const EGLint funcId, void * func ) {
    EGLBoolean ret = EGL_FALSE;

    if ( EglIsValidVndrExFuncId(funcId) ) {
        PRINT2N( "EglRegisterVndrExRootFunc: ROOT:%(_eglVndrExFuncId) @ %p", funcId, (DWORD)func );

        ret = EGL_TRUE;

        switch (funcId) {
            case EGL_VENDOREX_GET_ERROR:                eglGetErrorVndr = (EGL_GET_ERROR_FUNCPTR)func; break; 
            case EGL_VENDOREX_GET_DISPLAY:              eglGetDisplayVndr = (EGL_GET_DISPLAY_FUNCPTR)func; break;
            case EGL_VENDOREX_INITIALIZE:               eglInitializeVndr = (EGL_INITIALIZE_FUNCPTR)func; break;
            case EGL_VENDOREX_TERMINATE:                eglTerminateVndr = (EGL_TERMINATE_FUNCPTR)func; break;
            case EGL_VENDOREX_QUERY_STRING:             eglQueryStringVndr = (EGL_QUERY_STRING_FUNCPTR)func; break;
            case EGL_VENDOREX_GET_CONFIGS:              eglGetConfigsVndr = (EGL_GET_CONFIGS_FUNCPTR)func; break;
            case EGL_VENDOREX_CHOOSE_CONFIG:            eglChooseConfigVndr = (EGL_CHOOSE_CONFIG_FUNCPTR)func; break;
            case EGL_VENDOREX_GET_CONFIG_ATTRIB:        eglGetConfigAttribVndr = (EGL_GET_CONFIG_ATTRIB_FUNCPTR)func; break;
            case EGL_VENDOREX_CREATE_WINDOW_SURFACE:    eglCreateWindowSurfaceVndr = (EGL_CREATE_WINDOW_SURFACE_FUNCPTR)func; break;
            case EGL_VENDOREX_CREATE_PBUFFER_SURFACE:   eglCreatePbufferSurfaceVndr = (EGL_CREATE_PBUFFER_SURFACE_FUNCPTR)func; break;
            case EGL_VENDOREX_CREATE_PIXMAP_SURFACE:    eglCreatePixmapSurfaceVndr = (EGL_CREATE_PIXMAP_SURFACE_FUNCPTR)func; break;
            case EGL_VENDOREX_DESTROY_SURFACE:          eglDestroySurfaceVndr = (EGL_DESTROY_SURFACE_FUNCPTR)func; break;
            case EGL_VENDOREX_QUERY_SURFACE:            eglQuerySurfaceVndr = (EGL_QUERY_SURFACE_FUNCPTR)func; break;
            case EGL_VENDOREX_BIND_API:                 eglBindAPIVndr = (EGL_BIND_API_FUNCPTR)func; break;
            case EGL_VENDOREX_QUERY_API:                eglQueryAPIVndr = (EGL_QUERY_API_FUNCPTR)func; break;
            case EGL_VENDOREX_WAIT_CLIENT:              eglWaitClientVndr = (EGL_WAIT_CLIENT_FUNCPTR)func; break;
            case EGL_VENDOREX_RELEASE_THREAD:           eglReleaseThreadVndr = (EGL_RELEASE_THREAD_FUNCPTR)func; break;
            case EGL_VENDOREX_CREATE_PBUFFER_FROM_CLIENT_BUFFER:   eglCreatePbufferFromClientBufferVndr = (EGL_CREATE_PBUFFER_FROMCLIENTBUFFER_FUNCPTR)func; break;
            case EGL_VENDOREX_SURFACE_ATTRIB:           eglSurfaceAttribVndr = (EGL_SURFACE_ATTRIB_FUNCPTR)func; break;
            case EGL_VENDOREX_BIND_TEX_IMAGE:           eglBindTexImageVndr = (EGL_BIND_TEXIMAGE_FUNCPTR)func; break;
            case EGL_VENDOREX_RELEASE_TEX_IMAGE:        eglReleaseTexImageVndr = (EGL_RELEASE_TEXIMAGE_FUNCPTR)func; break;
            case EGL_VENDOREX_SWAP_INTERVAL:            eglSwapIntervalVndr = (EGL_SWAP_INTERVAL_FUNCPTR)func; break;
            case EGL_VENDOREX_CREATE_CONTEXT:           eglCreateContextVndr = (EGL_CREATE_CONTEXT_FUNCPTR)func; break;
            case EGL_VENDOREX_DESTROY_CONTEXT:          eglDestroyContextVndr = (EGL_DESTROY_CONTEXT_FUNCPTR)func; break;
            case EGL_VENDOREX_MAKE_CURRENT:             eglMakeCurrentVndr = (EGL_MAKE_CURRENT_FUNCPTR)func; break;
            case EGL_VENDOREX_GET_CURRENT_CONTEXT:      eglGetCurrentContextVndr = (EGL_GET_CURRENT_CONTEXT_FUNCPTR)func; break;
            case EGL_VENDOREX_GET_CURRENT_SURFACE:      eglGetCurrentSurfaceVndr = (EGL_GET_CURRENT_SURFACE_FUNCPTR)func; break;
            case EGL_VENDOREX_GET_CURRENT_DISPLAY:      eglGetCurrentDisplayVndr = (EGL_GET_CURRENT_DISPLAY_FUNCPTR)func; break;
            case EGL_VENDOREX_QUERY_CONTEXT:            eglQueryContextVndr = (EGL_QUERY_CONTEXT_FUNCPTR)func; break;
            case EGL_VENDOREX_WAIT_GL:                  eglWaitGLVndr = (EGL_WAIT_GL_FUNCPTR)func; break;
            case EGL_VENDOREX_WAIT_NATIVE:              eglWaitNativeVndr = (EGL_WAIT_NATIVE_FUNCPTR)func; break;
            case EGL_VENDOREX_SWAP_BUFFERS:             eglSwapBuffersVndr = (EGL_SWAP_BUFFERS_FUNCPTR)func; break;
            case EGL_VENDOREX_COPY_BUFFERS:             eglCopyBuffersVndr = (EGL_COPY_BUFFERS_FUNCPTR)func; break;
            default:
                WARN("unexpected auto-function!");
                ret = EGL_FALSE;
                break;
        }
    } else {
        WARNN( "EglRegisterVndrExRootFunc failed ROOT:%(_eglVndrExFuncId)", funcId );
    }

    return ret;
}

EGLBoolean EglRegisterVndrExApiFunc( const EGLint apiId, const EGLint funcId, void * func, const char* funcName ) {

    EGLBoolean ret = EGL_FALSE;

    if ( EglIsValidVndrExApiId(apiId) && EglIsValidVndrExFuncId(funcId) ) {
        EGLVndrExFuncList * list = &EglVndrExFuncs[ apiId ];

        // we're going to do some pointer arithmetic, so treat every function pointer in the struct as the same (VOID_FUNC)
        VOID_FUNC_PTR * funcPtr = (VOID_FUNC_PTR *)list;

        funcPtr += funcId;
        *funcPtr = (VOID_FUNC_PTR)func;

        PRINT3N( "Register: %(_eglVndrExApiId):%(_eglVndrExFuncId) @ %p", apiId, funcId, (DWORD)func );

        strncpy( &EglVndrExFuncNames[ apiId ][0], funcName, EGL_VENDOREX_NAME_LENGTH-1 );
        EglVndrExFuncNames[ apiId ][ EGL_VENDOREX_NAME_LENGTH - 1 ] = '\0';

        ret = EGL_TRUE;
        
    } else {
        WARN2N( "EglRegisterVndrExFunc failed %(_eglVndrExApiId):%(_eglVndrExFuncId)", apiId, funcId );
    }

    return ret;
}

EGLBoolean EglIsValidVndrExApiId( const EGLint apiId ) {
    return (apiId >= 0 && apiId < EGL_NUM_VENDOREX_APIS) ? EGL_TRUE : EGL_FALSE;
}

EGLBoolean EglIsValidVndrExFuncId( const EGLint funcId ) {
    return (funcId >= 0 && funcId < EGL_NUM_VENDOREX_FUNCS) ? EGL_TRUE : EGL_FALSE;
}

EGLint EglGetCurrentVndrExApiId( void ) {
    EGLint ret = EGL_NUM_VENDOREX_APIS;
    EGLenum currApi = (EGLenum)kdGetThreadStorageKHR(eglCurrentApiKey);
    switch (currApi) {
        case EGL_OPENVG_API:    ret = EGL_VENDOREX_API_VG; break;
        case EGL_OPENGL_ES_API: ret = EGL_VENDOREX_API_GLES; break;
        default:
            WARNN("Unknown current API 0x%04x", currApi);
            break;
    }

    return ret;
}

static EGLint eglCommonVoidSimpleParamVndrEx( const EGLint funcId, const EGLint param1 ) {
    EGLint retVal = -1;
    const EGLint currVndrExApiId = EglGetCurrentVndrExApiId();

    if ( EglIsValidVndrExApiId(currVndrExApiId) && EglIsValidVndrExFuncId(funcId) ) {
        switch (funcId) {
            case EGL_VENDOREX_QUERY_API:
                if ( EglVndrExFuncs[currVndrExApiId].eglQueryAPIVndr ) {
                    retVal = (EGLint)EglVndrExFuncs[currVndrExApiId].eglQueryAPIVndr();
                }
                break;
            case EGL_VENDOREX_WAIT_CLIENT:
                if ( EglVndrExFuncs[currVndrExApiId].eglWaitClientVndr ) {
                    retVal = (EGLint)EglVndrExFuncs[currVndrExApiId].eglWaitClientVndr();
                }
                break;
            case EGL_VENDOREX_RELEASE_THREAD:
                if ( EglVndrExFuncs[currVndrExApiId].eglReleaseThreadVndr ) {
                    retVal = (EGLint)EglVndrExFuncs[currVndrExApiId].eglReleaseThreadVndr();
                }
                break;
            case EGL_VENDOREX_GET_CURRENT_CONTEXT:
                if ( EglVndrExFuncs[currVndrExApiId].eglGetCurrentContextVndr ) {
                    retVal = (EGLint)EglVndrExFuncs[currVndrExApiId].eglGetCurrentContextVndr();
                }
                break;
            case EGL_VENDOREX_GET_CURRENT_SURFACE:
                if ( EglVndrExFuncs[currVndrExApiId].eglGetCurrentSurfaceVndr ) {
                    retVal = (EGLint)EglVndrExFuncs[currVndrExApiId].eglGetCurrentSurfaceVndr( param1 );
                }
                break;
            case EGL_VENDOREX_GET_CURRENT_DISPLAY:
                if ( EglVndrExFuncs[currVndrExApiId].eglGetCurrentDisplayVndr ) {
                    retVal = (EGLint)EglVndrExFuncs[currVndrExApiId].eglGetCurrentDisplayVndr();
                }
                break;
            case EGL_VENDOREX_WAIT_GL:
                if ( EglVndrExFuncs[currVndrExApiId].eglWaitGLVndr ) {
                    retVal = (EGLint)EglVndrExFuncs[currVndrExApiId].eglWaitGLVndr();
                }
                break;
            case EGL_VENDOREX_WAIT_NATIVE:
                if ( EglVndrExFuncs[currVndrExApiId].eglWaitNativeVndr ) {
                    retVal = (EGLint)EglVndrExFuncs[currVndrExApiId].eglWaitNativeVndr( param1 );
                }
                break;
            default:
                WARNN( "eglCommonVoidSimpleParamVndrEx: unepxected function %(_eglVndrExFuncId)", funcId );
                return -1;
            }

            // no need to re-set the error key for GetError()
            if ( funcId != EGL_VENDOREX_GET_ERROR ) {
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EglVndrExFuncs[currVndrExApiId].eglGetErrorVndr()));
            }
    } else {
        WARN2N( "eglCommonVoidSimpleParamVndrEx failed %(_eglVndrExApiId):%(_eglVndrExFuncId)", currVndrExApiId, funcId );
    }

    return retVal;
}

EGLint eglGetErrorVndrEx( void ) {
    return (EGLint)kdGetThreadStorageKHR(eglLastErrorKey);
}

EGLDisplay eglGetDisplayVndrEx(NativeDisplayType display_id) {
    int i;
    int api;
    EGLDisplay newDisplay = EGL_NO_DISPLAY;

    PRINT("eglGetDisplayVndrEx");

    for ( i = 0; i < RIM_LCD_NUM_DISPLAYS; ++i) {
        if (EglDpyContext[i].id == display_id) {

            // found the display index
            for ( api = 0; api < EGL_NUM_VENDOREX_APIS; ++api ) {
                if ( EglVndrExFuncs[api].eglGetDisplayVndr ) {
                    newDisplay = EglVndrExFuncs[api].eglGetDisplayVndr( display_id );
                    if ( newDisplay == EGL_NO_DISPLAY ) {
                        WARNN( "GetDisplay failed: %(_eglVndrExApiId)", api );
                        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EglVndrExFuncs[api].eglGetErrorVndr()));
                        break;
                    }

                    EglVndrExDisplay[i].vndrDisplay[ api ] = newDisplay;
                    PRINT2N( "display = 0x%x %(_eglVndrExApiId)", (DWORD)newDisplay, api );
                }
            }

            if ( newDisplay == EGL_NO_DISPLAY ) {
                // error occured, reset
                for ( api = 0; api < EGL_NUM_VENDOREX_APIS; ++api ) {
                    EglVndrExDisplay[i].vndrDisplay[ api ] = EGL_NO_DISPLAY;
                }
            } else {
                // return value points to vender-ex struct...
                newDisplay = (EGLDisplay)&EglVndrExDisplay[i];
            }
            
            break;
        }
    }

    return newDisplay;
}

EGLBoolean eglTerminateVndrEx(EGLDisplay dpy) {
    int i;
    EGLVndrExDisplay * vDisp = (EGLVndrExDisplay *)dpy;
    EGLBoolean ret = EGL_TRUE;

    PRINT("eglTerminateVndrEx");

    for ( i = 0; vDisp != NULL && i < EGL_NUM_VENDOREX_APIS; ++i ) {
        if ( EglVndrExFuncs[i].eglTerminateVndr ) {
            ret = EglVndrExFuncs[i].eglTerminateVndr( vDisp->vndrDisplay[i] );
            //PRINT3N( "%(_eglVndrExApiId) ret=%c, disp=0x%x", i, (ret) ? 'T' : 'F', (int)vDisp->vndrDisplay[i] );
            if ( ret != EGL_TRUE ) {
                PRINTN( "eglTerminateVndrEx - failed for %(_eglVndrExApiId)", i );
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EglVndrExFuncs[i].eglGetErrorVndr()));                
                break;
            }
        }
    }

    return ret;
}

EglStr eglQueryStringVndrEx(EGLDisplay dpy, EGLint name) {
    int i;
    EglStr ret = "invalid";
    EGLVndrExDisplay * vDisp = (EGLVndrExDisplay *)dpy;

    PRINT("eglQueryStringVndrEx");

    // first registered api function wins...
    for ( i = 0; i < EGL_NUM_VENDOREX_APIS; ++i ) {
        if ( EglVndrExFuncs[i].eglQueryStringVndr ) {
            ret = EglVndrExFuncs[i].eglQueryStringVndr( vDisp->vndrDisplay[i], name );
            if ( EglVndrExFuncs[i].eglGetErrorVndr() != EGL_SUCCESS ) {
                WARNN( "eglQueryStringVndrEx - failed for %(_eglVndrExApiId)", i );
            }
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EglVndrExFuncs[i].eglGetErrorVndr()));
            break;
        }
    }

    return ret;
}

EGLBoolean eglGetConfigsVndrEx(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config) {
    WARN("eglGetConfigsVndrEx - do nothing");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean eglChooseConfigVndrEx(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config) {
    WARN("eglChooseConfigVndrEx - do nothing");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean eglGetConfigAttribVndrEx(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value) {
    EGLBoolean retVal = EGL_FALSE;
    EGLVndrExDisplay * vDisp = (EGLVndrExDisplay *)dpy;
    EGLVndrExConfig * vConfig = (EGLVndrExConfig *)config;

    PRINT("eglGetConfigAttribVndrEx");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONFIG));

    if ( vConfig ) {
        if ( EglIsValidVndrExApiId(vConfig->vndrApiId) ) {
            if ( EglVndrExFuncs[vConfig->vndrApiId].eglGetConfigAttribVndr ) {
                retVal = EglVndrExFuncs[vConfig->vndrApiId].eglGetConfigAttribVndr( vDisp->vndrDisplay[vConfig->vndrApiId], vConfig->vndrConfig, attribute, value );
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EglVndrExFuncs[vConfig->vndrApiId].eglGetErrorVndr()));
            } else {
                WARNN("eglGetConfigAttribVndrEx: no impl registered for %(_eglVndrExApiId)", vConfig->vndrApiId );
            }
        } else {
            WARNN("eglGetConfigAttribVndrEx: Unknown apiId %(_eglVndrExApiId)", vConfig->vndrApiId );
        }
    }

    return retVal;
}

static EGLSurface eglCreateSurfaceVndrExInt(const EGLint funcId, EGLDisplay dpy, EGLConfig config, void * customParam, const EGLint *attrib_list) {
    EGLVndrExDisplay * vDisp = (EGLVndrExDisplay *)dpy;
    EGLVndrExConfig * vConfig = (EGLVndrExConfig *)config;
    EGLVndrExSurface * vSurface = NULL;
    NativeWindowType win = (NativeWindowType)customParam; // for EGL_VENDOREX_CREATE_WINDOW_SURFACE
    NativePixmapType pixmap = (NativePixmapType)customParam; // for EGL_VENDOREX_CREATE_PIXMAP_SURFACE

    switch (funcId) {
        case EGL_VENDOREX_CREATE_WINDOW_SURFACE:
        case EGL_VENDOREX_CREATE_PBUFFER_SURFACE:
        case EGL_VENDOREX_CREATE_PIXMAP_SURFACE:
            // valid funcs
            break;
        default:
            WARNN( "eglCreateSurfaceVndrExInt - invalid %(_eglVndrExFuncId)", funcId );
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_CONFIG);
            return EGL_NO_SURFACE;
    }
    
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_SUCCESS);

    if (vConfig) {
        vSurface = (EGLVndrExSurface *)kdMalloc(sizeof(EGLVndrExSurface));
        if(!vSurface){
            PRINT( "eglCreateWindowSurfaceVndrEx - alloc fail" );
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_ALLOC);
            return EGL_NO_SURFACE;
        }
        vSurface->vndrSurface = EGL_NO_SURFACE;
        vSurface->vndrApiId = EGL_NUM_VENDOREX_APIS;
        vSurface->parentSurface = NULL;

        switch (vConfig->vndrApiId) {
            case EGL_VENDOREX_API_VG:
            case EGL_VENDOREX_API_GLES:
                vSurface->vndrApiId = vConfig->vndrApiId;
                
                switch (funcId) {
                    case EGL_VENDOREX_CREATE_WINDOW_SURFACE:
                        if ( EglVndrExFuncs[vConfig->vndrApiId].eglCreateWindowSurfaceVndr ) {
                            vSurface->vndrSurface = EglVndrExFuncs[vConfig->vndrApiId].eglCreateWindowSurfaceVndr( vDisp->vndrDisplay[vConfig->vndrApiId], vConfig->vndrConfig, win, attrib_list );
                            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EglVndrExFuncs[vConfig->vndrApiId].eglGetErrorVndr()));
                        }
                        break;
                    case EGL_VENDOREX_CREATE_PBUFFER_SURFACE:
                        if ( EglVndrExFuncs[vConfig->vndrApiId].eglCreatePbufferSurfaceVndr ) {
                            vSurface->vndrSurface = EglVndrExFuncs[vConfig->vndrApiId].eglCreatePbufferSurfaceVndr( vDisp->vndrDisplay[vConfig->vndrApiId], vConfig->vndrConfig, attrib_list );
                            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EglVndrExFuncs[vConfig->vndrApiId].eglGetErrorVndr()));
                        }
                        break;
                    case EGL_VENDOREX_CREATE_PIXMAP_SURFACE:
                        if ( EglVndrExFuncs[vConfig->vndrApiId].eglCreatePixmapSurfaceVndr ) {
                            vSurface->vndrSurface = EglVndrExFuncs[vConfig->vndrApiId].eglCreatePixmapSurfaceVndr( vDisp->vndrDisplay[vConfig->vndrApiId], vConfig->vndrConfig, pixmap, attrib_list );
                            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EglVndrExFuncs[vConfig->vndrApiId].eglGetErrorVndr()));
                        }
                        break;
                    default:
                        WARNN( "eglCreateSurfaceVndrExInt - invalid %(_eglVndrExFuncId)", funcId );
                        kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_CONFIG);
                        return EGL_NO_SURFACE;
                }
                break;
            default:
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_CONFIG);                
                WARNN("Unknown apiId %(_eglVndrExApiId)", vConfig->vndrApiId);
                break;
        }
                
        if (vSurface->vndrSurface != EGL_NO_SURFACE) {
            // Return the new surface
            PRINTN( "new surface: v0x%08x", vSurface->vndrSurface );
            return (EGLSurface)vSurface;
        } else {
            // Free the EGLVendorSurfaceAmanithQC if we end up here
            kdFree(vSurface);
            return EGL_NO_SURFACE;
        }
    }

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_CONFIG);
    return EGL_NO_SURFACE;
}

EGLSurface eglCreateWindowSurfaceVndrEx(EGLDisplay dpy, EGLConfig config, NativeWindowType win, const EGLint *attrib_list) {
    PRINT("eglCreateWindowSurfaceVndrEx");
    return eglCreateSurfaceVndrExInt(EGL_VENDOREX_CREATE_WINDOW_SURFACE, dpy, config, win, attrib_list);
}

EGLSurface eglCreatePbufferSurfaceVndrEx(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list) {
    PRINT("eglCreatePbufferSurfaceVndrEx");
    return eglCreateSurfaceVndrExInt(EGL_VENDOREX_CREATE_PBUFFER_SURFACE, dpy, config, NULL, attrib_list);
}

EGLSurface eglCreatePixmapSurfaceVndrEx(EGLDisplay dpy, EGLConfig config, NativePixmapType pixmap, const EGLint *attrib_list) {
    PRINT("eglCreatePixmapSurfaceVndrEx");
    return eglCreateSurfaceVndrExInt(EGL_VENDOREX_CREATE_PIXMAP_SURFACE, dpy, config, pixmap, attrib_list);
}

EGLBoolean eglDestroySurfaceVndrEx(EGLDisplay dpy, EGLSurface surface) {
    EGLBoolean retVal = EGL_FALSE;
    EGLVndrExDisplay * vDisp = (EGLVndrExDisplay *)dpy;
    EGLVndrExSurface * vSurface = (EGLVndrExSurface *)surface;
    PRINT2N("eglDestroySurfaceVndrEx - 0x%08x (v0x%08x)", vSurface, (vSurface) ? vSurface->vndrSurface : (EGLSurface)EGL_NO_SURFACE);

    if (vSurface && vSurface->vndrSurface != EGL_NO_SURFACE) {
        if ( EglIsValidVndrExApiId(vSurface->vndrApiId) ) {
            if ( EglVndrExFuncs[vSurface->vndrApiId].eglDestroySurfaceVndr ) {
                retVal = EglVndrExFuncs[vSurface->vndrApiId].eglDestroySurfaceVndr( vDisp->vndrDisplay[vSurface->vndrApiId], vSurface->vndrSurface );
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EglVndrExFuncs[vSurface->vndrApiId].eglGetErrorVndr()));
            }
        } else {
            WARNN("eglDestroySurfaceVndrEx: Unknown apiId %(_eglVndrExApiId)", vSurface->vndrApiId);
        }
        
        // If everything went well, destroy the vendor surface struct
        if (retVal == EGL_TRUE) {
            kdFree(vSurface);
            return retVal;
        }
    }
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_SURFACE);
    
    return EGL_FALSE;
}

EGLBoolean eglQuerySurfaceVndrEx(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value) {
    EGLBoolean retVal = EGL_FALSE;
    EGLVndrExDisplay * vDisp = (EGLVndrExDisplay *)dpy;
    EGLVndrExSurface * vSurface = (EGLVndrExSurface *)surface;

    PRINT("eglQuerySurfaceVndrEx");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));

    if ( vSurface ) {
        if ( EglIsValidVndrExApiId(vSurface->vndrApiId) ) {
            if ( EglVndrExFuncs[vSurface->vndrApiId].eglQuerySurfaceVndr ) {
                retVal = EglVndrExFuncs[vSurface->vndrApiId].eglQuerySurfaceVndr( vDisp->vndrDisplay[vSurface->vndrApiId], vSurface->vndrSurface, attribute, value );
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EglVndrExFuncs[vSurface->vndrApiId].eglGetErrorVndr()));
            } else {
                WARNN("eglQuerySurfaceVndrEx: no impl registered for %(_eglVndrExApiId)", vSurface->vndrApiId);
            }
        } else {
            WARNN("eglQuerySurfaceVndrEx: Unknown apiId %(_eglVndrExApiId)", vSurface->vndrApiId);
        }
    }

    return retVal;
}

EGLBoolean eglBindAPIVndrEx(EGLenum api) {
    int i;
    EGLBoolean ret = EGL_FALSE;

    PRINT("eglBindAPIVndrEx");

    for ( i = 0; i < EGL_NUM_VENDOREX_APIS; ++i ) {
        if ( EglVndrExFuncs[i].eglBindAPIVndr) {
            ret = EglVndrExFuncs[i].eglBindAPIVndr( api );
            if ( ret != EGL_TRUE ) {
                PRINTN( "eglBindAPIVndrEx - failed for %(_eglVndrExApiId)", i );
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EglVndrExFuncs[i].eglGetErrorVndr()));
                break;
            }
        }
    }

    return ret;
}

EGLenum eglQueryAPIVndrEx(void) {
    PRINT( "eglQueryAPIVndrEx" );
    return (EGLenum)eglCommonVoidSimpleParamVndrEx( EGL_VENDOREX_QUERY_API, -1 );
}
EGLBoolean eglWaitClientVndrEx(void) {
    PRINT( "eglQueryAPIVndrEx" );
    return (EGLBoolean)eglCommonVoidSimpleParamVndrEx( EGL_VENDOREX_WAIT_CLIENT, -1 );
}

EGLBoolean eglReleaseThreadVndrEx(void) {
    PRINT( "eglReleaseThreadVndrEx" );
    return (EGLBoolean)eglCommonVoidSimpleParamVndrEx( EGL_VENDOREX_RELEASE_THREAD, -1 );
}

EGLSurface eglCreatePbufferFromClientBufferVndrEx(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list) {
    EGLVndrExDisplay * vDisp = (EGLVndrExDisplay *)dpy;
    EGLVndrExSurface * vSurface = NULL;
    EGLVndrExConfig * vConfig = (EGLVndrExConfig *)config;

    PRINT("eglCreatePbufferFromClientBufferVndrEx");

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_CONFIG);

    if (vConfig) {
        switch (vConfig->vndrApiId) {
            case EGL_VENDOREX_API_VG:
                vSurface = (EGLVndrExSurface *)kdMalloc(sizeof(EGLVndrExSurface));
                if ( !vSurface ) {
                    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_ALLOC);
                    return EGL_NO_SURFACE;
                }
                
                vSurface->vndrApiId = EGL_VENDOREX_API_VG;
                vSurface->parentSurface = NULL;
                vSurface->vndrSurface = EGL_NO_SURFACE;

                if ( EglVndrExFuncs[vConfig->vndrApiId].eglCreatePbufferFromClientBufferVndr ) {
                    EGL_CREATE_PBUFFER_FROMCLIENTBUFFER_FUNCPTR func = EglVndrExFuncs[vConfig->vndrApiId].eglCreatePbufferFromClientBufferVndr;
                    vSurface->vndrSurface = func( vDisp->vndrDisplay[vConfig->vndrApiId], buftype, buffer, vConfig->vndrConfig, attrib_list);
                    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EglVndrExFuncs[vSurface->vndrApiId].eglGetErrorVndr()));
                }
                    
                if ( vSurface->vndrSurface != EGL_NO_SURFACE ) {
                    // Return the new surface
                    return (EGLSurface)vSurface;
                } else {
                    // Free the vndrEx surface if we end up here
                    kdFree(vSurface);
                }
                break;
            case EGL_VENDOREX_API_GLES:
                WARN("OpenGL ES rendering is not supported on client buffers");
                break;
            default:
                WARNN("eglCreatePbufferFromClientBufferVndrEx: Unknown apiId %(_eglVndrExApiId)", vConfig->vndrApiId);
                break;
        }
    }

    return EGL_NO_SURFACE;
}

EGLBoolean eglSurfaceAttribVndrEx(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value) {
    EGLBoolean retVal = EGL_FALSE;
    EGLVndrExDisplay * vDisp = (EGLVndrExDisplay *)dpy;
    EGLVndrExSurface * vSurface = (EGLVndrExSurface *)surface;

    PRINT("eglSurfaceAttribVndrEx");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));

    if ( vSurface ) {
        if ( EglIsValidVndrExApiId(vSurface->vndrApiId) ) {
            if ( EglVndrExFuncs[vSurface->vndrApiId].eglSurfaceAttribVndr ) {
                retVal = EglVndrExFuncs[vSurface->vndrApiId].eglSurfaceAttribVndr( vDisp->vndrDisplay[vSurface->vndrApiId], vSurface->vndrSurface, attribute, value );
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EglVndrExFuncs[vSurface->vndrApiId].eglGetErrorVndr()));
            } else {
                WARNN("eglSurfaceAttribVndrEx: no impl registered for %(_eglVndrExApiId)", vSurface->vndrApiId );
            }
        } else {
            WARNN("eglSurfaceAttribVndrEx: Unknown apiId %(_eglVndrExApiId)", vSurface->vndrApiId );
        }
    }

    return retVal;
}

EGLBoolean eglBindTexImageVndrEx(EGLDisplay dpy, EGLSurface surface, EGLint buffer) {
    EGLBoolean retVal = EGL_FALSE;
    EGLVndrExDisplay * vDisp = (EGLVndrExDisplay *)dpy;
    EGLVndrExSurface * vSurface = (EGLVndrExSurface *)surface;

    PRINT("eglBindTexImageVndrEx");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));

    if ( vSurface ) {
        if ( EglIsValidVndrExApiId(vSurface->vndrApiId) ) {
            if ( EglVndrExFuncs[vSurface->vndrApiId].eglBindTexImageVndr ) {
                retVal = EglVndrExFuncs[vSurface->vndrApiId].eglBindTexImageVndr( vDisp->vndrDisplay[vSurface->vndrApiId], vSurface->vndrSurface, buffer );
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EglVndrExFuncs[vSurface->vndrApiId].eglGetErrorVndr()));
            } else {
                WARNN("eglBindTexImageVndrEx: no impl registered for %(_eglVndrExApiId)", vSurface->vndrApiId);
            }
        } else {
            WARNN("eglBindTexImageVndrEx: Unknown apiId %(_eglVndrExApiId)", vSurface->vndrApiId);
        }
    }

    return retVal;
}

EGLBoolean eglReleaseTexImageVndrEx(EGLDisplay dpy, EGLSurface surface, EGLint buffer) {
    EGLBoolean retVal = EGL_FALSE;
    EGLVndrExDisplay * vDisp = (EGLVndrExDisplay *)dpy;
    EGLVndrExSurface * vSurface = (EGLVndrExSurface *)surface;

    PRINT("eglReleaseTexImageVndrEx");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));

    if ( vSurface ) {
        if ( EglIsValidVndrExApiId(vSurface->vndrApiId) ) {
            if ( EglVndrExFuncs[vSurface->vndrApiId].eglReleaseTexImageVndr ) {
                retVal = EglVndrExFuncs[vSurface->vndrApiId].eglReleaseTexImageVndr( vDisp->vndrDisplay[vSurface->vndrApiId], vSurface->vndrSurface, buffer );
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EglVndrExFuncs[vSurface->vndrApiId].eglGetErrorVndr()));
            } else {
                WARNN("eglReleaseTexImageVndrEx: no impl registered for %(_eglVndrExApiId)", vSurface->vndrApiId);
            }
        } else {
            WARNN("eglReleaseTexImageVndrEx: Unknown apiId %(_eglVndrExApiId)", vSurface->vndrApiId);
        }
    }

    return retVal;
}

EGLBoolean eglSwapIntervalVndrEx(EGLDisplay dpy, EGLint interval) {
    WARN("eglSwapIntervalVndrEx - do nothing");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLContext eglCreateContextVndrEx(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list) {
    EGLVndrExDisplay * vDisp = (EGLVndrExDisplay *)dpy;
    EGLVndrExConfig * vConfig = (EGLVndrExConfig *)config;
    EGLContext vndrex_share_ctx = EGL_NO_CONTEXT;
    EGLVndrExContext * newContext = NULL;
    const EGLint currVndrExApiId = EglGetCurrentVndrExApiId();

    PRINTN("eglCreateContextVndrEx - config = 0x%08x", (vConfig) ? vConfig->vndrConfig : (EGLConfig)-1);
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_SUCCESS);

    if (vConfig) {
        if (vConfig->vndrConfig) {
            switch (currVndrExApiId) {
                case EGL_VENDOREX_API_VG:
                case EGL_VENDOREX_API_GLES:
                    if ( vConfig->vndrApiId != currVndrExApiId ) {
                        PRINT2N( "Config doesn't match API %(_eglVndrExApiId) != %(_eglVndrExApiId)", vConfig->vndrApiId, currVndrExApiId );
                        kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_CONFIG);
                    }
                    else if ( EglVndrExFuncs[currVndrExApiId].eglCreateContextVndr ) {
                        PRINTN( "Creating %(_eglVndrExApiId) context", currVndrExApiId );
                        
                        // Allocate memory for context...
                        newContext = (EGLVndrExContext *)kdMalloc( sizeof(EGLVndrExContext) );
                        if (!newContext) {
                            kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_ALLOC);
                            return EGL_NO_CONTEXT;
                        }
                        newContext->vndrApiId = currVndrExApiId;

                        vndrex_share_ctx = share_context;
                        if ( share_context != EGL_NO_CONTEXT ) {
                            // TODO(BT) - can VG/GLES w/ different vendors support shared contexts?
                            vndrex_share_ctx = ((EGLVndrExContext *)share_context)->vndrContext;
                        }
                            
                        newContext->vndrContext = EglVndrExFuncs[currVndrExApiId].eglCreateContextVndr( vDisp->vndrDisplay[currVndrExApiId], vConfig->vndrConfig, 
                            vndrex_share_ctx, attrib_list );

                        PRINT2N("%(_eglVndrExApiId) context = 0x%08x", currVndrExApiId, (DWORD)newContext->vndrContext);
                        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EglVndrExFuncs[currVndrExApiId].eglGetErrorVndr()));

                        // Check if the returned context is valid
                        if(newContext->vndrContext == EGL_NO_CONTEXT){
                            kdFree(newContext);
                            return EGL_NO_CONTEXT;
                        }

                        return (EGLContext)newContext;
                    }
		            break;
                default:
                    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_MATCH);
                    break;
            }
        }
    }

    return EGL_NO_CONTEXT;
}

EGLBoolean eglDestroyContextVndrEx(EGLDisplay dpy, EGLContext ctx) {
    EGLBoolean retVal = EGL_FALSE;
    EGLVndrExDisplay * vDisp = (EGLVndrExDisplay *)dpy;
    EGLVndrExContext * vContext = (EGLVndrExContext *)ctx;
    PRINT("eglDestroyContextVndrEx");

    if (vContext && vContext->vndrContext != EGL_NO_SURFACE) {
        if ( EglIsValidVndrExApiId(vContext->vndrApiId) ) {
            if ( EglVndrExFuncs[vContext->vndrApiId].eglDestroyContextVndr ) {
                retVal = EglVndrExFuncs[vContext->vndrApiId].eglDestroyContextVndr( vDisp->vndrDisplay[vContext->vndrApiId], vContext->vndrContext );
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EglVndrExFuncs[vContext->vndrApiId].eglGetErrorVndr()));
            }
        } else {
            WARNN("eglDestroyContextVndrEx: Unknown apiId %(_eglVndrExApiId)", vContext->vndrApiId);
        }
        
        // If everything went well, destroy the vendor surface struct
        if (retVal == EGL_TRUE) {
            kdFree(vContext);
            return retVal;
        }
    }
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)EGL_BAD_CONTEXT);
    
    return EGL_FALSE;
}


EGLBoolean eglMakeCurrentVndrEx(EGLDisplay dpy, EGLSurface drawSurf, EGLSurface readSurf, EGLContext ctx) {
    EGLBoolean retBool = EGL_TRUE;
    EGLDisplayContext * dpyContext = NULL;
    EGLSurfaceDescriptor * readSurfaceDesc = (EGLSurfaceDescriptor *)readSurf;
    EGLSurfaceDescriptor * drawSurfaceDesc = (EGLSurfaceDescriptor *)drawSurf;
    EGLRenderingContext * context = (EGLRenderingContext *)ctx;
    EGLVndrExDisplay * vDisp = NULL;

    PRINT("eglMakeCurrentVndrEx");
    PRINT3N("read 0x%08x draw 0x%08x ctx 0x%08x", readSurfaceDesc, drawSurfaceDesc, context);

    // Get the EGLDisplayContext, we already checked if it's valid
    dpyContext = eglGetDisplayContextInt( dpy );
    if (!dpyContext) {
        WARN( "eglMakeCurrentVndrEx : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    vDisp = (EGLVndrExDisplay *)dpyContext->vendorDisplay;

    if (drawSurf == EGL_NO_SURFACE && readSurf == EGL_NO_SURFACE && ctx == EGL_NO_CONTEXT) {
        const EGLint currVndrExApiId = EglGetCurrentVndrExApiId();
        
        switch (currVndrExApiId) {
            case EGL_VENDOREX_API_VG:
            case EGL_VENDOREX_API_GLES:
                PRINTN("eglMakeCurrentVndrEx - making %(_eglVndrExApiId) NULL", currVndrExApiId);
                if ( EglVndrExFuncs[currVndrExApiId].eglMakeCurrentVndr ) {
                    retBool = EglVndrExFuncs[currVndrExApiId].eglMakeCurrentVndr( vDisp->vndrDisplay[currVndrExApiId], EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
                    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EglVndrExFuncs[currVndrExApiId].eglGetErrorVndr()));
                } else {
                    WARNN("eglMakeCurrentVndrEx - no implementation for %(_eglVndrExApiId)", currVndrExApiId);
                }
                break;
            default:
                // RIM implementation...
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
                retBool = EGL_TRUE;
                break;
        }       
    } else if (drawSurf != EGL_NO_SURFACE && readSurf != EGL_NO_SURFACE && ctx != EGL_NO_CONTEXT) {
        EGLVndrExSurface *vendorRead = (EGLVndrExSurface *)(readSurfaceDesc->vendorSurface);
        EGLVndrExSurface *vendorDraw = (EGLVndrExSurface *)(drawSurfaceDesc->vendorSurface);
        EGLVndrExContext *vendorContext = (EGLVndrExContext *)(context->vendorContext);
        PRINT3N("Vendor read 0x%08x draw 0x%08x ctx 0x%08x", vendorRead, vendorDraw, vendorContext);

        if (vendorRead != EGL_NO_SURFACE && vendorDraw != EGL_NO_SURFACE && vendorContext != EGL_NO_CONTEXT) {
            // Check for surface and context compatibility
            switch (vendorContext->vndrApiId) {
                case EGL_VENDOREX_API_VG:
                case EGL_VENDOREX_API_GLES:
                    // Make sure that the surfaces and the context have the same vendor
                    if (vendorDraw->vndrApiId == vendorContext->vndrApiId && vendorRead->vndrApiId == vendorContext->vndrApiId) {
                        PRINTN("eglMakeCurrentVndrEx - making %(_eglVndrExApiId)", vendorContext->vndrApiId);
                        if ( EglVndrExFuncs[vendorContext->vndrApiId].eglMakeCurrentVndr ) {
                            retBool = EglVndrExFuncs[vendorContext->vndrApiId].eglMakeCurrentVndr( vDisp->vndrDisplay[vendorContext->vndrApiId], vendorDraw->vndrSurface, vendorRead->vndrSurface, vendorContext->vndrContext );
                            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EglVndrExFuncs[vendorContext->vndrApiId].eglGetErrorVndr()));
                        } else {
                            WARNN("eglMakeCurrentVndrEx - no implementation for %(_eglVndrExApiId)", vendorContext->vndrApiId);
                        }
                    } else {
                        retBool = EGL_FALSE;
                        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
                    }
                    break;
                default:
                    WARNN("Unknown apiId %(_eglVndrExApiId)", vendorContext->vndrApiId);
                    retBool = EGL_FALSE;
                    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONTEXT));
                    break;
            }
        } else if (vendorRead != EGL_NO_SURFACE || vendorDraw != EGL_NO_SURFACE || vendorContext != EGL_NO_CONTEXT){
            //Can't mix our implementation and vendor implementation
            WARN("Can't mix surfaces and context from different EGL implementations");
            PRINT3N("draw 0x%08x, read 0x%08x, ctx 0x%08x", vendorDraw, vendorRead, vendorContext);
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
            return EGL_FALSE;
        } else {
            // All of the parameters are RIM Implementation
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
            retBool = EGL_TRUE;
        }
    } else {
        retBool = EGL_FALSE;
    #if !defined(RIM_NDK)    
        CATFAIL_WITH_CODE( "broken promise in eglMakeCurrentMRVLWrap", FAILURE_DESCRIBED );
    #else
        log_printf(LOG_CRITICAL, "broken promise in eglMakeCurrentMRVLWrap");
        _exit(0);
    #endif    
        //WARN("eglMakeCurrentVndrEx - Bad mix of parameters");
        //kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
        //return EGL_FALSE;
    }
    
    return retBool;
}

EGLContext eglGetCurrentContextVndrEx(void) {
    PRINT( "eglGetCurrentContextVndrEx" );
    return (EGLContext)eglCommonVoidSimpleParamVndrEx( EGL_VENDOREX_GET_CURRENT_CONTEXT, -1 );
}

EGLSurface eglGetCurrentSurfaceVndrEx(EGLint readdraw) {
    PRINT( "eglGetCurrentSurfaceVndrEx" );
    return (EGLSurface)eglCommonVoidSimpleParamVndrEx( EGL_VENDOREX_GET_CURRENT_SURFACE, readdraw );
}

EGLDisplay eglGetCurrentDisplayVndrEx(void) {
    PRINT( "eglGetCurrentDisplayVndrEx" );
    return (EGLDisplay)eglCommonVoidSimpleParamVndrEx( EGL_VENDOREX_GET_CURRENT_DISPLAY, -1 );
}

EGLBoolean eglQueryContextVndrEx(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value) {
    EGLBoolean retVal = EGL_FALSE;
    EGLVndrExDisplay * vDisp = (EGLVndrExDisplay *)dpy;
    EGLVndrExContext * vContext = (EGLVndrExContext *)ctx;

    PRINT("eglQueryContextVndrEx");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONTEXT));

    if ( vContext ) {
        if ( EglIsValidVndrExApiId(vContext->vndrApiId) ) {
            if ( EglVndrExFuncs[vContext->vndrApiId].eglQueryContextVndr ) {
                retVal = EglVndrExFuncs[vContext->vndrApiId].eglQueryContextVndr( vDisp->vndrDisplay[vContext->vndrApiId], vContext->vndrContext, attribute, value );
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EglVndrExFuncs[vContext->vndrApiId].eglGetErrorVndr()));
            } else {
                WARNN("eglQueryContextVndrEx: no impl registered for %(_eglVndrExApiId)", vContext->vndrApiId);
            }
        } else {
            WARNN("eglQueryContextVndrEx: Unknown apiId %(_eglVndrExApiId)", vContext->vndrApiId);
        }
    }

    return retVal;
}

EGLBoolean eglWaitGLVndrEx(void) {
    PRINT( "eglWaitGLVndrEx" );
    return (EGLBoolean)eglCommonVoidSimpleParamVndrEx( EGL_VENDOREX_WAIT_GL, -1 );
}

EGLBoolean eglWaitNativeVndrEx(EGLint engine) {
    PRINT( "eglWaitNativeVndrEx" );
    return (EGLBoolean)eglCommonVoidSimpleParamVndrEx( EGL_VENDOREX_WAIT_NATIVE, engine );
}

EGLBoolean eglSwapBuffersVndrEx(EGLDisplay dpy, EGLSurface surface) {
    EGLDisplayContext       *dpyContext = NULL;
    EGLBoolean              retVal = EGL_TRUE;
    EGLSurfaceDescriptor    *surfaceDesc = (EGLSurfaceDescriptor *)surface;
#ifndef RIM_WINDOW_MANAGER
    LcdConfig               *lcdCfg = NULL;
#endif // RIM_WINDOW_MANAGER
    EGLVndrExDisplay *      vDisp = NULL;

    PRINT("eglSwapBuffersVndrEx");

    //LV1( LOG_EGL, PRINT("eglSwapBuffersVTMVWrap"));

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if( EGL_NO_CONTEXT == dpyContext ) {
        WARN( "eglSwapBuffersVndrEx : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    vDisp = (EGLVndrExDisplay *)dpyContext->vendorDisplay;

#ifndef RIM_WINDOW_MANAGER
    lcdCfg = LcdGetLcdConfigPtr( dpyContext->id );
#endif // RIM_WINDOW_MANAGER

    PRINTN("surfaceDesc 0x%08x", surfaceDesc);
    if (surfaceDesc) {
        PRINTN("eglSwapBuffersVndrEx: surfaceDesc->surfaceType 0x%08x", surfaceDesc->surfaceType);
    }

    if( EGL_NO_SURFACE != surfaceDesc && EGL_WINDOW_BIT == surfaceDesc->surfaceType ) {
        EGLVndrExSurface * vendorSurface = (EGLVndrExSurface *)surfaceDesc->vendorSurface;
    
    #ifndef RIM_WINDOW_MANAGER
        const NativeWindow      *win = surfaceDesc->nativeWindow;
        const NativeWinRect_t   *clip = &win->clipRect;
        const DWORD             lcdXPos = win->windowSize.x + clip->x;
        const DWORD             lcdYPos = lcdCfg->height - win->windowSize.y
                                        - clip->y - clip->height;
    #endif // RIM_WINDOW_MANAGER

        //LV1( LOG_EGL, PRINT("VTMV eglSwapBuffers"));

        switch (vendorSurface->vndrApiId) {
            case EGL_VENDOREX_API_VG:
            case EGL_VENDOREX_API_GLES:
                if ( EglVndrExFuncs[vendorSurface->vndrApiId].eglSwapBuffersVndr ) {
                    //PRINT2N( "Swap vendor ex: surface 0x%08x (v0x%08x)", vendorSurface, vendorSurface->vndrSurface );
                    retVal = EglVndrExFuncs[vendorSurface->vndrApiId].eglSwapBuffersVndr( vDisp->vndrDisplay[vendorSurface->vndrApiId], vendorSurface->vndrSurface );

                    // BT - the vendor impl will set the error key (unforunate inconsistency)
                    //kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EglVndrExFuncs[vendorSurface->vndrApiId].eglGetErrorVndr()));
                }
                break;
            default:
                WARNN("eglSwapBuffersVndrEx: Unknown apiId %(_eglVndrExApiId)", vendorSurface->vndrApiId);
                retVal = EGL_FALSE;
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
                break;
        }

    #ifndef RIM_WINDOW_MANAGER
        //LV1(LOG_EGL, PRINT4N( "Swap buffers updating LCD %d %d %d %d",
                               //lcdXPos, lcdYPos,
                               //clip->width,
                               //clip->height));
    
        LcdUpdateInt(dpyContext->id, lcdXPos, lcdYPos, clip->width, clip->height);
    #endif // RIM_WINDOW_MANAGER
    }

    return retVal;
}

EGLBoolean eglCopyBuffersVndrEx(EGLDisplay dpy, EGLSurface surface, NativePixmapType target) {
    EGLBoolean retVal = EGL_FALSE;
    EGLSurfaceDescriptor *surfaceDesc = (EGLSurfaceDescriptor *)surface;
    // We already checked if a vendorSurface exists in eglCopyBuffers()
    EGLVndrExSurface *vendorSurface = (EGLVndrExSurface *)(surfaceDesc->vendorSurface);

    PRINT("eglCopyBuffersVndrEx");

    switch (vendorSurface->vndrApiId) {
        case EGL_VENDOREX_API_VG:
        case EGL_VENDOREX_API_GLES:
            if ( EglVndrExFuncs[vendorSurface->vndrApiId].eglCopyBuffersVndr ) {
                retVal = EglVndrExFuncs[vendorSurface->vndrApiId].eglCopyBuffersVndr( dpy, surface, target );
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EglVndrExFuncs[vendorSurface->vndrApiId].eglGetErrorVndr()));
            }
            break;
        default:
            WARNN("eglCopyBuffersAmanithQC: Unknown apiId %(_eglVndrExApiId)", vendorSurface->vndrApiId);
            retVal = EGL_FALSE;
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
            break;
    }
    
    return retVal;
}

#if defined RIM_EGL_MRVL && defined RIM_EGL_VTMV

#include <egl_mrvl.h>
#include <egl_vtmv.h>

EGLConfig EglVendorExAllocConfig( EGLint renderableType, EGLConfig vendorConfig ) {
    EGLConfig ret = EGL_NO_CONFIG;

    EGLVndrExConfig * vConfig = (EGLVndrExConfig *)kdMalloc( sizeof(EGLVndrExConfig) );
    if (vConfig) {
        switch (renderableType) {
        case EGL_OPENVG_BIT:
            vConfig->vndrApiId = EGL_VENDOREX_API_VG;
            break;
        case EGL_OPENGL_ES_BIT:
        case EGL_OPENGL_ES2_BIT:
            vConfig->vndrApiId = EGL_VENDOREX_API_GLES;
            break;
        default:
            WARNN( "EglVendorExAllocConfig : invalid renderableType=%d", renderableType );
            kdFree( vConfig );
            vConfig = NULL;
            break;
        }

        if ( vConfig ) {
            vConfig->vndrConfig = vendorConfig;
            ret = (EGLConfig)vConfig;
        }
    } else {
        WARN( "EglVendorExAllocConfig - kdMalloc() failure" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));
    }

    return ret;
}

void EglVendorExFreeConfig( EGLConfig config ) {
    EGLVndrExConfig * vConfig = (EGLVndrExConfig *)config;
    if ( vConfig ) {
        kdFree( vConfig );
    }
}

EGLBoolean eglInitializeVndrMrvlVtmv(EGLDisplay dpy, EGLint *major, EGLint *minor) {
    EGLBoolean retBool = EGL_TRUE;
    EGLDisplayContext *dpyContext = NULL;
    EGLVndrExDisplay * vDisp = NULL;

    DWORD i,j;
    EGLint numConfig = 0;
    EGLConfig   tempConfig;
    EGLint renderableType = 0;

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


    PRINT("eglInitializeVndrMrvlVtmv");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglInitializeVndrMrvlVtmv : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        retBool = EGL_FALSE;
    }

    if ( retBool ) {
        vDisp = (EGLVndrExDisplay *)dpyContext->vendorDisplay;

        // TODO(BT): Check should be added to not reinitialize a display -> change EGLDisplayContext architeture

        retBool = eglInitializeMRVL( vDisp->vndrDisplay[EGL_VENDOREX_API_VG], major, minor );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EglVndrExFuncs[EGL_VENDOREX_API_VG].eglGetErrorVndr()));

        if ( retBool ) {
            retBool = eglInitializeVTMV( vDisp->vndrDisplay[EGL_VENDOREX_API_GLES], major, minor );
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EglVndrExFuncs[EGL_VENDOREX_API_GLES].eglGetErrorVndr()));

            if ( retBool ) {
                //Initialize Extensions to match our function pointers
                eglInitExtension();
            } else {
                WARN("eglInitializeVTMV failed!");
                eglTerminateMRVL( vDisp->vndrDisplay[EGL_VENDOREX_API_VG] );
            }   
        } else {
            WARN("eglInitializeMRVL failed!");
        }
    }

    if ( retBool ) {
    #if defined (EGL_EXTRA_CONFIG_DEBUG)
        printAllVendorConfigs( vndrExVGDisplay );
    #endif
    }
    
    if ( eglUseNewChooseConfigVendor ) {
        if ( retBool ) {
            EGLint * renderableTypeVal = NULL;
            DWORD j;
            for ( j = 0; attribList[2*j] != EGL_NONE; ++j) {
                //attribList[2*j+1] = eglConfigDescriptors[i].ourConfig[CONFIG_ELEMENT(attribList[2*j])];
                // Use the renderable type to decide which set of vendor functions to use
                if(attribList[2*j] == EGL_RENDERABLE_TYPE){
                    renderableTypeVal = &attribList[2*j+1];
                }
            }

            if ( renderableTypeVal ) {
                PRINT( "eglGenericChooseConfigVendor - OpenVG" );
                *renderableTypeVal = EGL_OPENVG_BIT;
                retBool = eglGenericChooseConfigVendor( dpy, vDisp->vndrDisplay[EGL_VENDOREX_API_VG], attribList, eglGetConfigsMRVL, eglGetConfigAttribMRVL, EglVendorExAllocConfig, EglVendorExFreeConfig );
                if ( retBool ) {
                    PRINT( "eglGenericChooseConfigVendor - OpenGL ES" );
                    *renderableTypeVal = (EGL_OPENGL_ES_BIT | EGL_OPENGL_ES2_BIT);
                    retBool = eglGenericChooseConfigVendor( dpy, vDisp->vndrDisplay[EGL_VENDOREX_API_GLES], attribList, eglGetConfigsVTMV, eglGetConfigAttribVTMV, EglVendorExAllocConfig, EglVendorExFreeConfig );
                }
            } else {
                retBool = EGL_FALSE;
            }
        }
    } else {
        // We need to populate our configs with all the configs available thru the vendor implementation
        for (i = 0; i < TOTAL_NUMBER_OF_CONFIGS; ++i) {
            // Generate the attribute list
            for ( j = 0; attribList[2*j] != EGL_NONE; ++j) {
                attribList[2*j+1] = eglConfigDescriptors[i].ourConfig[CONFIG_ELEMENT(attribList[2*j])];
                // Use the renderable type to decide which set of vendor functions to use
                if(attribList[2*j] == EGL_RENDERABLE_TYPE){
                    renderableType = attribList[2*j+1];
                }
            }

            // Intialize the vendor config map to its default values
            dpyContext->vendorConfigMap[i].localConfig = (EGLConfigDescriptor*)&eglConfigDescriptors[i];
            dpyContext->vendorConfigMap[i].vendorConfigAvail = FALSE;
            dpyContext->vendorConfigMap[i].vendorConfig = EGL_NO_CONTEXT;

            // Initialize different vendor configs based on the supported rendering api
            if ( renderableType == EGL_OPENVG_BIT ) {
                // Grab Amanith Configs only
                if ( eglChooseConfigMRVL( vDisp->vndrDisplay[EGL_VENDOREX_API_VG], attribList, &tempConfig, 1, &numConfig ) && numConfig == 1 ) {
                    EGLint red = 0, green = 0, blue = 0, alpha = 0;
                    EGLVndrExConfig * vConfig = (EGLVndrExConfig *)kdMalloc( sizeof(EGLVndrExConfig) );
                    PRINT2N("Found Amanith config 0x%08x to match our %d", &eglConfigDescriptors[i], i+1);
                    if (!vConfig) {
                        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));
                        break;
                    }
                    vConfig->vndrApiId = EGL_VENDOREX_API_VG;
                    vConfig->vndrConfig = tempConfig;//(EGLConfig)&eglConfigDescriptors[i];

                    dpyContext->vendorConfigMap[i].localConfig = (EGLConfigDescriptor*)&eglConfigDescriptors[i];
                    dpyContext->vendorConfigMap[i].vendorConfigAvail = TRUE;
                    dpyContext->vendorConfigMap[i].vendorConfig = (EGLConfig)vConfig;

                    eglGetConfigAttribVndr(vDisp, vConfig, EGL_RED_SIZE, &red);
                    eglGetConfigAttribVndr(vDisp, vConfig, EGL_GREEN_SIZE, &green);
                    eglGetConfigAttribVndr(vDisp, vConfig, EGL_BLUE_SIZE, &blue);
                    eglGetConfigAttribVndr(vDisp, vConfig, EGL_ALPHA_SIZE, &alpha);
                    PRINT4N( "RGBA = %d%d%d%d", red, green, blue, alpha );
                } else {
                    PRINT2N("VG: No config to match our %d ( 0x%04x )", i+1, EglVndrExFuncs[EGL_VENDOREX_API_VG].eglGetErrorVndr() );
                }
            } else if (renderableType == EGL_OPENGL_ES_BIT || renderableType == EGL_OPENGL_ES2_BIT) {
                if ( eglChooseConfigVTMV( vDisp->vndrDisplay[EGL_VENDOREX_API_GLES], attribList, &tempConfig, 1, &numConfig ) && (numConfig == 1) ) {
                    EGLVndrExConfig * vConfig = (EGLVndrExConfig *)kdMalloc( sizeof(EGLVndrExConfig) );
                    PRINT2N("Found VT config 0x%08x to match our %d", &eglConfigDescriptors[i], i+1);
                    if (!vConfig) {
                        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));
                        break;
                    }
                    vConfig->vndrApiId = EGL_VENDOREX_API_GLES;
                    vConfig->vndrConfig = tempConfig;//(EGLConfig)&eglConfigDescriptors[i];

                    dpyContext->vendorConfigMap[i].localConfig = (EGLConfigDescriptor*)&eglConfigDescriptors[i];
                    dpyContext->vendorConfigMap[i].vendorConfigAvail = TRUE;
                    dpyContext->vendorConfigMap[i].vendorConfig = (EGLConfig)vConfig;

                    {
                        EGLint value;
                        eglGetConfigAttribVTMV(vDisp->vndrDisplay[EGL_VENDOREX_API_GLES], tempConfig, EGL_CONFIG_ID, &value);
                        PRINT2N("Found native config id %d to match our %d", value, i+1);
                    }
                } else {
                    PRINT2N("EG: No config to match our %d ( 0x%04x )", i+1, EglVndrExFuncs[EGL_VENDOREX_API_GLES].eglGetErrorVndr() );
                }
            } else if (renderableType == (EGL_OPENGL_ES_BIT|EGL_OPENVG_BIT)) {
                WARN("This implementation does not support OpenGL ES and OpenVG configurations");
                kdCatfailRim("Invalid config");
            } else {
                // Grab no vendor configs - default to dummy RIM implementation
                PRINTN("No vendor config to match our %d", i+1);
            }
        }

        // Destroy all other allocated resources if there is not enough memory
        if( (EGLint)(kdGetThreadStorageKHR(eglLastErrorKey)) == EGL_BAD_ALLOC ) {
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
            for(i=0; i<TOTAL_NUMBER_OF_CONFIGS; ++i) {
                if (dpyContext->vendorConfigMap[i].vendorConfig) {
                    kdFree(dpyContext->vendorConfigMap[i].vendorConfig);
                    dpyContext->vendorConfigMap[i].vendorConfig = EGL_NO_CONTEXT;
                }
            }
            retBool = EGL_FALSE;
        }
    }

    return retBool;
}

EglStr eglQueryStringVndrMrvlVtmv( EGLDisplay dpy, EGLint name ) {
    static char strQueried[169];

    PRINT("eglQueryStringVndrMrvlVtmv");

    switch( name ) {
    case EGL_CLIENT_APIS:
        strcpy(strQueried, "OpenVG OpenGL_ES");
        break;
    case EGL_EXTENSIONS:
        strncpy(strQueried, "EGL_KHR_reusable_sync EGL_KHR_image_base EGL_KHR_image_pixmap EGL_KHR_image KHR_gl_texture_2D_image KHR_gl_tex EGL_KHR_config_attribs EGL_KHR_lock_surface EGL_KHR_vg_p", sizeof(strQueried) - 1 );
        break;
    case EGL_VENDOR:
        strcpy(strQueried, "Marvell/Vivante");
        break;
    case EGL_VERSION:
        strcpy(strQueried, "1.4");
        break;
    default:
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_PARAMETER));
        return NULL;
    }

    return strQueried;
}

#endif // defined RIM_EGL_MRVL && defined RIM_EGL_VTMV

#endif // defined(RIM_EGL_VENDOR_EX_ENABLED)
