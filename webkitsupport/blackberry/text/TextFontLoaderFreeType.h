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

#ifndef TextFontLoaderFreeType_H
#define TextFontLoaderFreeType_H

#include "text_api.h"

namespace TextAPI {

class EngineFreeType;
class FontLoaderFreeType: public FontLoader
{
public:
    FontLoaderFreeType(EngineFreeType* engine);
    virtual ReturnCode loadFontData(FontDataId& id, const Utf16Char* path, const Utf16Char* name, const AdvancedFontLoadingParam* advancedParam = 0);
    virtual ReturnCode loadFontData(FontDataId& id, const Stream& stream, const Utf16Char* name, const AdvancedFontLoadingParam* advancedParam = 0);
    ReturnCode unloadFontData(FontDataId id);

private:
    ReturnCode loadFontData(FontDataId& id,
        const Utf16Char* path,
        const Stream* stream,
        const Utf16Char* name,
        const AdvancedFontLoadingParam* advancedParam);

    void setDefaultFontFamilyIfNeeded(const FontDataId& id, const Utf16Char* familyName);
    void addFamilyIntoMap(const FontDataId& id, const Utf16Char* familyName);

private:
    FontDataId generateFontDataId() { return ++s_globalId; }
    EngineFreeType* m_engine;
    static FontDataId s_globalId;
};

}

#endif
