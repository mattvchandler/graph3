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
#include <utility>
#include <vector>

#include <cmath>

#include <png++/png.hpp> // TODO: look into using gtk or sfml for loading arbitrary images

#include <GL/glew.h>

#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>

#include <gtkmm/colorbutton.h>
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

#include "camera.h"
#include "graph.h"

// TODO: axes
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

std::pair<std::vector<float>, glm::ivec2> read_png(const char * filename)
{
    const png::image<png::rgba_pixel> image(filename);
    std::pair<std::vector<float>, glm::ivec2> ret(std::vector<float>(image.get_height() * image.get_width() * 4),
        glm::ivec2(image.get_height(), image.get_width()));

    for(size_t r = 0; r < image.get_height(); ++r)
    {
        for(size_t c = 0; c < image.get_width(); ++c)
        {
            ret.first[(image.get_width() * r + c) * 4 + 0] = (float)image[r][c].red / 255.0f;
            ret.first[(image.get_width() * r + c) * 4 + 1] = (float)image[r][c].green / 255.0f;
            ret.first[(image.get_width() * r + c) * 4 + 2] = (float)image[r][c].blue / 255.0f;
            ret.first[(image.get_width() * r + c) * 4 + 3] = (float)image[r][c].alpha / 255.0f;
        }
    }
    return ret;
}

struct Light
{
    glm::vec3 pos;
    glm::vec3 color;
    float strength;
    float const_attenuation;
    float linear_attenuation;
    float quad_attenuation;
};

class Cursor
{
public:
    Cursor(): tex(0), shininess(90.0f), specular(1.0f),
       _vao(0), _vbo(0), _num_indexes(0)
    {
    }

    ~Cursor()
    {
        if(_vao)
            glDeleteVertexArrays(1, &_vao);
        if(_vbo)
            glDeleteBuffers(1, &_vbo);
    }

    void draw() const
    {
        glBindVertexArray(_vao);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBindTexture(GL_TEXTURE_2D, tex);

        glDrawArrays(GL_TRIANGLES, 0, _num_indexes);
    }

    void build()
    {
        std::vector<glm::vec3> coords =
        {
            glm::vec3(0.0f, 0.0f, sqrtf(2) / 2.0f),
            glm::vec3(-0.5f, -0.5f, 0.0f),
            glm::vec3(0.5f, -0.5f, 0.0f),

            glm::vec3(0.0f, 0.0f, sqrtf(2) / 2.0f),
            glm::vec3(-0.5f, 0.5f, 0.0f),
            glm::vec3(-0.5f, -0.5f, 0.0f),

            glm::vec3(0.0f, 0.0f, sqrtf(2) / 2.0f),
            glm::vec3(0.5f, 0.5f, 0.0f),
            glm::vec3(-0.5f, 0.5f, 0.0f),

            glm::vec3(0.0f, 0.0f, sqrtf(2) / 2.0f),
            glm::vec3(0.5f, -0.5f, 0.0f),
            glm::vec3(0.5f, 0.5f, 0.0f),

            glm::vec3(0.0f, 0.0f, -sqrtf(2) / 2.0f),
            glm::vec3(0.5f, -0.5f, 0.0f),
            glm::vec3(-0.5f, -0.5f, 0.0f),

            glm::vec3(0.0f, 0.0f, -sqrtf(2) / 2.0f),
            glm::vec3(-0.5f, -0.5f, 0.0f),
            glm::vec3(-0.5f, 0.5f, 0.0f),

            glm::vec3(0.0f, 0.0f, -sqrtf(2) / 2.0f),
            glm::vec3(-0.5f, 0.5f, 0.0f),
            glm::vec3(0.5f, 0.5f, 0.0f),

            glm::vec3(0.0f, 0.0f, -sqrtf(2) / 2.0f),
            glm::vec3(0.5f, 0.5f, 0.0f),
            glm::vec3(0.5f, -0.5f, 0.0f)
        };
        std::vector<glm::vec2> tex_coords;
        std::vector<glm::vec3> normals;

        for(size_t i = 0; i < coords.size(); i += 3)
        {
            tex_coords.push_back(glm::vec2(0.5f, 1.0f - sqrtf(3.0f) / 2.0f));
            tex_coords.push_back(glm::vec2(0.0f, 1.0f));
            tex_coords.push_back(glm::vec2(1.0f, 1.0f));

            glm::vec3 norm = glm::normalize(glm::cross(coords[i + 1] - coords[i], coords[i + 2] - coords[i]));

            for(int j = 0; j < 3; ++j)
                normals.push_back(norm);
        }

        // OpenGL structs

        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);

        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * coords.size() + sizeof(glm::vec2) * tex_coords.size() +
            sizeof(glm::vec3) * normals.size(), NULL, GL_STATIC_DRAW);

        // store vertex data
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * coords.size(), &coords[0]);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * coords.size(), sizeof(glm::vec2) * tex_coords.size(), &tex_coords[0]);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * coords.size() + sizeof(glm::vec2) * tex_coords.size(),
            sizeof(glm::vec3) * normals.size(), &normals[0]);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(glm::vec3) * coords.size()));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(glm::vec3) * coords.size() + sizeof(glm::vec2) * tex_coords.size()));
        glEnableVertexAttribArray(2);

        _num_indexes = coords.size();
    }

    // material properties
    GLuint tex;
    float shininess;
    glm::vec3 specular;

