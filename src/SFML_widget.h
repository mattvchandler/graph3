// SFML_widget.h
// GTK widget containing an SFML OpenGL drawing context

// Copyright 2013 Matthew Chandler

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef __SFML_WIDGET_H__
#define __SFML_WIDGET_H__

#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>

#include <gdkmm/general.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include <gtkmm/widget.h>

class SFML_widget: public Gtk::Widget, public sf::Window
{
public:
    SFML_widget(sf::VideoMode Mode);

protected:
    sf::VideoMode m_vMode;

    virtual void on_size_request(Gtk::Requisition* requisition);
    virtual void on_size_allocate(Gtk::Allocation& allocation);
    virtual void on_map();
    virtual void on_unmap();
    virtual void on_realize();
    virtual void on_unrealize();
    virtual bool on_idle();
    virtual bool on_expose_event(GdkEventExpose* event);

    Glib::RefPtr<Gdk::Window> gdk_window;
};

#endif // __SFML_WIDGET_H__
