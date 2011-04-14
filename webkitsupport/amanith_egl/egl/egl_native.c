/******************************************************************************
 * Filename:      egl_native.c
 *
 * Copyright 2005, Research In Motion Ltd
 *
 * Author:        Russell Andrade
 *
 * Created:       Oct 22, 2007
 *
 * Description:   Implementation of EGL native (i.e glue) layer
 *
 *****************************************************************************/

#if defined(RIM_NDK)
#include <egl_user_types.h>
#include <graphicsinit.h>
#endif

#include <basetype.h>
#include <lcd.h>

#if !defined (RIM_NDK)
#include <i_lcd.h>
#include <bugdispc.h>
#include <log_verbosity.h>
#include <graphics_mempool.h>
#include <i_graphics_mempool.h>

#define SRCFILE     FILE_EGL_NATIVE
#define SRCGROUP    GROUP_GRAPHICS
#endif

#include <nativewin.h>
#include <bitmap.h>
#include <egl.h>
#include <eglplatform.h>
#include <egl_native.h>
#include <egl_globals.h>
#include <kd.h>

#ifdef RIM_WINDOW_MANAGER
#include "windowmanager.h"
#include "i_windowmanager.h"
#else
#endif

#ifndef RIM_WINDOW_MANAGER
/**
 * Global NativeWindow Table
 * Contains pointers to all the available NativeWindows on the system. If a certain slot is not active, it is
 * represented by a NULL pointer in the array, otherwise it contains a pointer to a NativeWindow struct.
 * Each Native Window Handle (NativeWindowType) maps directly to one of the slots in this table.
 * A valid Native Window Handle can be translated to a NativeWindow* by using NativeTranslateHandleInt().
 */
extern NativeWindow*   globalNativeWindows[RIM_LCD_NUM_DISPLAYS][NATIVE_WIN_MAX_WINDOWS];
BYTE            globalNativeWindowCounter = 0;
#endif // RIM_WINDOW_MANAGER

/*!
*******************************************************************************
*
*       Function used to get native display info, native implementation
*               should fill the information structure when EGL call it.
* param[in]  display_id      - Native display id.
* param[out] pDisplayInfo    - Pointer to the display information structure.
* return
* EGL_TRUE for success, otherwise return EGL_FALSE.
******************************************************************************/
IPIFUNC EGLBoolean nativeGetDisplayInfo(
    MarvellNativeDisplayType display_id,
    _NATIVE_DISPLAY_INFO_T* pDisplayInfo)
{
    LcdConfig* config;
    DisplayId_t lcdId = (DisplayId_t)((DWORD)display_id & 0x000000ff);

    PRINT("Native get display info");

    config = LcdGetLcdConfigPtr(lcdId);

    pDisplayInfo->display_id = display_id;
    //pDisplayInfo->format = NATIVE_PIXFMT_RGB565;
    pDisplayInfo->format = ( config->depth == 16 ) ? ( NATIVE_PIXFMT_RGB565 )  : ( NATIVE_PIXFMT_ARGB8888 );
    pDisplayInfo->height = config->height;
    pDisplayInfo->width = config->width;
    pDisplayInfo->horzResolution = config->widthInMicrons * 1000; //convert to millimetres
    pDisplayInfo->vertResolution = config->heightInMicrons * 1000; //convert to millimetere
    pDisplayInfo->pixelAspectRatio = (pDisplayInfo->vertResolution != 0) ?
        (pDisplayInfo->horzResolution / pDisplayInfo->vertResolution) : 0;

    return EGL_TRUE;
}

/*!
*******************************************************************************
* \brief
*       Function to initialize native, native implementation can do any
*               initialization work they need here.
* \param[in]  pInitInfo - Pointer to initialization information structure.
* \return
*     - None
******************************************************************************/
IPIFUNC EGLBoolean nativeInitialize(
    _NATIVE_INIT_INFO_T* pInitInfo)
{
    //no specific initialization work necessary
    PRINT("Native initialize");
    return TRUE;

}


/*!
*******************************************************************************
* \brief
*       Function to finalize native, native implementation can do any
*               finalization work they need here.
* \param[in]  display - Native display handle.
* \return
*     - None
******************************************************************************/
IPIFUNC void nativeFinalize()
{
    // Nothing to do for win32 native
    PRINT("Native finalize");
}


