/*
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 * Copyright (C) 2010 Torch Mobile(Beijing) CO. Ltd. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "config.h"
#include "TextFontFreeType.h"

#include <EGL/egl.h>
#include "EGLUtils.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#if defined(USE_FREETYPE_RENDER) && USE_FREETYPE_RENDER
#include FT_STROKER_H
#endif
#include <limits>
#include "OlympiaPlatformAssert.h"
#include "TextEngineFreeType.h"
#include "TextGraphicsContextOpenVG.h"
#include "TextUtilsFreeType.h"
#include <VG/openvg.h>
#include "VGUtils.h"

#define UNITS_PER_PIXEL 64.0f

namespace TextAPI {

inline static bool operator==(const ColorSpec& c1, const ColorSpec& c2)
{
    return c1.m_color == c2.m_color
        && c1.m_useColor == c2.m_useColor;
}

inline static bool operator==(const Dimension& d1, const Dimension& d2)
{
    return d1.m_value == d2.m_value
        && d1.m_unit == d2.m_unit;
}

inline static bool operator==(const Shadow& s1, const Shadow& s2)
{
    return s1.m_type == s2.m_type
        && s1.m_xOffset == s2.m_xOffset
        && s1.m_yOffset == s2.m_yOffset
        && s1.m_blurRadius == s2.m_blurRadius
        && s1.m_glyphColor == s2.m_glyphColor
        && s1.m_shadowColor == s2.m_shadowColor;
}

inline static bool operator==(const OutlineEffect& oe1, const OutlineEffect& oe2)
{
    return oe1.m_type == oe2.m_type
        && oe1.m_penWidth == oe2.m_penWidth
        && oe1.m_glyphColor == oe2.m_glyphColor
        && oe1.m_outlineColor == oe2.m_outlineColor;
}

inline static bool operator==(const FontSpec& spec1, const FontSpec& spec2)
{
    return spec1.m_height == spec2.m_height
        && spec1.m_weight == spec2.m_weight
        && spec1.m_style == spec2.m_style
        && spec1.m_variant == spec2.m_variant
        && spec1.m_monospace == spec2.m_monospace
        && spec1.m_underline == spec2.m_underline
        && spec1.m_hintingMode == spec2.m_hintingMode
        && spec1.m_transform[0] == spec2.m_transform[0]
        && spec1.m_transform[1] == spec2.m_transform[1]
        && spec1.m_transform[2] == spec2.m_transform[2]
        && spec1.m_transform[3] == spec2.m_transform[3]
        && spec1.m_outline == spec2.m_outline
        && spec1.m_shadow == spec2.m_shadow
        && Utf16StringCompareIgnoreCase(spec1.m_name, spec2.m_name) == 0;
}

static int addVGMoveToSegment(const FT_Vector* to, void* vgPath)
{
    static const VGubyte pathSegments[] = { VG_MOVE_TO_ABS };
    const VGfloat pathData[] = { to->x / UNITS_PER_PIXEL, to->y / -UNITS_PER_PIXEL };
    vgAppendPathData(*static_cast<VGPath*>(vgPath), 1, pathSegments, pathData);
    ASSERT_VG_NO_ERROR();
    return 0;
}

static int addVGLineToSegment(const FT_Vector* to, void* vgPath)
{
    static const VGubyte pathSegments[] = { VG_LINE_TO_ABS };
    const VGfloat pathData[] = { to->x / UNITS_PER_PIXEL, to->y / -UNITS_PER_PIXEL };
    vgAppendPathData(*static_cast<VGPath*>(vgPath), 1, pathSegments, pathData);
    ASSERT_VG_NO_ERROR();
    return 0;
}

static int addVGConicToSegment(const FT_Vector* control, const FT_Vector* to, void* vgPath)
{
    static const VGubyte pathSegments[] = { VG_QUAD_TO_ABS };
    const VGfloat pathData[] = { control->x / UNITS_PER_PIXEL,
        control->y / -UNITS_PER_PIXEL,
        to->x / UNITS_PER_PIXEL,
        to->y / -UNITS_PER_PIXEL
    };
    vgAppendPathData(*static_cast<VGPath*>(vgPath), 1, pathSegments, pathData);
    ASSERT_VG_NO_ERROR();
    return 0;
}

static int addVGCubicToSegment(const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, void* vgPath)
{
    static const VGubyte pathSegments[] = { VG_CUBIC_TO_ABS };
    const VGfloat pathData[] = { control1->x / UNITS_PER_PIXEL,
        control1->y / -UNITS_PER_PIXEL,
        control2->x / UNITS_PER_PIXEL,
        control2->y / -UNITS_PER_PIXEL,
        to->x / UNITS_PER_PIXEL,
        to->y / -UNITS_PER_PIXEL
    };
    vgAppendPathData(*static_cast<VGPath*>(vgPath), 1, pathSegments, pathData);
    ASSERT_VG_NO_ERROR();
    return 0;
}

#if defined(USE_FREETYPE_RENDER) && USE_FREETYPE_RENDER

struct RasterUserData {
    unsigned char* imageBuffer;
    int imageStride;
    int imageHeight;
    int xMin;
    int yMin;
    Color strokeColor;
    Color fillColor;
    bool strokeStage;
};

static void rasterCallback(int y, int count, const FT_Span* ftSpans, void* user)
{
    RasterUserData* data = static_cast<RasterUserData*>(user);
    Color& color = data->strokeStage ? data->strokeColor : data->fillColor;
    unsigned char* lineBuffer = data->imageBuffer + (data->imageHeight - 1 - (y - data->yMin)) * data->imageStride;
    for (int i = 0; i < count; ++i) {
        int x = ftSpans[i].x - data->xMin;
        for (int w = 0; w < ftSpans[i].len; ++w) {
            unsigned char* buffer = lineBuffer + (x + w) * sizeof(VGuint);
            unsigned char alpha = color.getAlpha();
            unsigned char coverage = ftSpans[i].coverage;
            buffer[3] = color.getBlue();
            buffer[2] = color.getGreen();
            buffer[1] = color.getRed();
            // Blend alpha only.
            buffer[0] = alpha == 255 ? coverage : (coverage == 255 ? alpha : static_cast<unsigned int>(alpha) * coverage / 255);
        }
    }
}
#endif

FontFreeType::FontFreeType(EngineFreeType* engine)
    : m_ftFace(0)
    , m_engine(engine)
    , m_averageCharWidth(0)
    , m_spaceWidth(0)
    , m_vgFont(0)
    , m_needsCheckCharactersFallback(true)
{
    OLYMPIA_ASSERT(m_engine);
}

FontFreeType::~FontFreeType()
{
    clearVGGlyphs();
    if (m_vgFont) {
        vgDestroyFont(m_vgFont);
        ASSERT_VG_NO_ERROR();
    }
}

void FontFreeType::drawGlyphs(GraphicsContext* gc, VGFont vgFont, VGfloat x, VGfloat y, VGuint* vgGlyphs, VGuint numGlyphs, VGfloat* adjustmentsX, VGfloat* adjustmentsY)
{
    GraphicsContextOpenVG* context = 0;
    if (gc->getType() == OpenVGGraphicsContext)
        context = static_cast<GraphicsContextOpenVG*>(gc);
    if (!context)
        return;

    eglMakeCurrent(context->eglDisplay(), context->eglSurface(), context->eglSurface(), context->eglContext());
    ASSERT_EGL_NO_ERROR();

    const VGfloat vgPoint[2] = { x, y };
    vgSetfv(VG_GLYPH_ORIGIN, 2, vgPoint);
    ASSERT_VG_NO_ERROR();
 
    VGbitfield paintModes = VG_FILL_PATH;

    vgDrawGlyphs(vgFont, numGlyphs, vgGlyphs, adjustmentsX, adjustmentsY, paintModes, VG_TRUE);
    ASSERT_VG_NO_ERROR();
}

void FontFreeType::loadGlyph(Utf16Char character, FT_GlyphSlot slot)
{
    if (slot->format == FT_GLYPH_FORMAT_OUTLINE) {
        VGfloat escapement[] = { slot->advance.x / UNITS_PER_PIXEL, slot->advance.y / UNITS_PER_PIXEL };

#if defined(USE_FREETYPE_RENDER) && USE_FREETYPE_RENDER
        VGfloat glyphOrigin[] = { /*slot->metrics.horiBearingX / UNITS_PER_PIXEL*/0, slot->metrics.horiBearingY / UNITS_PER_PIXEL };
        FT_Glyph ftGlyph;
        FT_Error error = FT_Get_Glyph(slot, &ftGlyph);
        OLYMPIA_ASSERT(!error);
        OLYMPIA_ASSERT(ftGlyph->format == FT_GLYPH_FORMAT_OUTLINE);
        FT_BBox glyphBox;
        FT_Glyph_Get_CBox(ftGlyph, FT_GLYPH_BBOX_PIXELS, &glyphBox);

        int imageWidth = glyphBox.xMax - glyphBox.xMin;
        int imageHeight = glyphBox.yMax - glyphBox.yMin;
        if (imageWidth && imageHeight) {
            int imageSizeInBytes = imageWidth * imageHeight * sizeof(VGuint);
            int imageStride = imageWidth * sizeof(VGuint);
            RasterUserData userData;
            unsigned char* imageBuffer = new unsigned char[imageSizeInBytes];
            memset(imageBuffer, 0, imageSizeInBytes);
            userData.imageBuffer = imageBuffer;
            userData.imageHeight = imageHeight;
            userData.imageStride = imageStride;
            userData.xMin = glyphBox.xMin;
            userData.yMin = glyphBox.yMin;
            userData.strokeColor = m_strokeColor;
            userData.fillColor = m_fillColor;
            userData.strokeStage = false;

            FT_Raster_Params ftRasterParams;
            memset(&ftRasterParams, 0, sizeof(ftRasterParams));
            ftRasterParams.flags = FT_RASTER_FLAG_AA | FT_RASTER_FLAG_DIRECT;
            ftRasterParams.gray_spans = rasterCallback;
            ftRasterParams.user = &userData;
            error = FT_Outline_Render(m_engine->ftLibrary(), &slot->outline, &ftRasterParams);
            OLYMPIA_ASSERT(!error);

            FT_Done_Glyph(ftGlyph);

            VGImageFormat imageFormat = VG_sBGRA_8888;
            VGImage image = vgCreateImage(imageFormat,
                imageWidth,
                imageHeight,
                VG_IMAGE_QUALITY_FASTER);
            ASSERT_VG_NO_ERROR();

            vgImageSubData(image,
                           imageBuffer,
                           imageStride,
                           imageFormat,
                           0,
                           0,
                           imageWidth,
                           imageHeight);
            delete[] imageBuffer;
            ASSERT_VG_NO_ERROR();

            vgSetGlyphToImage(m_vgFont, character, image, glyphOrigin, escapement);
            ASSERT_VG_NO_ERROR();
            vgDestroyImage(image);
            ASSERT_VG_NO_ERROR();
        } else {
            vgSetGlyphToPath(m_vgFont, character, VG_INVALID_HANDLE, VG_TRUE, glyphOrigin, escapement);
            ASSERT_VG_NO_ERROR();
        }
