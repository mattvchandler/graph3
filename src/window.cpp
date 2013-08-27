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

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <cmath>

#include <png++/png.hpp>

#include <GL/glew.h>

#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>

#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/main.h>
#include <gtkmm/scale.h>
#include <gtkmm/stock.h>
#include <gtkmm/window.h>

#include <glibmm/main.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "SFMLWidget/SFMLWidget.h"
#include "graph.h"

void check_error(const char * at)
{
    GLenum e = glGetError();
    if(e == GL_NO_ERROR)
        return;
    std::cerr<<"OpenGL Error at "<<at<<": "<<gluErrorString(e)<<std::endl;
}

GLuint compile_shader(const char * filename, GLenum shader_type)
{
    std::ifstream in(filename, std::ios::binary | std::ios::in);
    std::vector <char> buff;

    if(in)
    {
        in.seekg(0, std::ios::end);
        size_t in_size = in.tellg();
        in.seekg(0, std::ios::beg);

        buff.resize(in_size + 1);
        buff.back() = '\0';
        in.read(&buff[0], in_size);

        if(!in)
            return 0;
    }
    else
        return 0;

    GLuint shader = glCreateShader(shader_type);

    const char * src = &buff[0];
    glShaderSource(shader, 1, &src, NULL);

    glCompileShader(shader);

    GLint compile_status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);

    if(compile_status != GL_TRUE)
    {
        GLint log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> log(log_length + 1);
        log.back() = '\0';
        glGetShaderInfoLog(shader, log_length, NULL, &log[0]);

        std::cerr<<"Error compiling shader: "<<filename<<std::endl;
        std::cerr<<&log[0]<<std::endl;

        glDeleteShader(shader);

        return 0;
    }

    return shader;
}

GLuint link_shader_prog(const std::vector<GLuint> & shaders)
{
    GLuint prog = glCreateProgram();
    for(auto &i: shaders)
        glAttachShader(prog, i);

    glLinkProgram(prog);

    GLint link_status;
    glGetProgramiv(prog, GL_LINK_STATUS, &link_status);

    if(link_status != GL_TRUE)
    {
        GLint log_length;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &log_length);
        std::vector<char> log(log_length + 1);
        log.back() = '\0';
        glGetProgramInfoLog(prog, log_length, NULL, &log[0]);

        std::cerr<<"Error linking program: "<<std::endl;
        std::cerr<<&log[0]<<std::endl;

        glDeleteProgram(prog);

        return 0;
    }

    return prog;

}

std::vector<float> read_png(const char * filename)
{
    const png::image<png::rgba_pixel> image(filename);
    std::vector<float> data(image.get_height() * image.get_width() * 4);

    for(size_t r = 0; r < image.get_height(); ++r)
    {
        for(size_t c = 0; c < image.get_width(); ++c)
        {
            data[(image.get_width() * r + c) * 4 + 0] = (float)image[r][c].red / 255.0f;
            data[(image.get_width() * r + c) * 4 + 1] = (float)image[r][c].green / 255.0f;
            data[(image.get_width() * r + c) * 4 + 2] = (float)image[r][c].blue / 255.0f;
            data[(image.get_width() * r + c) * 4 + 3] = (float)image[r][c].alpha / 255.0f;
        }
    }

    return data;
}

//using Rodrigues' rotation formula
//http://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula
glm::vec3 rotate_vec(const glm::vec3 & v, const glm::vec3 & axis, const float angle)
{
    //using Rodrigues' rotation formula
    //http://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula
    return v * cosf(angle) + glm::cross(axis, v) * sinf(angle)
        + axis * glm::dot(axis, v) * (1.0f - cosf(angle));
}

class Graph_disp final: public SFMLWidget
{
public:
    Graph_disp(const sf::VideoMode & mode, const int size_reqest = - 1, const sf::ContextSettings & context_settings= sf::ContextSettings()):
        SFMLWidget(mode, size_reqest),
        test_graph(new Graph_cartesian("x^2 +y^2"))
    {
        signal_realize().connect(sigc::mem_fun(*this, &Graph_disp::realize));
        signal_size_allocate().connect(sigc::mem_fun(*this, &Graph_disp::resize));
        signal_draw().connect(sigc::mem_fun(*this, &Graph_disp::draw));
        Glib::signal_timeout().connect(sigc::mem_fun(*this, &Graph_disp::input), 10);
        // Glib::signal_idle().connect(sigc::mem_fun(*this, &Graph_disp::idle));

        set_can_focus();
        set_can_default();
    }

