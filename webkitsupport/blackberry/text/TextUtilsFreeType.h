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

#ifndef TextUtilsFreeType_H
#define TextUtilsFreeType_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include <list>
#include <map>
#include "OlympiaPlatformAssert.h"
#include <string.h>
#include "text_api.h"
#include <vector>
#include <wtf/unicode/UTF8.h>

namespace TextAPI {

using namespace WTF;
using namespace WTF::Unicode;

#define UNUSED_PARAM(param) (void)param

#ifdef NDEBUG
#ifdef _MSC_VER
#define DEBUG_PRINT(fmt, ...)
#else
#define DEBUG_PRINT(fmt, args...)
#endif
#define DEBUG_PRINT_UTF16_STRING(string)
#else // NDEBUG
#ifdef _MSC_VER
#define DEBUG_PRINT(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, args...) fprintf(stderr, fmt, ##args)
#endif

#define DEBUG_PRINT_UTF16_STRING(string) do { \
    const UChar* utf16Buffer_macro_ = string; \
    int utf16BufferLength_macro_ = Utf16StringLength(string);   \
    int utf8BufferLength_macro_ = utf16BufferLength_macro_ * 3 + 1; \
    char* utf8Buffer_macro_ = new char[utf8BufferLength_macro_]; \
    char* utf8String_macro_ = utf8Buffer_macro_; \
    convertUTF16ToUTF8(&utf16Buffer_macro_, \
        utf16Buffer_macro_ + utf16BufferLength_macro_, \
        &utf8Buffer_macro_, \
        utf8Buffer_macro_ + utf8BufferLength_macro_, \
        true); \
    utf8String_macro_[utf8Buffer_macro_ - utf8String_macro_] = 0; \
    DEBUG_PRINT("%s", utf8String_macro_); \
    delete[] utf8String_macro_; \
} while(0)
#endif

// Endiness detect macro
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define TEXTAPI_LITTLE_ENDIAN 1
#elif (__BYTE_ORDER == __BIG_ENDIAN)
#define TEXTAPI_BIG_ENDIAN 1
#else
#error unsupported endiness
#endif

inline static int Utf16StringLength(const Utf16Char* string)
{
    int n = 0;
    while (string && string[n])
        ++n;
    return n;
}

inline static Utf16Char* asciiToUtf16String(const char* asciiString)
{
    Utf16Char* utf16Buffer = 0;
    if (asciiString) {
        int length = strlen(asciiString);
        utf16Buffer = new Utf16Char[length + 1];
        int i = 0;
        while ((utf16Buffer[i] = asciiString[i]))
            i++;
    }
    return utf16Buffer;
}

inline static wchar_t* Utf16StringToWCharString(const Utf16Char* utfString)
{
    size_t utfStrLen = Utf16StringLength(utfString) + 1;
    wchar_t* wcsString = new wchar_t[utfStrLen];
#if OLYMPIA_WINDOWS
    memcpy(wcsString, (const wchar_t*)utfString, utfStrLen*2);
#else
    for (size_t i = 0; i < utfStrLen; i++) {
        wcsString[i] = (wchar_t)utfString[i];
    }
#endif

    return wcsString;
}

inline static wchar_t* localeStringToWCharString(const char* locEncString)
{
    size_t locEncStrLen = strlen(locEncString) + 1;
    wchar_t *wcsString = new wchar_t[locEncStrLen];

#if OLYMPIA_WINDOWS
    _locale_t loc = _create_locale(LC_ALL, "");
    _mbstowcs_l(wcsString, locEncString, locEncStrLen, loc);
    _free_locale(loc);
#else
    mbstowcs(wcsString, locEncString, locEncStrLen);
#endif
    return wcsString;
}

