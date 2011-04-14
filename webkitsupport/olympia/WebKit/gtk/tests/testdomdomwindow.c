/*
 * Copyright (C) 2010 Igalia S.L.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "test_utils.h"

#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <webkit/webkit.h>

#if GLIB_CHECK_VERSION(2, 16, 0) && GTK_CHECK_VERSION(2, 14, 0)

#define HTML_DOCUMENT "<html><head><title>This is the title</title></head><body></body></html>"

typedef struct {
    GtkWidget* webView;
    GtkWidget* window;
    WebKitDOMDOMWindow* domWindow;
    GMainLoop* loop;

    gboolean loaded;
    gboolean clicked;
    gconstpointer data;
} DomDomviewFixture;

static gboolean finish_loading(DomDomviewFixture* fixture)
{
    if (g_main_loop_is_running(fixture->loop))
        g_main_loop_quit(fixture->loop);

    return FALSE;
}

static void dom_domview_fixture_setup(DomDomviewFixture* fixture, gconstpointer data)
{
    fixture->loop = g_main_loop_new(NULL, TRUE);
    fixture->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    fixture->webView = webkit_web_view_new();
    fixture->data = data;

    gtk_container_add(GTK_CONTAINER(fixture->window), GTK_WIDGET(fixture->webView));
}

static void dom_domview_fixture_teardown(DomDomviewFixture* fixture, gconstpointer data)
{
    gtk_widget_destroy(fixture->window);
    g_main_loop_unref(fixture->loop);
}

static gboolean loadedCallback(WebKitDOMDOMWindow* view, WebKitDOMEvent* event, DomDomviewFixture* fixture)
{
    g_assert(fixture->loaded == FALSE);
    fixture->loaded = TRUE;

    return FALSE;
}

static gboolean clickedCallback(WebKitDOMDOMWindow* view, WebKitDOMEvent* event, DomDomviewFixture* fixture)
{
    WebKitDOMEventTarget* target;
    gushort phase;

    g_assert(event);
    g_assert(WEBKIT_DOM_IS_EVENT(event));

    // We should catch this in the bubbling up phase, since we are connecting to the toplevel object
    phase = webkit_dom_event_get_event_phase(event);
    g_assert_cmpint(phase, ==, 3);

    target = webkit_dom_event_get_current_target(event);
    g_assert(target == WEBKIT_DOM_EVENT_TARGET(view));

    g_assert(fixture->clicked == FALSE);
    fixture->clicked = TRUE;

    finish_loading(fixture);

    return FALSE;
}

gboolean map_event_cb(GtkWidget *widget, GdkEvent* event, DomDomviewFixture* fixture)
{
    webkit_web_view_load_string(WEBKIT_WEB_VIEW (fixture->webView), (const char*)fixture->data, NULL, NULL, NULL);

    return FALSE;
}

static void load_event_callback(WebKitWebView* webView, GParamSpec* spec, DomDomviewFixture* fixture)
{
    WebKitLoadStatus status = webkit_web_view_get_load_status(webView);
    if (status == WEBKIT_LOAD_FINISHED) {
        g_signal_connect(fixture->domWindow, "click-event", G_CALLBACK(clickedCallback), fixture);

        g_assert(fixture->clicked == FALSE);
        gtk_test_widget_click (GTK_WIDGET(fixture->webView), 1, 0);
    }

}

static void test_dom_domview_signals(DomDomviewFixture* fixture, gconstpointer data)
{
    g_assert(fixture);
    WebKitWebView* view = (WebKitWebView*)fixture->webView;
    g_assert(view);
    WebKitDOMDocument* document = webkit_web_view_get_dom_document(view);
    g_assert(document);
    WebKitDOMDOMWindow* domWindow = webkit_dom_document_get_default_view(document);
    g_assert(domWindow);

    fixture->domWindow = domWindow;

    g_signal_connect(fixture->domWindow, "load-event", G_CALLBACK(loadedCallback), fixture);
    g_signal_connect(fixture->window, "map-event", G_CALLBACK(map_event_cb), fixture);
    g_signal_connect(fixture->webView, "notify::load-status", G_CALLBACK(load_event_callback), fixture);

    gtk_widget_show_all(fixture->window);
    gtk_window_present(GTK_WINDOW(fixture->window));

    g_main_loop_run (fixture->loop);

    g_assert(fixture->loaded);
    g_assert(fixture->clicked);
}

int main(int argc, char** argv)
{
    if (!g_thread_supported())
        g_thread_init(NULL);

    gtk_test_init(&argc, &argv, NULL);

    g_test_bug_base("https://bugs.webkit.org/");

    g_test_add("/webkit/domdomview/signals",
               DomDomviewFixture, HTML_DOCUMENT,
               dom_domview_fixture_setup,
               test_dom_domview_signals,
               dom_domview_fixture_teardown);

    return g_test_run();
}

#else
int main(int argc, char** argv)
{
    g_critical("You will need at least glib-2.16.0 and gtk-2.14.0 to run the unit tests. Doing nothing now.");
    return 0;
}

#endif