/*!
*******************************************************************************
* \brief
*       Function to get native window surface information.
*
* \param[in]  nativeWin       - Native window handle.
* \param[out] pWinSurfaceInfo - Pointer to native window surface information.
* \return
*     - EGL_TRUE means success, else return EGL_FALSE.
******************************************************************************/
IPIFUNC EGLBoolean nativeGetWinSurfaceInfo(
    MarvellNativeWindowType nativeWin,
    _NATIVE_WIN_SURFACE_INFO_T* pWinSurfaceInfo)
{
#ifdef RIM_WINDOW_MANAGER
    WMWindow_t      nativeWindowRim = (WMWindow_t)nativeWin;
    WMError_t       result;
    DWORD           bitDepthBytes;
    NativePixelFormat pixelFormat;
    SDWORD winContext[1];
    EGLSurfaceDescriptor * surfaceDesc = NULL;

    result = WMIntGetWindowPropertyiv( nativeWindowRim, WM_PROPERTY_USER_CONTEXT, winContext );
    if( result != WM_E_OK ) {
        WARN("Failed to get window context info");
        return EGL_FALSE;
    }
    
    surfaceDesc = (EGLSurfaceDescriptor *)winContext[0];
    if ( !surfaceDesc ) {
        WARN("nativeGetWinSurfaceInfo: missing property context?!");
        return EGL_FALSE;
    }

    PRINT("Native get surface info");

    if (surfaceDesc->bitDepth == 0) {
        kdCatfailRim("No bit depth for EGL front buffer");
        return EGL_FALSE;
    }

    // convert bimap format to pixel format
    switch(surfaceDesc->frontBuffer.bType) {
        case BMT_32BPP_ARGB8888:
            pixelFormat = NATIVE_PIXFMT_ARGB8888;
            break;
        case BMT_32BPP_RGBA8888:
            pixelFormat = NATIVE_PIXFMT_RGBA8888;
            break;
        case BMT_16BPP_RGB565:
            pixelFormat = NATIVE_PIXFMT_RGB565;
            break;
        default:
            WARN("Unsupported pixel format");
            return EGL_FALSE;
    }

    bitDepthBytes = (surfaceDesc->bitDepth / 8);

    // NOTE: need to flip data vertically so use origin of lower left and
    // negative stride
    pWinSurfaceInfo->bBackBuffer = FALSE;
    pWinSurfaceInfo->format = pixelFormat;
    pWinSurfaceInfo->width = surfaceDesc->width;
    pWinSurfaceInfo->height = surfaceDesc->height;
    pWinSurfaceInfo->xStride = bitDepthBytes;
    pWinSurfaceInfo->yStride = -(( (pWinSurfaceInfo->width * bitDepthBytes) + 7 ) & ~7);

    if ( EglUseNewSwapLogic ) {
        pWinSurfaceInfo->pPrivate = (void*)surfaceDesc;
    } else {
        pWinSurfaceInfo->pPrivate = (void*)nativeWindowRim;
    }

    PRINT4N("Returning %d %d %d %d", pWinSurfaceInfo->height, pWinSurfaceInfo->width,
        pWinSurfaceInfo->xStride, pWinSurfaceInfo->yStride);

    return EGL_TRUE;
#else
    DisplayId_t     id;
    LcdConfig      *lcdCfg;
    NativeWindow   *nativeWindowRim = NativeTranslateHandleInt((NativeWindowType)nativeWin);

    if(!nativeWindowRim){
        WARNN("Invalid window handle 0x%08x",nativeWin);
        return EGL_FALSE;
    }

    id = nativeWindowRim->display;
    lcdCfg = LcdGetLcdConfigPtr( id );

    PRINT("Native get surface info");

    // NOTE: need to flip data vertically so use origin of lower left and
    // negative stride


    pWinSurfaceInfo->bBackBuffer = FALSE;
    pWinSurfaceInfo->format =  ( lcdCfg->depth == 16 ) ? ( NATIVE_PIXFMT_RGB565 )  : ( NATIVE_PIXFMT_ARGB8888 );
    pWinSurfaceInfo->height = lcdCfg->height;
    pWinSurfaceInfo->width = lcdCfg->width;
    pWinSurfaceInfo->xStride = ( pWinSurfaceInfo->format == NATIVE_PIXFMT_RGB565 ) ? 2 : 4;
    pWinSurfaceInfo->yStride = -(lcdCfg->width * pWinSurfaceInfo->xStride);
    pWinSurfaceInfo->pPrivate = NULL;
        

    PRINT4N("Returning %d %d %d %d", pWinSurfaceInfo->height, pWinSurfaceInfo->width,
        pWinSurfaceInfo->xStride, pWinSurfaceInfo->yStride);

    PRINT2N("Format =  %d , stride = %d ", pWinSurfaceInfo->format, pWinSurfaceInfo->xStride );

    return EGL_TRUE;
#endif // RIM_WINDOW_MANAGER
}

