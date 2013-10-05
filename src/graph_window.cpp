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

    set_img_from_color(Graph_page::start_color);

    attach(_tab_pic, 0, 0, 1, 1);
    attach(_close_butt, 1, 0, 1, 1);

    _close_butt.signal_clicked().connect(sigc::mem_fun(*this, &Tab_label::on_button_press));

    show_all_children();
}

void Tab_label::set_img_from_color(const glm::vec3 & color)
{
    guint8 r = (guint8)(color.r * 256.0f);
    guint8 g = (guint8)(color.g * 256.0f);
    guint8 b = (guint8)(color.b * 256.0f);

    guint32 hex_color = r << 24 | g << 16 | b << 8;

    Glib::RefPtr<Gdk::Pixbuf> image = Gdk::Pixbuf::create(Gdk::Colorspace::COLORSPACE_RGB, false, 8, 16, 16);
    image->fill(hex_color);

    _tab_pic.set(image);
}

void Tab_label::set_img(const std::string & filename)
{
    try
    {
        _tab_pic.set(Gdk::Pixbuf::create_from_file(filename)->scale_simple(16, 16, Gdk::InterpType::INTERP_BILINEAR));
    }
    catch(Glib::Exception &e)
    {
        _tab_pic.set(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_MENU);
    }
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
    _add_tab_butt(Gtk::Stock::ADD),
    _draw_axes("Draw Axes"),
    _draw_cursor("Draw Cursor")
{
    set_title("Graph 3");
    set_default_size(800, 600);

    _gl_window.set_hexpand(true);
    _gl_window.set_vexpand(true);

    _notebook.set_vexpand(true);
    _notebook.set_scrollable(true);

    add(_main_grid);

    _main_grid.attach(_gl_window, 0, 0, 1, 2);
    _main_grid.attach(_cursor_text, 0, 2, 1, 1);

    _main_grid.attach(_add_tab_butt, 1, 0, 1, 1);
    _main_grid.attach(_notebook, 1, 1, 2, 1);
    _main_grid.attach(_draw_axes, 1, 2, 1, 1);
    _main_grid.attach(_draw_cursor, 2, 2, 1, 1);

    _add_tab_butt.signal_clicked().connect(sigc::mem_fun(*this, &Graph_window::tab_new));
    _notebook.signal_switch_page().connect(sigc::mem_fun(*this, &Graph_window::tab_change));

    show_all_children();

    _pages.push_back(std::unique_ptr<Graph_page>(new Graph_page(&_gl_window)));
    _notebook.append_page(*_pages.back(), *Gtk::manage(new Tab_label));

    dynamic_cast<Tab_label *>(_notebook.get_tab_label(*_pages.back()))
        ->signal_close_tab().connect(sigc::bind<Graph_page *>(sigc::mem_fun(*this, &Graph_window::tab_close),
        _pages.back().get()));
    dynamic_cast<Graph_page *>(_pages.back().get())->signal_color_changed()
        .connect(sigc::mem_fun(*dynamic_cast<Tab_label *>(_notebook.get_tab_label(*_pages.back())),
        &Tab_label::set_img_from_color));
    dynamic_cast<Graph_page *>(_pages.back().get())->signal_tex_changed()
        .connect(sigc::mem_fun(*dynamic_cast<Tab_label *>(_notebook.get_tab_label(*_pages.back())),
        &Tab_label::set_img));
    _pages.back()->show();

    _draw_axes.set_active(true);
    _draw_cursor.set_active(true);

    _draw_axes.signal_toggled().connect(sigc::mem_fun(*this, &Graph_window::change_flags));
    _draw_cursor.signal_toggled().connect(sigc::mem_fun(*this, &Graph_window::change_flags));

    _gl_window.invalidate();
}

void Graph_window::tab_new()
{
    _pages.push_back(std::unique_ptr<Graph_page>(new Graph_page(&_gl_window)));
    _notebook.append_page(*_pages.back(), *Gtk::manage(new Tab_label));

    dynamic_cast<Tab_label *>(_notebook.get_tab_label(*_pages.back()))
        ->signal_close_tab().connect(sigc::bind<Graph_page *>(sigc::mem_fun(*this, &Graph_window::tab_close),
        _pages.back().get()));
    dynamic_cast<Graph_page *>(_pages.back().get())->signal_color_changed()
        .connect(sigc::mem_fun(*dynamic_cast<Tab_label *>(_notebook.get_tab_label(*_pages.back())),
        &Tab_label::set_img_from_color));
    dynamic_cast<Graph_page *>(_pages.back().get())->signal_tex_changed()
        .connect(sigc::mem_fun(*dynamic_cast<Tab_label *>(_notebook.get_tab_label(*_pages.back())),
        &Tab_label::set_img));

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

void Graph_window::change_flags()
{
    _gl_window.draw_axes_flag = _draw_axes.get_active();
    _gl_window.draw_cursor_flag = _draw_cursor.get_active();

    if(!_draw_cursor.get_active())
        update_cursor("");
    else
        dynamic_cast<Graph_page *>(_notebook.get_nth_page(_notebook.get_current_page()))->set_active();

    _gl_window.invalidate();
}

void Graph_window::update_cursor(const std::string & text)
{
    _cursor_text.set_text(text);
}
