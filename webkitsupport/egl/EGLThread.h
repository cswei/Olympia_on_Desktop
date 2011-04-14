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

#ifndef EGLThread_H
#define EGLThread_H

#include <EGL/egl.h>
#include "EGLGlobal.h"
#include "EGLContextImpl.h"
#include "EGLDisplayImpl.h"
#include "EGLSurfaceImpl.h"
#include "EGLNoncopyable.h"

class EGL;
class EGLThreadData: public EGLNoncopyable {
public:
    EGLint error() const { return m_error; }
    void setError(EGLint error)
    {
        m_error = error;
    }

    void makeCurrent(EGLDisplayImpl* display, EGLContextImpl* context, EGLSurfaceImpl* surface)
    {
        m_display = display;
        m_context = context;
        m_surface = surface;
    }

    EGLDisplayImpl* display() const { return m_display; }

    EGLContextImpl* context() const { return m_context; }

    EGLSurfaceImpl* surface() const { return m_surface; }

    EGLThreadID id() const { return m_threadID; }

    void bindAPI(EGLint api) { m_boundAPIType = api; }

    EGLint boundAPIType() const { return m_boundAPIType; }

private:
    EGLThreadData(EGLThreadID id)
        : m_display(0)
        , m_context(0)
        , m_surface(0)
        , m_error(EGL_SUCCESS)
        , m_threadID(id)
        , m_boundAPIType(EGL_NONE)
    { }

    ~EGLThreadData()
    { }

private:
    EGLDisplayImpl* m_display;
    EGLContextImpl* m_context;
    EGLSurfaceImpl* m_surface;
    EGLint m_error;
    EGLThreadID m_threadID;
    EGLint m_boundAPIType;
    friend class EGL;
};

class EGLThreadLocker: public EGLNoncopyable {
public:
    EGLThreadLocker()
    {
        platformAcquireMutex();
    }

    ~ EGLThreadLocker()
    {
        platformReleaseMutex();
    }
};

#endif
