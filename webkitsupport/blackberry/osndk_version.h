/*****************************************************************************
 * Copyright (c) 2009, Research In Motion, Ltd.
 *
 * Filename:    osndk_version.h
 *
 * Description: OS NDK ABI version control
 *
 * See Athens Code and Header Locations:
 * http://confluence/display/OS/Code+and+Header+Locations
 ****************************************************************************/
#ifndef __NDK_ABI_VERSION_H__
#define __NDK_ABI_VERSION_H__
/**
 * The NDK Application Binary Interface (ABI) refers to the run-time
 * interface between native user processes and the Nessus OS.
 *
 * The ABI versioning control covers the following areas:
 *  1) Loader/object file format
 *  2) System Application Programming Interface (API) calls
 *  3) Run-time linking and loading
 *
 * Whenever an incompatibility is introduced in the above categories, then the
 * major version must be incremented.
 */

/**
 * OS NDK ABI version tuple values
 */
#define NDK_ABI_VERSION_MAJOR    0 // NDK ABI major version, incremented when incompatibilities are introduced
#define NDK_ABI_VERSION_MINOR    1 // NDK ABI minor version, incremented on compatible functional changes

#if (NDK_ABI_VERSION_MAJOR > 65535) || (NDK_ABI_VERSION_MINOR > 65535)
    #error "Invalid NDK_ABI_VERSION tuple"
#endif

#define NDK_ABI_VERSION ((NDK_ABI_VERSION_MAJOR) << 16 | (NDK_ABI_VERSION_MINOR))


#endif // __NDK_ABI_VERSION_H__

/**
 * @file
 *
 * OS NDK ABI version control.
 *
 * The documentation for the OS NDK ABI versioning scheme can be found here:
 *
 * <a href=http://confluence/display/OS/Code+and+Header+Locations</a>
 *
 * <h3>OS NDK ABI History Summary</h3>
 * <ul>
 * <li> 0.1: Initial development ABI version released
 * </ul>
 */