/*!
*******************************************************************************
* \brief
*       Function to release native window surface information
*
* \param[in]  nativeWin - Native window handle.
* \param[in]  pWinSurfaceInfo - Pointer to native window surface information.
* \return
*     - None
******************************************************************************/
IPIFUNC void nativeReleaseWinSurfaceInfo(
    MarvellNativeWindowType nativeWin,
    _NATIVE_WIN_SURFACE_INFO_T* pWinSurfaceInfo)
{
    //if we allocate data in pWinSurfaceInfo->private then release
    //not doing it now, so don't need to do anything
    PRINT("Native release surface info");

}

/*!
*******************************************************************************
* \brief
*       Function to get native window surface linear address.
*
* \param[in]  naiveWin        - Native window handle.
* \param[in]  pWinSurfaceInfo - Pointer to native window surface information.
*   \param[in]  pLockData       - Pointer to lock data structure.
* \return
*     - EGL_TRUE means success, and the lock data has been filled to
*               correct values.
*           -   EGL_FALSE means fail, the values in lock data are undefined.
*   \note
*               Native implementation can choose to return real address on lock,
*               or return another buffer address on lock and perform real copy
*               back to window surface on unlock.
******************************************************************************/
IPIFUNC EGLBoolean nativeLockWinSurface(
    MarvellNativeWindowType  nativeWin,
    _NATIVE_WIN_SURFACE_INFO_T* pWinSurfaceInfo,
    _NATIVE_LOCK_DATA_T* pLockData)
{
#ifdef RIM_WINDOW_MANAGER
    BitMap          buffer;

    if ( EglUseNewSwapLogic ) {
        EGLSurfaceDescriptor * surfaceDesc = (EGLSurfaceDescriptor *)pWinSurfaceInfo->pPrivate;

        PRINTN( "NEW: Native lock win surface (surf = 0x%08x)", surfaceDesc );
        if ( surfaceDesc ) {
            kdThreadMutexLock(EglProcessState.SurfaceMutex);
            memcpy(&buffer, &surfaceDesc->backBuffer, sizeof(buffer));
            if ( !buffer.data ) {
                WARN( "nativeLockWinSurface - no backbuffer?" );
                kdThreadMutexUnlock(EglProcessState.SurfaceMutex);
                return EGL_FALSE;
            }
        } else {
            WARN( "nativeLockWinSurface - no surface?" );
            return EGL_FALSE;
        }
    } else {
        WMWindow_t      nativeWindowRim;
        WMError_t       result;

        PRINT( "OLD: Native lock win surface" );

        // get window from surface info
        nativeWindowRim = (WMWindow_t)pWinSurfaceInfo->pPrivate;

        // lock window buffer
        result = WMIntLockWindowBuffer(nativeWindowRim, &buffer);
        if (result != WM_E_OK) {
            WARN("Failed to lock window buffer");
            return EGL_FALSE;
        }
    }

    // NOTE: need to flip data vertically so use origin of lower left and
    // negative stride
    pLockData->bSurfaceNotChanged = EGL_FALSE;
    pLockData->pData = buffer.data + ( (pWinSurfaceInfo->height-1) * -pWinSurfaceInfo->yStride );
    if (!pLockData->pData) {
        WARN("Failed to lock window buffer");
        return EGL_FALSE;
    }

    PRINTN("Returning pointer to 0x%x", (DWORD)pLockData->pData );

    return EGL_TRUE;
#else
#if !defined(RIM_NDK)
    WORD           *lcdBuffer;
    DWORD           lcdStride;
    DWORD           lcdFormat;
#endif    
    DisplayId_t     id;
    NativeWindow   *nativeWindowRim = NativeTranslateHandleInt((NativeWindowType)nativeWin);

    if(!nativeWindowRim){
        WARNN("Invalid window handle 0x%08x",nativeWin);
        return EGL_FALSE;
    }

    id = nativeWindowRim->display;

    PRINTN("Native lock win surface id = %d", id);

    // NOTE: need to flip data vertically so use origin of lower left and
    // negative stride
#if !defined (RIM_NDK)
    LcdGetBuffer( id, LCD_FRONT_BUFFER, &lcdBuffer, &lcdFormat, &lcdStride );
    pLockData->pData = lcdBuffer + ( (pWinSurfaceInfo->height-1) * pWinSurfaceInfo->width * (pWinSurfaceInfo->xStride >> 1) );
    pLockData->bSurfaceNotChanged = EGL_FALSE;
    PRINTN("Returning pointer to 0x%x", (DWORD)pLockData->pData );
    return EGL_TRUE;
#else
    WARN("nativeLockWinSurface not supported in user mode");
    return EGL_FALSE;
#endif

#endif // RIM_WINDOW_MANAGER
}

