/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Google Inc. All rights reserved.
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

#ifndef FloatArray_h
#define FloatArray_h

#include "TypedArrayBase.h"
#include <wtf/MathExtras.h>

namespace WebCore {

class FloatArray : public TypedArrayBase<float> {
  public:
    static PassRefPtr<FloatArray> create(unsigned length);
    static PassRefPtr<FloatArray> create(float* array, unsigned length);
    static PassRefPtr<FloatArray> create(PassRefPtr<ArrayBuffer> buffer, unsigned byteOffset, unsigned length);

    using TypedArrayBase<float>::set;

    void set(unsigned index, double value)
    {
        if (index >= TypedArrayBase<float>::m_length)
            return;
        if (isnan(value)) // Clamp NaN to 0
            value = 0;
        TypedArrayBase<float>::data()[index] = static_cast<float>(value);
    }

    // Invoked by the indexed getter. Does not perform range checks; caller
    // is responsible for doing so and returning undefined as necessary.
    float item(unsigned index) const
    {
        ASSERT(index < TypedArrayBase<float>::m_length);
        float result = TypedArrayBase<float>::data()[index];
        if (isnan(result)) {
            // Clamp NaN to 0
            result = 0;
        }
        return result;
    }

  private:
    FloatArray(PassRefPtr<ArrayBuffer> buffer,
                    unsigned byteOffset,
                    unsigned length);
    // Make constructor visible to superclass.
    friend class TypedArrayBase<float>;

    // Overridden from ArrayBufferView.
    virtual bool isFloatArray() const { return true; }
    virtual PassRefPtr<ArrayBufferView> slice(int start, int end) const;
};

} // namespace WebCore

#endif // FloatArray_h
