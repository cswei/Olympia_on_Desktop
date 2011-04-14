/*
 * Copyright (C) 2006, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2008, 2010 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2007 Alp Toker <alp@atoker.com>
 * Copyright (C) 2008 Eric Seidel <eric@webkit.org>
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

#include "config.h"
#include "CanvasStyle.h"

#include "CSSParser.h"
#include "CanvasGradient.h"
#include "CanvasPattern.h"
#include "GraphicsContext.h"
#include <wtf/PassRefPtr.h>

#if PLATFORM(CG)
#include <CoreGraphics/CGContext.h>
#endif

#if PLATFORM(QT)
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QColor>
#endif

namespace WebCore {

CanvasStyle::CanvasStyle(RGBA32 rgba)
    : m_type(RGBA)
    , m_rgba(rgba)
{
}

CanvasStyle::CanvasStyle(float grayLevel)
    : m_type(RGBA)
    , m_rgba(makeRGBA32FromFloats(grayLevel, grayLevel, grayLevel, 1.0f))
{
}

CanvasStyle::CanvasStyle(float grayLevel, float alpha)
    : m_type(RGBA)
    , m_rgba(makeRGBA32FromFloats(grayLevel, grayLevel, grayLevel, alpha))
{
}

CanvasStyle::CanvasStyle(float r, float g, float b, float a)
    : m_type(RGBA)
    , m_rgba(makeRGBA32FromFloats(r, g, b, a))
{
}

CanvasStyle::CanvasStyle(float c, float m, float y, float k, float a)
    : m_type(CMYKA)
    , m_rgba(makeRGBAFromCMYKA(c, m, y, k, a))
    , m_cmyka(c, m, y, k, a)
{
}

CanvasStyle::CanvasStyle(PassRefPtr<CanvasGradient> gradient)
    : m_type(Gradient)
    , m_gradient(gradient)
{
}

CanvasStyle::CanvasStyle(PassRefPtr<CanvasPattern> pattern)
    : m_type(ImagePattern)
    , m_pattern(pattern)
{
}

PassRefPtr<CanvasStyle> CanvasStyle::create(const String& color)
{
    RGBA32 rgba;
    if (!CSSParser::parseColor(rgba, color))
        return 0;
    return adoptRef(new CanvasStyle(rgba));
}

PassRefPtr<CanvasStyle> CanvasStyle::create(const String& color, float alpha)
{
    RGBA32 rgba;
    if (!CSSParser::parseColor(rgba, color))
        return 0;
    return adoptRef(new CanvasStyle(colorWithOverrideAlpha(rgba, alpha)));
}

PassRefPtr<CanvasStyle> CanvasStyle::create(PassRefPtr<CanvasGradient> gradient)
{
    if (!gradient)
        return 0;
    return adoptRef(new CanvasStyle(gradient));
}
PassRefPtr<CanvasStyle> CanvasStyle::create(PassRefPtr<CanvasPattern> pattern)
{
    if (!pattern)
        return 0;
    return adoptRef(new CanvasStyle(pattern));
}

void CanvasStyle::applyStrokeColor(GraphicsContext* context)
{
    if (!context)
        return;
    switch (m_type) {
    case RGBA:
        context->setStrokeColor(m_rgba, DeviceColorSpace);
        break;
    case CMYKA: {
        // FIXME: Do this through platform-independent GraphicsContext API.
        // We'll need a fancier Color abstraction to support CMYKA correctly
#if PLATFORM(CG)
        CGContextSetCMYKStrokeColor(context->platformContext(), m_cmyka.c, m_cmyka.m, m_cmyka.y, m_cmyka.k, m_cmyka.a);
#elif PLATFORM(QT)
        QPen currentPen = context->platformContext()->pen();
        QColor clr;
        clr.setCmykF(m_cmyka.c, m_cmyka.m, m_cmyka.y, m_cmyka.k, m_cmyka.a);
        currentPen.setColor(clr);
        context->platformContext()->setPen(currentPen);
#else
        context->setStrokeColor(m_rgba, DeviceColorSpace);
#endif
        break;
    }
    case Gradient:
        context->setStrokeGradient(canvasGradient()->gradient());
        break;
    case ImagePattern:
        context->setStrokePattern(canvasPattern()->pattern());
        break;
    }
}

void CanvasStyle::applyFillColor(GraphicsContext* context)
{
    if (!context)
        return;
    switch (m_type) {
    case RGBA:
        context->setFillColor(m_rgba, DeviceColorSpace);
        break;
    case CMYKA: {
        // FIXME: Do this through platform-independent GraphicsContext API.
        // We'll need a fancier Color abstraction to support CMYKA correctly
#if PLATFORM(CG)
        CGContextSetCMYKFillColor(context->platformContext(), m_cmyka.c, m_cmyka.m, m_cmyka.y, m_cmyka.k, m_cmyka.a);
#elif PLATFORM(QT)
        QBrush currentBrush = context->platformContext()->brush();
        QColor clr;
        clr.setCmykF(m_cmyka.c, m_cmyka.m, m_cmyka.y, m_cmyka.k, m_cmyka.a);
        currentBrush.setColor(clr);
        context->platformContext()->setBrush(currentBrush);
#else
        context->setFillColor(m_rgba, DeviceColorSpace);
#endif
        break;
    }
    case Gradient:
        context->setFillGradient(canvasGradient()->gradient());
        break;
    case ImagePattern:
        context->setFillPattern(canvasPattern()->pattern());
        break;
    }
}

}
