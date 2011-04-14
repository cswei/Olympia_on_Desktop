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
#ifndef IDBDatabaseRequest_h
#define IDBDatabaseRequest_h

#include "DOMStringList.h"
#include "IDBDatabase.h"
#include "PlatformString.h"
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>
#include <wtf/UnusedParam.h>

#if ENABLE(INDEXED_DATABASE)

namespace WebCore {

class IDBDatabaseRequest : public RefCounted<IDBDatabaseRequest> {
public:
    static PassRefPtr<IDBDatabaseRequest> create(PassRefPtr<IDBDatabase> idbDatabase)
    {
        return adoptRef(new IDBDatabaseRequest(idbDatabase));
    }
    ~IDBDatabaseRequest();

    // Implement the IDL
    String name() const { return m_idbDatabase->name(); }
    String description() const { return m_idbDatabase->description(); }
    String version() const { return m_idbDatabase->version(); }
    PassRefPtr<DOMStringList> objectStores() const { return m_idbDatabase->objectStores(); }

private:
    IDBDatabaseRequest(PassRefPtr<IDBDatabase>);

    RefPtr<IDBDatabase> m_idbDatabase;
};

} // namespace WebCore

#endif

#endif // IDBDatabaseRequest_h

