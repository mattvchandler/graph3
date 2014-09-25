// graph_page_file_io.cpp
// read and write graphs to disk

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

#include <sstream>

#include <gtkmm/messagedialog.h>
#include <gtkmm/stock.h>

#include <libconfig.h++>

#include "graph.hpp"
#include "graph_page.hpp"

// save graph to file
void Graph_page::save_graph(const std::string & filename)
{
    // build the config file from widget properties
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
    cfg_root.add("transparent", libconfig::Setting::TypeBoolean) = _transparent.get_active();
    cfg_root.add("draw_normals", libconfig::Setting::TypeBoolean) = _draw_normals.get_active();
    cfg_root.add("draw_grid", libconfig::Setting::TypeBoolean) = _draw_grid.get_active();

    cfg_root.add("use_color", libconfig::Setting::TypeBoolean) = _use_color.get_active();
    cfg_root.add("use_tex", libconfig::Setting::TypeBoolean) = _use_tex.get_active();

    libconfig::Setting & color = cfg_root.add("color", libconfig::Setting::TypeList);
    color.add(libconfig::Setting::TypeFloat) = _color.r;
    color.add(libconfig::Setting::TypeFloat) = _color.g;
    color.add(libconfig::Setting::TypeFloat) = _color.b;

    cfg_root.add("tex_filename", libconfig::Setting::TypeString) = _tex_filename;

    try
    {
        // write file
        cfg.writeFile(filename.c_str());
    }
    catch(const libconfig::FileIOException & e)
    {
        // create an error message box
        Gtk::MessageDialog error_dialog("Error writing to " + filename, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
        error_dialog.set_secondary_text(e.what());
        error_dialog.set_title("Error");
        error_dialog.run();
    }
}

// read from a file
bool Graph_page::load_graph(const std::string & filename)
{
    bool complete = true;

    libconfig::Config cfg;
    try
    {
        // open and parse file
        cfg.readFile(filename.c_str());
        libconfig::Setting & cfg_root = cfg.getRoot()["graph"];

        // set properties - reqired settings
        bool r_car = cfg_root["r_car"];
        bool r_cyl = cfg_root["r_cyl"];
        bool r_sph = cfg_root["r_sph"];
        bool r_par = cfg_root["r_par"];
        // one and only one should be set
        if((int)r_car + (int)r_cyl + (int)r_sph + (int)r_par != 1)
        {
            // show error message box
            Gtk::MessageDialog error_dialog("Error parsing " + filename, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
            error_dialog.set_secondary_text("Invalid combination of r_car, r_cyl, r_sph, r_par");
            error_dialog.set_title("Error");
            error_dialog.run();
            return false;
        }

        _r_car.set_active(r_car);
        _r_cyl.set_active(r_cyl);
        _r_sph.set_active(r_sph);
        _r_par.set_active(r_par);

        bool use_color = cfg_root["use_color"];
        bool use_tex = cfg_root["use_tex"];
        // check for mutual exclusion
        if((int)use_color + (int)use_tex != 1)
        {
            // show error message box
            Gtk::MessageDialog error_dialog("Error parsing " + filename, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
            error_dialog.set_secondary_text("Invalid combination of use_color, use_tex");
            error_dialog.set_title("Error");
            error_dialog.run();
            return false;
        }

        _use_color.set_active(use_color);
        _use_tex.set_active(use_tex);


        // non-required settings, but needed to draw graph
        try { _eqn.set_text(static_cast<const char *>(cfg_root["eqn"])); }
        catch(const libconfig::SettingNotFoundException) { complete = false; }

        try { _eqn_par_y.set_text(static_cast<const char *>(cfg_root["eqn_par_y"])); }
        catch(const libconfig::SettingNotFoundException)
        {
            if(r_par)
                complete = false;
        }

        try { _eqn_par_z.set_text(static_cast<const char *>(cfg_root["eqn_par_z"])); }
        catch(const libconfig::SettingNotFoundException)
        {
            if(r_par)
                complete = false;
        }

        try { _row_min.set_text(static_cast<const char *>(cfg_root["row_min"])); }
        catch(const libconfig::SettingNotFoundException) { complete = false; }

        try { _row_max.set_text(static_cast<const char *>(cfg_root["row_max"])); }
        catch(const libconfig::SettingNotFoundException) { complete = false; }

        try { _col_min.set_text(static_cast<const char *>(cfg_root["col_min"])); }
        catch(const libconfig::SettingNotFoundException) { complete = false; }

        try { _col_max.set_text(static_cast<const char *>(cfg_root["col_max"])); }
        catch(const libconfig::SettingNotFoundException) { complete = false; }

        // non-required settings
        try { _row_res.get_adjustment()->set_value(static_cast<int>(cfg_root["row_res"])); }
        catch(const libconfig::SettingNotFoundException) {}

        try { _col_res.get_adjustment()->set_value(static_cast<int>(cfg_root["col_res"])); }
        catch(const libconfig::SettingNotFoundException) {}

        try { _draw.set_active(static_cast<bool>(cfg_root["draw"])); }
        catch(const libconfig::SettingNotFoundException) {}

        try { _transparent.set_active(static_cast<bool>(cfg_root["transparent"])); }
        catch(const libconfig::SettingNotFoundException) {}

        try { _draw_normals.set_active(static_cast<bool>(cfg_root["draw_normals"])); }
        catch(const libconfig::SettingNotFoundException) {}

        try { _draw_grid.set_active(static_cast<bool>(cfg_root["draw_grid"])); }
        catch(const libconfig::SettingNotFoundException) {}

        try { _tex_filename = static_cast<const char *>(cfg_root["tex_filename"]); }
        catch(const libconfig::SettingNotFoundException) {}

        try
        {
            libconfig::Setting & color_l = cfg_root["color"];

            // check for valid color (list of 3)
            if(!color_l.isList() || color_l.getLength() != 3)
            {
                // show error message box
                Gtk::MessageDialog error_dialog("Error parsing " + filename, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
                std::ostringstream msg;
                msg<<"Invalid number of color elements (expected 3, got "<<color_l.getLength()<<")";
                error_dialog.set_secondary_text(msg.str());
                error_dialog.set_title("Error");
                error_dialog.run();
                return false;
            }
            _color.r = color_l[0];
            _color.g = color_l[1];
            _color.b = color_l[2];
        }
        catch(const libconfig::SettingNotFoundException) {}

    }
    catch(const libconfig::FileIOException & e)
    {
        // show error message box
        Gtk::MessageDialog error_dialog("Error reading from " + filename, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
        error_dialog.set_secondary_text(e.what());
        error_dialog.set_title("Error");
        error_dialog.run();
        return false;
    }
    catch(const libconfig::ParseException & e)
    {
        // show error message box
        Gtk::MessageDialog error_dialog("Error parsing " + filename, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
        std::ostringstream msg;
        msg<<e.getError()<<" on line: "<<e.getLine();
        error_dialog.set_secondary_text(msg.str());
        error_dialog.set_title("Error");
        error_dialog.run();
        return false;
    }
    catch(const libconfig::SettingTypeException & e)
    {
        // show error message box
        Gtk::MessageDialog error_dialog("Invalid setting type in" + filename, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
        error_dialog.set_secondary_text(e.getPath());
        error_dialog.set_title("Error");
        error_dialog.run();
        return false;
    }
    catch(const libconfig::SettingNotFoundException & e)
    {
        // show error message box
        Gtk::MessageDialog error_dialog("Could not find setting in" + filename, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
        error_dialog.set_secondary_text(e.getPath());
        error_dialog.set_title("Error");
        error_dialog.run();
        return false;
    }

    // try to open the texture file
    if(!_tex_filename.empty())
    {
        try
        {
            _tex_ico.set(Gdk::Pixbuf::create_from_file(_tex_filename)->scale_simple(32, 32, Gdk::InterpType::INTERP_BILINEAR));
        }
        catch(Glib::Exception &e)
        {
            // set image thumbnail to fallback
            _tex_ico.set(Gtk::Stock::MISSING_IMAGE, Gtk::ICON_SIZE_LARGE_TOOLBAR);

            // show error message box
            Gtk::MessageDialog error_dialog(e.what(), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
            error_dialog.set_title("Error");
            error_dialog.set_secondary_text("");
            error_dialog.run();
        }
    }

    // set color thumbnail
    guint8 r = (guint8)(_color.r * 256.0f);
    guint8 g = (guint8)(_color.g * 256.0f);
    guint8 b = (guint8)(_color.b * 256.0f);
    guint32 hex_color = r << 24 | g << 16 | b << 8;
    Glib::RefPtr<Gdk::Pixbuf> image = Gdk::Pixbuf::create(Gdk::Colorspace::COLORSPACE_RGB, false, 8, 32, 32);
    image->fill(hex_color);
    _color_ico.set(image);

    // signal a texture change
    _signal_tex_changed.emit(_tex_ico);

    // set properties from widget values
    change_type();
    change_coloring();

    if(complete)
        apply();

    return true;
}
