/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef WKFrame_h
#define WKFrame_h

#include <CoreFoundation/CoreFoundation.h>
#include <WebKit2/WKBase.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum WKFrameLoadState {
    kWKFrameLoadStateProvisional = 0,
    kWKFrameLoadStateCommitted = 1,
    kWKFrameLoadStateFinished = 2
};
typedef enum WKFrameLoadState WKFrameLoadState;

WK_EXPORT bool WKFrameIsMainFrame(WKFrameRef frame);
WK_EXPORT WKFrameLoadState WKFrameGetFrameLoadState(WKFrameRef frame);
WK_EXPORT WKURLRef WKFrameGetProvisionalURL(WKFrameRef frame);
WK_EXPORT WKURLRef WKFrameGetURL(WKFrameRef frame);

WK_EXPORT WKFrameRef WKFrameRetain(WKFrameRef frame);
WK_EXPORT void WKFrameRelease(WKFrameRef frame);

#ifdef __cplusplus
}
#endif

#endif /* WKFrame_h */
