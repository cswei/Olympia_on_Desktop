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

#include "NetworkRequest.h"

#include <string.h>
#include "NotImplemented.h"
#include <QString>


namespace Olympia {

namespace Platform {

class NetworkRequestPrivate
{
public:
    NetworkRequestPrivate()
        : m_isInit(false)
        , m_cachePolicy(NetworkRequest::UseProtocolCachePolicy)
        , m_targetType(NetworkRequest::TargetIsUnknown)
        , m_timeout(NetworkRequest::MAX_TIMEOUT)
    {
    }

    bool m_isInit;
    std::string m_url;
    std::string m_method;
    NetworkRequest::CachePolicy m_cachePolicy;
    NetworkRequest::TargetType m_targetType;
    double m_timeout;
    NetworkRequest::FormDataList m_formData;
    NetworkRequest::HeaderList m_headerList;
};


NetworkRequest::NetworkRequest()
    : d(new NetworkRequestPrivate())
{
}

NetworkRequest::~NetworkRequest()
{
    delete d;
}

void NetworkRequest::setRequestUrl(const char* url, const char* method, CachePolicy cachePolicy, TargetType targetType, double timeout)
{
    d->m_isInit = false;
    d->m_url = url;
    d->m_method = method;
    d->m_cachePolicy = cachePolicy;
    d->m_targetType = targetType;
    d->m_timeout = timeout;
}

void NetworkRequest::setRequestInitial(double timeout)
{
    d->m_isInit = true;
    d->m_url.clear();
    d->m_method.clear();
    d->m_cachePolicy = UseProtocolCachePolicy;
    d->m_targetType = TargetIsMainFrame;
    d->m_timeout = timeout;
}

void NetworkRequest::setData(const char* buf, size_t len)
{
    if(!buf || len == 0)
        return;
    d->m_formData.clear();
    addMultipartData(buf, len);
}

void NetworkRequest::addMultipartData(const char* buf, size_t len)
{
    if (!buf || !len)
        return;
    FormDataChunk formData;
    formData.buf.insert(formData.buf.end(), buf, buf + len);
    formData.isData = true;
    d->m_formData.push_back(formData);
}

void NetworkRequest::addMultipartFilename(const unsigned short* filename, size_t len)
{
    if (!filename || !len)
        return;
    FormDataChunk formData;
#if OLYMPIA_WINDOWS
	formData.filename = std::wstring((wchar_t*)filename,len);
#else
    formData.filename = QString::fromUtf16(filename,len).toStdWString();
#endif
    formData.isData = false;
    d->m_formData.push_back(formData);
}

void NetworkRequest::addHeader(const char* key, const char* value)
{
    d->m_headerList.push_back(std::make_pair(std::string(key), std::string(value)));
}

bool NetworkRequest::isInitial() const
{
    return d->m_isInit;
}

std::string& NetworkRequest::getUrlRef() const
{
    return d->m_url;
}

std::string& NetworkRequest::getMethodRef() const
{
    return d->m_method;
}

NetworkRequest::CachePolicy NetworkRequest::getCachePolicy() const
{
    return d->m_cachePolicy;
}

NetworkRequest::TargetType NetworkRequest::getTargetType() const
{
    return d->m_targetType;
}

double NetworkRequest::getTimeout() const
{
    return d->m_timeout;
}

NetworkRequest::FormDataList& NetworkRequest::getFormDataListRef() const
{
    return d->m_formData;
}

NetworkRequest::HeaderList& NetworkRequest::getHeaderListRef() const
{
    return d->m_headerList;
}

} // namespace Olympia

} // namespace Platform
