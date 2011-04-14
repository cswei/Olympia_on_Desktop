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

#include "WebErrors.h"

#include <WebCore/ResourceRequest.h>
#include <WebCore/ResourceResponse.h>
#include <pthread.h>

using namespace WebCore;

// FIXME: We probably don't need to use NSErrors here.

enum {
    WebKitErrorCannotShowMIMEType =                             100,
    WebKitErrorCannotShowURL =                                  101,
    WebKitErrorFrameLoadInterruptedByPolicyChange =             102,
};
enum {
    WebKitErrorCannotFindPlugIn =                               200,
    WebKitErrorCannotLoadPlugIn =                               201,
    WebKitErrorJavaUnavailable =                                202,
};
enum {
    WebKitErrorCannotUseRestrictedPort =                        103,
};

#define WebKitErrorPlugInCancelledConnection 203
#define WebKitErrorPlugInWillHandleLoad 204

static NSString *WebKitErrorDomain = @"WebKitErrorDomain";

static NSString * const WebKitErrorMIMETypeKey =               @"WebKitErrorMIMETypeKey";
static NSString * const WebKitErrorPlugInNameKey =             @"WebKitErrorPlugInNameKey";
static NSString * const WebKitErrorPlugInPageURLStringKey =    @"WebKitErrorPlugInPageURLStringKey";

#define UI_STRING(__str, __desc) [NSString stringWithUTF8String:__str]

// Policy errors
#define WebKitErrorDescriptionCannotShowMIMEType UI_STRING("Content with specified MIME type can’t be shown", "WebKitErrorCannotShowMIMEType description")
#define WebKitErrorDescriptionCannotShowURL UI_STRING("The URL can’t be shown", "WebKitErrorCannotShowURL description")
#define WebKitErrorDescriptionFrameLoadInterruptedByPolicyChange UI_STRING("Frame load interrupted", "WebKitErrorFrameLoadInterruptedByPolicyChange description")
#define WebKitErrorDescriptionCannotUseRestrictedPort UI_STRING("Not allowed to use restricted network port", "WebKitErrorCannotUseRestrictedPort description")

// Plug-in and java errors
#define WebKitErrorDescriptionCannotFindPlugin UI_STRING("The plug-in can’t be found", "WebKitErrorCannotFindPlugin description")
#define WebKitErrorDescriptionCannotLoadPlugin UI_STRING("The plug-in can’t be loaded", "WebKitErrorCannotLoadPlugin description")
#define WebKitErrorDescriptionJavaUnavailable UI_STRING("Java is unavailable", "WebKitErrorJavaUnavailable description")
#define WebKitErrorDescriptionPlugInCancelledConnection UI_STRING("Plug-in cancelled", "WebKitErrorPlugInCancelledConnection description")
#define WebKitErrorDescriptionPlugInWillHandleLoad UI_STRING("Plug-in handled load", "WebKitErrorPlugInWillHandleLoad description")

static pthread_once_t registerErrorsControl = PTHREAD_ONCE_INIT;
static void registerErrors(void);

@interface NSError (WebKitExtras)
+ (NSError *)_webKitErrorWithDomain:(NSString *)domain code:(int)code URL:(NSURL *)URL;
@end

@implementation NSError (WebKitExtras)

static NSMutableDictionary *descriptions = nil;

+ (void)_registerWebKitErrors
{
    pthread_once(&registerErrorsControl, registerErrors);
}

-(id)_webkit_initWithDomain:(NSString *)domain code:(int)code URL:(NSURL *)URL
{
    NSDictionary *descriptionsDict;
    NSString *localizedDesc;
    NSDictionary *dict;
    // insert a localized string here for those folks not savvy to our category methods
    descriptionsDict = [descriptions objectForKey:domain];
    localizedDesc = descriptionsDict ? [descriptionsDict objectForKey:[NSNumber numberWithInt:code]] : nil;
    dict = [NSDictionary dictionaryWithObjectsAndKeys:
        URL, @"NSErrorFailingURLKey",
        [URL absoluteString], @"NSErrorFailingURLStringKey",
        localizedDesc, NSLocalizedDescriptionKey,
        nil];
    return [self initWithDomain:domain code:code userInfo:dict];
}

