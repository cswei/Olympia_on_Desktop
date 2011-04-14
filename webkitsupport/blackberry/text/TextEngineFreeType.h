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

#ifndef TextEngineFreeType_H
#define TextEngineFreeType_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include <string>
#include "text_api.h"
#include "TextUtilsFreeType.h"
#include <VG/openvg.h>

namespace TextAPI {

class FontFreeType;

class GlyphDataSlot {
public:
    VGPath m_glyphPath;
private:
    GlyphDataSlot* m_next;
    friend class EngineFreeType;
};

class EngineFreeType: public Engine {
public:
    EngineFreeType();
    virtual ~EngineFreeType();
    virtual ReturnCode loadFontData(FontDataId& id, const Utf16Char* path, const Utf16Char* name, const AdvancedFontLoadingParam* advancedParam = 0);
    virtual ReturnCode loadFontData(FontDataId& id, const Stream& stream, const Utf16Char* name, const AdvancedFontLoadingParam* advancedParam = 0);
    virtual ReturnCode unloadFontData(FontDataId id);
    virtual Font* createFont(ReturnCode& returnCode, const FontSpec& fontSpec);
    virtual int clearGlyphCache();
    virtual void setFontLoaderDelegate(FontLoaderDelegate* fontLoaderDelegate);
    virtual ReturnCode setFontFamilyList(const Utf16Char* fontFamilyList); 
    virtual GraphicsContext* createGraphicsContext(ReturnCode& returnCode, GraphicsContextType type, NativeGraphicsDisplay display);
    virtual GraphicsContext* createGraphicsContext(ReturnCode& returnCode, GraphicsContextType type, NativeGraphicsDisplay display, NativeGraphicsContext context, NativeGraphicsSurface surface, bool ownGraphicsState);
    virtual ReturnCode drawText(GraphicsContext* gc, Font& font, const Utf16Char* text, int textLength, double x, double y, double wrap, const DrawParam* param, TextMetrics* metrics);
    virtual ReturnCode xToTextPos(double x, int& textPos, TextPosRounding rounding, Font& font, const Utf16Char* text, int textLength, const DrawParam& param);
    virtual ReturnCode textPosToX(int textPos, double &x, Font& font, const Utf16Char* text, int textLength, const DrawParam& param);
    virtual GlyphType setGlyphType(GlyphType type);
    virtual ReturnCode setResourceContext(NativeGraphicsDisplay aDisplay, NativeGraphicsSurface aReadSurface, NativeGraphicsSurface aDrawSurface, NativeGraphicsContext aContext);
    virtual ReturnCode setResourceContext(NativeGraphicsContext aContext);
    FT_Library ftLibrary() const { return m_ftLibrary; }
    FT_Face matchFontFaceBySpec(FontSpec& fontSpec, bool& isDefaultFace);
    FontDataMap& fontDataMap() { return m_fontDataMap; }
    FontIdMap& fontIdMap() { return m_fontIdMap; }
    void setDefaultFontFamilyName(const Utf16Char* familyName);
    const Utf16Char* defaultFontFamilyName () const { return m_defaultFontFamilyName; }
    FontFreeType* checkCharactersAndFallbackFont(const FT_Face& face, const FontSpec& spec, const Utf16Char* text, int length);
    GlyphDataSlot* allocateGlyphDataSlot();
    void freeGlyphDataSlot(GlyphDataSlot* slot);
    ReturnCode setFontFamilyAliasList(FontFamilyList* list);

private:
    FontDataIds matchFontDataIds(const FontSpec& fontSpec);
    FT_Face matchFontFace(FontSpec& fontSpec, const FontDataIds& ids, bool& isDefaultFace);
    void removeFontFamilyListByName(FontFamilyLists& lists, const UChar* family);
    void clearFontFamilyLists(FontFamilyLists& lists);
private:
    GlyphDataSlot* m_glyphDataSlotHead;
    int m_glyphDataSlotCount;
    FT_Library m_ftLibrary;
    GlyphType m_glyphType;
    FontLoaderDelegate* m_fontLoaderDelegate;
    FontLoader* m_fontLoader;
    FontDataMap m_fontDataMap;
    FontIdMap m_fontIdMap;
    FontFamilyLists m_fontFamilyFallbackLists;
    FontFamilyLists m_fontFamilyAliasLists;
    const Utf16Char* m_defaultFontFamilyName;
};

}

#endif
