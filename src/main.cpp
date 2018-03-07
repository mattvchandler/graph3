// main.cpp
// application start point

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

#include <string>
#include <vector>

#include <gtkmm/application.h>

#include <glibmm/exception.h>

#include "graph_window.hpp"

int return_code = EXIT_SUCCESS; // to be used w/ extern by any module that needs it

// get filenames to open
void gather_filenames(const Gio::Application::type_vec_files & files, const Glib::ustring & hint,
    const Glib::RefPtr<Gtk::Application> & app, Graph_window & win)
{
    std::vector<std::string> filenames;
    for(auto & f:files)
    {
        filenames.push_back(f->get_path());
    }

    win.open_at_startup(filenames);

    // for some stupid reason the window won't open  when given files if this isn't here
    app->activate();
}

int main(int argc, char * argv[])
{
    // create app and window objects
    Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "org.matt.graph3",
        Gio::APPLICATION_NON_UNIQUE | Gio::APPLICATION_HANDLES_OPEN);

    try
    {
        Graph_window gtk_window;

        // get files to open
        app->signal_open().connect(sigc::bind<const Glib::RefPtr<Gtk::Application> &,
            Graph_window &>(sigc::ptr_fun(&gather_filenames), app, gtk_window));

        // run main window
        int gtk_return_code = app->run(gtk_window);

        app->remove_window(gtk_window);

        if(return_code == EXIT_SUCCESS)
            return gtk_return_code;
        else
            return return_code;
    }
    catch(const Glib::Error & e)
    {
        return EXIT_FAILURE;
    }
}
