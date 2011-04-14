/*
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

#include <ft2build.h>
#include FT_FREETYPE_H
#include "OlympiaPlatformAssert.h"
#include <map>
#include "text_api.h"
#include "TextEngineFreeType.h"
#include <VG/openvg.h>

namespace TextAPI {

class FontFreeType: public Font {
public:
    FontFreeType(EngineFreeType* engine);
    ~FontFreeType();
    virtual ReturnCode setFontSpec(const FontSpec& fontSpec);
    virtual void getFontSpec(FontSpec& fontSpec);
    virtual void getMatchedFontSpec(FontSpec& fontSpec);
    virtual ReturnCode setHeight(double height);
    virtual bool operator==(const Font& font);
    virtual void getFontMetrics(FontMetrics& metrics);
    ReturnCode drawText(GraphicsContext* gc, const Utf16Char* text, int lenght, double x, double y, double wrap, const DrawParam* param, TextMetrics* metrics);
private:
    bool operator==(const FontFreeType& that);
    void calculateExtraWidths();
    void loadGlyph(Utf16Char charater, FT_GlyphSlot slot);
    void drawGlyphs(GraphicsContext* gc, VGFont vgFont, VGfloat x, VGfloat y, VGuint* vgGlyphs, VGuint numGlyphs, VGfloat* adjustmentsX, VGfloat* adjustmentsY);
    VGFont vgFont();
    void clearVGGlyphs();
private:
    FontSpec m_spec;
    FontSpec m_matchedSpec;
    FT_Face m_ftFace;
    EngineFreeType* m_engine;
    double m_averageCharWidth;
    double m_spaceWidth;
    VGFont m_vgFont;
    typedef std::map<VGuint, GlyphDataSlot*> GlyphCache;
    GlyphCache m_glyphCache;
    bool m_needsCheckCharactersFallback;
#if defined(USE_FREETYPE_RENDER) && USE_FREETYPE_RENDER
    Color m_fillColor;
    Color m_strokeColor;
#endif
};

}
