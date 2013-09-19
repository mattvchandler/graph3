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
#include "graph_parametric.hpp"

Graph_page::Graph_page(Graph_disp * gl_window): _gl_window(gl_window), _graph(nullptr), _apply_butt(Gtk::Stock::APPLY)
{
    attach(_color_butt, 0, 0, 1, 1);
    attach(_apply_butt, 1, 0, 1, 1);

    _apply_butt.signal_clicked().connect(sigc::mem_fun(*this, &Graph_page::apply));
    _color_butt.signal_color_set().connect(sigc::mem_fun(*this, &Graph_page::change_color));

    Gdk::RGBA start_rgba;
    start_rgba.set_rgba(0.2, 0.5, 0.2, 1.0);
    _color_butt.set_rgba(start_rgba);

    show_all_children();
}

Graph_page::~Graph_page()
{
    _gl_window->remove_graph(_graph.get());
    _gl_window->invalidate();
}

void Graph_page::apply()
{
    _gl_window->remove_graph(_graph.get());

    _graph = std::unique_ptr<Graph>(new Graph_parametric(
        "-2/15 * cos(u) * (3 * cos(v) - 30 * sin(u) + 90*cos(u)^4 * sin(u) - 60 * cos(u)^6 * sin(u) + 5 * cos(u) * cos(v) * sin(u)),"
        "-1/15 * sin(u) * (3 * cos(v) - 3 * cos(u)^2 * cos(v) - 48 * cos(u)^4 * cos(v) + 48 * cos(u)^6 * cos(v) - 60 * sin(u) + 5 * cos(u) * cos(v) * sin(u) -5 * cos(u)^3 * cos(v) * sin(u) - 80 * cos(u)^5 * cos(v) * sin(u) + 80 * cos(u)^7 * cos(v) * sin(u)),"
        "2/15 * (3 + 5 * cos(u) * sin(u)) * sin(v)",
        0.0f, M_PI, 50, 0.0f, 2.0f * M_PI, 50)); // klein bottle

    change_color();
    update_cursor(_graph->cursor_text());

    _gl_window->add_graph(_graph.get());
    _gl_window->set_active_graph(_graph.get());

    _graph->signal_cursor_moved().connect(sigc::mem_fun(*this, &Graph_page::update_cursor));
    // _signal_graph_regen.emit(_graph.get());
    _gl_window->invalidate();
}

void Graph_page::change_color()
{
    if(_graph.get())
    {
        _graph->color.r = _color_butt.get_rgba().get_red();
        _graph->color.g = _color_butt.get_rgba().get_green();
        _graph->color.b = _color_butt.get_rgba().get_blue();
        _graph->color.a = 1.0f;
        _gl_window->invalidate();
    }
}

void Graph_page::update_cursor(const std::string & text)
{
    _signal_cursor_moved.emit(text);
}

void Graph_page::set_active()
{
    _gl_window->set_active_graph(_graph.get());

    if(_graph.get())
        update_cursor(_graph->cursor_text());
    else
        update_cursor("");

    _gl_window->invalidate();
}
// sigc::signal<void, Graph *> Graph_page::signal_graph_regen()
// {
//     return _signal_graph_regen;
// }

sigc::signal<void, const std::string &> Graph_page::signal_cursor_moved()
{
    return _signal_cursor_moved;
}

Tab_label::Tab_label(): _close_img(Gtk::Stock::CLOSE, Gtk::ICON_SIZE_MENU)
{
    _close_butt.set_always_show_image(true);
    _close_butt.set_image(_close_img);
    _tab_text.set_text("test");

    attach(_tab_text, 0, 0, 1, 1);
    attach(_close_butt, 1, 0, 1, 1);

    _close_butt.signal_clicked().connect(sigc::mem_fun(*this, &Tab_label::on_button_press));

    show_all_children();
}

void Tab_label::on_button_press()
{
    _signal_close_tab.emit();
}

sigc::signal<void> Tab_label::signal_close_tab()
{
    return _signal_close_tab;
}

Graph_window::Graph_window(): _gl_window(sf::VideoMode(800, 600), -1, sf::ContextSettings(0, 0, 4, 4, 0)), // these do nothing yet - future SFML version should enable them
    _add_tab_butt(Gtk::Stock::ADD)
{
    set_title("Graph 3");
    set_default_size(800, 600);

    _gl_window.set_hexpand(true);
    _gl_window.set_vexpand(true);

    _notebook.set_vexpand(true);
    _notebook.set_hexpand(true);
    _notebook.set_scrollable(true);


    add(_main_grid);
    _main_grid.set_column_spacing(5);

    _main_grid.attach(_gl_window, 0, 0, 1, 100);
    _main_grid.attach(_add_tab_butt, 1, 0, 1, 1);
    _main_grid.attach(_notebook, 1, 1, 1, 100);
    _main_grid.attach(_cursor_text, 0, 100, 1, 1);

    _pages.push_back(std::unique_ptr<Graph_page>(new Graph_page(&_gl_window)));
    _notebook.append_page(*_pages.back(), *Gtk::manage(new Tab_label));

    dynamic_cast<Tab_label *>(_notebook.get_tab_label(*_pages.back()))
        ->signal_close_tab().connect(sigc::bind<Graph_page *>(sigc::mem_fun(*this, &Graph_window::tab_close),
        _pages.back().get()));

    _add_tab_butt.signal_clicked().connect(sigc::mem_fun(*this, &Graph_window::tab_new));
    _notebook.signal_switch_page().connect(sigc::mem_fun(*this, &Graph_window::tab_change));

    _gl_window.invalidate();
    show_all_children();
}

