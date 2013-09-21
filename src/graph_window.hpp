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

#include <memory>
#include <list>

#include <gtkmm/button.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/notebook.h>
#include <gtkmm/stock.h>
#include <gtkmm/window.h>

#include <sigc++/sigc++.h>

#include "graph_disp.hpp"

class Graph_page final: public Gtk::Grid
{
public:
    Graph_page(Graph_disp * gl_window);
    ~Graph_page();


    void apply();
    void change_color();
    void update_cursor(const std::string & text);
    void set_active();

    // sigc::signal<void, Graph *> signal_graph_regen();
    sigc::signal<void, const std::string &> signal_cursor_moved();

private:
    Graph_disp * _gl_window;
    std::unique_ptr<Graph> _graph;

    Gtk::Entry _eqn;
    Gtk::Entry _row_min, _row_max;
    Gtk::Entry _col_min, _col_max;
    Gtk::ColorButton _color_butt;
    Gtk::Button _apply_butt;

    // sigc::signal<void, Graph *> _signal_graph_regen;
    sigc::signal<void, const std::string &> _signal_cursor_moved;

    // make non-copyable
    Graph_page(const Graph_page &) = delete;
    Graph_page(const Graph_page &&) = delete;
    Graph_page & operator =(const Graph_page &) = delete;
    Graph_page & operator =(const Graph_page &&) = delete;
};


class Tab_label final: public Gtk::Grid
{
public:
    Tab_label();
    sigc::signal<void> signal_close_tab();
private:
    void on_button_press();

    Gtk::Label _tab_text;
    Gtk::Button _close_butt;
    Gtk::Image _close_img;
    sigc::signal<void> _signal_close_tab;
};

class Graph_window final: public Gtk::Window
{
public:
    Graph_window();

    void tab_new();
    void tab_close(Graph_page * page);
    void tab_change(Widget * page, guint page_no);
    void update_cursor(const std::string & text);

private:
    Graph_disp _gl_window;
    Gtk::Grid _main_grid;
    Gtk::Notebook _notebook;
    std::list<std::unique_ptr<Graph_page>> _pages;
    Gtk::Label _cursor_text;
    Gtk::Button _add_tab_butt;

    sigc::connection _cursor_conn;
};

#endif // __GRAPH_WINDOW_H__
