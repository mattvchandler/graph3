// window.cpp
// windowing code. Using GTK to create the window, SFML to do openGL graphics.

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

#include <iostream>

#include <gtkmm/box.h>
#include <gtkmm/main.h>
#include <gtkmm/window.h>

#include <glibmm/main.h>

#include "SFMLWidget/SFMLWidget.h"

class Graph_disp: public SFMLWidget
{
public:
    Graph_disp(sf::VideoMode Mode): SFMLWidget(Mode)
    {
        std::cout<<"constructing"<<std::endl;
        Glib::signal_idle().connect(sigc::mem_fun(*this, &Graph_disp::on_idle));
    }

    void on_realize()
    {
        std::cout<<"realizing"<<std::endl;
        SFMLWidget::on_realize();

        glClearColor(0.25f, 0.25f, 0.25f, 0.0f);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float aspect = (float)glWindow.getSize().x / (float)glWindow.getSize().y;
        gluOrtho2D(-1.0f * aspect, 1.0f * aspect, -1.0f, 1.0f);
        glMatrixMode(GL_MODELVIEW);
        invalidate();
    }

    void on_size_allocate(Gtk::Allocation & allocation)
    {
        std::cout<<"allocating"<<std::endl;
        SFMLWidget::on_size_allocate(allocation);

        if(m_refGdkWindow)
        {
            glViewport(0, 0, glWindow.getSize().x, glWindow.getSize().y);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            float aspect = (float)glWindow.getSize().x / (float)glWindow.getSize().y;
            gluOrtho2D(-1.0f * aspect, 1.0f * aspect, -1.0f, 1.0f);
            glMatrixMode(GL_MODELVIEW);
        }
        invalidate();
    }

    bool on_idle()
    {
        // std::cout<<"running"<<std::endl;
        if(m_refGdkWindow)
        {
            // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glLoadIdentity();
            glRotatef(rotation, 0.0f, 0.0f, 1.0f);

            glBegin(GL_TRIANGLES);
            glColor4f(1.0f, 0.0f, 0.0f, 0.0f);
            glVertex3f(0.0f, sqrt(3.0f) / 3.0f, 0.0f);
            glColor4f(0.0f, 1.0f, 0.0f, 0.0f);
            glVertex3f(0.5f, -sqrt(3.0f) / 6.0f, 0.0f);
            glColor4f(0.0f, 0.0f, 1.0f, 0.0f);
            glVertex3f(-0.5f, -sqrt(3.0f) / 6.0f, 0.0f);
            glEnd();

            display();

            rotation += .01f;
            invalidate();
        }
        return true;
    }

private:
    float rotation;
};

int main(int argc, char* argv[])
{
    Gtk::Main kit(argc, argv);
    Gtk::Window gtk_window;
    Gtk::VBox main_box;

    Graph_disp gl_window(sf::VideoMode(640, 480));

    gl_window.show();
    main_box.pack_start(gl_window);
    main_box.show();

    gtk_window.add(main_box);

    Gtk::Main::run(gtk_window);
    return 0;
}
