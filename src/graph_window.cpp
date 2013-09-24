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
#include <gtkmm/stock.h>

#include "graph_window.hpp"

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

    _add_tab_butt.signal_clicked().connect(sigc::mem_fun(*this, &Graph_window::tab_new));
    _notebook.signal_switch_page().connect(sigc::mem_fun(*this, &Graph_window::tab_change));

    show_all_children();

    _pages.push_back(std::unique_ptr<Graph_page>(new Graph_page(&_gl_window)));
    _notebook.append_page(*_pages.back(), *Gtk::manage(new Tab_label));

    dynamic_cast<Tab_label *>(_notebook.get_tab_label(*_pages.back()))
        ->signal_close_tab().connect(sigc::bind<Graph_page *>(sigc::mem_fun(*this, &Graph_window::tab_close),
        _pages.back().get()));
    _pages.back()->show();

    _gl_window.invalidate();
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
        if(_cursor_conn.connected())
            _cursor_conn.disconnect();
        _cursor_text.set_text("");
    }
    guint page_no = _notebook.page_num(*page);
    _notebook.remove_page(*page);
    _pages.erase(std::next(_pages.begin(), page_no));
}

void Graph_window::tab_change(Gtk::Widget * page, guint page_no)
{
    if(_cursor_conn.connected())
        _cursor_conn.disconnect();

    _cursor_conn = dynamic_cast<Graph_page *>(page)->signal_cursor_moved().connect(sigc::mem_fun(*this, &Graph_window::update_cursor));

    dynamic_cast<Graph_page *>(page)->set_active();
}

void Graph_window::update_cursor(const std::string & text)
{
    _cursor_text.set_text(text);
}