inline static Utf16Char* WCharStringToUtf16String(const wchar_t* wcsString)
{
    size_t wcharStrLen = wcslen(wcsString) + 1;
    Utf16Char *utf16Buf = new Utf16Char[wcharStrLen];
#if OLYMPIA_WINDOWS
    memcpy(utf16Buf, (const wchar_t*)wcsString, wcharStrLen*2);
#else
    for (size_t i = 0; i < wcharStrLen; i++) {
        utf16Buf[i] = (Utf16Char)wcsString[i];
    }
#endif

    return utf16Buf;
}

inline static Utf16Char* localeStringToUtf16String(const char* localeEncodedString)
{
    wchar_t* wcsBuf = localeStringToWCharString(localeEncodedString);
    Utf16Char *utf16Buf = WCharStringToUtf16String(wcsBuf);

    delete []wcsBuf;
    return utf16Buf;
}

inline static char* WCharStringToLocaleString(const wchar_t* wcsString)
{
    char* locEncBuf = new char[(wcslen(wcsString) + 1) * sizeof(wchar_t)];
#if OLYMPIA_WINDOWS
    _locale_t loc = _create_locale(LC_ALL, "");
    _wcstombs_l(locEncBuf, wcsString, (wcslen(wcsString) + 1) * sizeof(wchar_t), loc);
    _free_locale(loc);
#else
    wcstombs(locEncBuf, wcsString, (wcslen(wcsString) + 1) * sizeof(wchar_t));
#endif
    return locEncBuf;
}

inline static char* Utf16StringToLocaleString(const Utf16Char* utf16String)
{
    wchar_t* wcsBuf = Utf16StringToWCharString(utf16String);
    char* locEncBuf = WCharStringToLocaleString(wcsBuf);

    delete []wcsBuf;
    return locEncBuf;
}

inline static Utf16Char simpleToUpper(Utf16Char c)
{
    if (c >= 'a' && c <= 'z')
        c -= 'a' - 'A';
    return c;
}

inline static int Utf16StringCompareIgnoreCase(const Utf16Char* s1, const Utf16Char* s2)
{
    if (!s1 && s2)
        return -1;
    if (s1 && !s2)
        return 1;
    if (!s1 && !s2)
        return 0;
    while (simpleToUpper(*s1) == simpleToUpper(*s2) && *s1) {
        ++s1;
        ++s2;
    }
    return simpleToUpper(*s1) - simpleToUpper(*s2);
}

inline static int findUtf16Char(const Utf16Char* s, Utf16Char c)
{
    if (s) {
        int i = 0;
        while (s[i] && s[i] != c)
            ++i;
        if (s[i] || (!s[i] && !c))
            return i;
    }
    return -1;
}

struct Utf16StringLessThan {
    inline bool operator()(const Utf16Char* s1, const Utf16Char* s2) const
    {
        return Utf16StringCompareIgnoreCase(s1, s2) < 0;
    }
};

class FaceData {
private:
    FT_Face m_face;
    FT_Open_Args* m_args;

public:
    FaceData()
        : m_face(0)
        , m_args(0)
    {
    }

    FaceData(FT_Face face, FT_Open_Args* args)
        : m_face(face)
        , m_args(args)
    {
    }

    ~FaceData()
    {
        if (m_face)
            FT_Done_Face(m_face);
        if (m_args) {
            if (m_args->stream) {
                delete static_cast<Stream*>(m_args->stream->descriptor.pointer);
                delete m_args->stream;
            }
            delete[] m_args->pathname;
            delete m_args;
        }
    }

    FT_Face face()
    {
        return m_face;
    }
};

typedef std::vector<FontDataId> FontDataIds;
typedef std::map<const Utf16Char*, FontDataIds, Utf16StringLessThan> FontIdMap;
typedef std::vector<FaceData*> FontDataFreeType;
typedef std::map<FontDataId, FontDataFreeType> FontDataMap;
typedef std::list<const Utf16Char*> FontFamilyList;
typedef std::list<FontFamilyList*> FontFamilyLists;

}

#endif