#else
        if (m_glyphCache.find(character) != m_glyphCache.end())
            return;

        GlyphDataSlot* glyphDataSlot = m_engine->allocateGlyphDataSlot();
        VGPath path = glyphDataSlot->m_glyphPath;

        FT_Outline_Funcs callbacks;
        callbacks.move_to = addVGMoveToSegment;
        callbacks.line_to = addVGLineToSegment;
        callbacks.conic_to = addVGConicToSegment;
        callbacks.cubic_to = addVGCubicToSegment;
        callbacks.shift = 0;
        callbacks.delta = 0;
        FT_Error error = FT_Outline_Decompose(&slot->outline, &callbacks, &path);
        OLYMPIA_ASSERT(!error);

        // Auto-close the segment, as there's no callback function for that.
        static const VGubyte pathSegments[] = { VG_CLOSE_PATH };
        // pathData must not be 0, but certain compilers also don't create
        // zero-size arrays. So let's use a random aligned value (sizeof(VGfloat)),
        // it won't be accessed anyways as VG_CLOSE_PATH doesn't take coordinates.
        static const VGfloat* pathData = reinterpret_cast<VGfloat*>(sizeof(VGfloat));
        vgAppendPathData(path, 1, pathSegments, pathData);
        ASSERT_VG_NO_ERROR();

        VGfloat glyphOrigin[] = { 0.f, 0.f };
        vgSetGlyphToPath(m_vgFont, character, path, VG_TRUE /*isHinted*/, glyphOrigin, escapement);
        ASSERT_VG_NO_ERROR();
        m_glyphCache.insert(std::make_pair(character, glyphDataSlot));
