/*
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 */

#ifndef OlympiaPlatformKeyboardEvent_h
#define OlympiaPlatformKeyboardEvent_h

namespace Olympia {
namespace Platform {
class KeyboardEvent {
    public:
        enum Type { KeyDown, KeyUp, KeyChar };
        KeyboardEvent(const unsigned short character, bool shiftDown, bool keyDown = true)
            : m_character(character),
              m_shiftActive(shiftDown),
              m_keyDown(keyDown)
            {}

        unsigned short character() { return m_character; }
        bool shiftActive() { return m_shiftActive; }
        bool keyDown() { return m_keyDown; }
        void setKeyDown(bool down) { m_keyDown = down; }

    private:
        const unsigned short m_character;
        bool m_shiftActive;
        bool m_keyDown;
};
}
}

#endif // OlympiaPlatformKeyboardEvent_h
