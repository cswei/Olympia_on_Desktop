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
#include <unistd.h>
#include <graphicsinit.h>
#include <sys/mman.h>
#define MMU_PAGE_SIZE 0x1000u
#define MMU_PAGE_MASK (MMU_PAGE_SIZE-1)
#else
#include "bugdispc.h"
#include "bf_memalloc.h"

#define SRCFILE     FILE_EGL
#define SRCGROUP    GROUP_GRAPHICS

#include <log_verbosity.h>
#include <lcd_if.h>
#include <graphics_mempool.h>
#include <i_graphics_mempool.h>
#include <device.h>
#include <i_lcd.h>
#include <critical.h>
#include <i_mmu.h>
#endif

#include <string.h>
#include <basetype.h>
#include <lcd.h>
#include <Raster.h>
#include <i_raster.h>
#include <bitmap.h>


// EGL Header Files
#include <i_egl.h>
#include <egl_globals.h>
#include <egl_hw.h>
#include <nativewin.h>
#include <kd.h>

#if defined(RIM_WINDOW_MANAGER)
#include "windowmanager.h"
#include "i_windowmanager.h"
#endif

#include <KHR_thread_storage.h>
#include <gl_function_table.h>

//NOTE: DECLARE_EGL_CONFIGS should only be defined here
#define DECLARE_EGL_CONFIGS
#include <config_egl.h>

//#define EGL_DEBUG  // turn when debug

#if defined VECTOR_FRAMEWORK_AMANITH
#include "egl_amanith.h"
#endif

#if (defined( RIM_EGL_MRVL ) || \
     defined( RIM_USES_QCOMM_GRAPHICS ) || \
     defined( VECTOR_FRAMEWORK_AMANITH ) || \
     defined( RIM_EGL_VTMV ))

#define EGL_USE_VENDOR_IMPLEMENTATION   1
#endif

#if defined ( RIM_FLEDGE )
#define GRAPHICS_MEMPOOL_SIZE (6*1024*1024)
#elif defined (RIM_USES_QCOMM_GRAPHICS)
#define GRAPHICS_MEMPOOL_SIZE (600*1024)
#else
#define GRAPHICS_MEMPOOL_SIZE (6*1024)
#endif

#if !defined(RIM_NDK)
__align(32) static BYTE graphicsMemoryPool[GRAPHICS_MEMPOOL_SIZE];
#endif

#if defined ( RIM_NESSUS )
extern void TLSTableInit( void );
#endif

/* This portion might stay the same, it doesnt seem that any thread is going to modify this */
// Local display context - One for each display
EGLDisplayContext EglDpyContext[RIM_LCD_NUM_DISPLAYS];
//since we only have one window per display map on-screen rendering contexts to
//each display id
EGLSurfaceDescriptor   *pBufferFrontSurface[RIM_LCD_NUM_DISPLAYS];
EGLRenderingContext    *pBufferFrontContext[RIM_LCD_NUM_DISPLAYS];


//These are the TLS keys used to store thread specific data
KDThreadStorageKeyKHR eglLastErrorKey = 0;
KDThreadStorageKeyKHR eglCurrentApiKey = 0;
KDThreadStorageKeyKHR eglCurrentContextKey = 0;

BOOL eglUseNewChooseConfigVendor = TRUE;

// TODO(BT) - add define to enable code path...
#ifdef RIM_WINDOW_MANAGER
    BOOL EglUseNewSwapLogic = FALSE;
#else
    BOOL EglUseNewSwapLogic = FALSE;
#endif //RIM_WINDOW_MANAGER


//This portion stays the same
#ifndef RIM_WINDOW_MANAGER
// Needed to initialize the globalNativeWindow array
NativeWindow* globalNativeWindows[RIM_LCD_NUM_DISPLAYS][NATIVE_WIN_MAX_WINDOWS];
#endif // RIM_WINDOW_MANAGER

EGLProcessState          EglProcessState = {FALSE, NULL, NULL};

#if defined (RIM_GLES_11) && defined (RIM_GLES_20) && \
            ( defined(_WIN32) || \
              defined(RIM_EGL_VTMV) || \
              defined(RIM_USES_QCOMM_GRAPHICS) )
extern void initGLPipeline( void );
#endif

static void deleteSurfaceBackBuffer(EGLSurfaceDescriptor * surface);

typedef enum {
    EGL_RIM_CMD_POST_SWAP_NORMAL            = 0,
    EGL_RIM_CMD_POST_SWAP_SKIP_FB_DESTROY,
} EglRimCommand;

/* This portion might stay the same, maybe change to const */
static DWORD configSortingOrder[] ={
    EGL_CONFIG_CAVEAT,
    EGL_COLOR_BUFFER_TYPE,
    EGL_NONE, //color depth special case
    EGL_BUFFER_SIZE,
    EGL_SAMPLE_BUFFERS,
    EGL_SAMPLES,
    EGL_DEPTH_SIZE,
    EGL_STENCIL_SIZE,
    EGL_ALPHA_MASK_SIZE,
    EGL_NATIVE_VISUAL_TYPE,
    EGL_CONFIG_ID
};

static EGLConfigContext defaultsEglConfig = {
    0,                      //EGL_BUFFER_SIZE
    0,                      //EGL_ALPHA_SIZE
    0,                      //EGL_BLUE_SIZE
    0,                      //EGL_GREEN_SIZE
    0,                      //EGL_RED_SIZE
    0,                      //EGL_DEPTH_SIZE
    0,                      //EGL_STENCIL_SIZE
    EGL_DONT_CARE,          //EGL_CONFIG_CAVEAT
    EGL_DONT_CARE,          //EGL_CONFIG_ID
    0,                      //EGL_LEVEL
    0,                      //EGL_MAX_PBUFFER_HEIGHT
    0,                      //EGL_MAX_PBUFFER_PIXELS
    0,                      //EGL_MAX_PBUFFER_WIDTH
    EGL_DONT_CARE,          //EGL_NATIVE_RENDERABLE
    0,                      //EGL_NATIVE_VISUAL_ID
    EGL_DONT_CARE,          //EGL_NATIVE_VISUAL_TYPE
    0,                      //RESERVED
    0,                      //EGL_SAMPLES
    0,                      //EGL_SAMPLE_BUFFERS
    EGL_WINDOW_BIT,         //EGL_SURFACE_TYPE
    EGL_NONE,               //EGL_TRANSPARENT_TYPE
    EGL_DONT_CARE,          //EGL_TRANSPARENT_BLUE_VALUE
    EGL_DONT_CARE,          //EGL_TRANSPARENT_GREEN_VALUE
    EGL_DONT_CARE,          //EGL_TRANSPARENT_RED_VALUE
    0,                      //RESERVED
    EGL_DONT_CARE,          //EGL_BIND_TO_TEXTURE_RGB
    EGL_DONT_CARE,          //EGL_BIND_TO_TEXTURE_RGBA
    EGL_DONT_CARE,          //EGL_MIN_SWAP_INTERVAL
    EGL_DONT_CARE,          //EGL_MAX_SWAP_INTERVAL
    0,                      //EGL_LUMINANCE_SIZE
    0,                      //EGL_ALPHA_MASK_SIZE
    EGL_RGB_BUFFER,         //EGL_COLOR_BUFFER_TYPE
    EGL_DONT_CARE,          //EGL_RENDERABLE_TYPE
    EGL_DONT_CARE,          //EGL_CONFORMANT
};

void convertRGBA8888toRGB565(void * inBuff, void *outBuff, int size)
{
    unsigned char *inB, *outB;
    int temp=0;
    int i;
    unsigned char cR, cG, cB;
    inB = (unsigned char *)inBuff;
    outB = (unsigned char*)outBuff;
    for(i=0; i<size; i++) {
        cB = inB[4*i+1];
        cG = inB[4*i+2];
        cR = inB[4*i+3];
        temp = ((cB & 0xFF)>>3) |
                (((cG & 0xFF)>>2)<<5) |
                (((cR & 0xFF)>>3)<<11);
        outB[i*2+1] = ((temp >> 8) & 0xFF);
        outB[i*2] = (temp & 0xFF);
    }
}

void convertRGB565toRGBA8888(void * inBuff, void *outBuff, int size)
{
    unsigned char *inB, *outB;
    int temp=0;
    int i;
    inB = (unsigned char *)inBuff;
    outB = (unsigned char*)outBuff;
    for(i=0; i<size; i++){
        temp = inB[i*2] |((inB[i*2+1] & 0xFF) << 8);
        outB[4*i] = 255;
        outB[4*i+1] = 0xFF & (temp<<3);
        outB[4*i+2] = 0xFF & ((temp>>3) & 0xFC);
        outB[4*i+3] = 0xFF & ((temp>>8) & 0xF8);
    }
}
void eglInitRim(void)
{
    DWORD i;
#ifndef RIM_WINDOW_MANAGER
    DWORD j, k;
#endif // RIM_WINDOW_MANAGER

    PRINT("eglInitRim");

    if (!EglProcessState.IsInitialized) {
        //initialize semaphors
        EglProcessState.SurfaceMutex = kdThreadMutexCreate(NULL);
        EglProcessState.ContextMutex = kdThreadMutexCreate(NULL);

        /* It seems that both posix and nessus do not take this parameter into consideration, so I will use NULL for now */
        eglLastErrorKey = kdMapThreadStorageKHR(NULL);
        eglCurrentApiKey = kdMapThreadStorageKHR(NULL);
        eglCurrentContextKey = kdMapThreadStorageKHR(NULL);

        kdSetThreadStorageKHR(eglCurrentApiKey, (void *)(EGL_BITMAP_API_RIM));
        kdSetThreadStorageKHR(eglCurrentContextKey, (void *)(EGL_NO_CONTEXT));

        for ( i = 0; i < RIM_LCD_NUM_DISPLAYS; i++ ) {
            pBufferFrontSurface[i] = NULL;
            pBufferFrontContext[i] = NULL;
        }

        // Check for config id consistency
        for(i=0; i < TOTAL_NUMBER_OF_CONFIGS; i++){
            if(eglConfigDescriptors[i].ourConfig[CONFIG_ELEMENT(EGL_CONFIG_ID)] != (i+1)){
                WARNN("Please check the EGL Config ID for Config %d",i+1);
                kdCatfailRim ("Inconsistent EGL Config IDs");
            }
        }

        if(TOTAL_NUMBER_OF_CONFIGS > EGL_MAX_CONFIGS){
            WARN("Total number of EGL Configs exceeds EGL_MAX_CONFIGS");
            kdCatfailRim ("Inconsistent number of EGL Configs");
        }

        // Populate the EGLDisplayContext table with valid display ids
        // Populate the primary display, if it exists
        if( RIM_LCD_NUM_DISPLAYS > 0 ){
            DisplayId_t displayId;
            WARN("RIM_LCD_NUM_DISPLAYS > 0");
            if( LcdGetDisplayId( PRIMARY_DISPLAY, &displayId )){
                // Leave the bottom part of the handle as the index of EglDpyContext
                EglDpyContext[0].handle = (EGLDisplay)(0x36360000 | (displayId << 8));
                EglDpyContext[0].id = displayId;
            }else{
                WARN("Primary display not found");
                EglDpyContext[0].handle = EGL_NO_DISPLAY;
                EglDpyContext[0].id = 0xFF;
            }
            EglDpyContext[0].eglInitialized = FALSE;
            EglDpyContext[0].vendorConfigMap = NULL;
            EglDpyContext[0].strQueried[0] = '\0';
            EglDpyContext[0].vendorDisplay = EGL_NO_DISPLAY;
        }

        // Populate the auxillary display, if it exists
        if( RIM_LCD_NUM_DISPLAYS > 1 ){
            DisplayId_t displayId;
            WARN("RIM_LCD_NUM_DISPLAYS > 1");
            if( LcdGetDisplayId( AUXILLARY_DISPLAY, &displayId )){
                // Leave the bottom part of the handle as the index of EglDpyContext
                EglDpyContext[1].handle = (EGLDisplay)(0x36360000 | (displayId << 8) | 1);
                EglDpyContext[1].id = displayId;
#if defined( RIM_LCD_TVOUT )
            }else if( LcdGetDisplayId( TVOUT_DISPLAY, &displayId )){
                // Leave the bottom part of the handle as the index of EglDpyContext
                EglDpyContext[1].handle = (EGLDisplay)(0x36360000 | (displayId << 8) | 1);
                EglDpyContext[1].id = displayId;
#endif
            }else{
                WARN("Display #2 not found");
                EglDpyContext[1].handle = EGL_NO_DISPLAY;
                EglDpyContext[1].id = 0xFF;
            }
            EglDpyContext[1].eglInitialized = FALSE;
            EglDpyContext[1].vendorConfigMap = NULL;
            EglDpyContext[1].strQueried[0] = '\0';
            EglDpyContext[1].vendorDisplay = EGL_NO_DISPLAY;
        }

#ifndef RIM_WINDOW_MANAGER
        // Initialize the globalNativeWindow table
        for(k=0; k < RIM_LCD_NUM_DISPLAYS; k++){
            for(j=0; j < NATIVE_WIN_MAX_WINDOWS; j++){
                globalNativeWindows[k][j] = NULL;
            }
        }
#endif // RIM_WINDOW_MANAGER


#if defined VECTOR_FRAMEWORK_AMANITH
        eglInitAmanith();
#endif

        //Initialize OpenGL pipelines if they are present
#if defined (RIM_GLES_11) && defined (RIM_GLES_20) && \
            ( defined(_WIN32) || \
              defined(RIM_EGL_VTMV) || \
              defined(RIM_USES_QCOMM_GRAPHICS) )
        PRINT("initializing the 3d pipeline");
        initGLPipeline();
#endif

        EglProcessState.IsInitialized = TRUE;
    }

#if defined ( RIM_NESSUS )
#if !defined ( RIM_FLEDGE ) || !defined (RIM_NDK)
    TLSTableInit();
#endif
#endif
    //initialize all vendor functions
    EglSetupFunctions();
}

EGLint eglGetError( void )
{
    EGLint prevError = (EGLint)kdGetThreadStorageKHR(eglLastErrorKey);
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    return prevError;
}

EGLDisplayContext *eglGetDisplayContextInt( EGLDisplay dpy )
{
    // Get the table index from the lower byte
    BYTE index = (BYTE)((DWORD)dpy & 0xFF);
    // Check if the index and the handle are valid
    if(index < RIM_LCD_NUM_DISPLAYS && EglDpyContext[index].handle == dpy){
        return &EglDpyContext[index];
    }
    return NULL;
}

EGLDisplay eglGetDisplay( NativeDisplayType display_id )
{
    EGLDisplay newEglDisplay = EGL_NO_DISPLAY;
    DWORD i;

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    //MALLOC
#if !defined(RIM_NDK)
    if(!RimGraphicsMempoolInit(graphicsMemoryPool,GRAPHICS_MEMPOOL_SIZE)){
       kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));
       return EGL_NO_DISPLAY;
    }
#endif
    // Print which config file we're using
    PRINTCOPY("EGL Configs: %s",eglConfigName);

    // Map the EGL_DEFAULT_DISPLAY to the PRIMARY_DISPLAY
    if( display_id == EGL_DEFAULT_DISPLAY ) {
        LcdGetDisplayId( PRIMARY_DISPLAY, &display_id );
    }
    // Search through the EGLDisplayContext table to find a matching display id
    for(i=0; i < RIM_LCD_NUM_DISPLAYS; i++){
        if(EglDpyContext[i].id == display_id){
            // Grab the EGLDisplay handle
            newEglDisplay = EglDpyContext[i].handle;
            break;
        }
    }
    if(newEglDisplay != EGL_NO_DISPLAY){
#if defined EGL_USE_VENDOR_IMPLEMENTATION
        BYTE dpyIndex = (BYTE)((DWORD)newEglDisplay & 0xFF);
#endif
        PRINT2N("Found 0x%08x for display_id %d",newEglDisplay,display_id);
#if defined EGL_USE_VENDOR_IMPLEMENTATION
        // If a display is found and has not already been initialized, get a vendor display as well
        if( EglDpyContext[dpyIndex].eglInitialized == EGL_FALSE ){
            PRINT("Vndr eglGetDisplay");
            EglDpyContext[dpyIndex].vendorDisplay = eglGetDisplayVndr(display_id);
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVndr()));
            if(EglDpyContext[dpyIndex].vendorDisplay == EGL_NO_DISPLAY){
                // If something goes wrong in the vendor code...
                newEglDisplay = EGL_NO_DISPLAY;
            }
        }
#endif
    }else{
        WARNN("No display found for display_id %d",display_id);
    }
    return newEglDisplay;
}


EGLBoolean eglInitialize( EGLDisplay dpy, EGLint *major, EGLint *minor )
{
    EGLDisplayContext *dpyContext = NULL;
    EGLBoolean retBool = EGL_TRUE;
    DWORD i;

  kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

#if !defined(RIM_NDK)
    if (!RimGraphicsMempoolInit(graphicsMemoryPool,GRAPHICS_MEMPOOL_SIZE)) {
       kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));
       return EGL_FALSE;
    }
#endif

    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglInitialize : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    // Fill major and minor fields if they are not NULL
    if (major != NULL) {
        *major = 1;
    }


    if  ( minor != NULL ) {
        *minor = 4;
    }

    // Check if the display has already been initialized
    if(!dpyContext->eglInitialized){
        // Initialize the display context
        // Try to allocate the vendor config map
        dpyContext->vendorConfigMap = (EGLLocalConfig*)kdMalloc(sizeof(EGLLocalConfig) * TOTAL_NUMBER_OF_CONFIGS);

        if(!dpyContext->vendorConfigMap){
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));
            return EGL_FALSE;
        }
        memset(dpyContext->vendorConfigMap, 0, sizeof(EGLLocalConfig) * TOTAL_NUMBER_OF_CONFIGS);

        PRINT("Initializing Config Map");
        for (i=0; i < TOTAL_NUMBER_OF_CONFIGS; i++) {
            dpyContext->vendorConfigMap[i].localConfig = (EGLConfigDescriptor*)&eglConfigDescriptors[i];
            dpyContext->vendorConfigMap[i].vendorConfigAvail = FALSE;
            dpyContext->vendorConfigMap[i].vendorConfig = EGL_NO_CONFIG;
        }

#if defined ( EGL_USE_VENDOR_IMPLEMENTATION )
        PRINT("Vndr eglInitialize");
        // Use the vendor wrapper function to initialize the display and the vendor map
        retBool = eglInitializeVndr( dpy, major, minor );
        if( retBool == EGL_TRUE ) {
            // mark it initialized if the vendor returned successfully
            dpyContext->eglInitialized = EGL_TRUE;
        } else {
            // Otherwise if anything went wrong free the vendor config map
            kdFree(dpyContext->vendorConfigMap);
            dpyContext->vendorConfigMap = NULL;
        }

#else
        // No vendor implementation, the config map was already initialized for this case.
        dpyContext->eglInitialized = EGL_TRUE;
#endif
    }

    return retBool;
}

EGLBoolean eglTerminate( EGLDisplay dpy )
{
    EGLDisplayContext *dpyContext = NULL;
    EGLBoolean retVal = EGL_TRUE;

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglTerminate : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    // Check if the display has been initialized
    if(dpyContext->eglInitialized){
        // TODO According to the spec we should implement a catch all case where eglTerminate
        // destroys all contexts and surfaces associated with the display
        // This will involve keeping track of all contexts and surfaces created for a display

#if defined ( EGL_USE_VENDOR_IMPLEMENTATION )
        PRINT("Vndr eglTerminate");
        retVal = eglTerminateVndr( dpyContext->vendorDisplay );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVndr()));
#endif
        // Only mark it terminated if the vendor returned successfully or if there is no vendor
        if(retVal == EGL_TRUE){
            dpyContext->eglInitialized = FALSE;
            dpyContext->vendorDisplay = EGL_NO_DISPLAY;
            // Free the vendor config map
            if(dpyContext->vendorConfigMap){
                kdFree( dpyContext->vendorConfigMap );
                dpyContext->vendorConfigMap = NULL;
            }
        }
    }
    return retVal;
}