/*!
*******************************************************************************
* \brief
*       Function to release window surface linear address.
*
* \param[in]  nativeWin       - Native window handle.
* \param[in]  pWinSurfaceInfo - Pointer to native window surface information.
* \return
*     - None
******************************************************************************/
IPIFUNC void nativeUnlockWinSurface(
    MarvellNativeWindowType nativeWin,
    _NATIVE_WIN_SURFACE_INFO_T* pWinSurfaceInfo)
{
#ifdef RIM_WINDOW_MANAGER
    if ( EglUseNewSwapLogic ) {
        EGLSurfaceDescriptor * surfaceDesc = (EGLSurfaceDescriptor *)pWinSurfaceInfo->pPrivate;

        PRINTN( "NEW: Native unlock win surface (surf = 0x%08x)", surfaceDesc );
        if ( surfaceDesc ) {
            kdThreadMutexUnlock(EglProcessState.SurfaceMutex);
        } else {
            WARN( "nativeLockWinSurface - no surface?" );
            return;
        }
    } else {
        WMWindow_t      nativeWindowRim;

        PRINT( "OLD: Native unlock win surface" );

        // get window from surface info
        nativeWindowRim = (WMWindow_t)pWinSurfaceInfo->pPrivate;

        // unlock window buffer
        WMIntUnlockWindowBuffer(nativeWindowRim);
    }
#else
    PRINT("Native unlock win surface");
#endif // RIM_WINDOW_MANAGER
}

