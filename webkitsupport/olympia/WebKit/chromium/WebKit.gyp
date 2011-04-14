#
# Copyright (C) 2009 Google Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#         * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#         * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#         * Neither the name of Google Inc. nor the names of its
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
#

{
    'includes': [
        'features.gypi',
        '../../WebKitTools/DumpRenderTree/DumpRenderTree.gypi',
    ],
    'variables': {
        'webkit_target_type': 'static_library',
        'conditions': [
            # Location of the chromium src directory and target type is different
            # if webkit is built inside chromium or as standalone project.
            ['inside_chromium_build==0', {
                # Webkit is being built outside of the full chromium project.
                # e.g. via build-webkit --chromium
                'chromium_src_dir': '../../WebKit/chromium',
            },{
                # WebKit is checked out in src/chromium/third_party/WebKit
                'chromium_src_dir': '../../../..',
            }],
        ],
        'ahem_path': '../../WebKitTools/DumpRenderTree/qt/fonts/AHEM____.TTF',
    },
    'targets': [
        {
            'target_name': 'webkit',
            'type': '<(webkit_target_type)',
            'msvs_guid': '5ECEC9E5-8F23-47B6-93E0-C3B328B3BE65',
            'dependencies': [
                '../../WebCore/WebCore.gyp/WebCore.gyp:webcore',
                '<(chromium_src_dir)/skia/skia.gyp:skia',
                '<(chromium_src_dir)/third_party/npapi/npapi.gyp:npapi',
            ],
            'export_dependent_settings': [
                '<(chromium_src_dir)/skia/skia.gyp:skia',
                '<(chromium_src_dir)/third_party/npapi/npapi.gyp:npapi',
            ],
            'include_dirs': [
                'public',
                'src',
            ],
            'defines': [
                'WEBKIT_IMPLEMENTATION',
            ],
            'sources': [
                'public/gtk/WebInputEventFactory.h',
                'public/linux/WebFontRendering.h',
                'public/linux/WebFontRenderStyle.h',
                'public/linux/WebRenderTheme.h',
                'public/x11/WebScreenInfoFactory.h',
                'public/mac/WebInputEventFactory.h',
                'public/mac/WebScreenInfoFactory.h',
                'public/WebAccessibilityCache.h',
                'public/WebAccessibilityObject.h',
                'public/WebAccessibilityRole.h',
                'public/WebAnimationController.h',
                'public/WebApplicationCacheHost.h',
                'public/WebApplicationCacheHostClient.h',
                'public/WebBindings.h',
                'public/WebCache.h',
                'public/WebCanvas.h',
                'public/WebClipboard.h',
                'public/WebColor.h',
                'public/WebColorName.h',
                'public/WebCommon.h',
                'public/WebCommonWorkerClient.h',
                'public/WebCompositionCommand.h',
                'public/WebConsoleMessage.h',
                'public/WebContextMenuData.h',
                'public/WebCookie.h',
                'public/WebCookieJar.h',
                'public/WebCrossOriginPreflightResultCache.h',
                'public/WebCString.h',
                'public/WebCursorInfo.h',
                'public/WebDOMStringList.h',
                'public/WebData.h',
                'public/WebDatabase.h',
                'public/WebDatabaseObserver.h',
                'public/WebDataSource.h',
                'public/WebDevToolsAgent.h',
                'public/WebDevToolsAgentClient.h',
                'public/WebDevToolsFrontend.h',
                'public/WebDevToolsFrontendClient.h',
                'public/WebDevToolsMessageData.h',
                'public/WebDocument.h',
                'public/WebDragData.h',
                'public/WebEditingAction.h',
                'public/WebElement.h',
                'public/WebEvent.h',
                'public/WebEventListener.h',
                'public/WebFileChooserCompletion.h',
                'public/WebFileChooserParams.h',
                'public/WebFileInfo.h',
                'public/WebFileSystem.h',
                'public/WebFindOptions.h',
                'public/WebFrame.h',
                'public/WebFrameClient.h',
                'public/WebFontCache.h',
                'public/WebFormControlElement.h',
                'public/WebFormElement.h',
                'public/WebGeolocationService.h',
                'public/WebGeolocationServiceBridge.h',
                'public/WebGeolocationServiceMock.h',
                'public/WebGlyphCache.h',
                'public/WebGLES2Context.h',
                'public/WebGraphicsContext3D.h',
                'public/WebHistoryItem.h',
                'public/WebHTTPBody.h',
                'public/WebImage.h',
                'public/WebImageDecoder.h',
                'public/WebIDBCallbacks.h',
                'public/WebIDBDatabase.h',
                'public/WebIDBDatabaseError.h',
                'public/WebIndexedDatabase.h',
                'public/WebInputElement.h',
                'public/WebInputEvent.h',
                'public/WebKit.h',
                'public/WebKitClient.h',
                'public/WebLabelElement.h',
                'public/WebLocalizedString.h',
                'public/WebMediaPlayer.h',
                'public/WebMediaPlayerAction.h',
                'public/WebMediaPlayerClient.h',
                'public/WebMenuItemInfo.h',
                'public/WebMessagePortChannel.h',
                'public/WebMessagePortChannelClient.h',
                'public/WebMimeRegistry.h',
                'public/WebMutationEvent.h',
                'public/WebNavigationType.h',
                'public/WebNode.h',
                'public/WebNodeCollection.h',
                'public/WebNodeList.h',
                'public/WebNonCopyable.h',
                'public/WebNotification.h',
                'public/WebNotificationPresenter.h',
                'public/WebNotificationPermissionCallback.h',
                'public/WebPageSerializer.h',
                'public/WebPageSerializerClient.h',
                'public/WebPasswordAutocompleteListener.h',
                'public/WebPasswordFormData.h',
                'public/WebPlugin.h',
                'public/WebPluginContainer.h',
                'public/WebPluginDocument.h',
                'public/WebPluginListBuilder.h',
                'public/WebPoint.h',
                'public/WebPopupMenu.h',
                'public/WebPopupMenuInfo.h',
                'public/WebPopupType.h',
                'public/WebPrivatePtr.h',
                'public/WebRange.h',
                'public/WebRect.h',
                'public/WebRegularExpression.h',
                'public/WebRuntimeFeatures.h',
                'public/WebScrollbar.h',
                'public/WebScrollbarClient.h',
                'public/WebScreenInfo.h',
                'public/WebScriptController.h',
                'public/WebScriptSource.h',
                'public/WebSearchableFormData.h',
                'public/WebSecurityOrigin.h',
                'public/WebSecurityPolicy.h',
                'public/WebSelectElement.h',
                'public/WebSerializedScriptValue.h',
                'public/WebSettings.h',
                'public/WebSharedWorker.h',
                'public/WebSharedWorkerRepository.h',
                'public/WebSize.h',
                'public/WebSocketStreamError.h',
                'public/WebSocketStreamHandle.h',
                'public/WebSocketStreamHandleClient.h',
                'public/WebStorageArea.h',
                'public/WebStorageEventDispatcher.h',
                'public/WebStorageNamespace.h',
                'public/WebString.h',
                'public/WebTextAffinity.h',
                'public/WebTextCaseSensitivity.h',
                'public/WebTextDirection.h',
                'public/WebThemeEngine.h',
                'public/WebURL.h',
                'public/WebURLError.h',
                'public/WebURLLoader.h',
                'public/WebURLLoaderClient.h',
                'public/WebURLRequest.h',
                'public/WebURLResponse.h',
                'public/WebVector.h',
                'public/WebView.h',
                'public/WebViewClient.h',
                'public/WebWidget.h',
                'public/WebWidgetClient.h',
                'public/WebWorker.h',
                'public/WebWorkerClient.h',
                'public/win/WebInputEventFactory.h',
                'public/win/WebSandboxSupport.h',
                'public/win/WebScreenInfoFactory.h',
                'public/win/WebScreenInfoFactory.h',
                'src/APUAgentDelegate.h',
                'src/ApplicationCacheHost.cpp',
                'src/ApplicationCacheHostInternal.h',
                'src/AssertMatchingEnums.cpp',
                'src/AutocompletePopupMenuClient.cpp',
                'src/AutocompletePopupMenuClient.h',
                'src/AutoFillPopupMenuClient.cpp',
                'src/AutoFillPopupMenuClient.h',
                'src/BackForwardListClientImpl.cpp',
                'src/BackForwardListClientImpl.h',
                'src/BoundObject.cpp',
                'src/BoundObject.h',
                'src/ChromeClientImpl.cpp',
                'src/ChromeClientImpl.h',
                'src/ChromiumBridge.cpp',
                'src/ChromiumCurrentTime.cpp',
                'src/ChromiumThreading.cpp',
                'src/ContextMenuClientImpl.cpp',
                'src/ContextMenuClientImpl.h',
                'src/DatabaseObserver.cpp',
                'src/DebuggerAgent.h',
                'src/DebuggerAgentImpl.cpp',
                'src/DebuggerAgentImpl.h',
                'src/DebuggerAgentManager.cpp',
                'src/DebuggerAgentManager.h',
                'src/DevToolsRPC.h',
                'src/DevToolsRPCJS.h',
                'src/DOMUtilitiesPrivate.cpp',
                'src/DOMUtilitiesPrivate.h',
                'src/DragClientImpl.cpp',
                'src/DragClientImpl.h',
                'src/EditorClientImpl.cpp',
                'src/EditorClientImpl.h',
                'src/EventListenerWrapper.cpp',
                'src/EventListenerWrapper.h',
                'src/FrameLoaderClientImpl.cpp',
                'src/FrameLoaderClientImpl.h',
                'src/GLES2Context.cpp',
                'src/gtk/WebFontInfo.cpp',
                'src/gtk/WebFontInfo.h',
                'src/gtk/WebInputEventFactory.cpp',
                'src/IDBCallbacksProxy.cpp',
                'src/IDBCallbacksProxy.h',
                'src/IDBDatabaseProxy.cpp',
                'src/IDBDatabaseProxy.h',
                'src/IndexedDatabaseProxy.cpp',
                'src/IndexedDatabaseProxy.h',
                'src/InspectorClientImpl.cpp',
                'src/InspectorClientImpl.h',
                'src/InspectorFrontendClientImpl.cpp',
                'src/InspectorFrontendClientImpl.h',
                'src/linux/WebFontRendering.cpp',
                'src/linux/WebFontRenderStyle.cpp',
                'src/linux/WebRenderTheme.cpp',
                'src/x11/WebScreenInfoFactory.cpp',
                'src/mac/WebInputEventFactory.mm',
                'src/mac/WebScreenInfoFactory.mm',
                'src/LocalizedStrings.cpp',
                'src/MediaPlayerPrivateChromium.cpp',
                'src/NotificationPresenterImpl.h',
                'src/NotificationPresenterImpl.cpp',
                'src/PlatformMessagePortChannel.cpp',
                'src/PlatformMessagePortChannel.h',
                'src/ProfilerAgent.h',
                'src/ProfilerAgentImpl.cpp',
                'src/ProfilerAgentImpl.h',
                'src/ResourceHandle.cpp',
                'src/SharedWorkerRepository.cpp',
                'src/SocketStreamHandle.cpp',
                'src/StorageAreaProxy.cpp',
                'src/StorageAreaProxy.h',
                'src/StorageEventDispatcherChromium.cpp',
                'src/StorageEventDispatcherImpl.cpp',
                'src/StorageEventDispatcherImpl.h',
                'src/StorageNamespaceProxy.cpp',
                'src/StorageNamespaceProxy.h',
                'src/SuggestionsPopupMenuClient.cpp',
                'src/SuggestionsPopupMenuClient.h',
                'src/TemporaryGlue.h',
                'src/ToolsAgent.h',
                'src/WebAccessibilityCache.cpp',
                'src/WebAccessibilityCacheImpl.cpp',
                'src/WebAccessibilityCacheImpl.h',
                'src/WebAccessibilityObject.cpp',
                'src/WebAnimationControllerImpl.cpp',
                'src/WebAnimationControllerImpl.h',
                'src/WebBindings.cpp',
                'src/WebCache.cpp',
                'src/WebColor.cpp',
                'src/WebCommon.cpp',
                'src/WebCrossOriginPreflightResultCache.cpp',
                'src/WebCString.cpp',
                'src/WebCursorInfo.cpp',
                'src/WebDOMStringList.cpp',
                'src/WebData.cpp',
                'src/WebDatabase.cpp',
                'src/WebDataSourceImpl.cpp',
                'src/WebDataSourceImpl.h',
                'src/WebDevToolsAgentImpl.cpp',
                'src/WebDevToolsAgentImpl.h',
                'src/WebDevToolsFrontendImpl.cpp',
                'src/WebDevToolsFrontendImpl.h',
                'src/WebDocument.cpp',
                'src/WebDragData.cpp',
                'src/WebElement.cpp',
                'src/WebEntities.cpp',
                'src/WebEntities.h',
                'src/WebEvent.cpp',
                'src/WebEventListener.cpp',
                'src/WebEventListenerPrivate.cpp',
                'src/WebEventListenerPrivate.h',
                'src/WebFileChooserCompletionImpl.cpp',
                'src/WebFileChooserCompletionImpl.h',
                'src/WebFontCache.cpp',
                'src/WebFormControlElement.cpp',
                'src/WebFormElement.cpp',
                'src/WebFrameImpl.cpp',
                'src/WebFrameImpl.h',
                'src/WebGeolocationServiceBridgeImpl.cpp',
                'src/WebGeolocationServiceBridgeImpl.h',
                'src/WebGeolocationServiceMock.cpp',
                'src/WebGlyphCache.cpp',
                'src/WebGraphicsContext3D.cpp',
                'src/WebGraphicsContext3DDefaultImpl.cpp',
                'src/WebGraphicsContext3DDefaultImpl.h',
                'src/WebHistoryItem.cpp',
                'src/WebHTTPBody.cpp',
                'src/WebIDBCallbacksImpl.cpp',
                'src/WebIDBCallbacksImpl.h',
                'src/WebIDBDatabaseError.cpp',
                'src/WebIDBDatabaseImpl.cpp',
                'src/WebIDBDatabaseImpl.h',
                'src/WebImageCG.cpp',
                'src/WebImageDecoder.cpp',
                'src/WebImageSkia.cpp',
                'src/WebIndexedDatabaseImpl.cpp',
                'src/WebIndexedDatabaseImpl.h',
                'src/WebInputElement.cpp',
                'src/WebInputEvent.cpp',
                'src/WebInputEventConversion.cpp',
                'src/WebInputEventConversion.h',
                'src/WebKit.cpp',
                'src/WebLabelElement.cpp',
                'src/WebMediaPlayerClientImpl.cpp',
                'src/WebMediaPlayerClientImpl.h',
                'src/WebMutationEvent.cpp',
                'src/WebNode.cpp',
                'src/WebNodeCollection.cpp',
                'src/WebNodeList.cpp',
                'src/WebNotification.cpp',
                'src/WebPageSerializer.cpp',
                'src/WebPageSerializerImpl.cpp',
                'src/WebPageSerializerImpl.h',
                'src/WebPasswordFormData.cpp',
                'src/WebPasswordFormUtils.cpp',
                'src/WebPasswordFormUtils.h',
                'src/WebPluginContainerImpl.h',
                'src/WebPluginContainerImpl.cpp',
                'src/WebPluginDocument.cpp',
                'src/WebPluginListBuilderImpl.cpp',
                'src/WebPluginListBuilderImpl.h',
                'src/WebPluginLoadObserver.cpp',
                'src/WebPluginLoadObserver.h',
                'src/WebPopupMenuImpl.cpp',
                'src/WebPopupMenuImpl.h',
                'src/WebRange.cpp',
                'src/WebRegularExpression.cpp',
                'src/WebRuntimeFeatures.cpp',
                'src/WebScriptController.cpp',
                'src/WebScrollbarImpl.cpp',
                'src/WebScrollbarImpl.h',
                'src/WebSearchableFormData.cpp',
                'src/WebSecurityOrigin.cpp',
                'src/WebSecurityPolicy.cpp',
                'src/WebSelectElement.cpp',
                'src/WebSerializedScriptValue.cpp',
                'src/WebSettingsImpl.cpp',
                'src/WebSettingsImpl.h',
                'src/WebSharedWorkerImpl.cpp',
                'src/WebSharedWorkerImpl.h',
                'src/WebStorageAreaImpl.cpp',
                'src/WebStorageAreaImpl.h',
                'src/WebStorageEventDispatcherImpl.cpp',
                'src/WebStorageEventDispatcherImpl.h',
                'src/WebStorageNamespaceImpl.cpp',
                'src/WebStorageNamespaceImpl.h',
                'src/WebString.cpp',
                'src/WebURL.cpp',
                'src/WebURLRequest.cpp',
                'src/WebURLRequestPrivate.h',
                'src/WebURLResponse.cpp',
                'src/WebURLResponsePrivate.h',
                'src/WebURLError.cpp',
                'src/WebViewImpl.cpp',
                'src/WebViewImpl.h',
                'src/WebWorkerBase.cpp',
                'src/WebWorkerBase.h',
                'src/WebWorkerClientImpl.cpp',
                'src/WebWorkerClientImpl.h',
                'src/WebWorkerImpl.cpp',
                'src/WebWorkerImpl.h',
                'src/WrappedResourceRequest.h',
                'src/WrappedResourceResponse.h',
                'src/win/WebInputEventFactory.cpp',
                'src/win/WebScreenInfoFactory.cpp',
            ],
            'conditions': [
                ['OS=="linux" or OS=="freebsd"', {
                    'dependencies': [
                        '<(chromium_src_dir)/build/linux/system.gyp:fontconfig',
                        '<(chromium_src_dir)/build/linux/system.gyp:gtk',
                        '<(chromium_src_dir)/build/linux/system.gyp:x11',
                    ],
                    'include_dirs': [
                        'public/x11',
                        'public/gtk',
                        'public/linux',
                    ],
                }, { # else: OS!="linux" and OS!="freebsd"
                    'sources/': [
                        ['exclude', '/gtk/'],
                        ['exclude', '/x11/'],
                        ['exclude', '/linux/'],
                    ],
                }],
                ['OS=="mac"', {
                    'include_dirs': [
                        'public/mac',
                    ],
                    'sources/': [
                        ['exclude', 'Skia\\.cpp$'],
                    ],
                    'variables': {
                        # FIXME: Turn on warnings on other platforms and for
                        # other targets.
                        'chromium_code': 1,
                    }
                }, { # else: OS!="mac"
                    'sources/': [
                        ['exclude', '/mac/'],
                        ['exclude', 'CG\\.cpp$'],
                    ],
                }],
                ['OS=="win"', {
                    'include_dirs': [
                        'public/win',
                    ],
                }, { # else: OS!="win"
                    'sources/': [['exclude', '/win/']],
                }],
                ['"ENABLE_3D_CANVAS=1" in feature_defines', {
                    # Conditionally compile in GLEW and our GraphicsContext3D implementation.
                    'sources+': [
                        'src/GraphicsContext3D.cpp',
                        '<(chromium_src_dir)/third_party/glew/src/glew.c'
                    ],
                    'include_dirs+': [
                        '<(chromium_src_dir)/third_party/glew/include'
                    ],
                    'defines+': [
                        'GLEW_STATIC=1',
                        'GLEW_NO_GLU=1',
                    ],
                    'conditions': [
                        ['OS=="win"', {
                            'link_settings': {
                                'libraries': [
                                    '-lopengl32.lib',
                                ],
                            },
                        }],
                        ['OS=="mac"', {
                            'link_settings': {
                                'libraries': [
                                    '$(SDKROOT)/System/Library/Frameworks/OpenGL.framework',
                                ],
                            },
                        }],
                    ],
                }],
            ],
        },
        {
            'target_name': 'webkit_unit_tests',
            'type': 'executable',
            'msvs_guid': '7CEFE800-8403-418A-AD6A-2D52C6FC3EAD',
            'dependencies': [
                'webkit',
                '../../WebCore/WebCore.gyp/WebCore.gyp:webcore',
                '<(chromium_src_dir)/testing/gtest.gyp:gtest',
                '<(chromium_src_dir)/base/base.gyp:base',
                '<(chromium_src_dir)/base/base.gyp:base_i18n',
            ],
            'include_dirs': [
                'public',
                'src',
            ],
            'sources': [
                'tests/DragImageTest.cpp',
                'tests/KeyboardTest.cpp',
                'tests/KURLTest.cpp',
                'tests/RunAllTests.cpp',
            ],
            'conditions': [
                ['OS=="win"', {
                    'sources': [
                        # FIXME: Port PopupMenuTest to Linux and Mac.
                        'tests/PopupMenuTest.cpp',
                        'tests/TransparencyWinTest.cpp',
                        'tests/UniscribeHelperTest.cpp',
                    ],
                }],
                ['OS=="mac"', {
                    'sources!': [
                        # FIXME: Port DragImageTest to Mac.
                        'tests/DragImageTest.cpp',
                    ],
                }],
            ],
        },
        {
            'target_name': 'ImageDiff',
            'type': 'executable',
            'dependencies': [
                'webkit',
                '../../JavaScriptCore/JavaScriptCore.gyp/JavaScriptCore.gyp:wtf',
                '<(chromium_src_dir)/gfx/gfx.gyp:gfx',
            ],
            'include_dirs': [
                '../../JavaScriptCore',
                '<(DEPTH)',
            ],
            'sources': [
                '../../WebKitTools/DumpRenderTree/chromium/ImageDiff.cpp',
            ],
        },
        {
            'target_name': 'DumpRenderTree',
            'type': 'executable',
            'mac_bundle': 1,
            'dependencies': [
                'webkit',
                '../../JavaScriptCore/JavaScriptCore.gyp/JavaScriptCore.gyp:wtf_config',
                '<(chromium_src_dir)/third_party/icu/icu.gyp:icuuc',
                '<(chromium_src_dir)/webkit/support/webkit_support.gyp:webkit_support',
            ],
            'include_dirs': [
                '.',
                '../../JavaScriptCore',
                '../../JavaScriptCore/wtf', # wtf/text/*.h refers headers in wtf/ without wtf/.
                '<(DEPTH)',
            ],
            'defines': [
                # Technically not a unit test but require functions available only to
                # unit tests.
                'UNIT_TEST',
            ],
            'sources': [
                '<@(drt_files)',
            ],
            'conditions': [
                ['OS=="win"', {
                    'dependencies': ['LayoutTestHelper'],

                    'resource_include_dirs': ['<(SHARED_INTERMEDIATE_DIR)/webkit'],
                    'sources': [
                        '<(SHARED_INTERMEDIATE_DIR)/net/net_resources.rc',
                        '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_chromium_resources.rc',
                        '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_resources.rc',
                        '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_strings_en-US.rc',
                    ],
                    'copies': [{
                        'destination': '<(PRODUCT_DIR)',
                        'files': ['<(ahem_path)'],
                    }],
                }],
                ['OS=="mac"', {
                    'dependencies': ['LayoutTestHelper'],

                    'mac_bundle_resources': [
                        '<(ahem_path)',
                        '../../WebKitTools/DumpRenderTree/fonts/WebKitWeightWatcher100.ttf',
                        '../../WebKitTools/DumpRenderTree/fonts/WebKitWeightWatcher200.ttf',
                        '../../WebKitTools/DumpRenderTree/fonts/WebKitWeightWatcher300.ttf',
                        '../../WebKitTools/DumpRenderTree/fonts/WebKitWeightWatcher400.ttf',
                        '../../WebKitTools/DumpRenderTree/fonts/WebKitWeightWatcher500.ttf',
                        '../../WebKitTools/DumpRenderTree/fonts/WebKitWeightWatcher600.ttf',
                        '../../WebKitTools/DumpRenderTree/fonts/WebKitWeightWatcher700.ttf',
                        '../../WebKitTools/DumpRenderTree/fonts/WebKitWeightWatcher800.ttf',
                        '../../WebKitTools/DumpRenderTree/fonts/WebKitWeightWatcher900.ttf',
                        '<(SHARED_INTERMEDIATE_DIR)/webkit/textAreaResizeCorner.png',
                    ],
                    'actions': [
                        {
                            'action_name': 'repack_locale',
                            'variables': {
                                'repack_path': '<(chromium_src_dir)/tools/data_pack/repack.py',
                                'pak_inputs': [
                                    '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_chromium_resources.pak',
                                    '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_strings_en-US.pak',
                                    '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_resources.pak',
                            ]},
                            'inputs': [
                                '<(repack_path)',
                                '<@(pak_inputs)',
                            ],
                            'outputs': [
                                '<(INTERMEDIATE_DIR)/repack/DumpRenderTree.pak',
                            ],
                            'action': ['python', '<(repack_path)', '<@(_outputs)', '<@(pak_inputs)'],
                            'process_outputs_as_mac_bundle_resources': 1,
                        },
                    ], # actions
                }],
                ['OS=="linux"', {
                    'copies': [{
                        'destination': '<(PRODUCT_DIR)',
                        'files': [
                            '<(ahem_path)',
                            '../../WebKitTools/DumpRenderTree/chromium/fonts.conf',
                        ],
                    }],
                }],
                ['OS!="linux" and OS!="freebsd" and OS!="openbsd"', {
                    'sources/': [
                        ['exclude', '(Gtk|Linux)\\.cpp$']
                    ]
                }],
                ['OS!="win"', {
                    'sources/': [
                        ['exclude', 'Win\\.cpp$'],
                    ]
                }],
                ['OS!="mac"', {
                    'sources/': [
                        # .mm is already excluded by common.gypi
                        ['exclude', 'Mac\\.cpp$'],
                    ]
                }],
            ],
        },
    ], # targets
    'conditions': [
        ['OS=="win"', {
            'targets': [{
                'target_name': 'LayoutTestHelper',
                'type': 'executable',
                'sources': ['../../WebKitTools/DumpRenderTree/chromium/LayoutTestHelperWin.cpp'],
            }],
        }],
        ['OS=="mac"', {
            'targets': [
                {
                    'target_name': 'LayoutTestHelper',
                    'type': 'executable',
                    'sources': ['../../WebKitTools/DumpRenderTree/chromium/LayoutTestHelper.mm'],
                    'link_settings': {
                        'libraries': [
                            '$(SDKROOT)/System/Library/Frameworks/AppKit.framework',
                        ],
                    },
                },
            ],
        }],
    ], # conditions
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
