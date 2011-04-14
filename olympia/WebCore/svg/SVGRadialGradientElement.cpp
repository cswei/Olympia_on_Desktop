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
#include "SVGRadialGradientElement.h"

#include "Attribute.h"
#include "FloatConversion.h"
#include "FloatPoint.h"
#include "RadialGradientAttributes.h"
#include "RenderSVGResourceRadialGradient.h"
#include "SVGLength.h"
#include "SVGNames.h"
#include "SVGStopElement.h"
#include "SVGTransform.h"
#include "SVGTransformList.h"
#include "SVGUnitTypes.h"

namespace WebCore {

SVGRadialGradientElement::SVGRadialGradientElement(const QualifiedName& tagName, Document* doc)
    : SVGGradientElement(tagName, doc)
    , m_cx(LengthModeWidth, "50%")
    , m_cy(LengthModeHeight, "50%")
    , m_r(LengthModeOther, "50%")
    , m_fx(LengthModeWidth)
    , m_fy(LengthModeHeight)
{
    // Spec: If the cx/cy/r attribute is not specified, the effect is as if a value of "50%" were specified.
}

SVGRadialGradientElement::~SVGRadialGradientElement()
{
}

void SVGRadialGradientElement::parseMappedAttribute(Attribute* attr)
{
    if (attr->name() == SVGNames::cxAttr)
        setCxBaseValue(SVGLength(LengthModeWidth, attr->value()));
    else if (attr->name() == SVGNames::cyAttr)
        setCyBaseValue(SVGLength(LengthModeHeight, attr->value()));
    else if (attr->name() == SVGNames::rAttr) {
        setRBaseValue(SVGLength(LengthModeOther, attr->value()));
        if (rBaseValue().value(this) < 0.0)
            document()->accessSVGExtensions()->reportError("A negative value for radial gradient radius <r> is not allowed");
    } else if (attr->name() == SVGNames::fxAttr)
        setFxBaseValue(SVGLength(LengthModeWidth, attr->value()));
    else if (attr->name() == SVGNames::fyAttr)
        setFyBaseValue(SVGLength(LengthModeHeight, attr->value()));
    else
        SVGGradientElement::parseMappedAttribute(attr);
}

void SVGRadialGradientElement::svgAttributeChanged(const QualifiedName& attrName)
{
    SVGGradientElement::svgAttributeChanged(attrName);

    if (attrName == SVGNames::cxAttr
        || attrName == SVGNames::cyAttr
        || attrName == SVGNames::fxAttr
        || attrName == SVGNames::fyAttr
        || attrName == SVGNames::rAttr)
        invalidateResourceClients();
}

void SVGRadialGradientElement::synchronizeProperty(const QualifiedName& attrName)
{
    SVGGradientElement::synchronizeProperty(attrName);

    if (attrName == anyQName()) {
        synchronizeCx();
        synchronizeCy();
        synchronizeFx();
        synchronizeFy();
        synchronizeR();
        return;
    }

    if (attrName == SVGNames::cxAttr)
        synchronizeCx();
    else if (attrName == SVGNames::cyAttr)
        synchronizeCy();
    else if (attrName == SVGNames::fxAttr)
        synchronizeFx();
    else if (attrName == SVGNames::fyAttr)
        synchronizeFy();
    else if (attrName == SVGNames::rAttr)
        synchronizeR();
}

RenderObject* SVGRadialGradientElement::createRenderer(RenderArena* arena, RenderStyle*)
{
    return new (arena) RenderSVGResourceRadialGradient(this);
}

RadialGradientAttributes SVGRadialGradientElement::collectGradientProperties()
{
    RadialGradientAttributes attributes;
    HashSet<SVGGradientElement*> processedGradients;

    bool isRadial = true;
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

        if (isRadial) {
            SVGRadialGradientElement* radial = static_cast<SVGRadialGradientElement*>(current);

            if (!attributes.hasCx() && current->hasAttribute(SVGNames::cxAttr))
                attributes.setCx(radial->cx());

            if (!attributes.hasCy() && current->hasAttribute(SVGNames::cyAttr))
                attributes.setCy(radial->cy());

            if (!attributes.hasR() && current->hasAttribute(SVGNames::rAttr))
                attributes.setR(radial->r());

            if (!attributes.hasFx() && current->hasAttribute(SVGNames::fxAttr))
                attributes.setFx(radial->fx());

            if (!attributes.hasFy() && current->hasAttribute(SVGNames::fyAttr))
                attributes.setFy(radial->fy());
        }

        processedGradients.add(current);

        // Respect xlink:href, take attributes from referenced element
        Node* refNode = ownerDocument()->getElementById(SVGURIReference::getTarget(current->href()));
        if (refNode && (refNode->hasTagName(SVGNames::radialGradientTag) || refNode->hasTagName(SVGNames::linearGradientTag))) {
            current = static_cast<SVGGradientElement*>(refNode);

            // Cycle detection
            if (processedGradients.contains(current))
                return RadialGradientAttributes();

            isRadial = current->hasTagName(SVGNames::radialGradientTag);
        } else
            current = 0;
    }

    // Handle default values for fx/fy
    if (!attributes.hasFx())
        attributes.setFx(attributes.cx());

    if (!attributes.hasFy())
        attributes.setFy(attributes.cy());

    return attributes;
}

void SVGRadialGradientElement::calculateFocalCenterPointsAndRadius(const RadialGradientAttributes& attributes, FloatPoint& focalPoint, FloatPoint& centerPoint, float& radius)
{
    // Determine gradient focal/center points and radius
    if (attributes.boundingBoxMode()) {
        focalPoint = FloatPoint(attributes.fx().valueAsPercentage(), attributes.fy().valueAsPercentage());
        centerPoint = FloatPoint(attributes.cx().valueAsPercentage(), attributes.cy().valueAsPercentage());
        radius = attributes.r().valueAsPercentage();
    } else {
        focalPoint = FloatPoint(attributes.fx().value(this), attributes.fy().value(this));
        centerPoint = FloatPoint(attributes.cx().value(this), attributes.cy().value(this));
        radius = attributes.r().value(this);
    }

    // Eventually adjust focal points, as described below
    float deltaX = focalPoint.x() - centerPoint.x();
    float deltaY = focalPoint.y() - centerPoint.y();
    float radiusMax = 0.99f * radius;

    // Spec: If (fx, fy) lies outside the circle defined by (cx, cy) and r, set
    // (fx, fy) to the point of intersection of the line through (fx, fy) and the circle.
    // We scale the radius by 0.99 to match the behavior of FireFox.
    if (sqrt(deltaX * deltaX + deltaY * deltaY) > radiusMax) {
        float angle = atan2f(deltaY, deltaX);

        deltaX = cosf(angle) * radiusMax;
        deltaY = sinf(angle) * radiusMax;
        focalPoint = FloatPoint(deltaX + centerPoint.x(), deltaY + centerPoint.y());
    }
}

}

#endif // ENABLE(SVG)
