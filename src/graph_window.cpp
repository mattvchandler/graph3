// graph_window.cpp
// windowing code. Using GTK to create the window, SFML to do openGL graphics.

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

// TODO: axes
#include "graph_window.hpp"

#include "graph.hpp"
#include "graph_cartesian.hpp"
#include "graph_cylindrical.hpp"
#include "graph_spherical.hpp"

Graph_window::Graph_window(): gl_window(sf::VideoMode(800, 600), -1, sf::ContextSettings(0, 0, 4, 4, 0)) // these do nothing yet - future SFML version should enable them
{
    set_title("Graph 3");
    set_default_size(800, 600);
    gl_window.set_hexpand(true);
    gl_window.set_vexpand(true);

    add(main_grid);
    main_grid.set_column_spacing(5);

    main_grid.attach(gl_window, 0, 0, 1, 9);
    show_all_children();

    gl_window.signal_realize().connect(sigc::mem_fun(*this, &Graph_window::add_graphs)); // TODO: delete me!
}

void Graph_window::add_graphs() // TODO: delete me
{
    color_buts.push_back(std::unique_ptr<Gtk::ColorButton>(new Gtk::ColorButton));
    cursor_texts.push_back(std::unique_ptr<Gtk::Label>(new Gtk::Label));
    color_buts[0]->signal_color_set().connect(sigc::bind<size_t>(sigc::mem_fun(*this, &Graph_window::change_graph_color), 0));

    // color_buts.push_back(std::unique_ptr<Gtk::ColorButton>(new Gtk::ColorButton));
    // cursor_texts.push_back(std::unique_ptr<Gtk::Label>(new Gtk::Label));
    // color_buts[1]->signal_color_set().connect(sigc::bind<size_t>(sigc::mem_fun(*this, &Graph_window::change_graph_color), 1));

    // TODO error handling
    gl_window.graphs.push_back(std::unique_ptr<Graph>(new Graph_spherical("1", 0.0f, 2.0f * M_PI, 50, 0.0f, M_PI, 50)));
    // gl_window.graphs.push_back(std::unique_ptr<Graph>(new Graph_cylindrical("sqrt(r)", 0.0f, 10.0f, 50, 0.0f, 2.0f * M_PI, 50)));
    // gl_window.graphs.push_back(std::unique_ptr<Graph>(new Graph_cylindrical("-sqrt(r)", 0.0f, 10.0f, 50, 0.0f, 2.0f * M_PI, 50)));
    // gl_window.graphs.push_back(std::unique_ptr<Graph>(new Graph_cartesian("sqrt(1 - x^2 + y^2)", -2.0f, 2.0f, 50, -2.0f, 2.0f, 50)));
    // gl_window.graphs.push_back(std::unique_ptr<Graph>(new Graph_cartesian("-sqrt(1 - x^2 + y^2)", -2.0f, 2.0f, 50, -2.0f, 2.0f, 50)));

    gl_window.graphs[0]->signal_cursor_moved().connect(sigc::bind<size_t>(sigc::mem_fun(*this, &Graph_window::update_cursor_text), 0));
    // gl_window.graphs[1]->signal_cursor_moved().connect(sigc::bind<size_t>(sigc::mem_fun(*this, &Graph_window::update_cursor_text), 1));

    main_grid.attach(*cursor_texts[0], 0, 9, 1, 1);
    main_grid.attach(*color_buts[0], 1, 0, 1, 1);
    // main_grid.attach(*cursor_texts[1], 0, 10, 1, 1);
    // main_grid.attach(*color_buts[1], 1, 1, 1, 1);
    show_all_children();

    Gdk::RGBA start_rgba;
    start_rgba.set_rgba(0.2, 0.5, 0.2, 1.0);
    color_buts[0]->set_rgba(start_rgba);
    color_buts[0]->set_title("Graph 1 Color");
    // color_buts[1]->set_rgba(start_rgba);
    // color_buts[1]->set_title("Graph 2 Color");

    gl_window.graphs[0]->tex = gl_window.textures[0];

    change_graph_color(0);
    update_cursor_text(0);
    // change_graph_color(1);
    // update_cursor_text(1);

    gl_window.invalidate();
}
void Graph_window::update_cursor_text(size_t i)
{
    cursor_texts[i]->set_label(gl_window.graphs[i]->cursor_text());
}

void Graph_window::change_graph_color(size_t i)
{
    gl_window.graphs[i]->color.r = color_buts[i]->get_rgba().get_red();
    gl_window.graphs[i]->color.g = color_buts[i]->get_rgba().get_green();
    gl_window.graphs[i]->color.b = color_buts[i]->get_rgba().get_blue();
    gl_window.graphs[i]->color.a = 1.0f;
    gl_window.invalidate();
}