    ~Graph_disp()
    {
    }

    // openGL initialization should go here
    void realize()
    {
        // init glew
        if(glewInit() != GLEW_OK)
        {
            std::cerr<<"Error loading glew. Aborting"<<std::endl;
            exit(EXIT_FAILURE);
        }

        // init GL state vars
        glClearColor(0.25f, 0.25f, 0.25f, 0.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthRangef(0.0f, 1.0f);
        glLineWidth(5.0f);

        glEnable(GL_POLYGON_SMOOTH);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_DONT_CARE);
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

        // build shader programs
        GLuint vert_shader = compile_shader("src/tet.vert", GL_VERTEX_SHADER);
        GLuint frag_shader = compile_shader("src/tet.frag", GL_FRAGMENT_SHADER);

        if(frag_shader == 0 || vert_shader == 0)
            exit(EXIT_FAILURE);

        shader_prog = link_shader_prog(std::vector<GLuint> {vert_shader, frag_shader});
        if(shader_prog == 0)
            exit(EXIT_FAILURE);

        glDeleteShader(vert_shader);
        glDeleteShader(frag_shader);

        vert_shader = compile_shader("src/line.vert", GL_VERTEX_SHADER);
        frag_shader = compile_shader("src/line.frag", GL_FRAGMENT_SHADER);

        if(frag_shader == 0 || vert_shader == 0)
            exit(EXIT_FAILURE);

        shader_prog_line = link_shader_prog(std::vector<GLuint> {vert_shader, frag_shader});
        if(shader_prog_line == 0)
            exit(EXIT_FAILURE);

        glDeleteShader(vert_shader);
        glDeleteShader(frag_shader);

        // TODO: maybe have set methods that call this for us
        test_graph->build_graph();
    }

    void resize(Gtk::Allocation & allocation)
    {
        if(m_refGdkWindow)
        {
            glViewport(0, 0, glWindow.getSize().x, glWindow.getSize().y);
            invalidate();
        }
    }

    bool draw(const Cairo::RefPtr<Cairo::Context> & cr)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        test_graph->draw();

        display();
        return true;
    }

    bool input()
    {
        static std::unordered_map<sf::Keyboard::Key, bool, std::hash<int>> key_lock;
        static sf::Vector2i old_mouse_pos = sf::Mouse::getPosition(glWindow);

        // the neat thing about having this in a timeout func is that we
        // don't need to calc dt for movement controls.
        // it is always (almost) exactly 10ms
        if(dynamic_cast<Gtk::Window *>(get_toplevel())->is_active())
        {
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
                get_toplevel()->hide();

            sf::Vector2i new_mouse_pos = sf::Mouse::getPosition(glWindow);

            if(!has_focus() && sf::Mouse::isButtonPressed(sf::Mouse::Left) &&
                new_mouse_pos.x >= 0 && new_mouse_pos.y >= 0 &&
                new_mouse_pos.x < (int)glWindow.getSize().x && new_mouse_pos.y < (int)glWindow.getSize().y)
            {
                grab_focus();
            }
            if(has_focus())
            {
            }

            old_mouse_pos = new_mouse_pos;
        }
        return true;
    }

private:

    std::unique_ptr<Graph> test_graph;
    GLuint shader_prog;
    GLuint shader_prog_line;

    // make non-copyable
    Graph_disp(const Graph_disp &) = delete;
    Graph_disp(const Graph_disp &&) = delete;
    Graph_disp & operator=(const Graph_disp &) = delete;
    Graph_disp & operator=(const Graph_disp &&) = delete;
};

class Graph_window final: public Gtk::Window
{
public:
    Graph_window(): gl_window(sf::VideoMode(800, 600), -1, sf::ContextSettings(0, 0, 4, 4, 0)) // these do nothing yet - future SFML version should enable them
    {
        gl_window.set_hexpand(true);
        gl_window.set_vexpand(true);

        add(main_grid);
        main_grid.set_column_spacing(5);

        main_grid.attach(gl_window, 0, 0, 1, 9);
        show_all_children();
    }

    Graph_disp gl_window;
    Gtk::Grid main_grid;
};

int main(int argc, char* argv[])
{
    Gtk::Main kit(argc, argv);
    Graph_window gtk_window;

    Gtk::Main::run(gtk_window);
    return 0;
}
