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
#include <vector>

#include <GL/glew.h>

#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>

#include <gtkmm/box.h>
#include <gtkmm/main.h>
#include <gtkmm/window.h>

#include <glibmm/main.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "SFMLWidget/SFMLWidget.h"

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

class Graph_disp final: public SFMLWidget
{
public:
    Graph_disp(const sf::VideoMode & mode, const int size_reqest = - 1, const sf::ContextSettings & context_settings= sf::ContextSettings()):
        SFMLWidget(mode, size_reqest), perspective(1.0f), shader_prog(0)

    {
        std::cout<<"constructing"<<std::endl;
        signal_realize().connect(sigc::mem_fun(*this, &Graph_disp::realize));
        signal_size_allocate().connect(sigc::mem_fun(*this, &Graph_disp::resize));
        signal_draw().connect(sigc::mem_fun(*this, &Graph_disp::draw));
        Glib::signal_timeout().connect(sigc::mem_fun(*this, &Graph_disp::rotate_y), 25);
        Glib::signal_timeout().connect(sigc::mem_fun(*this, &Graph_disp::rotate_z), 25);

        rotation_y = 0.0f;
        rotation_z = 0.0f;

        // calculate normals
        for(auto &i: tet_coords)
        {
            normals.push_back(-glm::normalize(i));
        }
    }

    ~Graph_disp()
    {
        if(shader_prog != 0)
            glDeleteProgram(shader_prog);
        if(vao != 0)
            glDeleteVertexArrays(1, &vao);
        if(vao_lines != 0)
            glDeleteVertexArrays(1, &vao_lines);
        // if(buffer != 0)
        //     glInvalidateBufferData(buffer);
        // if(line_buffer != 0)
        //     glInvalidateBufferData(line_buffer);
    }