const char *eglQueryString( EGLDisplay dpy, EGLint name )
{
    EGLDisplayContext *dpyContext = NULL;

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglQueryString : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return NULL;
    }

    if(!dpyContext->eglInitialized){
        WARN( "eglQueryString : display not initialized" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return NULL;
    }

#if defined ( EGL_USE_VENDOR_IMPLEMENTATION )
    PRINT("eglQueryString");

    return eglQueryStringVndr( dpyContext->vendorDisplay, name );

#else
    switch( name ) {
        case EGL_CLIENT_APIS:
            kdStrcpy_s(dpyContext->strQueried, EGL_STR_QUERIED_SZ_BYTES,
                                                                   "RimBitmap");
            break;
        case EGL_EXTENSIONS:
            dpyContext->strQueried[0] = '\0';
            break;
        case EGL_VENDOR:
            kdStrcpy_s(dpyContext->strQueried, EGL_STR_QUERIED_SZ_BYTES, "RIM");
            break;
        case EGL_VERSION:
            kdStrcpy_s(dpyContext->strQueried, EGL_STR_QUERIED_SZ_BYTES, "1.4");
            break;
        default:
            break;
    }
    return dpyContext->strQueried;

#endif
}

EGLBoolean eglBindAPI( EGLenum api )
{
    EGLBoolean retBool = EGL_TRUE;
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

#if !defined(RIM_NDK)
    if(!RimGraphicsMempoolInit(graphicsMemoryPool,GRAPHICS_MEMPOOL_SIZE)){
       kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));
       return EGL_FALSE;
    }
#endif

    switch( api ) {
        case EGL_BITMAP_API_RIM:
#if defined RIM_OPENVG
        case EGL_OPENVG_API:
#endif

#if defined RIM_OPENGLES || defined FEATURE_GRAPHICS_OPENGLES_CL
        case EGL_OPENGL_ES_API:
#endif
            LV1(LOG_EGL ,PRINTN("Setting EGL current API to 0x%04x", api));
            kdSetThreadStorageKHR(eglCurrentApiKey, (void *) api);
            break;
        default:
            WARNN("Setting EGL current API to unsupported 0x%04x", api);
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_PARAMETER));
            return EGL_FALSE;
    }
#if defined ( EGL_USE_VENDOR_IMPLEMENTATION )

    if((EGLenum)kdGetThreadStorageKHR(eglCurrentApiKey) == EGL_OPENVG_API || (EGLenum)kdGetThreadStorageKHR(eglCurrentApiKey) == EGL_OPENGL_ES_API){
        retBool = eglBindAPIVndr( api );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVndr()));
    }
#endif
    return retBool;
}

EGLenum eglQueryAPI( void )
{
    return (EGLenum)kdGetThreadStorageKHR(eglCurrentApiKey);
}

#if defined(RIM_WINDOW_MANAGER)

static BOOL EglIntIsValidBufferPtr( BYTE * ptr ) {
    // BT - once we're using the single KD memory interface, then we'll
    // only need to check against NULL
    return (ptr != NULL && ptr != (BYTE *)-1);
}

static BOOL EglIsAllocatingBackBufferForDriver() {
    BOOL ret = FALSE;

    if ( (EGLenum)kdGetThreadStorageKHR(eglCurrentApiKey) == EGL_OPENGL_ES_API ) {
    #if defined( RIM_EGL_VTMV ) && !defined( VIVANTE_NO_3D )
        ret = TRUE;
    #else
        ret = FALSE;
    #endif
    }

    return ret;
}

static BOOL EglIntPrepareResizeFrontBuffer( EGLDisplay dpy, EGLSurfaceDescriptor * surfaceDesc, DWORD newWidth, DWORD newHeight ) {

    BOOL ret = FALSE;
    DWORD bufferSize = 0;

    if ( surfaceDesc ) {

        // cleanup if data pending...
        if ( EglIntIsValidBufferPtr(surfaceDesc->resizeFrontBuffer.data) ) {
            PRINT3N( "EglIntPrepareResizeFrontBuffer: free old FB buffer (%d,%d) 0x%08x", surfaceDesc->resizeFrontBuffer.wide, surfaceDesc->resizeFrontBuffer.high, surfaceDesc->resizeFrontBuffer.data );

            bufferSize = surfaceDesc->resizeFrontBuffer.stride * surfaceDesc->resizeFrontBuffer.high;
            kdMunmapRim( surfaceDesc->resizeFrontBuffer.data, bufferSize);
        }

        memcpy(&surfaceDesc->resizeFrontBuffer, &surfaceDesc->frontBuffer, sizeof(surfaceDesc->resizeFrontBuffer));
        surfaceDesc->resizeFrontBuffer.wide = newWidth;
        surfaceDesc->resizeFrontBuffer.high = newHeight;
        surfaceDesc->resizeFrontBuffer.stride = (( (newWidth * (surfaceDesc->bitDepth / 8)) + 7 ) & ~7);
        bufferSize = surfaceDesc->resizeFrontBuffer.stride * surfaceDesc->resizeFrontBuffer.high;
        surfaceDesc->resizeFrontBuffer.data = kdMmapRim( bufferSize );

        if ( EglIntIsValidBufferPtr(surfaceDesc->resizeFrontBuffer.data) ) {
            PRINT3N( "EglIntPrepareResizeFrontBuffer: new FB buffer (%d,%d) 0x%08x", surfaceDesc->resizeFrontBuffer.wide, surfaceDesc->resizeFrontBuffer.high, surfaceDesc->resizeFrontBuffer.data );
            memset(surfaceDesc->resizeFrontBuffer.data, 0x0, bufferSize);
            ret = TRUE;
        } else {
            WARN("EglIntPrepareResizeFrontBuffer : unable to alloc FB buffer");
            memset( &surfaceDesc->resizeFrontBuffer, 0x0, sizeof(BitMap) );
        }
    }

    return ret;
}

static BOOL EglIntResizeBackBuffer( EGLDisplay dpy, EGLSurface surface, DWORD newWidth, DWORD newHeight ) {
    EGLSurface  retSurf;
    EGLBoolean  retValue;
    EGLint      config_id;
    BOOL        createVendorSurface = FALSE;
    DWORD       oldWidth;
    DWORD       oldHeight;

    EGLSurfaceDescriptor * surfaceDesc = (EGLSurfaceDescriptor *)surface;
    EGLBoolean eglCurrent              = EGL_FALSE;
    EGLContext  eglContext             = EGL_NO_CONTEXT;
    EGLDisplayContext *dpyContext      = NULL;

    dpyContext   = eglGetDisplayContextInt( dpy );
    oldWidth = surfaceDesc->width;
    oldHeight = surfaceDesc->height;

    eglQuerySurface( dpy,  surface, EGL_CONFIG_ID, &config_id);
    PRINT4N("EglIntResizeBackBuffer: (%d,%d)->(%d,%d)", oldWidth, oldHeight, newWidth, newHeight);

    // 1. vendor specific prep work
    if( surfaceDesc->vendorSurface != EGL_NO_SURFACE ) {
        // 1a. unbind context - if applicable
        if( surfaceDesc->boundToContext ) {
            eglCurrent = EGL_TRUE;
            eglContext = kdGetThreadStorageKHR(eglCurrentContextKey);
            eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        }

        // 1b. destroy vendor surface (if applicable)
        createVendorSurface = TRUE;

        PRINT( "Vdr eglDestroySurface" );
        retValue = eglDestroySurfaceVndr( dpyContext->vendorDisplay, surfaceDesc->vendorSurface );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVndr()));

        if( retValue ) {
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
        } else {
            WARN( "Vendor eglDestroySurface failed" );
            PRINTN( "Error 0x%04x", (EGLint)kdGetThreadStorageKHR(eglLastErrorKey));
            return FALSE;
        }

        //if ( EglIntIsValidBufferPtr(surfaceDesc->backBuffer.data) ) {
            //ASSERTC( "EglIntResizeBackBuffer: should own back buffer!", surfaceDesc->ownsBackBuffer );
        //}
    }

    // 3. destroy back buffer surface
    if ( EglIntIsValidBufferPtr(surfaceDesc->backBuffer.data) ) {
        deleteSurfaceBackBuffer( surfaceDesc );
    }

    // 4. create back buffer surface (new dimensions)
    if ( surfaceDesc->ownsBackBuffer ) {
        DWORD backBufferSize;
        surfaceDesc->backBuffer.wide = newWidth;
        surfaceDesc->backBuffer.high = newHeight;
        surfaceDesc->backBuffer.stride = (( (newWidth * (surfaceDesc->bitDepth / 8)) + 7 ) & ~7);
        backBufferSize = surfaceDesc->backBuffer.stride * surfaceDesc->backBuffer.high;
        surfaceDesc->backBuffer.data = kdMmapRim( backBufferSize);
        if ( !EglIntIsValidBufferPtr(surfaceDesc->backBuffer.data) ) {
            WARN("EglIntResizeBackBuffer : unable to alloc backbuffer");
            return FALSE;
        }

        // 4a. create front buffer surface (new dimensions)
        if ( createVendorSurface ) {
            // TODO(BT) - we create the new sized FB here because the Vivante driver will initialize a
            // newly created window the address of both the BB and FB (performance hack) and I'm
            // side-stepping the hack by having the new sized FB linger till the next eglSwapBuffers().
            // Make sure the GL surfaces on the device function once GL resizing works as expected (it
            // currently hangs) - Nov. 18, 2010
            if ( !EglIntPrepareResizeFrontBuffer( dpy, surfaceDesc, newWidth, newHeight ) ) {
                kdMunmapRim( surfaceDesc->backBuffer.data, backBufferSize);
                memset( &surfaceDesc->backBuffer, 0x0, sizeof(surfaceDesc->backBuffer) );
                return FALSE;
            }
        }
    }

    // 5. surface is now new dimensions
    surfaceDesc->width = newWidth;
    surfaceDesc->height = newHeight;

    // 6. create vendor surface
    if ( createVendorSurface ) {
        retSurf = eglCreateWindowSurfaceVndr(dpyContext->vendorDisplay, dpyContext->vendorConfigMap[config_id-1].vendorConfig,
                surfaceDesc->nativeWindow, NULL);
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVndr()));

        if(retSurf != EGL_NO_SURFACE){
            PRINT5N("vndr eglCreateWindowSurface success 0x%04x (%d,%d)->(%d,%d)",(EGLint)kdGetThreadStorageKHR(eglLastErrorKey), surfaceDesc->width, surfaceDesc->height, newWidth, newHeight);
            surfaceDesc->vendorSurface = retSurf;
            eglUpdateVendorSurface(surfaceDesc);
        } else {
            PRINTN("vndr eglCreateWindowSurface failed 0x%04x",(EGLint)kdGetThreadStorageKHR(eglLastErrorKey));
            surfaceDesc->width = oldWidth;
            surfaceDesc->height = oldHeight;
            return EGL_FALSE;
        }
    }

    // 7. restore context - if applicable
    if( eglCurrent ) {
        if(eglMakeCurrent(dpy, surface, surface, eglContext) != EGL_TRUE || eglGetError() != EGL_SUCCESS){
            WARN("eglResizeWindowSurfaceRim: can not make current");
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CURRENT_SURFACE));
            return EGL_FALSE;
        }
    }

    return EGL_TRUE;
}    


// EglIntPreSwap - returns TRUE if FrontBuffer has been resized, FALSE otherwise...
static EGLBoolean EglIntPreSwap( EGLDisplay dpy, EGLSurface surface ) {
    EGLBoolean retResized = FALSE;
    EGLBoolean success = TRUE;

    EGLSurfaceDescriptor * surfaceDesc = (EGLSurfaceDescriptor *)surface;
    if( !surfaceDesc ) {
        WARN("EglIntPreSwap: no surface");
        success = EGL_FALSE;
    }

    // only for window surfaces
    if ( surfaceDesc->surfaceType != EGL_WINDOW_BIT ) {
        success = EGL_FALSE;
    }

    if ( success ) {

        if ( surfaceDesc->width != surfaceDesc->frontBuffer.wide || surfaceDesc->height != surfaceDesc->frontBuffer.high ) {
            // detected size change - see if need to create a new FB buffer
            if ( !EglIntIsValidBufferPtr(surfaceDesc->resizeFrontBuffer.data) ) {
                if ( !EglIntPrepareResizeFrontBuffer( dpy, surfaceDesc, surfaceDesc->width, surfaceDesc->height ) ) {
                    success = FALSE;
                }
            }
        }

        if ( success && EglIntIsValidBufferPtr(surfaceDesc->resizeFrontBuffer.data) ) {
            // swap old and new FB...
            BitMap tempBitmap;

            PRINT4N( "EglIntPreSwap: swapping FB (%d,%d)->(%d,%d)", surfaceDesc->frontBuffer.wide, surfaceDesc->frontBuffer.high,
                surfaceDesc->resizeFrontBuffer.wide, surfaceDesc->resizeFrontBuffer.high );
            PRINT2N( "EglIntPreSwap: swapping FB 0x%08x -> 0x%08x", surfaceDesc->frontBuffer.data, surfaceDesc->resizeFrontBuffer.data );

            memcpy( &tempBitmap, &surfaceDesc->frontBuffer, sizeof(BitMap) );
            memcpy( &surfaceDesc->frontBuffer, &surfaceDesc->resizeFrontBuffer, sizeof(BitMap) );
            memcpy( &surfaceDesc->resizeFrontBuffer, &tempBitmap, sizeof(BitMap) );

            retResized = EGL_TRUE;
        }
    }

    return retResized;
}

static EGLBoolean EglIntPostSwap( EGLDisplay dpy, EGLSurface surface, DWORD newWidth, DWORD newHeight, EglRimCommand command ) {
    EGLBoolean success = EGL_TRUE;

    EGLSurfaceDescriptor * surfaceDesc = (EGLSurfaceDescriptor *)surface;
    if( !surfaceDesc ) {
        WARN("EglIntPostSwap: no surface");
        success = EGL_FALSE;
    }

    // only for window surfaces
    if ( surfaceDesc->surfaceType != EGL_WINDOW_BIT ) {
        success = EGL_FALSE;
    }

    if ( success ) {

        if ( command != EGL_RIM_CMD_POST_SWAP_SKIP_FB_DESTROY && EglIntIsValidBufferPtr(surfaceDesc->resizeFrontBuffer.data) ) {
            // destroy old FB...
            const DWORD oldFrontBufferSize = surfaceDesc->resizeFrontBuffer.stride * surfaceDesc->resizeFrontBuffer.high;

            PRINTN("EglIntPostSwap: destroy old FB 0x%08x", surfaceDesc->resizeFrontBuffer.data );

            kdMunmapRim( surfaceDesc->resizeFrontBuffer.data, oldFrontBufferSize);
            memset(&surfaceDesc->resizeFrontBuffer, 0x0, sizeof(surfaceDesc->resizeFrontBuffer));
        }

        if ( surfaceDesc->width != newWidth || surfaceDesc->height != newHeight ) {

            if ( !EglIntResizeBackBuffer(dpy, surface, newWidth, newHeight) ) {
                // clean up due to failure...
                success = FALSE;
            }
        }
    }

	return success;
}
#endif // defined(RIM_WINDOW_MANAGER) 

EGLBoolean eglSwapBuffers( EGLDisplay dpy, EGLSurface surface )
{
#if !defined( RIM_WINDOW_MANAGER ) && !defined( RIM_NDK )
    LcdConfig *lcdCfg = NULL;
    WORD *lcdBuffer;
    DWORD lcdStride;
    DWORD lcdFormat;
#endif

    EGLDisplayContext *dpyContext = NULL;
#if !defined(RIM_NDK) || defined(RIM_WINDOW_MANAGER)
    EGLBoolean retVal = EGL_FALSE;
    DWORD *backBuffer;
    DWORD backBitDepth;
    DWORD backPixelStride;
#ifdef RIM_WINDOW_MANAGER
    DWORD *frontBuffer;
    BYTE *tempBuffer;
    WMError_t result;
    DWORD frontBitDepth;
    DWORD frontPixelStride;
    SDWORD temp[4];
    Rect dirtyRect;
    SDWORD windowWidth;
    SDWORD windowHeight;
    SDWORD winSize[2];
#endif // RIM_WINDOW_MANAGER
#endif

    EGLSurfaceDescriptor * surfaceDesc = (EGLSurfaceDescriptor *)surface;
    if( NULL == surfaceDesc ) {
        WARN("eglSwapBuffers: no surface");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglSwapBuffers : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

#if !defined( RIM_WINDOW_MANAGER ) && !defined( RIM_NDK )
    lcdCfg = LcdGetLcdConfigPtr( dpyContext->id );
#endif

#if defined(RIM_NDK) && !defined(RIM_WINDOW_MANAGER)
    //This is temp solution.
    //TODO: remove this once window manager is hooked up
    if (surfaceDesc->surfaceType == EGL_WINDOW_BIT)
    {
        WARN("eglSwapBuffers is not supported in usermode.");
    }
    return EGL_FALSE;
#else

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    LV2( LOG_EGL,
         PRINT2N("eglswapbuffers surface = 0x%x, display = 0x%x",
         surface, dpy ));

    if(!dpyContext->eglInitialized){
        LV1( LOG_EGL,WARN( "eglSwapBuffers : display not initialized" ) );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_FALSE;
    }

    if( !surface ) {
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

#ifdef RIM_WINDOW_MANAGER
    WMGetWindowPropertyiv( surfaceDesc->nativeWindow, WM_PROPERTY_SIZE, winSize );
    windowWidth  = winSize[0];
    windowHeight = winSize[1];
#endif // RIM_WINDOW_MANAGER

    // if the current rendering surface is a special type of pBuffer that is equal
    // to the front buffer then swap it directly to the LCD.  This is a special
    // case that should eventually be retired gracefully
    if ( surfaceDesc->nativeRenderable ) {
        LV1( LOG_EGL, PRINT("Surface type is native renderable") );

        // *** Keeping legacy code separate ***
        if ( pBufferFrontSurface[dpyContext->id] == surface ) {
#ifndef RIM_WINDOW_MANAGER
            // get bit depth of back buffer
            backBitDepth = BitMapGetProperty(surfaceDesc->backBuffer.bType, BMP_BPP);
            if (backBitDepth == 0) {
                kdCatfailRim ("No bit depth for EGL back buffer");
                return EGL_FALSE;
            }

            // get back buffer and calculate pixels per row
            backBuffer = (DWORD*)surfaceDesc->backBuffer.data;
            backPixelStride = surfaceDesc->backBuffer.stride / (backBitDepth / 8);

            LV1( LOG_EGL, PRINT("Swapping Pixmaps or FRONT Pbuffers"));
            //need to do color conversion
            //for now we only support RGB888 to RGB565
            LcdGetBuffer( dpyContext->id, LCD_FRONT_BUFFER, &lcdBuffer, &lcdFormat, &lcdStride );
            if( surfaceDesc->bitDepth == 32){
                LV1( LOG_EGL, PRINT("Converting colors"));
                RasterRGBA8888ToRGB565Copy( backBuffer, backPixelStride, 0,
                    lcdBuffer, lcdCfg->width, 0,
                    lcdCfg->width, lcdCfg->height );
            } else{
                if( lcdBuffer != ((WORD*)backBuffer) ) {
                    LV1( LOG_EGL, PRINT("Copying buffers"));

                    RasterCopyC16( (WORD*)backBuffer, backPixelStride, 0,
                                   lcdBuffer, lcdCfg->width, 0,
                                   lcdCfg->width, lcdCfg->height );
                } else {
                    LV1( LOG_EGL,
                    PRINT("Color buffer is LCD FRONT BUFFER. No copy needed"));
                }
            }

            LV2( LOG_EGL,
                 PRINT4N( "Swap buffers updating LCD %d %d %d %d",
                 surfaceDesc->dirtyRegion.x,
                 surfaceDesc->dirtyRegion.y,
                 surfaceDesc->dirtyRegion.width,
                 surfaceDesc->dirtyRegion.height ) );

            LcdUpdateInt( dpyContext->id, surfaceDesc->dirtyRegion.x,
                surfaceDesc->dirtyRegion.y,
                surfaceDesc->dirtyRegion.width,
                surfaceDesc->dirtyRegion.height );

            retVal = EGL_TRUE;
#endif // !RIM_WINDOW_MANAGER

        } else if (surfaceDesc->surfaceType == EGL_WINDOW_BIT){
#ifdef RIM_WINDOW_MANAGER
            // check swap behavior of surface
            if (surfaceDesc->swapBehavior == EGL_BUFFER_PRESERVED) {
                EGLBoolean fbResized = EGL_FALSE;
                PRINT("surfaceDesc->swapBehavior == EGL_BUFFER_PRESERVED");
                
                fbResized = EglIntPreSwap( dpy, surface );
                if ( !fbResized ) {
                    result = WMIntLockWindowBuffer( surfaceDesc->nativeWindow, &surfaceDesc->frontBuffer );
                    if ( WM_E_OK != result ) {
                        WARN("eglSwapBuffers: bad native window");
                        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_NATIVE_WINDOW));
                        return EGL_FALSE;
                    }
                }

                // get bit depth of back buffer
                backBitDepth = BitMapGetProperty(surfaceDesc->backBuffer.bType, BMP_BPP);

                if (backBitDepth == 0) {
                    if ( !fbResized ) {
                        WMIntUnlockWindowBuffer(surfaceDesc->nativeWindow);
                    }
                    kdCatfailRim ("No bit depth for EGL back buffer");
                    return EGL_FALSE;
                }

                // get back buffer and calculate pixels per row
                backBuffer = (DWORD*)surfaceDesc->backBuffer.data;
                backPixelStride = surfaceDesc->backBuffer.stride / (backBitDepth / 8);

                // get bit depth of front buffer
                frontBitDepth = BitMapGetProperty(surfaceDesc->frontBuffer.bType, BMP_BPP);
                if (frontBitDepth == 0) {
                    if ( !fbResized ) {
                        WMIntUnlockWindowBuffer(surfaceDesc->nativeWindow);
                    }
                    kdCatfailRim ("No bit depth for EGL front buffer");
                    return EGL_FALSE;
                }

                // get front buffer and calculate pixels per row
                frontBuffer = (DWORD*)surfaceDesc->frontBuffer.data;
                frontPixelStride = surfaceDesc->frontBuffer.stride / (frontBitDepth / 8);

                // can only trust the dirty region from WMan when the width/height are identical
                if ( windowWidth == surfaceDesc->width && windowHeight == surfaceDesc->height ) {
                    // get native window dirty region
                    result = WMGetWindowPropertyiv(surfaceDesc->nativeWindow, WM_PROPERTY_DIRTY_REGION, temp);
                    if (result != WM_E_OK) {
                        WARN("eglSwapBuffers: bad native window");
                        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_NATIVE_WINDOW));
                        if ( !fbResized ) {
                            WMIntUnlockWindowBuffer(surfaceDesc->nativeWindow);
                        }
                        return EGL_FALSE;
                    }

                    dirtyRect.x = temp[0];
                    dirtyRect.y = temp[1];
                    dirtyRect.width = temp[2];
                    dirtyRect.height = temp[3];
                } else {
                    // we're in the middle of resizing - just refresh the entire surface for now until we update
                    // WMan w/ new FB (w/ new size)
                    dirtyRect.x = 0;
                    dirtyRect.width = surfaceDesc->width;
                    dirtyRect.y = 0;
                    dirtyRect.height = surfaceDesc->height;
                }

                LV1( LOG_EGL,PRINT("Copy back buffer"));
                if (surfaceDesc->bitDepth == 32) {
                    // Copy from the back colour buffer at the correct y position 
                    RasterCopyC32( backBuffer + dirtyRect.y * backPixelStride, backPixelStride, dirtyRect.x,
                            frontBuffer + dirtyRect.y * frontPixelStride, frontPixelStride, dirtyRect.x,
                            dirtyRect.width, dirtyRect.height );
                } else { 
                    // Copy from the back colour buffer at the correct y position
                    RasterCopyC16( ((WORD*)backBuffer) + dirtyRect.y * backPixelStride, backPixelStride, dirtyRect.x,
                            ((WORD*)frontBuffer) + dirtyRect.y * frontPixelStride, frontPixelStride, dirtyRect.x,
                            dirtyRect.width, dirtyRect.height );
                }

                if ( !fbResized ) {
                    WMIntUnlockWindowBuffer(surfaceDesc->nativeWindow);
                } else {
                    WMIntSetWindowBuffer(surfaceDesc->nativeWindow, &surfaceDesc->frontBuffer, WM_BUFFER_STATE_VALID);
                }
                retVal = EGL_TRUE;

            } else {

                PRINT("surfaceDesc->swapBehavior != EGL_BUFFER_PRESERVED");

                PRINT("flip front and back buffers");
                // flip front and back buffers
                EglIntPreSwap( dpy, surface );
                tempBuffer = surfaceDesc->backBuffer.data;
                surfaceDesc->backBuffer.data = surfaceDesc->frontBuffer.data;
                surfaceDesc->frontBuffer.data = tempBuffer;

                result = WMIntSetWindowBuffer(surfaceDesc->nativeWindow,
                        &surfaceDesc->frontBuffer, WM_BUFFER_STATE_VALID);
                if( WM_E_OK != result) {
                    WARN("eglSwapBuffers: set window buffer failed");
                    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_NATIVE_WINDOW));
                    return EGL_FALSE;
                }

            }

            EglIntPostSwap( dpy, surfaceDesc, windowWidth, windowHeight, EGL_RIM_CMD_POST_SWAP_NORMAL );

