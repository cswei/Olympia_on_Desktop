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

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>

#include "EGLGlobal.h"
#include "EGLConfigImpl.h"
#include "EGLContextImpl.h"
#include "EGLDisplayImpl.h"
#include "EGLImpl.h"
#include "EGLThread.h"

#include <algorithm>
#include <map>
#include <set>
#include <stdio.h>
#include <VG/openvg.h>
#include <VG/vgext.h>

inline EGL* egl()
{
    static EGL s_egl;
    return &s_egl;
}

inline static void eglSetError(EGLint error)
{
    EGLThreadData* threadData = egl()->ensureThreadData();
    if (threadData)
        threadData->setError(error);
}

inline static bool qualify(EGLint candidate, EGLint required)
{
    return (required == EGL_DONT_CARE) || (candidate >= required);
}

struct EGLConfigChooseResult {
    bool operator<(const EGLConfigChooseResult& other) const
    {
        return m_key < other.m_key;
    }
    int m_key;
    EGLConfig m_config;
};

EGLAPI EGLint EGLAPIENTRY eglGetError()
{
    EGLThreadLocker locker;
    EGLThreadData* threadData = egl()->currentThreadData();
    if (threadData)
        return threadData->error();
    return EGL_NOT_INITIALIZED;
}

EGLAPI EGLDisplay EGLAPIENTRY eglGetDisplay(EGLNativeDisplayType nativeDisplay)
{
    EGL_RETURN(EGL_SUCCESS, platformGetDisplay(nativeDisplay));
}

