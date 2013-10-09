// graph_page.cpp
// widgets for creating graphs

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

#include <sstream>

#include <gtkmm/colorchooserdialog.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

#include <gdkmm.h>

#include <libconfig.h++>

#include "graph_page.hpp"

#include "graph.hpp"
#include "graph_cartesian.hpp"
#include "graph_cylindrical.hpp"
#include "graph_spherical.hpp"
#include "graph_parametric.hpp"

const glm::vec3 Graph_page::start_color = glm::vec3(0.2f, 0.5f, 0.2f);

Graph_page::Graph_page(Graph_disp * gl_window): _gl_window(gl_window), _graph(nullptr),
    _r_car("Cartesian"),
    _r_cyl("Cylindrical"),
    _r_sph("Spherical"),
    _r_par("Parametric"),
    _eqn_l("z(x,y)="),
    _eqn_par_y_l("y(u,v)="),
    _eqn_par_z_l("z(u,v)="),
    _row_min_l("x min="),
    _row_max_l("x max="),
    _col_min_l("y min="),
    _col_max_l("y max="),
    _row_res_l("x resolution="),
    _col_res_l("y resolution="),
    _row_res(Gtk::Adjustment::create(50.0, 1.0, 1000.0)),
    _col_res(Gtk::Adjustment::create(50.0, 1.0, 1000.0)),
    _draw("Draw Graph"),
    _draw_grid("Draw Gridlines"),
    _draw_normals("Draw normals"),
    _use_color("Use Color"),
    _use_tex("Use Texture"),
    _tex_butt("Choose Color"),
    _apply_butt(Gtk::Stock::APPLY),
    _tex_ico(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_LARGE_TOOLBAR),
    _color(start_color)
{
    attach(_r_car, 0, 1, 2, 1);
    attach(_r_cyl, 2, 1, 2, 1);
    attach(_r_sph, 0, 2, 2, 1);
    attach(_r_par, 2, 2, 2, 1);
    attach(_eqn_l, 0, 3, 1, 1);
    attach(_eqn, 1, 3, 3, 1);
    attach(_eqn_par_y_l, 0, 4, 1, 1);
    attach(_eqn_par_y, 1, 4, 3, 1);
    attach(_eqn_par_z_l, 0, 5, 1, 1);
    attach(_eqn_par_z, 1, 5, 3, 1);
    attach(_row_min_l, 0, 6, 1, 1);
    attach(_row_min, 1, 6, 1, 1);
    attach(_row_max_l, 2, 6, 1, 1);
    attach(_row_max, 3, 6, 1, 1);
    attach(_col_min_l, 0, 7, 1, 1);
    attach(_col_min, 1, 7, 1, 1);
    attach(_col_max_l, 2, 7, 1, 1);
    attach(_col_max, 3, 7, 1, 1);
    attach(_row_res_l, 0, 8, 1, 1);
    attach(_row_res, 1, 8, 1, 1);
    attach(_col_res_l, 2, 8, 1, 1);
    attach(_col_res, 3, 8, 1, 1);
    attach(_draw, 0, 9, 1, 1);
    attach(_draw_grid, 1, 9, 1, 1);
    attach(_draw_normals, 2, 9, 2, 1);
    attach(_use_color, 0, 10, 2, 1);
    attach(_use_tex, 0, 11, 2, 1);
    attach(_tex_butt, 2, 10, 2, 2);
    attach(_apply_butt, 3, 12, 1, 1);

    Gtk::RadioButton::Group type_g = _r_car.get_group();
    _r_cyl.set_group(type_g);
    _r_sph.set_group(type_g);
    _r_par.set_group(type_g);

    _r_car.signal_toggled().connect(sigc::mem_fun(*this, &Graph_page::change_type));
    _r_cyl.signal_toggled().connect(sigc::mem_fun(*this, &Graph_page::change_type));
    _r_sph.signal_toggled().connect(sigc::mem_fun(*this, &Graph_page::change_type));
    _r_par.signal_toggled().connect(sigc::mem_fun(*this, &Graph_page::change_type));

    _eqn.signal_activate().connect(sigc::mem_fun(*this, &Graph_page::apply));
    _eqn_par_y.signal_activate().connect(sigc::mem_fun(*this, &Graph_page::apply));
    _eqn_par_z.signal_activate().connect(sigc::mem_fun(*this, &Graph_page::apply));
    _row_min.signal_activate().connect(sigc::mem_fun(*this, &Graph_page::apply));
    _row_max.signal_activate().connect(sigc::mem_fun(*this, &Graph_page::apply));
    _col_min.signal_activate().connect(sigc::mem_fun(*this, &Graph_page::apply));
    _col_max.signal_activate().connect(sigc::mem_fun(*this, &Graph_page::apply));
    _row_res.signal_activate().connect(sigc::mem_fun(*this, &Graph_page::apply));
    _col_res.signal_activate().connect(sigc::mem_fun(*this, &Graph_page::apply));

    _eqn.set_placeholder_text("z(x,y)");
    _eqn_par_y.set_placeholder_text("y(u,v)");
    _eqn_par_z.set_placeholder_text("z(u,v)");
    _row_min.set_placeholder_text("x min");
    _row_max.set_placeholder_text("x max");
    _col_min.set_placeholder_text("y min");
    _col_max.set_placeholder_text("y max");

    _draw.set_active(true);
    _draw_grid.set_active(true);
    _draw.signal_toggled().connect(sigc::mem_fun(*this, &Graph_page::change_flags));
    _draw_grid.signal_toggled().connect(sigc::mem_fun(*this, &Graph_page::change_flags));
    _draw_normals.signal_toggled().connect(sigc::mem_fun(*this, &Graph_page::change_flags));

    Gtk::RadioButton::Group tex_g = _use_color.get_group();
    _use_tex.set_group(tex_g);

    _use_color.signal_toggled().connect(sigc::mem_fun(*this, &Graph_page::change_coloring));
    _use_tex.signal_toggled().connect(sigc::mem_fun(*this, &Graph_page::change_coloring));

    Glib::RefPtr<Gdk::Pixbuf> image = Gdk::Pixbuf::create(Gdk::Colorspace::COLORSPACE_RGB, false, 8, 32, 32);

    guint8 r = (guint8)(_color.r * 256.0f);
    guint8 g = (guint8)(_color.g * 256.0f);
    guint8 b = (guint8)(_color.b * 256.0f);

    guint32 hex_color = r << 24 | g << 16 | b << 8;

    image->fill(hex_color);
    _color_ico.set(image);
    _tex_butt.set_image(_color_ico);
    _tex_butt.signal_clicked().connect(sigc::mem_fun(*this, &Graph_page::change_tex));

    _apply_butt.signal_clicked().connect(sigc::mem_fun(*this, &Graph_page::apply));

    show_all_children();
    _eqn_par_y_l.hide();
    _eqn_par_y.hide();
    _eqn_par_z_l.hide();
    _eqn_par_z.hide();


    // TODO: tooltips, fixed light, orbiting camera, widget spacing/layout, toolbar w/  add butt on right if we can, increase cursor text eqn width
}

