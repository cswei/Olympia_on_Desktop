/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "JavaScriptDebuggerBlackBerry.h"

#include "JavaScriptCallFrame.h"
#include "ScriptDebugServer.h"
#include "PlatformString.h"
#include "ScriptBreakpoint.h"
#include "SourceCode.h"
#include "WebPage.h"
#include "WebPage_p.h"
#include "WebPageClient.h"

namespace WebCore {

JavaScriptDebuggerBlackBerry::JavaScriptDebuggerBlackBerry(Olympia::WebKit::WebPage* page)
    : m_webPage(page)
    , m_pageClient(page ? page->client() : 0)
    , m_debugServer(ScriptDebugServer::shared())
{
    start();
}

JavaScriptDebuggerBlackBerry::~JavaScriptDebuggerBlackBerry()
{
    stop();
}

void JavaScriptDebuggerBlackBerry::start()
{
    m_debugServer.addListener(this, m_webPage->d->m_page);
}

void JavaScriptDebuggerBlackBerry::stop()
{
    m_debugServer.removeListener(this, m_webPage->d->m_page);
}

void JavaScriptDebuggerBlackBerry::addBreakpoint(const unsigned short* url, unsigned urlLength, unsigned lineNumber, const unsigned short* condition, unsigned conditionLength)
{
    if (!url || !urlLength)
        return;
    if (!m_currentCallFrame)
        return;

    String sourceString(url, urlLength);
    String conditionString(condition, conditionLength);
    m_debugServer.setBreakpoint(sourceString, lineNumber, ScriptBreakpoint(true, conditionString));
}

void JavaScriptDebuggerBlackBerry::updateBreakpoint(const unsigned short* url, unsigned urlLength, unsigned lineNumber, const unsigned short* condition, unsigned conditionLength)
{
    if (!url || !urlLength)
        return;
    if (!m_currentCallFrame)
        return;

    String sourceString(url, urlLength);
    String conditionString(condition, conditionLength);
    m_debugServer.setBreakpoint(sourceString, lineNumber, ScriptBreakpoint(true, conditionString));
}


void JavaScriptDebuggerBlackBerry::removeBreakpoint(const unsigned short* url, unsigned urlLength, unsigned lineNumber)
{
    if (!url || !urlLength)
        return;
    if (!m_currentCallFrame)
        return;

    String sourceString(url, urlLength);
    m_debugServer.removeBreakpoint(sourceString, lineNumber);
}


bool JavaScriptDebuggerBlackBerry::pauseOnExceptions()
{
    return m_debugServer.pauseOnExceptionsState() == ScriptDebugServer::PauseOnAllExceptions;
}

void JavaScriptDebuggerBlackBerry::setPauseOnExceptions(bool pause)
{
    m_debugServer.setPauseOnExceptionsState(pause ? ScriptDebugServer::PauseOnAllExceptions : ScriptDebugServer::DontPauseOnExceptions);
}

void JavaScriptDebuggerBlackBerry::pauseInDebugger()
{
    m_debugServer.pauseProgram();
}

void JavaScriptDebuggerBlackBerry::resumeDebugger()
{
    m_debugServer.continueProgram();
}

void JavaScriptDebuggerBlackBerry::stepOverStatementInDebugger()
{
    m_debugServer.stepOverStatement();
}

void JavaScriptDebuggerBlackBerry::stepIntoStatementInDebugger()
{
    m_debugServer.stepIntoStatement();
}

void JavaScriptDebuggerBlackBerry::stepOutOfFunctionInDebugger()
{
    m_debugServer.stepOutOfFunction();
}

void JavaScriptDebuggerBlackBerry::didParseSource(const String& sourceID, const String& url, const String& data, int firstLine)
{
    m_pageClient->javascriptSourceParsed(url.impl()->characters(), url.length(), data.impl()->characters(), data.length());
}

void JavaScriptDebuggerBlackBerry::failedToParseSource(const String& url, const String& data, int firstLine, int errorLine, const String& errorMessage)
{
    m_pageClient->javascriptParsingFailed(url.impl()->characters(), url.length(), errorMessage.impl()->characters(), errorMessage.length(), errorLine);
}

void JavaScriptDebuggerBlackBerry::didPause(ScriptState*)
{
    String stacks;

    m_currentCallFrame = m_debugServer.currentCallFrame();
    JavaScriptCallFrame* frame = m_currentCallFrame;

    while (frame && frame->isValid()) {
        JSC::SourceProvider* provider = reinterpret_cast<JSC::SourceProvider*>(frame->sourceID());
        String url(provider->url().data(), provider->url().size());
        if (url.length())
            stacks += url;
        stacks += ": ";

        if (frame->type() == JSC::DebuggerCallFrame::FunctionType) {
            String name = frame->functionName();
            if (name.length())
                stacks += name;
        }
        stacks += "(): ";

        String line = String::number(frame->line());
        stacks += line + "\n";

        frame = frame->caller();
    }

    m_pageClient->javascriptPaused(reinterpret_cast<const unsigned short*>(stacks.characters()), stacks.length());
}

void JavaScriptDebuggerBlackBerry::didContinue()
{
    m_currentCallFrame = 0;
    m_pageClient->javascriptContinued();
}

} // namespace WebCore
