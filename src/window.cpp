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

#include <GL/glew.h>

#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>

#include <gtkmm/box.h>
#include <gtkmm/main.h>
#include <gtkmm/window.h>

#include <glibmm/main.h>


#include "SFMLWidget/SFMLWidget.h"
#include "osl/vec4.h"

class Graph_disp: public SFMLWidget
{
public:
    Graph_disp(const sf::VideoMode & mode, const int size_reqest = - 1, const sf::ContextSettings & context_settings= sf::ContextSettings()): SFMLWidget(mode, size_reqest)
    {
        std::cout<<"constructing"<<std::endl;
        signal_realize().connect(sigc::mem_fun(*this, &Graph_disp::realize));
        signal_size_allocate().connect(sigc::mem_fun(*this, &Graph_disp::resize));
        signal_draw().connect(sigc::mem_fun(*this, &Graph_disp::draw));
        Glib::signal_timeout().connect(sigc::mem_fun(*this, &Graph_disp::rotate_y), 25);
        Glib::signal_timeout().connect(sigc::mem_fun(*this, &Graph_disp::rotate_z), 25);

        rotation_y = 0.0f;
        rotation_z = 0.0f;
    }

    void realize()
    {
        std::cout<<"I just realized something..."<<std::endl;
        glClearColor(0.25f, 0.25f, 0.25f, 0.0f);
        glEnable(GL_DEPTH_TEST);
        glLineWidth(5.0f);
        std::cout<<"OpenGL version: "<<glGetString(GL_VERSION)<<std::endl;
        std::cout<<"GLSL version: "<<glGetString(GL_SHADING_LANGUAGE_VERSION)<<std::endl;
        std::cout<<"Vendor: "<<glGetString(GL_VENDOR)<<std::endl;
        std::cout<<"Renderer: "<<glGetString(GL_RENDERER)<<std::endl;
        int bits;
        glGetIntegerv(GL_MAJOR_VERSION, &bits);
        std::cout<<"Major version: "<<bits<<std::endl;
        glGetIntegerv(GL_MINOR_VERSION, &bits);
        std::cout<<"Minor version: "<<bits<<std::endl;
    }
    void resize(Gtk::Allocation & allocation)
    {
        std::cout<<"allocating"<<std::endl;

        glViewport(0, 0, glWindow.getSize().x, glWindow.getSize().y);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        float aspect = (float)glWindow.getSize().x / (float)glWindow.getSize().y;
        // gluOrtho2D(-1.0f * aspect, 1.0f * aspect, -1.0f, 1.0f);
        gluPerspective(30.0f, aspect, 0.1f, 1000.0f);

        glMatrixMode(GL_MODELVIEW);
        invalidate();
    }

