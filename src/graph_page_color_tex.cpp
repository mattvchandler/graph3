// graph_page_color_tex.cpp
// color & texture selection

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

#include <gtkmm/colorchooserdialog.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/messagedialog.h>

#include "graph_page.hpp"

#include "graph.hpp"

// called when switching between color and texture
void Graph_page::change_coloring()
{
    if(_graph.get())
    {
        // set graph settings and redraw
        _graph->use_tex = _use_tex.get_active();
        _gl_window.invalidate();
    }

    // swap text and thumbnail
    if(_use_tex.get_active())
    {
        if(_tex_ico.get_pixbuf())
        {
            dynamic_cast<Gtk::Image *>(dynamic_cast<Gtk::Grid *>(_tex_butt.get_child())->get_child_at(0, 0))->
                set(_tex_ico.get_pixbuf()->copy());
        }
        else
        {
            dynamic_cast<Gtk::Image *>(dynamic_cast<Gtk::Grid *>(_tex_butt.get_child())->get_child_at(0, 0))->
                set_from_icon_name("image-missing", Gtk::ICON_SIZE_LARGE_TOOLBAR);
        }

        dynamic_cast<Gtk::Label *>(dynamic_cast<Gtk::Grid *>(_tex_butt.get_child())->get_child_at(1, 0))->set_text("Choose Texture");
        _signal_tex_changed.emit(_tex_ico);
    }
    else
    {
        dynamic_cast<Gtk::Image *>(dynamic_cast<Gtk::Grid *>(_tex_butt.get_child())->get_child_at(0, 0))->
            set(_color_ico.get_pixbuf()->copy());

        dynamic_cast<Gtk::Label *>(dynamic_cast<Gtk::Grid *>(_tex_butt.get_child())->get_child_at(1, 0))->set_text("Choose Color");
        _signal_tex_changed.emit(_color_ico);
    }
}

// called when the color or texture is changed
void Graph_page::change_tex()
{
    if(_use_tex.get_active())
    {
        // create file chooser
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

        tex_chooser.add_button("Cancel", Gtk::RESPONSE_CANCEL);
        tex_chooser.add_button("Select", Gtk::RESPONSE_OK);

        // show the dialog
        int response = tex_chooser.run();

        if(response == Gtk::RESPONSE_OK)
        {
            _tex_filename = tex_chooser.get_filename();
            try
            {
                // try to read chosen file
                _tex_ico.set(Gdk::Pixbuf::create_from_file(_tex_filename)->scale_simple(32, 32, Gdk::InterpType::INTERP_BILINEAR));
                dynamic_cast<Gtk::Image *>(dynamic_cast<Gtk::Grid *>(_tex_butt.get_child())->get_child_at(0, 0))->
                    set(_tex_ico.get_pixbuf()->copy());

                if(_graph.get())
                {
                    // load the texture into OpenGL and redraw
                    _graph->set_texture(_tex_filename);
                    _gl_window.invalidate();
                }
            }
            catch(Glib::Exception &e)
            {
                _tex_ico.set_from_icon_name("image-missing", Gtk::ICON_SIZE_LARGE_TOOLBAR);

                dynamic_cast<Gtk::Image *>(dynamic_cast<Gtk::Grid *>(_tex_butt.get_child())->get_child_at(0, 0))->
                    set_from_icon_name("image-missing", Gtk::ICON_SIZE_LARGE_TOOLBAR);

                // show error message box
                Gtk::MessageDialog error_dialog(e.what(), false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
                error_dialog.set_title("Error");
                error_dialog.set_secondary_text("");
                error_dialog.run();
            }
            // signal that the texture has changed
            _signal_tex_changed.emit(_tex_ico);
        }
    }
    else
    {
        // create and launch color chooser
        Gtk::ColorChooserDialog color_chooser;

        int response = color_chooser.run();

        if(response == Gtk::RESPONSE_OK)
        {
            _color.r = color_chooser.get_rgba().get_red();
            _color.g = color_chooser.get_rgba().get_green();
            _color.b = color_chooser.get_rgba().get_blue();

            if(_graph.get())
            {
                // set color in graph properties
                _graph->color = _color;
                _gl_window.invalidate();
            }

            // create thumbnail
            guint8 r = (guint8)(_color.r * 256.0f);
            guint8 g = (guint8)(_color.g * 256.0f);
            guint8 b = (guint8)(_color.b * 256.0f);
            guint32 hex_color = r << 24 | g << 16 | b << 8;
            Glib::RefPtr<Gdk::Pixbuf> image = Gdk::Pixbuf::create(Gdk::Colorspace::COLORSPACE_RGB, false, 8, 32, 32);
            image->fill(hex_color);
            _color_ico.set(image);

            dynamic_cast<Gtk::Image *>(dynamic_cast<Gtk::Grid *>(_tex_butt.get_child())->get_child_at(0, 0))->
                set(image);

            // signal that the texture has changed
            _signal_tex_changed.emit(_color_ico);
        }
    }
}
