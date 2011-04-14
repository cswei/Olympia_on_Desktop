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

#include "config.h"
#include "TextEngineFreeType.h"

#include <algorithm>
#if defined(OLYMPIA_LINUX) || defined(OLYMPIA_MAC)
#include <dirent.h>
#endif
#include "NotImplemented.h"
#include "OlympiaPlatformAssert.h"
#if defined(OLYMPIA_LINUX) || defined(OLYMPIA_MAC)
#include <stdlib.h>
#include <string.h>
#endif
#include "TextFontFreeType.h"
#include "TextFontLoaderFreeType.h"
#include "TextGraphicsContextOpenVG.h"
#include "TextUtilsFreeType.h"
#include "VGUtils.h"
#if !defined(TEST_TEXT_API) || !TEST_TEXT_API
#include "WebSettings.h"
#endif

namespace TextAPI {

const static FontDataId defaultFontDataId = 1;
const static int glyphDataSlotCacheCapability = 8 * 1024; // 8K

static FontFreeType* s_defaultFont = 0;
const static char* s_defaultFamilyAlias = "OlympiaDefaultFontFamily";

#if defined(OLYMPIA_LINUX) || defined(OLYMPIA_MAC)
// FIXME: choose proper default font file name and default font dir name.
#if defined(OLYMPIA_LINUX)
static const char* s_defaultFontFileName = "ukai.ttf";
static const char* s_defaultFontDirName = "/usr/share/fonts/truetype";
#elif defined(OLYMPIA_MAC)
static const char* s_defaultFontFileName = "Hiragino Sans GB W3.otf";
static const char* s_defaultFontDirName = "/Library/Fonts";
#endif

void searchAndLoadFontFile(EngineFreeType* engine, const char* fontDir);

static void loadSystemFontFiles(EngineFreeType* engine)
{
    const char* dirName = getenv("OLYMPIA_DEFAULT_FONT_DIR");
    if (!dirName)
        dirName = s_defaultFontDirName;

    const char* defaultFontFileName = getenv("OLYMPIA_DEFAULT_FONT_FILE");
    if (!defaultFontFileName)
        defaultFontFileName = s_defaultFontFileName;

    int dirNameLength = strlen(dirName);
    if (!dirNameLength)
        return;

    char* fullPathName = 0;
    int length = dirNameLength;
    if (dirName[dirNameLength - 1] != '/')
        ++length;

    fullPathName = static_cast<char*>(malloc(length + 1));
    if (!fullPathName)
        return;
    memcpy(fullPathName, dirName, dirNameLength);
    dirNameLength = length;
    fullPathName[dirNameLength - 1] = '/';
    fullPathName[dirNameLength] = 0;

    // Load default font file.
    int fileNameLength = strlen(defaultFontFileName);
    if (!fileNameLength) {
        free(fullPathName);
        return;
    }
    char* tmp = static_cast<char*>(realloc(fullPathName, dirNameLength + fileNameLength + 1));
    if (!tmp) {
        free(fullPathName);
        return;
    }
    fullPathName = tmp;
    memcpy(fullPathName + dirNameLength, defaultFontFileName, fileNameLength + 1);
    Utf16Char* fontFileName = localeStringToUtf16String(fullPathName);
    FontDataId id = 0;
    engine->loadFontData(id, fontFileName, 0, 0);
    delete [] fontFileName;

    free(fullPathName);

    // Load font files recursively.
    searchAndLoadFontFile(engine, dirName);
}

void searchAndLoadFontFile(EngineFreeType* engine, const char* dirName)
{
    DEBUG_PRINT("Enumerate dir: %s\n", dirName);
    DIR* dir;
    int dirNameLength = strlen(dirName);
    if (!(dir = opendir(dirName)))
        return;
    struct dirent* entry;
    while ((entry = readdir(dir))) {
        if (entry->d_type != DT_REG)
            continue;

        int fileNameLength = strlen(entry->d_name);

        // .ttf and .ttc file
        if (fileNameLength <= 4
            || (entry->d_name[fileNameLength - 1] != 'f' && entry->d_name[fileNameLength - 1] != 'c')
            || entry->d_name[fileNameLength - 2] != 't'
            || entry->d_name[fileNameLength - 3] != 't'
            || entry->d_name[fileNameLength - 4] != '.')
            continue;

        char* fullPathName = static_cast<char*>(malloc(dirNameLength + fileNameLength + 2));
        memcpy(fullPathName, dirName, dirNameLength + 1);
        strcat(fullPathName, "/");
        strcat(fullPathName, entry->d_name);
        Utf16Char* fontFileName = localeStringToUtf16String(fullPathName);
        FontDataId id = 0;
        engine->loadFontData(id, fontFileName, 0, 0);
        delete[] fontFileName;
        free(fullPathName);
    }
    closedir(dir);

    // reopen the dir to search subdir.
    dir = opendir(dirName);
    while ((entry = readdir(dir))) {
        if (entry->d_type != DT_DIR
        || strcmp(entry->d_name, ".") == 0
        || strcmp(entry->d_name, "..") == 0)
            continue;

        char* subDirPath = static_cast<char*>(malloc(dirNameLength + entry->d_reclen +2));
        memcpy(subDirPath, dirName, dirNameLength + 1);
        strcat(subDirPath, "/");
        strcat(subDirPath, entry->d_name);
        searchAndLoadFontFile(engine, subDirPath);
        free(subDirPath);
    }
    closedir(dir);
}
#elif OLYMPIA_WINDOWS
static const wchar_t* s_defaultFontFileName = L"msyh.ttf";
static const wchar_t* s_defaultFontDirName = L"C:\\Windows\\Fonts";

void searchAndLoadFontFile(EngineFreeType* engine, const wchar_t* fontDir, const wchar_t* filenameWildcard)
{
    wchar_t* fontNameWildcard = 0;
    wchar_t* fontFullPath = 0;
    FontDataId id = 0;
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;

    fontNameWildcard = new wchar_t[wcslen(fontDir) + wcslen(filenameWildcard) +2];
    swprintf(fontNameWildcard, L"%s\\%s", fontDir, filenameWildcard);

    hFind = FindFirstFile(WCharStringToLocaleString(fontNameWildcard), &FindFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        delete []fontNameWildcard;
        return;
    }

    wchar_t* wcsBuf = localeStringToWCharString(FindFileData.cFileName);
    fontFullPath = new wchar_t[wcslen(fontDir) + wcslen(wcsBuf) +2];
    swprintf(fontFullPath, L"%s\\%s", fontDir, wcsBuf);
    Utf16Char* u16FontPath = WCharStringToUtf16String(fontFullPath);
    engine->loadFontData(id, u16FontPath, 0, 0);
    delete []wcsBuf;
    delete []u16FontPath;
    delete []fontFullPath;

    while (FindNextFile(hFind, &FindFileData)) {
        wcsBuf = localeStringToWCharString(FindFileData.cFileName);
        fontFullPath = new wchar_t[wcslen(fontDir) + wcslen(wcsBuf) +2];
        swprintf(fontFullPath, L"%s\\%s", fontDir, wcsBuf);
        u16FontPath = WCharStringToUtf16String(fontFullPath);
        engine->loadFontData(id, u16FontPath, 0, 0);
        delete []wcsBuf;
        delete []u16FontPath;
        delete []fontFullPath;
    }

    delete []fontNameWildcard;
    FindClose(hFind);
}

void loadSystemFontFiles(EngineFreeType* engine)
{
    FontDataId id = 0;
    
    // generate font directory name and default font name.
    const wchar_t* fontDirName = _wgetenv(L"OLYMPIA_DEFAULT_FONT_DIR");
    if (!fontDirName)
        fontDirName = s_defaultFontDirName;

    const wchar_t* defaultFontFileName = _wgetenv(L"OLYMPIA_DEFAULT_FONT_FILE");
    if (!defaultFontFileName)
        defaultFontFileName = s_defaultFontFileName;

    // load default font first, as the default font should be loaded firstly in the font engine.
    wchar_t* defaultFontFullPath = new wchar_t[wcslen(fontDirName) + wcslen(defaultFontFileName) +2];
    swprintf(defaultFontFullPath, L"%s\\%s", fontDirName, defaultFontFileName);
    Utf16Char* u16FontPath = WCharStringToUtf16String(defaultFontFullPath);
    engine->loadFontData(id, u16FontPath, 0, 0);
    delete []u16FontPath;
    delete []defaultFontFullPath;

    // search the default font directory and load them all.
    searchAndLoadFontFile(engine, fontDirName, L"\*\.ttf");
    searchAndLoadFontFile(engine, fontDirName, L"\*\.ttc");
}

#else
void loadSystemFontFiles(EngineFreeType* engine)
{
    DEBUG_PRINT("Implement me: non-unix loadSystemFontFiles()\n");
}
#endif

EngineFreeType::EngineFreeType()
    : m_glyphDataSlotHead(0)
    , m_glyphDataSlotCount(0)
    , m_ftLibrary(0)
    , m_glyphType(OpenVGGlyphType)
    , m_fontLoaderDelegate(0)
    , m_fontLoader(0)
    , m_defaultFontFamilyName(0)
{
    FT_Init_FreeType(&m_ftLibrary);
    OLYMPIA_ASSERT(m_ftLibrary);
    m_fontLoader = new FontLoaderFreeType(this);

    loadSystemFontFiles(this);
}

EngineFreeType::~EngineFreeType()
{
    if (s_defaultFont) {
        delete s_defaultFont;
        s_defaultFont = 0;
    }

    if (m_fontLoader)
        delete m_fontLoader;

    for (FontDataMap::const_iterator fontDataIterator = m_fontDataMap.begin(); fontDataIterator != m_fontDataMap.end(); ++fontDataIterator) {
        for (FontDataFreeType::const_iterator fontFaceIterator = fontDataIterator->second.begin(); fontFaceIterator != fontDataIterator->second.end(); ++fontFaceIterator)
            delete *fontFaceIterator;
    }

    for (FontIdMap::const_iterator fontIdIterator = m_fontIdMap.begin(); fontIdIterator != m_fontIdMap.end(); ++fontIdIterator)
        delete[] fontIdIterator->first;

    while (m_glyphDataSlotHead) {
        vgDestroyPath(m_glyphDataSlotHead->m_glyphPath);
        GlyphDataSlot* next = m_glyphDataSlotHead->m_next;
        delete m_glyphDataSlotHead;
        m_glyphDataSlotHead = next;
    }

    clearFontFamilyLists(m_fontFamilyFallbackLists);
    clearFontFamilyLists(m_fontFamilyAliasLists);
    FT_Done_FreeType(m_ftLibrary);
}

ReturnCode EngineFreeType::loadFontData(FontDataId& id, const Utf16Char* path, const Utf16Char* name, const AdvancedFontLoadingParam* advancedParam /*= 0*/)
{
    if (m_fontLoader)
        return m_fontLoader->loadFontData(id, path, name, advancedParam);
    return -1;
}

ReturnCode EngineFreeType::loadFontData(FontDataId& id, const Stream& stream, const Utf16Char* name, const AdvancedFontLoadingParam* advancedParam /*= 0*/)
{
    if (m_fontLoader)
        return m_fontLoader->loadFontData(id, stream, name, advancedParam);
    return -1;
}

ReturnCode EngineFreeType::unloadFontData(FontDataId id)
{
    if (m_fontLoader)
        return static_cast<FontLoaderFreeType*>(m_fontLoader)->unloadFontData(id);
    return -1;
}

Font* EngineFreeType::createFont(ReturnCode& returnCode, const FontSpec& fontSpec)
{
    Font* font = new FontFreeType(this);
    font->setFontSpec(fontSpec);
    returnCode = 0;
    return font;
}

void EngineFreeType::setFontLoaderDelegate(FontLoaderDelegate* fontLoaderDelegate)
{
    m_fontLoaderDelegate = fontLoaderDelegate;
}

int EngineFreeType::clearGlyphCache()
{
    notImplemented();
    return 0;
}

ReturnCode EngineFreeType::setFontFamilyList(const Utf16Char* fontFamilyList)
{
    if (!fontFamilyList)
        return -1;

    int separatorPosition = findUtf16Char(fontFamilyList, ',');
    if (separatorPosition == 0)
        return -1;

    bool onlyOneFamily = false;
    if (separatorPosition < 0) {
        onlyOneFamily = true;
        separatorPosition = Utf16StringLength(fontFamilyList);
    }

    // To reduace a string copy modify and restore original buffer.
    if (!onlyOneFamily)
        *const_cast<Utf16Char*>(fontFamilyList + separatorPosition) = 0;
    removeFontFamilyListByName(m_fontFamilyFallbackLists, fontFamilyList);
    if (!onlyOneFamily)
        *const_cast<Utf16Char*>(fontFamilyList + separatorPosition) = ',';
    else
        return 0;

    int startPosition = 0;
    FontFamilyList* list = new FontFamilyList();
    do {
        int familyLength = separatorPosition - startPosition;
        if (familyLength) {
            Utf16Char* family = new Utf16Char[familyLength + 1];
            // We don't support {w=width, h=height} modifier.
            memcpy(family, fontFamilyList + startPosition, familyLength * sizeof(UChar));
            family[familyLength] = 0;
            list->push_back(family);
            DEBUG_PRINT("Pushing family [");
            DEBUG_PRINT_UTF16_STRING(family);
            DEBUG_PRINT("] to list.\n");
        }
        startPosition = separatorPosition + 1;
        int nextSeparatorRelativePosition = findUtf16Char(fontFamilyList + startPosition, ',');
        if (nextSeparatorRelativePosition == -1)
            nextSeparatorRelativePosition = Utf16StringLength(fontFamilyList + startPosition);
        separatorPosition = startPosition + nextSeparatorRelativePosition;
    } while (fontFamilyList[separatorPosition]);

    m_fontFamilyFallbackLists.push_back(list);
    return 0;
}

ReturnCode EngineFreeType::setFontFamilyAliasList(FontFamilyList* list)
{
    if (!list || list->empty())
        return -1;

    // We don't override an existing list in FontFamilyAliasLists.
    // If there is an existing list indentified by the first family
    // in the lists we just append the new elements into the existing
    // list.
    for (FontFamilyLists::const_iterator listsIterator = m_fontFamilyAliasLists.begin(); listsIterator != m_fontFamilyAliasLists.end(); ++listsIterator) {
        FontFamilyList* existList = *listsIterator;
        OLYMPIA_ASSERT(existList && !existList->empty());
        if (!Utf16StringCompareIgnoreCase(existList->front(), list->front())) {
            FontFamilyList::const_iterator familyIterator = list->begin();
            for (++familyIterator; familyIterator != list->end(); ++familyIterator)
                existList->push_back(*familyIterator);
            delete[] list->front();
            delete list;
            return 0;
        }
    }

    m_fontFamilyAliasLists.push_back(list);
    return 0;
}

void EngineFreeType::removeFontFamilyListByName(FontFamilyLists& lists, const UChar* family)
{
    for (FontFamilyLists::const_iterator listsIterator = lists.begin();
        listsIterator != lists.end(); ++listsIterator) {
        FontFamilyList* list = *listsIterator;
        OLYMPIA_ASSERT(list);
        OLYMPIA_ASSERT(!list->empty());
        if (!Utf16StringCompareIgnoreCase(list->front(), family)) {
            for (FontFamilyList::const_iterator familyIterator = list->begin();
                familyIterator != list->end(); ++familyIterator)
                delete[] *familyIterator;
            delete list;
            break;
        }
    }
}

void EngineFreeType::clearFontFamilyLists(FontFamilyLists& lists)
{
    for (FontFamilyLists::const_iterator listIterator = lists.begin();
        listIterator != lists.end(); ++listIterator) {
        FontFamilyList* list = *listIterator;
        for (FontFamilyList::const_iterator familyIterator = list->begin();
            familyIterator != list->end(); ++familyIterator)
            delete[] *familyIterator;
        delete list;
    }
    lists.clear();
}

GraphicsContext* EngineFreeType::createGraphicsContext(ReturnCode& returnCode, GraphicsContextType type, NativeGraphicsDisplay display)
{
    if (type == OpenVGGraphicsContext) {
        GraphicsContext* gc = new GraphicsContextOpenVG();
        OLYMPIA_ASSERT(gc);
        gc->setDisplay(display);
        returnCode = 0;
        return gc;
    } else if (type == BitmapGraphicsContext) {
        returnCode = -1;
        return 0;
    }
    OLYMPIA_ASSERT(false);
    returnCode = -1;
    return 0;
}

GraphicsContext* EngineFreeType::createGraphicsContext(ReturnCode& returnCode, GraphicsContextType type, NativeGraphicsDisplay display, NativeGraphicsContext context, NativeGraphicsSurface surface, bool ownGraphicsState)
{
    UNUSED_PARAM(ownGraphicsState);
    if (type == OpenVGGraphicsContext) {
        GraphicsContext* gc = new GraphicsContextOpenVG();
        OLYMPIA_ASSERT(gc);
        gc->setDisplay(display);
        gc->setSurface(surface);
        gc->setContext(context);
        returnCode = 0;
        return gc;
    } else if (type == BitmapGraphicsContext) {
        returnCode = -1;
        return 0;
    }
    returnCode = -1;
    return 0;
}

ReturnCode EngineFreeType::drawText(GraphicsContext* gc, Font& font, const Utf16Char* text, int textLength, double x, double y, double wrap, const DrawParam* param, TextMetrics* metrics)
{
    return static_cast<FontFreeType&>(font).drawText(gc, text, textLength, x, y, wrap, param, metrics);
}

ReturnCode EngineFreeType::xToTextPos(double x, int& textPos, TextPosRounding rounding, Font& font, const Utf16Char* text, int textLength, const DrawParam& param)
{
    // FIXME: support TextRounding.
    UNUSED_PARAM(rounding);
    TextMetrics metrics;
    int code = static_cast<FontFreeType&>(font).drawText(0, text, textLength, 0, 0, x, &param, &metrics);
    if (!code)
        textPos = metrics.m_consumed;
    return code;
}

ReturnCode EngineFreeType::textPosToX(int textPos, double &x, Font& font, const Utf16Char* text, int textLength, const DrawParam& param)
{
    // Range of textPos is [0, textLength]. If textPos == textLength, it's
    // calculating the cursor position which is the end of the text.
    OLYMPIA_ASSERT(textPos >= 0 && textPos <= textLength);
    if (textPos < 0 || textPos > textLength)
        return -1;

    // FontFreeType::drawText doesn't support drawing text with 0 length.
    if (textPos == 0) {
        x = 0;
        return 0;
    }

    TextMetrics metrics;
    int code = static_cast<FontFreeType&>(font).drawText(0, text, textPos, 0, 0, 0, &param, &metrics);
    if (!code)
        x = metrics.m_newX;
    return code;
}

GlyphType EngineFreeType::setGlyphType(GlyphType type)
{
    GlyphType previousGlyphType = m_glyphType;
    m_glyphType = type;
    return previousGlyphType;
}

ReturnCode EngineFreeType::setResourceContext(NativeGraphicsDisplay aDisplay, NativeGraphicsSurface aReadSurface, NativeGraphicsSurface aDrawSurface, NativeGraphicsContext aContext)
{
    UNUSED_PARAM(aDisplay);
    UNUSED_PARAM(aReadSurface);
    UNUSED_PARAM(aDrawSurface);
    UNUSED_PARAM(aContext);
    notImplemented();
    return -1;
}

ReturnCode EngineFreeType::setResourceContext(NativeGraphicsContext aContext)
{
    UNUSED_PARAM(aContext);
    notImplemented();
    return -1;
}

FontDataIds EngineFreeType::matchFontDataIds(const FontSpec& fontSpec)
{
    // So far we use strictly matching.
    FontIdMap::const_iterator fontIdIterator =  m_fontIdMap.find(fontSpec.m_name);

    if (fontIdIterator != m_fontIdMap.end())
        return fontIdIterator->second;

    // Use alias family.
    const Utf16Char* alias = 0;
    for (FontFamilyLists::const_iterator listsIterator = m_fontFamilyAliasLists.begin(); listsIterator != m_fontFamilyAliasLists.end(); ++listsIterator) {
        FontFamilyList::const_iterator listIterator = (*listsIterator)->begin();
        OLYMPIA_ASSERT(*listIterator);
        ++listIterator;
        for (; listIterator != (*listsIterator)->end(); ++listIterator) {
            DEBUG_PRINT("Comparing [");
            DEBUG_PRINT_UTF16_STRING(fontSpec.m_name);
            DEBUG_PRINT("] and [");
            DEBUG_PRINT_UTF16_STRING(*listIterator);
            DEBUG_PRINT("]\n");
            if (Utf16StringCompareIgnoreCase(fontSpec.m_name, *listIterator) == 0) {
                alias = (*listsIterator)->front();
                break;
            }
        }
        if (alias)
            break;
    }
    if (alias) {
        fontIdIterator = m_fontIdMap.find(alias);
        OLYMPIA_ASSERT(fontIdIterator != m_fontIdMap.end());
        if (fontIdIterator != m_fontIdMap.end())
            return fontIdIterator->second;
    }
    return FontDataIds();
}

FT_Face EngineFreeType::matchFontFace(FontSpec& fontSpec, const FontDataIds& ids, bool& isDefaultFace)
{
    FT_Face boldMatchedFace = 0;
    FT_Face italicMatchedFace = 0;
    // Details: http://www.w3.org/TR/CSS2/fonts.html#font-boldness.
    bool bold = fontSpec.m_weight > 400;
    bool italic = fontSpec.m_style == ItalicStyle;
    isDefaultFace = false;
    for (size_t i = 0; i < ids.size(); ++i) {
        FontDataMap::const_iterator fontDataIterator = m_fontDataMap.find(ids[i]);
        OLYMPIA_ASSERT(fontDataIterator != m_fontDataMap.end());
        for (FontDataFreeType::const_iterator faceIterator = fontDataIterator->second.begin(); faceIterator != fontDataIterator->second.end(); ++faceIterator) {
            FT_Long styleFlags = (*faceIterator)->face()->style_flags;
            if (bold == !!(styleFlags & FT_STYLE_FLAG_BOLD)) {
                isDefaultFace = ids[i] == defaultFontDataId;
                if (!boldMatchedFace)
                    boldMatchedFace = (*faceIterator)->face();
                // Bold and italic all matched
                if (italic == !!(styleFlags & FT_STYLE_FLAG_ITALIC))
                    return (*faceIterator)->face();
            }

            if (italic == !!(styleFlags & FT_STYLE_FLAG_ITALIC)) {
                isDefaultFace = ids[i] == defaultFontDataId;
                if (!italicMatchedFace)
                    italicMatchedFace = (*faceIterator)->face();
                // Bold and italic all matched
                if (bold == !!(styleFlags & FT_STYLE_FLAG_BOLD))
                    return (*faceIterator)->face();
            }
        }
    }

    // Bold has higher priority.
    if (boldMatchedFace) {
        if (fontSpec.m_style == ItalicStyle)
            fontSpec.m_style = PlainStyle;
        return boldMatchedFace;
    }

    if (italicMatchedFace) {
        if (fontSpec.m_weight > 400)
            fontSpec.m_weight = 400;
        return italicMatchedFace;
    }

    return 0;
}

FT_Face EngineFreeType::matchFontFaceBySpec(FontSpec& fontSpec, bool& isDefaultFace)
{
    FontDataIds ids = matchFontDataIds(fontSpec);
    FT_Face face = matchFontFace(fontSpec, ids, isDefaultFace);

    if (!face) {
        isDefaultFace = true;
        FontDataMap::const_iterator fontDataIterator = m_fontDataMap.find(defaultFontDataId);
        OLYMPIA_ASSERT(fontDataIterator != m_fontDataMap.end());
        face = fontDataIterator->second[0]->face();
        // The family name doesn't match, so return default family name.
        if (m_defaultFontFamilyName) {
            int length = Utf16StringLength(m_defaultFontFamilyName);
            length = std::min(length, MAX_FONT_NAME);
            memcpy(fontSpec.m_name, m_defaultFontFamilyName, length * sizeof(Utf16Char));
            fontSpec.m_name[length] = 0;
        }
    }

    return face;
}

void EngineFreeType::setDefaultFontFamilyName(const Utf16Char* familyName)
{
    OLYMPIA_ASSERT(!m_defaultFontFamilyName);
    m_defaultFontFamilyName = familyName;
#if !defined(TEST_TEXT_API) || !TEST_TEXT_API
    int length = Utf16StringLength(familyName);
    Utf16Char* name = new Utf16Char[length + 1];
    memcpy(name, familyName, length * sizeof(Utf16Char));
    name[length] = 0;
    FontFamilyList* list = new FontFamilyList();
    list->push_back(name);
    Utf16Char* alias = localeStringToUtf16String(s_defaultFamilyAlias);
    list->push_back(alias);
    setFontFamilyAliasList(list);
    // Olympia::WebKit::WebSettings support ASCII standard font family only.
    Olympia::WebKit::WebSettings::globalSettings()->setStandardFontFamily(s_defaultFamilyAlias);
#endif
}

FontFreeType* EngineFreeType::checkCharactersAndFallbackFont(const FT_Face& face, const FontSpec& spec, const Utf16Char* text, int length)
{
    OLYMPIA_ASSERT(face && text && length);
    for (int i = 0; i < length; ++i) {
        FT_UInt glyphIndex = FT_Get_Char_Index(face, text[i]);

        // FIXME: Currently we fallback to default font face
        // directly. Add a smarter logic to match other closer
        // face.
        if (!glyphIndex) {
            if (!s_defaultFont) {
                s_defaultFont = new FontFreeType(this);
                FontSpec defaultSpec;
                defaultSpec.m_height = spec.m_height;
                int i = 0;
                while (s_defaultFamilyAlias[i] && i < MAX_FONT_NAME - 1) {
                    defaultSpec.m_name[i] = s_defaultFamilyAlias[i];
                    ++i;
                }
                defaultSpec.m_name[i] = 0;
                s_defaultFont->setFontSpec(defaultSpec);
            } else
                s_defaultFont->setHeight(spec.m_height);
            return s_defaultFont;
        }
    }
    return 0;
}

// If allocated slot's m_next is not NULL the slot is
// managed by a linked list; otherwise it's out of the list.
// When this type of slot is called with freeGlyphDataSlot
// it will be destroyed directly insteand of being returned
// to the list.
GlyphDataSlot* EngineFreeType::allocateGlyphDataSlot()
{
    if (m_glyphDataSlotHead) {
        GlyphDataSlot* slot = m_glyphDataSlotHead;
        m_glyphDataSlotHead = slot->m_next;
        slot->m_next = reinterpret_cast<GlyphDataSlot*>(1);
        vgClearPath(slot->m_glyphPath, VG_PATH_CAPABILITY_APPEND_TO);
        return slot;
    }

    VGPath path = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
        1.0f, 0.0f,
        10, // segmentCapacityHint
        20, // coordCapacityHint
        VG_PATH_CAPABILITY_APPEND_TO);
    ASSERT_VG_NO_ERROR();
    GlyphDataSlot* slot = new GlyphDataSlot();
    slot->m_glyphPath = path;

    if (m_glyphDataSlotCount < glyphDataSlotCacheCapability) {
        m_glyphDataSlotCount++;
        slot->m_next = reinterpret_cast<GlyphDataSlot*>(1);
    } else
        slot->m_next = 0;
    return slot;
}

void EngineFreeType::freeGlyphDataSlot(GlyphDataSlot* slot)
{
    if (slot) {
        if (slot->m_next) {
            slot->m_next = m_glyphDataSlotHead;
            m_glyphDataSlotHead = slot;
        } else {
            vgDestroyPath(slot->m_glyphPath);
            delete slot;
        }
    }
}

Engine* Engine::create(ReturnCode& returnCode, char* heap, int heapSize)
{
    UNUSED_PARAM(heap);
    UNUSED_PARAM(heapSize);
    EngineFreeType* engine = new EngineFreeType();
    if (engine)
        returnCode = 0;
    else
        returnCode = -1;
    return engine;
}

void Engine::destroy(Engine* engine)
{
    // FIXME: Why Engine interface has non-virtual destructor ?
    delete static_cast<EngineFreeType*>(engine);
}

}