#endif
    } else {
        //TODO: support bitmap font
        VGfloat escapement[] = { slot->advance.x / UNITS_PER_PIXEL, slot->advance.y / UNITS_PER_PIXEL };
        VGfloat glyphOrigin[] = { 0.f, 0.f };
        vgSetGlyphToPath(m_vgFont, character, VG_INVALID_HANDLE, VG_TRUE, glyphOrigin, escapement);
    }
}

inline static bool isZeroWidthSpace(Utf16Char c)
{
    // Unicode standard.
    // http://www.unicodemap.org/details/0x200B/index.html
    return c == 0x200b;
}

ReturnCode FontFreeType::drawText(GraphicsContext* gc, const Utf16Char* text, int length, double x, double y, double wrap, const DrawParam* param, TextMetrics* metrics)
{
    OLYMPIA_ASSERT(m_ftFace && (gc || metrics) && text && length > 0);
    if (!m_ftFace || !(gc || metrics) || !text || length <= 0)
        return -1;

    // Some callers of EngineFreeType::matchFontBySpec may specify a
    // font family which doesn't contain characters that it wants to
    // be drawn. For example, a webpage author specify Times New
    // Roman in CSS but the content contains Chinese characters.
    // We check this case and switch font face if needed.
    // If fallback occurs in such a scenario:
    //    B: bad char which isn't in the current face
    //    G: good char which is in the current face
    //    Consider the following 3 continuous calls:
    //      1) drawText("GG"), 
    //      2) drawText("GBBG");
    //      3) drawText("GG");
    //    Good chars in 2) will be drawn using different face with 1) and 3).
    if (m_needsCheckCharactersFallback) {
        FontFreeType* fallbackFont = m_engine->checkCharactersAndFallbackFont(m_ftFace, m_matchedSpec, text, length);
        if (fallbackFont) {
            fallbackFont->m_needsCheckCharactersFallback = false;
            return fallbackFont->drawText(gc, text, length, x, y, wrap, param, metrics);
        }
    }

    // There maybe mulitple FontFreeType objects share the same
    // FreeType font face. The size maybe switched by other objects.
    // We switch back to ours here.
    FT_Error error = FT_Set_Pixel_Sizes(m_ftFace, 0, static_cast<FT_UInt>(m_matchedSpec.m_height));
    OLYMPIA_ASSERT(!error);

    FT_Bool useKerning = FT_HAS_KERNING(m_ftFace);
    FT_UInt previous = 0;
    double penX = x;
    double penY = y;

    bool rtl = false;
    double letterSpace = 0;
    double wordSpace = 0;
    CaseTransform caseTransform = NoCaseTransform;
    if (param) {
        // Letter space
        if (param->m_letterspace.m_unit == UserUnit)
            letterSpace = param->m_letterspace.m_value;
        else
            letterSpace = param->m_letterspace.m_value * m_matchedSpec.m_height;

        // Word space. WebCore only uses WordSpaceAdd mode.
        if (param->m_wordspace.m_mode == WordSpaceAdd) {
            if (param->m_wordspace.m_size.m_unit == UserUnit)
                wordSpace = param->m_wordspace.m_size.m_value;
            else
                wordSpace = param->m_wordspace.m_size.m_value * m_matchedSpec.m_height;
        }

        // Case transform
        caseTransform = param->m_caseTransform;

        // TextOrder
        rtl = param->m_textOrder == ReverseTextOrder;
    }

    if (metrics) {
        // FreeType's Y coordinate is opposite to metrics' Y coordinate.
        metrics->m_boundsLeft = metrics->m_boundsTop = std::numeric_limits<int>::max();
        metrics->m_boundsRight = metrics->m_boundsBottom = std::numeric_limits<int>::min();
        metrics->m_consumed = 0;
    }

    int i = rtl ? length - 1 : 0;
    int advance = 0;
    VGuint* vgGlyphs = 0;
    VGfloat* vgGlyphKernings = 0;
    if (gc) {
        vgGlyphs = new VGuint[length];
        vgGlyphKernings = new VGfloat[length];
#if defined(USE_FREETYPE_RENDER) && USE_FREETYPE_RENDER
        m_fillColor = static_cast<GraphicsContextOpenVG*>(gc)->fillColor();
        m_strokeColor = static_cast<GraphicsContextOpenVG*>(gc)->strokeColor();
#endif
        // Ensure m_vgFont is not NULL here to improve performance
        // because we need not call vgFont() in every loadGlyph call.
        vgFont();
    }
    VGuint numGlyphs = 0;
    bool isSmallCaps = m_matchedSpec.m_variant == SmallCapsVariant;
    double smallCapsHeight = m_matchedSpec.m_height * 0.75;
    double lastHeight = m_matchedSpec.m_height;
    while (i >= 0 && i < length && (!wrap || advance < wrap)) {
        if (isZeroWidthSpace(text[i])) {
            rtl ? --i : ++i;
            continue;
        }

        Utf16Char character = text[i];
        if (isSmallCaps) {
            if (character >= 'a' && character <= 'z') {
                character -= 'a' - 'A';
                if (lastHeight != smallCapsHeight) {
                    FT_Set_Pixel_Sizes(m_ftFace, 0, static_cast<FT_UInt>(smallCapsHeight));
                    lastHeight = smallCapsHeight;
                }
            } else if (lastHeight != m_matchedSpec.m_height) {
                FT_Set_Pixel_Sizes(m_ftFace, 0, static_cast<FT_UInt>(m_matchedSpec.m_height));
                lastHeight = m_matchedSpec.m_height;
            }
        }

        FT_UInt glyphIndex = FT_Get_Char_Index(m_ftFace, character);
        FT_Error error = FT_Load_Glyph(m_ftFace, glyphIndex, FT_LOAD_NO_BITMAP);
        OLYMPIA_ASSERT(!error);

        VGfloat kerning = static_cast<VGfloat>(letterSpace);
        if (text[i] == 0x20)
            kerning += static_cast<VGfloat>(wordSpace);
        if (useKerning && previous && glyphIndex) {
            FT_Vector delta;
            FT_Get_Kerning(m_ftFace, previous, glyphIndex, FT_KERNING_DEFAULT, &delta );
            kerning += delta.x >> 6;
        }

        if (rtl)
            penX -= (m_ftFace->glyph->advance.x >> 6) + kerning;
        else
            penX += (m_ftFace->glyph->advance.x >> 6) + kerning;

        advance = rtl ? static_cast<int>(x - penX) : static_cast<int>(penX - x);
        previous = glyphIndex;

        if (metrics) {
            FT_Glyph glyph;
            error = FT_Get_Glyph(m_ftFace->glyph, &glyph);
            OLYMPIA_ASSERT(!error);
            FT_BBox glyphBox;
            FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_PIXELS, &glyphBox);
            glyphBox.xMin += static_cast<FT_Pos>(penX);
            glyphBox.xMax += static_cast<FT_Pos>(penX);
            glyphBox.yMin += static_cast<FT_Pos>(penY);
            glyphBox.yMax += static_cast<FT_Pos>(penY);
            
            if (glyphBox.xMin < metrics->m_boundsLeft)
                metrics->m_boundsLeft = glyphBox.xMin;
            if (glyphBox.yMin < metrics->m_boundsTop)
                metrics->m_boundsTop = glyphBox.yMin;
            if (glyphBox.xMax > metrics->m_boundsRight)
                metrics->m_boundsRight = glyphBox.xMax;
            if (glyphBox.yMax > metrics->m_boundsBottom)
                metrics->m_boundsBottom = glyphBox.yMax; 

            metrics->m_newX = penX;
            metrics->m_newY = penY;
            metrics->m_consumed++;
            metrics->m_linearAdvance = advance;
            FT_Done_Glyph(glyph);
        }

        if (gc) {
            loadGlyph(text[i], m_ftFace->glyph);
            vgGlyphs[numGlyphs] = text[i];
            vgGlyphKernings[numGlyphs] = kerning;
            ++numGlyphs;
        }

        rtl ? --i : ++i;
    }

    if (gc) {
        if (numGlyphs)
            drawGlyphs(gc,
                m_vgFont,
                static_cast<VGfloat>(x),
                static_cast<VGfloat>(y),
                vgGlyphs,
                numGlyphs,
                vgGlyphKernings,
                0);
        delete[] vgGlyphs;
        delete[] vgGlyphKernings;
    }

    return 0;
}

