/*
    Copyright (C) 2004, 2005, 2006 Nikolas Zimmermann <wildfox@kde.org>
                  2004, 2005 Rob Buis <buis@kde.org>
                  2005 Eric Seidel <eric@webkit.org>
                  2010 Zoltan Herczeg <zherczeg@webkit.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    aint with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef FloatPoint3D_h
#define FloatPoint3D_h

#include "FloatPoint.h"

#if PLATFORM(OLYMPIA)
#include "PlatformString.h"
#include "CString.h"
#endif

namespace WebCore {

class FloatPoint3D {
public:
    FloatPoint3D()
        : m_x(0)
        , m_y(0)
        , m_z(0)
    {
    }

    FloatPoint3D(float x, float y, float z)
        : m_x(x)
        , m_y(y)
        , m_z(z)
    {
    }

    FloatPoint3D(const FloatPoint& p)
        : m_x(p.x())
        , m_y(p.y())
        , m_z(0)
    {
    }

    FloatPoint3D(const FloatPoint3D& p)
        : m_x(p.x())
        , m_y(p.y())
        , m_z(p.z())
    {
    }

    float x() const { return m_x; }
    void setX(float x) { m_x = x; }

    float y() const { return m_y; }
    void setY(float y) { m_y = y; }

    float z() const { return m_z; }
    void setZ(float z) { m_z = z; }

    void normalize();

#if PLATFORM(OLYMPIA)
    String toString() const
    {
        return String::format("(%.2f,%.2f,%.2f)", m_x, m_y, m_z);
    }
#endif

private:
    float m_x;
    float m_y;
    float m_z;
};

inline bool operator==(const FloatPoint3D& a, const FloatPoint3D& b)
{
    return a.x() == b.x() && a.y() == b.y() && a.z() == b.z();
}

inline bool operator!=(const FloatPoint3D& a, const FloatPoint3D& b)
{
    return a.x() != b.x() || a.y() != b.y() || a.z() != b.z();
}

inline float operator*(const FloatPoint3D& a, const FloatPoint3D& b)
{
    // dot product
    return a.x() * b.x() + a.y() * b.y() + a.z() * b.z();
}

} // namespace WebCore

#endif // FloatPoint3D_h
