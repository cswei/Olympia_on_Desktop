/*******************************************************
 *  EGL.c
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

#include "egl_globals.h"

#if !defined (RIM_NDK)
#include <bugdispc.h>
#include <bf_memalloc.h>

#define SRCFILE     FILE_EGL_UTILITY
#define SRCGROUP    GROUP_GRAPHICS

#include <i_lcd.h>
#include <log_verbosity.h>
#include <string.h>
#include <message.h>
#include <device.h>
#include <graphics_mempool.h>
#include <i_graphics_mempool.h>
#include <lcd_if.h>
#include <critical.h>
#endif

#include <Raster.h>
#include <basetype.h>
#include <lcd.h>

#include <egl.h>
#include <i_egl.h>
#include <egl_hw.h>

#if defined (RIM_NDK)
#include <egl_native.h>
#if defined (RIM_EGL_MRVL)
#include <egl_mrvl.h>
#include <eglext_mrvl.h>
#elif defined (RIM_EGL_VTMV)
#include <egl_vtmv.h>
//#include <vtvm_eglext.h>
#endif
#include <eglext.h>
#endif

#if defined(RIM_WINDOW_MANAGER)
#include "windowmanager.h"
#include "i_windowmanager.h"
#endif

#ifdef RIM_USES_QCOMM_GRAPHICS
#include "egl_entry.h"
#endif

#if defined ( RIM_GLES_20 ) || defined (RIM_FLEDGE)
#include "gl_function_table.h"
#include "KHR_thread_storage.h"

extern KDThreadStorageKeyKHR eglCurrentContextKey;


glPipelineId eglQueryGLPipeline()
{

    return ((EGLRenderingContext*)kdGetThreadStorageKHR(eglCurrentContextKey))->pipeline_id;


}
#endif



#if defined (EGL_EXTRA_CONFIG_DEBUG)
void printAllVendorConfigs(EGLDisplay dpy)
{
    EGLConfig *configs = NULL;
    EGLint num_config, value;
#if defined ( RIM_EGL_MRVL )
    if(eglGetConfigsMRVL( dpy, configs, 0, &num_config ) && num_config !=0)
#elif defined ( RIM_USES_QCOMM_GRAPHICS )
    if(qceglGetConfigs( dpy, configs, 0, &num_config ) && num_config !=0)
#elif defined ( RIM_EGL_VTMV )
	if(eglGetConfigsVTMV( dpy, configs, 0, &num_config ) && num_config !=0)
#endif
        configs = (EGLConfig*)kdMalloc(sizeof(EGLConfig) * num_config);
    else
        return;

#if defined ( RIM_EGL_MRVL )
    if(eglGetConfigsMRVL( dpy, configs, num_config, &num_config ) && num_config !=0){
#elif defined ( RIM_USES_QCOMM_GRAPHICS )
    if(qceglGetConfigs( dpy, configs, num_config, &num_config ) && num_config !=0){
#elif defined ( RIM_EGL_VTMV )
    if(eglGetConfigsVTMV( dpy, configs, num_config, &num_config ) && num_config !=0){
#endif
        DWORD i, j;
        EGLint value;
        for(i=0; i< num_config; i++)
        {
            LV2(LOG_EGL, PRINT("--"));
            LV2(LOG_EGL, PRINT("--"));
            LV2(LOG_EGL, PRINTN("Printing attributes for config %d", i));
            for(j=EGL_BUFFER_SIZE; j<=EGL_LAST_CONFIG_ATTRIBUTE; j++){
#if defined ( RIM_EGL_MRVL )
                if(eglGetConfigAttribMRVL( dpy, configs[i], j, &value )){
#elif defined ( RIM_USES_QCOMM_GRAPHICS )
                if(qceglGetConfigAttrib( dpy, configs[i], j, &value )){
#elif defined ( RIM_EGL_VTMV )
                if(eglGetConfigAttribVTMV( dpy, configs[i], j, &value )){
#endif
                    switch(j){
                        case EGL_BUFFER_SIZE:
                            LV2(LOG_EGL, PRINTN("Attribute EGL_BUFFER_SIZE = %04x",value));
                            break;
                        case EGL_ALPHA_SIZE:
                            LV2(LOG_EGL, PRINTN("Attribute EGL_ALPHA_SIZE = %04x",value));
                            break;
                        case EGL_BLUE_SIZE:
                            LV2(LOG_EGL, PRINTN("Attribute EGL_BLUE_SIZE = %04x",value));
                            break;
                        case EGL_GREEN_SIZE:
                            LV2(LOG_EGL, PRINTN("Attribute EGL_GREEN_SIZE = %04x",value));
                            break;
                        case EGL_RED_SIZE:
                            LV2(LOG_EGL, PRINTN("Attribute EGL_RED_SIZE = %04x",value));
                            break;
                        case EGL_DEPTH_SIZE:
                            LV2(LOG_EGL, PRINTN("Attribute EGL_DEPTH_SIZE = %04x",value));
                            break;
                        case EGL_STENCIL_SIZE:
                            LV2(LOG_EGL, PRINTN("Attribute EGL_STENCIL_SIZE = %04x",value));
                            break;
                        case EGL_CONFIG_CAVEAT:
                            switch(value){
                                case EGL_SLOW_CONFIG:
                                    LV2(LOG_EGL, PRINT("Attribute EGL_CONFIG_CAVEAT = EGL_SLOW_CONFIG"));
                                    break;
                                case EGL_NON_CONFORMANT_CONFIG:
                                    LV2(LOG_EGL, PRINT("Attribute EGL_CONFIG_CAVEAT = EGL_NON_CONFORMANT_CONFIG"));
                                    break;
                                case EGL_NONE:
                                    LV2(LOG_EGL, PRINT("Attribute EGL_CONFIG_CAVEAT = EGL_NONE"));
                                    break;
                            }
                            break;
                        case EGL_CONFIG_ID:
                            LV2(LOG_EGL, PRINTN("Attribute EGL_CONFIG_ID = %04x",value));
                            break;
                        case EGL_LEVEL:
                            LV2(LOG_EGL, PRINTN("Attribute EGL_LEVEL = %04x",value));
                            break;
                        case EGL_MAX_PBUFFER_HEIGHT:
                            LV2(LOG_EGL, PRINTN("Attribute EGL_MAX_PBUFFER_HEIGHT = %04x",value));
                            break;
                        case EGL_MAX_PBUFFER_PIXELS:
                            LV2(LOG_EGL, PRINTN("Attribute EGL_MAX_PBUFFER_PIXELS = %04x",value));
                            break;
                        case EGL_MAX_PBUFFER_WIDTH:
                            LV2(LOG_EGL, PRINTN("Attribute EGL_MAX_PBUFFER_WIDTH = %04x",value));
                            break;
                        case EGL_NATIVE_RENDERABLE:
                            switch(value){
                                case EGL_TRUE:
                                    LV2(LOG_EGL, PRINT("Attribute EGL_NATIVE_RENDERABLE = EGL_TRUE"));
                                    break;
                                case EGL_FALSE:
                                    LV2(LOG_EGL, PRINT("Attribute EGL_NATIVE_RENDERABLE = EGL_FALSE"));
                                    break;
                            }
                            break;
                        case EGL_NATIVE_VISUAL_ID:
                            LV2(LOG_EGL, PRINTN("Attribute EGL_NATIVE_VISUAL_ID = %04x",value));
                            break;
                        case EGL_NATIVE_VISUAL_TYPE:
                            LV2(LOG_EGL, PRINTN("Attribute EGL_NATIVE_VISUAL_TYPE = %04x",value));
                            break;
                        case EGL_PRESERVED_RESOURCES:
                            LV2(LOG_EGL, PRINTN("Attribute EGL_PRESERVED_RESOURCES = %04x",value));
                            break;
                        case EGL_SAMPLES:
                            LV2(LOG_EGL, PRINTN("Attribute EGL_SAMPLES = %04x",value));
                            break;
                        case EGL_SAMPLE_BUFFERS:
                            LV2(LOG_EGL, PRINTN("Attribute EGL_SAMPLE_BUFFERS = %04x",value));
                            break;
                        case EGL_SURFACE_TYPE:
                            switch(value){
                                case EGL_PBUFFER_BIT:
                                    LV2(LOG_EGL, PRINT("Attribute EGL_SURFACE_TYPE = EGL_PBUFFER_BIT"));
                                    break;
                                case EGL_PIXMAP_BIT:
                                    LV2(LOG_EGL, PRINT("Attribute EGL_SURFACE_TYPE = EGL_PIXMAP_BIT"));
                                    break;
                                case EGL_WINDOW_BIT:
                                    LV2(LOG_EGL, PRINT("Attribute EGL_SURFACE_TYPE = EGL_WINDOW_BIT"));
                                    break;
                                case (EGL_PBUFFER_BIT | EGL_PIXMAP_BIT):
                                    LV2(LOG_EGL, PRINT("Attribute EGL_SURFACE_TYPE = EGL_PBUFFER_BIT | EGL_PIXMAP_BIT"));
                                    break;
                                case (EGL_PBUFFER_BIT | EGL_WINDOW_BIT):
                                    LV2(LOG_EGL, PRINT("Attribute EGL_SURFACE_TYPE = EGL_PBUFFER_BIT | EGL_WINDOW_BIT"));
                                    break;
                                case (EGL_WINDOW_BIT | EGL_PIXMAP_BIT):
                                    LV2(LOG_EGL, PRINT("Attribute EGL_SURFACE_TYPE = EGL_WINDOW_BIT | EGL_PIXMAP_BIT"));
                                    break;
                                case (EGL_PBUFFER_BIT | EGL_PIXMAP_BIT | EGL_WINDOW_BIT):
                                    LV2(LOG_EGL, PRINT("Attribute EGL_SURFACE_TYPE = EGL_PBUFFER_BIT | EGL_PIXMAP_BIT | EGL_WINDOW_BIT"));
                                    break;
                                case EGL_FALSE:
                                    LV2(LOG_EGL, PRINT("Attribute EGL_SURFACE_TYPE = EGL_FALSE"));
                                    break;
                            }
                            break;
                        case EGL_TRANSPARENT_TYPE:
                            switch(value){
                                case EGL_TRANSPARENT_RGB:
                                    LV2(LOG_EGL, PRINT("Attribute EGL_TRANSPARENT_TYPE = EGL_TRANSPARENT_RGB"));
                                    break;
                                case EGL_NONE:
                                    LV2(LOG_EGL, PRINT("Attribute EGL_TRANSPARENT_TYPE = EGL_NONE"));
                                    break;
                            }
                            break;
                        case EGL_TRANSPARENT_BLUE_VALUE:
                            LV2(LOG_EGL, PRINTN("Attribute EGL_TRANSPARENT_BLUE_VALUE = %04x",value));
                            break;
                        case EGL_TRANSPARENT_GREEN_VALUE:
                            LV2(LOG_EGL, PRINTN("Attribute EGL_TRANSPARENT_GREEN_VALUE = %04x",value));
                            break;
                        case EGL_TRANSPARENT_RED_VALUE:
                            LV2(LOG_EGL, PRINTN("Attribute EGL_TRANSPARENT_RED_VALUE = %04x",value));
                            break;
                        case EGL_BIND_TO_TEXTURE_RGB:
                            switch(value){
                                case EGL_TRUE:
                                    LV2(LOG_EGL, PRINT("Attribute EGL_BIND_TO_TEXTURE_RGB = EGL_TRUE"));
                                    break;
                                case EGL_FALSE:
                                    LV2(LOG_EGL, PRINT("Attribute EGL_BIND_TO_TEXTURE_RGB = EGL_FALSE"));
                                    break;
                            }
                            break;
                        case EGL_BIND_TO_TEXTURE_RGBA:
                            switch(value){
                                case EGL_TRUE:
                                    LV2(LOG_EGL, PRINT("Attribute EGL_BIND_TO_TEXTURE_RGBA = EGL_TRUE"));
                                    break;
                                case EGL_FALSE:
                                    LV2(LOG_EGL, PRINT("Attribute EGL_BIND_TO_TEXTURE_RGBA = EGL_FALSE"));
                                    break;
                            }
                            break;
                        case EGL_MIN_SWAP_INTERVAL:
                            LV2(LOG_EGL, PRINTN("Attribute EGL_MIN_SWAP_INTERVAL = %04x",value));
                            break;
                        case EGL_MAX_SWAP_INTERVAL:
                            LV2(LOG_EGL, PRINTN("Attribute EGL_MAX_SWAP_INTERVAL = %04x",value));
                            break;
                        case EGL_LUMINANCE_SIZE:
                            LV2(LOG_EGL, PRINTN("Attribute EGL_LUMINANCE_SIZE = %04x",value));
                            break;
                        case EGL_ALPHA_MASK_SIZE:
                            LV2(LOG_EGL, PRINTN("Attribute EGL_ALPHA_MASK_SIZE = %04x",value));
                            break;
                        case EGL_COLOR_BUFFER_TYPE:
                            switch(value){
                                case EGL_RGB_BUFFER:
                                    LV2(LOG_EGL, PRINT("Attribute EGL_COLOR_BUFFER_TYPE = EGL_RGB_BUFFER"));
                                    break;
                                case EGL_LUMINANCE_BUFFER:
                                    LV2(LOG_EGL, PRINT("Attribute EGL_COLOR_BUFFER_TYPE = EGL_LUMINANCE_BUFFER"));
                                    break;
                            }
                            break;
                        case EGL_RENDERABLE_TYPE:
                            switch(value){
                                case EGL_OPENGL_ES_BIT:
                                    LV2(LOG_EGL, PRINT("Attribute EGL_RENDERABLE_TYPE = EGL_OPENGL_ES_BIT"));
                                    break;
                                case EGL_OPENVG_BIT:
                                    LV2(LOG_EGL, PRINT("Attribute EGL_RENDERABLE_TYPE = EGL_OPENVG_BIT"));
                                    break;
                                case (EGL_OPENVG_BIT | EGL_OPENGL_ES_BIT):
                                    LV2(LOG_EGL, PRINT("Attribute EGL_RENDERABLE_TYPE = EGL_OPENVG_BIT | EGL_OPENGL_ES_BIT"));
                                    break;
                                case EGL_FALSE:
                                    LV2(LOG_EGL, PRINT("Attribute EGL_RENDERABLE_TYPE = EGL_FALSE"));
                                    break;
                            }
                            break;
                        default:
                            LV2(LOG_EGL, PRINTN("This should not have happened attrib %04x", j));
                            break;
                    }
                }
                else
                    LV2(LOG_EGL, PRINTN("Missing attribute %04x", j));
            }

        }
    }
    else{
        LV2(LOG_EGL, PRINT("Vendor getConfigs failed"));
    }
    kdFree(configs);

}
#endif