    bool draw(const Cairo::RefPtr<Cairo::Context> & cr)
    {
        // coords are X,Z,Y in sane-coords™
        const vec3 tet_coords[4] = 
        {
            vec3(0.0f, sqrt(6.0f) / 4.0f, 0.0f),
            vec3(-0.5f, -sqrt(6.0f) / 12.0f, -sqrt(3.0f) / 6.0),
            vec3(0.5f, -sqrt(6.0f) / 12.0f, -sqrt(3.0f) / 6.0),
            vec3(0.0f, -sqrt(6.0f) / 12.0f, sqrt(3.0f) / 3.0)
        };

        const vec4 colors[8] = 
        {
            vec4(1.0f, 0.0f, 0.0f, 1.0f),
            vec4(0.1f, 0.5f, 0.1f, 1.0f),
            vec4(0.0f, 0.0f, 1.0f, 1.0f),
            vec4(1.0f, 1.0f, 0.0f, 1.0f),
            vec4(0.0f, 1.0f, 1.0f, 1.0f),
            vec4(1.0f, 0.0f, 1.0f, 1.0f),
            vec4(1.0f, 1.0f, 1.0f, 1.0f),
            vec4(0.0f, 0.0f, 0.0f, 1.0f)
        };
        enum {RED = 0, GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE, BLACK};

        // std::cout<<"drawing"<<std::endl;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, -3.0f);

        // glBegin(GL_LINES);
        // glColor4fv(colors[CYAN]);
        // glVertex3f(0.0f, -2.0f, 0.0f); glVertex3f(0.0f, 2.0f, 0.0f);
        // glEnd();

        glRotatef(rotation_z, 0.0f, 1.0f, 0.0f);
        glRotatef(rotation_y, 0.0f, 0.0f, 1.0f);

        glBegin(GL_TRIANGLES);

        // tet colored by face
        glColor4fv(colors[RED]);
        glVertex3fv(tet_coords[0]);
        glVertex3fv(tet_coords[1]);
        glVertex3fv(tet_coords[2]);

        glColor4fv(colors[GREEN]);
        glVertex3fv(tet_coords[0]);
        glVertex3fv(tet_coords[3]);
        glVertex3fv(tet_coords[1]);

        glColor4fv(colors[BLUE]);
        glVertex3fv(tet_coords[0]);
        glVertex3fv(tet_coords[2]);
        glVertex3fv(tet_coords[3]);

        glColor4fv(colors[YELLOW]);
        glVertex3fv(tet_coords[1]);
        glVertex3fv(tet_coords[2]);
        glVertex3fv(tet_coords[3]);

        glEnd();

        // tetrahedron edges
        glColor4fv(colors[BLACK]);
        glBegin(GL_LINES);

        glVertex3fv(tet_coords[0]); glVertex3fv(tet_coords[1]);
        glVertex3fv(tet_coords[0]); glVertex3fv(tet_coords[2]);
        glVertex3fv(tet_coords[0]); glVertex3fv(tet_coords[3]);

        glVertex3fv(tet_coords[1]); glVertex3fv(tet_coords[2]);
        glVertex3fv(tet_coords[1]); glVertex3fv(tet_coords[3]);
        glVertex3fv(tet_coords[2]); glVertex3fv(tet_coords[3]);

        // tesselate
        glVertex3fv(2.0f / 3.0f * tet_coords[0] + 1.0f / 3.0f * tet_coords[1]); glVertex3fv(2.0f / 3.0f * tet_coords[0] + 1.0f / 3.0f * tet_coords[2]);
        glVertex3fv(2.0f / 3.0f * tet_coords[0] + 1.0f / 3.0f * tet_coords[2]); glVertex3fv(2.0f / 3.0f * tet_coords[0] + 1.0f / 3.0f * tet_coords[3]);
        glVertex3fv(2.0f / 3.0f * tet_coords[0] + 1.0f / 3.0f * tet_coords[3]); glVertex3fv(2.0f / 3.0f * tet_coords[0] + 1.0f / 3.0f * tet_coords[1]);

        glVertex3fv(2.0f / 3.0f * tet_coords[1] + 1.0f / 3.0f * tet_coords[0]); glVertex3fv(2.0f / 3.0f * tet_coords[1] + 1.0f / 3.0f * tet_coords[2]);
        glVertex3fv(2.0f / 3.0f * tet_coords[1] + 1.0f / 3.0f * tet_coords[2]); glVertex3fv(2.0f / 3.0f * tet_coords[1] + 1.0f / 3.0f * tet_coords[3]);
        glVertex3fv(2.0f / 3.0f * tet_coords[1] + 1.0f / 3.0f * tet_coords[3]); glVertex3fv(2.0f / 3.0f * tet_coords[1] + 1.0f / 3.0f * tet_coords[0]);

        glVertex3fv(2.0f / 3.0f * tet_coords[2] + 1.0f / 3.0f * tet_coords[0]); glVertex3fv(2.0f / 3.0f * tet_coords[2] + 1.0f / 3.0f * tet_coords[1]);
        glVertex3fv(2.0f / 3.0f * tet_coords[2] + 1.0f / 3.0f * tet_coords[1]); glVertex3fv(2.0f / 3.0f * tet_coords[2] + 1.0f / 3.0f * tet_coords[3]);
        glVertex3fv(2.0f / 3.0f * tet_coords[2] + 1.0f / 3.0f * tet_coords[3]); glVertex3fv(2.0f / 3.0f * tet_coords[2] + 1.0f / 3.0f * tet_coords[0]);

        glVertex3fv(2.0f / 3.0f * tet_coords[3] + 1.0f / 3.0f * tet_coords[0]); glVertex3fv(2.0f / 3.0f * tet_coords[3] + 1.0f / 3.0f * tet_coords[1]);
        glVertex3fv(2.0f / 3.0f * tet_coords[3] + 1.0f / 3.0f * tet_coords[1]); glVertex3fv(2.0f / 3.0f * tet_coords[3] + 1.0f / 3.0f * tet_coords[2]);
        glVertex3fv(2.0f / 3.0f * tet_coords[3] + 1.0f / 3.0f * tet_coords[2]); glVertex3fv(2.0f / 3.0f * tet_coords[3] + 1.0f / 3.0f * tet_coords[0]);

        glVertex3fv(1.0f / 3.0f * tet_coords[0] + 2.0f / 3.0f * tet_coords[1]); glVertex3fv(1.0f / 3.0f * tet_coords[0] + 2.0f / 3.0f * tet_coords[2]);
        glVertex3fv(1.0f / 3.0f * tet_coords[0] + 2.0f / 3.0f * tet_coords[2]); glVertex3fv(1.0f / 3.0f * tet_coords[0] + 2.0f / 3.0f * tet_coords[3]);
        glVertex3fv(1.0f / 3.0f * tet_coords[0] + 2.0f / 3.0f * tet_coords[3]); glVertex3fv(1.0f / 3.0f * tet_coords[0] + 2.0f / 3.0f * tet_coords[1]);

        glVertex3fv(1.0f / 3.0f * tet_coords[1] + 2.0f / 3.0f * tet_coords[0]); glVertex3fv(1.0f / 3.0f * tet_coords[1] + 2.0f / 3.0f * tet_coords[2]);
        glVertex3fv(1.0f / 3.0f * tet_coords[1] + 2.0f / 3.0f * tet_coords[2]); glVertex3fv(1.0f / 3.0f * tet_coords[1] + 2.0f / 3.0f * tet_coords[3]);
        glVertex3fv(1.0f / 3.0f * tet_coords[1] + 2.0f / 3.0f * tet_coords[3]); glVertex3fv(1.0f / 3.0f * tet_coords[1] + 2.0f / 3.0f * tet_coords[0]);

        glVertex3fv(1.0f / 3.0f * tet_coords[2] + 2.0f / 3.0f * tet_coords[0]); glVertex3fv(1.0f / 3.0f * tet_coords[2] + 2.0f / 3.0f * tet_coords[1]);
        glVertex3fv(1.0f / 3.0f * tet_coords[2] + 2.0f / 3.0f * tet_coords[1]); glVertex3fv(1.0f / 3.0f * tet_coords[2] + 2.0f / 3.0f * tet_coords[3]);
        glVertex3fv(1.0f / 3.0f * tet_coords[2] + 2.0f / 3.0f * tet_coords[3]); glVertex3fv(1.0f / 3.0f * tet_coords[2] + 2.0f / 3.0f * tet_coords[0]);

        glVertex3fv(1.0f / 3.0f * tet_coords[3] + 2.0f / 3.0f * tet_coords[0]); glVertex3fv(1.0f / 3.0f * tet_coords[3] + 2.0f / 3.0f * tet_coords[1]);
        glVertex3fv(1.0f / 3.0f * tet_coords[3] + 2.0f / 3.0f * tet_coords[1]); glVertex3fv(1.0f / 3.0f * tet_coords[3] + 2.0f / 3.0f * tet_coords[2]);
        glVertex3fv(1.0f / 3.0f * tet_coords[3] + 2.0f / 3.0f * tet_coords[2]); glVertex3fv(1.0f / 3.0f * tet_coords[3] + 2.0f / 3.0f * tet_coords[0]);

        // axes
        // glColor4fv(colors[RED]);
        // glVertex3f(-2.0f, 0.0f, 0.0f); glVertex3f(2.0f, 0.0f, 0.0f);
        // glColor4fv(colors[GREEN]);
        // glVertex3f(0.0f, -2.0f, 0.0f); glVertex3f(0.0f, 2.0f, 0.0f);
        // glColor4fv(colors[BLUE]);
        // glVertex3f(0.0f, 0.0f, -2.0f); glVertex3f(0.0f, 0.0f, 2.0f);

        glEnd();
        
        display();
        return true;
    }

    bool rotate_z()
    {
        rotation_z += .8;
        if(rotation_z > 360.0f)
            rotation_z -= 360.0f;

        invalidate();
        return true;
    }

    bool rotate_y()
    {
        rotation_y += .4f;
        if(rotation_y > 360.0f)
            rotation_y -= 360.0f;

        invalidate();
        return true;
    }

private:
    float rotation_y;
    float rotation_z;
};

int main(int argc, char* argv[])
{
    Gtk::Main kit(argc, argv);
    Gtk::Window gtk_window;
    Gtk::VBox main_box;

    Graph_disp gl_window(sf::VideoMode(800, 600), -1, sf::ContextSettings(0, 0, 0, 4, 0));

    gl_window.show();
    main_box.pack_start(gl_window);
    main_box.show();

    gtk_window.add(main_box);

    Gtk::Main::run(gtk_window);
    return 0;
}
