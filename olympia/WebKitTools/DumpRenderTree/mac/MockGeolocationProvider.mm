/*
 * Copyright (C) 2010 Apple Inc. All Rights Reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#import "MockGeolocationProvider.h"


@implementation MockGeolocationProvider

+ (MockGeolocationProvider *)shared
{
    static MockGeolocationProvider *provider = [[MockGeolocationProvider alloc] init];
    return provider;
}

- (void)dealloc
{
    ASSERT(_registeredViews.isEmpty());

    [_lastPosition release];
    [_error release];
    [super dealloc];
}

- (void)setPosition:(WebGeolocationPosition *)position
{
    if (_lastPosition != position) {
        [_lastPosition release];
        _lastPosition = [position retain];
    }
    
    [_error release];
    _error = 0;

    if (!_timer)
        _timer = [NSTimer scheduledTimerWithTimeInterval:0 target:self selector:@selector(timerFired) userInfo:0 repeats:NO];
}

- (void)setError:(NSError *)error
{
    if (_error != error) {
        [_error release];
        _error = [error retain];
    }
    
    [_lastPosition release];
    _lastPosition = 0;

    if (!_timer)
        _timer = [NSTimer scheduledTimerWithTimeInterval:0 target:self selector:@selector(timerFired) userInfo:0 repeats:NO];
}

- (void)registerWebView:(WebView *)webView
{
    _registeredViews.add(webView);
}

- (void)unregisterWebView:(WebView *)webView
{
    _registeredViews.remove(webView);
}

- (WebGeolocationPosition *)lastPosition
{
    return _lastPosition;
}

- (void)stopTimer
{
    [_timer invalidate];
    _timer = 0;
}

- (void)timerFired
{
    _timer = 0;

    // Expect that views won't be (un)registered while iterating.
    HashSet<WebView*> views = _registeredViews;
    for (HashSet<WebView*>::iterator iter = views.begin(); iter != views.end(); ++iter) {
        if (_error)
            [*iter _geolocationDidFailWithError:_error];
        else
            [*iter _geolocationDidChangePosition:_lastPosition];
    }
}

@end
