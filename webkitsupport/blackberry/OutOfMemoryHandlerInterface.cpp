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

#include "OutOfMemoryHandlerInterface.h"

#include <OlympiaPlatformAssert.h>

namespace Olympia {
namespace Platform {

static OutOfMemoryHandlerInterface* s_handler;

void setOOMHandler(OutOfMemoryHandlerInterface* handler)
{
    s_handler = handler;
}

// The functions below are never be called at present, so, add a OLYMPIA_CRASH() now.
// If they are really neccessary in the future, implement them and remove OLYMPIA_CRASH().
OOMHandledResult handleOOM(unsigned size)
{
    OLYMPIA_CRASH();
    if (s_handler)
        s_handler->handleOutOfMemory(size);
    return OOMHandledRetry;
}

void reportLowMemory()
{
    OLYMPIA_CRASH();
    // FIXME: Popup a dialog to notify user and ask whether keep reporting low memory.
}

void lowMemoryConfirmedByUser(bool keepReporting)
{
    OLYMPIA_CRASH();
}

void shrinkMemoryUsageIfScheduled()
{
    OLYMPIA_CRASH();
    if (s_handler)
        s_handler->shrinkMemoryUsage();
}

} // Platform
} // Olympia