+(id)_webkit_errorWithDomain:(NSString *)domain code:(int)code URL:(NSURL *)URL
{
    return [[[self alloc] _webkit_initWithDomain:domain code:code URL:URL] autorelease];
}

+ (NSError *)_webKitErrorWithDomain:(NSString *)domain code:(int)code URL:(NSURL *)URL
{
    [self _registerWebKitErrors];
    return [self _webkit_errorWithDomain:domain code:code URL:URL];
}

+ (NSError *)_webKitErrorWithCode:(int)code failingURL:(NSString *)URLString
{
    return [self _webKitErrorWithDomain:WebKitErrorDomain code:code URL:[NSURL URLWithString:URLString]];
}

+ (void)_webkit_addErrorsWithCodesAndDescriptions:(NSDictionary *)dictionary inDomain:(NSString *)domain
{
    if (!descriptions)
        descriptions = [[NSMutableDictionary alloc] init];

    [descriptions setObject:dictionary forKey:domain];
}

static void registerErrors()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    NSDictionary *dict = [NSDictionary dictionaryWithObjectsAndKeys:
        // Policy errors
        WebKitErrorDescriptionCannotShowMIMEType,                   [NSNumber numberWithInt: WebKitErrorCannotShowMIMEType],
        WebKitErrorDescriptionCannotShowURL,                        [NSNumber numberWithInt: WebKitErrorCannotShowURL],
        WebKitErrorDescriptionFrameLoadInterruptedByPolicyChange,   [NSNumber numberWithInt: WebKitErrorFrameLoadInterruptedByPolicyChange],
        WebKitErrorDescriptionCannotUseRestrictedPort,              [NSNumber numberWithInt: WebKitErrorCannotUseRestrictedPort],
        
        // Plug-in and java errors
        WebKitErrorDescriptionCannotFindPlugin,                     [NSNumber numberWithInt: WebKitErrorCannotFindPlugIn],
        WebKitErrorDescriptionCannotLoadPlugin,                     [NSNumber numberWithInt: WebKitErrorCannotLoadPlugIn],
        WebKitErrorDescriptionJavaUnavailable,                      [NSNumber numberWithInt: WebKitErrorJavaUnavailable],
        WebKitErrorDescriptionPlugInCancelledConnection,            [NSNumber numberWithInt: WebKitErrorPlugInCancelledConnection],
        WebKitErrorDescriptionPlugInWillHandleLoad,                 [NSNumber numberWithInt: WebKitErrorPlugInWillHandleLoad],
        nil];

    [NSError _webkit_addErrorsWithCodesAndDescriptions:dict inDomain:WebKitErrorDomain];

    [pool drain];
}

@end

namespace WebKit {

ResourceError cancelledError(const ResourceRequest& request)
{
    return [NSError _webKitErrorWithDomain:NSURLErrorDomain code:NSURLErrorCancelled URL:request.url()];
}

ResourceError blockedError(const ResourceRequest& request)
{
    return [NSError _webKitErrorWithDomain:WebKitErrorDomain code:WebKitErrorCannotUseRestrictedPort URL:request.url()];
}

ResourceError cannotShowURLError(const ResourceRequest& request)
{
    return [NSError _webKitErrorWithDomain:WebKitErrorDomain code:WebKitErrorCannotShowURL URL:request.url()];
}

ResourceError interruptForPolicyChangeError(const ResourceRequest& request)
{
    return [NSError _webKitErrorWithDomain:WebKitErrorDomain code:WebKitErrorFrameLoadInterruptedByPolicyChange URL:request.url()];
}

ResourceError cannotShowMIMETypeError(const ResourceResponse& response)
{
    return [NSError _webKitErrorWithDomain:NSURLErrorDomain code:WebKitErrorCannotShowMIMEType URL:response.url()];
}

ResourceError fileDoesNotExistError(const ResourceResponse& response)
{
    return [NSError _webKitErrorWithDomain:NSURLErrorDomain code:NSURLErrorFileDoesNotExist URL:response.url()];    
}

} // namespace WebKit
