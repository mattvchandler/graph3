// graph_disp.cpp
// Graphics display module

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
#include <fstream>
#include <utility>
#include <cmath>

#include <png++/png.hpp> // TODO: look into using gtk or sfml for loading arbitrary images

#include <glm/gtc/matrix_transform.hpp>

#include "graph_disp.hpp"

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

Cursor::Cursor(): tex(0), shininess(90.0f), specular(1.0f),
    _vao(0), _vbo(0), _num_indexes(0)
{
}

Cursor::~Cursor()
{
    if(_vao)
        glDeleteVertexArrays(1, &_vao);
    if(_vbo)
        glDeleteBuffers(1, &_vbo);
}
void Cursor::draw() const
{
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBindTexture(GL_TEXTURE_2D, tex);

    glDrawArrays(GL_TRIANGLES, 0, _num_indexes);
}

void Cursor::build()
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

Graph_disp::Graph_disp(const sf::VideoMode & mode, const int size_reqest, const sf::ContextSettings & context_settings):
    SFMLWidget(mode, size_reqest), active_graph(0),
    cam(glm::vec3(0.0f, -10.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
    perspective_mat(1.0f),
    light({glm::vec3(0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 0.8f, 1.0f, 0.5f, 0.0f}),
    ambient_light(0.4f, 0.4f, 0.4f)
{
    signal_realize().connect(sigc::mem_fun(*this, &Graph_disp::realize));
    signal_size_allocate().connect(sigc::mem_fun(*this, &Graph_disp::resize));
    signal_draw().connect(sigc::mem_fun(*this, &Graph_disp::draw));
    signal_key_press_event().connect(sigc::mem_fun(*this, &Graph_disp::key_press));
    Glib::signal_timeout().connect(sigc::mem_fun(*this, &Graph_disp::input), 10);
    // Glib::signal_idle().connect(sigc::mem_fun(*this, &Graph_disp::idle));

    set_can_focus();
    set_can_default();
}

Graph_disp::~Graph_disp()
{
    for(auto &i: textures)
    {
        if(i != 0)
            glDeleteTextures(1, &i);
    }
}

// key press handler
bool Graph_disp::key_press(GdkEventKey * e)
{
    // don't propagate key presses up - capture them in this widget
    return true;
}

void Graph_disp::realize()
{
    // TODO: add check for required OpenGL features
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
    GLuint graph_vert = compile_shader("src/graph.vert", GL_VERTEX_SHADER);
    GLuint line_vert = compile_shader("src/line.vert", GL_VERTEX_SHADER);
    GLuint tex_frag = compile_shader("src/tex.frag", GL_FRAGMENT_SHADER);
    GLuint color_frag = compile_shader("src/color.frag", GL_FRAGMENT_SHADER);
    GLuint flat_color_frag = compile_shader("src/flat_color.frag", GL_FRAGMENT_SHADER);

    if(graph_vert == 0 || line_vert == 0 || tex_frag == 0 || color_frag == 0 || flat_color_frag == 0)
        exit(EXIT_FAILURE);

    prog_tex = link_shader_prog(std::vector<GLuint> {graph_vert, tex_frag});
    prog_color = link_shader_prog(std::vector<GLuint> {graph_vert, color_frag});
    prog_line = link_shader_prog(std::vector<GLuint> {line_vert, flat_color_frag});

    if(prog_tex == 0 || prog_color == 0 || prog_line == 0)
        exit(EXIT_FAILURE);

    glDeleteShader(graph_vert);
    glDeleteShader(line_vert);
    glDeleteShader(tex_frag);
    glDeleteShader(color_frag);
    glDeleteShader(flat_color_frag);

    // get uniform locations
    prog_tex_uniforms["view_model_perspective"] = glGetUniformLocation(prog_tex, "view_model_perspective");
    prog_tex_uniforms["view_model"] = glGetUniformLocation(prog_tex, "view_model");
    prog_tex_uniforms["normal_transform"] = glGetUniformLocation(prog_tex, "normal_transform");
    prog_tex_uniforms["shininess"] = glGetUniformLocation(prog_tex, "shininess");
    prog_tex_uniforms["specular"] = glGetUniformLocation(prog_tex, "specular");
    prog_tex_uniforms["ambient_color"] = glGetUniformLocation(prog_tex, "ambient_color");
    prog_tex_uniforms["light_color"] = glGetUniformLocation(prog_tex, "light_color");
    prog_tex_uniforms["light_pos"] = glGetUniformLocation(prog_tex, "light_pos");
    prog_tex_uniforms["light_strength"] = glGetUniformLocation(prog_tex, "light_strength");
    prog_tex_uniforms["const_atten"] = glGetUniformLocation(prog_tex, "const_atten");
    prog_tex_uniforms["linear_atten"] = glGetUniformLocation(prog_tex, "linear_atten");
    prog_tex_uniforms["quad_atten"] = glGetUniformLocation(prog_tex, "quad_atten");
    prog_tex_uniforms["cam_forward"] = glGetUniformLocation(prog_tex, "cam_forward");

    prog_color_uniforms["view_model_perspective"] = glGetUniformLocation(prog_color, "view_model_perspective");
    prog_color_uniforms["view_model"] = glGetUniformLocation(prog_color, "view_model");
    prog_color_uniforms["normal_transform"] = glGetUniformLocation(prog_color, "normal_transform");
    prog_color_uniforms["color"] = glGetUniformLocation(prog_color, "color");
    prog_color_uniforms["shininess"] = glGetUniformLocation(prog_color, "shininess");
    prog_color_uniforms["specular"] = glGetUniformLocation(prog_color, "specular");
    prog_color_uniforms["ambient_color"] = glGetUniformLocation(prog_color, "ambient_color");
    prog_color_uniforms["light_color"] = glGetUniformLocation(prog_color, "light_color");
    prog_color_uniforms["light_pos"] = glGetUniformLocation(prog_color, "light_pos");
    prog_color_uniforms["light_strength"] = glGetUniformLocation(prog_color, "light_strength");
    prog_color_uniforms["const_atten"] = glGetUniformLocation(prog_color, "const_atten");
    prog_color_uniforms["linear_atten"] = glGetUniformLocation(prog_color, "linear_atten");
    prog_color_uniforms["quad_atten"] = glGetUniformLocation(prog_color, "quad_atten");
    prog_color_uniforms["cam_forward"] = glGetUniformLocation(prog_color, "cam_forward");

    prog_line_uniforms["perspective"] = glGetUniformLocation(prog_line, "perspective");
    prog_line_uniforms["view_model"] = glGetUniformLocation(prog_line, "view_model");
    prog_line_uniforms["color"] = glGetUniformLocation(prog_line, "color");

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

    // set up un-changing lighting values
    glm::vec3 light_pos_eye(0.0f);
    glm::vec3 light_forward(0.0f, 0.0f, 1.0f); // in eye space

    glUseProgram(prog_tex);
    glUniform3fv(prog_tex_uniforms["ambient_color"], 1, &ambient_light[0]);
    glUniform3fv(prog_tex_uniforms["light_color"], 1, &light.color[0]);
    glUniform3fv(prog_tex_uniforms["light_pos"], 1, &light_pos_eye[0]);
    glUniform1f(prog_tex_uniforms["light_strength"], light.strength);
    glUniform1f(prog_tex_uniforms["const_atten"], light.const_attenuation);
    glUniform1f(prog_tex_uniforms["linear_atten"], light.linear_attenuation);
    glUniform1f(prog_tex_uniforms["quad_atten"], light.quad_attenuation);
    glUniform3fv(prog_tex_uniforms["cam_forward"], 1, &light_forward[0]);

    glUseProgram(prog_color);
    glUniform3fv(prog_color_uniforms["ambient_color"], 1, &ambient_light[0]);
    glUniform3fv(prog_color_uniforms["light_color"], 1, &light.color[0]);
    glUniform3fv(prog_color_uniforms["light_pos"], 1, &light_pos_eye[0]);
    glUniform1f(prog_color_uniforms["light_strength"], light.strength);
    glUniform1f(prog_color_uniforms["const_atten"], light.const_attenuation);
    glUniform1f(prog_color_uniforms["linear_atten"], light.linear_attenuation);
    glUniform1f(prog_color_uniforms["quad_atten"], light.quad_attenuation);
    glUniform3fv(prog_color_uniforms["cam_forward"], 1, &light_forward[0]);

    cursor.build();
    cursor.tex = textures[1];

    invalidate();
}

void Graph_disp::resize(Gtk::Allocation & allocation)
{
    if(m_refGdkWindow)
    {
        glViewport(0, 0, glWindow.getSize().x, glWindow.getSize().y);
        perspective_mat = glm::perspective(30.0f, (float)glWindow.getSize().x / (float)glWindow.getSize().y,
            0.1f, 100.0f);
        invalidate();
    }
}

bool Graph_disp::draw(const Cairo::RefPtr<Cairo::Context> & cr)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFFFFFF);

    // set up transformation matrices
    glm::mat4 view_model = cam.view_mat();
    glm::mat4 view_model_perspective = perspective_mat * view_model;
    glm::mat3 normal_transform = glm::transpose(glm::inverse(glm::mat3(view_model)));


    for(auto &graph: graphs)
    {
        if(graph->tex != 0)
        {
            glUseProgram(prog_tex);

            glUniformMatrix4fv(prog_tex_uniforms["view_model_perspective"], 1, GL_FALSE, &view_model_perspective[0][0]);
            glUniformMatrix4fv(prog_tex_uniforms["view_model"], 1, GL_FALSE, &view_model[0][0]);
            glUniformMatrix3fv(prog_tex_uniforms["normal_transform"], 1, GL_FALSE, &normal_transform[0][0]);

            // material properties
            glUniform1f(prog_tex_uniforms["shininess"], graph->shininess);
            glUniform3fv(prog_tex_uniforms["specular"], 1, &graph->specular[0]);
        }
        else
        {
            glUseProgram(prog_color);

            glUniformMatrix4fv(prog_color_uniforms["view_model_perspective"], 1, GL_FALSE, &view_model_perspective[0][0]);
            glUniformMatrix4fv(prog_color_uniforms["view_model"], 1, GL_FALSE, &view_model[0][0]);
            glUniformMatrix3fv(prog_color_uniforms["normal_transform"], 1, GL_FALSE, &normal_transform[0][0]);

            // material properties
            glUniform4fv(prog_color_uniforms["color"], 1, &graph->color[0]);
            glUniform1f(prog_color_uniforms["shininess"], graph->shininess);
            glUniform3fv(prog_color_uniforms["specular"], 1, &graph->specular[0]);
        }

        check_error("pre draw");

        graph->draw();

        // switch to line shader
        glUseProgram(prog_line);

        glUniformMatrix4fv(prog_line_uniforms["perspective"], 1, GL_FALSE, &perspective_mat[0][0]);
        glUniformMatrix4fv(prog_line_uniforms["view_model"], 1, GL_FALSE, &view_model[0][0]);

        // material properties
        glUniform4fv(prog_line_uniforms["color"], 1, &graph->grid_color[0]);

        graph->draw_grid();

        if(false)
        {
            glUniform4fv(prog_line_uniforms["color"], 1, &graph->normal_color[0]);

            graph->draw_normals();

            check_error("draw");
        }
    }

    if(active_graph < graphs.size())
    {
        // draw cursor
        if(graphs[active_graph]->cursor_defined())
        {
            glUseProgram(prog_tex);

            view_model = glm::translate(cam.view_mat(), graphs[active_graph]->cursor_pos()) * glm::scale(glm::mat4(), glm::vec3(0.25f));
            view_model_perspective = perspective_mat * view_model;
            normal_transform = glm::transpose(glm::inverse(glm::mat3(view_model)));

            glUniformMatrix4fv(prog_tex_uniforms["view_model_perspective"], 1, GL_FALSE, &view_model_perspective[0][0]);
            glUniformMatrix4fv(prog_tex_uniforms["view_model"], 1, GL_FALSE, &view_model[0][0]);
            glUniformMatrix3fv(prog_tex_uniforms["normal_transform"], 1, GL_FALSE, &normal_transform[0][0]);

            // material properties
            glUniform1f(prog_tex_uniforms["shininess"], cursor.shininess);
            glUniform3fv(prog_tex_uniforms["specular"], 1, &cursor.specular[0]);

            cursor.draw();
        }
    }

    check_error("post draw");
    display();
    return true;
}

// TODO: add zoom w/ mouse wheel
bool Graph_disp::input()
{
    static std::unordered_map<sf::Keyboard::Key, bool, std::hash<int>> key_lock;
    static sf::Vector2i old_mouse_pos = sf::Mouse::getPosition(glWindow);
    static sf::Clock cursor_delay;

    // the neat thing about having this in a timeout func is that we
    // don't need to calc dt for movement controls.
    // it is always (almost) exactly 10ms
    if(dynamic_cast<Gtk::Window *>(get_toplevel())->is_active())
    {
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

            if(active_graph < graphs.size())
            {
                // Cursor controls
                int cursor_timeout = 200;
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && cursor_delay.getElapsedTime().asMilliseconds() >= cursor_timeout)
                {
                    graphs[active_graph]->move_cursor(Graph::UP);
                    cursor_delay.restart();
                    invalidate();
                }

                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && cursor_delay.getElapsedTime().asMilliseconds() >= cursor_timeout)
                {
                    graphs[active_graph]->move_cursor(Graph::DOWN);
                    cursor_delay.restart();
                    invalidate();
                }

                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && cursor_delay.getElapsedTime().asMilliseconds() >= cursor_timeout)
                {
                    graphs[active_graph]->move_cursor(Graph::LEFT);
                    cursor_delay.restart();
                    invalidate();
                }

                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && cursor_delay.getElapsedTime().asMilliseconds() >= cursor_timeout)
                {
                    graphs[active_graph]->move_cursor(Graph::RIGHT);
                    cursor_delay.restart();
                    invalidate();
                }

                // change active graph w/ page up/down
                if(graphs.size() > 1)
                {
                    if(sf::Keyboard::isKeyPressed(sf::Keyboard::PageUp) && cursor_delay.getElapsedTime().asMilliseconds() >= cursor_timeout)
                    {
                        if(++active_graph == graphs.size())
                            active_graph = 0;

                        cursor_delay.restart();
                        invalidate();
                    }

                    if(sf::Keyboard::isKeyPressed(sf::Keyboard::PageDown) && cursor_delay.getElapsedTime().asMilliseconds() >= cursor_timeout)
                    {
                        if(active_graph-- == 0)
                            active_graph = graphs.size() - 1;

                        cursor_delay.restart();
                        invalidate();
                    }
                }
            }
        }
        old_mouse_pos = new_mouse_pos;
    }
    return true;
}