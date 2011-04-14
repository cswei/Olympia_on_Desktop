#!/usr/bin/perl -w
#
# Copyright (C) Research in Motion Limited 2010. All Rights Reserved.
# Copyright (C) 2010 Chris Jerdonek (chris.jerdonek@gmail.com)
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Apple Computer, Inc. ("Apple") nor the names of
# its contributors may be used to endorse or promote products derived
# from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# Unit tests of parseSvnPropertyValue().

use strict;
use warnings;

use Test::More;
use VCSUtils;

my @testCaseHashRefs = (
{
    # New test
    diffName => "singe-line '+' change",
    inputText => <<'END',
   + *
END
    expectedReturn => ["*", undef],
    expectedNextLine => undef,
},
{
    # New test
    diffName => "single-line '-' change",
    inputText => <<'END',
   - *
END
    expectedReturn => ["*", undef],
    expectedNextLine => undef,
},
{
    # New test
    diffName => "single-line '-' change followed by empty line",
    inputText => <<'END',
   - *

END
    expectedReturn => ["*", "\n"],
    expectedNextLine => undef,
},
{
    # New test
    diffName => "single-line '-' change followed by the next property",
    inputText => <<'END',
   - *
Deleted: svn:executable
END
    expectedReturn => ["*", "Deleted: svn:executable\n"],
    expectedNextLine => undef,
},
{
    # New test
    diffName => "multi-line '+' change and start of binary patch",
    inputText => <<'END',
   + A
long sentence that spans
multiple lines.

Q1dTBx0AAAB42itg4GlgYJjGwMDDyODMxMDw34GBgQEAJPQDJA==
END
    expectedReturn => ["A\nlong sentence that spans\nmultiple lines.", "\n"],
    expectedNextLine => "Q1dTBx0AAAB42itg4GlgYJjGwMDDyODMxMDw34GBgQEAJPQDJA==\n",
},
{
    # New test
    diffName => "multi-line '-' change followed by '+' single-line change",
    inputText => <<'END',
   - A
long sentence that spans
multiple lines.
   + A single-line.
END
    expectedReturn => ["A\nlong sentence that spans\nmultiple lines.", "   + A single-line.\n"],
    expectedNextLine => undef,
},
{
    # New test
    diffName => "multi-line '-' change followed by the next property",
    inputText => <<'END',
   - A
long sentence that spans
multiple lines.
Added: svn:executable
END
    expectedReturn => ["A\nlong sentence that spans\nmultiple lines.", "Added: svn:executable\n"],
    expectedNextLine => undef,
},
{
    # New test
    diffName => "multi-line '-' change followed by '+' multi-line change",
    inputText => <<'END',
   - A
long sentence that spans
multiple lines.
   + Another
long sentence that spans
multiple lines.
END
    expectedReturn => ["A\nlong sentence that spans\nmultiple lines.", "   + Another\n"],
    expectedNextLine => "long sentence that spans\n",
},
);

my $testCasesCount = @testCaseHashRefs;
plan(tests => 2 * $testCasesCount); # Total number of assertions.

foreach my $testCase (@testCaseHashRefs) {
    my $testNameStart = "parseSvnPropertyValue(): $testCase->{diffName}: comparing";

    my $fileHandle;
    open($fileHandle, "<", \$testCase->{inputText});
    my $line = <$fileHandle>;

    my @got = VCSUtils::parseSvnPropertyValue($fileHandle, $line);
    my $expectedReturn = $testCase->{expectedReturn};

    is_deeply(\@got, $expectedReturn, "$testNameStart return value.");

    my $gotNextLine = <$fileHandle>;
    is($gotNextLine, $testCase->{expectedNextLine},  "$testNameStart next read line.");
}
