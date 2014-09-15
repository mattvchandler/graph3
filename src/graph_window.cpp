// graph_window.cpp
// windowing code. Using GTK to create the window, SFML to do openGL graphics.

// Copyright 2014 Matthew Chandler

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

#include <glibmm/exception.h>

#include <gtkmm/button.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/grid.h>
#include <gtkmm/image.h>
#include <gtkmm/separator.h>
#include <gtkmm/stock.h>

#include "graph_window.hpp"
#include "config.hpp"

extern int return_code; // from main.cpp

Graph_window::Graph_window():
    _gl_window(sf::VideoMode(800, 600), -1),
    _draw_axes("Draw Axes"),
    _draw_cursor("Draw Cursor"),
    _use_orbit_cam("Use Orbiting Camera"),
    _use_free_cam("Use Free Camera")
{
    set_title(GRAPH_TITLE);
    set_default_size(800, 600);

    try
    {
        set_default_icon_from_file(check_in_pwd("icons/icon.svg"));
    }
    catch(Glib::Exception & e)
    {
        // do nothing on errors. use default "broken image" ico if we can't load any icons
    }

    // build menu
    _menu_act = Gtk::ActionGroup::create();

    _menu_act->add(Gtk::Action::create("File", "File"));
    _menu_act->add(Gtk::Action::create("File_save", Gtk::Stock::SAVE, "_Save", "Save"), sigc::mem_fun(*this, &Graph_window::save_graph));
    _menu_act->add(Gtk::Action::create("File_open", Gtk::Stock::OPEN, "_Open", "Open"), sigc::mem_fun(*this, &Graph_window::load_graph));
    _menu_act->add(Gtk::Action::create("File_quit", Gtk::Stock::QUIT, "_Quit", "Quit"), sigc::mem_fun(*this, &Graph_window::hide));

    _menu_act->add(Gtk::Action::create("Settings", "Settings"));
    _menu_act->add(Gtk::Action::create("Settings_lighting", "_Lighting", "Lighting"), sigc::mem_fun(*this, &Graph_window::lighting));

    _menu_act->add(Gtk::Action::create("Help", "Help"));
    _menu_act->add(Gtk::Action::create("Help_about", "About", "About"), sigc::mem_fun(*this, &Graph_window::hide));

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
    "       <menu action='Settings'>"
    "           <menuitem action='Settings_lighting'/>"
    "       </menu>"
    "       <menu action='Help'>"
    "           <menuitem action='Help_about'/>"
    "       </menu>"
    "   </menubar>"
    "</ui>";

    try
    {
        _menu->add_ui_from_string(menu_str);
    }
    catch(const Glib::Error & e)
    {
        std::cerr<<"Error building menu: "<<e.what()<<std::endl;
        return_code = EXIT_FAILURE;
        throw;
    }

    // widget layout
    _gl_window.set_hexpand(true);
    _gl_window.set_vexpand(true);

    _notebook.set_vexpand(true);
    _notebook.set_scrollable(true);

    _cursor_text.set_halign(Gtk::ALIGN_CENTER);

    Gtk::Grid * main_grid = new Gtk::Grid;
    Gtk::Grid * toolbar = new Gtk::Grid;

    main_grid->set_border_width(3);
    main_grid->set_row_spacing(3);
    main_grid->set_column_spacing(3);
    toolbar->set_border_width(3);
    toolbar->set_column_spacing(3);

    add(*Gtk::manage(main_grid));

    main_grid->attach(*_menu->get_widget("/Menubar"), 0, 0, 2, 1);
    main_grid->attach(*Gtk::manage(toolbar), 0, 1, 2, 1);

    // build toolbar
    Gtk::Button * save_butt = Gtk::manage(new Gtk::Button(Gtk::Stock::SAVE));
    Gtk::Button * load_butt = Gtk::manage(new Gtk::Button(Gtk::Stock::OPEN));
    Gtk::Button * reset_cam_butt = Gtk::manage(new Gtk::Button("Reset Camera"));

    toolbar->attach(*save_butt, 0, 0, 1, 1);
    toolbar->attach(*load_butt, 1, 0, 1, 1);
    toolbar->attach(*Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_VERTICAL)), 2, 0, 1, 1);
    toolbar->attach(_draw_axes, 3, 0, 1, 1);
    toolbar->attach(_draw_cursor, 4, 0, 1, 1);
    toolbar->attach(*Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_VERTICAL)), 5, 0, 1, 1);
    toolbar->attach(_use_orbit_cam, 6, 0, 1, 1);
    toolbar->attach(_use_free_cam, 7, 0, 1, 1);
    toolbar->attach(*reset_cam_butt, 8, 0, 1, 1);

    Gtk::Label * tool_sep = Gtk::manage(new Gtk::Label); // blank label for spacing
    tool_sep->set_hexpand(true);
    toolbar->attach(*tool_sep, 9, 0, 1, 1);

    Gtk::Button * add_butt = Gtk::manage(new Gtk::Button(Gtk::Stock::ADD));
    toolbar->attach(*add_butt, 10, 0, 1, 1);

    main_grid->attach(_gl_window, 0, 2, 1, 1);
    main_grid->attach(_notebook, 1, 2, 1, 1);
    main_grid->attach(_cursor_text, 0, 3, 2, 1);

    save_butt->signal_clicked().connect(sigc::mem_fun(*this, &Graph_window::save_graph));
    load_butt->signal_clicked().connect(sigc::mem_fun(*this, &Graph_window::load_graph));

    _draw_axes.set_active(true);
    _draw_cursor.set_active(true);

    _draw_axes.signal_toggled().connect(sigc::mem_fun(*this, &Graph_window::change_flags));
    _draw_cursor.signal_toggled().connect(sigc::mem_fun(*this, &Graph_window::change_flags));

    Gtk::RadioButton::Group cam_g = _use_orbit_cam.get_group();
    _use_free_cam.set_group(cam_g);

    _use_free_cam.signal_toggled().connect(sigc::mem_fun(*this, &Graph_window::change_flags));
    _use_orbit_cam.signal_toggled().connect(sigc::mem_fun(*this, &Graph_window::change_flags));

    reset_cam_butt->set_image(*Gtk::manage(new Gtk::Image(Gtk::Stock::REFRESH, Gtk::ICON_SIZE_SMALL_TOOLBAR)));
    reset_cam_butt->signal_clicked().connect(sigc::mem_fun(_gl_window, &Graph_disp::reset_cam));

    // signal when new page is requested
    add_butt->signal_clicked().connect(sigc::mem_fun(*this, &Graph_window::tab_new));

    // signal when page is changed
    _notebook.signal_switch_page().connect(sigc::mem_fun(*this, &Graph_window::tab_change));

    show_all_children();

    // create a starting page
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

