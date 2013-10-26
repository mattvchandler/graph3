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

#include <gtkmm/filechooserdialog.h>
#include <gtkmm/stock.h>

#include "graph_window.hpp"

Graph_window::Graph_window():
    _gl_window(sf::VideoMode(800, 600), -1)
{
    set_title("Graph 3");
    set_default_size(800, 600);

    // build menu
    _menu_act = Gtk::ActionGroup::create();
    _menu_act->add(Gtk::Action::create("File", "File"));

    _menu_act->add(Gtk::Action::create("File_save", Gtk::Stock::SAVE, "_Save", "Save"), sigc::mem_fun(*this, &Graph_window::save_graph));
    _menu_act->add(Gtk::Action::create("File_open", Gtk::Stock::OPEN, "_Open", "Open"), sigc::mem_fun(*this, &Graph_window::load_graph));
    _menu_act->add(Gtk::Action::create("File_quit", Gtk::Stock::QUIT, "_Quit", "Quit"), sigc::mem_fun(*this, &Graph_window::hide));

    _menu_act->add(Gtk::Action::create("Display", "Display"));

    _draw_axes = Gtk::ToggleAction::create("Display_draw_axes", "Draw Axes", "Enable / Disable drawing of axes", true);
    _draw_cursor = Gtk::ToggleAction::create("Display_draw_cursor", "Draw Cursor", "Enable / Disable drawing of cursors", true);
    _menu_act->add(_draw_axes, sigc::mem_fun(*this, &Graph_window::change_flags));
    _menu_act->add(_draw_cursor, sigc::mem_fun(*this, &Graph_window::change_flags));

    Gtk::RadioAction::Group cam_g;
    _use_orbit_cam = Gtk::RadioAction::create(cam_g, "Display_orbit_cam", "Orbital camera", "Use orbital camera");
    _use_free_cam = Gtk::RadioAction::create(cam_g, "Display_free_cam", "Free camera", "Use free camera");
    _menu_act->add(_use_orbit_cam, sigc::mem_fun(*this, &Graph_window::change_flags));
    _menu_act->add(_use_free_cam, sigc::mem_fun(*this, &Graph_window::change_flags));

    _menu_act->add(Gtk::Action::create("Toolbar_add", Gtk::Stock::ADD, "Add Graph", "Add new graph"), sigc::mem_fun(*this, &Graph_window::tab_new));
    _menu_act->get_action("Toolbar_add")->set_is_important(true);

    _menu = Gtk::UIManager::create();
    _menu->insert_action_group(_menu_act);
    add_accel_group(_menu->get_accel_group());

    Glib::ustring menu_str =
    "<ui>"
    "   <menubar name='Menubar'>"
    "       <menu action='File'>"
    "           <menuitem action='File_save'/>"
    "           <menuitem action='File_open'/>"
    "           <separator/>"
    "           <menuitem action='File_quit'/>"
    "       </menu>"
    "       <menu action='Display'>"
    "           <menuitem action='Display_draw_axes'/>"
    "           <menuitem action='Display_draw_cursor'/>"
    "           <separator/>"
    "           <menuitem action='Display_orbit_cam'/>"
    "           <menuitem action='Display_free_cam'/>"
    "       </menu>"
    "   </menubar>"
    "   <toolbar name='Toolbar'>"
    "       <toolitem action='File_save'/>"
    "       <toolitem action='File_open'/>"
    "       <separator expand='true'/>"
    "       <toolitem action='Toolbar_add'/>"
    "   </toolbar>"
    "</ui>";

    try
    {
        _menu->add_ui_from_string(menu_str);
    }
    catch(const Glib::Error & e)
    {
        std::cerr<<"Error building menu: "<<e.what()<<std::endl;
        get_application()->quit();
    }

    _gl_window.set_hexpand(true);
    _gl_window.set_vexpand(true);

    _notebook.set_vexpand(true);
    _notebook.set_scrollable(true);

    _cursor_text.set_halign(Gtk::ALIGN_CENTER);

    _main_grid.set_border_width(3);
    _main_grid.set_row_spacing(3);
    _main_grid.set_column_spacing(3);

    add(_main_grid);

    _main_grid.attach(*_menu->get_widget("/Menubar"), 0, 0, 2, 1);
    _main_grid.attach(*_menu->get_widget("/Toolbar"), 0, 1, 2, 1);

    _main_grid.attach(_gl_window, 0, 2, 1, 1);
    _main_grid.attach(_notebook, 1, 2, 1, 1);
    _main_grid.attach(_cursor_text, 0, 3, 2, 1);

    _notebook.signal_switch_page().connect(sigc::mem_fun(*this, &Graph_window::tab_change));

    show_all_children();

    _pages.push_back(std::unique_ptr<Graph_page>(new Graph_page(_gl_window)));
    _notebook.append_page(*_pages.back(), *Gtk::manage(new Tab_label));

    dynamic_cast<Tab_label &>(*_notebook.get_tab_label(*_pages.back())).
        signal_close_tab().connect(sigc::bind<Graph_page &>(sigc::mem_fun(*this, &Graph_window::tab_close),
        *_pages.back()));
    dynamic_cast<Graph_page &>(*_pages.back()).signal_tex_changed()
        .connect(sigc::mem_fun(dynamic_cast<Tab_label &>(*_notebook.get_tab_label(*_pages.back())),
        &Tab_label::set_img));
    _pages.back()->show();

    _gl_window.invalidate();
}