    void realize()
    {
        // openGL initialization should go here
        std::cout<<"I just realized something..."<<std::endl;

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

        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_POLYGON_SMOOTH);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_DONT_CARE);

        // build shader program
        GLuint vert_shader = compile_shader("src/tet.vert", GL_VERTEX_SHADER);
        GLuint frag_shader = compile_shader("src/tet.frag", GL_FRAGMENT_SHADER);

        if(frag_shader == 0 || vert_shader == 0)
            exit(EXIT_FAILURE);

        shader_prog = link_shader_prog(std::vector<GLuint> {vert_shader, frag_shader});
        if(shader_prog == 0)
            exit(EXIT_FAILURE);

        glDeleteShader(vert_shader);
        glDeleteShader(frag_shader);

        glUseProgram(shader_prog);

        // generate buffers for verts and colors
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        std::vector<glm::vec3> verts;
        std::vector<glm::vec4> vert_color;

        vert_color.push_back(colors[RED]); vert_color.push_back(colors[RED]); vert_color.push_back(colors[RED]);
        verts.push_back(tet_coords[0]); verts.push_back(tet_coords[1]); verts.push_back(tet_coords[2]);
        vert_color.push_back(colors[GREEN]); vert_color.push_back(colors[GREEN]); vert_color.push_back(colors[GREEN]);
        verts.push_back(tet_coords[0]); verts.push_back(tet_coords[3]); verts.push_back(tet_coords[1]);
        vert_color.push_back(colors[BLUE]); vert_color.push_back(colors[BLUE]); vert_color.push_back(colors[BLUE]);
        verts.push_back(tet_coords[0]); verts.push_back(tet_coords[2]); verts.push_back(tet_coords[3]);
        vert_color.push_back(colors[YELLOW]); vert_color.push_back(colors[YELLOW]); vert_color.push_back(colors[YELLOW]);
        verts.push_back(tet_coords[1]); verts.push_back(tet_coords[2]); verts.push_back(tet_coords[3]);

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec3) + vert_color.size() * sizeof(glm::vec4), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, verts.size() * sizeof(glm::vec3), &verts[0]);
        glBufferSubData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec3), vert_color.size() * sizeof(glm::vec4), &vert_color[0]);

        glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(pos);

        glVertexAttribPointer(color, 4, GL_FLOAT, GL_TRUE, 0, (void *)(verts.size() * sizeof(glm::vec3)));
        glEnableVertexAttribArray(color);

        //buffers for lines
        glGenVertexArrays(1, &vao_lines);
        glBindVertexArray(vao_lines);

        verts.clear();
        vert_color.clear();

        // outlines
        verts.push_back(tet_coords[0]); verts.push_back(tet_coords[1]);
        verts.push_back(tet_coords[0]); verts.push_back(tet_coords[2]);
        verts.push_back(tet_coords[0]); verts.push_back(tet_coords[3]);
        verts.push_back(tet_coords[1]); verts.push_back(tet_coords[2]);
        verts.push_back(tet_coords[2]); verts.push_back(tet_coords[3]);
        verts.push_back(tet_coords[3]); verts.push_back(tet_coords[1]);

        // inner lines
        verts.push_back(2.0f / 3.0f * tet_coords[0] + 1.0f / 3.0f * tet_coords[1]); verts.push_back(2.0f / 3.0f * tet_coords[0] + 1.0f / 3.0f * tet_coords[2]);
        verts.push_back(2.0f / 3.0f * tet_coords[0] + 1.0f / 3.0f * tet_coords[2]); verts.push_back(2.0f / 3.0f * tet_coords[0] + 1.0f / 3.0f * tet_coords[3]);
        verts.push_back(2.0f / 3.0f * tet_coords[0] + 1.0f / 3.0f * tet_coords[3]); verts.push_back(2.0f / 3.0f * tet_coords[0] + 1.0f / 3.0f * tet_coords[1]);

        verts.push_back(2.0f / 3.0f * tet_coords[1] + 1.0f / 3.0f * tet_coords[0]); verts.push_back(2.0f / 3.0f * tet_coords[1] + 1.0f / 3.0f * tet_coords[2]);
        verts.push_back(2.0f / 3.0f * tet_coords[1] + 1.0f / 3.0f * tet_coords[2]); verts.push_back(2.0f / 3.0f * tet_coords[1] + 1.0f / 3.0f * tet_coords[3]);
        verts.push_back(2.0f / 3.0f * tet_coords[1] + 1.0f / 3.0f * tet_coords[3]); verts.push_back(2.0f / 3.0f * tet_coords[1] + 1.0f / 3.0f * tet_coords[0]);

        verts.push_back(2.0f / 3.0f * tet_coords[2] + 1.0f / 3.0f * tet_coords[0]); verts.push_back(2.0f / 3.0f * tet_coords[2] + 1.0f / 3.0f * tet_coords[1]);
        verts.push_back(2.0f / 3.0f * tet_coords[2] + 1.0f / 3.0f * tet_coords[1]); verts.push_back(2.0f / 3.0f * tet_coords[2] + 1.0f / 3.0f * tet_coords[3]);
        verts.push_back(2.0f / 3.0f * tet_coords[2] + 1.0f / 3.0f * tet_coords[3]); verts.push_back(2.0f / 3.0f * tet_coords[2] + 1.0f / 3.0f * tet_coords[0]);

        verts.push_back(2.0f / 3.0f * tet_coords[3] + 1.0f / 3.0f * tet_coords[0]); verts.push_back(2.0f / 3.0f * tet_coords[3] + 1.0f / 3.0f * tet_coords[1]);
        verts.push_back(2.0f / 3.0f * tet_coords[3] + 1.0f / 3.0f * tet_coords[1]); verts.push_back(2.0f / 3.0f * tet_coords[3] + 1.0f / 3.0f * tet_coords[2]);
        verts.push_back(2.0f / 3.0f * tet_coords[3] + 1.0f / 3.0f * tet_coords[2]); verts.push_back(2.0f / 3.0f * tet_coords[3] + 1.0f / 3.0f * tet_coords[0]);

        verts.push_back(1.0f / 3.0f * tet_coords[0] + 2.0f / 3.0f * tet_coords[1]); verts.push_back(1.0f / 3.0f * tet_coords[0] + 2.0f / 3.0f * tet_coords[2]);
        verts.push_back(1.0f / 3.0f * tet_coords[0] + 2.0f / 3.0f * tet_coords[2]); verts.push_back(1.0f / 3.0f * tet_coords[0] + 2.0f / 3.0f * tet_coords[3]);
        verts.push_back(1.0f / 3.0f * tet_coords[0] + 2.0f / 3.0f * tet_coords[3]); verts.push_back(1.0f / 3.0f * tet_coords[0] + 2.0f / 3.0f * tet_coords[1]);

        verts.push_back(1.0f / 3.0f * tet_coords[1] + 2.0f / 3.0f * tet_coords[0]); verts.push_back(1.0f / 3.0f * tet_coords[1] + 2.0f / 3.0f * tet_coords[2]);
        verts.push_back(1.0f / 3.0f * tet_coords[1] + 2.0f / 3.0f * tet_coords[2]); verts.push_back(1.0f / 3.0f * tet_coords[1] + 2.0f / 3.0f * tet_coords[3]);
        verts.push_back(1.0f / 3.0f * tet_coords[1] + 2.0f / 3.0f * tet_coords[3]); verts.push_back(1.0f / 3.0f * tet_coords[1] + 2.0f / 3.0f * tet_coords[0]);

        verts.push_back(1.0f / 3.0f * tet_coords[2] + 2.0f / 3.0f * tet_coords[0]); verts.push_back(1.0f / 3.0f * tet_coords[2] + 2.0f / 3.0f * tet_coords[1]);
        verts.push_back(1.0f / 3.0f * tet_coords[2] + 2.0f / 3.0f * tet_coords[1]); verts.push_back(1.0f / 3.0f * tet_coords[2] + 2.0f / 3.0f * tet_coords[3]);
        verts.push_back(1.0f / 3.0f * tet_coords[2] + 2.0f / 3.0f * tet_coords[3]); verts.push_back(1.0f / 3.0f * tet_coords[2] + 2.0f / 3.0f * tet_coords[0]);

        verts.push_back(1.0f / 3.0f * tet_coords[3] + 2.0f / 3.0f * tet_coords[0]); verts.push_back(1.0f / 3.0f * tet_coords[3] + 2.0f / 3.0f * tet_coords[1]);
        verts.push_back(1.0f / 3.0f * tet_coords[3] + 2.0f / 3.0f * tet_coords[1]); verts.push_back(1.0f / 3.0f * tet_coords[3] + 2.0f / 3.0f * tet_coords[2]);
        verts.push_back(1.0f / 3.0f * tet_coords[3] + 2.0f / 3.0f * tet_coords[2]); verts.push_back(1.0f / 3.0f * tet_coords[3] + 2.0f / 3.0f * tet_coords[0]);

        for(size_t i = 0; i < verts.size(); ++i)
            vert_color.push_back(colors[BLACK]);

        glGenBuffers(1, &line_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, line_buffer);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec3) + vert_color.size() * sizeof(glm::vec4), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, verts.size() * sizeof(glm::vec3), &verts[0]);
        glBufferSubData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec3), vert_color.size() * sizeof(glm::vec4), &vert_color[0]);

        glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(pos);

        glVertexAttribPointer(color, 4, GL_FLOAT, GL_TRUE, 0, (void *)(verts.size() * sizeof(glm::vec3)));
        glEnableVertexAttribArray(color);

        std::cout<<"OpenGL version: "<<glGetString(GL_VERSION)<<std::endl;
        std::cout<<"GLSL version: "<<glGetString(GL_SHADING_LANGUAGE_VERSION)<<std::endl;
        std::cout<<"Vendor: "<<glGetString(GL_VENDOR)<<std::endl;
        std::cout<<"Renderer: "<<glGetString(GL_RENDERER)<<std::endl;

        int bits;
        glGetIntegerv(GL_MAJOR_VERSION, &bits);
        std::cout<<"Major version: "<<bits<<std::endl;
        glGetIntegerv(GL_MINOR_VERSION, &bits);
        std::cout<<"Minor version: "<<bits<<std::endl;

        glGetIntegerv(GL_SAMPLES, &bits);
        std::cout<<"Samples: "<<bits<<std::endl;
    }

    void resize(Gtk::Allocation & allocation)
    {
        std::cout<<"allocating"<<std::endl;

        if(m_refGdkWindow)
        {
            glViewport(0, 0, glWindow.getSize().x, glWindow.getSize().y);

            // glMatrixMode(GL_PROJECTION);
            // glLoadIdentity();
            float aspect = (float)glWindow.getSize().x / (float)glWindow.getSize().y;
            // gluOrtho2D(-1.0f * aspect, 1.0f * aspect, -1.0f, 1.0f);
            // gluPerspective(30.0f, aspect, 0.1f, 1000.0f);

            // glMatrixMode(GL_MODELVIEW);

            // float f = 1.0f / tan(30.0f * 0.5f * M_PI / 180.0f);

            // perspective = glm::mat4(
            //         glm::vec4(f / aspect, 0.0f, 0.0f, 0.0f),
            //         glm::vec4(0.0f, f, 0.0f, 0.0f),
            //         glm::vec4(0.0f, 0.0f, (1000.0f + 0.1f) / (0.1f - 1000.0f), -1.0f),
            //         glm::vec4(0.0f, 0.0f,  (2 * 1000.0f * 0.1f) / (0.1f - 1000.0f), 0.0f));
            perspective = glm::perspective(30.0f, aspect, 0.1f, 1000.0f);

            invalidate();
        }
    }

    bool draw(const Cairo::RefPtr<Cairo::Context> & cr)
    {
        // std::cout<<"drawing"<<std::endl;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // set up some matrices
        // glm::mat4 translate(
        //     glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
        //     glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
        //     glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
        //     glm::vec4(0.0f, 0.0f, -3.0f, 1.0f));
        glm::mat4 translate = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, -3.0f));

        // float rotate_z_rad = rotation_z * M_PI / 180.0f;
        // glm::mat4 rotate_z_mat(
        //     glm::vec4(cos(rotate_z_rad), 0.0f, -sin(rotate_z_rad), 0.0f),
        //     glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
        //     glm::vec4(sin(rotate_z_rad), 0.0f, cos(rotate_z_rad), 0.0f),
        //     glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        glm::mat4 rotate_z_mat = glm::rotate(glm::mat4(), rotation_z, glm::vec3(0.0f, 1.0f, 0.0f));

        // float rotate_y_rad = rotation_y * M_PI / 180.0f;
        // glm::mat4 rotate_y_mat(
        //     glm::vec4(cos(rotate_y_rad), sin(rotate_y_rad), 0.0f, 0.0f),
        //     glm::vec4(-sin(rotate_y_rad), cos(rotate_y_rad), 0.0f, 0.0f),
        //     glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
        //     glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        glm::mat4 rotate_y_mat = glm::rotate(glm::mat4(), rotation_y, glm::vec3(0.0f, 0.0f, 1.0f));

        glm::mat4 view_model_perspective = perspective * translate * rotate_z_mat * rotate_y_mat;

        // glm::mat3 normal_transform = glm::transpose(glm::inverse(glm::mat3(view_model_perspective)));

        glUniformMatrix4fv(glGetUniformLocation(shader_prog, "view_model_perspective"), 1, GL_FALSE, &view_model_perspective[0][0]);

        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 12);

        glBindVertexArray(vao_lines);
        glDrawArrays(GL_LINES, 0, 60);

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
    // make non-copyable
    Graph_disp(const Graph_disp &) = delete;
    Graph_disp(const Graph_disp &&) = delete;
    Graph_disp & operator=(const Graph_disp &) = delete;
    Graph_disp & operator=(const Graph_disp &&) = delete;

    float rotation_y;
    float rotation_z;

    glm::mat4 perspective;

    GLuint shader_prog;

    GLuint vao;
    GLuint vao_lines;
    GLuint buffer;
    GLuint line_buffer;

    enum {pos = 0, color};

    // coords are X,Z,Y in sane-coords™
    const std::vector<glm::vec3> tet_coords =
    {
        glm::vec3(0.0f, sqrt(6.0f) / 4.0f, 0.0f),
        glm::vec3(-0.5f, -sqrt(6.0f) / 12.0f, -sqrt(3.0f) / 6.0f),
        glm::vec3(0.5f, -sqrt(6.0f) / 12.0f, -sqrt(3.0f) / 6.0f),
        glm::vec3(0.0f, -sqrt(6.0f) / 12.0f, sqrt(3.0f) / 3.0f)
    };

    const std::vector<glm::vec4> colors =
    {
        glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
        glm::vec4(0.1f, 0.5f, 0.1f, 1.0f),
        glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
        glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),
        glm::vec4(0.0f, 1.0f, 1.0f, 1.0f),
        glm::vec4(1.0f, 0.0f, 1.0f, 1.0f),
        glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    };
    enum {RED = 0, GREEN, BLUE, YELLOW, CYAN, MAGENTA, WHITE, BLACK};

    std::vector<glm::vec3> normals;
};

int main(int argc, char* argv[])
{
    Gtk::Main kit(argc, argv);
    Gtk::Window gtk_window;
    Gtk::VBox main_box;

    Graph_disp gl_window(sf::VideoMode(800, 600), -1, sf::ContextSettings(0, 0, 4, 4, 0)); // these do nothing yet - future SFML version should enable them

    gl_window.show();
    main_box.pack_start(gl_window);
    main_box.show();

    gtk_window.add(main_box);

    Gtk::Main::run(gtk_window);
    return 0;
}
