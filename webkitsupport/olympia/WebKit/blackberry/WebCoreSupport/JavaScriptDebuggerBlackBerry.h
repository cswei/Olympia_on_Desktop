/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#ifndef JavaScriptDebuggerBlackBerry_h
#define JavaScriptDebuggerBlackBerry_h

#include "ScriptDebugListener.h"

namespace Olympia {
    namespace WebKit {
        class WebPage;
        class WebPageClient;
    }
}

namespace JSC {
    class ExecState;
    class SourceCode;
    class UString;
}

namespace WebCore {

class JavaScriptCallFrame;
class ScriptDebugServer;

class JavaScriptDebuggerBlackBerry : public ScriptDebugListener {
public:
    JavaScriptDebuggerBlackBerry(Olympia::WebKit::WebPage* page);
    ~JavaScriptDebuggerBlackBerry();

    void addBreakpoint(const unsigned short* url, unsigned urlLength, unsigned lineNumber, const unsigned short* condition, unsigned conditionLength);
    void updateBreakpoint(const unsigned short* url, unsigned urlLength, unsigned lineNumber, const unsigned short* condition, unsigned conditionLength);
    void removeBreakpoint(const unsigned short* url, unsigned urlLength, unsigned lineNumber);

    bool pauseOnExceptions();
    void setPauseOnExceptions(bool pause);

    void pauseInDebugger();
    void resumeDebugger();

    void stepOverStatementInDebugger();
    void stepIntoStatementInDebugger();
    void stepOutOfFunctionInDebugger();

    /* From ScriptDebugListener */
    virtual void didParseSource(const String&  sourceID, const String& url, const String& data, int firstLine);
    virtual void failedToParseSource(const String& url, const String& data, int firstLine, int errorLine, const String& errorMessage);
    virtual void didPause(ScriptState*);
    virtual void didContinue();

protected:
    void start();
    void stop();

private:
    Olympia::WebKit::WebPage* m_webPage;
    Olympia::WebKit::WebPageClient* m_pageClient;
    ScriptDebugServer& m_debugServer;

    JavaScriptCallFrame* m_currentCallFrame;
};

} // WebCore

#endif // JavaScriptDebuggerBlackBerry_h
