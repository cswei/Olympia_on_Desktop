/*
    Copyright (C) 2006 Apple Computer, Inc.
                  2006, 2008 Nikolas Zimmermann <zimmermann@kde.org>

    This file is part of the WebKit project

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

#ifndef SVGDocumentExtensions_h
#define SVGDocumentExtensions_h

#if ENABLE(SVG)
#include "AtomicStringHash.h"
#include "StringImpl.h"
#include <wtf/HashMap.h>
#include <wtf/HashSet.h>
#include <wtf/PassOwnPtr.h>

namespace WebCore {

class Document;
class RenderSVGResourceContainer;
class String;
class SVGStyledElement;
class SVGSMILElement;
class SVGSVGElement;

class SVGDocumentExtensions : public Noncopyable {
public:
    SVGDocumentExtensions(Document*);
    ~SVGDocumentExtensions();
    
    void addTimeContainer(SVGSVGElement*);
    void removeTimeContainer(SVGSVGElement*);

    void addResource(const AtomicString& id, RenderSVGResourceContainer*);
    void removeResource(const AtomicString& id);
    RenderSVGResourceContainer* resourceById(const AtomicString& id) const;

    void startAnimations();
    void pauseAnimations();
    void unpauseAnimations();
    bool sampleAnimationAtTime(const String& elementId, SVGSMILElement*, double time);

    void reportWarning(const String&);
    void reportError(const String&);

private:
    Document* m_doc; // weak reference
    HashSet<SVGSVGElement*> m_timeContainers; // For SVG 1.2 support this will need to be made more general.
    HashMap<AtomicString, RenderSVGResourceContainer*> m_resources;
    HashMap<AtomicString, HashSet<SVGStyledElement*>*> m_pendingResources;

    SVGDocumentExtensions(const SVGDocumentExtensions&);
    SVGDocumentExtensions& operator=(const SVGDocumentExtensions&);

public:
    // This HashMap contains a list of pending resources. Pending resources, are such
    // which are referenced by any object in the SVG document, but do NOT exist yet.
    // For instance, dynamically build gradients / patterns / clippers...
    void addPendingResource(const AtomicString& id, SVGStyledElement*);
    bool isPendingResource(const AtomicString& id) const;
    PassOwnPtr<HashSet<SVGStyledElement*> > removePendingResource(const AtomicString& id);
};

}

#endif
#endif