#else
            DWORD lcdXPos = surfaceDesc->nativeWindow->windowSize.x + surfaceDesc->nativeWindow->clipRect.x;
            DWORD lcdYPos = lcdCfg->height - surfaceDesc->nativeWindow->windowSize.y - surfaceDesc->nativeWindow->clipRect.y
                            - surfaceDesc->nativeWindow->clipRect.height;
            DWORD windowXPos = surfaceDesc->nativeWindow->clipRect.x;
            DWORD windowYPos = surfaceDesc->nativeWindow->windowSize.height - surfaceDesc->nativeWindow->clipRect.y
                               - surfaceDesc->nativeWindow->clipRect.height;

            // get bit depth of back buffer
            backBitDepth = BitMapGetProperty(surfaceDesc->backBuffer.bType, BMP_BPP);

            if (backBitDepth == 0) {
                kdCatfailRim ("No bit depth for EGL back buffer");
                return EGL_FALSE;
            }

            // get back buffer and calculate pixels per row
            backBuffer = (DWORD*)surfaceDesc->backBuffer.data;
            backPixelStride = surfaceDesc->backBuffer.stride / (backBitDepth / 8);

            LV1(LOG_EGL, PRINT4N( "Swap buffers updating LCD %d %d %d %d", lcdXPos, lcdYPos,
                surfaceDesc->nativeWindow->clipRect.width, surfaceDesc->nativeWindow->clipRect.height ));

            LcdGetBuffer( dpyContext->id, LCD_FRONT_BUFFER, &lcdBuffer, &lcdFormat, &lcdStride);

            if( surfaceDesc->bitDepth == 32) {
                LV1( LOG_EGL,PRINT("Converting colors"));
                // Adjust the front buffer to the correct y position
                lcdBuffer += lcdYPos*lcdCfg->width;
                // Copy and convert from the colour buffer at the correct y position
                RasterARGB8888ToRGB565Copy( backBuffer + windowYPos * backPixelStride, backPixelStride, windowXPos,
                    lcdBuffer, lcdCfg->width, lcdXPos,
                    surfaceDesc->nativeWindow->clipRect.width, surfaceDesc->nativeWindow->clipRect.height );
            } else {
                if ( lcdBuffer != ((WORD*)backBuffer) ) {
                    LV1( LOG_EGL,PRINT("Copying buffers"));
                    // Adjust the front buffer to the correct y position
                    lcdBuffer += lcdYPos*lcdCfg->width;
                    // Copy from the colour buffer at the correct y position
                    RasterCopyC16( ((WORD*)backBuffer) + windowYPos * backPixelStride, backPixelStride, windowXPos,
                        lcdBuffer, lcdCfg->width, lcdXPos,
                        surfaceDesc->nativeWindow->clipRect.width, surfaceDesc->nativeWindow->clipRect.height );
                } else {
                    LV1( LOG_EGL,PRINT("Color buffer is LCD FRONT BUFFER. No copy needed"));
                }
            }
            LcdUpdateInt( dpyContext->id, lcdXPos, lcdYPos,
                surfaceDesc->nativeWindow->clipRect.width, surfaceDesc->nativeWindow->clipRect.height );
#endif // RIM_WINDOW_MANAGER

            retVal = EGL_TRUE;
        }

    } else {
        LV2( LOG_EGL, PRINT(" call eglSwapBuffersVndr") );
#ifdef RIM_WINDOW_MANAGER
        if ( EglIntPreSwap( dpy, surface ) ) {
            // TODO: RESIZING: the frontbuffer has been resized.  Ideally you will also not pass the new buffer to 
            // the WM until after you've swapped into it, otherwise artifacts will appear.  As a first pass, to 
            // get things working you could pass the new buffer to the WM (you may want to do a copy from the old 
            // frontbuffer to the new frontbuffer so there will at least be some valid content in the buffer).
            // Once the first pass is working, you can check it in, but then you should do some refactoring of the
            // vendor swap functions so that you don't need to pass the new buffer to the WM until after the backbuffer
            // has been copied into it.  This basically means ensuring that the vendor code never has any calls to
            // WMIntLockWindowBuffer or WMIntUnlockWindowBuffer in it.  Instead, abstract this code into one location
            // (in this file?) and when the vendor wants to lock the frontbuffer, only call the WM APIs if resizing is
            // not occurring, otherwise just return the newly allocated frontbuffer instead.
            WMIntSetWindowBuffer(surfaceDesc->nativeWindow, &surfaceDesc->frontBuffer, WM_BUFFER_STATE_VALID);
        }
#endif // RIM_WINDOW_MANAGER

        retVal = eglSwapBuffersVndr( dpy, surface );
        if( retVal ) {
        #ifdef RIM_WINDOW_MANAGER

            if ( EglUseNewSwapLogic ) {
                kdThreadMutexLock(EglProcessState.SurfaceMutex);
                    tempBuffer = surfaceDesc->backBuffer.data;
                    surfaceDesc->backBuffer.data = surfaceDesc->frontBuffer.data;
                    surfaceDesc->frontBuffer.data = tempBuffer;
                    WMIntSetWindowBuffer(surfaceDesc->nativeWindow, &surfaceDesc->frontBuffer, WM_BUFFER_STATE_VALID);
                kdThreadMutexUnlock(EglProcessState.SurfaceMutex);
            }

            EglIntPostSwap( dpy, surfaceDesc, windowWidth, windowHeight, EGL_RIM_CMD_POST_SWAP_NORMAL );
        #endif // RIM_WINDOW_MANAGER
        } else {
            WARN( "eglSwapBuffersVndr failed!" );
        }
    }
    return retVal;
#endif
}

EGLBoolean eglCopyBuffers(EGLDisplay dpy, EGLSurface surface,
                          NativePixmapType target)
{
    EGLBoolean retBool = EGL_TRUE;
    EGLDisplayContext *dpyContext = NULL;
    EGLSurfaceDescriptor *surfaceDesc = (EGLSurfaceDescriptor *)surface;

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN("eglCopyBuffers: bad display");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    if(!dpyContext->eglInitialized){
        WARN("eglCopyBuffers: display not initialized");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_FALSE;
    }

    if(!surfaceDesc){
        WARNN("eglCopyBuffers: surface 0x%08x",surface);
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    if(target == NULL){
        WARNN("eglCopyBuffers: target 0x%08x",target);
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_PARAMETER));
        return EGL_FALSE;
    }

    // Check the NativePixmapType fields
    if(target->data == NULL || target->wide == 0 || target->high == 0 || target->stride == 0){
        WARNN( "eglCopyBuffers: target 0x%08x",target);
        WARN4N("                wide 0x%04x high 0x%04x stride 0x%04x data 0x%08x",target->wide,target->high,target->stride,target->data);
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_NATIVE_PIXMAP));
        return EGL_FALSE;
    }

    if(surfaceDesc->vendorSurface){
        retBool = eglCopyBuffersVndr(dpy, surface, target);
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVndr()));
    }else{
        // RIM Implementation
        if(surfaceDesc->backBuffer.data){ 
            // 32 bit back buffer is ARGB8888, 16bit back buffer is RGB565
            if(surfaceDesc->bitDepth == 32){

                switch( target->bType ) {
                    case BMT_16BPP_RGB565:
                        RasterARGB8888ToRGB565Copy( (DWORD *)surfaceDesc->backBuffer.data, surfaceDesc->width, 0,
                            (WORD*)target->data, target->stride/sizeof(WORD), 0,
                            (target->wide > surfaceDesc->width) ? surfaceDesc->width : target->wide,
                            (target->high > surfaceDesc->height) ? surfaceDesc->height : target->high );
                        break;
                    case BMT_32BPP_XRGB8888:
                    case BMT_32BPP_ARGB8888:
                    case BMT_32BPP_ARGB8888_PMA:
                        RasterCopyC32( (DWORD *)surfaceDesc->backBuffer.data, surfaceDesc->width, 0,
                            (DWORD*)target->data, target->stride/sizeof(DWORD), 0,
                            (target->wide > surfaceDesc->width) ? surfaceDesc->width : target->wide,
                            (target->high > surfaceDesc->height) ? surfaceDesc->height : target->high );
                        break;
                    default:
                        WARNN("Unsupported target bitmap type for eglCopyBuffers: %x", target->bType);
                        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
                        retBool = EGL_FALSE;
                        break;
                }
            }else if(surfaceDesc->bitDepth == 16){

                switch( target->bType ) {
                    case BMT_16BPP_RGB565:
                        RasterCopyC16( (WORD *)surfaceDesc->backBuffer.data, surfaceDesc->width, 0,
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
                        WARNN("Unsupported target bitmap type for eglCopyBuffers: %x", target->bType);
                        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
                        retBool = EGL_FALSE;
                        break;
                }
                    
            }else{
                WARNN("Unknown bit depth %d",surfaceDesc->bitDepth);
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
                retBool = EGL_FALSE;
            }
        }else{
            WARNN( "eglCopyBuffers: buffer 0x%08x",surfaceDesc->backBuffer.data);
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
            retBool = EGL_FALSE;
        }
    }
    return retBool;
}

EGLBoolean eglWaitClient( void )
{
#if defined ( EGL_USE_VENDOR_IMPLEMENTATION  )
    EGLBoolean retBool;
    PRINT("Vndr eglWaitClient");
    retBool = eglWaitClientVndr();
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVndr()));
    return retBool;
#else
    return EGL_TRUE;
#endif
}

EGLBoolean eglGetConfigs( EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config )
{
    EGLDisplayContext *dpyContext = NULL;
    DWORD   i;

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    if ( num_config == NULL ) {
        WARN("eglGetConfigs : num_config is NULL");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_PARAMETER));
        return EGL_FALSE;
    }

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglGetConfigs : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    if(!dpyContext->eglInitialized){
        WARN( "eglGetConfigs : display not initialized" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_FALSE;
    }

    *num_config = 0;

    for (i=1; i<=TOTAL_NUMBER_OF_CONFIGS; i++){
        if(dpyContext->vendorConfigMap[i-1].localConfig->displayMask & DISPLAY(dpyContext->id)){
            PRINT2N("Selecting config %d for display %d", i, dpyContext->id);
            if(configs != NULL){
                configs[*num_config] = (EGLConfig) i;
            }
            (*num_config)++;
            if(*num_config == config_size)
                    return EGL_TRUE;
        }
    }
    return EGL_TRUE;
}

void initializeSurface( EGLDisplay dpy, EGLSurfaceDescriptor *newEglSurface, BYTE surfaceType )
{
    LcdConfig *lcdCfg;
    EGLDisplayContext *dpyContext = NULL;

    if( NULL == newEglSurface ) {
        WARN("initializeSurface: NULL surface");
        return;
    }

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if( !dpyContext ) {
        WARN("initializeSurface : bad display");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return;
    }

    PRINTN("Initializing surface 0x%x", (DWORD)newEglSurface);

    //zero out all surface parameters
    memset( newEglSurface, 0, sizeof(EGLSurfaceDescriptor) );

    newEglSurface->surfaceType = surfaceType;
    newEglSurface->nativeWindow = NULL;

#ifdef RIM_WINDOW_MANAGER
    newEglSurface->swapBehavior = EGL_BUFFER_PRESERVED;
#endif // RIM_WINDOW_MANAGER

    //initialize dirty region
    lcdCfg = LcdGetLcdConfigPtr( dpyContext->id );

    PRINT4N("Initializing dirty region to %d %d %d %d", 0, 0,
        lcdCfg->width, lcdCfg->height );
    newEglSurface->dirtyRegion.x = 0;
    newEglSurface->dirtyRegion.y = 0;
    newEglSurface->dirtyRegion.width = lcdCfg->width;
    newEglSurface->dirtyRegion.height = lcdCfg->height;


}

EGLBoolean eglGetConfigAttrib( EGLDisplay dpy, EGLConfig config, EGLint attribute,
                              EGLint *value )
{
    EGLDisplayContext *dpyContext = NULL;
    EGLLocalConfig *currentLocalConfig = NULL;
    EGLBoolean retBool;
    DWORD config_id = (DWORD)config;

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglGetConfigAttrib : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    if(!dpyContext->eglInitialized){
        WARN( "eglGetConfigAttrib : display not initialized" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_FALSE;
    }

    // Check if this is a valid EGL config
    if( config_id<1 || config_id>TOTAL_NUMBER_OF_CONFIGS){
        WARN ("eglGetConfigAttrib : invalid config");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONFIG));
        return EGL_FALSE;
    }

    if( attribute < EGL_BUFFER_SIZE || attribute > EGL_CONFORMANT ||
        attribute == EGL_NONE || attribute == EGL_PRESERVED_RESOURCES ){
        WARN ("eglGetConfigAttrib : egl bad attribute");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ATTRIBUTE));
        return EGL_FALSE;
    }

    // Get the EGLLocalConfig from the display's vendor mapping
    currentLocalConfig = &dpyContext->vendorConfigMap[config_id-1];

    if(!(currentLocalConfig->localConfig->displayMask & DISPLAY(dpyContext->id))){
        WARN ("eglGetConfigAttrib : egl bad display");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        retBool = EGL_FALSE;
        return retBool;
    }

    PRINT2N("Getting attribute 0x%04x for config %d", attribute, (DWORD)config);

    if(currentLocalConfig->vendorConfigAvail){
#if defined ( EGL_USE_VENDOR_IMPLEMENTATION )
        switch(attribute){

            // Handle a few config attributes ourselves
            case EGL_CONFIG_ID:
                *value = currentLocalConfig->localConfig->ourConfig[CONFIG_ELEMENT(attribute)];
                return EGL_TRUE;
            case EGL_LEVEL:
                *value = currentLocalConfig->localConfig->ourConfig[CONFIG_ELEMENT(attribute)];
                return EGL_TRUE;
            case EGL_NATIVE_RENDERABLE:
                *value = currentLocalConfig->localConfig->ourConfig[CONFIG_ELEMENT(attribute)];
                return EGL_TRUE;
            case EGL_SURFACE_TYPE:
                *value = currentLocalConfig->localConfig->ourConfig[CONFIG_ELEMENT(attribute)];
                return EGL_TRUE;
            case EGL_BIND_TO_TEXTURE_RGB:
                *value = currentLocalConfig->localConfig->ourConfig[CONFIG_ELEMENT(attribute)];
                return EGL_TRUE;
            case EGL_BIND_TO_TEXTURE_RGBA:
                *value = currentLocalConfig->localConfig->ourConfig[CONFIG_ELEMENT(attribute)];
                return EGL_TRUE;
            case EGL_RENDERABLE_TYPE:
                *value = currentLocalConfig->localConfig->ourConfig[CONFIG_ELEMENT(attribute)];
                return EGL_TRUE;

            // Otherwise use the vendor function
            default:
                PRINT("Vndr eglGetConfigAttrib");
                retBool = eglGetConfigAttribVndr(dpyContext->vendorDisplay, currentLocalConfig->vendorConfig, attribute, value);
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVndr()));
                return retBool;
        }
#else
        retBool = EGL_FALSE;
        return retBool;
#endif

    }else if( CONFIG_ELEMENT(attribute) >= EGL_NUM_OF_CONFIG_ATTRIBUTES ) {
        WARNN("eglGetConfigAttrib : egl bad attribute 0x%04x", attribute);
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ATTRIBUTE));
        return EGL_FALSE;
    }else{
        *value = currentLocalConfig->localConfig->ourConfig[CONFIG_ELEMENT(attribute)];
    }
    retBool = EGL_TRUE;
    return retBool;
}

