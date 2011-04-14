/*
 * Copyright (c) 2011, Torch Mobile (Beijing) Co. Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms,
 * with or without modification, are permitted provided that
 * the following conditions are met:
 *
 *  -- Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *  -- Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *  -- Neither the name of the Torch Mobile (Beijing) Co. Ltd. nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "EGLGlobal.h"

#include "EGLSurfaceImpl.h"
#include <VG/openvg.h>
#include <windows.h>

EGLDisplay platformGetDisplay(EGLNativeDisplayType dpy)
{
    if (dpy == EGL_DEFAULT_DISPLAY)
        return reinterpret_cast<EGLDisplay>(1);
    return static_cast<EGLDisplay>(dpy);
}

struct WindowContext {
    HWND m_window;
    HDC m_bitmapDC;
    HBITMAP m_bitmap;
    unsigned int* m_buffer;
    int m_width;
    int m_height;
};

PlatformWindowContext platformCreateWindowContext(EGLNativeWindowType window)
{
    WindowContext* context = static_cast<WindowContext*>(malloc(sizeof(WindowContext)));
    if (!context)
        return 0;
    memset(context, 0, sizeof(WindowContext));
    context->m_window = static_cast<HWND>(window);
    HDC winDC = ::GetDC(context->m_window);
    if (!winDC) {
        free(context);
        return 0;
    }
    context->m_bitmapDC = ::CreateCompatibleDC(winDC);
    ::ReleaseDC(context->m_window, winDC);
    if (!context->m_bitmapDC) {
        free(context);
        return 0;
    }
    return context;
}

void platformDestroyWindowContext(PlatformWindowContext context)
{
    WindowContext* implContext = static_cast<WindowContext*>(context);
    if (implContext) {
        if (implContext->m_bitmapDC) {
            ::SelectObject(implContext->m_bitmapDC, NULL);
            ::DeleteDC(implContext->m_bitmapDC);
        }
        if (implContext->m_bitmap)
            ::DeleteObject(implContext->m_bitmap);
        free(implContext);
    }
}

bool platformContextIsWindow(const PlatformWindowContext context)
{
    WindowContext* implContext = static_cast<WindowContext*>(context);
    return implContext && ::IsWindow(implContext->m_window);
}

void platformGetWindowSize(const PlatformWindowContext context, int& width, int& height)
{
    WindowContext* implContext = static_cast<WindowContext*>(context);
    if (implContext) {
        RECT rect;
        ::GetClientRect(implContext->m_window, &rect);
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;
    } else {
        width = 0;
        height = 0;
    }
}

void platformBlitToWindow(EGLSurfaceImpl* surface)
{
    WindowContext* context = static_cast<WindowContext*>(surface->windowContext());
    if (!context || !surface)
        return;

    int width = surface->width();
    int height = surface->height();
    if (!context->m_buffer
        || !context->m_bitmap
        || context->m_width != width
        || context->m_height != height) {
        if (context->m_bitmap) {
            ::DeleteObject(context->m_bitmap);
            context->m_bitmap = 0;
        }
        context->m_buffer = 0;
        context->m_width = width;
        context->m_height = height;

        struct {
            BITMAPINFOHEADER header;
            DWORD rMask;
            DWORD gMask;
            DWORD bMask;
        } bmi;

        memset(&bmi, 0, sizeof(bmi));
        bmi.header.biSize = sizeof(BITMAPINFOHEADER);
        bmi.header.biWidth = width;
        bmi.header.biHeight = height;
        bmi.header.biPlanes = 1;
        bmi.header.biBitCount = static_cast<WORD>(32);
        bmi.header.biCompression = BI_BITFIELDS;
        bmi.rMask = 0x000000ff;
        bmi.gMask = 0x0000ff00;
        bmi.bMask = 0x00ff0000;
        context->m_bitmap = ::CreateDIBSection(
            context->m_bitmapDC,
            reinterpret_cast<BITMAPINFO*>(&bmi),
            DIB_RGB_COLORS,
            reinterpret_cast<void**>(&context->m_buffer),
            NULL,
            0);
        if (!context->m_bitmap) {
            context->m_buffer = 0;
            return;
        }
    }

    if (context->m_buffer) {
        ::GdiFlush();
        VGImageFormat format = VG_sXBGR_8888; // Little endian.
        vgReadPixels(context->m_buffer,
            width * sizeof(unsigned int),
            format,
            0,
            0,
            width,
            height);
        ::SelectObject(context->m_bitmapDC, context->m_bitmap);
        HDC winDC = ::GetDC(context->m_window);
        ::BitBlt(winDC,
            0,
            0,
            width,
            height,
            context->m_bitmapDC,
            0,
            0,
            SRCCOPY);
        ::ReleaseDC(context->m_window, winDC);
        ::SelectObject(context->m_bitmapDC, NULL);
    }
}