ReturnCode FontFreeType::setFontSpec(const FontSpec& fontSpec)
{
    OLYMPIA_ASSERT(m_engine);
    if (fontSpec == m_spec || fontSpec == m_matchedSpec)
        return 0;

    DEBUG_PRINT("FontFreeType::setFontSpec:\n");
    DEBUG_PRINT("\tFamily: ");
    DEBUG_PRINT_UTF16_STRING(fontSpec.m_name);
    DEBUG_PRINT("\n\tHeight: %.4lf\n", fontSpec.m_height);

    m_matchedSpec = fontSpec;
    bool isDefaultFace = false;
    m_ftFace = m_engine->matchFontFaceBySpec(m_matchedSpec, isDefaultFace);
    if (!m_ftFace)
        return -1;

    m_needsCheckCharactersFallback = !isDefaultFace;

    DEBUG_PRINT("\tMatched family: ");
    DEBUG_PRINT_UTF16_STRING(m_matchedSpec.m_name);
    DEBUG_PRINT("\n");

    FT_Error error = FT_Set_Pixel_Sizes(m_ftFace, 0, static_cast<FT_UInt>(m_matchedSpec.m_height));
    if (error) {
        DEBUG_PRINT("FrontFreeType::setFontSpec, can't set size: %d\n", static_cast<int>(fontSpec.m_height));
        return -1;
    }

    calculateExtraWidths();

    m_spec = fontSpec;

    clearVGGlyphs();
    return 0;
}

