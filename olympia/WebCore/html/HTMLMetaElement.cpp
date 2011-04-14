/**
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003 Apple Computer, Inc.
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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

#include "config.h"
#include "HTMLMetaElement.h"

#include "Attribute.h"
#include "Document.h"
#include "HTMLNames.h"

namespace WebCore {

using namespace HTMLNames;

HTMLMetaElement::HTMLMetaElement(const QualifiedName& tagName, Document* doc)
    : HTMLElement(tagName, doc)
{
    ASSERT(hasTagName(metaTag));
}

HTMLMetaElement::~HTMLMetaElement()
{
}

void HTMLMetaElement::parseMappedAttribute(Attribute* attr)
{
    if (attr->name() == http_equivAttr) {
        m_equiv = attr->value();
        process();
    } else if (attr->name() == contentAttr) {
        m_content = attr->value();
        process();
    } else if (attr->name() == nameAttr) {
        // Do nothing.
    } else
        HTMLElement::parseMappedAttribute(attr);
}

void HTMLMetaElement::insertedIntoDocument()
{
    HTMLElement::insertedIntoDocument();
    process();
}

void HTMLMetaElement::process()
{
    if (!inDocument() || m_content.isNull())
        return;

    if (equalIgnoringCase(name(), "viewport"))
        document()->processViewport(m_content);

#if PLATFORM(OLYMPIA)
    if (equalIgnoringCase(name(), "format-detection"))
        document()->processFormatDetection(m_content);
    else if (equalIgnoringCase(name(), "HandheldFriendly"))
        document()->processHandheldFriendly(m_content);
#endif

    // Get the document to process the tag, but only if we're actually part of DOM tree (changing a meta tag while
    // it's not in the tree shouldn't have any effect on the document)
    if (!m_equiv.isNull())
        document()->processHttpEquiv(m_equiv, m_content);
}

String HTMLMetaElement::content() const
{
    return getAttribute(contentAttr);
}

void HTMLMetaElement::setContent(const String& value)
{
    setAttribute(contentAttr, value);
}

String HTMLMetaElement::httpEquiv() const
{
    return getAttribute(http_equivAttr);
}

void HTMLMetaElement::setHttpEquiv(const String& value)
{
    setAttribute(http_equivAttr, value);
}

String HTMLMetaElement::name() const
{
    return getAttribute(nameAttr);
}

void HTMLMetaElement::setName(const String& value)
{
    setAttribute(nameAttr, value);
}

String HTMLMetaElement::scheme() const
{
    return getAttribute(schemeAttr);
}

void HTMLMetaElement::setScheme(const String &value)
{
    setAttribute(schemeAttr, value);
}

}
