// lighting_window.hpp
// lighting settings window

// Copyright 2014 Matthew Chandler

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

#ifndef LIGHTING_WINDOW_H
#define LIGHTING_WINDOW_H

#include <gtkmm/colorbutton.h>
#include <gtkmm/dialog.h>
#include <gtkmm/scale.h>
#include <gtkmm/spinbutton.h>

#include <glm/glm.hpp>

#include "graph_disp.hpp"

// lighting settings window
class Lighting_window final: public Gtk::Dialog
{
public:
    // ctor takes ref to 2 light types
    Lighting_window(Light & dir_light, Light & cam_light);
    // store values to referenced light objs before closing
    void store(int response);

private:
    // dir light adjustments
    Gtk::SpinButton _x_dir, _y_dir, _z_dir;
    Gtk::ColorButton _dir_color;
    Gtk::Scale _dir_strength;

    // cam light adjustments
    Gtk::ColorButton _cam_color;
    Gtk::Scale _cam_strength;
    Gtk::Scale _cam_const_atten, _cam_linear_atten, _cam_quad_atten;

    // keep ref to lights so we can write to them
    Light & _dir_light, & _cam_light;
};

#endif // LIGHTING_WINDOW_H
