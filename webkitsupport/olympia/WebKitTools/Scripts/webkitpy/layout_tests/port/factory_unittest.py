# Copyright (C) 2010 Google Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#    * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#    * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
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

import sys
import unittest

import chromium_linux
import chromium_mac
import chromium_win
import dryrun
import factory
import gtk
import mac
import qt
import test
import win


class FactoryTest(unittest.TestCase):
    """Test factory creates proper port object for the target.

    Target is specified by port_name, sys.platform and options.

    """
    # FIXME: The ports themselves should expose what options they require,
    # instead of passing generic "options".

    class WebKitOptions(object):
        """Represents the minimum options for WebKit port."""
        def __init__(self):
            self.pixel_tests = False

    class ChromiumOptions(WebKitOptions):
        """Represents minimum options for Chromium port."""
        def __init__(self):
            FactoryTest.WebKitOptions.__init__(self)
            self.chromium = True

    def setUp(self):
        self.real_sys_platform = sys.platform
        self.webkit_options = FactoryTest.WebKitOptions()
        self.chromium_options = FactoryTest.ChromiumOptions()

    def tearDown(self):
        sys.platform = self.real_sys_platform

    def assert_port(self, port_name, expected_port):
        """Helper assert for port_name.

        Args:
          port_name: port name to get port object.
          expected_port: class of expected port object.

        """
        self.assertTrue(isinstance(factory.get(port_name=port_name),
                                   expected_port))

    def assert_platform_port(self, platform, options, expected_port):
        """Helper assert for platform and options.

        Args:
          platform: sys.platform.
          options: options to get port object.
          expected_port: class of expected port object.

        """
        orig_platform = sys.platform
        sys.platform = platform
        self.assertTrue(isinstance(factory.get(options=options),
                                   expected_port))
        sys.platform = orig_platform

    def test_test(self):
        self.assert_port("test", test.TestPort)

    def test_dryrun(self):
        self.assert_port("dryrun-test", dryrun.DryRunPort)
        self.assert_port("dryrun-mac", dryrun.DryRunPort)

    def test_mac(self):
        self.assert_port("mac", mac.MacPort)
        self.assert_platform_port("darwin", None, mac.MacPort)
        self.assert_platform_port("darwin", self.webkit_options, mac.MacPort)

    def test_win(self):
        self.assert_port("win", win.WinPort)
        self.assert_platform_port("win32", None, win.WinPort)
        self.assert_platform_port("win32", self.webkit_options, win.WinPort)
        self.assert_platform_port("cygwin", None, win.WinPort)
        self.assert_platform_port("cygwin", self.webkit_options, win.WinPort)

    def test_gtk(self):
        self.assert_port("gtk", gtk.GtkPort)

    def test_qt(self):
        self.assert_port("qt", qt.QtPort)

    def test_chromium_mac(self):
        self.assert_port("chromium-mac", chromium_mac.ChromiumMacPort)
        self.assert_platform_port("darwin", self.chromium_options,
                                  chromium_mac.ChromiumMacPort)

    def test_chromium_linux(self):
        self.assert_port("chromium-linux", chromium_linux.ChromiumLinuxPort)
        self.assert_platform_port("linux2", self.chromium_options,
                                  chromium_linux.ChromiumLinuxPort)

    def test_chromium_win(self):
        self.assert_port("chromium-win", chromium_win.ChromiumWinPort)
        self.assert_platform_port("win32", self.chromium_options,
                                  chromium_win.ChromiumWinPort)
        self.assert_platform_port("cygwin", self.chromium_options,
                                  chromium_win.ChromiumWinPort)
