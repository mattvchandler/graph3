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
#include "graph_window.h"

Graph_window::Graph_window(): gl_window(sf::VideoMode(800, 600), -1, sf::ContextSettings(0, 0, 4, 4, 0)) // these do nothing yet - future SFML version should enable them
{
    set_title("Graph 3");
    set_default_size(800, 600);
    gl_window.set_hexpand(true);
    gl_window.set_vexpand(true);

    add(main_grid);
    main_grid.set_column_spacing(5);

    main_grid.attach(gl_window, 0, 0, 1, 9);
    main_grid.attach(cursor_text, 0, 9, 1, 1);
    main_grid.attach(color_but, 1, 0, 1, 1);
    show_all_children();

    gl_window.test_graph->signal_cursor_moved().connect(sigc::mem_fun(*this, &Graph_window::update_cursor_text));

    Gdk::RGBA start_rgba;
    start_rgba.set_rgba(0.2, 0.5, 0.2, 1.0);
    color_but.set_rgba(start_rgba);
    color_but.set_title("Graph 1 Color");
    color_but.signal_color_set().connect(sigc::mem_fun(*this, &Graph_window::change_graph_color));
}

void Graph_window::update_cursor_text()
{
    cursor_text.set_label(gl_window.test_graph->cursor_text());
}

void Graph_window::change_graph_color()
{
    gl_window.test_graph->color.r = color_but.get_rgba().get_red();
    gl_window.test_graph->color.g = color_but.get_rgba().get_green();
    gl_window.test_graph->color.b = color_but.get_rgba().get_blue();
    gl_window.test_graph->color.a = 1.0f;
    gl_window.invalidate();
}

