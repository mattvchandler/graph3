// graph_page.cpp
// widgets for creating graphs

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

#include <gtkmm/messagedialog.h>
#include <gtkmm/separator.h>

#include "graph_page.hpp"

#include "graph.hpp"
#include "graph_cartesian.hpp"
#include "graph_cylindrical.hpp"
#include "graph_spherical.hpp"
#include "graph_parametric.hpp"

const glm::vec3 Graph_page::start_color = glm::vec3(0.2f, 0.5f, 0.2f);

Graph_page::Graph_page(Graph_disp & gl_window): _gl_window(gl_window), _graph(nullptr),
    _r_car("Cartesian"),
    _r_cyl("Cylindrical"),
    _r_sph("Spherical"),
    _r_par("Parametric"),
    _row_res_l("x resolution"),
    _col_res_l("y resolution"),
    _row_res(Gtk::Adjustment::create(50.0, 1.0, 1000.0)),
    _col_res(Gtk::Adjustment::create(50.0, 1.0, 1000.0)),
    _use_color("Use Color"),
    _use_tex("Use Texture"),
    _draw("Draw Graph"),
    _transparent("Transparent Graph"),
    _draw_normals("Draw Normals"),
    _draw_grid("Draw Gridlines"),
    _transparency_l("Opacity:"),
    _transparency(Gtk::Adjustment::create(0.5, 0.0, 1.0, 0.01), Gtk::ORIENTATION_HORIZONTAL),
    _color(start_color)
{
    // set page properties
    set_border_width(3);
    set_row_spacing(3);
    set_column_spacing(3);

    Image_button * apply_butt = Gtk::manage(new Image_button);

    // add and position widgets to page
    attach(_r_car, 0, 1, 1, 1);
    attach(_r_cyl, 1, 1, 1, 1);
    attach(_r_sph, 0, 2, 1, 1);
    attach(_r_par, 1, 2, 1, 1);
    attach(_eqn, 0, 3, 2, 1);
    attach(_eqn_par_y, 0, 4, 2, 1);
    attach(_eqn_par_z, 0, 5, 2, 1);
    attach(_row_min, 0, 6, 1, 1);
    attach(_row_max, 1, 6, 1, 1);
    attach(_col_min, 0, 7, 1, 1);
    attach(_col_max, 1, 7, 1, 1);
    attach(_row_res_l, 0, 8, 1, 1);
    attach(_row_res, 1, 8, 1, 1);
    attach(_col_res_l, 0, 9, 1, 1);
    attach(_col_res, 1, 9, 1, 1);
    attach(*Gtk::manage(new Gtk::Separator), 0, 10, 2, 1);
    attach(_use_color, 0, 11, 1, 1);
    attach(_use_tex, 0, 12, 1, 1);
    attach(_tex_butt, 1, 11, 1, 2);
    attach(*Gtk::manage(new Gtk::Separator), 0, 13, 2, 1);
    attach(_draw, 0, 14, 1, 1);
    attach(_transparent, 1, 14, 1, 1);
    attach(_draw_normals, 0, 15, 1, 1);
    attach(_draw_grid, 1, 15, 1, 1);
    attach(_transparency_l, 0, 16, 1, 1);
    attach(_transparency, 1, 16, 1, 1);
    attach(*Gtk::manage(new Gtk::Separator), 0, 17, 2, 1);
    attach(*apply_butt, 0, 18, 2, 1);

    // set button properties
    _tex_butt.set_valign(Gtk::ALIGN_CENTER);
    _tex_butt.set_halign(Gtk::ALIGN_CENTER);
    _tex_butt.set_vexpand(false);
    _tex_butt.set_hexpand(false);
    _tex_butt.lbl.set_text_with_mnemonic("_Choose Color");

    apply_butt->set_halign(Gtk::ALIGN_CENTER);
    apply_butt->set_hexpand(false);
    apply_butt->lbl.set_text_with_mnemonic("A_pply");
    apply_butt->img.set_from_icon_name("emblem-default", Gtk::ICON_SIZE_SMALL_TOOLBAR);

    // set up graph type radio buttons
    Gtk::RadioButton::Group type_g = _r_car.get_group();
    _r_cyl.set_group(type_g);
    _r_sph.set_group(type_g);
    _r_par.set_group(type_g);

    _r_car.signal_toggled().connect(sigc::mem_fun(*this, &Graph_page::change_type));
    _r_cyl.signal_toggled().connect(sigc::mem_fun(*this, &Graph_page::change_type));
    _r_sph.signal_toggled().connect(sigc::mem_fun(*this, &Graph_page::change_type));
    _r_par.signal_toggled().connect(sigc::mem_fun(*this, &Graph_page::change_type));

    // set signal when Enter is pressed inside a text box
    _eqn.signal_activate().connect(sigc::mem_fun(*this, &Graph_page::apply));
    _eqn_par_y.signal_activate().connect(sigc::mem_fun(*this, &Graph_page::apply));
    _eqn_par_z.signal_activate().connect(sigc::mem_fun(*this, &Graph_page::apply));
    _row_min.signal_activate().connect(sigc::mem_fun(*this, &Graph_page::apply));
    _row_max.signal_activate().connect(sigc::mem_fun(*this, &Graph_page::apply));
    _col_min.signal_activate().connect(sigc::mem_fun(*this, &Graph_page::apply));
    _col_max.signal_activate().connect(sigc::mem_fun(*this, &Graph_page::apply));
    _row_res.signal_activate().connect(sigc::mem_fun(*this, &Graph_page::apply));
    _col_res.signal_activate().connect(sigc::mem_fun(*this, &Graph_page::apply));

    // set placeholder text in text boxes
    _eqn.set_placeholder_text("z(x,y)");
    _eqn_par_y.set_placeholder_text("y(u,v)");
    _eqn_par_z.set_placeholder_text("z(u,v)");
    _row_min.set_placeholder_text("x min");
    _row_max.set_placeholder_text("x max");
    _col_min.set_placeholder_text("y min");
    _col_max.set_placeholder_text("y max");

    // set color radio buttons
    Gtk::RadioButton::Group tex_g = _use_color.get_group();
    _use_tex.set_group(tex_g);

    _use_color.signal_toggled().connect(sigc::mem_fun(*this, &Graph_page::change_coloring));
    _use_tex.signal_toggled().connect(sigc::mem_fun(*this, &Graph_page::change_coloring));

    // set up color thumbnail
    Glib::RefPtr<Gdk::Pixbuf> image = Gdk::Pixbuf::create(Gdk::Colorspace::COLORSPACE_RGB, false, 8, 32, 32);
    guint8 r = (guint8)(_color.r * 256.0f);
    guint8 g = (guint8)(_color.g * 256.0f);
    guint8 b = (guint8)(_color.b * 256.0f);
    guint32 hex_color = r << 24 | g << 16 | b << 8;
    image->fill(hex_color);

    _color_ico.set(image);
    _tex_ico.set_from_icon_name("image-missing", Gtk::ICON_SIZE_LARGE_TOOLBAR);
    _tex_butt.img.set(image);

    // connect color / texture change signal
    _tex_butt.signal_clicked().connect(sigc::mem_fun(*this, &Graph_page::change_tex));

    // setup apply button
    apply_butt->signal_clicked().connect(sigc::mem_fun(*this, &Graph_page::apply));

    // set checkbox properties
    _draw.set_active(true);
    _draw_grid.set_active(true);
    _draw.signal_toggled().connect(sigc::mem_fun(*this, &Graph_page::change_flags));
    _transparent.signal_toggled().connect(sigc::mem_fun(*this, &Graph_page::change_flags));
    _draw_normals.signal_toggled().connect(sigc::mem_fun(*this, &Graph_page::change_flags));
    _draw_grid.signal_toggled().connect(sigc::mem_fun(*this, &Graph_page::change_flags));

    // set opacity slider properties & signal
    _transparency.set_digits(2);
    _transparency.signal_value_changed().connect(sigc::mem_fun(*this, &Graph_page::change_transparency));

    // set visibility
    show_all_children();
    _eqn_par_y.hide();
    _eqn_par_z.hide();
    _transparency_l.hide();
    _transparency.hide();
}