void Graph_window::tab_new()
{
    _pages.push_back(std::unique_ptr<Graph_page>(new Graph_page(_gl_window)));
    _notebook.append_page(*_pages.back(), *Gtk::manage(new Tab_label));

    dynamic_cast<Tab_label &>(*_notebook.get_tab_label(*_pages.back())).
        signal_close_tab().connect(sigc::bind<Graph_page &>(sigc::mem_fun(*this, &Graph_window::tab_close),
        *_pages.back()));
    dynamic_cast<Graph_page &>(*_pages.back()).signal_tex_changed()
        .connect(sigc::mem_fun(dynamic_cast<Tab_label &>(*_notebook.get_tab_label(*_pages.back())),
        &Tab_label::set_img));

    _pages.back()->show();
}

void Graph_window::tab_close(Graph_page & page)
{
    if(_notebook.get_n_pages() == 1)
    {
        if(_cursor_conn.connected())
            _cursor_conn.disconnect();
        _cursor_text.set_text("");
    }
    guint page_no = _notebook.page_num(page);
    _notebook.remove_page(page);
    _pages.erase(std::next(_pages.begin(), page_no));
}

void Graph_window::tab_change(Gtk::Widget * page, guint page_no)
{
    if(_cursor_conn.connected())
        _cursor_conn.disconnect();

    _cursor_conn = dynamic_cast<Graph_page &>(*page).signal_cursor_moved().connect(sigc::mem_fun(*this, &Graph_window::update_cursor));

    dynamic_cast<Graph_page &>(*page).set_active();
}

void Graph_window::change_flags()
{
    _gl_window.draw_axes_flag = _draw_axes->get_active();
    _gl_window.draw_cursor_flag = _draw_cursor->get_active();

    if(!_draw_cursor->get_active())
        update_cursor("");
    else
        dynamic_cast<Graph_page &>(*_notebook.get_nth_page(_notebook.get_current_page())).set_active();

    _gl_window.use_orbit_cam = _use_orbit_cam->get_active();

    _gl_window.invalidate();
}

void Graph_window::update_cursor(const std::string & text)
{
    _cursor_text.set_text(text);
}

std::string curr_dir = "";
std::string curr_file = "";

void Graph_window::save_graph()
{
    Gtk::FileChooserDialog graph_chooser("Save Graph", Gtk::FileChooserAction::FILE_CHOOSER_ACTION_SAVE);
    Glib::RefPtr<Gtk::FileFilter> graph_types;
    Glib::RefPtr<Gtk::FileFilter> all_types;

    all_types = Gtk::FileFilter::create();
    all_types->add_pattern("*.gra");
    all_types->set_name("Graph files (*.gra)");
    graph_chooser.add_filter(all_types);

    all_types = Gtk::FileFilter::create();
    all_types->add_pattern("*");
    all_types->set_name("All files");
    graph_chooser.add_filter(all_types);

    if(!curr_dir.empty())
        graph_chooser.set_current_folder(curr_dir);
    else
        graph_chooser.set_current_folder(".");

    if(!curr_file.empty())
        graph_chooser.set_current_name(curr_file);
    else
        graph_chooser.set_current_name(".gra");

    graph_chooser.set_create_folders(true);

    graph_chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    graph_chooser.add_button("Save", Gtk::RESPONSE_OK);

    graph_chooser.set_do_overwrite_confirmation(true);

    if(graph_chooser.run() != Gtk::RESPONSE_OK)
        return;

    std::string filename = graph_chooser.get_filename();
    curr_dir = graph_chooser.get_current_folder();
    curr_file = filename.substr(curr_dir.size() + 1);

    dynamic_cast<Graph_page &>(*_notebook.get_nth_page(_notebook.get_current_page())).save_graph(filename);
}

void Graph_window::load_graph()
{
    Gtk::FileChooserDialog graph_chooser("Open Graph", Gtk::FileChooserAction::FILE_CHOOSER_ACTION_OPEN);
    Glib::RefPtr<Gtk::FileFilter> graph_types;
    Glib::RefPtr<Gtk::FileFilter> all_types;

    all_types = Gtk::FileFilter::create();
    all_types->add_pattern("*.gra");
    all_types->set_name("Graph files (*.gra)");
    graph_chooser.add_filter(all_types);

    all_types = Gtk::FileFilter::create();
    all_types->add_pattern("*");
    all_types->set_name("All files");
    graph_chooser.add_filter(all_types);

    graph_chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    graph_chooser.add_button("Select", Gtk::RESPONSE_OK);

    graph_chooser.add_shortcut_folder("./examples");

    if(!curr_dir.empty())
    {
        if(!curr_file.empty())
            graph_chooser.select_filename(curr_dir + "/" + curr_file);
        else
            graph_chooser.set_current_folder(curr_dir);
    }
    else
        graph_chooser.set_current_folder(".");

    if(graph_chooser.run() != Gtk::RESPONSE_OK)
        return;

    std::string filename = graph_chooser.get_filename();
    curr_dir = graph_chooser.get_current_folder();
    curr_file = filename.substr(curr_dir.size() + 1);

    int current_tab = _notebook.get_current_page();

    tab_new();
    _notebook.set_current_page(_notebook.get_n_pages() - 1);
    Graph_page & new_tab = dynamic_cast<Graph_page &>(*_notebook.get_nth_page(_notebook.get_current_page()));
    if(!new_tab.load_graph(filename))
    {
       tab_close(new_tab);
       _notebook.set_current_page(current_tab);
    }
}
