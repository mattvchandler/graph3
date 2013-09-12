// graph_window.hpp
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

#ifndef __GRAPH_WINDOW_H__
#define __GRAPH_WINDOW_H__

#include <vector>

#include <gtkmm/colorbutton.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>
#include <gtkmm/stock.h>
#include <gtkmm/window.h>

#include "graph_disp.hpp"

class Graph_window final: public Gtk::Window
{
public:
    Graph_window();

    void update_cursor_text(size_t i);
    void change_graph_color(size_t i);
    void add_graphs(); // TODO: delete me

private:
    Graph_disp gl_window;
    Gtk::Grid main_grid;

    std::vector<std::unique_ptr<Gtk::Label>> cursor_texts;
    std::vector<std::unique_ptr<Gtk::ColorButton>> color_buts;
};

#endif // __GRAPH_WINDOW_H__