Graph_page::~Graph_page()
{
    _gl_window->remove_graph(_graph.get());
    _gl_window->invalidate();
}

void Graph_page::set_active()
{
    _gl_window->set_active_graph(_graph.get());

    if(_graph.get() && _gl_window->draw_cursor_flag)
        update_cursor(_graph->cursor_text());
    else
        update_cursor("");

    _gl_window->invalidate();
}

void Graph_page::save_graph(const std::string & filename)
{
    libconfig::Config cfg;
    libconfig::Setting & cfg_root = cfg.getRoot().add("graph", libconfig::Setting::TypeGroup);

    cfg_root.add("r_car", libconfig::Setting::TypeBoolean) = _r_car.get_active();
    cfg_root.add("r_cyl", libconfig::Setting::TypeBoolean) = _r_cyl.get_active();
    cfg_root.add("r_sph", libconfig::Setting::TypeBoolean) = _r_sph.get_active();
    cfg_root.add("r_par", libconfig::Setting::TypeBoolean) = _r_par.get_active();

    cfg_root.add("eqn", libconfig::Setting::TypeString) = _eqn.get_text();
    cfg_root.add("eqn_par_y", libconfig::Setting::TypeString) = _eqn_par_y.get_text();
    cfg_root.add("eqn_par_z", libconfig::Setting::TypeString) = _eqn_par_z.get_text();

    cfg_root.add("row_min", libconfig::Setting::TypeString) = _row_min.get_text();
    cfg_root.add("row_max", libconfig::Setting::TypeString) = _row_max.get_text();
    cfg_root.add("col_min", libconfig::Setting::TypeString) = _col_min.get_text();
    cfg_root.add("col_max", libconfig::Setting::TypeString) = _col_max.get_text();

    cfg_root.add("row_res", libconfig::Setting::TypeInt) = _row_res.get_value_as_int();
    cfg_root.add("col_res", libconfig::Setting::TypeInt) = _col_res.get_value_as_int();

    cfg_root.add("draw", libconfig::Setting::TypeBoolean) = _draw.get_active();
    cfg_root.add("draw_grid", libconfig::Setting::TypeBoolean) = _draw_grid.get_active();
    cfg_root.add("draw_normals", libconfig::Setting::TypeBoolean) = _draw_normals.get_active();

    cfg_root.add("use_color", libconfig::Setting::TypeBoolean) = _use_color.get_active();
    cfg_root.add("use_tex", libconfig::Setting::TypeBoolean) = _use_tex.get_active();

    libconfig::Setting & color = cfg_root.add("color", libconfig::Setting::TypeList);
    color.add(libconfig::Setting::TypeFloat) = _color.r;
    color.add(libconfig::Setting::TypeFloat) = _color.g;
    color.add(libconfig::Setting::TypeFloat) = _color.b;

    cfg_root.add("tex_filename", libconfig::Setting::TypeString) = _tex_filename;

    try
    {
        cfg.writeFile(filename.c_str());
    }
    catch(const libconfig::FileIOException & e)
    {
        Gtk::MessageDialog error_dialog("Error writing to " + filename, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
        error_dialog.set_secondary_text(e.what());
        error_dialog.set_title("Errror");
        error_dialog.run();
    }
}

bool Graph_page::load_graph(const std::string & filename)
{
    libconfig::Config cfg;
    try
    {
        cfg.readFile(filename.c_str());
        libconfig::Setting & cfg_root = cfg.getRoot()["graph"];

        bool r_car = cfg_root["r_car"];
        bool r_cyl = cfg_root["r_cyl"];
        bool r_sph = cfg_root["r_sph"];
        bool r_par = cfg_root["r_par"];
        if((int)r_car + (int)r_cyl + (int)r_sph + (int)r_par != 1)
        {
            Gtk::MessageDialog error_dialog("Error parsing " + filename, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
            error_dialog.set_secondary_text("Invalid combination of r_car, r_cyl, r_sph, r_par");
            error_dialog.set_title("Errror");
            error_dialog.run();
            return false;
        }

        std::string eqn = cfg_root["eqn"];
        std::string eqn_par_y = cfg_root["eqn_par_y"];
        std::string eqn_par_z = cfg_root["eqn_par_z"];

        std::string row_min = cfg_root["row_min"];
        std::string row_max = cfg_root["row_max"];
        std::string col_min = cfg_root["col_min"];
        std::string col_max = cfg_root["col_max"];

        int row_res = cfg_root["row_res"];
        int col_res = cfg_root["col_res"];

        bool draw = cfg_root["draw"];
        bool draw_grid = cfg_root["draw_grid"];
        bool draw_normals = cfg_root["draw_normals"];

        bool use_color = cfg_root["use_color"];
        bool use_tex = cfg_root["use_tex"];
        if((int)use_color + (int)use_tex != 1)
        {
            Gtk::MessageDialog error_dialog("Error parsing " + filename, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
            error_dialog.set_secondary_text("Invalid combination of use_color, use_tex");
            error_dialog.set_title("Errror");
            error_dialog.run();
            return false;
        }

        libconfig::Setting & color_l = cfg_root["color"];
        if(!color_l.isList() || color_l.getLength() != 3)
        {
            Gtk::MessageDialog error_dialog("Error parsing " + filename, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
            std::ostringstream msg;
            msg<<"Invalid number of color elements (expected 3, got "<<color_l.getLength()<<")";
            error_dialog.set_secondary_text(msg.str());
            error_dialog.set_title("Errror");
            error_dialog.run();
            return false;
        }
        glm::vec3 color;
        color.r = color_l[0];
        color.g = color_l[1];
        color.b = color_l[2];

        std::string tex_filename = cfg_root["tex_filename"];

        _r_car.set_active(r_car);
        _r_cyl.set_active(r_cyl);
        _r_sph.set_active(r_sph);
        _r_par.set_active(r_par);

        _eqn.set_text(eqn);
        _eqn_par_y.set_text(eqn_par_y);
        _eqn_par_z.set_text(eqn_par_z);

        _row_min.set_text(row_min);
        _row_max.set_text(row_max);
        _col_min.set_text(col_min);
        _col_max.set_text(col_max);

        _row_res.get_adjustment()->set_value((double)row_res);
        _col_res.get_adjustment()->set_value((double)col_res);

        _draw.set_active(draw);
        _draw_grid.set_active(draw_grid);
        _draw_normals.set_active(draw_normals);

        _use_color.set_active(use_color);
        _use_tex.set_active(use_tex);

        _color = color;

        _tex_filename = tex_filename;
    }
    catch(const libconfig::FileIOException & e)
    {
        Gtk::MessageDialog error_dialog("Error writing to " + filename, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
        error_dialog.set_secondary_text(e.what());
        error_dialog.set_title("Errror");
        error_dialog.run();
        return false;
    }
    catch(const libconfig::ParseException & e)
    {
        Gtk::MessageDialog error_dialog("Error parsing " + filename, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
        std::ostringstream msg;
        msg<<e.getError()<<" on line: "<<e.getLine();
        error_dialog.set_secondary_text(msg.str());
        error_dialog.set_title("Errror");
        error_dialog.run();
        return false;
    }
    catch(const libconfig::SettingTypeException & e)
    {
        Gtk::MessageDialog error_dialog("Invalid setting type in" + filename, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
        error_dialog.set_secondary_text(e.getPath());
        error_dialog.set_title("Errror");
        error_dialog.run();
        return false;
    }
    catch(const libconfig::SettingNotFoundException & e)
    {
        Gtk::MessageDialog error_dialog("Could not find setting in" + filename, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
        error_dialog.set_secondary_text(e.getPath());
        error_dialog.set_title("Errror");
        error_dialog.run();
        return false;
    }

    if(!_tex_filename.empty())
    {
        try
        {
            _tex_ico.set(Gdk::Pixbuf::create_from_file(_tex_filename)->scale_simple(32, 32, Gdk::InterpType::INTERP_BILINEAR));
        }
        catch(Glib::Exception &e)
        {
            _tex_ico.set(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_LARGE_TOOLBAR);

            Gtk::MessageDialog error_dialog(e.what(), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
            error_dialog.set_title("Errror");
            error_dialog.set_secondary_text("");
            error_dialog.run();
        }
    }

    guint8 r = (guint8)(_color.r * 256.0f);
    guint8 g = (guint8)(_color.g * 256.0f);
    guint8 b = (guint8)(_color.b * 256.0f);

    guint32 hex_color = r << 24 | g << 16 | b << 8;

    Glib::RefPtr<Gdk::Pixbuf> image = Gdk::Pixbuf::create(Gdk::Colorspace::COLORSPACE_RGB, false, 8, 32, 32);
    image->fill(hex_color);
    _color_ico.set(image);

    _signal_tex_changed.emit(_tex_ico);

    change_type();
    change_coloring();

    return true;
}

sigc::signal<void, const std::string &> Graph_page::signal_cursor_moved() const
{
    return _signal_cursor_moved;
}

sigc::signal<void, const Gtk::Image &> Graph_page::signal_tex_changed() const
{
    return _signal_tex_changed;
}

void Graph_page::change_type()
{
    if(_r_car.get_active())
    {
        _eqn_l.set_text("z(x,y)=");
        _row_min_l.set_text("x min=");
        _row_max_l.set_text("x max=");
        _col_min_l.set_text("y min=");
        _col_max_l.set_text("y max=");
        _row_res_l.set_text("x resolution");
        _col_res_l.set_text("y resolution");

        _eqn.set_placeholder_text("z(x,y)");
        _row_min.set_placeholder_text("x min");
        _row_max.set_placeholder_text("x max");
        _col_min.set_placeholder_text("y min");
        _col_max.set_placeholder_text("y max");
    }
    else if(_r_cyl.get_active())
    {
        _eqn_l.set_text(u8"z(r,θ)=");
        _row_min_l.set_text("r min=");
        _row_max_l.set_text("r max=");
        _col_min_l.set_text(u8"θ min=");
        _col_max_l.set_text(u8"θ max=");
        _row_res_l.set_text("r resolution");
        _col_res_l.set_text(u8"θ resolution");

        _eqn.set_placeholder_text(u8"z(r,θ)");
        _row_min.set_placeholder_text("r min");
        _row_max.set_placeholder_text("r max");
        _col_min.set_placeholder_text(u8"θ min");
        _col_max.set_placeholder_text(u8"θ max");
    }
    else if(_r_sph.get_active())
    {
        _eqn_l.set_text(u8"z(θ,ϕ)=");
        _row_min_l.set_text(u8"θ min=");
        _row_max_l.set_text(u8"θ max=");
        _col_min_l.set_text(u8"ϕ min=");
        _col_max_l.set_text(u8"ϕ max=");
        _row_res_l.set_text(u8"θ resolution");
        _col_res_l.set_text(u8"ϕ resolution");

        _eqn.set_placeholder_text(u8"z(θ,ϕ)");
        _row_min.set_placeholder_text(u8"θ min");
        _row_max.set_placeholder_text(u8"θ max");
        _col_min.set_placeholder_text(u8"ϕ min");
        _col_max.set_placeholder_text(u8"ϕ max");
    }
    if(_r_par.get_active())
    {
        _eqn_l.set_text("x(u,v)=");
        _row_min_l.set_text("u min=");
        _row_max_l.set_text("u max=");
        _col_min_l.set_text("v min=");
        _col_max_l.set_text("v max=");
        _row_res_l.set_text("u resolution");
        _col_res_l.set_text("v resolution");

        _eqn.set_placeholder_text("x(u,v)");
        _row_min.set_placeholder_text("u min");
        _row_max.set_placeholder_text("u max");
        _col_min.set_placeholder_text("v min");
        _col_max.set_placeholder_text("v max");

        _eqn_par_y_l.show();
        _eqn_par_y.show();
        _eqn_par_z_l.show();
        _eqn_par_z.show();
    }
    else
    {
        _eqn_par_y_l.hide();
        _eqn_par_y.hide();
        _eqn_par_z_l.hide();
        _eqn_par_z.hide();
    }
}

void Graph_page::apply()
{
    _gl_window->remove_graph(_graph.get());
    _graph.reset();

    try
    {
        if(_r_car.get_active())
        {
            _graph = std::unique_ptr<Graph>(new Graph_cartesian(_eqn.get_text(),
                        _row_min.get_text(), _row_max.get_text(), _row_res.get_value_as_int(),
                        _col_min.get_text(), _col_max.get_text(), _col_res.get_value_as_int()));
        }
        else if(_r_cyl.get_active())
        {
            _graph = std::unique_ptr<Graph>(new Graph_cylindrical(_eqn.get_text(),
                        _row_min.get_text(), _row_max.get_text(), _row_res.get_value_as_int(),
                        _col_min.get_text(), _col_max.get_text(), _col_res.get_value_as_int()));
        }
        else if(_r_sph.get_active())
        {
            _graph = std::unique_ptr<Graph>(new Graph_spherical(_eqn.get_text(),
                        _row_min.get_text(), _row_max.get_text(), _row_res.get_value_as_int(),
                        _col_min.get_text(), _col_max.get_text(), _col_res.get_value_as_int()));
        }
        else if(_r_par.get_active())
        {
            _graph = std::unique_ptr<Graph>(new Graph_parametric(_eqn.get_text(), _eqn_par_y.get_text(), _eqn_par_z.get_text(),
                        _row_min.get_text(), _row_max.get_text(), _row_res.get_value_as_int(),
                        _col_min.get_text(), _col_max.get_text(), _col_res.get_value_as_int()));
        }
    }
    catch(const Graph_exception &e)
    {
        // show parsing error message
        Gtk::MessageDialog error_dialog(e.GetMsg(), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
        error_dialog.set_title("Errror");
        error_dialog.set_secondary_text("In Expression: " + e.GetExpr());
        error_dialog.run();

        // highlight error
        int start, end;
        if(e.GetToken().size() > 0 && e.GetPos() < e.GetExpr().size())
        {
            start = e.GetPos();
            end = e.GetPos() + e.GetToken().size();
        }
        else
        {
            start = 0;
            end = -1;
        }

        switch(e.GetLocation())
        {
        case Graph_exception::ROW_MIN:
            _row_min.grab_focus();
            _row_min.select_region(start, end);
            break;

        case Graph_exception::ROW_MAX:
            _row_max.grab_focus();
            _row_max.select_region(start, end);
            break;

        case Graph_exception::COL_MIN:
            _col_min.grab_focus();
            _col_min.select_region(start, end);
            break;

        case Graph_exception::COL_MAX:
            _col_max.grab_focus();
            _col_max.select_region(start, end);
            break;

        case Graph_exception::EQN:
        case Graph_exception::EQN_X:
            _eqn.grab_focus();
            _eqn.select_region(start, end);
            break;

        case Graph_exception::EQN_Y:
            _eqn_par_y.grab_focus();
            _eqn_par_y.select_region(start, end);
            break;

        case Graph_exception::EQN_Z:
            _eqn_par_z.grab_focus();
            _eqn_par_z.select_region(start, end);
            break;

        default:
            break;
        }

        _graph.reset();
        update_cursor("");
        _gl_window->invalidate();
        _gl_window->set_active_graph(nullptr);
        return;
    }

    _graph->draw_flag = _draw.get_active();
    _graph->draw_grid_flag = _draw_grid.get_active();
    _graph->draw_normals_flag = _draw_normals.get_active();

    _graph->use_tex = _use_tex.get_active();

    if(!_tex_filename.empty())
    {
        try
        {
            _graph->set_texture(_tex_filename);
        }
        catch(Glib::Exception &e)
        {
            Gtk::MessageDialog error_dialog(e.what(), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
            error_dialog.set_title("Errror");
            error_dialog.set_secondary_text("");
            error_dialog.run();
        }
    }
    _graph->color = glm::vec4(_color, 1.0f);

    update_cursor(_graph->cursor_text());

    _gl_window->add_graph(_graph.get());
    _gl_window->set_active_graph(_graph.get());

    _graph->signal_cursor_moved().connect(sigc::mem_fun(*this, &Graph_page::update_cursor));
    _gl_window->invalidate();
}

void Graph_page::change_flags()
{
    if(_graph.get())
    {
        _graph->draw_flag = _draw.get_active();
        _graph->draw_grid_flag = _draw_grid.get_active();
        _graph->draw_normals_flag = _draw_normals.get_active();
        _gl_window->invalidate();
    }
}

void Graph_page::change_coloring()
{
    if(_graph.get())
    {
        _graph->use_tex = _use_tex.get_active();
        _gl_window->invalidate();
    }

    if(_use_tex.get_active())
    {
        _tex_butt.set_label("Choose Texture");
        _tex_butt.set_image(_tex_ico);
        _signal_tex_changed.emit(_tex_ico);
    }
    else
    {
        _tex_butt.set_label("Choose Color");
        _tex_butt.set_image(_color_ico);
        _signal_tex_changed.emit(_color_ico);
    }
}

void Graph_page::change_tex()
{
    if(_use_tex.get_active())
    {
        Gtk::FileChooserDialog tex_chooser("Choose Texture", Gtk::FileChooserAction::FILE_CHOOSER_ACTION_OPEN);
        Glib::RefPtr<Gtk::FileFilter> tex_types;
        Glib::RefPtr<Gtk::FileFilter> all_types;

        tex_types = Gtk::FileFilter::create();
        tex_types->add_pixbuf_formats();
        tex_types->set_name("Image files");
        tex_chooser.add_filter(tex_types);

        all_types = Gtk::FileFilter::create();
        all_types->add_pattern("*");
        all_types->set_name("All files");
        tex_chooser.add_filter(all_types);

        tex_chooser.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
        tex_chooser.add_button("Select", Gtk::RESPONSE_OK);

        int response = tex_chooser.run();

        if(response == Gtk::RESPONSE_OK)
        {
            _tex_filename = tex_chooser.get_filename();

            try
            {
                _tex_ico.set(Gdk::Pixbuf::create_from_file(_tex_filename)->scale_simple(32, 32, Gdk::InterpType::INTERP_BILINEAR));
                if(_graph.get())
                {
                    _graph->set_texture(_tex_filename);
                    _gl_window->invalidate();
                }
            }
            catch(Glib::Exception &e)
            {
                _tex_ico.set(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_LARGE_TOOLBAR);

                Gtk::MessageDialog error_dialog(e.what(), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
                error_dialog.set_title("Errror");
                error_dialog.set_secondary_text("");
                error_dialog.run();
            }
            _signal_tex_changed.emit(_tex_ico);
        }
    }
    else
    {
        Gtk::ColorChooserDialog color_chooser;

        int response = color_chooser.run();

        if(response == Gtk::RESPONSE_OK)
        {
            _color.r = color_chooser.get_rgba().get_red();
            _color.g = color_chooser.get_rgba().get_green();
            _color.b = color_chooser.get_rgba().get_blue();

            if(_graph.get())
            {
                _graph->color = glm::vec4(_color, 1.0f);
                _gl_window->invalidate();
            }

            guint8 r = (guint8)(_color.r * 256.0f);
            guint8 g = (guint8)(_color.g * 256.0f);
            guint8 b = (guint8)(_color.b * 256.0f);

            guint32 hex_color = r << 24 | g << 16 | b << 8;

            Glib::RefPtr<Gdk::Pixbuf> image = Gdk::Pixbuf::create(Gdk::Colorspace::COLORSPACE_RGB, false, 8, 32, 32);
            image->fill(hex_color);
            _color_ico.set(image);
            _signal_tex_changed.emit(_color_ico);
        }
    }
}

void Graph_page::update_cursor(const std::string & text) const
{
    _signal_cursor_moved.emit(text);
}
