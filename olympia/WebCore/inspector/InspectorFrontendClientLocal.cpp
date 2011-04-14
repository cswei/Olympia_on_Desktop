/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "InspectorFrontendClientLocal.h"

#if ENABLE(INSPECTOR)

#include "Chrome.h"
#include "FloatRect.h"
#include "Frame.h"
#include "FrameView.h"
#include "InspectorController.h"
#include "InspectorFrontend.h"
#include "InspectorFrontendHost.h"
#include "Page.h"
#include "PlatformString.h"
#include "ScriptFunctionCall.h"
#include "ScriptObject.h"

namespace WebCore {

static const char* const inspectorAttachedHeightName = "inspectorAttachedHeight";
static const unsigned defaultAttachedHeight = 300;
static const float minimumAttachedHeight = 250.0f;
static const float maximumAttachedHeightRatio = 0.75f;

InspectorFrontendClientLocal::InspectorFrontendClientLocal(InspectorController* inspectorController, Page* frontendPage)
    : m_inspectorController(inspectorController)
    , m_frontendPage(frontendPage)
    , m_frontendScriptState(0)
{
}

InspectorFrontendClientLocal::~InspectorFrontendClientLocal()
{
    if (m_frontendHost)
        m_frontendHost->disconnectClient();
    m_frontendScriptState = 0;
    m_frontendPage = 0;
    m_inspectorController = 0;
}
    
void InspectorFrontendClientLocal::windowObjectCleared()
{
    // Grant the inspector the ability to script the inspected page.
    m_frontendPage->mainFrame()->document()->securityOrigin()->grantUniversalAccess();
    // FIXME: don't keep reference to the script state
    m_frontendScriptState = scriptStateFromPage(debuggerWorld(), m_frontendPage);
    ScriptGlobalObject::set(m_frontendScriptState, "InspectorBackend", m_inspectorController->inspectorBackend());
    m_frontendHost = InspectorFrontendHost::create(this, m_frontendPage);
    ScriptGlobalObject::set(m_frontendScriptState, "InspectorFrontendHost", m_frontendHost.get());
}

void InspectorFrontendClientLocal::frontendLoaded()
{
    bringToFront();
    // Create InspectorFrontend and set it to InspectorController.
    ASSERT(m_frontendScriptState);
    ScriptObject webInspectorObj;
    if (!ScriptGlobalObject::get(m_frontendScriptState, "WebInspector", webInspectorObj)) {
        ASSERT_NOT_REACHED();
        return;
    }
    m_inspectorController->setFrontend(new InspectorFrontend(webInspectorObj));
}

void InspectorFrontendClientLocal::requestAttachWindow()
{
    if (!canAttachWindow())
        return;
    attachWindow();
    setAttachedWindow(true);
}

void InspectorFrontendClientLocal::requestDetachWindow()
{
    detachWindow();
    setAttachedWindow(false);
}

bool InspectorFrontendClientLocal::canAttachWindow()
{
    unsigned inspectedPageHeight = m_inspectorController->inspectedPage()->mainFrame()->view()->visibleHeight();

    // Don't allow the attach if the window would be too small to accommodate the minimum inspector height.
    return minimumAttachedHeight <= inspectedPageHeight * maximumAttachedHeightRatio;
}

void InspectorFrontendClientLocal::changeAttachedWindowHeight(unsigned height)
{
    unsigned totalHeight = m_frontendPage->mainFrame()->view()->visibleHeight() + m_inspectorController->inspectedPage()->mainFrame()->view()->visibleHeight();
    unsigned attachedHeight = constrainedAttachedWindowHeight(height, totalHeight);
    m_inspectorController->setSetting(inspectorAttachedHeightName, String::number(attachedHeight));
    setAttachedWindowHeight(attachedHeight);
}

void InspectorFrontendClientLocal::moveWindowBy(float x, float y)
{
    FloatRect frameRect = m_frontendPage->chrome()->windowRect();
    frameRect.move(x, y);
    m_frontendPage->chrome()->setWindowRect(frameRect);
}

void InspectorFrontendClientLocal::setAttachedWindow(bool attached)
{
    ScriptObject webInspectorObj;
    if (!ScriptGlobalObject::get(m_frontendScriptState, "WebInspector", webInspectorObj)) {
        ASSERT_NOT_REACHED();
        return;
    }
    ScriptFunctionCall function(webInspectorObj, "dispatch"); 
    function.appendArgument("setAttachedWindow");
    function.appendArgument(attached);
    function.call();
}

void InspectorFrontendClientLocal::restoreAttachedWindowHeight()
{
    unsigned inspectedPageHeight = m_inspectorController->inspectedPage()->mainFrame()->view()->visibleHeight();
    String attachedHeight = m_inspectorController->setting(inspectorAttachedHeightName);
    bool success = true;
    int height = attachedHeight.toInt(&success);
    unsigned preferredHeight = success ? height : defaultAttachedHeight;
    
    // This call might not go through (if the window starts out detached), but if the window is initially created attached,
    // InspectorController::attachWindow is never called, so we need to make sure to set the attachedWindowHeight.
    // FIXME: Clean up code so we only have to call setAttachedWindowHeight in InspectorController::attachWindow
    setAttachedWindowHeight(constrainedAttachedWindowHeight(preferredHeight, inspectedPageHeight));
}

unsigned InspectorFrontendClientLocal::constrainedAttachedWindowHeight(unsigned preferredHeight, unsigned totalWindowHeight)
{
    using namespace std;
    return roundf(max(minimumAttachedHeight, min<float>(preferredHeight, totalWindowHeight * maximumAttachedHeightRatio)));
}
    
} // namespace WebCore

#endif
