/*
    Copyright (C) 2004, 2005, 2006, 2008 Nikolas Zimmermann <zimmermann@kde.org>
                  2004, 2005, 2006, 2007 Rob Buis <buis@kde.org>
                  2008 Eric Seidel <eric@webkit.org>
                  2008 Dirk Schulze <krit@webkit.org>
    Copyright (C) Research In Motion Limited 2010. All rights reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"

#if ENABLE(SVG)
#include "SVGLinearGradientElement.h"

#include "Attribute.h"
#include "Document.h"
#include "FloatPoint.h"
#include "LinearGradientAttributes.h"
#include "RenderSVGResourceLinearGradient.h"
#include "SVGLength.h"
#include "SVGNames.h"
#include "SVGTransform.h"
#include "SVGTransformList.h"
#include "SVGUnitTypes.h"

namespace WebCore {

SVGLinearGradientElement::SVGLinearGradientElement(const QualifiedName& tagName, Document* doc)
    : SVGGradientElement(tagName, doc)
    , m_x1(LengthModeWidth)
    , m_y1(LengthModeHeight)
    , m_x2(LengthModeWidth, "100%")
    , m_y2(LengthModeHeight)
{
    // Spec: If the x2 attribute is not specified, the effect is as if a value of "100%" were specified.
}

SVGLinearGradientElement::~SVGLinearGradientElement()
{
}

void SVGLinearGradientElement::parseMappedAttribute(Attribute* attr)
{
    if (attr->name() == SVGNames::x1Attr)
        setX1BaseValue(SVGLength(LengthModeWidth, attr->value()));
    else if (attr->name() == SVGNames::y1Attr)
        setY1BaseValue(SVGLength(LengthModeHeight, attr->value()));
    else if (attr->name() == SVGNames::x2Attr)
        setX2BaseValue(SVGLength(LengthModeWidth, attr->value()));
    else if (attr->name() == SVGNames::y2Attr)
        setY2BaseValue(SVGLength(LengthModeHeight, attr->value()));
    else
        SVGGradientElement::parseMappedAttribute(attr);
}

void SVGLinearGradientElement::svgAttributeChanged(const QualifiedName& attrName)
{
    SVGGradientElement::svgAttributeChanged(attrName);

    if (attrName == SVGNames::x1Attr
        || attrName == SVGNames::y1Attr
        || attrName == SVGNames::x2Attr
        || attrName == SVGNames::y2Attr)
        invalidateResourceClients();
}

void SVGLinearGradientElement::synchronizeProperty(const QualifiedName& attrName)
{
    SVGGradientElement::synchronizeProperty(attrName);

    if (attrName == anyQName()) {
        synchronizeX1();
        synchronizeY1();
        synchronizeX2();
        synchronizeY2();
        return;
    }

    if (attrName == SVGNames::x1Attr)
        synchronizeX1();
    else if (attrName == SVGNames::y1Attr)
        synchronizeY1();
    else if (attrName == SVGNames::x2Attr)
        synchronizeX2();
    else if (attrName == SVGNames::y2Attr)
        synchronizeY2();
}

RenderObject* SVGLinearGradientElement::createRenderer(RenderArena* arena, RenderStyle*)
{
    return new (arena) RenderSVGResourceLinearGradient(this);
}

LinearGradientAttributes SVGLinearGradientElement::collectGradientProperties()
{
    LinearGradientAttributes attributes;
    HashSet<SVGGradientElement*> processedGradients;

    bool isLinear = true;
    SVGGradientElement* current = this;

    while (current) {
        if (!attributes.hasSpreadMethod() && current->hasAttribute(SVGNames::spreadMethodAttr))
            attributes.setSpreadMethod((GradientSpreadMethod) current->spreadMethod());

        if (!attributes.hasBoundingBoxMode() && current->hasAttribute(SVGNames::gradientUnitsAttr))
            attributes.setBoundingBoxMode(current->gradientUnits() == SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX);

        if (!attributes.hasGradientTransform() && current->hasAttribute(SVGNames::gradientTransformAttr))
            attributes.setGradientTransform(current->gradientTransform()->consolidate().matrix());

        if (!attributes.hasStops()) {
            const Vector<Gradient::ColorStop>& stops(current->buildStops());
            if (!stops.isEmpty())
                attributes.setStops(stops);
        }

        if (isLinear) {
            SVGLinearGradientElement* linear = static_cast<SVGLinearGradientElement*>(current);

            if (!attributes.hasX1() && current->hasAttribute(SVGNames::x1Attr))
                attributes.setX1(linear->x1());

            if (!attributes.hasY1() && current->hasAttribute(SVGNames::y1Attr))
                attributes.setY1(linear->y1());

            if (!attributes.hasX2() && current->hasAttribute(SVGNames::x2Attr))
                attributes.setX2(linear->x2());

            if (!attributes.hasY2() && current->hasAttribute(SVGNames::y2Attr))
                attributes.setY2(linear->y2());
        }

        processedGradients.add(current);

        // Respect xlink:href, take attributes from referenced element
        Node* refNode = ownerDocument()->getElementById(SVGURIReference::getTarget(current->href()));
        if (refNode && (refNode->hasTagName(SVGNames::linearGradientTag) || refNode->hasTagName(SVGNames::radialGradientTag))) {
            current = static_cast<SVGGradientElement*>(refNode);

            // Cycle detection
            if (processedGradients.contains(current))
                return LinearGradientAttributes();

            isLinear = current->hasTagName(SVGNames::linearGradientTag);
        } else
            current = 0;
    }

    return attributes;
}

void SVGLinearGradientElement::calculateStartEndPoints(const LinearGradientAttributes& attributes, FloatPoint& startPoint, FloatPoint& endPoint)
{
    // Determine gradient start/end points
    if (attributes.boundingBoxMode()) {
        startPoint = FloatPoint(attributes.x1().valueAsPercentage(), attributes.y1().valueAsPercentage());
        endPoint = FloatPoint(attributes.x2().valueAsPercentage(), attributes.y2().valueAsPercentage());
    } else {
        startPoint = FloatPoint(attributes.x1().value(this), attributes.y1().value(this));
        endPoint = FloatPoint(attributes.x2().value(this), attributes.y2().value(this));
    }
}

}

#endif // ENABLE(SVG)
