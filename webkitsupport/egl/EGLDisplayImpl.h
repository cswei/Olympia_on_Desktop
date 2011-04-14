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

#ifndef EGLDisplayImpl_H
#define EGLDisplayImpl_H

#include "EGLConfigImpl.h"
#include <EGL/egl.h>
#include "EGLNoncopyable.h"
#include <set>

class EGLContextImpl;
class EGLSurfaceImpl;

class EGLDisplayImpl: public EGLNoncopyable {
public:
    EGLDisplayImpl(EGLDisplay id);
    int numConfigs() const { return m_numConfigs; }
    EGLConfigImpl* config(int index) const;
    EGLDisplay id() const { return m_id; }
    void addContext(EGLContextImpl* context);
    void removeContext(EGLContextImpl* context);
    void addSurface(EGLSurfaceImpl* surface);
    void removeSurface(EGLSurfaceImpl* surface);
    bool containsContext(EGLContextImpl* context) const;
    bool containsSurface(EGLSurfaceImpl* surface) const;
    bool containsConfig(EGLConfigImpl* config) const;
    static const int MAX_NUM_CONFIGS = 60;
private:
    typedef std::set<EGLContextImpl*> ContextSet;
    typedef std::set<EGLSurfaceImpl*> SurfaceSet;
    ContextSet m_contexts;
    SurfaceSet m_surfaces;
    EGLDisplay m_id;
    EGLConfigImpl* m_configs;
    int m_numConfigs;
};

#endif
