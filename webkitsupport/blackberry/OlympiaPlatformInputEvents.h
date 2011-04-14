/*
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 */

#ifndef OlympiaPlatformInputEvents_h
#define OlympiaPlatformInputEvents_h

#include "OlympiaPlatformPrimitives.h"

#include <vector>

namespace Olympia {
namespace Platform {

// NOTE:  Conversion between values in HTMLInputElement and these values is in WebPage.cpp and any changes made
// here will require a corresponding update.

enum OlympiaInputType {
    InputTypeText = 0,
    InputTypePassword,
    InputTypeIsIndex,
    InputTypeSearch,
    InputTypeEmail,
    InputTypeNumber,
    InputTypeTelephone,
    InputTypeURL,
    InputTypeColor,
    InputTypeTextArea
};

// Scroll directions, this maps to WebCore::ScrollDirection defined in ScrollTypes.h.
// Currently used in link-to-link navigation.

enum ScrollDirection { ScrollUp, ScrollDown, ScrollLeft, ScrollRight };


class TouchPoint {
public:
    int m_id;
    enum State { TouchReleased, TouchMoved, TouchPressed, TouchStationary };
    State m_state;
    IntPoint m_screenPos;
    IntPoint m_pos;
};


class TouchEvent {
public:
    enum Type { TouchStart, TouchMove, TouchEnd, TouchCancel };
    enum SingleType { SingleReleased, SingleMoved, SinglePressed, SingleNone };
    Type m_type;
    SingleType m_singleType;
    bool m_altKey;
    bool m_shiftKey;
    std::vector<TouchPoint> m_points;
};


} // namespace Platform
} // namespace Olympia

#endif // OlympiaPlatformInputEvents_h
