// tab_label.cpp
// tab labels

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
#include <gtkmm/stock.h>

#include "graph_page.hpp"
#include "tab_label.hpp"

Tab_label::Tab_label(): _close_img(Gtk::Stock::CLOSE, Gtk::ICON_SIZE_MENU)
{
    guint8 r = (guint8)(Graph_page::start_color.r * 256.0f);
    guint8 g = (guint8)(Graph_page::start_color.g * 256.0f);
    guint8 b = (guint8)(Graph_page::start_color.b * 256.0f);

    guint32 hex_color = r << 24 | g << 16 | b << 8;

    Glib::RefPtr<Gdk::Pixbuf> image = Gdk::Pixbuf::create(Gdk::Colorspace::COLORSPACE_RGB, false, 8, 16, 16);
    image->fill(hex_color);
    _tab_pic.set(image);

    _close_butt.set_always_show_image(true);
    _close_butt.set_image(_close_img);

    attach(_tab_pic, 0, 0, 1, 1);
    attach(_close_butt, 1, 0, 1, 1);

    _close_butt.signal_clicked().connect(sigc::mem_fun(*this, &Tab_label::on_button_press));

    show_all_children();
}

void Tab_label::set_img(const Gtk::Image & img)
{
    if(img.get_pixbuf())
        _tab_pic.set(img.get_pixbuf()->scale_simple(16, 16, Gdk::InterpType::INTERP_BILINEAR));
    else
        _tab_pic.set(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_SMALL_TOOLBAR);
}

void Tab_label::on_button_press()
{
    _signal_close_tab.emit();
}

sigc::signal<void> Tab_label::signal_close_tab()
{
    return _signal_close_tab;
}
