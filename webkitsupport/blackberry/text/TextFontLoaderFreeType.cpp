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
#include "TextFontLoaderFreeType.h"

#include <ft2build.h>
#include FT_TRUETYPE_IDS_H
#include FT_SFNT_NAMES_H
#include "NotImplemented.h"
#include "OlympiaPlatformAssert.h"
#include <string.h>
#include "TextEngineFreeType.h"
#include "TextUtilsFreeType.h"

namespace TextAPI {

static Utf16Char* extractFontFamilyAndSetFamilyAliasListIfNeeded(FT_Face ftFace, EngineFreeType* engine)
{
    FT_SfntName sfntName;
    FontFamilyList* familyList = 0;
    int familyCount = 0;
    Utf16Char* firstFamily = 0;
    bool firstFamilyFound = false;
    int nameTableSize = FT_Get_Sfnt_Name_Count(ftFace);
    for (int nameTableIndex = 0; nameTableIndex < nameTableSize; ++nameTableIndex) {
        FT_Error error = FT_Get_Sfnt_Name(ftFace, nameTableIndex, &sfntName);
        if (error)
            continue;
        // FIXME: Need to support more platform_id and encoding_id?
        // Currently we only support platform_id is Windows
        // and encoding_id is UNICODE BMP. This is very common for many
        // ttf and ttc files.
        // Details: http://www.microsoft.com/typography/otspec/name.htm
        if (sfntName.platform_id == TT_PLATFORM_MICROSOFT
            && sfntName.encoding_id == TT_MS_ID_UNICODE_CS
            && sfntName.name_id == TT_NAME_ID_FONT_FAMILY) {
            int utf16StringLength = sfntName.string_len >> 1;
            if (!utf16StringLength)
                continue;
            if (!firstFamilyFound) {
                firstFamily = new Utf16Char[utf16StringLength + 1];
                firstFamily[utf16StringLength] = 0;
            }

            Utf16Char* family = new UChar[utf16StringLength + 1];
            family[utf16StringLength] = 0;

            for (size_t j = 0; j < sfntName.string_len; j += 2) {
#if defined(TEXTAPI_LITTLE_ENDIAN) && TEXTAPI_LITTLE_ENDIAN
                Utf16Char character = (static_cast<Utf16Char>(sfntName.string[j]) << 8) | sfntName.string[j + 1];
#else
                Utf16Char character = (static_cast<Utf16Char>(sfntName.string[j + 1]) << 8) | sfntName.string[j];
#endif
                int index = j >> 1;
                if (!firstFamilyFound)
                    firstFamily[index] = character;
                family[index] = character;
            }
            firstFamilyFound = true;
            ++familyCount;
            if (!familyList)
                familyList = new FontFamilyList();
            familyList->push_back(family);
        }
    }
    if (familyCount > 1)
        engine->setFontFamilyAliasList(familyList);
    else if (familyList) {
        for (FontFamilyList::const_iterator familyIterator = familyList->begin();
            familyIterator != familyList->end(); ++familyIterator)
            delete[] *familyIterator;
        delete familyList;
    }

    if (firstFamilyFound)
        return firstFamily;

    // FT_Face::family_name is ASCII.
    const char* asciiFamilyName = ftFace->family_name;
    if (asciiFamilyName)
        return asciiToUtf16String(asciiFamilyName);

    return asciiToUtf16String("@NULL");
}

static unsigned long readFTStreamForOlympiaStream(FT_Stream stream,
    unsigned long offset,
    unsigned char* buffer,
    unsigned long count)
{
    if (buffer && count) {
        Stream* s = static_cast<Stream*>(stream->descriptor.pointer);
        return s->m_read(*s, offset, buffer, count);
    }
    return 0;
}

static void closeFTStreamForOlympiaStream(FT_Stream stream)
{
    Stream* s = static_cast<Stream*>(stream->descriptor.pointer);
    s->m_close(*s);
}

static FT_Open_Args* prepareOpenArgs(const Stream* stream, const char* path, int pathLength)
{
    FT_Open_Args* openArgs = new FT_Open_Args;
    memset(openArgs, 0, sizeof(FT_Open_Args));
    if (stream) {
        Stream* s = new Stream;
        s->m_data = stream->m_data;
        s->m_read = stream->m_read;
        s->m_close = stream->m_close;
        s->m_getLength = stream->m_getLength;
        FT_Stream ftStream = new FT_StreamRec;
        memset(ftStream, 0, sizeof(FT_StreamRec));
        ftStream->size = s->m_getLength(*s);
        ftStream->pos = 0;
        ftStream->descriptor.pointer = s;
        ftStream->read = readFTStreamForOlympiaStream;
        ftStream->close = closeFTStreamForOlympiaStream;
        openArgs->flags = FT_OPEN_STREAM;
        openArgs->stream = ftStream;
    } else {
        char* p = new char[pathLength + 1];
        memcpy(p, path, pathLength);
        p[pathLength] = 0;
        openArgs->flags = FT_OPEN_PATHNAME;
        openArgs->pathname = p;
    }
    return openArgs;
}

static void cleanupOpenArgs(FT_Open_Args* openArgs)
{
    if (openArgs->stream) {
        delete static_cast<Stream*>(openArgs->stream->descriptor.pointer);
        delete openArgs->stream;
    }
    delete[] openArgs->pathname;
    delete openArgs;
}

FontLoaderFreeType::FontLoaderFreeType(EngineFreeType* engine)
    : m_engine(engine)
{
    OLYMPIA_ASSERT(m_engine);
}

ReturnCode FontLoaderFreeType::loadFontData(FontDataId& id,
    const Utf16Char* path,
    const Utf16Char* name,
    const AdvancedFontLoadingParam* advancedParam /*= 0*/)
{
    return loadFontData(id, path, 0, name, advancedParam);
}

ReturnCode FontLoaderFreeType::loadFontData(FontDataId& id,
    const Stream& stream,
    const Utf16Char* name,
    const AdvancedFontLoadingParam* advancedParam /*= 0*/)
{
    return loadFontData(id, 0, &stream, name, advancedParam);
}

ReturnCode FontLoaderFreeType::loadFontData(FontDataId& id,
    const Utf16Char* path,
    const Stream* stream,
    const Utf16Char* name,
    const AdvancedFontLoadingParam* advancedParam)
{
    UNUSED_PARAM(advancedParam);

    DEBUG_PRINT("FontLoaderFreeType::loadFontData(path=");
    DEBUG_PRINT_UTF16_STRING(path);
    DEBUG_PRINT(", stream=%p, name=", stream);
    DEBUG_PRINT_UTF16_STRING(name);
    DEBUG_PRINT(")\n");

    OLYMPIA_ASSERT(path || stream);
    if (!path && !stream)
        return -1;

    const char* locEncPath = 0;
    int locEncPathLen = 0;
    if (path && path[0]) {
        locEncPath = Utf16StringToLocaleString(path);
        if (!locEncPath)
            return -1;
        locEncPathLen = strlen(locEncPath);
    }

    // Open a FT_Face only contains num_faces.
    // FreeType supports face index with -1 to open this type of face.
    FT_Face provisionalFace = 0;
    FT_Open_Args* provisionalOpenArgs = prepareOpenArgs(stream, locEncPath, locEncPathLen);
    FT_Error error = FT_Open_Face(m_engine->ftLibrary(),
            provisionalOpenArgs,
            -1,
            &provisionalFace);
    if (error || !provisionalFace->num_faces) {
        if (provisionalFace)
            FT_Done_Face(provisionalFace);
        cleanupOpenArgs(provisionalOpenArgs);
        delete[] locEncPath;
        return -1;
    }

    // Fill font data
    bool isFamilyNominated = false;
    id = generateFontDataId();
    if (name && name[0]) {
        isFamilyNominated = true;
        Utf16Char* nominatedFamilyName = 0;
        int length = Utf16StringLength(name);
        nominatedFamilyName = new Utf16Char[length + 1];
        memcpy(nominatedFamilyName, name, length * sizeof(UChar));
        nominatedFamilyName[length] = 0;
        DEBUG_PRINT("\tFamily=");
        DEBUG_PRINT_UTF16_STRING(nominatedFamilyName);
        DEBUG_PRINT("\n");
        addFamilyIntoMap(id, nominatedFamilyName);
    }
    FontDataMap& fontDataMap = m_engine->fontDataMap();
    for (int i = 0; i < provisionalFace->num_faces; ++i) {
        FT_Face face = 0;
        FaceData* faceData = 0;
        FT_Open_Args* openArgs = prepareOpenArgs(stream, locEncPath, locEncPathLen);
        error = FT_Open_Face(m_engine->ftLibrary(), openArgs, i, &face);
        if (!error && face)
            faceData = new FaceData(face, openArgs);
        else {
            cleanupOpenArgs(openArgs);
            continue;
        }
        if (!isFamilyNominated) {
            Utf16Char* familyName = extractFontFamilyAndSetFamilyAliasListIfNeeded(face, m_engine);
            DEBUG_PRINT("\tFamily=");
            DEBUG_PRINT_UTF16_STRING(familyName);
            DEBUG_PRINT("\n");
            addFamilyIntoMap(id, familyName);
        }
        fontDataMap[id].push_back(faceData);
    }
    FT_Done_Face(provisionalFace);
    cleanupOpenArgs(provisionalOpenArgs);
    delete[] locEncPath;
    return 0;
}

ReturnCode FontLoaderFreeType::unloadFontData(FontDataId id)
{
    DEBUG_PRINT("Unloading font id=%d\n", id);
    FontIdMap& fontIdMap = m_engine->fontIdMap();
    FontDataMap& fontDataMap = m_engine->fontDataMap();
    FontDataMap::iterator fontDataIterator = fontDataMap.find(id);
    if (fontDataIterator != fontDataMap.end()) {
        for (FontDataFreeType::const_iterator cit = fontDataIterator->second.begin(); cit != fontDataIterator->second.end(); ++cit)
            delete *cit;
        fontDataMap.erase(fontDataIterator);
    }

    for (FontIdMap::iterator fontIdIterator = fontIdMap.begin(); fontIdIterator != fontIdMap.end(); ++fontIdIterator) {
        for (FontDataIds::iterator it = fontIdIterator->second.begin(); it != fontIdIterator->second.end(); ++it) {
            if (*it == id) {
                fontIdIterator->second.erase(it);
                break;
            }
        }
        if (fontIdIterator->second.empty()) {
            delete[] fontIdIterator->first;
            fontIdMap.erase(fontIdIterator);
            break;
        }
    }
    return 0;
}

void FontLoaderFreeType::setDefaultFontFamilyIfNeeded(const FontDataId& id, const Utf16Char* familyName)
{
    if (id == 1 && !m_engine->defaultFontFamilyName()) {
        m_engine->setDefaultFontFamilyName(familyName);
        DEBUG_PRINT("Default font family: ");
        DEBUG_PRINT_UTF16_STRING(familyName);
        DEBUG_PRINT("\n");
    }
}

void FontLoaderFreeType::addFamilyIntoMap(const FontDataId& id, const Utf16Char* familyName)
{
    FontIdMap& fontIdMap = m_engine->fontIdMap();
    FontIdMap::iterator familyIterator = fontIdMap.find(familyName);
    if (familyIterator != fontIdMap.end()) {
        familyIterator->second.push_back(id);
        delete[] familyName;
        setDefaultFontFamilyIfNeeded(id, familyIterator->first);
    } else {
        fontIdMap[familyName].push_back(id);
        setDefaultFontFamilyIfNeeded(id, familyName);
    }
}

FontDataId FontLoaderFreeType::s_globalId = 0;

}