void Graph_window::tab_new()
{
    _pages.push_back(std::unique_ptr<Graph_page>(new Graph_page(&_gl_window)));
    _notebook.append_page(*_pages.back(), *Gtk::manage(new Tab_label));

    dynamic_cast<Tab_label *>(_notebook.get_tab_label(*_pages.back()))
        ->signal_close_tab().connect(sigc::bind<Graph_page *>(sigc::mem_fun(*this, &Graph_window::tab_close),
        _pages.back().get()));

    _pages.back()->show();
}

void Graph_window::tab_close(Graph_page * page)
{
    if(_notebook.get_n_pages() == 1)
    {
        _cursor_conn.disconnect();
        _cursor_text.set_text("");
    }
    guint page_no = _notebook.page_num(*page);
    _notebook.remove_page(*page);
    _pages.erase(std::next(_pages.begin(), page_no));
}

void Graph_window::tab_change(Gtk::Widget * page, guint page_no)
{
    _cursor_conn.disconnect();

    _cursor_conn = (*(std::next(_pages.begin(), page_no)))->signal_cursor_moved().connect(sigc::mem_fun(*this, &Graph_window::update_cursor));
    dynamic_cast<Graph_page *>(page)->set_active();
}

void Graph_window::update_cursor(const std::string & text)
{
    _cursor_text.set_text(text);
}

    // TODO error handling
    // gl_window.graphs.push_back(std::unique_ptr<Graph>(new Graph_parametric("sin(v) * cos(u),sin(v) * sin(u),cos(v)", 0.0f, 2.0f * M_PI, 50, 0.0f, M_PI, 50))); // sphere
    // gl_window.graphs.push_back(std::unique_ptr<G:raph>(new Graph_parametric(
    //     "-2/15 * cos(u) * (3 * cos(v) - 30 * sin(u) + 90*cos(u)^4 * sin(u) - 60 * cos(u)^6 * sin(u) + 5 * cos(u) * cos(v) * sin(u)),"
    //     "-1/15 * sin(u) * (3 * cos(v) - 3 * cos(u)^2 * cos(v) - 48 * cos(u)^4 * cos(v) + 48 * cos(u)^6 * cos(v) - 60 * sin(u) + 5 * cos(u) * cos(v) * sin(u) -5 * cos(u)^3 * cos(v) * sin(u) - 80 * cos(u)^5 * cos(v) * sin(u) + 80 * cos(u)^7 * cos(v) * sin(u)),"
    //     "2/15 * (3 + 5 * cos(u) * sin(u)) * sin(v)",
    //     0.0f, M_PI, 50, 0.0f, 2.0f * M_PI, 50))); // klein bottle
    // gl_window.graphs.push_back(std::unique_ptr<Graph>(new Graph_parametric("(2 + cos(u/2) * cos(v) - sin(u/2) * sin(2*v)) * cos(u),(2 + cos(u/2) * cos(v) - sin(u/2) * sin(2*v)) * sin(u),sin(u/2) * cos(v) + cos(u/2) * sin(2*v)", 0.0f, 2.0f * M_PI, 50, 0.0f, 2.0f * M_PI, 50))); // klein bagel
    // gl_window.graphs.push_back(std::unique_ptr<Graph>(new Graph_parametric("(1 + v / 2 * cos(u / 2)) * cos(u),(1 + v / 2 * cos(u / 2)) * sin(u),v / 2 * sin(u / 2)", 0.0f, 2.0f * M_PI, 50, -1.0f, 1.0f, 50))); // möbius strip
    // gl_window.graphs.push_back(std::unique_ptr<Graph>(new Graph_parametric("(2 + .5 * cos(v)) * cos(u),(2 + .5 * cos(v)) * sin(u),.5 * sin(v)", 0.0f, 2.0f * M_PI, 50, 0.0f, 2.0f * M_PI, 50))); // torus
    // gl_window.graphs.push_back(std::unique_ptr<Graph>(new Graph_spherical("1", 0.0f, 2.0f * M_PI, 50, 0.0f, M_PI, 50)));
    // gl_window.graphs.push_back(std::unique_ptr<Graph>(new Graph_cylindrical("sqrt(r)", 0.0f, 10.0f, 50, 0.0f, 2.0f * M_PI, 50)));
    // gl_window.graphs.push_back(std::unique_ptr<Graph>(new Graph_cylindrical("-sqrt(r)", 0.0f, 10.0f, 50, 0.0f, 2.0f * M_PI, 50)));
    // gl_window.graphs.push_back(std::unique_ptr<Graph>(new Graph_cartesian("sqrt(1 - x^2 + y^2)", -2.0f, 2.0f, 50, -2.0f, 2.0f, 50)));
    // gl_window.graphs.push_back(std::unique_ptr<Graph>(new Graph_cartesian("-sqrt(1 - x^2 + y^2)", -2.0f, 2.0f, 50, -2.0f, 2.0f, 50)));
