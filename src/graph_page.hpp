// graph_page.hpp
// widgets for creating graphs

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

#ifndef __GRAPH_PAGE_H__
#define __GRAPH_PAGE_H__

#include <memory>

#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/spinbutton.h>

#include <sigc++/sigc++.h>

#include "graph_disp.hpp"

class Graph_page final: public Gtk::Grid
{
public:
    Graph_page(Graph_disp * gl_window);
    ~Graph_page();

    void change_type();
    void apply();
    void change_flags();
    void change_color();
    void update_cursor(const std::string & text) const;
    void set_active();

    sigc::signal<void, const std::string &> signal_cursor_moved() const;

private:
    Graph_disp * _gl_window;
    std::unique_ptr<Graph> _graph;

    Gtk::RadioButton _r_car, _r_cyl, _r_sph, _r_par;
    Gtk::Entry _eqn;
    Gtk::Entry _eqn_par_y, _eqn_par_z;
    Gtk::Entry _row_min, _row_max;
    Gtk::Entry _col_min, _col_max;
    Gtk::SpinButton _row_res, _col_res;
    Gtk::CheckButton _draw_grid, _draw_normals;
    Gtk::ColorButton _color_butt;
    Gtk::Button _apply_butt;

    sigc::signal<void, const std::string &> _signal_cursor_moved;

    // make non-copyable
    Graph_page(const Graph_page &) = delete;
    Graph_page(const Graph_page &&) = delete;
    Graph_page & operator =(const Graph_page &) = delete;
    Graph_page & operator =(const Graph_page &&) = delete;
};

#endif // __GRAPH_PAGE_H__
