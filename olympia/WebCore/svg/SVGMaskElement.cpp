/*
    Copyright (C) 2004, 2005, 2006, 2008 Nikolas Zimmermann <zimmermann@kde.org>
                  2004, 2005, 2006, 2007 Rob Buis <buis@kde.org>
                  2005 Alexander Kellett <lypanov@kde.org>
                  2009 Dirk Schulze <krit@webkit.org>
    Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.

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
#include "SVGMaskElement.h"

#include "Attribute.h"
#include "CSSStyleSelector.h"
#include "RenderSVGResourceMasker.h"
#include "SVGLength.h"
#include "SVGNames.h"
#include "SVGRenderSupport.h"
#include "SVGUnitTypes.h"

using namespace std;

namespace WebCore {

SVGMaskElement::SVGMaskElement(const QualifiedName& tagName, Document* doc)
    : SVGStyledLocatableElement(tagName, doc)
    , SVGURIReference()
    , SVGTests()
    , SVGLangSpace()
    , SVGExternalResourcesRequired()
    , m_maskUnits(SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX)
    , m_maskContentUnits(SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE)
    , m_x(LengthModeWidth, "-10%")
    , m_y(LengthModeHeight, "-10%")
    , m_width(LengthModeWidth, "120%")
    , m_height(LengthModeHeight, "120%")
{
    // Spec: If the x/y attribute is not specified, the effect is as if a value of "-10%" were specified.
    // Spec: If the width/height attribute is not specified, the effect is as if a value of "120%" were specified.
}

SVGMaskElement::~SVGMaskElement()
{
}

void SVGMaskElement::parseMappedAttribute(Attribute* attr)
{
    if (attr->name() == SVGNames::maskUnitsAttr) {
        if (attr->value() == "userSpaceOnUse")
            setMaskUnitsBaseValue(SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE);
        else if (attr->value() == "objectBoundingBox")
            setMaskUnitsBaseValue(SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX);
    } else if (attr->name() == SVGNames::maskContentUnitsAttr) {
        if (attr->value() == "userSpaceOnUse")
            setMaskContentUnitsBaseValue(SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE);
        else if (attr->value() == "objectBoundingBox")
            setMaskContentUnitsBaseValue(SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX);
    } else if (attr->name() == SVGNames::xAttr)
        setXBaseValue(SVGLength(LengthModeWidth, attr->value()));
    else if (attr->name() == SVGNames::yAttr)
        setYBaseValue(SVGLength(LengthModeHeight, attr->value()));
    else if (attr->name() == SVGNames::widthAttr)
        setWidthBaseValue(SVGLength(LengthModeWidth, attr->value()));
    else if (attr->name() == SVGNames::heightAttr)
        setHeightBaseValue(SVGLength(LengthModeHeight, attr->value()));
    else {
        if (SVGURIReference::parseMappedAttribute(attr))
            return;
        if (SVGTests::parseMappedAttribute(attr))
            return;
        if (SVGLangSpace::parseMappedAttribute(attr))
            return;
        if (SVGExternalResourcesRequired::parseMappedAttribute(attr))
            return;
        SVGStyledElement::parseMappedAttribute(attr);
    }
}

void SVGMaskElement::svgAttributeChanged(const QualifiedName& attrName)
{
    SVGStyledElement::svgAttributeChanged(attrName);

    if (attrName == SVGNames::maskUnitsAttr || attrName == SVGNames::maskContentUnitsAttr ||
        attrName == SVGNames::xAttr || attrName == SVGNames::yAttr ||
        attrName == SVGNames::widthAttr || attrName == SVGNames::heightAttr ||
        SVGURIReference::isKnownAttribute(attrName) ||
        SVGTests::isKnownAttribute(attrName) ||
        SVGLangSpace::isKnownAttribute(attrName) ||
        SVGExternalResourcesRequired::isKnownAttribute(attrName) ||
        SVGStyledElement::isKnownAttribute(attrName))
        invalidateResourceClients();
}

void SVGMaskElement::synchronizeProperty(const QualifiedName& attrName)
{
    SVGStyledElement::synchronizeProperty(attrName);

    if (attrName == anyQName()) {
        synchronizeMaskUnits();
        synchronizeMaskContentUnits();
        synchronizeX();
        synchronizeY();
        synchronizeExternalResourcesRequired();
        synchronizeHref();
        return;
    }

    if (attrName == SVGNames::maskUnitsAttr)
        synchronizeMaskUnits();
    else if (attrName == SVGNames::maskContentUnitsAttr)
        synchronizeMaskContentUnits();
    else if (attrName == SVGNames::xAttr)
        synchronizeX();
    else if (attrName == SVGNames::yAttr)
        synchronizeY();
    else if (SVGExternalResourcesRequired::isKnownAttribute(attrName))
        synchronizeExternalResourcesRequired();
    else if (SVGURIReference::isKnownAttribute(attrName))
        synchronizeHref();
}

void SVGMaskElement::childrenChanged(bool changedByParser, Node* beforeChange, Node* afterChange, int childCountDelta)
{
    SVGStyledElement::childrenChanged(changedByParser, beforeChange, afterChange, childCountDelta);

    if (!changedByParser)
        invalidateResourceClients();
}

FloatRect SVGMaskElement::maskBoundingBox(const FloatRect& objectBoundingBox) const
{
    FloatRect maskBBox;
    if (maskUnits() == SVGUnitTypes::SVG_UNIT_TYPE_OBJECTBOUNDINGBOX)
        maskBBox = FloatRect(x().valueAsPercentage() * objectBoundingBox.width() + objectBoundingBox.x(),
                             y().valueAsPercentage() * objectBoundingBox.height() + objectBoundingBox.y(),
                             width().valueAsPercentage() * objectBoundingBox.width(),
                             height().valueAsPercentage() * objectBoundingBox.height());
    else
        maskBBox = FloatRect(x().value(this),
                             y().value(this),
                             width().value(this),
                             height().value(this));

    return maskBBox;
}

RenderObject* SVGMaskElement::createRenderer(RenderArena* arena, RenderStyle*)
{
    return new (arena) RenderSVGResourceMasker(this);
}

}

#endif // ENABLE(SVG)
