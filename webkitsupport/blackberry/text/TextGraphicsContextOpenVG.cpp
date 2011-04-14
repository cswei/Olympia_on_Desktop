/*
 * Copyright (C) 2010 Torch Mobile(Beijing) CO. Ltd. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "TextGraphicsContextOpenVG.h"

#include "EGLUtils.h"
#include "OlympiaPlatformAssert.h"
#include "VGUtils.h"

namespace TextAPI {

GraphicsContextOpenVG::GraphicsContextOpenVG()
    : m_eglDisplay(EGL_NO_DISPLAY)
    , m_eglContext(EGL_NO_CONTEXT)
    , m_eglSurface(EGL_NO_SURFACE)
{
}

GraphicsContextOpenVG::~GraphicsContextOpenVG()
{
}

ReturnCode GraphicsContextOpenVG::setContext(NativeGraphicsContext nativeContext)
{
    OLYMPIA_ASSERT(m_eglDisplay != EGL_NO_DISPLAY);
    OLYMPIA_ASSERT(m_eglSurface != EGL_NO_SURFACE);

    m_eglContext = static_cast<EGLContext>(nativeContext);
    OLYMPIA_ASSERT(m_eglContext != EGL_NO_CONTEXT);

    eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext);
    ASSERT_EGL_NO_ERROR();

    // FIXME: Do we need to sync fill and stroke paint?
    // It seems that description of syning fill paint and stroke
    // paint in text_api.h isn't correct.
#if 0
    vgSetPaint(m_fillPaint, VG_FILL_PATH);
    ASSERT_VG_NO_ERROR();

    vgSetPaint(m_strokePaint, VG_STROKE_PATH);
    ASSERT_VG_NO_ERROR();
#endif

    return 0;
}

void GraphicsContextOpenVG::setColor(VGPaintMode mode, Color color)
{
    OLYMPIA_ASSERT(m_eglDisplay != EGL_NO_DISPLAY);
    OLYMPIA_ASSERT(m_eglContext != EGL_NO_CONTEXT);
    OLYMPIA_ASSERT(m_eglSurface != EGL_NO_SURFACE);

    eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext);
    ASSERT_EGL_NO_ERROR();

    VGPaint paint = vgGetPaint(mode);
    ASSERT_VG_NO_ERROR();
    if (paint == VG_INVALID_HANDLE) {
        paint = vgCreatePaint();
        ASSERT_VG_NO_ERROR();
        vgSetPaint(paint, mode);
        ASSERT_VG_NO_ERROR();
    }
    vgSetParameteri(paint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
    ASSERT_VG_NO_ERROR();
    vgSetColor(paint, color);
    ASSERT_VG_NO_ERROR();
}

#if defined(USE_FREETYPE_RENDER) && USE_FREETYPE_RENDER
Color GraphicsContextOpenVG::color(VGPaintMode mode) const
{
    OLYMPIA_ASSERT(m_eglDisplay != EGL_NO_DISPLAY);
    OLYMPIA_ASSERT(m_eglContext != EGL_NO_CONTEXT);
    OLYMPIA_ASSERT(m_eglSurface != EGL_NO_SURFACE);

    eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext);
    ASSERT_EGL_NO_ERROR();

    VGPaint paint = vgGetPaint(mode);
    ASSERT_VG_NO_ERROR();
    Color c = 0;
    if (paint != VG_INVALID_HANDLE) {
        vgSetParameteri(paint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
        ASSERT_VG_NO_ERROR();
        c = vgGetColor(paint);
        ASSERT_VG_NO_ERROR();
    }
    return c;
}
#endif

}