private:
    GLuint _vao;
    GLuint _vbo;
    GLuint _num_indexes;

    // make non-copyable
    Cursor(const Cursor &) = delete;
    Cursor(const Cursor &&) = delete;
    Cursor & operator=(const Cursor &) = delete;
    Cursor & operator=(const Cursor &&) = delete;
};

class Graph_disp final: public SFMLWidget
{
public:
    Graph_disp(const sf::VideoMode & mode, const int size_reqest = - 1, const sf::ContextSettings & context_settings= sf::ContextSettings()):
        SFMLWidget(mode, size_reqest),
        test_graph(new Graph_cartesian("sqrt(1 - x^2 + y^2)", -2.0f, 2.0f, 50, -2.0f, 2.0f, 50)),
        cam(glm::vec3(0.0f, -10.0f, 2.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        perspective_mat(1.0f),
        light({glm::vec3(0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 0.8f, 1.0f, 0.5f, 0.0f}),
        ambient_light(0.4f, 0.4f, 0.4f)
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
        for(auto &i: textures)
        {
            if(i != 0)
                glDeleteTextures(1, &i);
        }
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
        GLuint graph_vert_shader = compile_shader("src/graph.vert", GL_VERTEX_SHADER);
        GLuint line_vert_shader = compile_shader("src/line.vert", GL_VERTEX_SHADER);
        GLuint tex_frag_shader = compile_shader("src/tex.frag", GL_FRAGMENT_SHADER);
        GLuint color_frag_shader = compile_shader("src/color.frag", GL_FRAGMENT_SHADER);

        if(graph_vert_shader == 0 || line_vert_shader == 0 || tex_frag_shader == 0 || color_frag_shader == 0)
            exit(EXIT_FAILURE);

        shader_prog_tex = link_shader_prog(std::vector<GLuint> {graph_vert_shader, tex_frag_shader});
        shader_prog_color = link_shader_prog(std::vector<GLuint> {graph_vert_shader, color_frag_shader});
        shader_prog_line = link_shader_prog(std::vector<GLuint> {line_vert_shader, color_frag_shader});

        if(shader_prog_tex == 0 || shader_prog_color == 0 || shader_prog_line == 0)
            exit(EXIT_FAILURE);

        glDeleteShader(graph_vert_shader);
        glDeleteShader(line_vert_shader);
        glDeleteShader(tex_frag_shader);
        glDeleteShader(color_frag_shader);

        // load images
        glEnable(GL_TEXTURE_2D);

        std::vector<std::pair<std::vector<float>, glm::ivec2>> texture_data;

        try
        {
            texture_data.push_back(read_png("img/test.png"));
            texture_data.push_back(read_png("img/cursor.png"));
        }
        catch(std::exception &e)
        {
            std::cerr<<"Error reading image file: "<<e.what()<<std::endl;
            exit(EXIT_FAILURE);
        }

        textures.resize(texture_data.size());
        glGenTextures(textures.size(), &textures[0]);

        for(size_t i = 0; i < textures.size(); ++i)
        {
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glTexStorage2D(GL_TEXTURE_2D, (int)(log2(std::min(texture_data[i].second.x, texture_data[i].second.y))) + 1,
                GL_RGBA8, texture_data[i].second.x, texture_data[i].second.y);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 512, 512, GL_RGBA, GL_FLOAT, &texture_data[i].first[0]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        // TODO: maybe have set methods that call this for us
        test_graph->build_graph();
        test_graph->tex = 0;
        test_graph->color = glm::vec4(0.2f, 0.5f, 0.2f, 1.0f);

        cursor.build();
        cursor_text = test_graph->cursor_text();
        cursor.tex = textures[1];
    }

    void resize(Gtk::Allocation & allocation)
    {
        if(m_refGdkWindow)
        {
            glViewport(0, 0, glWindow.getSize().x, glWindow.getSize().y);
            perspective_mat = glm::perspective(30.0f, (float)glWindow.getSize().x / (float)glWindow.getSize().y,
                0.1f, 100.0f);
            invalidate();
        }
    }

    bool draw(const Cairo::RefPtr<Cairo::Context> & cr)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(0xFFFFFFFF);

        // set up transformation matrices
        glm::mat4 view_model = cam.view_mat();
        glm::mat4 view_model_perspective = perspective_mat * view_model;
        glm::mat3 normal_transform = glm::transpose(glm::inverse(glm::mat3(view_model)));

        // set up light
        // TODO: add separate specular color
        // TODO: add material properties
        glm::vec3 light_pos_eye(0.0f);
        glm::vec3 light_forward(0.0f, 0.0f, 1.0f); // in eye space

        if(test_graph->tex != 0)
        {
            glUseProgram(shader_prog_tex);

            glUniformMatrix4fv(glGetUniformLocation(shader_prog_tex, "view_model_perspective"), 1, GL_FALSE, &view_model_perspective[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shader_prog_tex, "view_model"), 1, GL_FALSE, &view_model[0][0]);
            glUniformMatrix3fv(glGetUniformLocation(shader_prog_tex, "normal_transform"), 1, GL_FALSE, &normal_transform[0][0]);

            // light properties
            // TODO: store uniform locations
            // TODO: move unchanged vars elsewhere
            glUniform3fv(glGetUniformLocation(shader_prog_tex, "light_pos"), 1, &light_pos_eye[0]);
            glUniform3fv(glGetUniformLocation(shader_prog_tex, "cam_forward"), 1, &light_forward[0]);
            glUniform3fv(glGetUniformLocation(shader_prog_tex, "ambient_color"), 1, &ambient_light[0]);
            glUniform3fv(glGetUniformLocation(shader_prog_tex, "light_color"), 1, &light.color[0]);
            glUniform1f(glGetUniformLocation(shader_prog_tex, "light_strength"), light.strength);
            glUniform1f(glGetUniformLocation(shader_prog_tex, "const_atten"), light.const_attenuation);
            glUniform1f(glGetUniformLocation(shader_prog_tex, "linear_atten"), light.linear_attenuation);
            glUniform1f(glGetUniformLocation(shader_prog_tex, "quad_atten"), light.quad_attenuation);
            // material properties
            glUniform1f(glGetUniformLocation(shader_prog_tex, "shininess"), test_graph->shininess);
            glUniform3fv(glGetUniformLocation(shader_prog_tex, "specular"), 1, &test_graph->specular[0]);
        }
        else
        {
            glUseProgram(shader_prog_color);

            glUniformMatrix4fv(glGetUniformLocation(shader_prog_color, "view_model_perspective"), 1, GL_FALSE, &view_model_perspective[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shader_prog_color, "view_model"), 1, GL_FALSE, &view_model[0][0]);
            glUniformMatrix3fv(glGetUniformLocation(shader_prog_color, "normal_transform"), 1, GL_FALSE, &normal_transform[0][0]);

            // light properties
            // TODO: store uniform locations
            // TODO: move unchanged vars elsewhere
            glUniform3fv(glGetUniformLocation(shader_prog_color, "light_pos"), 1, &light_pos_eye[0]);
            glUniform3fv(glGetUniformLocation(shader_prog_color, "cam_forward"), 1, &light_forward[0]);
            glUniform3fv(glGetUniformLocation(shader_prog_color, "ambient_color"), 1, &ambient_light[0]);
            glUniform3fv(glGetUniformLocation(shader_prog_color, "light_color"), 1, &light.color[0]);
            glUniform1f(glGetUniformLocation(shader_prog_color, "light_strength"), light.strength);
            glUniform1f(glGetUniformLocation(shader_prog_color, "const_atten"), light.const_attenuation);
            glUniform1f(glGetUniformLocation(shader_prog_color, "linear_atten"), light.linear_attenuation);
            glUniform1f(glGetUniformLocation(shader_prog_color, "quad_atten"), light.quad_attenuation);
            // material properties
            glUniform4fv(glGetUniformLocation(shader_prog_color, "color"), 1, &graph_color[0]);
            glUniform1f(glGetUniformLocation(shader_prog_color, "shininess"), test_graph->shininess);
            glUniform3fv(glGetUniformLocation(shader_prog_color, "specular"), 1, &test_graph->specular[0]);
        }

        check_error("pre draw");

        test_graph->draw();

        // switch to line shader
        glUseProgram(shader_prog_line);

        glUniformMatrix4fv(glGetUniformLocation(shader_prog_line, "perspective"), 1, GL_FALSE, &perspective_mat[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shader_prog_line, "view_model"), 1, GL_FALSE, &view_model[0][0]);
        glUniformMatrix3fv(glGetUniformLocation(shader_prog_line, "normal_transform"), 1, GL_FALSE, &normal_transform[0][0]);

        // light properties
        // TODO: store uniform locations
        // TODO: move unchanged vars elsewhere
        glUniform3fv(glGetUniformLocation(shader_prog_line, "light_pos"), 1, &light_pos_eye[0]);
        glUniform3fv(glGetUniformLocation(shader_prog_line, "cam_forward"), 1, &light_forward[0]);
        glUniform3fv(glGetUniformLocation(shader_prog_line, "ambient_color"), 1, &ambient_light[0]);
        glUniform3fv(glGetUniformLocation(shader_prog_line, "light_color"), 1, &light.color[0]);
        glUniform1f(glGetUniformLocation(shader_prog_line, "light_strength"), light.strength);
        glUniform1f(glGetUniformLocation(shader_prog_line, "const_atten"), light.const_attenuation);
        glUniform1f(glGetUniformLocation(shader_prog_line, "linear_atten"), light.linear_attenuation);
        glUniform1f(glGetUniformLocation(shader_prog_line, "quad_atten"), light.quad_attenuation);
        // material properties
        glUniform4fv(glGetUniformLocation(shader_prog_line, "color"), 1, &test_graph->grid_color[0]);
        glUniform1f(glGetUniformLocation(shader_prog_line, "shininess"), test_graph->grid_shininess);
        glUniform3fv(glGetUniformLocation(shader_prog_line, "specular"), 1, &test_graph->grid_specular[0]);

        test_graph->draw_grid();

        check_error("draw");

        // draw cursor
        if(test_graph->cursor_defined())
        {
            glUseProgram(shader_prog_tex);

            view_model = glm::translate(cam.view_mat(), test_graph->cursor_pos()) * glm::scale(glm::mat4(), glm::vec3(0.25f));
            view_model_perspective = perspective_mat * view_model;
            normal_transform = glm::transpose(glm::inverse(glm::mat3(view_model)));

            glUniformMatrix4fv(glGetUniformLocation(shader_prog_tex, "view_model_perspective"), 1, GL_FALSE, &view_model_perspective[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(shader_prog_tex, "view_model"), 1, GL_FALSE, &view_model[0][0]);
            glUniformMatrix3fv(glGetUniformLocation(shader_prog_tex, "normal_transform"), 1, GL_FALSE, &normal_transform[0][0]);

            // light properties
            // TODO: store uniform locations
            // TODO: move unchanged vars elsewhere
            glUniform3fv(glGetUniformLocation(shader_prog_tex, "light_pos"), 1, &light_pos_eye[0]);
            glUniform3fv(glGetUniformLocation(shader_prog_tex, "cam_forward"), 1, &light_forward[0]);
            glUniform3fv(glGetUniformLocation(shader_prog_tex, "ambient_color"), 1, &ambient_light[0]);
            glUniform3fv(glGetUniformLocation(shader_prog_tex, "light_color"), 1, &light.color[0]);
            glUniform1f(glGetUniformLocation(shader_prog_tex, "light_strength"), light.strength);
            glUniform1f(glGetUniformLocation(shader_prog_tex, "const_atten"), light.const_attenuation);
            glUniform1f(glGetUniformLocation(shader_prog_tex, "linear_atten"), light.linear_attenuation);
            glUniform1f(glGetUniformLocation(shader_prog_tex, "quad_atten"), light.quad_attenuation);

            // material properties
            // TODO: store uniform locations
            // TODO: move unchanged vars elsewhere
            glUniform1f(glGetUniformLocation(shader_prog_tex, "shininess"), cursor.shininess);
            glUniform3fv(glGetUniformLocation(shader_prog_tex, "specular"), 1, &cursor.specular[0]);

            cursor.draw();
        }

        check_error("post draw");
        display();
        return true;
    }

    bool input()
    {
        static std::unordered_map<sf::Keyboard::Key, bool, std::hash<int>> key_lock;
        static sf::Vector2i old_mouse_pos = sf::Mouse::getPosition(glWindow);
        static sf::Clock cursor_delay;

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
                // Camera controls
                // reset
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::R) && !key_lock[sf::Keyboard::R])
                {
                    key_lock[sf::Keyboard::R] = true;
                    cam.set();
                    invalidate();
                }
                else if(!sf::Keyboard::isKeyPressed(sf::Keyboard::R))
                    key_lock[sf::Keyboard::R] = false;

                float scale = 0.1f;

                if(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
                {
                    scale *= 2.0f;
                }

                if(sf::Keyboard::isKeyPressed(sf::Keyboard::LControl))
                {
                    scale *= 0.1f;
                }

                if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                {
                    cam.translate(scale * glm::normalize(glm::vec3(cam.forward().x, cam.forward().y, 0.0f)));
                    invalidate();
                }

                if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                {
                    cam.translate(-scale * glm::normalize(glm::vec3(cam.forward().x, cam.forward().y, 0.0f)));
                    invalidate();
                }

                if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
                {
                    cam.translate(-scale * cam.right());
                    invalidate();
                }

                if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
                {
                    cam.translate(scale * cam.right());
                    invalidate();
                }

                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
                {
                    cam.translate(scale * glm::vec3(0.0f, 0.0f, 1.0f));
                    invalidate();
                }

                if(sf::Keyboard::isKeyPressed(sf::Keyboard::E))
                {
                    cam.translate(-scale * glm::vec3(0.0f, 0.0f, 1.0f));
                    invalidate();
                }

                if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
                {
                    int d_x = new_mouse_pos.x - old_mouse_pos.x;
                    int d_y = new_mouse_pos.y - old_mouse_pos.y;

                    cam.rotate(0.01f * scale * d_y, cam.right());
                    cam.rotate(0.01f * scale * d_x, glm::vec3(0.0f, 0.0f, 1.0f));

                    invalidate();
                }

                // Cursor controls
                int cursor_timeout = 200;
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && cursor_delay.getElapsedTime().asMilliseconds() >= cursor_timeout)
                {
                    test_graph->move_cursor(Graph::UP);
                    cursor_delay.restart();
                    cursor_text = test_graph->cursor_text();
                    invalidate();
                }

                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && cursor_delay.getElapsedTime().asMilliseconds() >= cursor_timeout)
                {
                    test_graph->move_cursor(Graph::DOWN);
                    cursor_delay.restart();
                    cursor_text = test_graph->cursor_text();
                    invalidate();
                }

                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && cursor_delay.getElapsedTime().asMilliseconds() >= cursor_timeout)
                {
                    test_graph->move_cursor(Graph::LEFT);
                    cursor_delay.restart();
                    cursor_text = test_graph->cursor_text();
                    invalidate();
                }

                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && cursor_delay.getElapsedTime().asMilliseconds() >= cursor_timeout)
                {
                    test_graph->move_cursor(Graph::RIGHT);
                    cursor_delay.restart();
                    cursor_text = test_graph->cursor_text();
                    invalidate();
                }
                
                // TODO: pgup/ pgdn to switch cursor to different graph
            }
            old_mouse_pos = new_mouse_pos;
        }
        return true;
    }

    std::string cursor_text;
    glm::vec4 graph_color;