void FontFreeType::getFontSpec(FontSpec& fontSpec)
{
    fontSpec = m_spec;
}

void FontFreeType::getMatchedFontSpec(FontSpec& fontSpec)
{
    fontSpec = m_matchedSpec;
}

ReturnCode FontFreeType::setHeight(double height)
{
    OLYMPIA_ASSERT(m_ftFace);
    if (height == m_matchedSpec.m_height)
        return 0;

    DEBUG_PRINT("FontFreeType::setFontHeight: height = %.4lf\n", height);

    FT_Error error = FT_Set_Pixel_Sizes(m_ftFace, 0, static_cast<FT_UInt>(height));
    if (error) {
        DEBUG_PRINT("FrontFreeType::setFontSpec, can't set size: %d\n", static_cast<int>(height));
        return -1;
    }
    m_matchedSpec.m_height = height;
    calculateExtraWidths();
    clearVGGlyphs();
    return 0;
}

bool FontFreeType::operator==(const FontFreeType& that)
{
    return m_ftFace == that.m_ftFace
        && m_matchedSpec == that.m_matchedSpec
        && m_engine == that.m_engine;
}

bool FontFreeType::operator==(const Font& font)
{
    return *this == static_cast<const FontFreeType&>(font);
}

void FontFreeType::getFontMetrics(FontMetrics& metrics) 
{
    OLYMPIA_ASSERT(m_ftFace);
    if (!(m_ftFace->face_flags & FT_FACE_FLAG_SCALABLE)) {
        DEBUG_PRINT("FontFreeType::getFontMetrics, not a scalable font face\n");
        return;
    }
    FT_Fixed yScale = m_ftFace->size->metrics.y_scale;
    metrics.m_height = FT_MulFix(m_ftFace->height, yScale) / UNITS_PER_PIXEL;
    metrics.m_ascent = FT_MulFix(m_ftFace->ascender, yScale) / UNITS_PER_PIXEL;
    metrics.m_maxAscent = FT_MulFix(m_ftFace->bbox.yMax, yScale) / UNITS_PER_PIXEL;
    metrics.m_descent = -FT_MulFix(m_ftFace->descender, yScale) / UNITS_PER_PIXEL;
    metrics.m_maxDescent = -FT_MulFix(m_ftFace->bbox.yMin, yScale) / UNITS_PER_PIXEL;
    metrics.m_underlineOffset = -FT_MulFix(m_ftFace->underline_position, m_ftFace->size->metrics.y_scale) / UNITS_PER_PIXEL;
    metrics.m_underlineWeight = FT_MulFix(m_ftFace->underline_thickness, m_ftFace->size->metrics.y_scale) / UNITS_PER_PIXEL;
    metrics.m_leadingAbove = metrics.m_maxAscent - metrics.m_ascent;
    metrics.m_leadingBelow = metrics.m_maxDescent - metrics.m_descent;
    metrics.m_spaceWidth = m_spaceWidth;
    metrics.m_maxCharWidth =  FT_MulFix(m_ftFace->max_advance_width, m_ftFace->size->metrics.x_scale) / UNITS_PER_PIXEL;
    metrics.m_averageCharWidth = m_averageCharWidth;
}

