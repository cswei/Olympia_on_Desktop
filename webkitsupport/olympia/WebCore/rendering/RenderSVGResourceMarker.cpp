/*
 * Copyright (C) 2004, 2005, 2007, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 *               2004, 2005, 2006, 2007, 2008 Rob Buis <buis@kde.org>
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
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
 *
 */

#include "config.h"
#if ENABLE(SVG)
#include "RenderSVGResourceMarker.h"

#include "GraphicsContext.h"
#include "RenderSVGContainer.h"
#include "SVGElement.h"
#include "SVGMarkerElement.h"
#include "SVGRenderSupport.h"
#include "SVGStyledElement.h"
#include "SVGStyledTransformableElement.h"

namespace WebCore {

RenderSVGResourceType RenderSVGResourceMarker::s_resourceType = MarkerResourceType;

RenderSVGResourceMarker::RenderSVGResourceMarker(SVGMarkerElement* node)
    : RenderSVGResourceContainer(node)
{
}

RenderSVGResourceMarker::~RenderSVGResourceMarker()
{
    m_marker.clear();
}

void RenderSVGResourceMarker::layout()
{
    // RenderSVGHiddenContainer overwrites layout(). We need the
    // layouting of RenderSVGContainer for calculating  local
    // transformations and repaint.
    RenderSVGContainer::layout();
}

void RenderSVGResourceMarker::addClient(const RenderObject* object)
{
    m_marker.add(object);
}

void RenderSVGResourceMarker::invalidateClients()
{
    const HashSet<const RenderObject*>::const_iterator end = m_marker.end();
    for (HashSet<const RenderObject*>::const_iterator it = m_marker.begin(); it != end; ++it) {
        RenderObject* renderer = const_cast<RenderObject*>(*it);
        renderer->setNeedsBoundariesUpdate();
        renderer->setNeedsLayout(true);
    }

    m_marker.clear();
}

void RenderSVGResourceMarker::invalidateClient(RenderObject* object)
{
    ASSERT(object);

    // FIXME: The HashSet should always contain the object on calling invalidateClient. A race condition
    // during the parsing can causes a call of invalidateClient right before the call of applyResource.
    // We return earlier for the moment. This bug should be fixed in:
    // https://bugs.webkit.org/show_bug.cgi?id=35181
    if (!m_marker.contains(object))
        return;

    m_marker.remove(object);
    markForLayoutAndResourceInvalidation(object);
}

void RenderSVGResourceMarker::applyViewportClip(PaintInfo& paintInfo)
{
    if (SVGRenderBase::isOverflowHidden(this))
        paintInfo.context->clip(m_viewport);
}

FloatRect RenderSVGResourceMarker::markerBoundaries(const AffineTransform& markerTransformation) const
{
    FloatRect coordinates = RenderSVGContainer::repaintRectInLocalCoordinates();

    // Map repaint rect into parent coordinate space, in which the marker boundaries have to be evaluated
    coordinates = localToParentTransform().mapRect(coordinates);

    return markerTransformation.mapRect(coordinates);
}

const AffineTransform& RenderSVGResourceMarker::localToParentTransform() const
{
    AffineTransform viewportTranslation(viewportTransform());
    m_localToParentTransform = viewportTranslation.translateRight(m_viewport.x(), m_viewport.y());
    return m_localToParentTransform;
    // If this class were ever given a localTransform(), then the above would read:
    // return viewportTransform() * localTransform() * viewportTranslation;
}

FloatPoint RenderSVGResourceMarker::referencePoint() const
{
    SVGMarkerElement* marker = static_cast<SVGMarkerElement*>(node());
    ASSERT(marker);

    return FloatPoint(marker->refX().value(marker), marker->refY().value(marker));
}

float RenderSVGResourceMarker::angle() const
{
    SVGMarkerElement* marker = static_cast<SVGMarkerElement*>(node());
    ASSERT(marker);

    float angle = -1;
    if (marker->orientType() == SVGMarkerElement::SVG_MARKER_ORIENT_ANGLE)
        angle = marker->orientAngle().value();

    return angle;
}

AffineTransform RenderSVGResourceMarker::markerTransformation(const FloatPoint& origin, float autoAngle, float strokeWidth) const
{
    SVGMarkerElement* marker = static_cast<SVGMarkerElement*>(node());
    ASSERT(marker);

    float markerAngle = angle();
    bool useStrokeWidth = (marker->markerUnits() == SVGMarkerElement::SVG_MARKERUNITS_STROKEWIDTH);

    AffineTransform transform;
    transform.translate(origin.x(), origin.y());
    transform.rotate(markerAngle == -1 ? autoAngle : markerAngle);
    transform = markerContentTransformation(transform, referencePoint(), useStrokeWidth ? strokeWidth : -1);
    return transform;
}

void RenderSVGResourceMarker::draw(RenderObject::PaintInfo& paintInfo, const AffineTransform& transform)
{
    DEFINE_STATIC_LOCAL(HashSet<RenderSVGResourceMarker*>, currentlyDrawingMarkers, ());

    // avoid drawing circular marker references
    if (currentlyDrawingMarkers.contains(this))
        return;

    currentlyDrawingMarkers.add(this);
    RenderObject::PaintInfo info(paintInfo);
    info.context->save();
    applyTransformToPaintInfo(info, transform);
    RenderSVGContainer::paint(info, 0, 0);
    info.context->restore();

    currentlyDrawingMarkers.remove(this);
}

AffineTransform RenderSVGResourceMarker::markerContentTransformation(const AffineTransform& contentTransformation, const FloatPoint& origin, float strokeWidth) const
{
    // The 'origin' coordinate maps to SVGs refX/refY, given in coordinates relative to the viewport established by the marker
    FloatPoint mappedOrigin = viewportTransform().mapPoint(origin);

    AffineTransform transformation = contentTransformation;
    if (strokeWidth != -1)
        transformation.scaleNonUniform(strokeWidth, strokeWidth);

    transformation.translate(-mappedOrigin.x(), -mappedOrigin.y());
    return transformation;
}

AffineTransform RenderSVGResourceMarker::viewportTransform() const
{
    SVGMarkerElement* marker = static_cast<SVGMarkerElement*>(node());
    ASSERT(marker);

    return marker->viewBoxToViewTransform(m_viewport.width(), m_viewport.height());
}

void RenderSVGResourceMarker::calcViewport()
{
    if (!selfNeedsLayout())
        return;

    SVGMarkerElement* marker = static_cast<SVGMarkerElement*>(node());
    ASSERT(marker);

    float w = marker->markerWidth().value(marker);
    float h = marker->markerHeight().value(marker);
    m_viewport = FloatRect(0, 0, w, h);
}

}

#endif // ENABLE(SVG)
