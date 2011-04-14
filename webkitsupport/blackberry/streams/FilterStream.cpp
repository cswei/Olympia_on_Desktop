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

#include "FilterStream.h"

namespace Olympia {

namespace Platform {

FilterStream::FilterStream()
    : m_wrappedStream(0)
{
}

FilterStream::~FilterStream()
{
    clearWrappedStream();
}

void FilterStream::setWrappedStream(IStream* stream)
{
    clearWrappedStream();
    m_wrappedStream = stream;
    if (m_wrappedStream)
        m_wrappedStream->setListener(this);
}

IStream* FilterStream::wrappedStream() const
{
    return m_wrappedStream;
}

void FilterStream::notifyOpen(int status, const char* message)
{
    if (m_listener)
        m_listener->notifyOpen(status, message);
}

void FilterStream::notifyWMLOverride()
{
    if (m_listener)
        m_listener->notifyWMLOverride();
}

void FilterStream::notifyHeaderReceived(const char* key, const char* value)
{
    if (m_listener)
        m_listener->notifyHeaderReceived(key, value);
}

void FilterStream::notifyDataReceived(const char* buf, size_t len)
{
    if (m_listener)
        m_listener->notifyDataReceived(buf, len);
}

void FilterStream::notifyDone()
{
    if (m_listener)
        m_listener->notifyDone();
}

void FilterStream::setRequest(const NetworkRequest& request)
{
    if (m_wrappedStream)
        m_wrappedStream->setRequest(request);
}

int FilterStream::open()
{
    if (m_wrappedStream)
        return m_wrappedStream->open();
    return -1;
}

int FilterStream::cancel()
{
    if (m_wrappedStream)
        return m_wrappedStream->cancel();
    return -1;
}

void FilterStream::clearWrappedStream()
{
    if (m_wrappedStream) {
        m_wrappedStream->setListener(0);
        delete m_wrappedStream;
        m_wrappedStream = 0;
    }
}

} // namespace Olympia

} // namespace Platform
