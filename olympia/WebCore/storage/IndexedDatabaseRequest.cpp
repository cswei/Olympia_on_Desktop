/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "IndexedDatabaseRequest.h"

#include "Document.h"
#include "ExceptionCode.h"
#include "Frame.h"
#include "IDBDatabase.h"
#include "IDBRequest.h"
#include "IndexedDatabase.h"

#if ENABLE(INDEXED_DATABASE)

namespace WebCore {

IndexedDatabaseRequest::IndexedDatabaseRequest(IndexedDatabase* indexedDatabase, Frame* frame)
    : m_indexedDatabase(indexedDatabase)
    , m_frame(frame)
{
    m_this = IDBAny::create();
    m_this->set(this);
}

IndexedDatabaseRequest::~IndexedDatabaseRequest()
{
}

PassRefPtr<IDBRequest> IndexedDatabaseRequest::open(const String& name, const String& description, ExceptionCode& exception)
{
    RefPtr<IDBRequest> request = IDBRequest::create(m_frame->document(), m_this);
    m_indexedDatabase->open(name, description, request, m_frame->document()->securityOrigin(), m_frame, exception);
    return request;
}

} // namespace WebCore

#endif // ENABLE(INDEXED_DATABASE)