/*!
*******************************************************************************
* \brief
*           Function to get native window client rect, the returned rects
*           should be in window relative coordinate system.
*
* \param[in]  nativeWin       - Native window handle.
* \param[in]  pWinSurfaceInfo - Pointer to native window surface information.
*   \param[out] pRects          - Pointer to NATIVE_RECT array
*   \param[in]  maxRectCount    - The size of NATIVE_RECT array.
*   \param[out] pRectCount      - The returned rect count.
* \return
*     - EGL_TRUE means success, otherwise return EGL_FALSE.
*           -   If pRects is NULL, native implementation should return clip rects
*               count to *pRectCount. If pRects is not NULL, native implementation
*               copy clip rects to the array pointed by pRects, and should not
*               exceed the array size defined by maxRectCount.
******************************************************************************/
IPIFUNC EGLBoolean nativeGetClipRects(
    MarvellNativeWindowType nativeWin,
    _NATIVE_WIN_SURFACE_INFO_T* pWinSurfaceInfo,
    _NATIVE_RECT_T* pRects,
    EGLint maxRectCount,
    EGLint* pRectCount)
{
#ifdef RIM_WINDOW_MANAGER
    WMError_t result;
    SDWORD temp[4];
    Rect dirtyRect;
#endif // RIM_WINDOW_MANAGER

    if(pRects)
    {
#ifdef RIM_WINDOW_MANAGER
        WMWindow_t      nativeWindowRim = (WMWindow_t)nativeWin;
#else
        NativeWindow *nativeWindowRim = NativeTranslateHandleInt((NativeWindowType)nativeWin);
#endif // RIM_WINDOW_MANAGER
        if(!nativeWindowRim){
            WARNN("Invalid window handle 0x%08x",nativeWin);
            return EGL_FALSE;
        }

#ifdef RIM_WINDOW_MANAGER
        result = WMGetWindowPropertyiv( nativeWindowRim, WM_PROPERTY_SIZE, temp );
        if( result != WM_E_OK ) {
            WARN("Failed to get window size info");
            return EGL_FALSE;
        }

        // can only trust the dirty region from WMan when the width/height are identical
        if ( temp[0] == pWinSurfaceInfo->width && temp[1] == pWinSurfaceInfo->height ) {
            // get native window dirty region
            result = WMGetWindowPropertyiv(nativeWindowRim, WM_PROPERTY_DIRTY_REGION, temp);
            if (result != WM_E_OK) {
                WARNN("Invalid window handle 0x%08x",nativeWin);
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
            dirtyRect.width = pWinSurfaceInfo->width;
            dirtyRect.y = 0;
            dirtyRect.height = pWinSurfaceInfo->height;
        }

        // Orient the clipping region correctly;
        // Dirty rect origin is upper left and clip rect origin is lower left
        pRects->left    = dirtyRect.x;
        pRects->top     = pWinSurfaceInfo->height - (dirtyRect.y + dirtyRect.height);
        pRects->right   = dirtyRect.x + dirtyRect.width;
        pRects->bottom  = pWinSurfaceInfo->height - dirtyRect.y;
#else
        // Orient the clipping region correctly
        pRects->left    = nativeWindowRim->clipRect.x;
        pRects->top     = nativeWindowRim->clipRect.y;
        pRects->right   = nativeWindowRim->clipRect.x + nativeWindowRim->clipRect.width;
        pRects->bottom  = nativeWindowRim->clipRect.y + nativeWindowRim->clipRect.height;
#endif // RIM_WINDOW_MANAGER

        LV1(LOG_EGL, PRINT4N("ClipRect left: %d bottom: %d right: %d top: %d", pRects->left, pRects->top, pRects->right, pRects->bottom));
    }
    *pRectCount = 1;

    return EGL_TRUE;
}


/*!
*******************************************************************************
* \brief
*     Function to wait for native rendering complete
* \param
* \return
- EGL_TRUE  means all native rendering operations are completed.
- EGL_FALSE means native rendering has error.
******************************************************************************/
IPIFUNC EGLBoolean nativeFinish( void )
{
    PRINT("Native finish");
    return EGL_TRUE;
}

void NativeGetDisplayWindow( NativeWindowType *nativeWin )
{
    WARN("This function has been deprecated - use NativeCreateWindow instead");
    *nativeWin = NULL;
}

/*!
*******************************************************************************
* \brief
*       Function to get native pixmap information.
*
* \param[in]  nativePixmap  - Native pixmap.
* \param[out] pPixmapInfo   - Pointer to native pixmap information.
* \return
*       - EGL_TRUE means success, else return EGL_FALSE.
******************************************************************************/
IPIFUNC EGLBoolean nativeGetPixmapInfo(EGLNativePixmapType  nativePixmap,
                                          _NATIVE_PIXMAP_INFO_T* pPixmapInfo)
{
    EGLBoolean ret = EGL_FALSE;

    // Use raw pixmap structure as native pixmap.

    if(nativePixmap)
    {
        NativePixmap_t* pPixmap = (NativePixmap_t*)nativePixmap;

        if(pPixmap->data)
        {
            pPixmapInfo->format     = pPixmap->format;
            pPixmapInfo->width      = pPixmap->width;
            pPixmapInfo->height     = pPixmap->height;
            pPixmapInfo->stride     = pPixmap->stride;
            pPixmapInfo->pPrivate   = NULL;

            ret = EGL_TRUE;
        }
    }

    return ret;
}

/*!
*******************************************************************************
* \brief
*       Function to release native pixmap information
*
* \param[in]  nativePixmap - Native pixmap.
* \param[in]  pPixmapInfo  - Pointer to native pixmap information.
* \return
*       - None
******************************************************************************/
IPIFUNC void nativeReleasePixmapInfo(EGLNativePixmapType  nativePixmap,
                                        _NATIVE_PIXMAP_INFO_T* pPixmapInfo)
{
    // Use raw pixmap structure as native pixmap.
    // Need to do nothing here.
}

/*!
*******************************************************************************
* \brief
*       Function to get native pixmap linear address.
*
* \param[in]  nativePixmap    - Native pixmap.
* \param[in]  pPixmapInfo     - Pointer to native pixmap information.
* \param[in]  pLockData       - Pointer to lock data structure.
* \return
*       - EGL_TRUE means success, and the lock data has been filled to correct
*         values.
*       - EGL_FALSE means fail, the values in lock data are undefined.
* \note
*       Native implementation can choose to return real pixmap buffer address
*       or another buffer address on lock. But it should make sure that pixmap
*       content has been updated after unlock.
*       For example
*       1) Native implementation return pixmap buffer address on lock, then EGL
*          will directly update content to the pixmap buffer.
*       2) Native implementation return another buffer address on lock,
*          after EGL updating  content to the buffer, native should copy content
*          from the buffer to the real pixmap buffer on nativeUnlockPixmap().
******************************************************************************/
IPIFUNC EGLBoolean nativeLockPixmap(EGLNativePixmapType   nativePixmap,
                                       _NATIVE_PIXMAP_INFO_T* pPixmapInfo,
                                       _NATIVE_LOCK_DATA_T*   pLockData)
{
    EGLBoolean ret = EGL_FALSE;

    // Use raw pixmap structure as native pixmap.
    // Directly return the pixmap buffer address.

    if(nativePixmap)
    {
        NativePixmap_t* pPixmap = (NativePixmap_t*)nativePixmap;
        if(pPixmap->data)
        {
            pLockData->pData                = pPixmap->data;
            pLockData->bSurfaceNotChanged   = EGL_TRUE;
            ret = EGL_TRUE;
        }
    }

    return ret;
}

/*!
*******************************************************************************
* \brief
*       Function to release pixmap linear address.
*
* \param[in]  nativePixmap  - Native pixmap.
* \param[in]  pPixmapInfo   - Pointer to native pixmap information.
* \return
*       - None
******************************************************************************/
IPIFUNC void nativeUnlockPixmap(EGLNativePixmapType  nativePixmap,
                                   _NATIVE_PIXMAP_INFO_T* pPixmapInfo)
{

    // Use raw pixmap structure as native pixmap.
    // Need to do nothing here.

}

#ifdef RIM_WINDOW_MANAGER
/******************************************************************************
 * Native Window API
 ******************************************************************************/
static DWORD WMErrorToWinError( WMError_t wmError )
{
    switch( wmError ) {
        case WM_E_OK: return WIN_SUCCESS;

        case WM_E_HANDLE: return WIN_BAD_HANDLE;
        case WM_E_PERM: return WIN_BAD_HANDLE; // Doesn't have permission to do this -> Invalid handle.

        case WM_E_ARG: return WIN_BAD_PARAMETER;

        case WM_E_FULL: return WIN_NO_FREE_HANDLES;

        case WM_E_ISBUSY: return WIN_IN_USE;

        case WM_E_NOMEM: return WIN_NO_MEM;
        case WM_E_INTERNAL: return WIN_NO_MEM; // Internal error -> no memory
        default: return WIN_NO_MEM; // default -> no memory
    }
}
#endif

DWORD NativeCreateWindow(NativeWindowType *nativeWin, const NativeWindowAttrib *nativeAttrib)
{
#ifdef RIM_WINDOW_MANAGER
#if defined(RIM_NDK)
    (void) nativeWin;
    (void) nativeAttrib;
    WARN("NativeCreateWindow not supported in NDK");
    return WIN_NO_MEM;
#else
    DWORD result;
    WMWindow_t window;
    WMWindowAttributes_t windowAttributes;

    if (nativeAttrib->display >= RIM_LCD_NUM_DISPLAYS) {
        WARNN("Invalid display: %d",nativeAttrib->display);
        return WIN_BAD_DISPLAY;
    }

    // create window
    memset( &windowAttributes, 0, sizeof( windowAttributes ) );
    windowAttributes.displayId = nativeAttrib->display;
    result = WMErrorToWinError( WMCreateWindow(&windowAttributes, &window) );
    if (result != WIN_SUCCESS) {
        WARNN("NativeCreateWindow create error %d", result);
        return result;
    }

    // add the window to the display
    result = WMErrorToWinError( WMAddWindowToDisplay( window, WM_NO_TRANSACTION ) );
    if (result != WIN_SUCCESS) {
        WMDestroyWindow(window);
        WARNN("NativeCreateWindow add error %d", result);
        return result;
    }

    *nativeWin = (NativeWindowType)window;

    return WIN_SUCCESS;
#endif // defined(RIM_NDK)
#else
    NativeWindow   *newNativeWindow;
    BYTE            newNativeWindowId = NATIVE_WIN_INVALID_WINDOW_ID;
    LcdConfig      *lcdConfig;
    int             i;

    LV1(LOG_EGL, PRINT("NativeCreateWindow"));

    // Check the requested attributes
    if(nativeAttrib->display < RIM_LCD_NUM_DISPLAYS){
        // Find an unused window in the global array
        for(i=0; i < NATIVE_WIN_MAX_WINDOWS; i++){
            if(globalNativeWindows[nativeAttrib->display][i] == NULL){
                newNativeWindowId = i;
                break;
            }
        }
        // If newNativeWindowId is unchanged then we are out of window handles
        if(newNativeWindowId == NATIVE_WIN_INVALID_WINDOW_ID){
            WARNN("No window handles: display %d",nativeAttrib->display);
            return WIN_NO_FREE_HANDLES;
        }
    }else{
        WARNN("Invalid display: %d",nativeAttrib->display);
        return WIN_BAD_DISPLAY;
    }

    // Create a new window
    newNativeWindow = kdMalloc(sizeof(NativeWindow));

    if(!newNativeWindow){
        WARN("Not enough memory for window");
        return WIN_NO_MEM;
    }

    // Set native window attributes
    newNativeWindow->display = nativeAttrib->display;
    newNativeWindow->windowId = newNativeWindowId;
    LV1(LOG_EGL, PRINT2N("Window: display %d window %d", newNativeWindow->display, newNativeWindow->windowId));

    // Default the window size to the size of the LCD
    lcdConfig = LcdGetLcdConfigPtr( newNativeWindow->display );
    newNativeWindow->windowSize.x = 0;
    newNativeWindow->windowSize.y = 0;
    newNativeWindow->windowSize.width = lcdConfig->width;
    newNativeWindow->windowSize.height = lcdConfig->height;
    newNativeWindow->clipRect = newNativeWindow->windowSize;
    LV1(LOG_EGL, PRINT4N("Window: x: %d y: %d width: %d height: %d",newNativeWindow->windowSize.x,newNativeWindow->windowSize.y,
        newNativeWindow->windowSize.width,newNativeWindow->windowSize.height));

    // Initalize to the default values
    newNativeWindow->level = 0;
    newNativeWindow->boundToEGLSurface = FALSE;

    // Increment the globalNativeWindowCounter and make sure that it is not zero, so that the handle is never null
    globalNativeWindowCounter++;
    if(globalNativeWindowCounter == 0){
        globalNativeWindowCounter++;
    }

    // Create the window handle
    newNativeWindow->handle =
        (NATIVE_WIN_COUNTER_MASK & (DWORD)(globalNativeWindowCounter << 24)) |
        (NATIVE_WIN_DISPLAY_MASK & (DWORD)(newNativeWindow->display << 16)) |
        (NATIVE_WIN_WINDOW_MASK  & (DWORD)newNativeWindow->windowId);

    // Return the handle
    (*nativeWin) = (NativeWindowType)newNativeWindow->handle;
    // Register the native window with our global array
    globalNativeWindows[newNativeWindow->display][newNativeWindow->windowId] = newNativeWindow;
    // Window is initialized
    newNativeWindow->initialized = TRUE;
    return WIN_SUCCESS;
#endif // RIM_WINDOW_MANAGER
}

DWORD NativeDestroyWindow(NativeWindowType nativeWin)
{
#ifdef RIM_WINDOW_MANAGER
    WMWindow_t window = (WMWindow_t)nativeWin;
    DWORD result;

    // destroy window
    result = WMErrorToWinError( WMDestroyWindow(window) );
    if (result != WIN_SUCCESS) {
        WARN2N("NativeDestroyWindow error=%d h=0x%08x",result,nativeWin);
        return result;
    }

    return WIN_SUCCESS;
#else
    NativeWindow *nativeWindowRim;

    LV1(LOG_EGL, PRINT("NativeDestroyWindow"));

    // Get the NativeWindow from the handle
    nativeWindowRim = NativeTranslateHandleInt(nativeWin);
    if(!nativeWindowRim){
        WARNN("Invalid window handle 0x%08x",nativeWin);
        return WIN_BAD_HANDLE;
    }

    // Check if the window is being used by EGL
    if(nativeWindowRim->boundToEGLSurface){
        WARNN("Window 0x%08x in use by EGL", nativeWindowRim);
        return WIN_IN_USE;
    }

    nativeWindowRim->initialized = FALSE;
    // Deregister the native window from our global array
    globalNativeWindows[nativeWindowRim->display][nativeWindowRim->windowId] = NULL;

    // Destroy the old window
    kdFree(nativeWindowRim);
    return WIN_SUCCESS;
#endif // RIM_WINDOW_MANAGER
}

DWORD NativeClipRectWindow( NativeWindowType nativeWin, const NativeWinRect_t *clipRect )
{
#ifdef RIM_WINDOW_MANAGER
    WMWindow_t window = (WMWindow_t)nativeWin;
    SDWORD size[2];
    SDWORD dirtyRect[4];
    DWORD result;

    // check for valid argument
    if(!clipRect){
        WARN("Invalid clipRect parameter");
        return WIN_BAD_PARAMETER;
    }

    // get window dimensions
    result = WMErrorToWinError( WMGetWindowPropertyiv(window, WM_PROPERTY_SIZE, size) );
    if (result != WIN_SUCCESS) {
        WARN2N("NativeClipRectWindow error=%d, handle=0x%08x",result,nativeWin);
        return result;
    }

    // convert rect struct to array and flip vertically;
    // clip rect origin is lower left and dirty rect origin is upper left
    dirtyRect[0] = clipRect->x;
    dirtyRect[1] = size[1] - (clipRect->y + clipRect->height);
    dirtyRect[2] = clipRect->width;
    dirtyRect[3] = clipRect->height;

    // set dirty region for window
    result = WMErrorToWinError( WMSetWindowPropertyiv(window, WM_PROPERTY_DIRTY_REGION, dirtyRect, WM_NO_TRANSACTION) );
    if (result != WIN_SUCCESS) {
        WARN2N("NativeClipRectWindow error=%d, handle=0x%08x",result,nativeWin);
        return result;
    }

    return WIN_SUCCESS;
#else
    NativeWindow *nativeWindowRim;

    LV1(LOG_EGL, PRINT("NativeSetClipRect"));

    // Get the NativeWindow from the handle
    nativeWindowRim = NativeTranslateHandleInt(nativeWin);
    if(!nativeWindowRim){
        WARNN("Invalid window handle 0x%08x",nativeWin);
        return WIN_BAD_HANDLE;
    }

    if(!clipRect){
        WARN("Invalid clipRect parameter");
        return WIN_BAD_PARAMETER;
    }

    if(!nativeWindowRim->initialized){
        WARNN("Invalid window: 0x%08x",nativeWindowRim);
        return WIN_NOT_INITIALIZED;
    }

    // Clamp the new clipRect to be within the acutal window size
    nativeWindowRim->clipRect.x = (clipRect->x <= nativeWindowRim->windowSize.width) ? clipRect->x : nativeWindowRim->windowSize.width;
    nativeWindowRim->clipRect.y = (clipRect->y <= nativeWindowRim->windowSize.height) ? clipRect->y : nativeWindowRim->windowSize.height;
    nativeWindowRim->clipRect.width = ((clipRect->width + nativeWindowRim->clipRect.x) <= nativeWindowRim->windowSize.width) ?
        clipRect->width : (nativeWindowRim->windowSize.width - nativeWindowRim->clipRect.x);
    nativeWindowRim->clipRect.height = ((clipRect->height + nativeWindowRim->clipRect.y) <= nativeWindowRim->windowSize.height) ?
        clipRect->height : (nativeWindowRim->windowSize.height - nativeWindowRim->clipRect.y);

    LV1(LOG_EGL, PRINT4N("ClipRect x: %d y: %d width: %d height: %d",nativeWindowRim->clipRect.x,nativeWindowRim->clipRect.y,
        nativeWindowRim->clipRect.width,nativeWindowRim->clipRect.height));

    return WIN_SUCCESS;
#endif // RIM_WINDOW_MANAGER
}

DWORD NativeQueryWindow( DWORD attribute, DWORD *value )
{
    if(value == NULL){
        WARNN("Invalid value parameter: 0x%08x",value);
        return WIN_BAD_PARAMETER;
    }

    switch(attribute){
        case WIN_MAX_LEVELS_PER_DISPLAY:
            *value = NATIVE_WIN_LEVELS_PER_DISPLAY;
            break;
        default:
            WARNN("Invalid attribute: 0x%04x",attribute);
            return WIN_BAD_ATTRIBUTE;
    }
    return WIN_SUCCESS;
}

#ifndef RIM_WINDOW_MANAGER
/******************************************************************************
 * Native Window API Internal Functions
 *      to be used within the Native Window API or EGL
 ******************************************************************************/
NativeWindow* NativeTranslateHandleInt(NativeWindowType nativeWin)
{
    NativeWindow *nativeWindowRim;
    // Decipher the handle
    BYTE display =  (BYTE)(((DWORD)nativeWin & NATIVE_WIN_DISPLAY_MASK) >> 16);
    BYTE windowId = (BYTE)((DWORD) nativeWin & NATIVE_WIN_WINDOW_MASK);

    LV2(LOG_EGL, PRINT("NativeTranslateHandleInt"));

    // Check the deciphered data
    if(display >= RIM_LCD_NUM_DISPLAYS){
        return NULL;
    }

    if(windowId >= NATIVE_WIN_MAX_WINDOWS){
        return NULL;
    }

    // Get the NativeWindow from the global table, if it exists
    if(globalNativeWindows[display][windowId]){
        nativeWindowRim = globalNativeWindows[display][windowId];
    }else{
        return NULL;
    }

    // Check if the handle is valid
    if(nativeWindowRim->handle != (DWORD)nativeWin){
        return NULL;
    }

    // Get the NativeWindow from the global table
    return nativeWindowRim;
}

DWORD NativeCheckSetLevelWindowInt(NativeWindowType nativeWin, NativeWindowAttrib *nativeAttrib, DWORD nativeLevel)
{
    NativeWindow *nativeWindowRim;

    LV1(LOG_EGL, PRINT("NativeCheckWindowInt"));

    // Get the NativeWindow from the handle
    nativeWindowRim = NativeTranslateHandleInt(nativeWin);
    if(!nativeWindowRim){
        WARNN("Invalid window handle: 0x%08x",nativeWin);
        return WIN_BAD_HANDLE;
    }

    if(!nativeWindowRim->initialized){
        WARNN("Invalid window: 0x%08x",nativeWindowRim);
        return WIN_NOT_INITIALIZED;
    }

    // Check the display
    if(nativeAttrib->display != nativeWindowRim->display){
        WARN2N("Display %d is invalid for window 0x%08x", nativeAttrib->display, nativeWindowRim);
        return WIN_BAD_PARAMETER;
    }

    // Check if it's bound to an EGL surface
    if(nativeWindowRim->boundToEGLSurface){
        WARNN("Window 0x%08x is already used by EGL", nativeWindowRim);
        return WIN_BAD_PARAMETER;
    }

    // If everything is fine, set the level
    if(nativeLevel >= NATIVE_WIN_LEVELS_PER_DISPLAY){
        WARN2N("Level %d is invalid for window 0x%08x", nativeLevel, nativeWindowRim);
        return WIN_BAD_PARAMETER;
    }else{
        nativeWindowRim->level = nativeLevel;
    }

    return WIN_SUCCESS;
}
#endif // RIM_WINDOW_MANAGER