//config comparators
typedef BOOL (*COMPARATOR_FUNCPTR)(EGLConfigContext *configMap, DWORD firstInd, DWORD secInd, DWORD attr);

static BOOL genComp(EGLConfigContext *configMap, DWORD firstConf, DWORD secConf, DWORD attrib) {
    BOOL ret = FALSE;
    EGLConfigContextPtr configContext1 = &configMap[firstConf-1][0];
    EGLConfigContextPtr configContext2 = &configMap[secConf-1][0];

    if ( configContext1 && configContext2 ) {
        ret = configContext1[CONFIG_ELEMENT(attrib)] > configContext2[CONFIG_ELEMENT(attrib)];
    } else {
        WARN2N( "genComp invalid ptrs: p1=0x%08x p2=0x%08x", configContext1, configContext2 );
    }

    return ret;
}

static BOOL colComp(EGLConfigContext *configMap, DWORD firstConf, DWORD secConf, DWORD attrib) {
    BOOL ret = FALSE;
    EGLConfigContextPtr configContext1 = &configMap[firstConf-1][0];
    EGLConfigContextPtr configContext2 = &configMap[secConf-1][0];

    if ( configContext1 && configContext2 ) {
        const DWORD colBufType = configContext1[CONFIG_ELEMENT(EGL_COLOR_BUFFER_TYPE)];
    if(colBufType == EGL_RGB_BUFFER){
            DWORD firstColorSum, secColorSum;
            firstColorSum = configContext1[CONFIG_ELEMENT(EGL_RED_SIZE)] + configContext1[CONFIG_ELEMENT(EGL_GREEN_SIZE)] + configContext1[CONFIG_ELEMENT(EGL_BLUE_SIZE)];
            secColorSum = configContext2[CONFIG_ELEMENT(EGL_RED_SIZE)] + configContext2[CONFIG_ELEMENT(EGL_GREEN_SIZE)] + configContext2[CONFIG_ELEMENT(EGL_BLUE_SIZE)];
            ret = (firstColorSum > secColorSum);
    }
    else{
            ret = (configContext1[CONFIG_ELEMENT(EGL_LUMINANCE_SIZE)] + configContext1[CONFIG_ELEMENT(EGL_ALPHA_SIZE)])
                >
                  (configContext2[CONFIG_ELEMENT(EGL_LUMINANCE_SIZE)] + configContext2[CONFIG_ELEMENT(EGL_ALPHA_SIZE)]);
    }
    } else {
        WARN2N( "colComp invalid ptrs: p1=0x%08x p2=0x%08x", configContext1, configContext2 );
    }

    return ret;
}

static void qsortConf(EGLConfigContext *configMap, DWORD * array, DWORD startInd, DWORD endInd, DWORD attrib, COMPARATOR_FUNCPTR comparator)
{
    //for now this algorithm is a classical in-place quicksort
    DWORD tempWord, pivValue, placeInd, i;
    //PRINT2N("qsortConf from %d to %d", startInd, endInd);
    //PRINTN("attrib %d", attrib);
    if(startInd >= endInd)
        return;
    if((endInd-startInd) == 1)
        if(comparator(configMap, array[startInd], array[endInd], attrib)){ //startInd > endInd
            //PRINT2N("Only Two: swapping %d to %d", startInd, endInd);
            tempWord = array[startInd];
            array[startInd] = array[endInd];
            array[endInd] = tempWord;
        }
    pivValue = array[endInd];
    placeInd = startInd;
    for( i = startInd; i<endInd; i++){
        //PRINTN("inspecting %d", i);
        if(!comparator(configMap, array[i], pivValue, attrib)){
            if(i!=placeInd){
                //PRINT2N("swapping %d to %d", i, placeInd);
                tempWord = array[i];
                array[i] = array[placeInd];
                array[placeInd] = tempWord;
            }
            placeInd++;
        }
    }
    tempWord = array[placeInd];
    array[placeInd] = array[endInd];
    array[endInd] = tempWord;

    if(placeInd > startInd)
        qsortConf(configMap, array, startInd, placeInd-1, attrib, comparator);
    if(placeInd<endInd)
        qsortConf(configMap, array, placeInd+1, endInd, attrib, comparator);
}

static void sortConfigs(EGLConfigContext *configMap, DWORD * array, DWORD startInd, DWORD endInd, DWORD sortBy){
    //we need to sort configs in the priority, specified in the EGL
    //specification
    DWORD i = 0;
    DWORD prevConfInd;
    COMPARATOR_FUNCPTR compFunc;
    PRINTN("Sorting by attribute 0x%04x", sortBy);
    PRINT2N("Index %d to %d", startInd, endInd);
    if(startInd >= endInd)
        return;
    if(sortBy < 11 && sortBy!=2){
        qsortConf(configMap, array, startInd, endInd, configSortingOrder[sortBy], genComp);
        compFunc = genComp;
    }
    else if(sortBy == 2){
        qsortConf(configMap, array, startInd, endInd, configSortingOrder[sortBy-1], colComp);
        compFunc = colComp;
    }
    else{
        WARN("Configs cannot be sorted by this attribute");
        return;
    }

    prevConfInd = startInd;
    for(i=startInd; i<=endInd; i++){
         if(compFunc(configMap, array[i], array[prevConfInd], configSortingOrder[sortBy])) {
            if(prevConfInd < i-1)
                sortConfigs(configMap, array, prevConfInd, i-1, sortBy+1);
            prevConfInd = i;
         }
    }
    if(prevConfInd < endInd)
        sortConfigs(configMap, array, prevConfInd, endInd, sortBy+1);
}

BOOL EglIntDoesAttribMatchConfig( EGLConfigContextPtr configContext, const DWORD attribute, const EGLint attributeValue, const EGLint attributeIndex ) {
    BOOL match = TRUE;

    switch ( attribute ) {
        //'Exact' parametrs
        case EGL_BIND_TO_TEXTURE_RGB:
        case EGL_BIND_TO_TEXTURE_RGBA:
        case EGL_COLOR_BUFFER_TYPE:
        case EGL_CONFIG_CAVEAT:
        case EGL_CONFIG_ID:
        case EGL_LEVEL:
        case EGL_MIN_SWAP_INTERVAL:
        case EGL_MAX_SWAP_INTERVAL:
        case EGL_NATIVE_RENDERABLE:
        case EGL_NATIVE_VISUAL_TYPE:
        case EGL_TRANSPARENT_TYPE:
        case EGL_TRANSPARENT_BLUE_VALUE:
        case EGL_TRANSPARENT_GREEN_VALUE:
        case EGL_TRANSPARENT_RED_VALUE:
            if(attributeValue != configContext[CONFIG_ELEMENT(attribute)]){
                match = FALSE;
                PRINT2N("Failed Exact attribute %d 0x%x", attributeIndex, attribute);
            }
            break;
        //'AtLeast' parametrs
        case EGL_BUFFER_SIZE:
        case EGL_RED_SIZE:
        case EGL_GREEN_SIZE:
        case EGL_BLUE_SIZE:
        case EGL_LUMINANCE_SIZE:
        case EGL_ALPHA_SIZE:
        case EGL_ALPHA_MASK_SIZE:
        case EGL_DEPTH_SIZE:
        case EGL_SAMPLE_BUFFERS:
        case EGL_SAMPLES:
        case EGL_STENCIL_SIZE:
            if(attributeValue > configContext[CONFIG_ELEMENT(attribute)]){
                match = FALSE;
                PRINT2N("Failed AtLeast attribute %d 0x%x", attributeIndex, attribute);
            }
            break;

        //'Mask' Parametrs
        case EGL_RENDERABLE_TYPE:
        case EGL_SURFACE_TYPE:
        //case EGL_CONFORMANT:  // only used to filter out vendor configs
            PRINT2N("Attribute = 0x%x, value = %d", attribute, attributeValue);
            if((attributeValue & (configContext[CONFIG_ELEMENT(attribute)])) != attributeValue){
                match = FALSE;
                PRINTN("Failed Mask attribute %d", attributeIndex);
            }

            break;

        default:
            break;
}

    return match;
}

EGLBoolean eglChooseConfig( EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs,
                           EGLint config_size, EGLint *num_config )
{
    DWORD i,j;
    EGLDisplayContext *dpyContext = NULL;
    EGLConfigContextPtr currentConfigContext;
    EGLConfigContext compareContext={0};
    DWORD tempConf[EGL_MAX_CONFIGS];
    EGLConfigContext configMap[EGL_MAX_CONFIGS];
    EGLint conf_count=0;
    DWORD attribute;
    EGLint attribute_value;
    BOOL match;

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    if(num_config == NULL){
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_PARAMETER));
        return EGL_FALSE;
    }

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglChooseConfig : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    if(!dpyContext->eglInitialized){
        WARN( "eglChooseConfig : display not initialized" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_FALSE;
    }

    PRINT("eglChooseConfig is trying to match attribute list to our own config");
    *num_config = 0;
    match = TRUE;
    // Construct the selection criterias based on the passed parameters and
    // the default values as specified by the EGL spec
    for(i=0; i<EGL_NUM_OF_CONFIG_ATTRIBUTES; i++ ) {
        compareContext[i] = defaultsEglConfig[i];
    }

    // Parse the attribute list only if there is one
    if(attrib_list){
        for(i=0; attrib_list[i] != EGL_NONE; i+=2 ) {
            if(attrib_list[i] < EGL_BUFFER_SIZE || attrib_list[i] > EGL_PBUFFER_SOURCE_RIM){
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ATTRIBUTE));
                return EGL_FALSE;
            }
            compareContext[CONFIG_ELEMENT(attrib_list[i])] = attrib_list[i+1];
        }
    }

    for (j=1; j<=TOTAL_NUMBER_OF_CONFIGS; j++){
        memcpy( &configMap[j-1][0], &dpyContext->vendorConfigMap[j-1].localConfig->ourConfig[0], sizeof(configMap[j-1]) );
    }

    for (j=1; j<=TOTAL_NUMBER_OF_CONFIGS; j++){

        PRINTN("Parsing config %d", j );

        if(dpyContext->vendorConfigMap[j-1].localConfig->displayMask & DISPLAY(dpyContext->id)){
            currentConfigContext = configMap[j-1];
            match = TRUE;
            for(i=0; i<EGL_NUM_OF_CONFIG_ATTRIBUTES && match; i++) {
                attribute = CONFIG_ATTRIBUTE(i);
                attribute_value = compareContext[i];

                if(attribute_value == EGL_DONT_CARE){
                    PRINTN("Ignoring DONT_CARE attribute %d", i);
                    continue;
                }

                if ( EglIntDoesAttribMatchConfig( currentConfigContext, attribute, attribute_value, i ) ) {
                    match = TRUE;
                } else {
                            match = FALSE;
                        }
            }
            if(match){
                PRINTN("Found matching EGL config %d", j);
                tempConf[conf_count++] = j;
            }
        }
    }
    if (conf_count!=0){
        sortConfigs(configMap, tempConf, 0, conf_count-1, 0);
        // Output sorted configs
        PRINT("Configs sorted in the following order");
        for(i=0; i<conf_count; i++)
            PRINTN("Config #%d", tempConf[i]);
        *num_config = conf_count;
        // If the user is requesting configs, then fill them in
        if(configs){
            if(conf_count > config_size)
                conf_count = config_size;
            *num_config = conf_count;
            conf_count--;
            for(; conf_count>=0; conf_count--){
                configs[conf_count] = (EGLConfig)(tempConf[conf_count]);
            }
        }
    }
    else{
        PRINT("No matching configs found");
    }
    return EGL_TRUE;
}

EGLBoolean eglGenericChooseConfigVendor( EGLDisplay dpy, EGLDisplay vendorDpy, EGLint *attrib_list, 
                                 EGL_GET_CONFIGS_FUNCPTR getConfigsFunc, EGL_GET_CONFIG_ATTRIB_FUNCPTR getConfigAttribFunc, 
                                 EGL_ALLOC_CONFIG_CALLBACK allocConfigFunc, EGL_FREE_CONFIG_CALLBACK freeConfigFunc )
{
    DWORD i, j, k;
    EGLDisplayContext *dpyContext = NULL;
    EGLConfigContextPtr currentConfigContext;
    EGLConfigContext compareContext={0};
    DWORD * tempConf = NULL;
    EGLint conf_count=0;
    DWORD attribute;
    EGLint attribute_value;
    BOOL match;
    EGLConfigContext * configMap = NULL;
    EGLConfig * vendorConfigs = NULL;
    EGLint numVendorConfigs = 0;
    BOOL success = TRUE;

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglGenericChooseConfigVendor : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    PRINT("eglGenericChooseConfigVendor is trying to match attribute list to our own config");
    match = TRUE;

    // Ensure attribute list exists + valid
    if (attrib_list) {
        for(i = 0; attrib_list[i] != EGL_NONE; i += 2 ) {
            if(attrib_list[i] < EGL_BUFFER_SIZE || attrib_list[i] > EGL_CONFORMANT){
                WARN2N( "eglGenericChooseConfigVendor : bad attribs, attrib_list[%d]=%d", i, attrib_list[i] );
                success = FALSE;
                break;
            }
        }
    } else {
        success = FALSE;
    }

    if ( !getConfigsFunc || !getConfigAttribFunc ) {
        WARN( "eglGenericChooseConfigVendor : no config func pointers?!" );
        success = FALSE;
    }

    // fill up vendor config descriptors
    if(success && getConfigsFunc( vendorDpy, vendorConfigs, 0, &numVendorConfigs ) && numVendorConfigs !=0) {
        vendorConfigs = (EGLConfig*)kdMalloc(sizeof(EGLConfig) * numVendorConfigs);
        configMap = (EGLConfigContext *)kdMalloc(sizeof(EGLConfigContext) * numVendorConfigs);
        tempConf = (DWORD *)kdMalloc(sizeof(DWORD) * numVendorConfigs);
        if ( !vendorConfigs || !configMap || !tempConf ) {
            success = FALSE;
        }

        if(success && getConfigsFunc( vendorDpy, vendorConfigs, numVendorConfigs, &numVendorConfigs ) && numVendorConfigs !=0) {
            DWORD i, j;
            EGLint value;
            for(i = 0; i < numVendorConfigs; ++i) {
                currentConfigContext = configMap[i];

                for(j = EGL_BUFFER_SIZE; j <= EGL_LAST_CONFIG_ATTRIBUTE; ++j) {
                    switch(j) {
                        case EGL_PRESERVED_RESOURCES:
                        case EGL_NONE:
                        case EGL_MATCH_NATIVE_PIXMAP:
                            // non-queryable attribs
                            break;
                        default:
                            if(getConfigAttribFunc( vendorDpy, vendorConfigs[i], j, &value )) {
                                currentConfigContext[ CONFIG_ELEMENT(j) ] = value;
                            }
                            break;
                    }
                }
            }
        } else {
            success = FALSE;
        }
    }
    else {
        success = FALSE;
    }

    if ( vendorConfigs && configMap && tempConf ) {
        for (i = 0; success && i < TOTAL_NUMBER_OF_CONFIGS; ++i) {
            BOOL findVendorConfig = FALSE;
            EGLint conformantBits = 0x0;

            // Construct the selection criterias based on the passed parameters and
            // the default values as specified by the EGL spec
            for(j = 0; j < EGL_NUM_OF_CONFIG_ATTRIBUTES; ++j ) {
                compareContext[ j ] = defaultsEglConfig[ j ];
            }

            //generate the attribute list
            for(j = 0; attrib_list[j] != EGL_NONE; j += 2 ) {
                const EGLint attribIndex = CONFIG_ELEMENT(attrib_list[j]);

                // SPECIAL CASE: eglConfigDescriptors doesn't have a "conformant" field
                if ( attrib_list[j] == EGL_CONFORMANT ) {
                    conformantBits = attrib_list[j + 1];
                    compareContext[ attribIndex ] = conformantBits;
                } else {
                    compareContext[ attribIndex ] = eglConfigDescriptors[i].ourConfig[ attribIndex ];
                }

                // When the renderable type is 0, don't bother looking for a vendor config
                if(attrib_list[j] == EGL_RENDERABLE_TYPE && compareContext[ attribIndex ] != 0 ) {
                    if ( compareContext[ attribIndex ] & attrib_list[j + 1] ) {
                        findVendorConfig = TRUE;
                    }
                }
            }

            conf_count = 0;
            memset( tempConf, 0x0, sizeof(tempConf[0]) * numVendorConfigs );

            for (j = 1; findVendorConfig && j <= numVendorConfigs; ++j) {
                PRINTN("Parsing config %d", j );

                currentConfigContext = configMap[j - 1];
                match = TRUE;
                for(k = 0; k < EGL_NUM_OF_CONFIG_ATTRIBUTES && match; ++k) {
                    attribute = CONFIG_ATTRIBUTE(k);
                    attribute_value = compareContext[k];

                    if(attribute_value == EGL_DONT_CARE){
                        continue;
                    }

                    if ( EglIntDoesAttribMatchConfig( currentConfigContext, attribute, attribute_value, k ) ) {
                        match = TRUE;
                    } else {
                        match = FALSE;
                    }
                }

                if (match) {
                    PRINTN("Found matching EGL config %d", j);
                    tempConf[conf_count++] = j;
                }
            }

            if ( conf_count > 0 ) {
                EGLConfig vendorConfig = EGL_NO_CONFIG;
                EGLint renderableType = 0;

                sortConfigs(configMap, tempConf, 0, conf_count-1, 0);

                if ( dpyContext->vendorConfigMap[i].vendorConfigAvail ) {
                    WARNN( "Already have a vendor config for %d - choosing twice for same client API?", i+1 );
                } else {
                    PRINT3N("Found vendor config %d (0x%x) to match our %d", tempConf[0], (DWORD)vendorConfigs[ tempConf[0] - 1 ], i+1);
                    dpyContext->vendorConfigMap[i].vendorConfigAvail = TRUE;

                    renderableType = dpyContext->vendorConfigMap[i].localConfig->ourConfig[CONFIG_ELEMENT(EGL_RENDERABLE_TYPE)];
                    vendorConfig = vendorConfigs[ tempConf[0] - 1 ];
                    if ( allocConfigFunc ) {
                        vendorConfig = allocConfigFunc( renderableType, vendorConfigs[ tempConf[0] - 1 ] );
                        if ( vendorConfig == EGL_NO_CONFIG ) {
                            success = FALSE;
                            break;
                        }
                    }

                    // bind RIM config to Vendor config
                    dpyContext->vendorConfigMap[i].vendorConfig = vendorConfig;

                    {
                        EGLint red = 0, green = 0, blue = 0, alpha = 0;
                        eglGetConfigAttribVndr(dpyContext->vendorDisplay, vendorConfig, EGL_RED_SIZE, &red);
                        eglGetConfigAttribVndr(dpyContext->vendorDisplay, vendorConfig, EGL_GREEN_SIZE, &green);
                        eglGetConfigAttribVndr(dpyContext->vendorDisplay, vendorConfig, EGL_BLUE_SIZE, &blue);
                        eglGetConfigAttribVndr(dpyContext->vendorDisplay, vendorConfig, EGL_ALPHA_SIZE, &alpha);
                        PRINT4N( "RGBA = %d%d%d%d", red, green, blue, alpha );
                    }
                }
            } else if ( findVendorConfig ) {
                PRINT2N( "No vendor config to match our %d (conf. bits = 0x%0x)", i+1, conformantBits );
            }
        }
    }

    // Destroy all other allocated resources if there is not enough memory
    if( (EGLint)(kdGetThreadStorageKHR(eglLastErrorKey)) == EGL_BAD_ALLOC ) {
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        for(i=0; i<TOTAL_NUMBER_OF_CONFIGS; ++i) {
            if ( dpyContext->vendorConfigMap[i].vendorConfig ) {
                if ( freeConfigFunc ) {
                    freeConfigFunc( dpyContext->vendorConfigMap[i].vendorConfig );
                }
                dpyContext->vendorConfigMap[i].vendorConfig = EGL_NO_CONTEXT;
            }
        }
        success = EGL_FALSE;
    }

    kdFree( tempConf );
    kdFree( configMap );
    kdFree( vendorConfigs );

    return success;
}

