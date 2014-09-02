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

#include <iostream>

#include <gdkmm/rgba.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/grid.h>
#include <gtkmm/separator.h>
#include <gtkmm/stock.h>

#include "lighting_window.hpp"

// ctor takes ref to 2 light types
Lighting_window::Lighting_window(Light & dir_light, Light & cam_light):
    _x_dir(Gtk::Adjustment::create(dir_light.pos.x, -100.0, 100.0, 0.1), 0.1, 1),
    _y_dir(Gtk::Adjustment::create(dir_light.pos.y, -100.0, 100.0, 0.1), 0.1, 1),
    _z_dir(Gtk::Adjustment::create(dir_light.pos.z, -100.0, 100.0, 0.1), 0.1, 1),
    _dir_strength(Gtk::Adjustment::create(dir_light.strength, 0.0, 10.0, 0.1), Gtk::ORIENTATION_HORIZONTAL),
    _cam_strength(Gtk::Adjustment::create(cam_light.strength, 0.0, 10.0, 0.1), Gtk::ORIENTATION_HORIZONTAL),
    _cam_const_atten(Gtk::Adjustment::create(cam_light.const_atten, 0.0, 1.0, 0.01)),
    _cam_linear_atten(Gtk::Adjustment::create(cam_light.linear_atten, 0.0, 1.0, 0.01)),
    _cam_quad_atten(Gtk::Adjustment::create(cam_light.quad_atten, 0.0, 10., 0.01)),
    _dir_light(dir_light),
    _cam_light(cam_light)
{
    set_title("Lighting Options");
    Gtk::manage(add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL));
    Gtk::manage(add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK));

    Gdk::RGBA start_color;
    start_color.set_rgba(dir_light.color.r, dir_light.color.g, dir_light.color.b, 1.0);
    _dir_color.set_rgba(start_color);
    start_color.set_rgba(cam_light.color.r, cam_light.color.g, cam_light.color.b, 1.0);
    _cam_color.set_rgba(start_color);

    Gtk::Grid * grid = new Gtk::Grid;
    get_content_area()->add(*Gtk::manage(grid));

    grid->set_border_width(3);
    grid->set_row_spacing(3);
    grid->set_column_spacing(3);
    grid->set_hexpand(true);
    grid->set_vexpand(true);

    grid->attach(*Gtk::manage(new Gtk::Label("Directional Light")), 0, 0, 7, 1);
    Gtk::Separator * spacer = new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL);
    spacer->set_hexpand(true);
    grid->attach(*Gtk::manage(spacer), 0, 1, 7, 1);

    grid->attach(*Gtk::manage(new Gtk::Label("Direction (x,y,z) = (")), 0, 2, 1, 1);
    grid->attach(_x_dir, 1, 2, 1, 1);
    grid->attach(*Gtk::manage(new Gtk::Label(",")), 2, 2, 1, 1);
    grid->attach(_y_dir, 3, 2, 1, 1);
    grid->attach(*Gtk::manage(new Gtk::Label(",")), 4, 2, 1, 1);
    grid->attach(_z_dir, 5, 2, 1, 1);
    grid->attach(*Gtk::manage(new Gtk::Label(")")), 6, 2, 1, 1);

    grid->attach(*Gtk::manage(new Gtk::Label("Color:")), 0, 3, 1, 1);
    grid->attach(_dir_color, 1, 3, 1, 1);

    grid->attach(*Gtk::manage(new Gtk::Label("Strength:")), 3, 3, 1, 1);
    grid->attach(_dir_strength, 4, 3, 3, 1);

    grid->attach(*Gtk::manage(new Gtk::Label("Camera Light")), 0, 4, 7, 1);
    grid->attach(*Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_HORIZONTAL)), 0, 5, 7, 1);

    grid->attach(*Gtk::manage(new Gtk::Label("Color:")), 0, 6, 1, 1);
    grid->attach(_cam_color, 1, 6, 1, 1);

    grid->attach(*Gtk::manage(new Gtk::Label("Strength:")), 3, 6, 1, 1);
    grid->attach(_cam_strength, 4, 6, 3, 1);

    grid->attach(*Gtk::manage(new Gtk::Label("")), 0, 7, 7, 1);

    grid->attach(*Gtk::manage(new Gtk::Label("Attenuation")), 0, 8, 1, 2);

    grid->attach(*Gtk::manage(new Gtk::Label("Constant")), 1, 9, 1, 1);
    grid->attach(*Gtk::manage(new Gtk::Label("Linear")), 3, 9, 1, 1);
    grid->attach(*Gtk::manage(new Gtk::Label("Quadratic")), 5, 9, 1, 1);

    grid->attach(_cam_const_atten, 1, 8, 1, 1);
    grid->attach(_cam_linear_atten, 3, 8, 1, 1);
    grid->attach(_cam_quad_atten, 5, 8, 1, 1);

    show_all_children();

    // attach handler for user's response
    signal_response().connect(sigc::mem_fun(*this, &Lighting_window::store));
}

// store values to referenced light objs before closing
void Lighting_window::store(int response)
{
    _dir_light.pos.x = _x_dir.get_value();
    _dir_light.pos.y = _y_dir.get_value();
    _dir_light.pos.z = _z_dir.get_value();

    _dir_light.color.r = _dir_color.get_rgba().get_red();
    _dir_light.color.g = _dir_color.get_rgba().get_green();
    _dir_light.color.b = _dir_color.get_rgba().get_blue();

    _dir_light.strength = _dir_strength.get_value();

    _cam_light.color.r = _cam_color.get_rgba().get_red();
    _cam_light.color.g = _cam_color.get_rgba().get_green();
    _cam_light.color.b = _cam_color.get_rgba().get_blue();

    _cam_light.strength = _cam_strength.get_value();

    _cam_light.const_atten = _cam_const_atten.get_value();
    _cam_light.linear_atten = _cam_linear_atten.get_value();
    _cam_light.quad_atten = _cam_quad_atten.get_value();
}
