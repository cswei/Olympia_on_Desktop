/*
 * Copyright (C) 2009 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Google Inc. All rights reserved.
 * Copyright (C) 2009 Joseph Pecoraro
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
#include "InspectorDOMAgent.h"

#if ENABLE(INSPECTOR)

#include "AtomicString.h"
#include "CSSComputedStyleDeclaration.h"
#include "CSSMutableStyleDeclaration.h"
#include "CSSRule.h"
#include "CSSRuleList.h"
#include "CSSStyleRule.h"
#include "CSSStyleSelector.h"
#include "CSSStyleSheet.h"
#include "ContainerNode.h"
#include "Cookie.h"
#include "CookieJar.h"
#include "DOMWindow.h"
#include "Document.h"
#include "DocumentType.h"
#include "Event.h"
#include "EventListener.h"
#include "EventNames.h"
#include "EventTarget.h"
#include "HTMLFrameOwnerElement.h"
#include "InspectorFrontend.h"
#include "markup.h"
#include "MutationEvent.h"
#include "Node.h"
#include "NodeList.h"
#include "PlatformString.h"
#include "RenderStyle.h"
#include "RenderStyleConstants.h"
#include "ScriptEventListener.h"
#include "ScriptObject.h"
#include "StyleSheetList.h"
#include "Text.h"

#include <wtf/text/CString.h>
#include <wtf/HashSet.h>
#include <wtf/OwnPtr.h>
#include <wtf/Vector.h>

namespace WebCore {

InspectorDOMAgent::InspectorDOMAgent(InspectorCSSStore* cssStore, InspectorFrontend* frontend)
    : EventListener(InspectorDOMAgentType)
    , m_cssStore(cssStore)
    , m_frontend(frontend)
    , m_lastNodeId(1)
{
}

InspectorDOMAgent::~InspectorDOMAgent()
{
    reset();
}

void InspectorDOMAgent::reset()
{
    discardBindings();

    ListHashSet<RefPtr<Document> > copy = m_documents;
    for (ListHashSet<RefPtr<Document> >::iterator it = copy.begin(); it != copy.end(); ++it)
        stopListening((*it).get());

    ASSERT(!m_documents.size());
}

void InspectorDOMAgent::setDocument(Document* doc)
{
    if (doc == mainFrameDocument())
        return;

    reset();

    if (doc) {
        startListening(doc);
        if (doc->documentElement())
            pushDocumentToFrontend();
    } else
        m_frontend->setDocument(ScriptObject());
}

void InspectorDOMAgent::releaseDanglingNodes()
{
    deleteAllValues(m_danglingNodeToIdMaps);
    m_danglingNodeToIdMaps.clear();
}

void InspectorDOMAgent::startListening(Document* doc)
{
    if (m_documents.contains(doc))
        return;

    doc->addEventListener(eventNames().DOMContentLoadedEvent, this, false);
    doc->addEventListener(eventNames().loadEvent, this, true);
    m_documents.add(doc);
}

void InspectorDOMAgent::stopListening(Document* doc)
{
    if (!m_documents.contains(doc))
        return;

    doc->removeEventListener(eventNames().DOMContentLoadedEvent, this, false);
    doc->removeEventListener(eventNames().loadEvent, this, true);
    m_documents.remove(doc);
}

void InspectorDOMAgent::handleEvent(ScriptExecutionContext*, Event* event)
{
    AtomicString type = event->type();
    Node* node = event->target()->toNode();

    if (type == eventNames().DOMContentLoadedEvent) {
        // Re-push document once it is loaded.
        discardBindings();
        pushDocumentToFrontend();
    } else if (type == eventNames().loadEvent) {
        long frameOwnerId = m_documentNodeToIdMap.get(node);
        if (!frameOwnerId)
            return;

        if (!m_childrenRequested.contains(frameOwnerId)) {
            // No children are mapped yet -> only notify on changes of hasChildren.
            m_frontend->childNodeCountUpdated(frameOwnerId, innerChildNodeCount(node));
        } else {
            // Re-add frame owner element together with its new children.
            long parentId = m_documentNodeToIdMap.get(innerParentNode(node));
            m_frontend->childNodeRemoved(parentId, frameOwnerId);
            ScriptObject value = buildObjectForNode(node, 0, &m_documentNodeToIdMap);
            Node* previousSibling = innerPreviousSibling(node);
            long prevId = previousSibling ? m_documentNodeToIdMap.get(previousSibling) : 0;
            m_frontend->childNodeInserted(parentId, prevId, value);
            // Invalidate children requested flag for the element.
            m_childrenRequested.remove(m_childrenRequested.find(frameOwnerId));
        }
    }
}

long InspectorDOMAgent::bind(Node* node, NodeToIdMap* nodesMap)
{
    long id = nodesMap->get(node);
    if (id)
        return id;
    id = m_lastNodeId++;
    nodesMap->set(node, id);
    m_idToNode.set(id, node);
    m_idToNodesMap.set(id, nodesMap);
    return id;
}

void InspectorDOMAgent::unbind(Node* node, NodeToIdMap* nodesMap)
{
    if (node->isFrameOwnerElement()) {
        const HTMLFrameOwnerElement* frameOwner = static_cast<const HTMLFrameOwnerElement*>(node);
        stopListening(frameOwner->contentDocument());
        cssStore()->removeDocument(frameOwner->contentDocument());
    }

    long id = nodesMap->get(node);
    if (!id)
        return;
    m_idToNode.remove(id);
    nodesMap->remove(node);
    bool childrenRequested = m_childrenRequested.contains(id);
    if (childrenRequested) {
        // Unbind subtree known to client recursively.
        m_childrenRequested.remove(id);
        Node* child = innerFirstChild(node);
        while (child) {
            unbind(child, nodesMap);
            child = innerNextSibling(child);
        }
    }
}

bool InspectorDOMAgent::pushDocumentToFrontend()
{
    Document* document = mainFrameDocument();
    if (!document)
        return false;
    if (!m_documentNodeToIdMap.contains(document))
        m_frontend->setDocument(buildObjectForNode(document, 2, &m_documentNodeToIdMap));
    return true;
}

void InspectorDOMAgent::pushChildNodesToFrontend(long nodeId)
{
    Node* node = nodeForId(nodeId);
    if (!node || (node->nodeType() != Node::ELEMENT_NODE && node->nodeType() != Node::DOCUMENT_NODE && node->nodeType() != Node::DOCUMENT_FRAGMENT_NODE))
        return;
    if (m_childrenRequested.contains(nodeId))
        return;

    NodeToIdMap* nodeMap = m_idToNodesMap.get(nodeId);
    ScriptArray children = buildArrayForContainerChildren(node, 1, nodeMap);
    m_childrenRequested.add(nodeId);
    m_frontend->setChildNodes(nodeId, children);
}

void InspectorDOMAgent::discardBindings()
{
    m_documentNodeToIdMap.clear();
    m_idToNode.clear();
    releaseDanglingNodes();
    m_childrenRequested.clear();
}

Node* InspectorDOMAgent::nodeForId(long id)
{
    if (!id)
        return 0;

    HashMap<long, Node*>::iterator it = m_idToNode.find(id);
    if (it != m_idToNode.end())
        return it->second;
    return 0;
}

Node* InspectorDOMAgent::nodeForPath(const String& path)
{
    // The path is of form "1,HTML,2,BODY,1,DIV"
    Node* node = mainFrameDocument();
    if (!node)
        return 0;

    Vector<String> pathTokens;
    path.split(",", false, pathTokens);
    for (size_t i = 0; i < pathTokens.size() - 1; i += 2) {
        bool success = true;
        unsigned childNumber = pathTokens[i].toUInt(&success);
        if (!success)
            return 0;
        if (childNumber >= innerChildNodeCount(node))
            return 0;

        Node* child = innerFirstChild(node);
        String childName = pathTokens[i + 1];
        for (size_t j = 0; child && j < childNumber; ++j)
            child = innerNextSibling(child);

        if (!child || child->nodeName() != childName)
            return 0;
        node = child;
    }
    return node;
}

void InspectorDOMAgent::getChildNodes(long callId, long nodeId)
{
    pushChildNodesToFrontend(nodeId);
    m_frontend->didGetChildNodes(callId);
}

long InspectorDOMAgent::pushNodePathToFrontend(Node* nodeToPush)
{
    ASSERT(nodeToPush);  // Invalid input

    // If we are sending information to the client that is currently being created. Send root node first.
    if (!pushDocumentToFrontend())
        return 0;

    // Return id in case the node is known.
    long result = m_documentNodeToIdMap.get(nodeToPush);
    if (result)
        return result;

    Node* node = nodeToPush;
    Vector<Node*> path;
    NodeToIdMap* danglingMap = 0;
    while (true) {
        Node* parent = innerParentNode(node);
        if (!parent) {
            // Node being pushed is detached -> push subtree root.
            danglingMap = new NodeToIdMap();
            m_danglingNodeToIdMaps.append(danglingMap);
            m_frontend->setDetachedRoot(buildObjectForNode(node, 0, danglingMap));
            break;
        } else {
            path.append(parent);
            if (m_documentNodeToIdMap.get(parent))
                break;
            else
                node = parent;
        }
    }

    NodeToIdMap* map = danglingMap ? danglingMap : &m_documentNodeToIdMap;
    for (int i = path.size() - 1; i >= 0; --i) {
        long nodeId = map->get(path.at(i));
        ASSERT(nodeId);
        pushChildNodesToFrontend(nodeId);
    }
    return map->get(nodeToPush);
}

void InspectorDOMAgent::setAttribute(long callId, long elementId, const String& name, const String& value)
{
    Node* node = nodeForId(elementId);
    if (node && (node->nodeType() == Node::ELEMENT_NODE)) {
        Element* element = static_cast<Element*>(node);
        ExceptionCode ec = 0;
        element->setAttribute(name, value, ec);
        m_frontend->didApplyDomChange(callId, ec == 0);
    } else {
        m_frontend->didApplyDomChange(callId, false);
    }
}

void InspectorDOMAgent::removeAttribute(long callId, long elementId, const String& name)
{
    Node* node = nodeForId(elementId);
    if (node && (node->nodeType() == Node::ELEMENT_NODE)) {
        Element* element = static_cast<Element*>(node);
        ExceptionCode ec = 0;
        element->removeAttribute(name, ec);
        m_frontend->didApplyDomChange(callId, ec == 0);
    } else {
        m_frontend->didApplyDomChange(callId, false);
    }
}

void InspectorDOMAgent::removeNode(long callId, long nodeId)
{
    Node* node = nodeForId(nodeId);
    if (!node) {
        // Use -1 to denote an error condition.
        m_frontend->didRemoveNode(callId, -1);
        return;
    }

    Node* parentNode = node->parentNode();
    if (!parentNode) {
        m_frontend->didRemoveNode(callId, -1);
        return;
    }

    ExceptionCode code;
    parentNode->removeChild(node, code);
    if (code) {
        m_frontend->didRemoveNode(callId, -1);
        return;
    }

    m_frontend->didRemoveNode(callId, nodeId);
}

void InspectorDOMAgent::changeTagName(long callId, long nodeId, const AtomicString& tagName, bool expanded)
{
    Node* oldNode = nodeForId(nodeId);
    if (!oldNode || !oldNode->isElementNode()) {
        // Use -1 to denote an error condition.
        m_frontend->didChangeTagName(callId, -1);
        return;
    }

    ExceptionCode code = 0;
    RefPtr<Element> newElem = oldNode->document()->createElement(tagName, code);
    if (code) {
        m_frontend->didChangeTagName(callId, -1);
        return;
    }

    // Copy over the original node's attributes.
    Element* oldElem = static_cast<Element*>(oldNode);
    newElem->copyNonAttributeProperties(oldElem);
    if (oldElem->attributes())
        newElem->attributes()->setAttributes(*(oldElem->attributes(true)));

    // Copy over the original node's children.
    Node* child;
    while ((child = oldNode->firstChild()))
        newElem->appendChild(child, code);

    // Replace the old node with the new node
    Node* parent = oldNode->parentNode();
    parent->insertBefore(newElem, oldNode->nextSibling(), code);
    parent->removeChild(oldNode, code);

    if (code) {
        m_frontend->didChangeTagName(callId, -1);
        return;
    }

    long newId = pushNodePathToFrontend(newElem.get());
    if (expanded)
        pushChildNodesToFrontend(newId);
    m_frontend->didChangeTagName(callId, newId);
}

void InspectorDOMAgent::setTextNodeValue(long callId, long nodeId, const String& value)
{
    Node* node = nodeForId(nodeId);
    if (node && (node->nodeType() == Node::TEXT_NODE)) {
        Text* text_node = static_cast<Text*>(node);
        ExceptionCode ec = 0;
        text_node->replaceWholeText(value, ec);
        m_frontend->didApplyDomChange(callId, ec == 0);
    } else {
        m_frontend->didApplyDomChange(callId, false);
    }
}

void InspectorDOMAgent::getEventListenersForNode(long callId, long nodeId)
{
    Node* node = nodeForId(nodeId);
    ScriptArray listenersArray = m_frontend->newScriptArray();
    unsigned counter = 0;
    EventTargetData* d;

    // Quick break if a null node or no listeners at all
    if (!node || !(d = node->eventTargetData())) {
        m_frontend->didGetEventListenersForNode(callId, nodeId, listenersArray);
        return;
    }

    // Get the list of event types this Node is concerned with
    Vector<AtomicString> eventTypes;
    const EventListenerMap& listenerMap = d->eventListenerMap;
    EventListenerMap::const_iterator end = listenerMap.end();
    for (EventListenerMap::const_iterator iter = listenerMap.begin(); iter != end; ++iter)
        eventTypes.append(iter->first);

    // Quick break if no useful listeners
    size_t eventTypesLength = eventTypes.size();
    if (eventTypesLength == 0) {
        m_frontend->didGetEventListenersForNode(callId, nodeId, listenersArray);
        return;
    }

    // The Node's Event Ancestors (not including self)
    Vector<RefPtr<ContainerNode> > ancestors;
    node->eventAncestors(ancestors);

    // Nodes and their Listeners for the concerned event types (order is top to bottom)
    Vector<EventListenerInfo> eventInformation;
    for (size_t i = ancestors.size(); i; --i) {
        ContainerNode* ancestor = ancestors[i - 1].get();
        for (size_t j = 0; j < eventTypesLength; ++j) {
            AtomicString& type = eventTypes[j];
            if (ancestor->hasEventListeners(type))
                eventInformation.append(EventListenerInfo(static_cast<Node*>(ancestor), type, ancestor->getEventListeners(type)));
        }
    }

    // Insert the Current Node at the end of that list (last in capturing, first in bubbling)
    for (size_t i = 0; i < eventTypesLength; ++i) {
        const AtomicString& type = eventTypes[i];
        eventInformation.append(EventListenerInfo(node, type, node->getEventListeners(type)));
    }

    // Get Capturing Listeners (in this order)
    size_t eventInformationLength = eventInformation.size();
    for (size_t i = 0; i < eventInformationLength; ++i) {
        const EventListenerInfo& info = eventInformation[i];
        const EventListenerVector& vector = info.eventListenerVector;
        for (size_t j = 0; j < vector.size(); ++j) {
            const RegisteredEventListener& listener = vector[j];
            if (listener.useCapture)
                listenersArray.set(counter++, buildObjectForEventListener(listener, info.eventType, info.node));
        }
    }

    // Get Bubbling Listeners (reverse order)
    for (size_t i = eventInformationLength; i; --i) {
        const EventListenerInfo& info = eventInformation[i - 1];
        const EventListenerVector& vector = info.eventListenerVector;
        for (size_t j = 0; j < vector.size(); ++j) {
            const RegisteredEventListener& listener = vector[j];
            if (!listener.useCapture)
                listenersArray.set(counter++, buildObjectForEventListener(listener, info.eventType, info.node));
        }
    }

    m_frontend->didGetEventListenersForNode(callId, nodeId, listenersArray);
}

String InspectorDOMAgent::documentURLString(Document* document) const
{
    if (!document || document->url().isNull())
        return "";
    return document->url().string();
}

ScriptObject InspectorDOMAgent::buildObjectForNode(Node* node, int depth, NodeToIdMap* nodesMap)
{
    ScriptObject value = m_frontend->newScriptObject();

    long id = bind(node, nodesMap);
    String nodeName;
    String localName;
    String nodeValue;

    switch (node->nodeType()) {
        case Node::TEXT_NODE:
        case Node::COMMENT_NODE:
            nodeValue = node->nodeValue();
            break;
        case Node::ATTRIBUTE_NODE:
            localName = node->localName();
            break;
        case Node::DOCUMENT_FRAGMENT_NODE:
            break;
        case Node::DOCUMENT_NODE:
        case Node::ELEMENT_NODE:
        default:
            nodeName = node->nodeName();
            localName = node->localName();
            break;
    }

    value.set("id", id);
    value.set("nodeType", node->nodeType());
    value.set("nodeName", nodeName);
    value.set("localName", localName);
    value.set("nodeValue", nodeValue);

    if (node->nodeType() == Node::ELEMENT_NODE || node->nodeType() == Node::DOCUMENT_NODE || node->nodeType() == Node::DOCUMENT_FRAGMENT_NODE) {
        int nodeCount = innerChildNodeCount(node);
        value.set("childNodeCount", nodeCount);
        ScriptArray children = buildArrayForContainerChildren(node, depth, nodesMap);
        if (children.length() > 0)
            value.set("children", children);

        if (node->nodeType() == Node::ELEMENT_NODE) {
            Element* element = static_cast<Element*>(node);
            value.set("attributes", buildArrayForElementAttributes(element));
            if (node->isFrameOwnerElement()) {
                HTMLFrameOwnerElement* frameOwner = static_cast<HTMLFrameOwnerElement*>(node);
                value.set("documentURL", documentURLString(frameOwner->contentDocument()));
            }
        } else if (node->nodeType() == Node::DOCUMENT_NODE) {
            Document* document = static_cast<Document*>(node);
            value.set("documentURL", documentURLString(document));
        }
    } else if (node->nodeType() == Node::DOCUMENT_TYPE_NODE) {
        DocumentType* docType = static_cast<DocumentType*>(node);
        value.set("publicId", docType->publicId());
        value.set("systemId", docType->systemId());
        value.set("internalSubset", docType->internalSubset());
    }
    return value;
}

ScriptArray InspectorDOMAgent::buildArrayForElementAttributes(Element* element)
{
    ScriptArray attributesValue = m_frontend->newScriptArray();
    // Go through all attributes and serialize them.
    const NamedNodeMap* attrMap = element->attributes(true);
    if (!attrMap)
        return attributesValue;
    unsigned numAttrs = attrMap->length();
    int index = 0;
    for (unsigned i = 0; i < numAttrs; ++i) {
        // Add attribute pair
        const Attribute *attribute = attrMap->attributeItem(i);
        attributesValue.set(index++, attribute->name().toString());
        attributesValue.set(index++, attribute->value());
    }
    return attributesValue;
}

ScriptArray InspectorDOMAgent::buildArrayForContainerChildren(Node* container, int depth, NodeToIdMap* nodesMap)
{
    ScriptArray children = m_frontend->newScriptArray();
    if (depth == 0) {
        int index = 0;
        // Special case the_only text child.
        if (innerChildNodeCount(container) == 1) {
            Node *child = innerFirstChild(container);
            if (child->nodeType() == Node::TEXT_NODE)
                children.set(index++, buildObjectForNode(child, 0, nodesMap));
        }
        return children;
    } else if (depth > 0) {
        depth--;
    }

    int index = 0;
    for (Node *child = innerFirstChild(container); child; child = innerNextSibling(child))
        children.set(index++, buildObjectForNode(child, depth, nodesMap));
    return children;
}

ScriptObject InspectorDOMAgent::buildObjectForEventListener(const RegisteredEventListener& registeredEventListener, const AtomicString& eventType, Node* node)
{
    RefPtr<EventListener> eventListener = registeredEventListener.listener;
    ScriptObject value = m_frontend->newScriptObject();
    value.set("type", eventType);
    value.set("useCapture", registeredEventListener.useCapture);
    value.set("isAttribute", eventListener->isAttribute());
    value.set("nodeId", pushNodePathToFrontend(node));
    value.set("listenerBody", eventListenerHandlerBody(node->document(), m_frontend->scriptState(), eventListener.get()));
    String sourceName;
    int lineNumber;
    if (eventListenerHandlerLocation(node->document(), m_frontend->scriptState(), eventListener.get(), sourceName, lineNumber)) {
        value.set("sourceName", sourceName);
        value.set("lineNumber", lineNumber);
    }
    return value;
}

Node* InspectorDOMAgent::innerFirstChild(Node* node)
{
    if (node->isFrameOwnerElement()) {
        HTMLFrameOwnerElement* frameOwner = static_cast<HTMLFrameOwnerElement*>(node);
        Document* doc = frameOwner->contentDocument();
        if (doc) {
            startListening(doc);
            return doc->firstChild();
        }
    }
    node = node->firstChild();
    while (isWhitespace(node))
        node = node->nextSibling();
    return node;
}

Node* InspectorDOMAgent::innerNextSibling(Node* node)
{
    do {
        node = node->nextSibling();
    } while (isWhitespace(node));
    return node;
}

Node* InspectorDOMAgent::innerPreviousSibling(Node* node)
{
    do {
        node = node->previousSibling();
    } while (isWhitespace(node));
    return node;
}

unsigned InspectorDOMAgent::innerChildNodeCount(Node* node)
{
    unsigned count = 0;
    Node* child = innerFirstChild(node);
    while (child) {
        count++;
        child = innerNextSibling(child);
    }
    return count;
}

Node* InspectorDOMAgent::innerParentNode(Node* node)
{
    Node* parent = node->parentNode();
    if (parent && parent->nodeType() == Node::DOCUMENT_NODE)
        return static_cast<Document*>(parent)->ownerElement();
    return parent;
}

bool InspectorDOMAgent::isWhitespace(Node* node)
{
    //TODO: pull ignoreWhitespace setting from the frontend and use here.
    return node && node->nodeType() == Node::TEXT_NODE && node->nodeValue().stripWhiteSpace().length() == 0;
}

Document* InspectorDOMAgent::mainFrameDocument() const
{
    ListHashSet<RefPtr<Document> >::const_iterator it = m_documents.begin();
    if (it != m_documents.end())
        return it->get();
    return 0;
}

bool InspectorDOMAgent::operator==(const EventListener& listener)
{
    if (const InspectorDOMAgent* inspectorDOMAgentListener = InspectorDOMAgent::cast(&listener))
        return mainFrameDocument() == inspectorDOMAgentListener->mainFrameDocument();
    return false;
}

void InspectorDOMAgent::didInsertDOMNode(Node* node)
{
    if (isWhitespace(node))
        return;

    // We could be attaching existing subtree. Forget the bindings.
    unbind(node, &m_documentNodeToIdMap);

    Node* parent = node->parentNode();
    long parentId = m_documentNodeToIdMap.get(parent);
    // Return if parent is not mapped yet.
    if (!parentId)
        return;

    if (!m_childrenRequested.contains(parentId)) {
        // No children are mapped yet -> only notify on changes of hasChildren.
        m_frontend->childNodeCountUpdated(parentId, innerChildNodeCount(parent));
    } else {
        // Children have been requested -> return value of a new child.
        Node* prevSibling = innerPreviousSibling(node);
        long prevId = prevSibling ? m_documentNodeToIdMap.get(prevSibling) : 0;
        ScriptObject value = buildObjectForNode(node, 0, &m_documentNodeToIdMap);
        m_frontend->childNodeInserted(parentId, prevId, value);
    }
}

void InspectorDOMAgent::didRemoveDOMNode(Node* node)
{
    if (isWhitespace(node))
        return;

    Node* parent = node->parentNode();
    long parentId = m_documentNodeToIdMap.get(parent);
    // If parent is not mapped yet -> ignore the event.
    if (!parentId)
        return;

    if (!m_childrenRequested.contains(parentId)) {
        // No children are mapped yet -> only notify on changes of hasChildren.
        if (innerChildNodeCount(parent) == 1)
            m_frontend->childNodeCountUpdated(parentId, 0);
    } else
        m_frontend->childNodeRemoved(parentId, m_documentNodeToIdMap.get(node));
    unbind(node, &m_documentNodeToIdMap);
}

void InspectorDOMAgent::didModifyDOMAttr(Element* element)
{
    long id = m_documentNodeToIdMap.get(element);
    // If node is not mapped yet -> ignore the event.
    if (!id)
        return;

    m_frontend->attributesUpdated(id, buildArrayForElementAttributes(element));
}

void InspectorDOMAgent::getStyles(long callId, long nodeId, bool authorOnly)
{
    Node* node = nodeForId(nodeId);
    if (!node || node->nodeType() != Node::ELEMENT_NODE) {
        m_frontend->didGetStyles(callId, ScriptValue::undefined());
        return;
    }

    DOMWindow* defaultView = node->ownerDocument()->defaultView();
    if (!defaultView) {
        m_frontend->didGetStyles(callId, ScriptValue::undefined());
        return;
    }

    Element* element = static_cast<Element*>(node);
    RefPtr<CSSComputedStyleDeclaration> computedStyleInfo = computedStyle(node, true); // Support the viewing of :visited information in computed style.

    ScriptObject result = m_frontend->newScriptObject();
    if (element->style())
        result.set("inlineStyle", buildObjectForStyle(element->style(), true));
    result.set("computedStyle", buildObjectForStyle(computedStyleInfo.get(), false));

    CSSStyleSelector* selector = element->ownerDocument()->styleSelector();
    RefPtr<CSSRuleList> matchedRules = selector->styleRulesForElement(element, authorOnly);
    result.set("matchedCSSRules", buildArrayForCSSRules(node->ownerDocument(), matchedRules.get()));

    result.set("styleAttributes", buildObjectForAttributeStyles(element));
    result.set("pseudoElements", buildArrayForPseudoElements(element, authorOnly));

    ScriptObject currentStyle = result;
    Element* parentElement = element->parentElement();
    while (parentElement) {
        ScriptObject parentStyle = m_frontend->newScriptObject();
        currentStyle.set("parent", parentStyle);
        if (parentElement->style() && parentElement->style()->length())
            parentStyle.set("inlineStyle", buildObjectForStyle(parentElement->style(), true));

        CSSStyleSelector* parentSelector = parentElement->ownerDocument()->styleSelector();
        RefPtr<CSSRuleList> parentMatchedRules = parentSelector->styleRulesForElement(parentElement, authorOnly);
        parentStyle.set("matchedCSSRules", buildArrayForCSSRules(parentElement->ownerDocument(), parentMatchedRules.get()));

        parentElement = parentElement->parentElement();
        currentStyle = parentStyle;
    }
    m_frontend->didGetStyles(callId, result);
}

void InspectorDOMAgent::getAllStyles(long callId)
{
    ScriptArray result = m_frontend->newScriptArray();
    unsigned counter = 0;
    for (ListHashSet<RefPtr<Document> >::iterator it = m_documents.begin(); it != m_documents.end(); ++it) {
        StyleSheetList* list = (*it)->styleSheets();
        for (unsigned i = 0; i < list->length(); ++i) {
            StyleSheet* styleSheet = list->item(i);
            if (styleSheet->isCSSStyleSheet())
                result.set(counter++, buildObjectForStyleSheet((*it).get(), static_cast<CSSStyleSheet*>(styleSheet)));
        }
    }
    m_frontend->didGetAllStyles(callId, result);
}

void InspectorDOMAgent::getInlineStyle(long callId, long nodeId)
{
    Node* node = nodeForId(nodeId);
    if (!node || node->nodeType() != Node::ELEMENT_NODE) {
        m_frontend->didGetInlineStyle(callId, ScriptValue::undefined());
        return;
    }
    Element* element = static_cast<Element*>(node);
    m_frontend->didGetInlineStyle(callId, buildObjectForStyle(element->style(), true));
}

void InspectorDOMAgent::getComputedStyle(long callId, long nodeId)
{
    Node* node = nodeForId(nodeId);
    if (!node || node->nodeType() != Node::ELEMENT_NODE) {
        m_frontend->didGetComputedStyle(callId, ScriptValue::undefined());
        return;
    }

    DOMWindow* defaultView = node->ownerDocument()->defaultView();
    if (!defaultView) {
        m_frontend->didGetComputedStyle(callId, ScriptValue::undefined());
        return;
    }

    Element* element = static_cast<Element*>(node);
    RefPtr<CSSStyleDeclaration> computedStyle = defaultView->getComputedStyle(element, "");
    m_frontend->didGetComputedStyle(callId, buildObjectForStyle(computedStyle.get(), false));
}

ScriptObject InspectorDOMAgent::buildObjectForAttributeStyles(Element* element)
{
    ScriptObject styleAttributes = m_frontend->newScriptObject();
    NamedNodeMap* attributes = element->attributes();
    for (unsigned i = 0; attributes && i < attributes->length(); ++i) {
        Attribute* attribute = attributes->attributeItem(i);
        if (attribute->style()) {
            String attributeName = attribute->localName();
            styleAttributes.set(attributeName.utf8().data(), buildObjectForStyle(attribute->style(), true));
        }
    }
    return styleAttributes;
}

ScriptArray InspectorDOMAgent::buildArrayForCSSRules(Document* ownerDocument, CSSRuleList* matchedRules)
{
    ScriptArray matchedCSSRules = m_frontend->newScriptArray();
    unsigned counter = 0;
    for (unsigned i = 0; matchedRules && i < matchedRules->length(); ++i) {
        CSSRule* rule = matchedRules->item(i);
        if (rule->type() == CSSRule::STYLE_RULE)
            matchedCSSRules.set(counter++, buildObjectForRule(ownerDocument, static_cast<CSSStyleRule*>(rule)));
    }
    return matchedCSSRules;
}

ScriptArray InspectorDOMAgent::buildArrayForPseudoElements(Element* element, bool authorOnly)
{
    ScriptArray result = m_frontend->newScriptArray();
    CSSStyleSelector* selector = element->ownerDocument()->styleSelector();
    RefPtr<RenderStyle> renderStyle = element->styleForRenderer();
    unsigned counter = 0;

    for (PseudoId pseudoId = FIRST_PUBLIC_PSEUDOID; pseudoId < AFTER_LAST_INTERNAL_PSEUDOID; pseudoId = static_cast<PseudoId>(pseudoId + 1)) {
        RefPtr<CSSRuleList> matchedRules = selector->pseudoStyleRulesForElement(element, pseudoId, authorOnly);
        if (matchedRules && matchedRules->length()) {
            ScriptObject pseudoStyles = m_frontend->newScriptObject();
            pseudoStyles.set("pseudoId", static_cast<int>(pseudoId));
            pseudoStyles.set("rules", buildArrayForCSSRules(element->ownerDocument(), matchedRules.get()));
            result.set(counter++, pseudoStyles);
        }
    }
    return result;
}

void InspectorDOMAgent::applyStyleText(long callId, long styleId, const String& styleText, const String& propertyName)
{
    CSSStyleDeclaration* style = cssStore()->styleForId(styleId);
    if (!style) {
        m_frontend->didApplyStyleText(callId, false, ScriptValue::undefined(), m_frontend->newScriptArray());
        return;
    }

    // Remove disabled property entry for property with given name.
    DisabledStyleDeclaration* disabledStyle = cssStore()->disabledStyleForId(styleId, false);
    if (disabledStyle)
        disabledStyle->remove(propertyName);

    int styleTextLength = styleText.length();

    RefPtr<CSSMutableStyleDeclaration> tempMutableStyle = CSSMutableStyleDeclaration::create();
    tempMutableStyle->parseDeclaration(styleText);
    CSSStyleDeclaration* tempStyle = static_cast<CSSStyleDeclaration*>(tempMutableStyle.get());

    if (tempStyle->length() || !styleTextLength) {
        ExceptionCode ec = 0;
        // The input was parsable or the user deleted everything, so remove the
        // original property from the real style declaration. If this represents
        // a shorthand remove all the longhand properties.
        if (style->getPropertyShorthand(propertyName).isEmpty()) {
            Vector<String> longhandProps = longhandProperties(style, propertyName);
            for (unsigned i = 0; !ec && i < longhandProps.size(); ++i)
                style->removeProperty(longhandProps[i], ec);
        }
        // Explicitly delete properties with no shorthands as well as shorthands themselves.
        if (!ec)
            style->removeProperty(propertyName, ec);

        if (ec) {
            m_frontend->didApplyStyleText(callId, false, ScriptValue::undefined(), m_frontend->newScriptArray());
            return;
        }
    }

    // Notify caller that the property was successfully deleted.
    if (!styleTextLength) {
        ScriptArray changedProperties = m_frontend->newScriptArray();
        changedProperties.set(0, propertyName);
        m_frontend->didApplyStyleText(callId, true, ScriptValue::undefined(), changedProperties);
        return;
    }

    if (!tempStyle->length()) {
        m_frontend->didApplyStyleText(callId, false, ScriptValue::undefined(), m_frontend->newScriptArray());
        return;
    }

    // Iterate of the properties on the test element's style declaration and
    // add them to the real style declaration. We take care to move shorthands.
    HashSet<String> foundShorthands;
    Vector<String> changedProperties;

    for (unsigned i = 0; i < tempStyle->length(); ++i) {
        String name = tempStyle->item(i);
        String shorthand = tempStyle->getPropertyShorthand(name);

        if (!shorthand.isEmpty() && foundShorthands.contains(shorthand))
            continue;

        String value;
        String priority;
        if (!shorthand.isEmpty()) {
            value = shorthandValue(tempStyle, shorthand);
            priority = shorthandPriority(tempStyle, shorthand);
            foundShorthands.add(shorthand);
            name = shorthand;
        } else {
            value = tempStyle->getPropertyValue(name);
            priority = tempStyle->getPropertyPriority(name);
        }

        // Set the property on the real style declaration.
        ExceptionCode ec = 0;
        style->setProperty(name, value, priority, ec);
        // Remove disabled property entry for property with this name.
        if (disabledStyle)
            disabledStyle->remove(name);
        changedProperties.append(name);
    }
    m_frontend->didApplyStyleText(callId, true, buildObjectForStyle(style, true), toArray(changedProperties));
}

void InspectorDOMAgent::setStyleText(long callId, long styleId, const String& cssText)
{
    CSSStyleDeclaration* style = cssStore()->styleForId(styleId);
    if (!style) {
        m_frontend->didSetStyleText(callId, false);
        return;
    }
    ExceptionCode ec = 0;
    style->setCssText(cssText, ec);
    m_frontend->didSetStyleText(callId, !ec);
}

void InspectorDOMAgent::setStyleProperty(long callId, long styleId, const String& name, const String& value)
{
    CSSStyleDeclaration* style = cssStore()->styleForId(styleId);
    if (!style) {
        m_frontend->didSetStyleProperty(callId, false);
        return;
    }

    ExceptionCode ec = 0;
    style->setProperty(name, value, ec);
    m_frontend->didSetStyleProperty(callId, !ec);
}

void InspectorDOMAgent::toggleStyleEnabled(long callId, long styleId, const String& propertyName, bool disabled)
{
    CSSStyleDeclaration* style = cssStore()->styleForId(styleId);
    if (!style) {
        m_frontend->didToggleStyleEnabled(callId, ScriptValue::undefined());
        return;
    }

    DisabledStyleDeclaration* disabledStyle = cssStore()->disabledStyleForId(styleId, true);

    // TODO: make sure this works with shorthands right.
    ExceptionCode ec = 0;
    if (disabled) {
        disabledStyle->set(propertyName, std::make_pair(style->getPropertyValue(propertyName), style->getPropertyPriority(propertyName)));
        if (!ec)
            style->removeProperty(propertyName, ec);
    } else if (disabledStyle->contains(propertyName)) {
        PropertyValueAndPriority valueAndPriority = disabledStyle->get(propertyName);
        style->setProperty(propertyName, valueAndPriority.first, valueAndPriority.second, ec);
        if (!ec)
            disabledStyle->remove(propertyName);
    }
    if (ec) {
        m_frontend->didToggleStyleEnabled(callId, ScriptValue::undefined());
        return;
    }
    m_frontend->didToggleStyleEnabled(callId, buildObjectForStyle(style, true));
}

void InspectorDOMAgent::setRuleSelector(long callId, long ruleId, const String& selector, long selectedNodeId)
{
    CSSStyleRule* rule = cssStore()->ruleForId(ruleId);
    if (!rule) {
        m_frontend->didSetRuleSelector(callId, ScriptValue::undefined(), false);
        return;
    }

    Node* node = nodeForId(selectedNodeId);

    CSSStyleSheet* styleSheet = rule->parentStyleSheet();
    ExceptionCode ec = 0;
    styleSheet->addRule(selector, rule->style()->cssText(), ec);
    if (ec) {
        m_frontend->didSetRuleSelector(callId, ScriptValue::undefined(), false);
        return;
    }

    CSSStyleRule* newRule = static_cast<CSSStyleRule*>(styleSheet->item(styleSheet->length() - 1));
    for (unsigned i = 0; i < styleSheet->length(); ++i) {
        if (styleSheet->item(i) == rule) {
            styleSheet->deleteRule(i, ec);
            break;
        }
    }

    if (ec) {
        m_frontend->didSetRuleSelector(callId, ScriptValue::undefined(), false);
        return;
    }

    m_frontend->didSetRuleSelector(callId, buildObjectForRule(node->ownerDocument(), newRule), ruleAffectsNode(newRule, node));
}

void InspectorDOMAgent::addRule(long callId, const String& selector, long selectedNodeId)
{
    Node* node = nodeForId(selectedNodeId);
    if (!node) {
        m_frontend->didAddRule(callId, ScriptValue::undefined(), false);
        return;
    }

    CSSStyleSheet* styleSheet = cssStore()->inspectorStyleSheet(node->ownerDocument(), true, callId);
    if (!styleSheet)
        return; // could not add a stylesheet to the ownerDocument

    ExceptionCode ec = 0;
    styleSheet->addRule(selector, "", ec);
    if (ec) {
        m_frontend->didAddRule(callId, ScriptValue::undefined(), false);
        return;
    }

    CSSStyleRule* newRule = static_cast<CSSStyleRule*>(styleSheet->item(styleSheet->length() - 1));
    m_frontend->didAddRule(callId, buildObjectForRule(node->ownerDocument(), newRule), ruleAffectsNode(newRule, node));
}

ScriptObject InspectorDOMAgent::buildObjectForStyle(CSSStyleDeclaration* style, bool bind)
{
    ScriptObject result = m_frontend->newScriptObject();
    if (bind) {
        long styleId = cssStore()->bindStyle(style);
        result.set("id", styleId);

        DisabledStyleDeclaration* disabledStyle = cssStore()->disabledStyleForId(styleId, false);
        if (disabledStyle)
            result.set("disabled", buildArrayForDisabledStyleProperties(disabledStyle));
    }
    result.set("width", style->getPropertyValue("width"));
    result.set("height", style->getPropertyValue("height"));
    if (bind) {
        CSSRule* parentRule = style->parentRule();
        if (parentRule && parentRule->type() == CSSRule::STYLE_RULE) {
            CSSStyleRule* parentStyleRule = static_cast<CSSStyleRule*>(parentRule);
            std::pair<unsigned, unsigned> startEnd = cssStore()->getStartEndOffsets(parentStyleRule);
            if (startEnd.second) {
                result.set("bodyStartOffset", startEnd.first);
                result.set("bodyEndOffset", startEnd.second);
            }
        }
    }
    populateObjectWithStyleProperties(style, result);
    return result;
}

void InspectorDOMAgent::populateObjectWithStyleProperties(CSSStyleDeclaration* style, ScriptObject& result)
{
    ScriptArray properties = m_frontend->newScriptArray();
    ScriptObject shorthandValues = m_frontend->newScriptObject();
    result.set("properties", properties);
    result.set("shorthandValues", shorthandValues);

    HashSet<String> foundShorthands;
    for (unsigned i = 0; i < style->length(); ++i) {
        ScriptObject property = m_frontend->newScriptObject();
        String name = style->item(i);
        property.set("name", name);
        property.set("priority", style->getPropertyPriority(name));
        property.set("implicit", style->isPropertyImplicit(name));
        String shorthand =  style->getPropertyShorthand(name);
        property.set("shorthand", shorthand);
        if (!shorthand.isEmpty() && !foundShorthands.contains(shorthand)) {
            foundShorthands.add(shorthand);
            shorthandValues.set(shorthand, shorthandValue(style, shorthand));
        }
        property.set("value", style->getPropertyValue(name));
        properties.set(i, property);
    }
}

ScriptArray InspectorDOMAgent::buildArrayForDisabledStyleProperties(DisabledStyleDeclaration* declaration)
{
    int counter = 0;
    ScriptArray properties = m_frontend->newScriptArray();
    for (DisabledStyleDeclaration::iterator it = declaration->begin(); it != declaration->end(); ++it) {
        ScriptObject property = m_frontend->newScriptObject();
        property.set("name", it->first);
        property.set("value", it->second.first);
        property.set("priority", it->second.second);
        properties.set(counter++, property);
    }
    return properties;
}

ScriptObject InspectorDOMAgent::buildObjectForStyleSheet(Document* ownerDocument, CSSStyleSheet* styleSheet)
{
    ScriptObject result = m_frontend->newScriptObject();
    result.set("disabled", styleSheet->disabled());
    result.set("href", styleSheet->href());
    result.set("title", styleSheet->title());
    result.set("documentElementId", m_documentNodeToIdMap.get(styleSheet->doc()));
    ScriptArray cssRules = m_frontend->newScriptArray();
    result.set("cssRules", cssRules);
    PassRefPtr<CSSRuleList> cssRuleList = CSSRuleList::create(styleSheet, true);
    if (!cssRuleList)
        return result;
    unsigned counter = 0;
    for (unsigned i = 0; i < cssRuleList->length(); ++i) {
        CSSRule* rule = cssRuleList->item(i);
        if (rule->isStyleRule())
            cssRules.set(counter++, buildObjectForRule(ownerDocument, static_cast<CSSStyleRule*>(rule)));
    }
    return result;
}

ScriptObject InspectorDOMAgent::buildObjectForRule(Document* ownerDocument, CSSStyleRule* rule)
{
    CSSStyleSheet* parentStyleSheet = rule->parentStyleSheet();

    ScriptObject result = m_frontend->newScriptObject();
    result.set("selectorText", rule->selectorText());
    result.set("cssText", rule->cssText());
    result.set("sourceLine", rule->sourceLine());
    std::pair<unsigned, unsigned> startEnd = cssStore()->getStartEndOffsets(rule);
    if (startEnd.second) {
        result.set("bodyStartOffset", startEnd.first);
        result.set("bodyEndOffset", startEnd.second);
    }
    if (parentStyleSheet) {
        ScriptObject parentStyleSheetValue = m_frontend->newScriptObject();
        result.set("parentStyleSheet", parentStyleSheetValue);
        parentStyleSheetValue.set("href", parentStyleSheet->href());
    }
    bool isUserAgent = parentStyleSheet && !parentStyleSheet->ownerNode() && parentStyleSheet->href().isEmpty();
    bool isUser = parentStyleSheet && parentStyleSheet->ownerNode() && parentStyleSheet->ownerNode()->nodeName() == "#document";
    result.set("isUserAgent", isUserAgent);
    result.set("isUser", isUser);
    result.set("isViaInspector", rule->parentStyleSheet() == cssStore()->inspectorStyleSheet(ownerDocument, false, -1));

    // Bind editable scripts only.
    bool bind = !isUserAgent && !isUser;
    result.set("style", buildObjectForStyle(rule->style(), bind));

    if (bind)
        result.set("id", cssStore()->bindRule(rule));
    return result;
}

Vector<String> InspectorDOMAgent::longhandProperties(CSSStyleDeclaration* style, const String& shorthandProperty)
{
    Vector<String> properties;
    HashSet<String> foundProperties;

    for (unsigned i = 0; i < style->length(); ++i) {
        String individualProperty = style->item(i);
        if (foundProperties.contains(individualProperty) || style->getPropertyShorthand(individualProperty) != shorthandProperty)
            continue;
        foundProperties.add(individualProperty);
        properties.append(individualProperty);
    }

    return properties;
}

String InspectorDOMAgent::shorthandValue(CSSStyleDeclaration* style, const String& shorthandProperty)
{
    String value = style->getPropertyValue(shorthandProperty);
    if (value.isEmpty()) {
        // Some shorthands (like border) return a null value, so compute a shorthand value.
        // FIXME: remove this when http://bugs.webkit.org/show_bug.cgi?id=15823 is fixed.
        for (unsigned i = 0; i < style->length(); ++i) {
            String individualProperty = style->item(i);
            if (style->getPropertyShorthand(individualProperty) != shorthandProperty)
                continue;
            if (style->isPropertyImplicit(individualProperty))
                continue;
            String individualValue = style->getPropertyValue(individualProperty);
            if (individualValue == "initial")
                continue;
            if (value.length())
                value.append(" ");
            value.append(individualValue);
        }
    }
    return value;
}

String InspectorDOMAgent::shorthandPriority(CSSStyleDeclaration* style, const String& shorthandProperty)
{
    String priority = style->getPropertyPriority(shorthandProperty);
    if (priority.isEmpty()) {
        for (unsigned i = 0; i < style->length(); ++i) {
            String individualProperty = style->item(i);
            if (style->getPropertyShorthand(individualProperty) != shorthandProperty)
                continue;
            priority = style->getPropertyPriority(individualProperty);
            break;
        }
    }
    return priority;
}

bool InspectorDOMAgent::ruleAffectsNode(CSSStyleRule* rule, Node* node)
{
    if (!node)
        return false;
    ExceptionCode ec = 0;
    RefPtr<NodeList> nodes = node->ownerDocument()->querySelectorAll(rule->selectorText(), ec);
    if (ec)
        return false;
    for (unsigned i = 0; i < nodes->length(); ++i) {
        if (nodes->item(i) == node)
            return true;
    }
    return false;
}

ScriptArray InspectorDOMAgent::toArray(const Vector<String>& data)
{
    ScriptArray result = m_frontend->newScriptArray();
    for (unsigned i = 0; i < data.size(); ++i)
        result.set(i, data[i]);
    return result;
}

} // namespace WebCore

#endif // ENABLE(INSPECTOR)