EGLAPI EGLBoolean EGLAPIENTRY eglInitialize(EGLDisplay dpy, EGLint* major, EGLint* minor)
{
    EGLThreadLocker locker;
    EGLDisplayImpl* display = egl()->findDisplayByID(dpy);
    if (display)
        EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
    display = new EGLDisplayImpl(dpy); // FIXME: handle OOM.
    egl()->addDisplay(display);
    if (major)
        *major = 1;
    if (minor)
        *minor = 2;
    EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

EGLAPI EGLBoolean EGLAPIENTRY eglTerminate(EGLDisplay dpy)
{
    EGLThreadLocker locker;
    EGLDisplayImpl* display = egl()->findDisplayByID(dpy);
    if (display) {
        egl()->removeDisplay(display);
        delete display;
    }
    EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

EGLAPI const char* EGLAPIENTRY eglQueryString(EGLDisplay dpy, EGLint name)
{
    EGLThreadLocker locker;
    EGLDisplayImpl* display = egl()->findDisplayByID(dpy);
    if (!display)
        EGL_RETURN(EGL_NOT_INITIALIZED, 0);

    static const char apis[] = "OpenVG";
    static const char extensions[] = "";
    static const char vendor[] = "Torch Mobile (Beijing) CO. Ltd.";
    static const char version[] = "1.2";

    const char* ret = 0;
    switch(name) {
    case EGL_CLIENT_APIS:
        ret = apis;
        break;
    case EGL_EXTENSIONS:
        ret = extensions;
        break;
    case EGL_VENDOR:
        ret = vendor;
        break;
    case EGL_VERSION:
        ret = version;
        break;
    default:
        EGL_RETURN(EGL_BAD_PARAMETER, 0);
    }
    EGL_RETURN(EGL_SUCCESS, ret);
}

EGLAPI EGLBoolean EGLAPIENTRY eglGetConfigs(EGLDisplay dpy, EGLConfig* configs, EGLint config_size, EGLint* num_config)
{
    EGLThreadLocker locker;
    EGLDisplayImpl* display = egl()->findDisplayByID(dpy);
    if (!display)
        EGL_RETURN(EGL_NOT_INITIALIZED, EGL_FALSE);
    if (!num_config)
        EGL_RETURN(EGL_BAD_PARAMETER, EGL_FALSE);
    int available = display->numConfigs();
    if (!configs) {
        *num_config = available;
        EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
    }
    *num_config = EGLMIN(available, config_size);
    for (int i = 0; i < *num_config; ++i)
        configs[i] = static_cast<EGLConfig>(display->config(i));
    EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

EGLAPI EGLBoolean EGLAPIENTRY eglChooseConfig(EGLDisplay dpy, const EGLint* attrib_list, EGLConfig* configs, EGLint config_size, EGLint* num_config)
{
    EGLThreadLocker locker;
    EGLDisplayImpl* display = egl()->findDisplayByID(dpy);
    if (!display)
        EGL_RETURN(EGL_NOT_INITIALIZED, EGL_FALSE);
    if (!num_config)
        EGL_RETURN(EGL_BAD_PARAMETER, EGL_FALSE);
    int available = display->numConfigs();
    if (!configs) {
        *num_config = available;
        EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
    }

    *num_config = 0;
    if (!config_size)
        EGL_RETURN(EGL_SUCCESS, EGL_TRUE);

    EGLint bufferSize = 0;
    EGLint redSize = 0;
    EGLint blueSize = 0;
    EGLint greenSize = 0;
    EGLint luminanceSize = 0;
    EGLint alphaSize = 0;
    EGLint colorBufferType = EGL_RGB_BUFFER;
    EGLint configID = EGL_DONT_CARE;
    EGLint sampleBuffers = 0;
    EGLint samples = 0;
    if (attrib_list) {
        for (int i = 0; attrib_list[i] != EGL_NONE; i += 2) {
            switch (attrib_list[i]) {
            case EGL_BUFFER_SIZE:
                bufferSize = attrib_list[i + 1];
                break;
            case EGL_RED_SIZE:
                redSize = attrib_list[i + 1];
                break;
            case EGL_BLUE_SIZE:
                blueSize = attrib_list[i + 1];
                break;
            case EGL_GREEN_SIZE:
                greenSize = attrib_list[i + 1];
                break;
            case EGL_LUMINANCE_SIZE:
                luminanceSize = attrib_list[i + 1];
                break;
            case EGL_ALPHA_SIZE:
                alphaSize = attrib_list[i + 1];
                break;
            case EGL_ALPHA_MASK_SIZE:
                if (attrib_list[i + 1] > 8) // Not supported
                    EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
                break;
            case EGL_COLOR_BUFFER_TYPE:
            {
                EGLint value = attrib_list[i + 1];
                if (value != EGL_DONT_CARE
                    && value != EGL_RGB_BUFFER
                    && value != EGL_LUMINANCE_BUFFER)
                    EGL_RETURN(EGL_BAD_ATTRIBUTE, EGL_FALSE);
                colorBufferType = value;
                break;
            }
            case EGL_CONFIG_ID:
                configID = attrib_list[i + 1];
                break;
            case EGL_SAMPLE_BUFFERS:
                sampleBuffers = attrib_list[i + 1];
                break;
            case EGL_SAMPLES:
                samples = attrib_list[i + 1];
                break;

            case EGL_BIND_TO_TEXTURE_RGB:  // Always EGL_FALSE
            case EGL_BIND_TO_TEXTURE_RGBA: // Always EGL_FALSE
            case EGL_DEPTH_SIZE:           // Always 0
            case EGL_LEVEL:                // Always 0
            case EGL_NATIVE_RENDERABLE:    // Always EGL_FALSE
            case EGL_STENCIL_SIZE:         // Always 0
                if (attrib_list[i + 1])    // Not supported
                    EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
                break;

            case EGL_CONFIG_CAVEAT:        // Always EGL_NONE
            case EGL_NATIVE_VISUAL_TYPE:   // Always EGL_NONE
                if (attrib_list[i + 1] != EGL_NONE) // Not supported
                    EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
                break;

            case EGL_MAX_SWAP_INTERVAL:    // Always 1
            case EGL_MIN_SWAP_INTERVAL:    // Always 1
                if (attrib_list[i + 1] != 1)
                    EGL_RETURN(EGL_SUCCESS, EGL_TRUE); // Not supported
                break;

            case EGL_RENDERABLE_TYPE:      // Always EGL_OPENVG_BIT
                if (attrib_list[i + 1] != EGL_OPENVG_BIT) // Not supported
                    EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
                break;

            case EGL_SURFACE_TYPE:
                break; // All types are supported.
                       // Currently we don't
                       // support Window surface and Pixmap surface.
                       // We may support in the short future.

            case EGL_TRANSPARENT_TYPE:     // Always EGL_NONE
            case EGL_NATIVE_VISUAL_ID:     // Always 0
            case EGL_MAX_PBUFFER_WIDTH:    // Always INT_MAX
            case EGL_MAX_PBUFFER_HEIGHT:   // Always INT_MAX
            case EGL_MAX_PBUFFER_PIXELS:   // Always INT_MAX
            case EGL_TRANSPARENT_RED_VALUE:   // Undefined
            case EGL_TRANSPARENT_BLUE_VALUE:  // Undefined
            case EGL_TRANSPARENT_GREEN_VALUE: // Undefined
                break; // Ignored

            default:
                EGL_RETURN(EGL_BAD_ATTRIBUTE, EGL_FALSE);
            }
        }
    }

    if (configID && configID != EGL_DONT_CARE) {
        int index = configID - 1;
        *configs = static_cast<EGLConfig>(display->config(index));
        *num_config = 1;
        EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
    }

    EGLConfigChooseResult result[EGLDisplayImpl::MAX_NUM_CONFIGS];
    int resultCount = 0;
    for (int i = 0; i < available; ++i) {
        EGLConfigImpl* candidate = display->config(i);
        int candidateColorBits = candidate->m_redBits + candidate->m_blueBits + candidate->m_greenBits;
        int candidateLuminanceBits = candidate->m_luminanceBits;
        int candidateBufferSize;
        if (candidateColorBits) {
            EGL_ASSERT(!candidateLuminanceBits);
            candidateColorBits += candidate->m_alphaBits;
            candidateBufferSize = candidateColorBits;
        } else if (candidateLuminanceBits) {
            candidateLuminanceBits += candidate->m_alphaBits;
            candidateBufferSize = candidateLuminanceBits;
        } else {
            candidateColorBits = candidate->m_alphaBits;
            candidateLuminanceBits = candidateColorBits;
            candidateBufferSize = candidateColorBits;
        }
        if (!qualify(candidateBufferSize, bufferSize))
            continue;

        // FIXME: is it correct?
        int candidateSampleBuffers = candidate->m_samples == 1 ? 0 : 1;
        if (!qualify(candidateSampleBuffers, sampleBuffers))
            continue;

        if (!qualify(candidate->m_samples, samples))
            continue;

        if (!qualify(candidate->m_redBits, redSize)
            || !qualify(candidate->m_blueBits, blueSize)
            || !qualify(candidate->m_greenBits, greenSize)
            || !qualify(candidate->m_alphaBits, alphaSize)
            || !qualify(candidate->m_luminanceBits, luminanceSize))
            continue;

        if ((colorBufferType == EGL_RGB_BUFFER && !candidateColorBits)
            || (colorBufferType == EGL_LUMINANCE_BUFFER && !candidateLuminanceBits))
            continue;

        // Sort critera:
        //    * config ID: increasing
        //    * color depth: decreasing
        int key = candidate->m_id;
        int depth = 0;
        if (redSize && redSize != EGL_DONT_CARE)
            depth += candidate->m_redBits;
        if (blueSize && blueSize != EGL_DONT_CARE)
            depth += candidate->m_blueBits;
        if (greenSize && greenSize != EGL_DONT_CARE)
            depth += candidate->m_greenBits;
        if (alphaSize && alphaSize != EGL_DONT_CARE)
            depth += candidate->m_alphaBits;
        if (luminanceSize && luminanceSize != EGL_DONT_CARE)
            depth += candidate->m_luminanceBits;
        EGL_ASSERT(depth <= 32);
        key |= (32 - depth) << 6; // Note: make sure config ID < 2^6
        result[resultCount].m_key = key;
        result[resultCount].m_config = static_cast<EGLConfig>(candidate);
        resultCount++;
    }
    if (!resultCount)
        EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
    std::sort(result, result + resultCount);
    resultCount = EGLMIN(resultCount, config_size);
    for (int i = 0; i < resultCount; ++i)
        configs[i] = result[i].m_config;
    *num_config = resultCount;
    EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

EGLAPI EGLBoolean EGLAPIENTRY eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint* value)
{
    EGLThreadLocker locker;
    EGLDisplayImpl* display = egl()->findDisplayByID(dpy);
    if (!display)
        EGL_RETURN(EGL_NOT_INITIALIZED, EGL_FALSE);
    if (!value)
        EGL_RETURN(EGL_BAD_PARAMETER, EGL_FALSE);
    EGLConfigImpl* implConfig = static_cast<EGLConfigImpl*>(config);
    if (!display->containsConfig(implConfig))
        EGL_RETURN(EGL_BAD_CONFIG, EGL_FALSE);
    switch (attribute) {
    case EGL_BUFFER_SIZE:
        *value = EGLMAX(implConfig->m_redBits
                     + implConfig->m_blueBits
                     + implConfig->m_greenBits
                     + implConfig->m_alphaBits,
                     implConfig->m_alphaBits
                     + implConfig->m_luminanceBits);
        break;
    case EGL_RED_SIZE:
        *value = implConfig->m_redBits;
        break;
    case EGL_BLUE_SIZE:
        *value = implConfig->m_blueBits;
        break;
    case EGL_GREEN_SIZE:
        *value = implConfig->m_greenBits;
        break;
    case EGL_ALPHA_SIZE:
        *value = implConfig->m_alphaBits;
        break;
    case EGL_LUMINANCE_SIZE:
        *value = implConfig->m_luminanceBits;
        break;
    case EGL_ALPHA_MASK_SIZE:
        *value = implConfig->m_maskBits;
        break;
    case EGL_BIND_TO_TEXTURE_RGB:
    case EGL_BIND_TO_TEXTURE_RGBA:
        *value = EGL_FALSE;
        break;
    case EGL_COLOR_BUFFER_TYPE:
        if (implConfig->m_redBits)
            *value = EGL_RGB_BUFFER;
        else
            *value = EGL_LUMINANCE_BUFFER;
        break;
    case EGL_CONFIG_CAVEAT:
        *value = EGL_NONE;
        break;
    case EGL_CONFIG_ID:
        *value = implConfig->m_id;
        break;
    case EGL_DEPTH_SIZE:
        *value = 0;
        break;
    case EGL_LEVEL:
        *value = 0;
        break;
    case EGL_MAX_PBUFFER_WIDTH:
        *value = 32767;
        break;
    case EGL_MAX_PBUFFER_HEIGHT:
        *value = 32767;
        break;
    case EGL_MAX_PBUFFER_PIXELS:
        *value = 32767 * 32767;
        break;
    case EGL_MAX_SWAP_INTERVAL:
    case EGL_MIN_SWAP_INTERVAL:
        *value = 1;
        break;
    case EGL_NATIVE_RENDERABLE:
        *value = EGL_FALSE;
        break;
    case EGL_NATIVE_VISUAL_ID:
        *value = 0;
        break;
    case EGL_NATIVE_VISUAL_TYPE:
        *value = EGL_NONE;
        break;
    case EGL_RENDERABLE_TYPE:
        *value = EGL_OPENVG_BIT;
        break;
    case EGL_SAMPLE_BUFFERS:
        *value = implConfig->m_samples > 1 ? 1 : 0;
        break;
    case EGL_SAMPLES:
        *value = implConfig->m_samples > 1 ? implConfig->m_samples : 0;
        break;
    case EGL_STENCIL_SIZE:
        *value = 0;
        break;
    case EGL_SURFACE_TYPE:
        *value = EGL_WINDOW_BIT
               | EGL_PIXMAP_BIT
               | EGL_PBUFFER_BIT
               | EGL_VG_COLORSPACE_LINEAR_BIT
               | EGL_VG_ALPHA_FORMAT_PRE_BIT;
        break;
    case EGL_TRANSPARENT_TYPE:
        *value = EGL_NONE;
        break;
    case EGL_TRANSPARENT_RED_VALUE:
    case EGL_TRANSPARENT_BLUE_VALUE:
    case EGL_TRANSPARENT_GREEN_VALUE:
        *value = 0;
        break;
    case EGL_CONFORMANT:
        *value = EGL_OPENVG_BIT; // FIXME: use proper value
        break;
    default:
        EGL_RETURN(EGL_BAD_ATTRIBUTE, EGL_FALSE);
    }
    EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

EGLAPI EGLSurface EGLAPIENTRY eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint* attrib_list)
{
    EGLThreadLocker locker;
    EGLDisplayImpl* display = egl()->findDisplayByID(dpy);
    if (!display)
        EGL_RETURN(EGL_NOT_INITIALIZED, EGL_NO_SURFACE);
    EGLConfigImpl* implConfig = static_cast<EGLConfigImpl*>(config);
    if (!display->containsConfig(implConfig))
        EGL_RETURN(EGL_BAD_CONFIG, EGL_NO_SURFACE);
    EGLint renderBuffer = EGL_BACK_BUFFER;
    EGLint colorSpace = EGL_VG_COLORSPACE_sRGB;
    EGLint alphaFormat = EGL_VG_ALPHA_FORMAT_NONPRE;
    if (attrib_list) {
        for (int i = 0; attrib_list[i] != EGL_NONE; i += 2) {
            switch (attrib_list[i]) {
            case EGL_RENDER_BUFFER:
                renderBuffer = attrib_list[i + 1];
                break;
            case EGL_VG_COLORSPACE:
                colorSpace = attrib_list[i + 1];
                break;
            case EGL_VG_ALPHA_FORMAT:
                alphaFormat = attrib_list[i + 1];
                break;
            default:
                EGL_RETURN(EGL_BAD_ATTRIBUTE, EGL_NO_SURFACE);
            }
        }
    }

    PlatformWindowContext windowContext = platformCreateWindowContext(win);
    if (!windowContext)
        EGL_RETURN(EGL_BAD_ALLOC, EGL_NO_SURFACE);
    int width = 0;
    int height = 0;
    platformGetWindowSize(windowContext, width, height);
    if (width <=0 || height <= 0 || !platformContextIsWindow(windowContext)) {
        platformDestroyWindowContext(windowContext);
        EGL_RETURN(EGL_BAD_NATIVE_WINDOW, EGL_NO_SURFACE);
    }
    EGLSurfaceImpl* implSurface = EGLSurfaceImpl::create(
        windowContext,
        implConfig,
        width,
        height,
        EGL_BACK_BUFFER,
        colorSpace == EGL_VG_COLORSPACE_LINEAR,
        !!implConfig->m_maskBits,
        false);
    if (!implSurface || !implSurface->isValid()) {
        if (implSurface)
            implSurface->deref();
        platformDestroyWindowContext(windowContext);
        EGL_RETURN(EGL_BAD_ALLOC, EGL_NO_SURFACE);
    }
    display->addSurface(implSurface);
    EGL_RETURN(EGL_SUCCESS, static_cast<EGLSurface>(implSurface));
}

EGLAPI EGLSurface EGLAPIENTRY eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint* attrib_list)
{
    EGLThreadLocker locker;
    EGLDisplayImpl* display = egl()->findDisplayByID(dpy);
    if (!display)
        EGL_RETURN(EGL_NOT_INITIALIZED, EGL_NO_SURFACE);
    EGLConfigImpl* implConfig = static_cast<EGLConfigImpl*>(config);
    if (!display->containsConfig(implConfig))
        EGL_RETURN(EGL_BAD_CONFIG, EGL_NO_SURFACE);

    EGLint colorSpace = EGL_VG_COLORSPACE_sRGB;
    EGLint alphaFormat = EGL_VG_ALPHA_FORMAT_NONPRE;
    EGLint width = 0;
    EGLint height = 0;
    EGLint largestPbuffer = 0;
    if (attrib_list) {
        for (int i = 0; attrib_list[i] != EGL_NONE; i += 2) {
            switch (attrib_list[i]) {
            case EGL_WIDTH:
                width = attrib_list[i + 1];
                break;
            case EGL_HEIGHT:
                height = attrib_list[i + 1];
                break;
            case EGL_LARGEST_PBUFFER:
                largestPbuffer = attrib_list[i + 1];
                break;
            case EGL_VG_COLORSPACE:
                colorSpace = attrib_list[i + 1];
                break;
            case EGL_VG_ALPHA_FORMAT:
                alphaFormat = attrib_list[i + 1];
                break;

            case EGL_TEXTURE_FORMAT:
            case EGL_TEXTURE_TARGET:
            case EGL_MIPMAP_TEXTURE:
            default:
                EGL_RETURN(EGL_BAD_ATTRIBUTE, EGL_NO_SURFACE);
            }
        }
    }

    if (width <= 0 || height <= 0)
        EGL_RETURN(EGL_BAD_ATTRIBUTE, EGL_NO_SURFACE);

    EGLSurfaceImpl* implSurface = EGLSurfaceImpl::create(
        0,
        implConfig,
        width,
        height,
        EGL_BACK_BUFFER,
        colorSpace == EGL_VG_COLORSPACE_LINEAR,
        !!implConfig->m_maskBits,
        !!largestPbuffer);
    if (!implSurface || !implSurface->isValid()) {
        if (implSurface)
            implSurface->deref();
        EGL_RETURN(EGL_BAD_ALLOC, EGL_NO_SURFACE);
    }
    display->addSurface(implSurface);

    EGL_RETURN(EGL_SUCCESS, static_cast<EGLSurface>(implSurface));
}

EGLAPI EGLSurface EGLAPIENTRY eglCreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint* attrib_list)
{
    EGLThreadLocker locker;

    EGLDisplayImpl* display = egl()->findDisplayByID(dpy);
    if (!display)
        EGL_RETURN(EGL_NOT_INITIALIZED, EGL_NO_SURFACE);

    if (buftype != EGL_OPENVG_IMAGE)
        EGL_RETURN(EGL_BAD_PARAMETER, EGL_NO_SURFACE);

    if (!buffer)
        EGL_RETURN(EGL_BAD_PARAMETER, EGL_NO_SURFACE);

    EGLThreadData* threadData = egl()->ensureThreadData();
    if (!threadData)
        EGL_RETURN(EGL_BAD_ALLOC, EGL_NO_SURFACE);

    EGLContextImpl* c = threadData->context();
    if (!c || !c->vgContext())
        EGL_RETURN(EGL_BAD_ACCESS, EGL_NO_SURFACE);

    if(vgPrivImageInUseByOpenVG(c->vgContext(), static_cast<VGHandle>(reinterpret_cast<intptr_t>(buffer))))
        EGL_RETURN(EGL_BAD_ACCESS, EGL_NO_SURFACE);

    EGLConfigImpl* implConfig = static_cast<EGLConfigImpl*>(config);
    if (!display->containsConfig(implConfig))
        EGL_RETURN(EGL_BAD_CONFIG, EGL_NO_SURFACE);

    if (attrib_list && attrib_list[0] != EGL_NONE)
        EGL_RETURN(EGL_BAD_ATTRIBUTE, EGL_NO_SURFACE);

    unsigned int isOpenVGImage = 0;
    unsigned int isInUse = 0;
    signed int width = 0;
    signed int height = 0;
    unsigned int colorSpaceLinear = 0;
    unsigned int alphaFormatPre = 0;
    signed int redSize = 0;
    signed int greenSize = 0;
    signed int blueSize = 0;
    signed int alphaSize = 0;
    signed int luminanceSize = 0;

    vgPrivImageInfoGet(
        c->vgContext(),
        static_cast<VGImage>(reinterpret_cast<intptr_t>(buffer)),
        &isOpenVGImage,
        &isInUse,
        &width,
        &height,
        &colorSpaceLinear,
        &alphaFormatPre ,
        &redSize,
        &greenSize,
        &blueSize,
        &alphaSize,
        &luminanceSize);

    if (redSize != implConfig->m_redBits ||
        greenSize != implConfig->m_greenBits ||
        blueSize != implConfig->m_blueBits ||
        alphaSize != implConfig->m_alphaBits ||
        luminanceSize != implConfig->m_luminanceBits)
        EGL_RETURN(EGL_BAD_MATCH, EGL_NO_SURFACE);

    EGLSurfaceImpl* implSurface = EGLSurfaceImpl::create(
        0,
        implConfig,
        width,
        height,
        EGL_BACK_BUFFER,
        colorSpaceLinear == EGL_VG_COLORSPACE_LINEAR,
        !!implConfig->m_maskBits,
        VG_FALSE,
        buffer);

    if (!implSurface || !implSurface->isValid()) {
        if (implSurface)
            implSurface->deref();
        EGL_RETURN(EGL_BAD_ALLOC, EGL_NO_SURFACE);
    }
    display->addSurface(implSurface);

    EGL_RETURN(EGL_SUCCESS, static_cast<EGLSurface>(implSurface));
}

EGLAPI EGLSurface EGLAPIENTRY eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list)
{
    EGLThreadLocker locker;
    EGL_UNUSED_PARAM(dpy);
    EGL_UNUSED_PARAM(config);
    EGL_UNUSED_PARAM(pixmap);
    EGL_UNUSED_PARAM(attrib_list);
    notImplemented();
    EGL_RETURN(EGL_NOT_INITIALIZED, EGL_NO_SURFACE);
}