void FontFreeType::calculateExtraWidths()
{
    OLYMPIA_ASSERT(m_ftFace);

    FT_Error error = FT_Load_Char(m_ftFace, 'x', FT_LOAD_NO_BITMAP);
    OLYMPIA_ASSERT(!error);
    m_averageCharWidth = m_ftFace->glyph->advance.x / UNITS_PER_PIXEL;

    error = FT_Load_Char(m_ftFace, ' ', FT_LOAD_NO_BITMAP);
    OLYMPIA_ASSERT(!error);
    m_spaceWidth = m_ftFace->glyph->advance.x / UNITS_PER_PIXEL;
}

VGFont FontFreeType::vgFont()
{
    if (!m_vgFont) {
        m_vgFont = vgCreateFont(0);
        ASSERT_VG_NO_ERROR();
    }
    return m_vgFont;
}

void FontFreeType::clearVGGlyphs()
{
    if (m_vgFont) {
        for (GlyphCache::const_iterator glyphIterator = m_glyphCache.begin(); glyphIterator != m_glyphCache.end(); ++glyphIterator) {
            vgClearGlyph(m_vgFont, glyphIterator->first);
            ASSERT_VG_NO_ERROR();
            m_engine->freeGlyphDataSlot(glyphIterator->second);
        }
    }
    m_glyphCache.clear();
}

}
