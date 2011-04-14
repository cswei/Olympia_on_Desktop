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

#include "EGLConfigImpl.h"
#include <VG/vgext.h>

EGLSurfaceImpl::EGLSurfaceImpl(
    PlatformWindowContext windowContext,
    EGLConfigImpl* config,
    EGLint width,
    EGLint height,
    EGLint renderBufferType,
    bool useLinearColorSpace,
    bool supportAlphaMask,
    bool isLargestPbufferSurface,
    EGLClientBuffer buffer)
    : m_vgSurface(0)
    , m_windowContext(windowContext)
    , m_config(config)
    , m_clientBuffer(buffer)
    , m_renderBufferType(renderBufferType)
    , m_useLinearColorSpace(useLinearColorSpace)
    , m_isLargestPbufferSurface(isLargestPbufferSurface)
{
    EGL_ASSERT((windowContext && !isLargestPbufferSurface) || !windowContext);
    EGL_ASSERT(config);

    m_vgSurface = vgPrivSurfaceCreateAM(
        static_cast<VGint>(width),
        static_cast<VGint>(height),
        static_cast<VGboolean>(useLinearColorSpace),
        static_cast<VGboolean>(supportAlphaMask));
}

EGLSurfaceImpl::~EGLSurfaceImpl()
{
    if (m_vgSurface) {
        vgPrivSurfaceDestroyAM(m_vgSurface);
        m_vgSurface = 0;
    }
}

EGLint EGLSurfaceImpl::width() const
{
    if (m_vgSurface)
        return vgPrivGetSurfaceWidthAM(m_vgSurface);
    return 0;
}

EGLint EGLSurfaceImpl::height() const
{
    if (m_vgSurface)
        return vgPrivGetSurfaceHeightAM(m_vgSurface);
    return 0;
}
