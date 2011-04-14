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

#include "config.h"
#include "OlympiaStreamFactory.h"

#include "HttpStreamDebugger.h"
#include "KURL.h"
#include "NetworkRequest.h"
#include "OlympiaHttpStreamQt.h"

namespace Olympia {

namespace Platform {

OlympiaStreamFactory* OlympiaStreamFactory::instance()
{
    static OlympiaStreamFactory* s_instance = 0;
    if (!s_instance)
        s_instance = new OlympiaStreamFactory;
    return s_instance;
}

OlympiaStreamFactory::OlympiaStreamFactory()
{

}

IStream* OlympiaStreamFactory::createNetworkStream(const NetworkRequest& request, int playerId, HttpStreamDebugger*)
{
    //std::string url = request.getUrlRef();
    //WebCore::KURL kurl(WebCore::ParsedURLString, url.data());
    //IStream* stream;
    //if(kurl.protocolIs("http") || kurl.protocolIs("https"))
    IStream* stream = new OlympiaHttpStreamQt(); // FIXME: just create this so far.
    stream->setRequest(request);
    return stream;
}

} // namespace Platform

} // namespace Olympia
