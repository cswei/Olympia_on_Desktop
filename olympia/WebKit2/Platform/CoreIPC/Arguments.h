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

#ifndef Arguments_h
#define Arguments_h

#include "ArgumentDecoder.h"
#include "ArgumentEncoder.h"

namespace CoreIPC {
    
class Arguments0 {
public:
    void encode(ArgumentEncoder&) const 
    {
    }

    static bool decode(ArgumentDecoder&, Arguments0&)
    {
        return true;
    }
};

inline Arguments0 In()
{
    return Arguments0();
}

inline Arguments0 Out()
{
    return Arguments0();
}

template<typename T1> class Arguments1 {
public:
    Arguments1(T1 t1) 
        : m_value(t1)
    {
    }

    void encode(ArgumentEncoder& encoder) const 
    {
        encoder.encode(m_value);
    }

    static bool decode(ArgumentDecoder& decoder, Arguments1<T1>& result)
    {
        return decoder.decode(result.m_value);
    }
    
private:
    T1 m_value;
};
    
template<typename T1> Arguments1<const T1&> In(const T1& t1) 
{
    return Arguments1<const T1&>(t1);
}

template<typename T1> Arguments1<T1&> Out(T1& t1)
{
    return Arguments1<T1&>(t1);
}

template<typename T1, typename T2> class Arguments2 : Arguments1<T1> {
public:
    Arguments2(T1 t1, T2 t2) 
        : Arguments1<T1>(t1)
        , m_value(t2)
    {
    }

    void encode(ArgumentEncoder& encoder) const 
    {
        Arguments1<T1>::encode(encoder);
        encoder.encode(m_value);
    }

    static bool decode(ArgumentDecoder& decoder, Arguments2<T1, T2>& result)
    {
        if (!Arguments1<T1>::decode(decoder, result))
            return false;
        
        return decoder.decode(result.m_value);
    }

private:
    T2 m_value;
};

template<typename T1, typename T2> Arguments2<const T1&, const T2&> In(const T1& t1, const T2& t2)
{
    return Arguments2<const T1&, const T2&>(t1, t2);
}

template<typename T1, typename T2> Arguments2<T1&, T2&> Out(T1& t1, T2& t2)
{
    return Arguments2<T1&, T2&>(t1, t2);
}

template<typename T1, typename T2, typename T3> class Arguments3 : Arguments2<T1, T2> {
public:
    Arguments3(T1 t1, T2 t2, T3 t3) 
        : Arguments2<T1, T2>(t1, t2)
        , m_value(t3)
    {
    }

    void encode(ArgumentEncoder& encoder) const 
    {
        Arguments2<T1, T2>::encode(encoder);
        encoder.encode(m_value);
    }

    static bool decode(ArgumentDecoder& decoder, Arguments3<T1, T2, T3>& result)
    {
        if (!Arguments2<T1, T2>::decode(decoder, result))
            return false;
        
        return decoder.decode(result.m_value);
    }

private:
    T3 m_value;
};

template<typename T1, typename T2, typename T3> Arguments3<const T1&, const T2&, const T3&> In(const T1& t1, const T2& t2, const T3 &t3)
{
    return Arguments3<const T1&, const T2&, const T3&>(t1, t2, t3);
}

template<typename T1, typename T2, typename T3> Arguments3<T1&, T2&, T3&> Out(T1& t1, T2& t2, T3& t3)
{
    return Arguments3<T1&, T2&, T3&>(t1, t2, t3);
}

template<typename T1, typename T2, typename T3, typename T4> class Arguments4 : Arguments3<T1, T2, T3> {
public:
    Arguments4(T1 t1, T2 t2, T3 t3, T4 t4)
        : Arguments3<T1, T2, T3>(t1, t2, t3)
        , m_value(t4)
    {
    }

    void encode(ArgumentEncoder& encoder) const
    {
        Arguments3<T1, T2, T3>::encode(encoder);
        encoder.encode(m_value);
    }
    
    static bool decode(ArgumentDecoder& decoder, Arguments4<T1, T2, T3, T4>& result)
    {
        if (!Arguments3<T1, T2, T3>::decode(decoder, result))
            return false;
        
        return decoder.decode(result.m_value);
    }

private:
    T4 m_value;
};

template<typename T1, typename T2, typename T3, typename T4> Arguments4<const T1&, const T2&, const T3&, const T4&> In(const T1& t1, const T2& t2, const T3 &t3, const T4& t4)
{
    return Arguments4<const T1&, const T2&, const T3&, const T4&>(t1, t2, t3, t4);
}

template<typename T1, typename T2, typename T3, typename T4> Arguments4<T1&, T2&, T3&, T4&> Out(T1& t1, T2& t2, T3& t3, T4& t4)
{
    return Arguments4<T1&, T2&, T3&, T4&>(t1, t2, t3, t4);
}

template<typename T1, typename T2, typename T3, typename T4, typename T5> class Arguments5 : Arguments4<T1, T2, T3, T4> {
public:
    Arguments5(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
        : Arguments4<T1, T2, T3, T4>(t1, t2, t3, t4)
        , m_value(t5)
    {
    }

    void encode(ArgumentEncoder& encoder) const
    {
        Arguments4<T1, T2, T3, T4>::encode(encoder);
        encoder.encode(m_value);
    }
    
    static bool decode(ArgumentDecoder& decoder, Arguments5<T1, T2, T3, T4, T5>& result)
    {
        if (!Arguments4<T1, T2, T3, T4>::decode(decoder, result))
            return false;
        
        return decoder.decode(result.m_value);
    }

private:
    T5 m_value;
};

template<typename T1, typename T2, typename T3, typename T4, typename T5> Arguments5<const T1&, const T2&, const T3&, const T4&, const T5&> In(const T1& t1, const T2& t2, const T3 &t3, const T4& t4, const T5& t5)
{
    return Arguments5<const T1&, const T2&, const T3&, const T4&, const T5&>(t1, t2, t3, t4, t5);
}

template<typename T1, typename T2, typename T3, typename T4, typename T5> Arguments5<T1&, T2&, T3&, T4&, T5&> Out(T1& t1, T2& t2, T3& t3, T4& t4, T5& t5)
{
    return Arguments5<T1&, T2&, T3&, T4&, T5&>(t1, t2, t3, t4, t5);
}

} // namespace CoreIPC

#endif // Arguments_h
