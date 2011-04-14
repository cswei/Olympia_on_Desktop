/*
 * This file is part of the WebKit project.
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)
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
 *
 */

#include "config.h"
#include "PlatformTouchEvent.h"
#include "OlympiaPlatformInputEvents.h"

#if ENABLE(TOUCH_EVENTS)

namespace WebCore {

PlatformTouchEvent::PlatformTouchEvent(Olympia::Platform::TouchEvent* event)
    : m_ctrlKey(0)
    , m_altKey(event->m_altKey)
    , m_shiftKey(event->m_shiftKey)
    , m_metaKey(0)
{
    switch (event->m_type) {
        case Olympia::Platform::TouchEvent::TouchStart:
            m_type = TouchStart;
            break;
        case Olympia::Platform::TouchEvent::TouchMove:
            m_type = TouchMove;
            break;
        case Olympia::Platform::TouchEvent::TouchEnd:
            m_type = TouchEnd;
            break;
        case Olympia::Platform::TouchEvent::TouchCancel:
            m_type = TouchCancel;
            break;
    }

    for (int i = 0; i < event->m_points.size(); ++i)
        m_touchPoints.append(PlatformTouchPoint(event->m_points[i]));
}

}

#endif
