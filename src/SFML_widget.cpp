// SFML_widget.cpp
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

#include "SFML_widget.h"

SFML_widget::SFML_widget(sf::VideoMode Mode):
    sf::Window(Mode, "")
{
    set_flags(Gtk::NO_WINDOW); //Makes this behave like an interal object rather then a parent window.
    Glib::signal_idle().connect( sigc::mem_fun(*this, &SFML_widget::on_idle) );
}

void SFML_widget::on_size_request(Gtk::Requisition* requisition)
{
    *requisition = Gtk::Requisition();

    sf::Vector2u v = getSize();

    requisition->width = v.x;
    requisition->height = v.y;
}

void SFML_widget::on_size_allocate(Gtk::Allocation& allocation)
{
    set_allocation(allocation);

    if(gdk_window)
    {
        gdk_window->move_resize(allocation.get_x(), allocation.get_y(), allocation.get_width(), allocation.get_height() );
        setSize(sf::Vector2u(allocation.get_width(), allocation.get_height()));
    }
}

void SFML_widget::on_map()
{
    Gtk::Widget::on_map();
}

void SFML_widget::on_unmap()
{
    Gtk::Widget::on_unmap();
}

void SFML_widget::on_realize()
{
    Gtk::Widget::on_realize();

    if(!gdk_window)
    {
        //Create the gdk_window:
        GdkWindowAttr attributes;
        memset(&attributes, 0, sizeof(attributes));

        Gtk::Allocation allocation = get_allocation();

        //Set initial position and size of the Gdk::Window:
        attributes.x = allocation.get_x();
        attributes.y = allocation.get_y();
        attributes.width = allocation.get_width();
        attributes.height = allocation.get_height();

        attributes.event_mask = get_events () | Gdk::EXPOSURE_MASK;
        attributes.window_type = GDK_WINDOW_CHILD;
        attributes.wclass = GDK_INPUT_OUTPUT;


        gdk_window = Gdk::Window::create(get_window(), &attributes,
                GDK_WA_X | GDK_WA_Y);
        unset_flags(Gtk::NO_WINDOW);
        set_window(gdk_window);

        //make the widget receive expose events
        gdk_window->set_user_data(gobj());

        sf::Window::create((GDK_WINDOW_XID(gdk_window->gobj())));
    }
}

void SFML_widget::on_unrealize()
{
  gdk_window.clear();
  Gtk::Widget::on_unrealize();
}

bool SFML_widget::on_idle()
{
    if(gdk_window)
    {
        display();
    }

    return true;
}

bool SFML_widget::on_expose_event(GdkEventExpose* event)
{
    if(gdk_window)
    {
        display();
    }

    return true;
}

