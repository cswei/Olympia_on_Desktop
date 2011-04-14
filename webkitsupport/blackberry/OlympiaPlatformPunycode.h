/*
 * Copyright (C) Research In Motion, Limited 2009. All rights reserved.
 */


/*
punycode.c from RFC 3492
http://www.nicemice.net/idn/
Adam M. Costello
http://www.nicemice.net/amc/

This is ANSI C code (C89) implementing Punycode (RFC 3492).

From the RFC: ftp://ftp.rfc-editor.org/in-notes/rfc3492.txt

B. Disclaimer and license

   Regarding this entire document or any portion of it (including the
   pseudocode and C code), the author makes no guarantees and is not
   responsible for any damage resulting from its use.  The author grants
   irrevocable permission to anyone to use, modify, and distribute it in
   any way that does not diminish the rights of anyone else to use,
   modify, and distribute it, provided that redistributed derivative
   works do not contain misleading author or version information.
   Derivative works need not be licensed under similar terms.
*/

/************************************************************/
/* Public interface (would normally go in its own .h file): */

#ifndef OlympiaPlatformPunycode_h
#define OlympiaPlatformPunycode_h

#include <limits.h>

namespace Olympia {
namespace Platform {

enum PunycodeStatus {
    PunycodeSuccess,
    PunycodeBigOutput,  /* Output would exceed the space provided. */
    PunycodeOverflow     /* Input needs wider integers to process.  */
};

PunycodeStatus punycodeEncode(unsigned input_length, const unsigned short* input, unsigned* output_length, char* output);

    /* punycode_encode() converts Unicode to Punycode.  The input     */
    /* is represented as an array of Unicode code points (not code    */
    /* units; surrogate pairs are not allowed), and the output        */
    /* will be represented as an array of ASCII code points.  The     */
    /* output string is *not* null-terminated; it will contain        */
    /* zeros if and only if the input contains zeros.  (Of course     */
    /* the caller can leave room for a terminator and add one if      */
    /* needed.)  The input_length is the number of code points in     */
    /* the input.  The output_length is an in/out argument: the       */
    /* caller passes in the maximum number of code points that it     */
    /* can receive, and on successful return it will contain the      */
    /* number of code points actually output.  The case_flags array   */
    /* holds input_length boolean values, where nonzero suggests that */
    /* the corresponding Unicode character be forced to uppercase     */
    /* after being decoded (if possible), and zero suggests that      */
    /* it be forced to lowercase (if possible).  ASCII code points    */
    /* are encoded literally, except that ASCII letters are forced    */
    /* to uppercase or lowercase according to the corresponding       */
    /* uppercase flags.  If case_flags is a null pointer then ASCII   */
    /* letters are left as they are, and other code points are        */
    /* treated as if their uppercase flags were zero.  The return     */
    /* value can be any of the punycode_status values defined above   */
    /* except punycode_bad_input; if not punycode_success, then       */
    /* output_size and output might contain garbage.                  */

} // namespace Platform
} // namespace Olympia

#endif // OlympiaPlatformPunycode_h
