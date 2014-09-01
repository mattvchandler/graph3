// lighting_window.cpp
// lighting settings window

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

#include <gdkmm/rgba.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/grid.h>
#include <gtkmm/separator.h>
#include <gtkmm/stock.h>

#include "lighting_window.hpp"

Lighting_window::Lighting_window(Light & dir_light, Light & cam_light):
    _x_dir(Gtk::Adjustment::create(-1.0, -100.0, 100.0, 0.1)),
    _y_dir(Gtk::Adjustment::create(-1.0, -100.0, 100.0, 0.1)),
    _z_dir(Gtk::Adjustment::create(-1.0, -100.0, 100.0, 0.1)),
    _dir_strength(Gtk::Adjustment::create(0.2, 0.0, 10.0, 0.1), Gtk::ORIENTATION_HORIZONTAL),
    _cam_strength(Gtk::Adjustment::create(0.2, 0.0, 10.0, 0.1)),
    _dir_light(dir_light),
    _cam_light(cam_light)
{
    Gtk::manage(add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL));
    Gtk::manage(add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK));

    Gdk::RGBA start_color;
    start_color.set_rgba(1.0, 1.0, 1.0, 1.0);
    _dir_color.set_rgba(start_color);
    _cam_color.set_rgba(start_color);

    Gtk::Grid * grid = new Gtk::Grid;
    get_content_area()->pack_start(*Gtk::manage(grid));

    grid->set_border_width(3);
    grid->set_row_spacing(3);
    grid->set_column_spacing(3);

    // TODO: labels, cleanup
    grid->attach(_x_dir, 0, 0, 1, 1);
    grid->attach(_y_dir, 1, 0, 1, 1);
    grid->attach(_z_dir, 2, 0, 1, 1);

    grid->attach(_dir_color, 0, 1, 3, 1);

    grid->attach(_dir_strength, 0, 2, 3, 1);

    grid->attach(*Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL)), 0, 3, 3, 1);

    grid->attach(_cam_color, 0, 4, 3, 1);

    grid->attach(_cam_strength, 0, 5, 3, 1);

    show_all_children();
}
