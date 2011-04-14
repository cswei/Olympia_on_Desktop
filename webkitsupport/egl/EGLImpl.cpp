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
#include "EGLImpl.h"

#include "EGLDisplayImpl.h"
#include "EGLThread.h"

EGL::~EGL()
{
    DisplayMap::iterator displayIterator = m_displays.begin();
    DisplayMap::iterator displayEnd = m_displays.end();
    for (; displayIterator != displayEnd; ++displayIterator)
        delete displayIterator->second;
    m_displays.clear();

    ThreadDataMap::iterator threadDataIterator = m_threads.begin();
    ThreadDataMap::iterator threadDataEnd = m_threads.end();
    for (; threadDataIterator != threadDataEnd; ++threadDataIterator)
        delete threadDataIterator->second;
    m_threads.clear();

}

void EGL::addDisplay(EGLDisplayImpl* display)
{
    EGL_ASSERT(display);
    m_displays.insert(std::make_pair(display->id(), display));
}

void EGL::removeDisplay(EGLDisplayImpl* display)
{
    EGL_ASSERT(display);
    m_displays.erase(display->id());
}

EGLDisplayImpl* EGL::findDisplayByID(EGLDisplay dpy) const
{
    DisplayMap::const_iterator it = m_displays.find(dpy);
    if (it != m_displays.end())
        return it->second;
    return 0;
}

EGLThreadData* EGL::currentThreadData() const
{
    EGLThreadID id = platformCurrentThreadID();
    ThreadDataMap::const_iterator it = m_threads.find(id);
    if (it != m_threads.end())
        return it->second;
    return 0;
}

EGLThreadData* EGL::ensureThreadData()
{
    EGLThreadData* threadData = currentThreadData();
    if (!threadData) {
        threadData = new EGLThreadData(platformCurrentThreadID());
        m_threads.insert(std::make_pair(threadData->id(), threadData));
    }
    return threadData;
}

void EGL::destroyCurrentThreadData()
{
    EGLThreadData* threadData = currentThreadData();
    if (threadData) {
        m_threads.erase(threadData->id());
        delete threadData;
    }
}
