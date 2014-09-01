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

#ifndef GRAPH_WINDOW_H
#define GRAPH_WINDOW_H

#include <memory>
#include <list>
#include <string>

#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/grid.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/notebook.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/window.h>

#include <sigc++/sigc++.h>

#include <glm/glm.hpp>

#include "graph_disp.hpp"
#include "graph_page.hpp"
#include "lighting_window.hpp"
#include "tab_label.hpp"

// main application window.
// contains / owns OpenGL display and a tabbed list of graph property pages
class Graph_window final: public Gtk::Window
{
public:
    Graph_window();

private:
    // select file to save to or load from
    void save_graph();
    void load_graph();
    // called when checkbox or radio buttons are pressed
    void change_flags();
    // display lighting options
    void lighting();
    // update cursor text
    void update_cursor(const std::string & text);
    // create a new graph page
    void tab_new();
    // close a graph page and delete the graph
    void tab_close(Graph_page & page);
    // change active graph
    void tab_change(Widget * page, guint page_no);

    // UI elements
    Glib::RefPtr<Gtk::UIManager> _menu;
    Glib::RefPtr<Gtk::ActionGroup> _menu_act;

    Gtk::Grid _main_grid;
    Gtk::Grid _toolbar;

    // widgets
    Graph_disp _gl_window;
    Gtk::Label _cursor_text;
    Gtk::CheckButton _draw_axes, _draw_cursor;
    Gtk::RadioButton _use_orbit_cam, _use_free_cam;

    sigc::connection _cursor_conn;

    // graph pages
    std::list<std::unique_ptr<Graph_page>> _pages;
    Gtk::Notebook _notebook; // this needs to be last, as several of the above are referenced during its destruction, due to dumbness. (I don't want to switch pages, I want to destroy them *all*)
};

#endif // GRAPH_WINDOW_H
