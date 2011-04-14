/*
 * Copyright (C) 2009 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef RuntimeEnabledFeatures_h
#define RuntimeEnabledFeatures_h

namespace WebCore {

// A class that stores static enablers for all experimental features. Note that
// the method names must line up with the JavaScript method they enable for code
// generation to work properly.

class RuntimeEnabledFeatures {
public:
    static void setLocalStorageEnabled(bool isEnabled) { isLocalStorageEnabled = isEnabled; }
    static bool localStorageEnabled() { return isLocalStorageEnabled; }

    static void setSessionStorageEnabled(bool isEnabled) { isSessionStorageEnabled = isEnabled; }
    static bool sessionStorageEnabled() { return isSessionStorageEnabled; }

    static void setWebkitNotificationsEnabled(bool isEnabled) { isWebkitNotificationsEnabled = isEnabled; }
    static bool webkitNotificationsEnabled() { return isWebkitNotificationsEnabled; }

    static void setApplicationCacheEnabled(bool isEnabled) { isApplicationCacheEnabled = isEnabled; }
    static bool applicationCacheEnabled() { return isApplicationCacheEnabled; }

    static void setGeolocationEnabled(bool isEnabled) { isGeolocationEnabled = isEnabled; }
    static bool geolocationEnabled() { return isGeolocationEnabled; }

    static void setIndexedDBEnabled(bool isEnabled) { isIndexedDBEnabled = isEnabled; }
    static bool indexedDBEnabled() { return isIndexedDBEnabled; }

#if ENABLE(VIDEO)
    static bool audioEnabled();
    static bool htmlMediaElementEnabled();
    static bool htmlAudioElementEnabled();
    static bool htmlVideoElementEnabled();
    static bool mediaErrorEnabled();
#endif

#if ENABLE(SHARED_WORKERS)
    static bool sharedWorkerEnabled();
#endif

#if ENABLE(WEB_SOCKETS)
    static bool webSocketEnabled();
#endif

#if ENABLE(DATABASE)
    static bool openDatabaseEnabled();
    static bool openDatabaseSyncEnabled();
#endif

#if ENABLE(3D_CANVAS)
    static void setWebGLEnabled(bool isEnabled) { isWebGLEnabled = isEnabled; }
    static bool arrayBufferEnabled() { return isWebGLEnabled; }
    static bool int8ArrayEnabled() { return isWebGLEnabled; }
    static bool uint8ArrayEnabled() { return isWebGLEnabled; }
    static bool int16ArrayEnabled() { return isWebGLEnabled; }
    static bool uint16ArrayEnabled() { return isWebGLEnabled; }
    static bool int32ArrayEnabled() { return isWebGLEnabled; }
    static bool uint32ArrayEnabled() { return isWebGLEnabled; }
    static bool floatArrayEnabled() { return isWebGLEnabled; }
    static bool webGLRenderingContextEnabled() { return isWebGLEnabled; }
    static bool webGLArrayBufferEnabled() { return isWebGLEnabled; }
    static bool webGLByteArrayEnabled() { return isWebGLEnabled; }
    static bool webGLUnsignedByteArrayEnabled() { return isWebGLEnabled; }
    static bool webGLShortArrayEnabled() { return isWebGLEnabled; }
    static bool webGLUnsignedShortArrayEnabled() { return isWebGLEnabled; }
    static bool webGLIntArrayEnabled() { return isWebGLEnabled; }
    static bool webGLUnsignedIntArrayEnabled() { return isWebGLEnabled; }
    static bool webGLFloatArrayEnabled() { return isWebGLEnabled; }
#endif

    static void setPushStateEnabled(bool isEnabled) { isPushStateEnabled = isEnabled; }
    static bool pushStateEnabled() { return isPushStateEnabled; }
    static bool replaceStateEnabled() { return isPushStateEnabled; }

#if ENABLE(TOUCH_EVENTS)
    static bool touchEnabled() { return isTouchEnabled; }
    static void setTouchEnabled(bool isEnabled) { isTouchEnabled = isEnabled; }
    static bool ontouchstartEnabled() { return isTouchEnabled; }
    static bool ontouchmoveEnabled() { return isTouchEnabled; }
    static bool ontouchendEnabled() { return isTouchEnabled; }
    static bool ontouchcancelEnabled() { return isTouchEnabled; }
#endif

private:
    // Never instantiate.
    RuntimeEnabledFeatures() { }

    static bool isLocalStorageEnabled;
    static bool isSessionStorageEnabled;
    static bool isWebkitNotificationsEnabled;
    static bool isApplicationCacheEnabled;
    static bool isGeolocationEnabled;
    static bool isIndexedDBEnabled;
    static bool isWebGLEnabled;
    static bool isPushStateEnabled;
    static bool isTouchEnabled;
};

} // namespace WebCore

#endif // RuntimeEnabledFeatures_h