EGLBoolean eglQuerySurface(EGLDisplay dpy, EGLSurface surface,
                           EGLint attribute, EGLint *value)
{
    LcdConfig *lcdCfg = NULL;
    EGLSurfaceDescriptor * surfaceDesc = (EGLSurfaceDescriptor *)surface;
    EGLDisplayContext *dpyContext = NULL;

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglQuerySurface : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    if(!dpyContext->eglInitialized){
        WARN( "eglQuerySurface : display not initialized" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_FALSE;
    }

    if( surface == EGL_NO_SURFACE ){
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    lcdCfg = LcdGetLcdConfigPtr( dpyContext->id );

    if ( surfaceDesc->vendorSurface ) {
        EGLBoolean retBool;
        PRINT("vendor eglQuerySurface");
        switch(attribute) {
            case EGL_CONFIG_ID:
                *value = surfaceDesc->ourConfigId;
                break;
            case EGL_HORIZONTAL_RESOLUTION:
                if(surfaceDesc->surfaceType == EGL_WINDOW_BIT) {
                    *value = lcdCfg->widthInMicrons * 1000;
                    }
                else {
                    *value = EGL_UNKNOWN;
                    }
                break;
             case EGL_VERTICAL_RESOLUTION:
                if(surfaceDesc->surfaceType == EGL_WINDOW_BIT) {
                    *value = lcdCfg->heightInMicrons * 1000;
                    }
                else {
                    *value = EGL_UNKNOWN;
                    }
                break;
            case EGL_PIXEL_ASPECT_RATIO:
                if(surfaceDesc->surfaceType == EGL_WINDOW_BIT)
                    *value = (lcdCfg->heightInMicrons != 0) ?
                        (lcdCfg->widthInMicrons/lcdCfg->heightInMicrons) : 0;
                else
                    *value = EGL_UNKNOWN;
                break;
            default:
                retBool = eglQuerySurfaceVndr(dpyContext->vendorDisplay, surfaceDesc->vendorSurface, attribute, value);
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVndr()));
                return retBool;
        }
        return EGL_TRUE;
    } else {
        PRINT("RIM eglQuerySurface");
        switch(attribute) {
            case EGL_ALPHA_FORMAT:
                *value = surfaceDesc->alphaFormat;
                break;
            case EGL_COLORSPACE:
                *value = surfaceDesc->colorspace;
                break;
            case EGL_CONFIG_ID:
                *value = surfaceDesc->ourConfigId;
                break;
            case EGL_HEIGHT:
                *value = surfaceDesc->height;
                break;

            case EGL_LARGEST_PBUFFER:
                //Not implemented -> returns default
                if(surfaceDesc->surfaceType == EGL_PBUFFER_BIT)
                    *value = EGL_FALSE;
                break;
            case EGL_MIPMAP_TEXTURE:
                //Not implemented -> returns default
                if(surfaceDesc->surfaceType == EGL_PBUFFER_BIT)
                    *value = EGL_FALSE;
                break;
            case EGL_MIPMAP_LEVEL:
                //Not implemented -> returns default
                if(surfaceDesc->surfaceType == EGL_PBUFFER_BIT)
                    *value = 0;
                break;
            case EGL_MULTISAMPLE_RESOLVE:
                //Not implemented -> returns default
                *value = EGL_MULTISAMPLE_RESOLVE_DEFAULT;
                break;
            case EGL_HORIZONTAL_RESOLUTION:
                if(surfaceDesc->surfaceType == EGL_WINDOW_BIT) {
                    *value = lcdCfg->widthInMicrons * 1000;
                    }
                else {
                    *value = EGL_UNKNOWN;
                    }
                break;
             case EGL_VERTICAL_RESOLUTION:
                if(surfaceDesc->surfaceType == EGL_WINDOW_BIT) {
                    *value = lcdCfg->heightInMicrons * 1000;
                    }
                else {
                    *value = EGL_UNKNOWN;
                    }
                break;
            case EGL_PIXEL_ASPECT_RATIO:
                if(surfaceDesc->surfaceType == EGL_WINDOW_BIT)
                    *value = (lcdCfg->heightInMicrons != 0) ?
                        (lcdCfg->widthInMicrons/lcdCfg->heightInMicrons) : 0;
                else
                    *value = EGL_UNKNOWN;
                break;
            case EGL_RENDER_BUFFER:
                *value = surfaceDesc->renderBuffer;
                break;
            case EGL_SWAP_BEHAVIOR:
#ifdef RIM_WINDOW_MANAGER
                *value = surfaceDesc->swapBehavior;
#else
                *value = EGL_BUFFER_PRESERVED;
#endif // RIM_WINDOW_MANAGER
                break;
            case EGL_TEXTURE_FORMAT:
                //Not implemented -> returns default
                if(surfaceDesc->surfaceType == EGL_PBUFFER_BIT)
                    *value = EGL_NO_TEXTURE;
                break;
            case EGL_TEXTURE_TARGET:
                //Not implemented -> returns default
                if(surfaceDesc->surfaceType == EGL_PBUFFER_BIT)
                    *value = EGL_NO_TEXTURE;
                break;

            case EGL_WIDTH:
                *value = surfaceDesc->width;
                break;

            default:
                break;
        }

        PRINT2N("Surface locked = %d, native renderable = %d", surfaceDesc->surfaceLocked,
            surfaceDesc->nativeRenderable );

        if (( surfaceDesc->surfaceLocked ) && ( surfaceDesc->nativeRenderable )) {
            switch(attribute) {
            case EGL_BITMAP_POINTER_KHR:
                *value = (EGLint)surfaceDesc->backBuffer.data;
//                *value = (EGLint)surfaceDesc->frontBuffer.data;
                PRINTN("Returning pointer to 0x%x", *value);
                break;
            case EGL_BITMAP_PITCH_KHR:
                // Number of bytes between successive rows
                *value = surfaceDesc->backBuffer.stride;
                break;
            case EGL_BITMAP_ORIGIN_KHR:
                //where the first pixel resides
                *value = EGL_UPPER_LEFT_KHR;
                break;
            case EGL_BITMAP_PIXEL_RED_OFFSET_KHR:
                if(surfaceDesc->bitDepth == 16){
                    *value = 11;
                }else if(surfaceDesc->bitDepth == 32){
                    *value = 16;
                }
                break;
            case EGL_BITMAP_PIXEL_GREEN_OFFSET_KHR:
                if(surfaceDesc->bitDepth == 16){
                    *value = 5;
                }else if(surfaceDesc->bitDepth == 32){
                    *value = 8;
                }
                break;
            case EGL_BITMAP_PIXEL_BLUE_OFFSET_KHR:
                if(surfaceDesc->bitDepth == 16){
                    *value = 0;
                }else if(surfaceDesc->bitDepth == 32){
                    *value = 0;
                }
                break;
            case EGL_BITMAP_PIXEL_ALPHA_OFFSET_KHR:
                if(surfaceDesc->bitDepth == 16){
                    *value = 0;
                }else if(surfaceDesc->bitDepth == 32){
                    *value = 24;
                }
                break;
            default:
                break;

            }
        }
        return EGL_TRUE;
    }
}

EGLBoolean eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface,
                            EGLint attribute, EGLint value){
    EGLSurfaceDescriptor * surfaceDesc = (EGLSurfaceDescriptor *)surface;
    EGLDisplayContext *dpyContext = NULL;

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglSurfaceAttrib : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    if(!dpyContext->eglInitialized){
        WARN( "eglSurfaceAttrib : display not initialized" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_FALSE;
    }

    if ( surfaceDesc->vendorSurface ) {
        EGLBoolean retBool;
        PRINT("vendor eglSurfaceAttrib");
        retBool = eglSurfaceAttribVndr(dpyContext->vendorDisplay, surfaceDesc->vendorSurface,
                attribute, value);
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVndr()));
        return retBool;
    }else{
        PRINT("RIM eglSurfaceAttrib");
        switch(attribute) {
        case EGL_MIPMAP_LEVEL:
            //Not implemented
            if(dpyContext->vendorConfigMap[surfaceDesc->ourConfigId-1].localConfig->ourConfig[CONFIG_ELEMENT(EGL_RENDERABLE_TYPE)]
                & EGL_OPENGL_ES_BIT){
                //Set new mipmap level
            }else{
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_PARAMETER));
                return EGL_FALSE;
            }
            break;
        case EGL_MULTISAMPLE_RESOLVE:
            //Not implemented
            if(dpyContext->vendorConfigMap[surfaceDesc->ourConfigId-1].localConfig->ourConfig[CONFIG_ELEMENT(EGL_SURFACE_TYPE)]
                & EGL_MULTISAMPLE_RESOLVE_BOX_BIT){
                //Set new multisample resolve
            }else{
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
                return EGL_FALSE;
            }
            break;
        case EGL_SWAP_BEHAVIOR:
            //Not implemented
            if(value == EGL_BUFFER_PRESERVED){
                if(dpyContext->vendorConfigMap[surfaceDesc->ourConfigId-1].localConfig->ourConfig[CONFIG_ELEMENT(EGL_SURFACE_TYPE)]
                    & EGL_SWAP_BEHAVIOR_PRESERVED_BIT){
                    //Set new swap behavior
#ifdef RIM_WINDOW_MANAGER
                    surfaceDesc->swapBehavior = value;
#endif // RIM_WINDOW_MANAGER
                }else{
                    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
                    return EGL_FALSE;
                }
            }else if(value == EGL_BUFFER_DESTROYED){
                //Set new swap behavior
#ifdef RIM_WINDOW_MANAGER
                surfaceDesc->swapBehavior = value;
#endif // RIM_WINDOW_MANAGER
            }else{
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ATTRIBUTE));
                return EGL_FALSE;
            }
            break;
        default:
            break;
        }
        return EGL_TRUE;
    }
}

static void deleteSurfaceBackBuffer(EGLSurfaceDescriptor * surface) {

    PRINT2N( "deleteSurface: 0x%08x v0x%08x", (EGLSurface)surface, (surface != EGL_NO_SURFACE) ? (EGLSurface)surface->vendorSurface : EGL_NO_SURFACE );

    if(surface && surface->ownsBackBuffer) {
            // TODO: If buffer created buffer for VTMV with alignment, the following calculation
            //     won't be valid


        //Front and back buffer surfaces must always be allocated/deallocated with kdMmapRim/kdMunmapRim
        {
            DWORD backBufferSize = surface->backBuffer.stride * surface->backBuffer.high;
            kdMunmapRim(surface->backBuffer.data, backBufferSize);
        }
    }


}




static void deleteSurface(EGLSurfaceDescriptor * surface){
   
    deleteSurfaceBackBuffer( surface );

#ifdef RIM_WINDOW_MANAGER
    // release buffer in window and surface
    if (surface->frontBuffer.data) {
        
        WMIntSetWindowBuffer(surface->nativeWindow, NULL, WM_BUFFER_STATE_NONE);

        //Front and back buffer surfaces must always be allocated/deallocated with kdMmapRim/kdMunmapRim
        {
            DWORD frontBufferSize = surface->frontBuffer.stride * surface->frontBuffer.high;
            kdMunmapRim(surface->frontBuffer.data, frontBufferSize);
        }
    }
#endif // RIM_WINDOW_MANAGER

    surface->markForDelete = FALSE;
    kdFree(surface);
}

static void deleteContext(EGLRenderingContext * context){
    //decrease the number of contexts using this config
    PRINT2N( "deleteContext: 0x%08x v0x%08x", (EGLContext)context, (context != EGL_NO_CONTEXT) ? (EGLContext)context->vendorContext : EGL_NO_CONTEXT);
    context->markForDelete = FALSE;
    kdFree(context);
}

EGLSurface eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config,
                                  NativeWindowType win, const EGLint *attrib_list )
{
    EGLDisplayContext *dpyContext = NULL;
    EGLSurfaceDescriptor * newEglSurface = EGL_NO_SURFACE;
    DWORD config_id = (DWORD)config;
    EGLConfigContextPtr currentConfigContext;

    DWORD transparentColour = 0x00000000;
    EGLint transparentType = EGL_NONE;
    EGLint colorSpace = EGL_COLORSPACE_sRGB;
    EGLint alphaFormat = EGL_ALPHA_FORMAT_NONPRE;
    EGLint *readPtr = (EGLint*)attrib_list;
    EGLint *writePtr = (EGLint*)attrib_list;
    DWORD i = 0;
    DWORD bitDepth;
    DWORD backBufferSize;
    EGLint colorBufferType;
#ifdef RIM_WINDOW_MANAGER
    WMError_t result;
    BitMap frontBuffer;
    DWORD frontBufferSize;
    WMWindow_t nativeWindow = (WMWindow_t)win;
    SDWORD winSize[2];
    SDWORD winContext[1];
    WMWindowAttributes_t windowAttributes;
    PRINT("define RIM_WINDOW_MANAGER");
#else
    NativeWindowAttrib nativeAttrib;

#endif // RIM_WINDOW_MANAGER

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );

    if(!dpyContext){
        WARN( "eglCreateWindowSurface : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_NO_SURFACE;
    }
    if(!dpyContext->eglInitialized){
        WARN( "eglCreateWindowSurface : display not initialized" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_NO_SURFACE;
    }

    // Check if this is a valid EGL config
    if( config_id < 1 || config_id > TOTAL_NUMBER_OF_CONFIGS){
        WARN ("eglCreateWindowSurface : invalid config");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONFIG));
        return EGL_NO_SURFACE;
    }
    if(!(dpyContext->vendorConfigMap[config_id-1].localConfig->displayMask & DISPLAY(dpyContext->id))){
        WARN ("eglCreateWindowSurface : config is not for this display");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONFIG));
        return EGL_NO_SURFACE;
    }
    currentConfigContext = dpyContext->vendorConfigMap[config_id-1].localConfig->ourConfig;

    // Check if the config supports rendering to windows
    if(!(currentConfigContext[CONFIG_ELEMENT(EGL_SURFACE_TYPE)] & EGL_WINDOW_BIT)){
        //the choosen config does not support rendering to windows
        WARN ("eglCreateWindowSurface : config does not support rendering to windows");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
        return EGL_NO_SURFACE;
    }

#ifdef RIM_WINDOW_MANAGER
    // get window size
    result = WMGetWindowPropertyiv(nativeWindow, WM_PROPERTY_SIZE, winSize);
    PRINTN("winSize[0]=%d",winSize[0]);
    PRINTN("winSize[1]=%d",winSize[1]);
    if (result != WM_E_OK) {
        WARN("eglCreateWindowSurface : bad native window");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_NATIVE_WINDOW));
        return EGL_NO_SURFACE;
    }

    // get display id for window
    result = WMGetWindowAttributes(nativeWindow, &windowAttributes);
    if (result != WM_E_OK) {
        WARN("eglCreateWindowSurface : bad native window");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_NATIVE_WINDOW));
        return EGL_NO_SURFACE;
    }

    // verify window and surface are for same display
    if (dpyContext->id != windowAttributes.displayId) {
        WARN("eglCreateWindowSurface : bad native window");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_NATIVE_WINDOW));
        return EGL_NO_SURFACE;
    }
#else
    nativeAttrib.display = dpyContext->id;

    if(NativeCheckSetLevelWindowInt(win, &nativeAttrib, 0) != WIN_SUCCESS){
        WARN("eglCreateWindowSurface : bad native window");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_NATIVE_WINDOW));
        return EGL_NO_SURFACE;
    }
#endif // RIM_WINDOW_MANAGER

    // Parse attribute list and store and remove EGL_TRANSPARENT attributes
    for(i=0; attrib_list!= NULL && attrib_list[i] != EGL_NONE; i+=2 ) {
        if(attrib_list[i] == EGL_COLORSPACE){
            colorSpace = attrib_list[i+1];
            if(readPtr != writePtr){
                *writePtr = *readPtr;
                *(writePtr + 1) = *(readPtr + 1);
            }
            writePtr += 2;
        }else if(attrib_list[i] == EGL_ALPHA_FORMAT){
            alphaFormat = attrib_list[i+1];
            if(readPtr != writePtr){
                *writePtr = *readPtr;
                *(writePtr + 1) = *(readPtr + 1);
            }
            writePtr += 2;
        }else if(attrib_list[i] == EGL_TRANSPARENT_TYPE){
            transparentType = attrib_list[i+1];
        }else if(transparentType != EGL_NONE){
            if(attrib_list[i] == EGL_TRANSPARENT_RED_VALUE){
                if(currentConfigContext[CONFIG_ELEMENT(EGL_RED_SIZE)] == 8){
                    transparentColour |= ((attrib_list[i+1] >> 3) << 11);
                }else{
                    transparentColour |= (attrib_list[i+1] << 11);
                }
            }else if(attrib_list[i] == EGL_TRANSPARENT_GREEN_VALUE){
                if(currentConfigContext[CONFIG_ELEMENT(EGL_GREEN_SIZE)] == 8){
                    transparentColour |= ((attrib_list[i+1] >> 2) << 5);
                }else{
                    transparentColour |= (attrib_list[i+1] << 5);
                }
            }else if(attrib_list[i] == EGL_TRANSPARENT_BLUE_VALUE){
                if(currentConfigContext[CONFIG_ELEMENT(EGL_BLUE_SIZE)] == 8){
                    transparentColour |= (attrib_list[i+1] >> 3);
                }else{
                    transparentColour |= attrib_list[i+1];
                }
            }
        }
        readPtr += 2;
    }
    if(readPtr != writePtr){
        *writePtr = *readPtr;
        *(writePtr + 1) = *(readPtr + 1);
    }
    if(transparentType == EGL_NONE){
        //Set transparent colour to black
        transparentColour = 0x00000000;
    }
    // Querying the bit depth of the config
    bitDepth = currentConfigContext[CONFIG_ELEMENT(EGL_BUFFER_SIZE)];
    PRINTN("Config BD: %d",bitDepth);
    colorBufferType = currentConfigContext[CONFIG_ELEMENT(EGL_COLOR_BUFFER_TYPE)];
    PRINTN("EGL_COLOR_BUFFER_TYPE is 0x%x",colorBufferType);

#ifdef RIM_WINDOW_MANAGER
    // Set front buffer format and size
    memset(&frontBuffer, 0, sizeof(BitMap));

//    frontBuffer.bType = (bitDepth == 32) ? BMT_32BPP_ARGB8888 : BMT_16BPP_RGB565; // original
    if( 32 == bitDepth ) {
        frontBuffer.bType = BMT_32BPP_ARGB8888;
    } else if( EGL_LUMINANCE_BUFFER == colorBufferType && 16 == bitDepth ) {
        frontBuffer.bType = BMT_16BPP_YUV422_SW;
        PRINT("is YUV422");
    } else if( EGL_LUMINANCE_BUFFER == colorBufferType && 12 == bitDepth ) {
        frontBuffer.bType = BMT_12BPP_YUV420;
        PRINT("is YUV420");
    } else {
        frontBuffer.bType = BMT_16BPP_RGB565;
    }

    frontBuffer.wide = winSize[0];
    frontBuffer.high = winSize[1];

    // Calculate bytes per row (must be 8-byte aligned)
    frontBuffer.stride = ( (frontBuffer.wide * (bitDepth / 8)) + 7 ) & ~7;

    // Calculate total buffer size in bytes
    frontBufferSize = frontBuffer.stride * frontBuffer.high;

    // Allocate memory for front buffer
    PRINT("Allocate memory for front buffer");
    frontBuffer.data = kdMmapRim( frontBufferSize );
    if ( !EglIntIsValidBufferPtr(frontBuffer.data) ) {
        WARN("Couldn't allocate front buffer");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));
        return EGL_NO_SURFACE;
    }

    memset(frontBuffer.data, 0, frontBufferSize);

    // Bind front buffer to window (must do before allocating vendor surface!)
    result = WMIntSetWindowBuffer(nativeWindow, &frontBuffer, WM_BUFFER_STATE_NEW);
    PRINT("Bind front buffer to window");
    if (result != WM_E_OK) {
        WARN("eglCreateWindowSurface : bad native window");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_NATIVE_WINDOW));
        kdMunmapRim(frontBuffer.data, frontBufferSize);
        return EGL_NO_SURFACE;
    }
