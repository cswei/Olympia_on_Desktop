/*
 * Copyright (C) Research In Motion, Limited 2009. All rights reserved.
 */

#ifndef OlympiaPlatformIDN_h
#define OlympiaPlatformIDN_h

namespace Olympia {
namespace Platform {

// Return the actual result length
unsigned idnToAscii(const unsigned short* input, unsigned inputLength, char* output, unsigned outputCapacity);

} // namespace Platform
} // namespace Olympia

#endif // OlympiaPlatformIDN_h