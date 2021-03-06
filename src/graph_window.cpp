// graph_window.cpp
// windowing code. Using GTK to create the window, SFML to do openGL graphics.

// Copyright 2018 Matthew Chandler

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

#include <gtkmm/aboutdialog.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/grid.h>
#include <gtkmm/image.h>
#include <gtkmm/menubar.h>
#include <gtkmm/menu.h>
#include <gtkmm/menuitem.h>
#include <gtkmm/separator.h>

#include "config.hpp"
#include "graph_window.hpp"
#include "image_button.hpp"

extern int return_code; // from main.cpp

Graph_window::Graph_window():
    _gl_window(sf::VideoMode(800, 600), -1, sf::ContextSettings(24, 8, 8, 3, 3)),
    _draw_axes("Draw Axes"),
    _draw_cursor("Draw Cursor"),
    _use_orbit_cam("Use Orbiting Camera"),
    _use_free_cam("Use Free Camera")
{
    set_title(TITLE);
    set_default_size(800, 600);

    try
    {
        set_default_icon_from_file(check_in_pwd("icons/icon.svg"));
    }
    catch(Glib::Exception & e)
    {
        // try to get icon from theme
        set_default_icon_name("graph3");
    }

    // build menu
    Glib::RefPtr<Gtk::AccelGroup> accel_group = get_accel_group();

    Gtk::MenuBar * main_menu = Gtk::manage(new Gtk::MenuBar);

    // create menu items
    Gtk::MenuItem * file_menu_item = Gtk::manage(new Gtk::MenuItem("_File", true));
    Gtk::MenuItem * settings_menu_item = Gtk::manage(new Gtk::MenuItem("Se_ttings", true));
    Gtk::MenuItem * help_menu_item = Gtk::manage(new Gtk::MenuItem("_Help", true));
    main_menu->append(*file_menu_item);
    main_menu->append(*settings_menu_item);
    main_menu->append(*help_menu_item);

    Gtk::Menu * file_menu = Gtk::manage(new Gtk::Menu);
    Gtk::Menu * settings_menu = Gtk::manage(new Gtk::Menu);
    Gtk::Menu * help_menu = Gtk::manage(new Gtk::Menu);
    file_menu_item->set_submenu(*file_menu);
    settings_menu_item->set_submenu(*settings_menu);
    help_menu_item->set_submenu(*help_menu);

    // File Menu
    Gtk::MenuItem * file_save = Gtk::manage(new Gtk::MenuItem("_Save", true));
    file_menu->append(*file_save);
    file_save->signal_activate().connect(sigc::mem_fun(*this, &Graph_window::save_graph));
    file_save->add_accelerator("activate", accel_group, GDK_KEY_s, Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);

    Gtk::MenuItem * file_load = Gtk::manage(new Gtk::MenuItem("_Open", true));
    file_menu->append(*file_load);
    file_load->signal_activate().connect(sigc::mem_fun(*this, &Graph_window::load_graph));
    file_load->add_accelerator("activate", accel_group, GDK_KEY_o, Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);

    Gtk::MenuItem * file_quit = Gtk::manage(new Gtk::MenuItem("_Quit", true));
    file_menu->append(*file_quit);
    file_quit->signal_activate().connect(sigc::mem_fun(*this, &Graph_window::hide));
    file_quit->add_accelerator("activate", accel_group, GDK_KEY_q, Gdk::CONTROL_MASK, Gtk::ACCEL_VISIBLE);

    // Settings menu
    Gtk::MenuItem * settings_lights = Gtk::manage(new Gtk::MenuItem("_Lighting & Color", true));
    settings_menu->append(*settings_lights);
    settings_lights->signal_activate().connect(sigc::mem_fun(*this, &Graph_window::lighting));

    // Help Menu
    Gtk::MenuItem * help_about = Gtk::manage(new Gtk::MenuItem("_About", true));
    help_menu->append(*help_about);
    help_about->signal_activate().connect(sigc::mem_fun(*this, &Graph_window::about));

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

    main_grid->attach(*main_menu, 0, 0, 2, 1);
    main_grid->attach(*Gtk::manage(toolbar), 0, 1, 2, 1);

    // build toolbar
    Image_button * save_butt = Gtk::manage(new Image_button);
    save_butt->lbl.set_text_with_mnemonic("Sa_ve");
    save_butt->img.set_from_icon_name("document-save", Gtk::ICON_SIZE_SMALL_TOOLBAR);

    Image_button * load_butt = Gtk::manage(new Image_button);
    load_butt->lbl.set_text_with_mnemonic("_Open");
    load_butt->img.set_from_icon_name("document-open", Gtk::ICON_SIZE_SMALL_TOOLBAR);

    Image_button * reset_cam_butt = Gtk::manage(new Image_button);
    reset_cam_butt->lbl.set_text_with_mnemonic("_Reset Camera");
    reset_cam_butt->img.set_from_icon_name("view-refresh", Gtk::ICON_SIZE_SMALL_TOOLBAR);

    Gtk::Label * tool_sep = Gtk::manage(new Gtk::Label); // blank label for spacing
    tool_sep->set_hexpand(true);

    Image_button * add_butt = Gtk::manage(new Image_button);
    add_butt->lbl.set_text_with_mnemonic("Add _Graph");
    add_butt->img.set_from_icon_name("list-add", Gtk::ICON_SIZE_SMALL_TOOLBAR);

    toolbar->attach(*save_butt, 0, 0, 1, 1);
    toolbar->attach(*load_butt, 1, 0, 1, 1);
    toolbar->attach(*Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_VERTICAL)), 2, 0, 1, 1);
    toolbar->attach(_draw_axes, 3, 0, 1, 1);
    toolbar->attach(_draw_cursor, 4, 0, 1, 1);
    toolbar->attach(*Gtk::manage(new Gtk::Separator(Gtk::ORIENTATION_VERTICAL)), 5, 0, 1, 1);
    toolbar->attach(_use_orbit_cam, 6, 0, 1, 1);
    toolbar->attach(_use_free_cam, 7, 0, 1, 1);
    toolbar->attach(*reset_cam_butt, 8, 0, 1, 1);
    toolbar->attach(*tool_sep, 9, 0, 1, 1);
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

    reset_cam_butt->signal_clicked().connect(sigc::mem_fun(_gl_window, &Graph_disp::reset_cam));

    // signal when new page is requested
    add_butt->signal_clicked().connect(sigc::mem_fun(*this, &Graph_window::tab_new));

    // signal when page is changed
    _notebook.signal_switch_page().connect(sigc::mem_fun(*this, &Graph_window::tab_change));

    // signal when setup is complete - ready to open files
    _gl_window.signal_initialized().connect(sigc::mem_fun(*this, &Graph_window::open_startup_files));

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