#endif // RIM_WINDOW_MANAGER

    if (dpyContext->vendorConfigMap[config_id-1].vendorConfigAvail) {
#if defined ( EGL_USE_VENDOR_IMPLEMENTATION )
        EGLSurface retSurf;
        PRINT("dpyContext->vendorConfigMap[config_id-1].vendorConfigAvail == TRUE");

        newEglSurface = (EGLSurfaceDescriptor*)kdMalloc(sizeof(EGLSurfaceDescriptor));
        if (newEglSurface == EGL_NO_SURFACE){
            WARN("Couldn't allocate wrapper");
    #ifdef RIM_WINDOW_MANAGER
            WMIntSetWindowBuffer(nativeWindow, NULL, WM_BUFFER_STATE_NONE);
            kdMunmapRim(frontBuffer.data, frontBufferSize);
    #endif // RIM_WINDOW_MANAGER
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));
            return EGL_NO_SURFACE;
        }

        kdThreadMutexLock(EglProcessState.SurfaceMutex);

        // Initialize the surface
        initializeSurface(dpy, newEglSurface, EGL_WINDOW_BIT);

        newEglSurface->ourConfigId = config_id;
        newEglSurface->alphaFormat = alphaFormat;
        newEglSurface->ownsBackBuffer = FALSE;  // till proven otherwise...
    #ifdef RIM_WINDOW_MANAGER
        newEglSurface->nativeWindow = nativeWindow;
        newEglSurface->width = winSize[0];
        newEglSurface->height = winSize[1];

        // FB & BB are identical (except data pointer)
        memcpy(&newEglSurface->frontBuffer, &frontBuffer, sizeof(BitMap));
        memcpy(&newEglSurface->backBuffer, &frontBuffer, sizeof(BitMap));

        // allocate back buffer - should only be used by VT driver
        if ( EglUseNewSwapLogic || EglIsAllocatingBackBufferForDriver() ) {
            PRINT("eglCreateWindowSurface: allocating back buffer to be used by driver");
            newEglSurface->ownsBackBuffer = TRUE;

            newEglSurface->backBuffer.data = kdMmapRim( frontBufferSize );
            if ( !EglIntIsValidBufferPtr(newEglSurface->backBuffer.data) ) {
                WARN("Couldn't allocate back buffer");
                kdThreadMutexUnlock(EglProcessState.SurfaceMutex);

                WMIntSetWindowBuffer(nativeWindow, NULL, WM_BUFFER_STATE_NONE);
                kdMunmapRim(frontBuffer.data, frontBufferSize);
                kdFree(newEglSurface);
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));
                return EGL_NO_SURFACE;
            }

            memset(newEglSurface->backBuffer.data, 0, frontBufferSize);
        }


        winContext[0] = (SDWORD)newEglSurface;
        result = WMIntSetWindowPropertyiv( nativeWindow, WM_PROPERTY_USER_CONTEXT, winContext, WM_NO_TRANSACTION );
        if( result != WM_E_OK ) {
            kdThreadMutexUnlock(EglProcessState.SurfaceMutex);

            WMIntSetWindowBuffer(nativeWindow, NULL, WM_BUFFER_STATE_NONE);
            kdMunmapRim(frontBuffer.data, frontBufferSize);
            kdMunmapRim(newEglSurface->backBuffer.data, frontBufferSize);
            kdFree(newEglSurface);
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_NATIVE_WINDOW));
            return EGL_NO_SURFACE;
        }
    #else // RIM_WINDOW_MANAGER
        newEglSurface->nativeWindow = NativeTranslateHandleInt(win);
        // Indicate that the NativeWindow is in use
        newEglSurface->nativeWindow->boundToEGLSurface = TRUE;
        // Initialize height and width parameters
        newEglSurface->height = newEglSurface->nativeWindow->windowSize.height;
        newEglSurface->width = newEglSurface->nativeWindow->windowSize.width;
    #endif // RIM_WINDOW_MANAGER
        newEglSurface->bitDepth = bitDepth;

        kdThreadMutexUnlock(EglProcessState.SurfaceMutex);

        // Create a vendor surface
        retSurf = eglCreateWindowSurfaceVndr(dpyContext->vendorDisplay, 
                                             dpyContext->vendorConfigMap[config_id-1].vendorConfig, 
                                             win, attrib_list);
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVndr()));

        if( EGL_NO_SURFACE == retSurf ) {
            PRINTN("vndr eglCreateWindowSurface failed 0x%04x",(EGLint)kdGetThreadStorageKHR(eglLastErrorKey));
    #ifdef RIM_WINDOW_MANAGER
            WMIntSetWindowBuffer(nativeWindow, NULL, WM_BUFFER_STATE_NONE);
            kdMunmapRim(frontBuffer.data, frontBufferSize);
            kdMunmapRim(newEglSurface->backBuffer.data, frontBufferSize);
    #endif // RIM_WINDOW_MANAGER
            kdFree(newEglSurface);
            return EGL_NO_SURFACE;
        } else {
            PRINTN("vndr eglCreateWindowSurface success 0x%04x",(EGLint)kdGetThreadStorageKHR(eglLastErrorKey));
            newEglSurface->vendorSurface = retSurf;
            eglUpdateVendorSurface(newEglSurface);
        }
#else // EGL_USE_VENDOR_IMPLEMENTATION
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));

        newEglSurface = EGL_NO_SURFACE;
#endif // EGL_USE_VENDOR_IMPLEMENTATION
    } else{
        PRINT("Creating window surface - non vendor");

        //MALLOC
        newEglSurface = (EGLSurfaceDescriptor*)kdMalloc(sizeof(EGLSurfaceDescriptor));

        if(!newEglSurface){
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));
#ifdef RIM_WINDOW_MANAGER
            WMIntSetWindowBuffer(nativeWindow, NULL, WM_BUFFER_STATE_NONE);
            kdMunmapRim(frontBuffer.data, frontBufferSize);
#endif // RIM_WINDOW_MANAGER
            return EGL_NO_SURFACE;
        }
        kdThreadMutexLock(EglProcessState.SurfaceMutex);

        // Initialize the surface
        initializeSurface(dpy, newEglSurface, EGL_WINDOW_BIT);

        //Initialize surface attributes to default values
        newEglSurface->colorspace = colorSpace;
        newEglSurface->alphaFormat = alphaFormat;
        newEglSurface->renderBuffer = EGL_BACK_BUFFER;
        newEglSurface->bitDepth = bitDepth;
#ifdef RIM_WINDOW_MANAGER
        newEglSurface->nativeWindow = nativeWindow;
        newEglSurface->width = winSize[0];
        newEglSurface->height = winSize[1];
        memcpy(&newEglSurface->frontBuffer, &frontBuffer, sizeof(BitMap));
#else
        newEglSurface->nativeWindow = NativeTranslateHandleInt(win);
        if( !newEglSurface->nativeWindow ) {
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));
            kdFree(newEglSurface);
            kdThreadMutexUnlock(EglProcessState.SurfaceMutex);
            return EGL_NO_SURFACE;
        }
        newEglSurface->height = newEglSurface->nativeWindow->windowSize.height;
        newEglSurface->width = newEglSurface->nativeWindow->windowSize.width;
#endif // RIM_WINDOW_MANAGER

        // Allocate back buffer for window surface
        PRINT("Using Auxillary buffer");
        newEglSurface->ownsBackBuffer = TRUE;
#ifdef RIM_WINDOW_MANAGER
        if( 32 == bitDepth ) {
            newEglSurface->backBuffer.bType = BMT_32BPP_ARGB8888; 
        } else if( EGL_LUMINANCE_BUFFER == colorBufferType && 16 == bitDepth ) {
            newEglSurface->backBuffer.bType = BMT_16BPP_YUV422_SW;
#ifdef RIM_WINDOW_MANAGER
            newEglSurface->swapBehavior = EGL_BUFFER_DESTROYED;
#endif
            PRINT("back buffer is YUV422");
        } else if( EGL_LUMINANCE_BUFFER == colorBufferType && 12 == bitDepth ) {
            newEglSurface->backBuffer.bType = BMT_12BPP_YUV420;
#ifdef RIM_WINDOW_MANAGER
            newEglSurface->swapBehavior = EGL_BUFFER_DESTROYED;
#endif
            PRINT("back buffer is YUV420");
        } else {
            newEglSurface->backBuffer.bType = BMT_16BPP_RGB565;
        }
#else
        newEglSurface->backBuffer.bType = (bitDepth == 32) ? BMT_32BPP_ARGB8888 : BMT_16BPP_RGB565; //original
#endif
        newEglSurface->backBuffer.wide = newEglSurface->width;
        newEglSurface->backBuffer.high = newEglSurface->height;

        // Calculate bytes per row (must be 8-byte aligned)
        newEglSurface->backBuffer.stride = ( (newEglSurface->backBuffer.wide * (bitDepth / 8)) + 7 ) & ~7;

        // Calculate total buffer size in bytes
        backBufferSize = newEglSurface->backBuffer.stride * newEglSurface->backBuffer.high;
        PRINTN("bufferSize = %d",backBufferSize);
        
        // Mark native renderable surfaces - this means that proprietary RIM API's can render to them
        PRINTCOPY("Current config context is %s",
            (currentConfigContext[CONFIG_ELEMENT(EGL_NATIVE_RENDERABLE)] == EGL_TRUE) ?
            "native renderable" : "not native renderable");

        if (currentConfigContext[CONFIG_ELEMENT(EGL_NATIVE_RENDERABLE)] == EGL_TRUE ) {
            newEglSurface->nativeRenderable = TRUE;
        }

        // allocate the back buffer.  If nativeRenderable && swap behaviour is BUFFER_DESTROYED, swap buffers
        // is implemented by swapping the buffer pointers, so the back buffer has to be allocated in the same
        // way as the front buffer (i.e using mmap).  Thus we use mmap whenever the surface is nativeRenderable,
        // as EGL_BUFFER_DESTROYED can be set later for the surface.
        
            newEglSurface->backBuffer.data = kdMmapRim( backBufferSize );
            if(!newEglSurface->backBuffer.data){
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));
#if defined( RIM_WINDOW_MANAGER )
                WMIntSetWindowBuffer(nativeWindow, NULL, WM_BUFFER_STATE_NONE);
                kdMunmapRim(frontBuffer.data, frontBufferSize);
#endif
            kdFree(newEglSurface);
            kdThreadMutexUnlock(EglProcessState.SurfaceMutex);
            return EGL_NO_SURFACE;
        }


        memset(newEglSurface->backBuffer.data, 0, backBufferSize);

        // No vendor surface used
        newEglSurface->vendorSurface = EGL_NO_SURFACE;

        // Record our EGL config id
        newEglSurface->ourConfigId = config_id;

#ifndef RIM_WINDOW_MANAGER
        // Indicate that the native window is in use
        newEglSurface->nativeWindow->boundToEGLSurface = TRUE;
#endif // RIM_WINDOW_MANAGER

        kdThreadMutexUnlock(EglProcessState.SurfaceMutex);
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    }

#ifdef RIM_WINDOW_MANAGER
    // destroy front buffer if surface allocation failed
    if (newEglSurface == EGL_NO_SURFACE) {
      
        WMIntSetWindowBuffer(nativeWindow, NULL, WM_BUFFER_STATE_NONE);
        kdMunmapRim(frontBuffer.data, frontBufferSize);
    }
#endif // RIM_WINDOW_MANAGER

    PRINT2N( "eglCreateWindowSurface: 0x%08x v0x%08x", (EGLSurface)newEglSurface, (newEglSurface != EGL_NO_SURFACE) ? (EGLSurface)newEglSurface->vendorSurface : EGL_NO_SURFACE );
    return (EGLSurface)newEglSurface;
}

EGLSurface eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config,
                                   const EGLint *attrib_list )
{
    BOOL bufferSourceFront;
    EGLDisplayContext *dpyContext = NULL;
    EGLSurfaceDescriptor * newEglSurface = EGL_NO_SURFACE;
    DWORD i;
    EGLConfigContextPtr currentConfigContext;
    DWORD config_id = (DWORD)config;
    LcdConfig *lcdCfg = NULL;
#if !defined(RIM_NDK)
    WORD *lcdBuffer;
    DWORD lcdStride;
    DWORD lcdFormat;
#endif
    DWORD bufferSize;

#if !defined (RIM_NDK)
    PRINT2N("Current API is Task [%d] API [0x%x]",
    RimGetCurrentTaskPriority(), (EGLenum)kdGetThreadStorageKHR(eglCurrentApiKey) );
#endif
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglCreatePbufferSurface : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_NO_SURFACE;
    }
    if(!dpyContext->eglInitialized){
        WARN( "eglCreatePbufferSurface : display not initialized" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_NO_SURFACE;
    }

    lcdCfg = LcdGetLcdConfigPtr( dpyContext->id );

    // Check if this is a valid EGL config
    if( config_id < 1 || config_id > TOTAL_NUMBER_OF_CONFIGS){
        WARN ("eglCreatePbufferSurface : invalid config");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONFIG));
        return EGL_NO_SURFACE;
    }
    if (!(dpyContext->vendorConfigMap[config_id-1].localConfig->displayMask & DISPLAY(dpyContext->id))){
        WARN ("eglCreatePbufferSurface : config is not for this display");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONFIG));
        return EGL_NO_SURFACE;
    }

    currentConfigContext = dpyContext->vendorConfigMap[config_id-1].localConfig->ourConfig;

    // Check if the config supports rendering to pbuffers
    if(!(currentConfigContext[CONFIG_ELEMENT(EGL_SURFACE_TYPE)] & EGL_PBUFFER_BIT)){
        // The choosen config does not support rendering to pbuffers
        WARN ("eglCreatePbufferSurface : config does not support rendering PBuffers");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
        return EGL_NO_SURFACE;
    }

    if ( (EGLenum)kdGetThreadStorageKHR(eglCurrentApiKey) == EGL_BITMAP_API_RIM ) {
        bufferSourceFront = TRUE;
    }
    else {
        bufferSourceFront = FALSE;
    }

#if defined( EGL_USE_VENDOR_IMPLEMENTATION )
    if( dpyContext->vendorConfigMap[config_id-1].vendorConfigAvail && !bufferSourceFront )
    {
        EGLSurface retSurf = EGL_NO_SURFACE;

        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONFIG));

        PRINT("Vndr eglCreatePbufferSurface");
        retSurf = eglCreatePbufferSurfaceVndr( dpyContext->vendorDisplay,
                                               dpyContext->vendorConfigMap[config_id-1].vendorConfig,
                                               attrib_list );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVndr()));

        if( retSurf!=EGL_NO_SURFACE ) {
            newEglSurface = (EGLSurfaceDescriptor*)kdMalloc(sizeof(EGLSurfaceDescriptor));

            if( !newEglSurface ) {
                eglDestroySurfaceVndr(dpyContext->vendorDisplay, retSurf);
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));

                return EGL_NO_SURFACE;
            }
            initializeSurface(dpy, newEglSurface, EGL_PBUFFER_BIT);
            newEglSurface->ownsBackBuffer = FALSE;
            newEglSurface->vendorSurface = retSurf;
            newEglSurface->ourConfigId = config_id;
            newEglSurface->bitDepth = currentConfigContext[CONFIG_ELEMENT(EGL_BUFFER_SIZE)];
            eglUpdateVendorSurface(newEglSurface);

            // Parse the attribute list to populate the width and height, which is needed by qceglCopyBuffersWrap()
            for(i=0; attrib_list!=NULL && attrib_list[i] != EGL_NONE; i+=2 ) {
                if (attrib_list[i] == EGL_HEIGHT ) {
                    newEglSurface->height = attrib_list[i+1];
                }else if (attrib_list[i] == EGL_WIDTH ) {
                    newEglSurface->width = attrib_list[i+1];
                }else if(attrib_list[i] == EGL_ALPHA_FORMAT){
                    newEglSurface->alphaFormat = attrib_list[i+1];
                    PRINTN("alphaFormat=0x%x",newEglSurface->alphaFormat);
                }
            }
        }
    } else
#endif
    {
        // Create a surface descriptor
        newEglSurface = (EGLSurfaceDescriptor*)kdMalloc(sizeof(EGLSurfaceDescriptor));

        if(!newEglSurface){
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));
            return EGL_NO_SURFACE;
        }

        kdThreadMutexLock(EglProcessState.SurfaceMutex);

        // Init the flags
        initializeSurface(dpy, newEglSurface, EGL_PBUFFER_BIT);

        // Initialize surface attributes to default values
        newEglSurface->colorspace = EGL_COLORSPACE_sRGB;
        newEglSurface->alphaFormat = EGL_ALPHA_FORMAT_NONPRE;
        newEglSurface->renderBuffer = EGL_BACK_BUFFER;

        for(i=0; attrib_list!=NULL && attrib_list[i] != EGL_NONE; i+=2 ) {
            if(attrib_list[i] == EGL_COLORSPACE){
                newEglSurface->colorspace = attrib_list[i+1];
            }else if(attrib_list[i] == EGL_ALPHA_FORMAT){
                newEglSurface->alphaFormat = attrib_list[i+1];
            }else if (attrib_list[i] == EGL_HEIGHT ) {
                newEglSurface->height = attrib_list[i+1];
            }else if (attrib_list[i] == EGL_WIDTH ) {
                newEglSurface->width = attrib_list[i+1];
            }
        }

        // For now if the height and width are not specified in the attribute list, default to the lcd height and width
        // NOTE: Should be removed and replace with errors when the JVM front pbuffer hack is removed
        if(newEglSurface->height == 0){
            newEglSurface->height = lcdCfg->height;
        }

        if(newEglSurface->width == 0){
            newEglSurface->width = lcdCfg->width;
        }

        // Querying the bit depth of the config
        newEglSurface->bitDepth = currentConfigContext[CONFIG_ELEMENT(EGL_BUFFER_SIZE)];

        // Not using a vendor surface
        newEglSurface->vendorSurface = EGL_NO_SURFACE;

        // Make sure to mark this surface as native renderable, otherwise it's
        // not going to be properly posted at eglSwapBuffers
        newEglSurface->nativeRenderable = TRUE;

        // Record our EGL config id
        newEglSurface->ourConfigId = config_id;

        // Check if we are using the front buffer directly or whether we are allocating an auxillary buffer
        if(bufferSourceFront && (newEglSurface->bitDepth == 16)){
#if !defined(RIM_NDK)
            PRINT("Creating EGL_PBUFFER_FRONT_RIM Pbuffer");
            PRINTN("EGL_PBUFFER_FRONT_RIM bitdepth=%d", newEglSurface->bitDepth);
            //update front buffer
            PRINT("Updating front buffer pointer with new EGL surface");
            pBufferFrontSurface[dpyContext->id] = newEglSurface;

            LcdGetBuffer( dpyContext->id, LCD_FRONT_BUFFER, &lcdBuffer, &lcdFormat, &lcdStride );

            newEglSurface->ownsBackBuffer = FALSE;
            newEglSurface->backBuffer.bType = lcdFormat;
            newEglSurface->backBuffer.wide = lcdCfg->width;
            newEglSurface->backBuffer.high = lcdCfg->height;
            newEglSurface->backBuffer.stride = lcdStride;
            newEglSurface->backBuffer.data = (BYTE*)lcdBuffer;
#else
            WARN("Using front buffer directly is not supported in user mode");
            _exit(0);
#endif
        }else{
            PRINT("Use an Auxillary buffer");
            newEglSurface->ownsBackBuffer = TRUE;
            newEglSurface->backBuffer.bType = (newEglSurface->bitDepth == 32) ? BMT_32BPP_ARGB8888 : BMT_16BPP_RGB565;
            newEglSurface->backBuffer.wide = newEglSurface->width;
            newEglSurface->backBuffer.high = newEglSurface->height;

            // Calculate bytes per row
            newEglSurface->backBuffer.stride = newEglSurface->width * (newEglSurface->bitDepth / 8);

            // Calculate total buffer size in bytes
            bufferSize = newEglSurface->backBuffer.stride * newEglSurface->backBuffer.high;

            newEglSurface->backBuffer.data = kdMmapRim(bufferSize);

            if(!newEglSurface->backBuffer.data){
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));
                kdFree(newEglSurface);

                kdThreadMutexUnlock(EglProcessState.SurfaceMutex);
                return EGL_NO_SURFACE;
            }
            memset(newEglSurface->backBuffer.data, 0, bufferSize);
        }

        kdThreadMutexUnlock(EglProcessState.SurfaceMutex);
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    }

    PRINT2N( "eglCreatePbufferSurface: 0x%08x v0x%08x", (EGLSurface)newEglSurface, (newEglSurface != EGL_NO_SURFACE) ? (EGLSurface)newEglSurface->vendorSurface : EGL_NO_SURFACE );
    return (EGLSurface)newEglSurface;
}

