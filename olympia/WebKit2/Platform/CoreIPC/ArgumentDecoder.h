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

#ifndef ArgumentDecoder_h
#define ArgumentDecoder_h

#include "Attachment.h"
#include <wtf/Deque.h>
#include <wtf/TypeTraits.h>
#include <wtf/Vector.h>

namespace CoreIPC {

class ArgumentDecoder;
class Attachment;

namespace ArgumentCoders {

template<typename T> bool decode(ArgumentDecoder& decoder, T& t)
{
    return WTF::RemovePointer<T>::Type::decode(decoder, t);
}

}

class ArgumentDecoder {
public:
    ArgumentDecoder(const uint8_t* buffer, size_t bufferSize);
    ArgumentDecoder(const uint8_t* buffer, size_t bufferSize, Deque<Attachment>&);
    ~ArgumentDecoder();

    uint64_t destinationID() const { return m_destinationID; }

    bool decodeBytes(Vector<uint8_t>&);
    bool decodeBytes(uint8_t*, size_t);

    bool decodeBool(bool&);
    bool decodeUInt32(uint32_t&);
    bool decodeUInt64(uint64_t&);
    bool decodeInt32(int32_t&);
    bool decodeInt64(int64_t&);
    bool decodeFloat(float&);
    bool decodeDouble(double&);

    // Generic type decode function.
    template<typename T> bool decode(T& t)
    {
        return ArgumentCoders::decode<T>(*this, t);
    }

    // This overload exists so we can pass temporaries to decode. In the Star Trek future, it 
    // can take an rvalue reference instead.
    template<typename T> bool decode(const T& t)
    {
        return decode(const_cast<T&>(t));
    }

    bool removeAttachment(Attachment&);

    void debug();

private:
    ArgumentDecoder(const ArgumentDecoder&);
    ArgumentDecoder& operator=(const ArgumentDecoder&);

    void initialize(const uint8_t* buffer, size_t bufferSize);

    bool alignBufferPosition(unsigned alignment, size_t size);

    uint64_t m_destinationID;

    uint8_t* m_buffer;
    uint8_t* m_bufferPos;
    uint8_t* m_bufferEnd;

    Deque<Attachment> m_attachments;
};

template<> inline bool ArgumentDecoder::decode(bool& n)
{
    return decodeBool(n);
}

template<> inline bool ArgumentDecoder::decode(uint32_t& n)
{
    return decodeUInt32(n);
}

template<> inline bool ArgumentDecoder::decode(uint64_t& n)
{
    return decodeUInt64(n);
}

template<> inline bool ArgumentDecoder::decode(int32_t& n)
{
    return decodeInt32(n);
}

template<> inline bool ArgumentDecoder::decode(int64_t& n)
{
    return decodeInt64(n);
}

template<> inline bool ArgumentDecoder::decode(float& n)
{
    return decodeFloat(n);
}

template<> inline bool ArgumentDecoder::decode(double& n)
{
    return decodeDouble(n);
}

} // namespace CoreIPC

#endif // ArgumentDecoder_h