private:
    std::unique_ptr<Graph> test_graph;
    Cursor cursor;
    GLuint shader_prog_tex;
    GLuint shader_prog_color;
    GLuint shader_prog_line;
    std::vector<GLuint> textures;

    Camera cam;
    glm::mat4 perspective_mat;
    Light light;
    glm::vec3 ambient_light;

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
        this->set_title("Graph 3");
        set_default_size(800, 600);
        gl_window.set_hexpand(true);
        gl_window.set_vexpand(true);

        add(main_grid);
        main_grid.set_column_spacing(5);

        main_grid.attach(gl_window, 0, 0, 1, 9);
        main_grid.attach(cursor_text, 0, 9, 1, 1);
        main_grid.attach(color_but, 1, 0, 1, 1);
        show_all_children();

        Glib::signal_timeout().connect(sigc::mem_fun(*this, &Graph_window::update), 100);

        Gdk::RGBA start_rgba;
        start_rgba.set_rgba(0.2, 0.5, 0.2, 1.0);
        color_but.set_rgba(start_rgba);
    }

    bool update()
    {
        cursor_text.set_label(gl_window.cursor_text);

        // TODO: move to callback, clean up interface set to graph material properties
        gl_window.graph_color.r = color_but.get_rgba().get_red();
        gl_window.graph_color.g = color_but.get_rgba().get_green();
        gl_window.graph_color.b = color_but.get_rgba().get_blue();
        gl_window.graph_color.a = 1.0f;
        gl_window.invalidate();

        return true;
    }

private:
    Graph_disp gl_window;
    Gtk::Grid main_grid;

    Gtk::Label cursor_text;
    Gtk::ColorButton color_but;
};

int main(int argc, char* argv[])
{
    Gtk::Main kit(argc, argv);
    Graph_window gtk_window;

    Gtk::Main::run(gtk_window);
    return 0;
}
