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

#include <gtkmm/colorchooserdialog.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

#include <gdkmm.h>

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
    _draw_grid("Draw Gridlines"),
    _draw_normals("Draw normals"),
    _use_color("Use Color"),
    _use_tex("Use Texture"),
    _tex_butt("Choose Texture"),
    _apply_butt(Gtk::Stock::APPLY),
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
    attach(_draw_grid, 0, 9, 2, 1);
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

    _draw_grid.set_active(true);
    _draw_grid.signal_toggled().connect(sigc::mem_fun(*this, &Graph_page::change_flags));
    _draw_normals.signal_toggled().connect(sigc::mem_fun(*this, &Graph_page::change_flags));

    Gtk::RadioButton::Group tex_g = _use_color.get_group();
    _use_tex.set_group(tex_g);

    _use_color.signal_toggled().connect(sigc::mem_fun(*this, &Graph_page::change_coloring));
    _use_tex.signal_toggled().connect(sigc::mem_fun(*this, &Graph_page::change_coloring));

    apply_tex();

    _tex_butt.set_image(_tex_ico);
    _tex_butt.signal_clicked().connect(sigc::mem_fun(*this, &Graph_page::change_tex));

    _apply_butt.signal_clicked().connect(sigc::mem_fun(*this, &Graph_page::apply));

    show_all_children();
    _eqn_par_y_l.hide();
    _eqn_par_y.hide();
    _eqn_par_z_l.hide();
    _eqn_par_z.hide();

    // TODO: tooltips, save/load, fixed light, orbiting camera, widget spacing/layout, texture_butt icon
}

Graph_page::~Graph_page()
{
    _gl_window->remove_graph(_graph.get());
    _gl_window->invalidate();
}

void Graph_page::change_type()
{
    // prevent being run when leaving one and again while entering another
    static bool change_in = false;
    change_in = !change_in;
    if(!change_in)
        return;

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
        Gtk::MessageDialog error_dialog(std::string(e.GetMsg()), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
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
            Gtk::MessageDialog error_dialog(std::string(e.what()), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
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
        _graph->draw_grid_flag = _draw_grid.get_active();
        _graph->draw_normals_flag = _draw_normals.get_active();
        _gl_window->invalidate();
    }
}

void Graph_page::change_coloring()
{
    // prevent being run when leaving one and again while entering another
    static bool change_in = false;
    change_in = !change_in;
    if(!change_in)
        return;

    if(_graph.get())
    {
        _graph->use_tex = _use_tex.get_active();
        _gl_window->invalidate();
    }
    apply_tex();
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
            apply_tex();
            apply_tex_to_graph();
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
            apply_tex();
            apply_tex_to_graph();
        }
    }
}

void Graph_page::apply_tex()
{
    if(_use_tex.get_active() && !_tex_filename.empty())
    {
        try
        {
            _tex_ico.set(Gdk::Pixbuf::create_from_file(_tex_filename)->scale_simple(32, 32, Gdk::InterpType::INTERP_BILINEAR));
        }
        catch(Glib::Exception &e)
        {
            _tex_ico.set(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_LARGE_TOOLBAR);

            Gtk::MessageDialog error_dialog(std::string(e.what()), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
            error_dialog.set_title("Errror");
            error_dialog.set_secondary_text("");
            error_dialog.run();
        }
    }
    else
    {
        guint8 r = (guint8)(_color.r * 256.0f);
        guint8 g = (guint8)(_color.g * 256.0f);
        guint8 b = (guint8)(_color.b * 256.0f);

        guint32 hex_color = r << 24 | g << 16 | b << 8;

        Glib::RefPtr<Gdk::Pixbuf> image = Gdk::Pixbuf::create(Gdk::Colorspace::COLORSPACE_RGB, false, 8, 32, 32);
        image->fill(hex_color);
        _tex_ico.set(image);
    }

    _signal_tex_changed.emit(_tex_ico);
}

void Graph_page::apply_tex_to_graph()
{
    if(_graph.get())
    {
        if(_use_tex.get_active() && !_tex_filename.empty())
        {
            try
            {
                _graph->set_texture(_tex_filename);
            }
            catch(Glib::Exception &e)
            {
                Gtk::MessageDialog error_dialog(std::string(e.what()), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
                error_dialog.set_title("Errror");
                error_dialog.run();
            }
        }
        else
        {
            _graph->color = glm::vec4(_color, 1.0f);
        }
        _gl_window->invalidate();
    }
}

void Graph_page::update_cursor(const std::string & text) const
{
    _signal_cursor_moved.emit(text);
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

sigc::signal<void, const std::string &> Graph_page::signal_cursor_moved() const
{
    return _signal_cursor_moved;
}

sigc::signal<void, const Gtk::Image &> Graph_page::signal_tex_changed() const
{
    return _signal_tex_changed;
}

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