EGLAPI EGLBoolean EGLAPIENTRY eglDestroySurface(EGLDisplay dpy, EGLSurface surface)
{
    EGLThreadLocker locker;
    EGLDisplayImpl* display = egl()->findDisplayByID(dpy);
    if (!display)
        EGL_RETURN(EGL_NOT_INITIALIZED, EGL_FALSE);
    EGLSurfaceImpl* implSurface = static_cast<EGLSurfaceImpl*>(surface);
    if (!display->containsSurface(implSurface))
        EGL_RETURN(EGL_BAD_SURFACE, EGL_FALSE);
    if (implSurface->hasOneRef())
        display->removeSurface(implSurface);
    implSurface->deref();
    EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

EGLAPI EGLBoolean EGLAPIENTRY eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value)
{
    EGLThreadLocker locker;
    EGLDisplayImpl* display = egl()->findDisplayByID(dpy);
    if (!display)
        EGL_RETURN(EGL_NOT_INITIALIZED, EGL_FALSE);
    if (!display->containsSurface(static_cast<EGLSurfaceImpl*>(surface)))
        EGL_RETURN(EGL_BAD_SURFACE, EGL_FALSE);
    EGL_UNUSED_PARAM(attribute);
    EGL_UNUSED_PARAM(value);
    EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

EGLAPI EGLBoolean EGLAPIENTRY eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint* value)
{
    EGLThreadLocker locker;
    EGLDisplayImpl* display = egl()->findDisplayByID(dpy);
    if (!display)
        EGL_RETURN(EGL_NOT_INITIALIZED, EGL_FALSE);
    EGLSurfaceImpl* implSurface = static_cast<EGLSurfaceImpl*>(surface);
    if (!display->containsSurface(implSurface))
        EGL_RETURN(EGL_BAD_SURFACE, EGL_FALSE);
    switch (attribute) {
    case EGL_VG_ALPHA_FORMAT:
        *value = EGL_VG_ALPHA_FORMAT_NONPRE;
        break;
    case EGL_VG_COLORSPACE:
        *value = implSurface->useLinearColorSpace() ? EGL_VG_COLORSPACE_LINEAR : EGL_VG_COLORSPACE_sRGB;
        break;
    case EGL_CONFIG_ID:
        *value = implSurface->config()->m_id;
        break;
    case EGL_HEIGHT:
        *value = implSurface->height();
        break;
    case EGL_WIDTH:
        *value = implSurface->width();
        break;
    case EGL_HORIZONTAL_RESOLUTION:
        *value = EGL_UNKNOWN; // FIXME
        break;
    case EGL_VERTICAL_RESOLUTION:
        *value = EGL_UNKNOWN; // FIXME
        break;
    case EGL_LARGEST_PBUFFER:
        if (implSurface->isWindowSurface())
            *value = EGL_FALSE;
        else
            *value = implSurface->isLargestPbufferSurface();
        break;
    case EGL_MIPMAP_TEXTURE:
        *value = EGL_FALSE;
        break;
    case EGL_MIPMAP_LEVEL:
        *value = 0;
        break;
    case EGL_PIXEL_ASPECT_RATIO:
        *value = EGL_UNKNOWN; // FIXME
        break;
    case EGL_RENDER_BUFFER:
        *value = implSurface->renderBufferType();
        break;
    case EGL_SWAP_BEHAVIOR:
        *value = EGL_BUFFER_PRESERVED;
        break;
    case EGL_TEXTURE_FORMAT:
        *value = EGL_NO_TEXTURE;
        break;
    case EGL_TEXTURE_TARGET:
        *value = EGL_NO_TEXTURE;
        break;
    default:
        EGL_RETURN(EGL_BAD_ATTRIBUTE, EGL_FALSE);
    }
    EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

EGLAPI EGLContext EGLAPIENTRY eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint* attrib_list)
{
    EGLThreadLocker locker;
    EGLDisplayImpl* display = egl()->findDisplayByID(dpy);
    if (!display)
        EGL_RETURN(EGL_NOT_INITIALIZED, EGL_NO_CONTEXT);
    EGLConfigImpl* implConfig = static_cast<EGLConfigImpl*>(config);
    if (!display->containsConfig(implConfig))
        EGL_RETURN(EGL_BAD_CONFIG, EGL_NO_CONTEXT);
    EGLContextImpl* shareContext = static_cast<EGLContextImpl*>(share_context);
    // FIXME: Do we need to check share_context?
    if (shareContext && display->containsContext(shareContext))
        EGL_RETURN(EGL_BAD_CONTEXT, EGL_NO_CONTEXT);

    EGL_UNUSED_PARAM(attrib_list);
    EGLThreadData* threadData = egl()->ensureThreadData();
    if (!threadData)
        EGL_RETURN(EGL_BAD_ALLOC, EGL_NO_CONTEXT);
    if (threadData->boundAPIType() != EGL_OPENVG_API)
        EGL_RETURN(EGL_BAD_MATCH, EGL_NO_CONTEXT);
    EGLContextImpl* implContext = EGLContextImpl::create(shareContext, implConfig);
    if (!implContext || !implContext->isValid()) {
        if (implContext)
            implContext->deref();
        EGL_RETURN(EGL_BAD_ALLOC, EGL_NO_CONTEXT);
    }
    display->addContext(implContext);
    EGL_RETURN(EGL_SUCCESS, static_cast<EGLContext>(implContext));
}