Graph_page::~Graph_page()
{
    // tell the display to drop the graph from its records
    _gl_window.remove_graph(_graph.get());
    // trigger a re-draw
    _gl_window.invalidate();
}

// sets the graph as the active graph
void Graph_page::set_active()
{
    // tell display
    _gl_window.set_active_graph(_graph.get());

    // get the cursor pos if we can
    if(_graph.get() && _gl_window.draw_cursor_flag)
        update_cursor(_graph->cursor_text());
    else
        update_cursor("");

    // trigger a re-draw
    _gl_window.invalidate();
}

// signaled when the cursor has moved and needs updating
sigc::signal<void, const std::string &> Graph_page::signal_cursor_moved() const
{
    return _signal_cursor_moved;
}

// signaled when the user selects a new texture
sigc::signal<void, const Gtk::Image &> Graph_page::signal_tex_changed() const
{
    return _signal_tex_changed;
}

    // called when the type changes (cartesian, cylindrical, etc.)
void Graph_page::change_type()
{
    // change text elements to appropriate values
    if(_r_car.get_active())
    {
        _eqn.set_placeholder_text("z(x,y)");
        _row_min.set_placeholder_text("x min");
        _row_max.set_placeholder_text("x max");
        _col_min.set_placeholder_text("y min");
        _col_max.set_placeholder_text("y max");

        _row_res_l.set_text("x resolution");
        _col_res_l.set_text("y resolution");
    }
    else if(_r_cyl.get_active())
    {
        _eqn.set_placeholder_text(u8"z(r,θ)");
        _row_min.set_placeholder_text("r min");
        _row_max.set_placeholder_text("r max");
        _col_min.set_placeholder_text(u8"θ min");
        _col_max.set_placeholder_text(u8"θ max");

        _row_res_l.set_text("r resolution");
        _col_res_l.set_text(u8"θ resolution");
    }
    else if(_r_sph.get_active())
    {
        _eqn.set_placeholder_text(u8"r(θ,ϕ)");
        _row_min.set_placeholder_text(u8"θ min");
        _row_max.set_placeholder_text(u8"θ max");
        _col_min.set_placeholder_text(u8"ϕ min");
        _col_max.set_placeholder_text(u8"ϕ max");

        _row_res_l.set_text(u8"θ resolution");
        _col_res_l.set_text(u8"ϕ resolution");
    }
    if(_r_par.get_active())
    {
        _eqn.set_placeholder_text("x(u,v)");
        _row_min.set_placeholder_text("u min");
        _row_max.set_placeholder_text("u max");
        _col_min.set_placeholder_text("v min");
        _col_max.set_placeholder_text("v max");

        _row_res_l.set_text("u resolution");
        _col_res_l.set_text("v resolution");

        // show extra text boxes
        _eqn_par_y.show();
        _eqn_par_z.show();
    }
    else
    {
        // hide the extra parametric equation boxes
        _eqn_par_y.hide();
        _eqn_par_z.hide();
    }
}

