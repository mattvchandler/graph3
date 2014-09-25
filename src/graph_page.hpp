// graph_page.hpp
// widgets for creating graphs

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

#ifndef GRAPH_PAGE_H
#define GRAPH_PAGE_H

#include <memory>
#include <string>

#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/scale.h>
#include <gtkmm/spinbutton.h>

#include <sigc++/sigc++.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "graph_disp.hpp"

// Graph parameters and properties page
// allows user to set equations, bounds, color/textures
// this class has ownership of the graph itself
class Graph_page final: public Gtk::Grid
{
public:
    Graph_page(Graph_disp & gl_window);
    ~Graph_page();

    // sets the graph as the active graph
    void set_active();
    // save and load to / from a file
    void save_graph(const std::string & filename);
    bool load_graph(const std::string & filename);

    // signaled when the cursor has moved and needs updating
    sigc::signal<void, const std::string &> signal_cursor_moved() const;

    // signaled when the user selects a new texture
    sigc::signal<void, const Gtk::Image &> signal_tex_changed() const;

    static const glm::vec3 start_color;

private:
    // called when the type changes (cartesian, cylindrical, etc.)
    void change_type();
    // called when checkboxes for displaying grid, normals are pressed
    void change_flags();
    // called when changing transparency
    void change_transparency();
    // called when switching between color and texture
    void change_coloring();
    // called when the color or texture is changed
    void change_tex();
    // apply changes and create/update graph
    void apply();
    // called when the cursor needs changed
    void update_cursor(const std::string & text) const;

    // reference to the OpenGL window/context
    Graph_disp & _gl_window;
    // the graph itself
    std::unique_ptr<Graph> _graph;

    // UI widgets
    Gtk::RadioButton _r_car, _r_cyl, _r_sph, _r_par; // for selecting type
    Gtk::Entry _eqn; // equation entry
    Gtk::Entry _eqn_par_y, _eqn_par_z; // extra boxes for parametric graphs
    Gtk::Entry _row_min, _row_max; // bounds
    Gtk::Entry _col_min, _col_max;
    Gtk::Label _row_res_l, _col_res_l; // resolution
    Gtk::SpinButton _row_res, _col_res;
    Gtk::RadioButton _use_color, _use_tex; // color/texture selection
    Gtk::Button _tex_butt; // color / texture chooser
    Gtk::CheckButton _draw, _transparent, _draw_normals, _draw_grid; // selects what is drawn
    Gtk::Label _transparency_l;
    Gtk::Scale _transparency;
    Gtk::Button _apply_butt; // apply changes

    // UI resources
    Gtk::Image _color_ico;
    Gtk::Image _tex_ico;
    glm::vec3 _color;
    std::string _tex_filename;

    // signal types
    sigc::signal<void, const std::string &> _signal_cursor_moved;
    sigc::signal<void, const Gtk::Image &> _signal_tex_changed;

    // make non-copyable
    Graph_page(const Graph_page &) = delete;
    Graph_page(const Graph_page &&) = delete;
    Graph_page & operator =(const Graph_page &) = delete;
    Graph_page & operator =(const Graph_page &&) = delete;
};

#endif // GRAPH_PAGE_H