EGLAPI EGLBoolean EGLAPIENTRY eglDestroyContext(EGLDisplay dpy, EGLContext context)
{
    EGLThreadLocker locker;
    EGLDisplayImpl* display = egl()->findDisplayByID(dpy);
    if (!display)
        EGL_RETURN(EGL_NOT_INITIALIZED, EGL_FALSE);
    EGLContextImpl* implContext = static_cast<EGLContextImpl*>(context);
    if (!display->containsContext(implContext))
        EGL_RETURN(EGL_BAD_CONTEXT, EGL_FALSE);
    if (implContext->hasOneRef())
        display->removeContext(implContext);
    implContext->deref();
    EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

EGLAPI EGLBoolean EGLAPIENTRY eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext context)
{
    EGLThreadLocker locker;
    EGLDisplayImpl* display = egl()->findDisplayByID(dpy);
    if (!display)
        EGL_RETURN(EGL_NOT_INITIALIZED, EGL_FALSE);
    EGLContextImpl* implContext = static_cast<EGLContextImpl*>(context);
    if (context != EGL_NO_CONTEXT && !display->containsContext(implContext))
        EGL_RETURN(EGL_BAD_CONTEXT, EGL_FALSE);
    EGLSurfaceImpl* implSurface = static_cast<EGLSurfaceImpl*>(draw);
    if (read != draw) // We only support read == draw
        EGL_RETURN(EGL_BAD_MATCH, EGL_FALSE);
    if (draw != EGL_NO_SURFACE && !display->containsSurface(implSurface))
        EGL_RETURN(EGL_BAD_SURFACE, EGL_FALSE);

    if ((implContext && !implSurface)
        || (!implContext && implSurface)
        || (implContext && implContext->config() != implSurface->config()))
        EGL_RETURN(EGL_BAD_MATCH, EGL_FALSE);

    EGLThreadData* threadData = egl()->ensureThreadData();
    if (!threadData)
        EGL_RETURN(EGL_BAD_ALLOC, EGL_FALSE);

    EGLDisplayImpl* previousDisplay = threadData->display();
    if (previousDisplay) {
        EGLContextImpl* c = threadData->context();
        EGLSurfaceImpl* s = threadData->surface();

        if (c)
            vgFlush();

        if(c && s && s->clientBuffer())
            vgPrivImageUnlock(c->vgContext(), static_cast<VGImage>(reinterpret_cast<intptr_t>(s->clientBuffer())), s->vgSurface());

        if (c) {
            if (c->hasOneRef())
                previousDisplay->removeContext(c);
            c->deref();
        }

        if (s) {
            if (s->hasOneRef())
                previousDisplay->removeSurface(s);
            s->deref();
        }
    }

    VGboolean result = vgPrivMakeCurrentAM(
        implContext ? implContext->vgContext() : 0,
        implSurface ? implSurface->vgSurface() : 0);
    if (!result)
        EGL_RETURN(EGL_BAD_MATCH, EGL_FALSE);
    if (implContext)
        implContext->ref();
    if (implSurface)
        implSurface->ref();

    if(implContext && implSurface && implSurface->clientBuffer())
        vgPrivImageLock(implContext->vgContext(), static_cast<VGImage>(reinterpret_cast<intptr_t>(implSurface->clientBuffer())), implSurface->vgSurface());

    threadData->makeCurrent(display, implContext, implSurface);

    EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

EGLAPI EGLContext EGLAPIENTRY eglGetCurrentContext()
{
    EGLThreadLocker locker;
    EGLContext context = EGL_NO_CONTEXT;
    EGLThreadData* threadData = egl()->currentThreadData();
    if (threadData && threadData->boundAPIType() == EGL_OPENVG_API)
        context = static_cast<EGLContext>(threadData->context());
    EGL_RETURN(EGL_SUCCESS, context);
}

EGLAPI EGLSurface EGLAPIENTRY eglGetCurrentSurface(EGLint readdraw)
{
    EGLThreadLocker locker;
    if (readdraw != EGL_READ && readdraw != EGL_DRAW)
        EGL_RETURN(EGL_BAD_PARAMETER, EGL_NO_SURFACE);
    EGLSurface surface = EGL_NO_SURFACE;
    EGLThreadData* threadData = egl()->currentThreadData();
    if (threadData && threadData->boundAPIType() == EGL_OPENVG_API)
        surface = static_cast<EGLSurface>(threadData->surface());
    EGL_RETURN(EGL_SUCCESS, surface);
}

EGLAPI EGLDisplay EGLAPIENTRY eglGetCurrentDisplay()
{
    EGLThreadLocker locker;
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLThreadData* threadData = egl()->currentThreadData();
    if (threadData
       && threadData->boundAPIType() == EGL_OPENVG_API
       && threadData->display())
        display = threadData->display()->id();
    EGL_RETURN(EGL_SUCCESS, display);
}

EGLAPI EGLBoolean EGLAPIENTRY eglQueryContext(EGLDisplay dpy, EGLContext context, EGLint attribute, EGLint* value)
{
    EGLThreadLocker locker;
    EGLDisplayImpl* display = egl()->findDisplayByID(dpy);
    if (!display)
        EGL_RETURN(EGL_NOT_INITIALIZED, EGL_FALSE);
    EGLContextImpl* implContext = static_cast<EGLContextImpl*>(context);
    if (!display->containsContext(implContext))
        EGL_RETURN(EGL_BAD_CONTEXT, EGL_FALSE);
    if (attribute != EGL_CONFIG_ID && attribute != EGL_CONTEXT_CLIENT_TYPE)
        EGL_RETURN(EGL_BAD_ATTRIBUTE, EGL_FALSE);
    if (attribute == EGL_CONFIG_ID)
        *value = implContext->config()->m_id;
    if (attribute == EGL_CONTEXT_CLIENT_TYPE)
        *value = EGL_OPENVG_API;
    EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

EGLAPI EGLBoolean EGLAPIENTRY eglBindAPI(EGLenum api)
{
    EGLThreadLocker locker;
    if (api != EGL_OPENVG_API && api != EGL_OPENGL_ES_API)
        EGL_RETURN(EGL_BAD_PARAMETER, EGL_FALSE);
    EGLThreadData* threadData = egl()->ensureThreadData();
    if (!threadData)
        EGL_RETURN(EGL_BAD_ALLOC, EGL_FALSE);
    threadData->bindAPI(api);
    EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

EGLAPI EGLenum EGLAPIENTRY eglQueryAPI()
{
    EGLThreadLocker locker;
    EGLenum type = EGL_NONE;
    EGLThreadData* threadData = egl()->ensureThreadData();
    if (threadData)
        type = static_cast<EGLenum>(threadData->boundAPIType());
    EGL_RETURN(EGL_SUCCESS, type);
}

EGLAPI EGLBoolean EGLAPIENTRY eglWaitClient()
{
    EGLThreadLocker locker;
    EGLThreadData* threadData = egl()->ensureThreadData();
    if (threadData && threadData->boundAPIType() == EGL_OPENVG_API)
        vgFinish();
    EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

EGLAPI EGLBoolean EGLAPIENTRY eglWaitGL()
{
    // Don't support GL ES.
    return EGL_TRUE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglWaitNative(EGLint engine)
{
    // Don't support native rendering.
    EGL_UNUSED_PARAM(engine);
    return EGL_TRUE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglSwapBuffers(EGLDisplay dpy, EGLSurface surface)
{
    EGLThreadLocker locker;
    EGLDisplayImpl* display = egl()->findDisplayByID(dpy);
    if (!display)
        EGL_RETURN(EGL_NOT_INITIALIZED, EGL_FALSE);
    EGLSurfaceImpl* implSurface = static_cast<EGLSurfaceImpl*>(surface);
    if (!display->containsSurface(implSurface))
        EGL_RETURN(EGL_BAD_SURFACE, EGL_FALSE);
    vgFlush();
    if (!implSurface->isWindowSurface())
        EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
    if (!platformContextIsWindow(implSurface->windowContext()))
        EGL_RETURN(EGL_BAD_NATIVE_WINDOW, EGL_FALSE);
    int width = 0;
    int height = 0;
    platformGetWindowSize(implSurface->windowContext(), width, height);

    // Resize handling.
    // vgPrivSurfaceResizeAM just realloc the underlying buffer.
    // This results the content become massy when the width changs.
    // So if only height changes we can use vgPrivSurfaceResizeAM
    // directly; if width changes we need to keep the content correctly.
    int surfaceWidth = implSurface->width();
    int surfaceHeight = implSurface->height();
    if (width == surfaceWidth && height != surfaceHeight) {
        VGboolean result = vgPrivSurfaceResizeAM(implSurface->vgSurface(), width, height);
        if (!result)
            EGL_RETURN(EGL_BAD_ALLOC, EGL_FALSE);
    } else if (width != implSurface->width()) {
        int contentWidth = EGLMIN(width, surfaceWidth);
        int contentHeight = EGLMIN(height, surfaceHeight);

        // Read content to a VGImage.
        VGuint* buffer = static_cast<VGuint*>(malloc(contentWidth * contentHeight * sizeof(VGuint)));
        if (!buffer)
            EGL_RETURN(EGL_BAD_ALLOC, EGL_FALSE);
        VGImageFormat format = VG_sBGRA_8888;
        vgReadPixels(buffer,
            contentWidth * sizeof(VGuint),
            format,
            0,
            0,
            contentWidth,
            contentHeight);
        VGImage image = vgCreateImage(format,
            contentWidth,
            contentHeight,
            VG_IMAGE_QUALITY_FASTER);
        if (image == VG_INVALID_HANDLE) {
            free(buffer);
            EGL_RETURN(EGL_BAD_ALLOC, EGL_FALSE);
        }
        vgImageSubData(image,
            buffer,
            contentWidth * sizeof(VGuint),
            format,
            0,
            0,
            contentWidth,
            contentHeight);
        free(buffer);

        // Resize vg surface.
        VGboolean result = vgPrivSurfaceResizeAM(implSurface->vgSurface(), width, height);
        if (!result) {
            vgDestroyImage(image);
            EGL_RETURN(EGL_BAD_ALLOC, EGL_FALSE);
        }

        VGfloat oldClearColor[4];
        VGfloat oldImageMatrix[9];
        VGint oldMatrixMode;
        vgGetfv(VG_CLEAR_COLOR, 4, oldClearColor);
        oldMatrixMode = vgGeti(VG_MATRIX_MODE);

        vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
        vgGetMatrix(oldImageMatrix);
        vgLoadIdentity();

        VGfloat clearColor[4] = { 0 };
        vgSetfv(VG_CLEAR_COLOR, 4, clearColor);
        vgClear(0, 0, width, height);

        vgDrawImage(image);
        vgDestroyImage(image);

        vgLoadMatrix(oldImageMatrix);
        vgSeti(VG_MATRIX_MODE, oldMatrixMode);
        vgSetfv(VG_CLEAR_COLOR, 4, oldClearColor);
        vgFlush();
    }
    platformBlitToWindow(implSurface);
    EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

EGLAPI EGLBoolean EGLAPIENTRY eglCopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target)
{
    EGLThreadLocker locker;
    notImplemented();
    EGL_UNUSED_PARAM(dpy);
    EGL_UNUSED_PARAM(surface);
    EGL_UNUSED_PARAM(target);
    EGL_RETURN(EGL_NOT_INITIALIZED, EGL_FALSE);
}

EGLAPI EGLBoolean EGLAPIENTRY eglSwapInterval(EGLDisplay dpy, EGLint interval)
{
    EGLThreadLocker locker;
    EGLDisplayImpl* display = egl()->findDisplayByID(dpy);
    if (!display)
        EGL_RETURN(EGL_NOT_INITIALIZED, EGL_FALSE);
    EGL_UNUSED_PARAM(interval);
    EGL_RETURN(EGL_SUCCESS, EGL_TRUE);
}

typedef void (*EGLProc)();

EGLAPI EGLProc EGLAPIENTRY eglGetProcAddress(const char *procname)
{
    EGL_UNUSED_PARAM(procname);

    // Per EGL spec, no error condition is raised.
    return 0;
}

EGLAPI EGLBoolean EGLAPIENTRY eglReleaseThread()
{
    EGLThreadLocker locker;
    EGLThreadData* threadData = egl()->currentThreadData();
    if (threadData) {
        EGLContextImpl* context = threadData->context();
        if (context) {
            vgFinish();
            context->deref();
        }
        EGLSurfaceImpl* surface = threadData->surface();
        if (surface)
            surface->deref();
        egl()->destroyCurrentThreadData();
    }
    platformReleaseThread();
    return EGL_TRUE;
}

