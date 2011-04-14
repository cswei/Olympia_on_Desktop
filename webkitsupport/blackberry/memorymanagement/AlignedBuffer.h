/*
 * Copyright (C) Research In Motion, Limited 2010. All rights reserved.
 */

#ifndef AlignedBuffer_h
#define AlignedBuffer_h

namespace Olympia {
namespace Platform {

    /**
     * A utility class useful in writing memory managers.
     * Can be used to allocate aligned space before the memory manager is set up.
     */
    template<unsigned size> union AlignedBuffer {
        char data[size];
        double makeItAlign;
    };

} // namespace Platform
} // namespace Olympia

#endif // AlignedBuffer_h