// give a list of files to open at startup
void Graph_window::open_at_startup(const std::vector<std::string> & filenames)
{
    _startup_files = filenames;
}

// global vars to track last selected file
std::string curr_dir = "";
std::string curr_file = "";

// select file to save to
void Graph_window::save_graph()
{
    // create file chooser
    Gtk::FileChooserDialog graph_chooser("Save Graph", Gtk::FileChooserAction::FILE_CHOOSER_ACTION_SAVE);
    graph_chooser.set_transient_for(*this);

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

    graph_chooser.add_button("Cancel", Gtk::RESPONSE_CANCEL);
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
    graph_chooser.set_transient_for(*this);

    graph_chooser.set_select_multiple(true);

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

    graph_chooser.add_button("Cancel", Gtk::RESPONSE_CANCEL);
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
    std::vector<std::string> filenames = graph_chooser.get_filenames();
    curr_dir = graph_chooser.get_current_folder();
    curr_file = filenames[0].substr(curr_dir.size() + 1);
    for(auto & filename: filenames)
    {
        int current_tab = _notebook.get_current_page();

        // create a new Graph_page and graph from file
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
    Dir_light dir_light = _gl_window.dir_light;
    Point_light cam_light = _gl_window.cam_light;
    glm::vec3 bkg_color = _gl_window.bkg_color;
    glm::vec3 ambient_color = _gl_window.ambient_color;

    Lighting_window light_win(dir_light, cam_light, bkg_color, ambient_color);
    light_win.set_modal(true);
    light_win.set_transient_for(*this);
    int response = light_win.run();
    if(response == Gtk::RESPONSE_OK)
    {
        _gl_window.dir_light = dir_light;
        _gl_window.cam_light = cam_light;
        _gl_window.bkg_color = bkg_color;
        _gl_window.ambient_color = ambient_color;
    }
}

void Graph_window::about()
{
    Gtk::AboutDialog about;
    about.set_modal(true);
    about.set_transient_for(*this);

    try
    {
        about.set_icon_from_file(check_in_pwd("icons/icon.svg"));
    }
    catch(Glib::Exception & e)
    {
        // try to get logo from theme
        about.set_logo_icon_name("graph3");
    }

    about.set_program_name(TITLE);
    #ifdef NDEBUG
    about.set_version(VERSION_SHORT);
    #else
    about.set_version(VERSION_FULL " (debug)");
    #endif
    about.set_copyright(u8"© " COPYRIGHT);
    about.set_comments(SUMMARY);
    about.set_license_type(Gtk::LICENSE_MIT_X11);
    about.set_website(WEBSITE);
    about.set_website_label(TITLE " on Github");
    about.set_authors({AUTHOR});
    about.set_artists({AUTHOR});
    about.set_documenters({AUTHOR});
    about.run();
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

// open files passed as program parameters
void Graph_window::open_startup_files()
{
    bool success = false;
    int first_tab = _notebook.get_current_page();
    for(auto & filename: _startup_files)
    {
        int current_tab = _notebook.get_current_page();

        // create a new Graph_page and graph from file
        tab_new();
        _notebook.set_current_page(_notebook.get_n_pages() - 1);
        Graph_page & new_tab = dynamic_cast<Graph_page &>(*_notebook.get_nth_page(_notebook.get_current_page()));

        if(!new_tab.load_graph(filename))
        {
            // revert
            tab_close(new_tab);
            _notebook.set_current_page(current_tab);
        }
        else
            success = true;
    }
    if(success)
        tab_close(dynamic_cast<Graph_page &>(*_notebook.get_nth_page(first_tab)));
    _startup_files.clear();
}