EGLSurface eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config,
                                  NativePixmapType pixmap,
                                  const EGLint *attrib_list)
{
    EGLDisplayContext *dpyContext = NULL;
    EGLSurfaceDescriptor * newEglSurface = EGL_NO_SURFACE;
    EGLConfigContextPtr currentConfigContext;
    DWORD config_id = (DWORD)config;
    BitMap* pixmapBitMap = (BitMap*)pixmap;

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglCreatePixmapSurface : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_NO_SURFACE;
    }

    if(!dpyContext->eglInitialized){
        WARN( "eglCreatePixmapSurface : display not initialized" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_NO_SURFACE;
    }

    // Check if this is a valid EGL config
    if( config_id<1 || config_id>TOTAL_NUMBER_OF_CONFIGS){
        WARN ("eglCreatePixmapSurface : invalid config");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONFIG));
        return EGL_NO_SURFACE;
    }

    if (!(dpyContext->vendorConfigMap[config_id-1].localConfig->displayMask & DISPLAY(dpyContext->id))){
        WARN ("eglCreatePixmapSurface : config is not for this display");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONFIG));
        return EGL_NO_SURFACE;
    }

    currentConfigContext = dpyContext->vendorConfigMap[config_id-1].localConfig->ourConfig;

    // Check if the config supports rendering to pixmaps
    if(!(currentConfigContext[CONFIG_ELEMENT(EGL_SURFACE_TYPE)] & EGL_PIXMAP_BIT)){
        //the choosen config does not support rendering to windows
        WARN ("eglCreatePbufferSurface : config does not support rendering Pixmaps");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
        return EGL_NO_SURFACE;
    }

    // Check if the pixmap is valid
    if(pixmapBitMap == NULL){
        WARNN("eglCreatePixmapSurface : invalid pixmap - 0x%08x",pixmapBitMap);
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_NATIVE_PIXMAP));
        return EGL_NO_SURFACE;
    }

    if(pixmapBitMap->stride == 0 || pixmapBitMap->wide == 0 || pixmapBitMap->high == 0 || pixmapBitMap->data == NULL){
        WARN4N("eglCreatePixmapSurface : invalid pixmap - stride %d wide %d high %d data 0x%08x",pixmapBitMap->stride, pixmapBitMap->wide, pixmapBitMap->high, pixmapBitMap->data);
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_NATIVE_PIXMAP));
        return EGL_NO_SURFACE;
    }

    // Check if the pixmap matches the config
    if((currentConfigContext[CONFIG_ELEMENT(EGL_BUFFER_SIZE)] / 8) != (pixmapBitMap->stride / pixmapBitMap->wide)){
        WARN("eglCreatePixmapSurface : pixmap does not match the config");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_NATIVE_PIXMAP));
        return EGL_NO_SURFACE;
    }

    if(dpyContext->vendorConfigMap[config_id-1].vendorConfigAvail) {
#if defined ( EGL_USE_VENDOR_IMPLEMENTATION )
        EGLSurface tempSurface = eglCreatePixmapSurfaceVndr(
                                        dpyContext->vendorDisplay, 
                                        dpyContext->vendorConfigMap[config_id-1].vendorConfig,
                                        pixmap, 
                                        attrib_list);

        if ( EGL_NO_SURFACE != tempSurface ) {
            DWORD i=0;

            newEglSurface = (EGLSurfaceDescriptor*)kdMalloc(sizeof(EGLSurfaceDescriptor));

            if(!newEglSurface){
                eglDestroySurfaceVndr( dpyContext->vendorDisplay, tempSurface );
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));
                return EGL_NO_SURFACE;
            }
        
            initializeSurface(dpy, newEglSurface, EGL_PIXMAP_BIT);

            // Querying the bit depth of the config
            newEglSurface->bitDepth = currentConfigContext[CONFIG_ELEMENT(EGL_BUFFER_SIZE)];

            // Buffer is givien to us, however we are not going to deallocate it
            newEglSurface->ownsBackBuffer = FALSE;
            memcpy(&newEglSurface->backBuffer, pixmapBitMap, sizeof(BitMap));

            // Initialize surface attributes to default values
            newEglSurface->colorspace = EGL_COLORSPACE_sRGB;
            newEglSurface->alphaFormat = EGL_ALPHA_FORMAT_NONPRE;
            newEglSurface->renderBuffer = EGL_SINGLE_BUFFER;
            newEglSurface->height = pixmapBitMap->high;
            newEglSurface->width = pixmapBitMap->wide;

            for(i=0; attrib_list!=NULL && attrib_list[i] != EGL_NONE; i+=2 ) {
                if(attrib_list[i] == EGL_COLORSPACE) {
                    newEglSurface->colorspace = attrib_list[i+1];
                } else if(attrib_list[i] == EGL_ALPHA_FORMAT) {
                    newEglSurface->alphaFormat = attrib_list[i+1];
                }
            }

            // No vendor surface
            newEglSurface->vendorSurface = tempSurface;
            // Record our EGL config id
            newEglSurface->ourConfigId = config_id;
        }
#else
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONFIG));
        newEglSurface = EGL_NO_SURFACE;
#endif
    } else {
        DWORD i=0;

        newEglSurface = (EGLSurfaceDescriptor*)kdMalloc(sizeof(EGLSurfaceDescriptor));

        if(!newEglSurface){
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));
            return EGL_NO_SURFACE;
        }

        kdThreadMutexLock(EglProcessState.SurfaceMutex);

        // Init the flags
        initializeSurface(dpy, newEglSurface, EGL_PIXMAP_BIT);

        // Querying the bit depth of the config
        newEglSurface->bitDepth = currentConfigContext[CONFIG_ELEMENT(EGL_BUFFER_SIZE)];

        // Buffer is givien to us, however we are not going to deallocate it
        newEglSurface->ownsBackBuffer = FALSE;
        memcpy(&newEglSurface->backBuffer, pixmapBitMap, sizeof(BitMap));

        // Initialize surface attributes to default values
        newEglSurface->colorspace = EGL_COLORSPACE_sRGB;
        newEglSurface->alphaFormat = EGL_ALPHA_FORMAT_NONPRE;
        newEglSurface->renderBuffer = EGL_SINGLE_BUFFER;
        newEglSurface->height = pixmapBitMap->high;
        newEglSurface->width = pixmapBitMap->wide;

        for(i=0; attrib_list!=NULL && attrib_list[i] != EGL_NONE; i+=2 ) {
            if(attrib_list[i] == EGL_COLORSPACE){
                newEglSurface->colorspace = attrib_list[i+1];
            }else if(attrib_list[i] == EGL_ALPHA_FORMAT){
                newEglSurface->alphaFormat = attrib_list[i+1];
            }
        }

        // No vendor surface
        newEglSurface->vendorSurface = EGL_NO_SURFACE;
        // Record our EGL config id
        newEglSurface->ourConfigId = config_id;

        kdThreadMutexUnlock(EglProcessState.SurfaceMutex);

        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    }

    PRINT2N( "eglCreatePixmapSurface: 0x%08x v0x%08x", (EGLSurface)newEglSurface, (newEglSurface != EGL_NO_SURFACE) ? (EGLSurface)newEglSurface->vendorSurface : EGL_NO_SURFACE );
    return (EGLSurface)newEglSurface;
}

EGLContext eglCreateContext( EGLDisplay dpy, EGLConfig config, EGLContext share_context,
                            const EGLint *attrib_list )
{
    EGLDisplayContext *dpyContext = NULL;
    EGLRenderingContext * newEglContext = EGL_NO_CONTEXT;
    DWORD config_id = (DWORD)config;
    #if defined ( EGL_USE_VENDOR_IMPLEMENTATION )
    BOOL useVendor = TRUE;
    #else
    BOOL useVendor = FALSE;
    #endif
    EGLint  renderableType;


    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglCreateContext : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_NO_CONTEXT;
    }

    if(!dpyContext->eglInitialized){
        WARN( "eglCreateContext : display not initialized" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_NO_CONTEXT;
    }

    // Check if this is a valid EGL config
    if( config_id<1 || config_id>TOTAL_NUMBER_OF_CONFIGS ){
        WARN ("eglCreateContext : invalid config");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONFIG));
        return EGL_NO_CONTEXT;
    }
    if( !(dpyContext->vendorConfigMap[config_id-1].localConfig->displayMask & DISPLAY(dpyContext->id)) ){
        WARN ("eglCreateContext : config is not for this display");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONFIG));
        return EGL_NO_CONTEXT;
    }
    if ( useVendor && dpyContext->vendorConfigMap[config_id-1].vendorConfigAvail ) {
        EGLContext retCtx = EGL_NO_CONTEXT;

        if(share_context == EGL_NO_CONTEXT) {
            retCtx = eglCreateContextVndr(dpyContext->vendorDisplay, dpyContext->vendorConfigMap[config_id-1].vendorConfig,
                share_context, attrib_list);
        }else if(((EGLRenderingContext *)share_context)->vendorContext != EGL_NO_CONTEXT){
            retCtx = eglCreateContextVndr(dpyContext->vendorDisplay, dpyContext->vendorConfigMap[config_id-1].vendorConfig,
                ((EGLRenderingContext *)share_context)->vendorContext, attrib_list);
        }else{
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
            return EGL_NO_CONTEXT;
        }
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)( eglGetErrorVndr()));

        // Create the context descriptor only if the vendor call succeeded
        if(retCtx != EGL_NO_CONTEXT){
            PRINTN("Vndr eglCreateContext, 0x%08x", retCtx);

            newEglContext = (EGLRenderingContext*)kdMalloc(sizeof(EGLRenderingContext));

            if(!newEglContext){
                WARN ("eglCreateContext : falied to allocate");
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));
                return EGL_NO_CONTEXT;
            }
            memset(newEglContext, 0, sizeof(EGLRenderingContext));

            //find out whether we should be binding to openGL ES 1.1 or 2.0
            eglGetConfigAttrib(dpy, config, EGL_RENDERABLE_TYPE, &renderableType);

            newEglContext->pipeline_id = (renderableType == EGL_OPENGL_ES2_BIT) ? (GL_PROGRAMMABLE_PIPELINE) :
                (GL_FIXED_PIPELINE);

            PRINTN("Pipeline id is 0x%x", newEglContext->pipeline_id);

            // Populate EGLContext Descriptor
            kdThreadMutexLock(EglProcessState.ContextMutex);

            newEglContext->vendorContext = retCtx;
            newEglContext->displayContext = dpyContext;
            newEglContext->api = (EGLenum)kdGetThreadStorageKHR(eglCurrentApiKey);
            newEglContext->markForDelete = FALSE;
            newEglContext->rendCxtInitialized = TRUE;
            newEglContext->config = config;
            newEglContext->current = FALSE;
            newEglContext->draw = EGL_NO_SURFACE;
            newEglContext->read = EGL_NO_SURFACE;

            kdThreadMutexUnlock(EglProcessState.ContextMutex);

        }
    } else {
        PRINTN("Creating context for config %d", config_id);

        if(share_context != EGL_NO_CONTEXT){
            WARN("eglCreateContext: Ignoring share_context");
        }

        // For now egl spec does not define any attributes, so we ignor them for
        // now print a warning if we get attributes
        if(attrib_list == NULL || attrib_list[0]!=EGL_NONE){
            WARN("eglCreateContext: Ignoring attrib_list");
        }
        // Try to allocate space for the context

        newEglContext = (EGLRenderingContext *)kdMalloc(sizeof(EGLRenderingContext));

        if(!newEglContext){
            WARN ("eglCreateContext : failed to allocate");
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));
            return EGL_NO_CONTEXT;
        }
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
        memset(newEglContext, 0, sizeof(EGLRenderingContext));

        // Populate EGLContext Descriptor
        kdThreadMutexLock(EglProcessState.ContextMutex);

        newEglContext->vendorContext = EGL_NO_CONTEXT;
        newEglContext->displayContext = dpyContext;
        newEglContext->api = (EGLenum)kdGetThreadStorageKHR(eglCurrentApiKey);
        newEglContext->markForDelete = FALSE;
        newEglContext->rendCxtInitialized = TRUE;
        newEglContext->config = config;
        newEglContext->current = FALSE;
        newEglContext->draw = EGL_NO_SURFACE;
        newEglContext->read = EGL_NO_SURFACE;

        kdThreadMutexUnlock(EglProcessState.ContextMutex);
    }

    PRINT2N( "eglCreateContext: 0x%08x v0x%08x", (EGLContext)newEglContext, (newEglContext != EGL_NO_CONTEXT) ? newEglContext->vendorContext : EGL_NO_CONTEXT);
    return (EGLContext)newEglContext;
}

EGLBoolean eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx)
{
    EGLDisplayContext *dpyContext = NULL;
    EGLRenderingContext *eglNewContext = ( EGLRenderingContext * )ctx;
    EGLSurfaceDescriptor *eglNewReadSurface = (EGLSurfaceDescriptor *)read;
    EGLSurfaceDescriptor *eglNewDrawSurface = (EGLSurfaceDescriptor *)draw;

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglMakeCurrent : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }
    if(!dpyContext->eglInitialized){
        WARN( "eglMakeCurrent : display not initialized" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_FALSE;
    }
    // check for valid context
    if (eglNewContext){
        if(!eglNewContext->rendCxtInitialized || eglNewContext->markForDelete) {
            WARN("Uninitialized or marked for delete context");
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONTEXT));
            return EGL_FALSE;
        }
        if ( eglNewContext->api == EGL_OPENVG_API ||
            eglNewContext->api == EGL_BITMAP_API_RIM) {
            if ( draw != read ) {
                WARN("Egl bad match");
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
                return EGL_FALSE;
            }
        }

        if( eglNewContext->draw != draw ){
            if(((EGLSurfaceDescriptor *)draw)->boundToContext ){
                WARN("draw buffer is already bound to some other context");
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ACCESS));
                return EGL_FALSE;
            }
        }
        if( eglNewContext->read != read ){
            if(((EGLSurfaceDescriptor *)read)->boundToContext && read!=draw){
                WARN("read buffer is already bound to some other context");
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ACCESS));
                return EGL_FALSE;
            }
        }
    }
    //check for walid surfaces
    if(eglNewReadSurface && eglNewReadSurface->markForDelete){
        WARN("Read surface marked for delete");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }
    if(eglNewDrawSurface && eglNewDrawSurface->markForDelete){
        WARN("Draw surface marked for delete");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }
    if( pBufferFrontSurface[dpyContext->id] != NULL &&
        (pBufferFrontSurface[dpyContext->id] == (EGLSurfaceDescriptor *)read
        || pBufferFrontSurface[dpyContext->id] ==(EGLSurfaceDescriptor *)draw)){
            LV1( LOG_EGL, PRINT("MakeCurrent for EGL_PBUFFER_FRONT_RIM"));
            pBufferFrontContext[dpyContext->id] = (EGLRenderingContext *)ctx;
    }
    //check if we need to delete curent contexts
    if(((EGLRenderingContext*)kdGetThreadStorageKHR(eglCurrentContextKey))){
        //we are switching out of the current context, so we have to check if any
        //cleanup is needed
        EGLRenderingContext * currContext = (EGLRenderingContext*)kdGetThreadStorageKHR(eglCurrentContextKey);
        EGLSurfaceDescriptor * currDrawSurfDesc = (EGLSurfaceDescriptor *)currContext->draw;
        EGLSurfaceDescriptor * currReadSurfDesc = (EGLSurfaceDescriptor *)currContext->read;
        
        if( currDrawSurfDesc->markForDelete ) {
            PRINT("deleting draw surface");
            deleteSurface( currDrawSurfDesc );
            currContext->draw = EGL_NO_SURFACE;
        }
        if( currReadSurfDesc != currDrawSurfDesc && currReadSurfDesc->markForDelete ) {
            PRINT("deleting read surface");
            deleteSurface( currReadSurfDesc);
            currContext->read = EGL_NO_SURFACE;
        }
        if( currContext->markForDelete ) {
            PRINT("Deleting context");
            deleteContext( currContext );
            kdSetThreadStorageKHR(eglCurrentContextKey, (void *)(EGL_NO_CONTEXT));
        }
    }
    //special case when called with all paraeters NULL
    if( eglNewContext == EGL_NO_CONTEXT){

        if( draw == EGL_NO_SURFACE && read == EGL_NO_SURFACE ){
            if (((EGLRenderingContext*)kdGetThreadStorageKHR(eglCurrentContextKey)) != EGL_NO_CONTEXT ) {
                ((EGLRenderingContext*)kdGetThreadStorageKHR(eglCurrentContextKey))->current = FALSE;  //change state of old context to "not current"
                if(((EGLRenderingContext*)kdGetThreadStorageKHR(eglCurrentContextKey))->draw){
                    ((EGLSurfaceDescriptor *)((EGLRenderingContext*)kdGetThreadStorageKHR(eglCurrentContextKey))->draw)->boundToContext = FALSE;
                    ((EGLRenderingContext*)kdGetThreadStorageKHR(eglCurrentContextKey))->draw = NULL;
                }
                if(((EGLRenderingContext*)kdGetThreadStorageKHR(eglCurrentContextKey))->read){
                    ((EGLSurfaceDescriptor *)((EGLRenderingContext*)kdGetThreadStorageKHR(eglCurrentContextKey))->read)->boundToContext = FALSE;
                    ((EGLRenderingContext*)kdGetThreadStorageKHR(eglCurrentContextKey))->read = NULL;
                }
            }
            kdSetThreadStorageKHR(eglCurrentContextKey, (void *)EGL_NO_CONTEXT);
#if defined ( EGL_USE_VENDOR_IMPLEMENTATION )
            if ( (EGLenum)kdGetThreadStorageKHR(eglCurrentApiKey) == EGL_BITMAP_API_RIM ) {
                return EGL_TRUE;
            } else {
                return eglMakeCurrentVndr( dpy, NULL, NULL, NULL );
            }
#else
            return EGL_TRUE;
#endif
        }
        else{
            WARN("Switching to EGL_NO_CONTEXT with valid buffers");
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
            return EGL_FALSE;
        }
    }
    else{
        if( draw == EGL_NO_SURFACE || read == EGL_NO_SURFACE ){
            WARN("Switching to EGL_NO_SURFACE with valid context");
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
            return EGL_FALSE;
        }
    }

#if defined ( EGL_USE_VENDOR_IMPLEMENTATION )
    if(!eglMakeCurrentVndr( dpy, draw, read, ctx )){
        return EGL_FALSE;
    }
#endif
    // bind context to current rendering thread,
    // draw and read surfaces

    eglNewContext->userTask = (DWORD)kdThreadSelf();

    eglNewContext->current = TRUE;

    if(eglNewContext->draw){
        //Unbind the previous draw surface
        ((EGLSurfaceDescriptor *)eglNewContext->draw)->boundToContext = FALSE;
    }
    eglNewContext->draw = draw;

    ((EGLSurfaceDescriptor *)eglNewContext->draw)->boundToContext = TRUE;

    if(eglNewContext->read){
        //Unbind the previous read surface
        ((EGLSurfaceDescriptor *)eglNewContext->read)->boundToContext = FALSE;
    }
    eglNewContext->read = read;
    ((EGLSurfaceDescriptor *)eglNewContext->read)->boundToContext = TRUE;

    //if the context passed in is not the current context, then update current
    //context to reflect the new context

    if( eglNewContext != ((EGLRenderingContext*)kdGetThreadStorageKHR(eglCurrentContextKey)) ){
        EGLRenderingContext *currentContext = (EGLRenderingContext*)kdGetThreadStorageKHR(eglCurrentContextKey);
        PRINT("Replacing old current context with new context");

        if (currentContext != EGL_NO_CONTEXT ) {
            EGLSurfaceDescriptor *oldSurface;
            currentContext->current = FALSE;  //change state of old context to "not current"
            oldSurface = (EGLSurfaceDescriptor *)currentContext->draw;
            if( oldSurface != NULL ) {
                oldSurface ->boundToContext = FALSE;
                currentContext->draw = NULL;
            }
            oldSurface = (EGLSurfaceDescriptor *)currentContext->read;
            if( oldSurface != NULL ) {
                oldSurface->boundToContext = FALSE;
                currentContext->read = NULL;
            }
        }
        kdSetThreadStorageKHR(eglCurrentContextKey, (void *)eglNewContext);
    }
    return EGL_TRUE;
}

