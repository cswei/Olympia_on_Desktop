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

#ifndef InspectorCSSStore_h
#define InspectorCSSStore_h

#include "StringHash.h"

#include <wtf/HashMap.h>
#include <wtf/RefPtr.h>

namespace WebCore {

class Document;
class InspectorController;
class InspectorFrontend;
class CSSMutableStyleDeclaration;
class CSSStyleDeclaration;
class CSSRuleList;
class CSSStyleRule;
class CSSStyleSheet;
class String;

typedef std::pair<String, String> PropertyValueAndPriority;
typedef std::pair<unsigned, unsigned> SourceRange;
typedef HashMap<String, PropertyValueAndPriority> DisabledStyleDeclaration;
typedef HashMap<CSSStyleDeclaration*, long> StyleToIdMap;
typedef HashMap<long, RefPtr<CSSStyleDeclaration> > IdToStyleMap;
typedef HashMap<CSSStyleRule*, long> RuleToIdMap;
typedef HashMap<long, RefPtr<CSSStyleRule> > IdToRuleMap;
typedef HashMap<CSSStyleSheet*, Vector<SourceRange>* > StyleSheetToOffsetsMap;
typedef HashMap<CSSStyleSheet*, long> StyleSheetToIdMap;
typedef HashMap<long, RefPtr<CSSStyleSheet> > IdToStyleSheetMap;
typedef HashMap<long, DisabledStyleDeclaration> IdToDisabledStyleMap;
typedef HashMap<RefPtr<Document>, RefPtr<CSSStyleSheet> > DocumentToStyleSheetMap;

class InspectorCSSStore {

public:
    InspectorCSSStore(InspectorController* inspectorController);
    ~InspectorCSSStore();
    void reset();
    SourceRange getStartEndOffsets(CSSStyleRule* rule);
    CSSStyleDeclaration* styleForId(long styleId);
    CSSStyleRule* ruleForId(long styleRuleId);
    DisabledStyleDeclaration* disabledStyleForId(long styleId, bool createIfAbsent);
    CSSStyleSheet* inspectorStyleSheet(Document* ownerDocument, bool createIfAbsent, long callId);
    void removeDocument(Document*);

    long bindRule(CSSStyleRule* rule);
    long bindStyle(CSSStyleDeclaration* style);
    long bindStyleSheet(CSSStyleSheet* styleSheet);

private:
    static unsigned getIndexInStyleRules(CSSStyleRule* rule, CSSRuleList* ruleList);

    StyleToIdMap m_styleToId;
    IdToStyleMap m_idToStyle;
    RuleToIdMap m_ruleToId;
    IdToRuleMap m_idToRule;
    StyleSheetToOffsetsMap m_styleSheetToOffsets;
    StyleSheetToIdMap m_styleSheetToId;
    IdToStyleSheetMap m_idToStyleSheet;
    IdToDisabledStyleMap m_idToDisabledStyle;
    DocumentToStyleSheetMap m_documentNodeToInspectorStyleSheetMap;

    InspectorController* m_inspectorController;
    long m_lastStyleId;
    long m_lastStyleSheetId;
    long m_lastRuleId;
};

} // namespace WebCore

#endif // !defined(InspectorCSSStore_h)
