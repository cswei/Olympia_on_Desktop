/*
 * Copyright (c) 2011, Torch Mobile (Beijing) Co. Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms,
 * with or without modification, are permitted provided that
 * the following conditions are met:
 *
 *  -- Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *  -- Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *  -- Neither the name of the Torch Mobile (Beijing) Co. Ltd. nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "OlympiaPlatformTextCodec.h"
#include <string.h>
#include <ucnv.h>
#include <ucnv_cb.h>
#include <urename.h>
#include <utypes.h>

namespace Olympia {
namespace Platform {
namespace TextCodec {

unsigned getSupportedEncodings(Encoding* enc, unsigned n)
{
    unsigned numEncodings = ucnv_countAvailable();
    if (!enc) 
        return numEncodings;
       
    if (numEncodings > n) { 
        numEncodings = n; 
    }
    for (unsigned i = 0; i < numEncodings; i++) {
        enc[i].m_id = i;
        enc[i].m_name = ucnv_getAvailableName(i);
    }
    return numEncodings;
}

unsigned getAliases(unsigned id, const char** pList, unsigned n) 
{
    const char* alias = ucnv_getAvailableName(id) ;
    UErrorCode err = U_ZERO_ERROR;
    unsigned numAliases = ucnv_countAliases(alias, &err);

    // add alias gb2312 for codec "gb18030"
    if (!strcmp(alias, "gb18030"))
        numAliases++;
    if (!pList) 
        return numAliases;
    if (numAliases > n) {
        numAliases = n;
    }
    ucnv_getAliases(alias, pList, &err);

    if (!strcmp(alias, "gb18030")) 
        pList[numAliases-1] = "gb2312";
    
    return numAliases;
}

class ToUnicodeCallbackSetter {
public:
    ToUnicodeCallbackSetter(UConverter* converter)
        : m_converter(converter)
    {
        UErrorCode error = U_ZERO_ERROR;
        ucnv_setToUCallBack(m_converter, UCNV_TO_U_CALLBACK_SUBSTITUTE,
            UCNV_SUB_STOP_ON_ILLEGAL, &m_savedAction,
            &m_savedContext, &error);
    }

    ~ToUnicodeCallbackSetter()
    {
        const void* oldContext;
        UErrorCode error = U_ZERO_ERROR;
        UConverterToUCallback oldAction;
        ucnv_setToUCallBack(m_converter, m_savedAction,
            m_savedContext, &oldAction,
            &oldContext, &error);
    }
private:
    UConverter* m_converter;
    const void* m_savedContext;
    UConverterToUCallback m_savedAction;
};

Result decode(unsigned id, const char*& sourceSTart, const char* sourceEnd, unsigned short*& destinationStart, unsigned short* end)
{ 
    static UConverter* conv = 0;
    const char* alias = ucnv_getAvailableName(id) ;
    UErrorCode err = U_ZERO_ERROR;
    if (!conv)
        conv = ucnv_open(alias, &err);

    {
        ToUnicodeCallbackSetter setter(conv);
        ucnv_toUnicode(conv,
            &destinationStart, end, 
            &sourceSTart, sourceEnd,
            NULL, FALSE, &err);
    }

    if (err == U_BUFFER_OVERFLOW_ERROR)
        return destinationPending;
    else {
        ucnv_close(conv);
        conv = 0;
        if (U_SUCCESS(err)) 
            return successful;
        return sourceBroken;
    }
}

class FromUnicodeCallbackSetter {
public:
    FromUnicodeCallbackSetter(UConverter* converter)
        : m_converter(converter)
    {
        UErrorCode error = U_ZERO_ERROR;
        ucnv_setFromUCallBack(m_converter, UCNV_FROM_U_CALLBACK_SUBSTITUTE,
            UCNV_SUB_STOP_ON_ILLEGAL, &m_savedAction,
            &m_savedContext, &error);
    }

    ~FromUnicodeCallbackSetter()
    {
        const void* oldContext;
        UErrorCode error = U_ZERO_ERROR;
        UConverterFromUCallback oldAction;
        ucnv_setFromUCallBack(m_converter, m_savedAction,
            m_savedContext, &oldAction,
            &oldContext, &error);
    }
private:
    UConverter* m_converter;
    const void* m_savedContext;
    UConverterFromUCallback m_savedAction;
};

Result encode(unsigned int id, const unsigned short*& start, const unsigned short* end, char*& destinationStart, char* destinationEnd) 
{
    static UConverter* conv = 0;
    const char* alias = ucnv_getAvailableName(id) ;
    UErrorCode err = U_ZERO_ERROR;
    if (!conv) 
        conv = ucnv_open(alias, &err);

    {
        FromUnicodeCallbackSetter setter(conv);
        ucnv_fromUnicode(conv,
            &destinationStart, destinationEnd,
            &start, end,
            NULL, FALSE, &err);
    }

    if (err == U_BUFFER_OVERFLOW_ERROR)
        return destinationPending;
    else {
        ucnv_close(conv);
        conv = 0;
        if (U_SUCCESS(err))
            return successful;
        return sourceBroken;
    }

}

} // namespace TextCodec 
} // namespace Platform
} // namespace Olympia