// called when checkboxes for displaying grid, normals are pressed
void Graph_page::change_flags()
{
    if(_graph.get())
    {
        // pass settings to the graph
        _graph->draw_flag = _draw.get_active();
        _graph->transparent_flag = _transparent.get_active();
        _graph->draw_normals_flag = _draw_normals.get_active();
        _graph->draw_grid_flag = _draw_grid.get_active();
        // redraw
        _gl_window.invalidate();
    }

    if(_transparent.get_active())
    {
        _transparency_l.show();
        _transparency.show();
    }
    else
    {
        _transparency_l.hide();
        _transparency.hide();
    }
}

// called when changing transparency
void Graph_page::change_transparency()
{
    if(_graph.get())
    {
        _graph->transparency = _transparency.get_value();
        // redraw
        _gl_window.invalidate();
    }
}

// apply changes and create/update graph
void Graph_page::apply()
{
    // destroy any existing graph
    _gl_window.remove_graph(_graph.get());
    _graph.reset();

    try
    {
        // create a new graph object
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
        error_dialog.set_title("Error");
        error_dialog.set_secondary_text("In Expression: " + e.GetExpr());
        error_dialog.run();

        // highlight the location of the error
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

        // remove and delete the partiall constructed graph object
        _graph.reset();
        update_cursor("");
        _gl_window.invalidate();
        _gl_window.set_active_graph(nullptr);
        return;
    }

    // set graph properties
    _graph->draw_flag = _draw.get_active();
    _graph->transparent_flag = _transparent.get_active();
    _graph->draw_normals_flag = _draw_normals.get_active();
    _graph->draw_grid_flag = _draw_grid.get_active();
    _graph->use_tex = _use_tex.get_active();

    // set the texture
    if(!_tex_filename.empty())
    {
        try
        {
            _graph->set_texture(_tex_filename);
        }
        catch(Glib::Exception &e)
        {
            // show error message box
            Gtk::MessageDialog error_dialog(e.what(), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
            error_dialog.set_title("Error");
            error_dialog.set_secondary_text("");
            error_dialog.run();
        }
    }
    _graph->color = _color;
    _graph->transparency = _transparency.get_value();

    update_cursor(_graph->cursor_text());

    // register and set active with display
    _gl_window.add_graph(_graph.get());
    _gl_window.set_active_graph(_graph.get());

    // signal a cursor update
    _graph->signal_cursor_moved().connect(sigc::mem_fun(*this, &Graph_page::update_cursor));
    _gl_window.invalidate();
}

void Graph_page::update_cursor(const std::string & text) const
{
    _signal_cursor_moved.emit(text);
}