// global vars to track last selected file
std::string curr_dir = "";
std::string curr_file = "";

// select file to save to
void Graph_window::save_graph()
{
    // create file chooser
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

    // try to set last selected file & directory
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

    // save selected filename & directory
    std::string filename = graph_chooser.get_filename();
    curr_dir = graph_chooser.get_current_folder();
    curr_file = filename.substr(curr_dir.size() + 1);

    // have the graph page save its graph
    dynamic_cast<Graph_page &>(*_notebook.get_nth_page(_notebook.get_current_page())).save_graph(filename);
}

// select file to load from
void Graph_window::load_graph()
{
    // create file chooser
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

    graph_chooser.add_shortcut_folder(check_in_pwd("examples"));

    // try to set last selected file & directory
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

    // save selected filename & directory
    std::string filename = graph_chooser.get_filename();
    curr_dir = graph_chooser.get_current_folder();
    curr_file = filename.substr(curr_dir.size() + 1);

    int current_tab = _notebook.get_current_page();

    // create a new Grap_page and graph from file
    tab_new();
    _notebook.set_current_page(_notebook.get_n_pages() - 1);
    Graph_page & new_tab = dynamic_cast<Graph_page &>(*_notebook.get_nth_page(_notebook.get_current_page()));

    if(!new_tab.load_graph(filename))
    {
        // revert
        tab_close(new_tab);
        _notebook.set_current_page(current_tab);
    }
}

// called when checkbox or radio buttons are pressed
void Graph_window::change_flags()
{
    // pass changes to GL display
    _gl_window.draw_axes_flag = _draw_axes.get_active();
    _gl_window.draw_cursor_flag = _draw_cursor.get_active();

    if(!_draw_cursor.get_active())
        update_cursor("");
    else
    {
        int cur_page = _notebook.get_current_page();
        if(cur_page >= 0)
            dynamic_cast<Graph_page &>(*_notebook.get_nth_page(cur_page)).set_active();
    }

    _gl_window.use_orbit_cam = _use_orbit_cam.get_active();

    _gl_window.invalidate();
}

void Graph_window::lighting()
{
    Light dir_light = _gl_window.dir_light;
    Light cam_light = _gl_window.cam_light;
    Lighting_window light_win(dir_light, cam_light);
    light_win.set_modal(true);
    light_win.set_transient_for(*this);
    int response = light_win.run();
    if(response == Gtk::RESPONSE_OK)
    {
        _gl_window.dir_light = dir_light;
        _gl_window.cam_light = cam_light;
    }
}

// update cursor text
void Graph_window::update_cursor(const std::string & text)
{
    _cursor_text.set_text(text);
}

// create a new graph page
void Graph_window::tab_new()
{
    // create and associate a new Graph_page
    _pages.push_back(std::unique_ptr<Graph_page>(new Graph_page(_gl_window)));
    _notebook.append_page(*_pages.back(), *Gtk::manage(new Tab_label));

    // connect close signal
    dynamic_cast<Tab_label &>(*_notebook.get_tab_label(*_pages.back())).
        signal_close_tab().connect(sigc::bind<Graph_page &>(sigc::mem_fun(*this, &Graph_window::tab_close),
        *_pages.back()));
    // connect texture change signal
    dynamic_cast<Graph_page &>(*_pages.back()).signal_tex_changed()
        .connect(sigc::mem_fun(dynamic_cast<Tab_label &>(*_notebook.get_tab_label(*_pages.back())),
        &Tab_label::set_img));

    _pages.back()->show();
}

// close a graph page and delete the graph
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

// change active graph
void Graph_window::tab_change(Gtk::Widget * page, guint page_no)
{
    // disconnect existing cursor signal
    if(_cursor_conn.connected())
        _cursor_conn.disconnect();

    // connect the new one
    _cursor_conn = dynamic_cast<Graph_page &>(*page).signal_cursor_moved().connect(sigc::mem_fun(*this, &Graph_window::update_cursor));

    // tell the page that it is now active
    dynamic_cast<Graph_page &>(*page).set_active();
}
