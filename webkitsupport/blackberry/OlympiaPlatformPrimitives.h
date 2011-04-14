/*
 * Copyright (C) 2010 Research In Motion Limited. http://www.rim.com/
 */

#ifndef OlympiaPlatformPrimitives_h
#define OlympiaPlatformPrimitives_h

namespace Olympia {
namespace Platform {

class IntSize {
public:
    IntSize(int width, int height) :
            m_width(width), m_height(height) {};
    IntSize() : m_width(0), m_height(0) {};

    int width() const { return m_width; }
    int height() const { return m_height; }

private:
    int m_width;
    int m_height;
};

class IntPoint {
public:
    IntPoint(int x, int y) :
            m_x(x), m_y(y) {};
    IntPoint() : m_x(0), m_y(0) {};

    int x() const { return m_x; }
    int y() const { return m_y; }

private:
    int m_x;
    int m_y;
};

class IntRect {
public:
    IntRect(int x, int y, int width, int height) :
            m_location(x, y), m_size(width, height) {};
    IntRect(IntPoint location, IntSize size) :
            m_location(location), m_size(size) {};
    IntRect() : m_location(0, 0), m_size(0, 0) {};

    int x() const { return m_location.x(); }
    int y() const { return m_location.y(); }
    int width() const { return m_size.width(); }
    int height() const { return m_size.height(); }

private:
    IntPoint m_location;
    IntSize m_size;
};

} // Platform
} // Olympia

#endif // OlympiaPlatformPrimitives_h
