/*
 * Copyright (C) 2010 University of Szeged
 * Copyright (C) 2010 Zoltan Herczeg
 * All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY UNIVERSITY OF SZEGED ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL UNIVERSITY OF SZEGED OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SVGFELighting_h
#define SVGFELighting_h

#if ENABLE(SVG) && ENABLE(FILTERS)
#include "Color.h"
#include "Filter.h"
#include "FilterEffect.h"
#include "SVGLightSource.h"
#include <wtf/AlwaysInline.h>

// Common base class for FEDiffuseLighting and FESpecularLighting

namespace WebCore {

class CanvasPixelArray;

class FELighting : public FilterEffect {
public:
    virtual FloatRect uniteChildEffectSubregions(Filter* filter) { return calculateUnionOfChildEffectSubregions(filter, m_in.get()); }
    void apply(Filter*);

protected:
    enum LightingType {
        DiffuseLighting,
        SpecularLighting
    };

    struct LightingData {
        FloatPoint3D normalVector;
        CanvasPixelArray* pixels;
        float lightStrength;
        float surfaceScale;
        int offset;
        int widthMultipliedByPixelSize;
        int widthDecreasedByOne;
        int heightDecreasedByOne;

        ALWAYS_INLINE int upLeftPixelValue();
        ALWAYS_INLINE int upPixelValue();
        ALWAYS_INLINE int upRightPixelValue();
        ALWAYS_INLINE int leftPixelValue();
        ALWAYS_INLINE int centerPixelValue();
        ALWAYS_INLINE int rightPixelValue();
        ALWAYS_INLINE int downLeftPixelValue();
        ALWAYS_INLINE int downPixelValue();
        ALWAYS_INLINE int downRightPixelValue();
    };

    FELighting(LightingType, FilterEffect*, const Color&, float, float, float,
        float, float, float, PassRefPtr<LightSource>);

    bool drawLighting(CanvasPixelArray*, int, int);
    ALWAYS_INLINE void setPixel(LightingData&, LightSource::PaintingData&,
        int lightX, int lightY, float factorX, int normalX, float factorY, int normalY);

    LightingType m_lightingType;
    RefPtr<FilterEffect> m_in;
    RefPtr<LightSource> m_lightSource;

    Color m_lightingColor;
    float m_surfaceScale;
    float m_diffuseConstant;
    float m_specularConstant;
    float m_specularExponent;
    float m_kernelUnitLengthX;
    float m_kernelUnitLengthY;
};

} // namespace WebCore

#endif // ENABLE(SVG) && ENABLE(FILTERS)

#endif // SVGFELighting_h
