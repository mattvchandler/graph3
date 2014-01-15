// main.cpp
// application start point

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

#include <gtkmm/application.h>

#include <glibmm/exception.h>

#include "graph_window.hpp"

int return_code = EXIT_SUCCESS; // to be used w/ extern by any module that needs it

int main(int argc, char * argv[])
{
    // create app and window objects
    Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv);
    try
    {
        Graph_window gtk_window;
        // run main window
        int gtk_return_code = app->run(gtk_window);
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
