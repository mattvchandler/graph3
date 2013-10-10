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
#include <string>

#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/grid.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/notebook.h>
#include <gtkmm/uimanager.h>
#include <gtkmm/window.h>

#include <sigc++/sigc++.h>

#include <glm/glm.hpp>

#include "graph_disp.hpp"
#include "graph_page.hpp"
#include "tab_label.hpp"

class Graph_window final: public Gtk::Window
{
public:
    Graph_window();

private:
    void tab_new();
    void tab_close(Graph_page * page);
    void tab_change(Widget * page, guint page_no);
    void change_flags();
    void update_cursor(const std::string & text);

    void save_graph();
    void load_graph();

    Glib::RefPtr<Gtk::UIManager> _menu;
    Glib::RefPtr<Gtk::ActionGroup> _menu_act;

    Gtk::Grid _main_grid;

    // widgets
    Graph_disp _gl_window;
    Gtk::Label _cursor_text;
    Gtk::Button _add_tab_butt;
    Gtk::Image _add_tab_butt_img;
    Gtk::CheckButton _draw_axes, _draw_cursor;

    sigc::connection _cursor_conn;

    std::list<std::unique_ptr<Graph_page>> _pages;
    Gtk::Notebook _notebook; // this needs to be last, as several of the above are referenced during its destruction, due to dumbness. (I don't want to switch pages, I want to destroy them *all*)
};

#endif // __GRAPH_WINDOW_H__
