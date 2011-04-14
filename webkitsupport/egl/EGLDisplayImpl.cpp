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
#include "EGLDisplayImpl.h"

#include "EGLContextImpl.h"
#include "EGLGlobal.h"
#include "EGLSurfaceImpl.h"

static EGLConfigImpl s_configTable[EGLDisplayImpl::MAX_NUM_CONFIGS] =
{
    // config ID begins with 1.
    //R  G  B  A  L BPP   S  M  config ID
    { 8, 8, 8, 8, 0, 32,  1, 8,  1 },
    { 8, 8, 8, 0, 0, 32,  1, 8,  2 },
    { 5, 5, 5, 1, 0, 16,  1, 4,  3 },
    { 5, 6, 5, 0, 0, 16,  1, 4,  4 },
    { 4, 4, 4, 4, 0, 16,  1, 4,  5 },
    { 0, 0, 0, 8, 0,  8,  1, 8,  6 },
    { 0, 0, 0, 4, 0,  4,  1, 4,  7 },
    { 0, 0, 0, 1, 0,  1,  1, 8,  8 },
    { 0, 0, 0, 0, 8,  8,  1, 8,  9 },
    { 0, 0, 0, 0, 1,  1,  1, 1, 10 },

    { 8, 8, 8, 8, 0, 32,  4, 1, 11 },
    { 8, 8, 8, 0, 0, 32,  4, 1, 12 },
    { 5, 5, 5, 1, 0, 16,  4, 1, 13 },
    { 5, 6, 5, 0, 0, 16,  4, 1, 14 },
    { 4, 4, 4, 4, 0, 16,  4, 1, 15 },
    { 0, 0, 0, 8, 0,  8,  4, 1, 16 },
    { 0, 0, 0, 4, 0,  4,  4, 1, 17 },
    { 0, 0, 0, 1, 0,  1,  4, 1, 18 },
    { 0, 0, 0, 0, 8,  8,  4, 1, 19 },
    { 0, 0, 0, 0, 1,  1,  4, 1, 20 },

    { 8, 8, 8, 8, 0, 32, 32, 1, 21 },
    { 8, 8, 8, 0, 0, 32, 32, 1, 22 },
    { 5, 5, 5, 1, 0, 16, 32, 1, 23 },
    { 5, 6, 5, 0, 0, 16, 32, 1, 24 },
    { 4, 4, 4, 4, 0, 16, 32, 1, 25 },
    { 0, 0, 0, 8, 0,  8, 32, 1, 26 },
    { 0, 0, 0, 4, 0,  4, 32, 1, 27 },
    { 0, 0, 0, 1, 0,  1, 32, 1, 28 },
    { 0, 0, 0, 0, 8,  8, 32, 1, 29 },
    { 0, 0, 0, 0, 1,  1, 32, 1, 30 },

    // Without mask
    { 8, 8, 8, 8, 0, 32,  1, 0, 31 },
    { 8, 8, 8, 0, 0, 32,  1, 0, 32 },
    { 5, 5, 5, 1, 0, 16,  1, 0, 33 },
    { 5, 6, 5, 0, 0, 16,  1, 0, 34 },
    { 4, 4, 4, 4, 0, 16,  1, 0, 35 },
    { 0, 0, 0, 8, 0,  8,  1, 0, 36 },
    { 0, 0, 0, 4, 0,  4,  1, 0, 37 },
    { 0, 0, 0, 1, 0,  1,  1, 0, 38 },
    { 0, 0, 0, 0, 8,  8,  1, 0, 39 },
    { 0, 0, 0, 0, 1,  1,  1, 0, 40 },

    { 8, 8, 8, 8, 0, 32,  4, 0, 41 },
    { 8, 8, 8, 0, 0, 32,  4, 0, 42 },
    { 5, 5, 5, 1, 0, 16,  4, 0, 43 },
    { 5, 6, 5, 0, 0, 16,  4, 0, 44 },
    { 4, 4, 4, 4, 0, 16,  4, 0, 45 },
    { 0, 0, 0, 8, 0,  8,  4, 0, 46 },
    { 0, 0, 0, 4, 0,  4,  4, 0, 47 },
    { 0, 0, 0, 1, 0,  1,  4, 0, 48 },
    { 0, 0, 0, 0, 8,  8,  4, 0, 49 },
    { 0, 0, 0, 0, 1,  1,  4, 0, 50 },

    { 8, 8, 8, 8, 0, 32, 32, 0, 51 },
    { 8, 8, 8, 0, 0, 32, 32, 0, 52 },
    { 5, 5, 5, 1, 0, 16, 32, 0, 53 },
    { 5, 6, 5, 0, 0, 16, 32, 0, 54 },
    { 4, 4, 4, 4, 0, 16, 32, 0, 55 },
    { 0, 0, 0, 8, 0,  8, 32, 0, 56 },
    { 0, 0, 0, 4, 0,  4, 32, 0, 57 },
    { 0, 0, 0, 1, 0,  1, 32, 0, 58 },
    { 0, 0, 0, 0, 8,  8, 32, 0, 59 },
    { 0, 0, 0, 0, 1,  1, 32, 0, 60 },
};

EGLDisplayImpl::EGLDisplayImpl(EGLDisplay id)
    : m_id(id)
{
    // We only support the configs in a static table instead
    // read system supported configs.
    m_numConfigs = EGLMIN(MAX_NUM_CONFIGS, sizeof(s_configTable));
    m_configs = s_configTable;
}

EGLConfigImpl* EGLDisplayImpl::config(int index) const
{
    EGL_ASSERT(index >= 0 && index < numConfigs());
    return m_configs + index;
}

void EGLDisplayImpl::addContext(EGLContextImpl* context)
{
    EGL_ASSERT(context);
    m_contexts.insert(context);
}

void EGLDisplayImpl::removeContext(EGLContextImpl* context)
{
    EGL_ASSERT(context);
    m_contexts.erase(context);
}

void EGLDisplayImpl::addSurface(EGLSurfaceImpl* surface)
{
    EGL_ASSERT(surface);
    m_surfaces.insert(surface);
}

void EGLDisplayImpl::removeSurface(EGLSurfaceImpl* surface)
{
    EGL_ASSERT(surface);
    m_surfaces.erase(surface);
}

bool EGLDisplayImpl::containsContext(EGLContextImpl* context) const
{
    return m_contexts.find(context) != m_contexts.end();
}

bool EGLDisplayImpl::containsSurface(EGLSurfaceImpl* surface) const
{
    return m_surfaces.find(surface) != m_surfaces.end();
}

bool EGLDisplayImpl::containsConfig(EGLConfigImpl* config) const
{
    return config && config->m_id > 0 && config->m_id <= numConfigs();
}
