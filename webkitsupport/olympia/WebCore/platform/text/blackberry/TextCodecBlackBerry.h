/*
 * Copyright (C) 2004, 2006, 2007 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Alexey Proskuryakov <ap@nypop.com>
 * Copyright (C) 2007-2008 Torch Mobile, Inc.
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef TextCodecBlackBerry_h
#define TextCodecBlackBerry_h

#include "PlatformString.h"
#include "TextCodec.h"
#include "TextEncoding.h"
#include <wtf/Vector.h>

namespace WebCore {

class TextCodecBlackBerry : public TextCodec {
public:
    static void registerBaseEncodingNames(EncodingNameRegistrar);
    static void registerBaseCodecs(TextCodecRegistrar);

    static void registerExtendedEncodingNames(EncodingNameRegistrar);
    static void registerExtendedCodecs(TextCodecRegistrar);

    TextCodecBlackBerry(unsigned);
    virtual ~TextCodecBlackBerry();

    virtual String decode(const char*, size_t length, bool flush, bool stopOnError, bool& sawError);
    virtual CString encode(const UChar*, size_t length, UnencodableHandling);

private:
    Vector<char> m_decodeBuffer;
    unsigned m_encodingID;
};

} // namespace WebCore

#endif // TextCodecBlackBerry_h
