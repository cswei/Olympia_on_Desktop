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

#ifndef TextGraphicsContextOpenVG_H
#define TextGraphicsContextOpenVG_H

#include <EGL/egl.h>
#include "text_api.h"
#include <VG/openvg.h>

namespace TextAPI {

class GraphicsContextOpenVG: public GraphicsContext {
public:
    GraphicsContextOpenVG();
    ~GraphicsContextOpenVG();
    virtual ReturnCode setDisplay(NativeGraphicsDisplay nativeDisplay)
    {
        m_eglDisplay = static_cast<EGLDisplay>(nativeDisplay);
        return 0;
    }

    virtual ReturnCode setContext(NativeGraphicsContext nativeContext);

    virtual ReturnCode setSurface(NativeGraphicsSurface nativeGraphicsSurface)
    {
        m_eglSurface = static_cast<EGLSurface>(nativeGraphicsSurface);
        return 0;
    }

    virtual GraphicsContextType getType()
    {
        return OpenVGGraphicsContext;
    }

    virtual NativeGraphicsSurface getSurface()
    {
        return static_cast<NativeGraphicsSurface>(m_eglSurface);
    }

    virtual void setFillColor(Color color)
    {
        setColor(VG_FILL_PATH, color);
    }

    virtual void setStrokeColor(Color color)
    {
        setColor(VG_STROKE_PATH, color);
    }

#if defined(USE_FREETYPE_RENDER) && USE_FREETYPE_RENDER
    Color fillColor() const
    {
        return color(VG_FILL_PATH);
    }

    Color strokeColor() const
    {
        return color(VG_STROKE_PATH);
    }
#endif

    EGLDisplay eglDisplay() const
    {
        return m_eglDisplay;
    }

    EGLContext eglContext() const
    {
        return m_eglContext;
    }

    EGLSurface eglSurface() const
    {
        return m_eglSurface;
    }

private:
    void setColor(VGPaintMode mode, Color color);
#if defined(USE_FREETYPE_RENDER) && USE_FREETYPE_RENDER
    Color color(VGPaintMode mode) const;
#endif

private:
    EGLDisplay m_eglDisplay;
    EGLContext m_eglContext;
    EGLSurface m_eglSurface;
};

}
#endif