EGLSurface eglCreatePbufferFromClientBuffer( EGLDisplay dpy, EGLenum buftype,
    EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list )
{
    EGLDisplayContext *dpyContext = NULL;
#if defined( EGL_USE_VENDOR_IMPLEMENTATION )
    EGLSurfaceDescriptor * newEglSurface = EGL_NO_SURFACE;
#endif
    EGLConfigContextPtr currentConfigContext;
    DWORD config_id = (DWORD)config;

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglCreatePbufferFromClientBuffer : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_NO_SURFACE;
    }

    if(!dpyContext->eglInitialized){
        WARN( "eglCreatePbufferFromClientBuffer : display not initialized" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_NO_SURFACE;
    }

    // Check if this is a valid EGL config
    if( config_id < 1 || config_id > TOTAL_NUMBER_OF_CONFIGS){
        WARN ("eglCreatePbufferFromClientBuffer : invalid config");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONFIG));
        return EGL_NO_SURFACE;
    }

    if (!(dpyContext->vendorConfigMap[config_id-1].localConfig->displayMask & DISPLAY(dpyContext->id))){
        WARN ("eglCreatePbufferFromClientBuffer : config is not for this display");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONFIG));
        return EGL_NO_SURFACE;
    }

    currentConfigContext = dpyContext->vendorConfigMap[config_id-1].localConfig->ourConfig;

    // Check if the config supports rendering to pbuffers
    if(!(currentConfigContext[CONFIG_ELEMENT(EGL_SURFACE_TYPE)] & EGL_PBUFFER_BIT)){
        // The choosen config does not support rendering to pbuffers
        WARN ("eglCreatePbufferFromClientBuffer : config does not support rendering PBuffers");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
        return EGL_NO_SURFACE;
    }

#if defined( EGL_USE_VENDOR_IMPLEMENTATION )
    if( dpyContext->vendorConfigMap[config_id-1].vendorConfigAvail ){
        EGLSurface retSurf = EGL_NO_SURFACE;
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONFIG));

        PRINT("Vndr eglCreatePbufferFromClientBuffer");
        retSurf = eglCreatePbufferFromClientBufferVndr( dpyContext->vendorDisplay, buftype,
            buffer, dpyContext->vendorConfigMap[config_id-1].vendorConfig, attrib_list );

        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVndr()));

        if( retSurf != EGL_NO_SURFACE ){
            newEglSurface = (EGLSurfaceDescriptor*)kdMalloc(sizeof(EGLSurfaceDescriptor));

            if( !newEglSurface ) {
                eglDestroySurfaceVndr(dpyContext->vendorDisplay, retSurf);
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ALLOC));
                return EGL_NO_SURFACE;
            }
            initializeSurface(dpy, newEglSurface, EGL_PBUFFER_BIT);
            newEglSurface->ownsBackBuffer = FALSE;
            newEglSurface->vendorSurface = retSurf;
            newEglSurface->ourConfigId = config_id;
        }

        return (EGLSurface)newEglSurface;
    }
#endif

    // Client buffers are not supported if vendors do not support them
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));

    return EGL_NO_SURFACE;
}

EGLContext eglGetCurrentContext( void )
{
    LV1(LOG_EGL, PRINT("Returning current context"));
    return (EGLContext) (((EGLRenderingContext*)kdGetThreadStorageKHR(eglCurrentContextKey)));

}

EGLDisplay eglGetCurrentDisplay( void )
{
    PRINT("Returning current display");
    if ( ((EGLRenderingContext*)kdGetThreadStorageKHR(eglCurrentContextKey)) == EGL_NO_CONTEXT ) {
        return EGL_NO_DISPLAY;
    }
    else {
        return ((EGLRenderingContext*)kdGetThreadStorageKHR(eglCurrentContextKey))->displayContext->handle;
    }
}

EGLBoolean eglDestroySurface( EGLDisplay dpy, EGLSurface surface )
{

    EGLDisplayContext *dpyContext = NULL;
    EGLSurfaceDescriptor * surfaceDesc = (EGLSurfaceDescriptor *)surface;
    EGLBoolean retValue = EGL_TRUE;

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglDestroySurface : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    if(!dpyContext->eglInitialized){
        WARN( "eglDestroySurface : display not initialized" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_FALSE;
    }

    if( surface == EGL_NO_SURFACE ) {
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    if( surfaceDesc->vendorSurface != EGL_NO_SURFACE &&
        surfaceDesc->vendorSurface != surfaceDesc )
    {
        //Desroying vendor specific surface
#if defined( EGL_USE_VENDOR_IMPLEMENTATION )
        PRINT( "Vdr eglDestroySurface" );
        retValue = eglDestroySurfaceVndr( dpyContext->vendorDisplay, surfaceDesc->vendorSurface );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVndr()));
#else
        retValue = EGL_FALSE;
#endif
    }

    if( retValue ) {
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

        kdThreadMutexLock(EglProcessState.SurfaceMutex);

        surfaceDesc->markForDelete = TRUE;
#ifndef RIM_WINDOW_MANAGER
        // If it is a window surface unbind the surface from the native window
        if(surfaceDesc->nativeWindow){
            surfaceDesc->nativeWindow->boundToEGLSurface = FALSE;
        }
#endif // RIM_WINDOW_MANAGER
        if( !surfaceDesc->boundToContext ) {           
            
            PRINTN( "eglDestroySurface: destroying 0x%08x", (DWORD)surface );
            deleteSurface( surfaceDesc );
        }

        kdThreadMutexUnlock(EglProcessState.SurfaceMutex);

    } else {
        WARN( "Vendor eglDestroySurface failed" );
        PRINTN( "Error 0x%04x", (EGLint)kdGetThreadStorageKHR(eglLastErrorKey));
        return EGL_FALSE;
    }
    return retValue;
}

EGLBoolean eglDestroyContext( EGLDisplay dpy, EGLContext ctx )
{
    EGLDisplayContext *dpyContext = NULL;
    EGLRenderingContext * context = (EGLRenderingContext *)ctx;
    EGLBoolean retValue = EGL_TRUE;

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));


    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglDestroyContext : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    if(!dpyContext->eglInitialized){
        WARN( "eglDestroyContext : display not initialized" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_FALSE;
    }

    if( ctx == EGL_NO_CONTEXT ){
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_CONTEXT));
        return EGL_FALSE;
    }

    if( context->vendorContext != EGL_NO_CONTEXT && context->vendorContext != context ){
#if defined ( EGL_USE_VENDOR_IMPLEMENTATION )
        PRINT( "Vdr eglDestroyContext" );
        retValue = eglDestroyContextVndr( dpyContext->vendorDisplay, context->vendorContext );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVndr()));
#else
        retValue = EGL_FALSE;
#endif
    }

    if( retValue ) {
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
        kdThreadMutexLock(EglProcessState.ContextMutex);

        context->markForDelete = 1;

        //only delete context if it is not current
        if( !context->current ) {
            PRINTN( "eglDestroyContext: destroying 0x%08x", (DWORD)ctx );
            deleteContext( context );
        }
        kdThreadMutexUnlock(EglProcessState.ContextMutex);

    } else {
        WARN( "Vendor eglDestroyContext failed" );
        PRINTN( "Error 0x%04x",  (EGLint)kdGetThreadStorageKHR(eglLastErrorKey));
        return EGL_FALSE;
    }
    return retValue;
}

EGLSurface eglGetCurrentSurface(EGLint readdraw)
{

    LV1(LOG_EGL, PRINT("eglgetcurrentsurface"));

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    if ( !((EGLRenderingContext*)kdGetThreadStorageKHR(eglCurrentContextKey)) ) {
        return EGL_NO_SURFACE;
     }

    switch(readdraw) {
        case EGL_READ:
            return ((EGLRenderingContext*)kdGetThreadStorageKHR(eglCurrentContextKey))->read;

        case EGL_DRAW:
            return ((EGLRenderingContext*)kdGetThreadStorageKHR(eglCurrentContextKey))->draw;

        default :
            return EGL_NO_SURFACE;
        }

}

EGLBoolean eglQueryContext(EGLDisplay dpy, EGLContext ctx,
                                   EGLint attribute, EGLint *value)
{
    EGLRenderingContext * context = (EGLRenderingContext *)ctx;
    EGLDisplayContext *dpyContext = NULL;
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglQueryContext : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    if(!dpyContext->eglInitialized){
        WARN( "eglQueryContext : display not initialized" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_FALSE;
    }

    if( pBufferFrontContext[dpyContext->id] != NULL && pBufferFrontContext[dpyContext->id] == (EGLRenderingContext *)ctx){
        if(attribute == EGL_RENDER_BUFFER){
            *value = (EGLint)(pBufferFrontSurface[dpyContext->id]->backBuffer.data);
        }
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
        return EGL_TRUE;
    }else{
#if defined ( EGL_USE_VENDOR_IMPLEMENTATION )
        EGLBoolean ret;
        switch(attribute){
            case EGL_CONFIG_ID:
                *value = (EGLint)(context->config);
                return EGL_TRUE;
            default:
                ret = eglQueryContextVndr( dpyContext->vendorDisplay, context->vendorContext, attribute, value);
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVndr()));
                return ret;
        }
#else
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

        switch(attribute){
            case EGL_CONFIG_ID:
                *value = (EGLint)(context->config);
                return EGL_TRUE;
            // NOTE: Should return EGL_SINGLE_BUFFER or EGL_BACK_BUFFER when JVM pBuffer hack is removed
            case EGL_RENDER_BUFFER:
                if (context->current){
                    *value = (EGLint)((EGLSurfaceDescriptor *)(context->draw))->backBuffer.data;
                }
                else{
                    *value = EGL_NONE;
                }
                return EGL_TRUE;
            default:
                kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_ATTRIBUTE));
                return EGL_FALSE;
            }
#endif
    }
}

IPIFUNC EGLBoolean eglGetCurrentDisplayIdRim( DisplayId_t *displayId ) {
    if(((EGLRenderingContext*)kdGetThreadStorageKHR(eglCurrentContextKey)) != EGL_NO_CONTEXT){
        *displayId = ((EGLRenderingContext*)kdGetThreadStorageKHR(eglCurrentContextKey))->displayContext->id;
    }else{
        LcdGetDisplayId( PRIMARY_DISPLAY, displayId);
    }
    return EGL_TRUE;
}

void eglResetDirtyRegion( DisplayId_t displayId )
{
    LcdConfig *lcdCfg;

    lcdCfg = LcdGetLcdConfigPtr( (BYTE)displayId );
    if ( pBufferFrontSurface[displayId] != NULL ) {
        PRINT4N("Setting dirty region to %d %d %d %d",
            0, 0, lcdCfg->width, lcdCfg->height);

        pBufferFrontSurface[displayId]->dirtyRegion.x = 0;
        pBufferFrontSurface[displayId]->dirtyRegion.y = 0;
        pBufferFrontSurface[displayId]->dirtyRegion.width = lcdCfg->width;
        pBufferFrontSurface[displayId]->dirtyRegion.height = lcdCfg->height;

    }
}

EGLBoolean eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    EGLDisplayContext *dpyContext = NULL;
    EGLBoolean retValue = EGL_FALSE;
#if defined ( EGL_USE_VENDOR_IMPLEMENTATION )
    EGLSurfaceDescriptor * surfaceDesc = (EGLSurfaceDescriptor *)surface;
#endif

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));


    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglBindTexImage : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    if(!dpyContext->eglInitialized){
        WARN( "eglBindTexImage : display not initialized" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_FALSE;
    }

#if defined ( EGL_USE_VENDOR_IMPLEMENTATION )
    PRINT("Vndr eglBindTexImage");
    retValue = eglBindTexImageVndr( dpyContext->vendorDisplay, surfaceDesc->vendorSurface, buffer );
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVndr()));
#endif

    return retValue;
}

EGLBoolean eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    EGLDisplayContext *dpyContext = NULL;
    EGLBoolean retValue = EGL_FALSE;
#if defined ( EGL_USE_VENDOR_IMPLEMENTATION )
    EGLSurfaceDescriptor * surfaceDesc = (EGLSurfaceDescriptor *)surface;
#endif
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglReleaseTexImage : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    if(!dpyContext->eglInitialized){
        WARN( "eglReleaseTexImage : display not initialized" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_FALSE;
    }

#if defined ( EGL_USE_VENDOR_IMPLEMENTATION )
    PRINT( "Vndr eglReleaseTexImage" );
    retValue = eglReleaseTexImageVndr( dpyContext->vendorDisplay, surfaceDesc->vendorSurface, buffer );
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVndr()));
#endif

    return retValue;
}

EGLBoolean eglSwapInterval(EGLDisplay dpy, EGLint interval)
{
    EGLDisplayContext *dpyContext = NULL;
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    // Get the EGLDisplayContext
    dpyContext = eglGetDisplayContextInt( dpy );
    if(!dpyContext){
        WARN( "eglSwapInterval : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    if(!dpyContext->eglInitialized){
        WARN( "eglSwapInterval : display not initialized" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

EGLBoolean eglWaitNative(EGLint engine)
{
    //not sure how to implement yet... just return EGL_TRUE
    return EGL_TRUE;
}

EGLBoolean eglWaitGL(void)
{
    EGLBoolean retValue = EGL_FALSE;

#if defined ( EGL_USE_VENDOR_IMPLEMENTATION )
    PRINT("Vndr eglWaitGL");
    retValue = eglWaitGLVndr();
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVndr()));
#endif
    return retValue;
}

EGLBoolean eglReleaseThread(void)
{
    EGLBoolean retValue = EGL_FALSE;

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));
    kdSetThreadStorageKHR(eglCurrentApiKey, (void *)(EGL_BITMAP_API_RIM));
    kdSetThreadStorageKHR(eglCurrentContextKey, (void *)(EGL_NO_CONTEXT));

#if defined ( EGL_USE_VENDOR_IMPLEMENTATION )
    PRINT("Vndr eglReleaseThread");
    retValue = eglReleaseThreadVndr();
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(eglGetErrorVndr()));
#else
    WARN("Vendor implementation has not been used. ");
    retValue = EGL_TRUE;
#endif
    return retValue;
}

EGLBoolean eglResizeWindowSurfaceRim( EGLDisplay dpy, EGLSurface surface, EGLint width, EGLint height )
{
#ifdef RIM_WINDOW_MANAGER
    EGLSurfaceDescriptor * surfaceDesc   = (EGLSurfaceDescriptor *)surface;
    EGLDisplayContext *dpyContext        = NULL;

    PRINT2N("eglResizeWindowSurfaceRim: w=%d h=%d", width, height);

    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    dpyContext = eglGetDisplayContextInt( dpy );
    if( !dpyContext ) {
        WARN( "eglResizeWindowSurfaceRim : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    if( !surface ) {
        WARN("no egl surface");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    if( surfaceDesc->surfaceLocked ) {
        WARN("surfaceLocked, can not resize");
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    // We call EglIntPostSwap() here to re-size the backbuffer.  The frontbuffer will get resized 
    // upon calling eglSwapBuffers().
    return EglIntPostSwap( dpy, surfaceDesc, width, height, EGL_RIM_CMD_POST_SWAP_SKIP_FB_DESTROY );

#else // RIM_WINDOW_MANAGER
    WARN("not defined RIM_WINDOW_MANAGER, can not call eglResizeWindowSurfaceRIM");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
    return EGL_FALSE;
#endif // RIM_WINDOW_MANAGER
}

EGLBoolean eglSwapMultipleBuffersRim( EGLDisplay dpy, EGLSurface *surfaces, EGLContext *ctxs )
{
#ifdef RIM_WINDOW_MANAGER
    EGLBoolean retValue = EGL_TRUE;
    EGLDisplayContext *dpyContext;

    EGLDisplay oldDpy;
    EGLContext oldCtx;
    EGLSurface oldReadSurface;
    EGLSurface oldDrawSurface;
    int i;

    // reset last error
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_SUCCESS));

    // check if display is valid
    dpyContext = eglGetDisplayContextInt( dpy );
    if( !dpyContext ) {
        WARN( "eglSwapMultipleBuffersRim : bad display" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    if( !dpyContext->eglInitialized ) {
        WARN( "eglSwapMultipleBuffersRim : display not initialized" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_NOT_INITIALIZED));
        return EGL_FALSE;
    }

    // validate array arguments
    if( surfaces == NULL ) {
        WARN( "eglSwapMultipleBuffersRim : surface array is null" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_PARAMETER));
        return EGL_FALSE;
    }

    if( ctxs == NULL ) {
        WARN( "eglSwapMultipleBuffersRim : context array is null" );
        kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_PARAMETER));
        return EGL_FALSE;
    }

    // validate array lengths
    i = 0;
    while( TRUE ) {

        if( surfaces[i] == EGL_NO_SURFACE && ctxs[i] == EGL_NO_CONTEXT ) {

            // arrays are equal length
            break;

        } else if ( ( surfaces[i] == EGL_NO_SURFACE && ctxs[i] != EGL_NO_CONTEXT ) || 
                    ( surfaces[i] != EGL_NO_SURFACE && ctxs[i] == EGL_NO_CONTEXT ) ) {
            
            WARN( "eglSwapMultipleBuffersRim : surface and context arrays not same length" );
            kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_PARAMETER));
            return EGL_FALSE;
        }

        i++;
    }

    // save current display, context, and surfaces
    oldDpy = eglGetCurrentDisplay();
    oldCtx = eglGetCurrentContext();
    oldReadSurface = eglGetCurrentSurface( EGL_READ );
    oldDrawSurface = eglGetCurrentSurface( EGL_DRAW );

    // clear save context and surfaces if any marked for deletion
    if( ( oldCtx != EGL_NO_CONTEXT && ((EGLRenderingContext*)oldCtx)->markForDelete ) || 
        ( oldReadSurface != EGL_NO_SURFACE && ((EGLSurfaceDescriptor*)oldReadSurface)->markForDelete ) || 
        ( oldDrawSurface != EGL_NO_SURFACE && ((EGLSurfaceDescriptor*)oldDrawSurface)->markForDelete ) ) {

        oldCtx = EGL_NO_CONTEXT;
        oldReadSurface = EGL_NO_SURFACE;
        oldDrawSurface = EGL_NO_SURFACE;
    }

    // begin swap group in window manager
    WMIntBeginSwapGroup( dpyContext->id );

    // visit each context/surface
    i = 0;
    while( TRUE ) {

        // check if end of arrays reached
        if( surfaces[i] == EGL_NO_SURFACE && ctxs[i] == EGL_NO_CONTEXT ) {
            break;
        }

        // make context and surface current
        if( !eglMakeCurrent( dpy, surfaces[i], surfaces[i], ctxs[i] ) ) {
            // note eglMakeCurrent will set the last error
            WARN( "eglSwapMultipleBuffersRim : eglMakeCurrent failed" );
            retValue = EGL_FALSE;
            break;
        }

        // swap current surface
        if( !eglSwapBuffers( dpy, surfaces[i] ) ) {
            // note eglSwapBuffers will set the last error
            WARN( "eglSwapMultipleBuffersRim : eglSwapBuffers failed" );
            retValue = EGL_FALSE;
            break;
        }

        i++;
    }

    // end swap group in window manager
    WMIntEndSwapGroup( dpyContext->id );

    // restore current display, context, and surface
    oldDpy = (oldDpy == EGL_NO_DISPLAY) ? eglGetCurrentDisplay() : oldDpy;
    eglMakeCurrent( oldDpy, oldDrawSurface, oldReadSurface, oldCtx );

    return retValue;

#else // RIM_WINDOW_MANAGER
    WARN("not defined RIM_WINDOW_MANAGER, can not call eglSwapMultipleBuffersRim");
    kdSetThreadStorageKHR(eglLastErrorKey, (void *)(EGL_BAD_MATCH));
    return EGL_FALSE;
#endif // RIM_WINDOW_MANAGER
}
