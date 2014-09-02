// tab_label.hpp
// tab labels

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

#ifndef TAB_LABEL_H
#define TAB_LABEL_H

#include <gtkmm/button.h>
#include <gtkmm/grid.h>
#include <gtkmm/image.h>

#include <sigc++/sigc++.h>

// label for a notebook page
// has a color/texture thumbnail and close button
class Tab_label final: public Gtk::Grid
{
public:
    Tab_label();

    // signals the lose button has been pressed
    sigc::signal<void> signal_close_tab();
    // set the thumbnail
    void set_img(const Gtk::Image & filename);

private:

    // called when close button is pressed
    void on_button_press();

    Gtk::Image _tab_pic;
    sigc::signal<void> _signal_close_tab;
};

#endif // TAB_LABEL_H
