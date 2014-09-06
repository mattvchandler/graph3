// graph_disp.cpp
// Graphics display module

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

#include <iostream>

#include <gtkmm/window.h>

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#include "gl_helpers.hpp"
#include "graph_disp.hpp"

extern int return_code; // from main.cpp

Cursor::Cursor(): shininess(90.0f), specular(1.0f),
    _tex(0), _vao(0), _vbo(0), _num_indexes(0)
{
}

Cursor::~Cursor()
{
    // free OpenGL resources
    if(_tex)
        glDeleteTextures(1, &_tex);
    if(_vao)
        glDeleteVertexArrays(1, &_vao);
    if(_vbo)
        glDeleteBuffers(1, &_vbo);
}

void Cursor::draw() const
{
    // load the vertices and texture
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBindTexture(GL_TEXTURE_2D, _tex);

    // draw the geometry
    glDrawArrays(GL_TRIANGLES, 0, _num_indexes);
}

// create a new cursor
void Cursor::build(const std::string & tex_file_name)
{
    // vertex coordinates
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

    // assign texture coords and normals
    for(size_t i = 0; i < coords.size(); i += 3)
    {
        tex_coords.push_back(glm::vec2(0.5f, 1.0f - sqrtf(3.0f) / 2.0f));
        tex_coords.push_back(glm::vec2(0.0f, 1.0f));
        tex_coords.push_back(glm::vec2(1.0f, 1.0f));

        glm::vec3 norm = glm::normalize(glm::cross(coords[i + 1] - coords[i], coords[i + 2] - coords[i]));

        for(int j = 0; j < 3; ++j)
            normals.push_back(norm);
    }

    // create OpenGL vertex objects
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * coords.size() + sizeof(glm::vec2) * tex_coords.size() +
        sizeof(glm::vec3) * normals.size(), NULL, GL_STATIC_DRAW);

    // store vertex data
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * coords.size(), coords.data());
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * coords.size(), sizeof(glm::vec2) * tex_coords.size(), tex_coords.data());
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * coords.size() + sizeof(glm::vec2) * tex_coords.size(),
        sizeof(glm::vec3) * normals.size(), normals.data());

    // set pointers to coordinates, texture coords, normals
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(glm::vec3) * coords.size()));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(glm::vec3) * coords.size() + sizeof(glm::vec2) * tex_coords.size()));
    glEnableVertexAttribArray(2);

    _num_indexes = coords.size();

    try
    {
        _tex = create_texture_from_file(tex_file_name);
    }
    catch(Glib::Exception &e)
    {
        std::cerr<<"Error reading cursor image file:"<<std::endl<<e.what()<<std::endl;
        throw;
    }
}

Axes::Axes(): color(0.0f, 0.0f, 0.0f, 1.0f), _vao(0), _vbo(0), _num_indexes(0)
{
}

Axes::~Axes()
{
    if(_vao)
        glDeleteVertexArrays(1, &_vao);
    if(_vbo)
        glDeleteBuffers(1, &_vbo);
}
void Axes::draw() const
{
    // load the vertices and texture
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    // draw the geometry
    glDrawArrays(GL_LINES, 0, _num_indexes);
}

void Axes::build()
{
    // vertex coordinates
    std::vector<glm::vec3> coords =
    {
        // X axis
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        // Y axis
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        // Z axis
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    };

    // create OpenGL vertex objects
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * coords.size(), coords.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    _num_indexes = coords.size();
}

Graph_disp::Graph_disp(const sf::VideoMode & mode, const int size_request):
    SFMLWidget(mode, size_request),
    draw_cursor_flag(true), draw_axes_flag(true), use_orbit_cam(true),
    cam_light({glm::vec3(0.0f), glm::vec3(1.0f), 0.2f, 1.0f, 0.5f, 0.0f}),
    dir_light({glm::vec3(-1.0f), glm::vec3(0.5f), 0.2f, 1.0f, 1.0f, 1.0f}),
    _prog_tex(0), _prog_color(0), _prog_line(0),
    _cam(glm::vec3(0.0f, -10.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
    _orbit_cam({10.0f, 0.0f, (float)M_PI / 2.0f}), _scale(1.0f), _perspective(1.0f),
    _ambient_color(0.4f, 0.4f, 0.4f),
    _active_graph(nullptr)

{
    // All OpenGL initialization has to wait until the drawing context actually exists
    // it is in the realize method - hook into that signal now
    signal_realize().connect(sigc::mem_fun(*this, &Graph_disp::realize));

    // connect event signals
    signal_size_allocate().connect(sigc::mem_fun(*this, &Graph_disp::resize));
    signal_draw().connect(sigc::mem_fun(*this, &Graph_disp::draw));
    signal_key_press_event().connect(sigc::mem_fun(*this, &Graph_disp::key_press));

    // input is checked every 10ms
    Glib::signal_timeout().connect(sigc::mem_fun(*this, &Graph_disp::input), 10);

    set_can_focus();
    set_can_default();
}

Graph_disp::~Graph_disp()
{
    if(_prog_tex)
        glDeleteProgram(_prog_tex);
    if(_prog_color)
        glDeleteProgram(_prog_color);
    if(_prog_line)
        glDeleteProgram(_prog_line);
}

// set and get the active graph (the one w/ the cursor on it)
void Graph_disp::set_active_graph(Graph * graph)
{
    _active_graph = graph;
}

// give and take graphs from the display
void Graph_disp::add_graph(const Graph * graph)
{
    _graphs.insert(graph);
}

void Graph_disp::remove_graph(const Graph * graph)
{
    if(graph == _active_graph)
        _active_graph = nullptr;
    _graphs.erase(graph);
}

// reset camera to starting position / orientation
void Graph_disp::reset_cam()
{
    if(use_orbit_cam)
    {
        _orbit_cam.r = 10.0f;
        _orbit_cam.theta = 0.0f;
        _orbit_cam.phi = (float)M_PI / 2.0f;
    }
    else
    {
        _cam.set(glm::vec3(0.0f, -10.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    }

    _scale = 1.0f;
    invalidate();
}

// called when OpenGL context is ready and GTK widget is ready
void Graph_disp::realize()
{
    // init glew
    if(glewInit() != GLEW_OK)
    {
        std::cerr<<"Error loading glew. Aborting"<<std::endl;
        dynamic_cast<Gtk::Window *>(get_toplevel())->hide();
        return_code = EXIT_FAILURE;
        return;
    }

    // check for required OpenGL version
    if(!GLEW_VERSION_3_0)
    {
        std::cerr<<"OpenGL version too low. Version 3.0 required"<<std::endl;
        std::cerr<<"Installed version is: "<<glGetString(GL_VERSION)<<std::endl;
        dynamic_cast<Gtk::Window *>(get_toplevel())->hide();
        return_code = EXIT_FAILURE;
        return;
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
    GLuint graph_vert = compile_shader("shaders/graph.vert", GL_VERTEX_SHADER);
    GLuint line_vert = compile_shader("shaders/line.vert", GL_VERTEX_SHADER);
    GLuint tex_frag = compile_shader("shaders/tex.frag", GL_FRAGMENT_SHADER);
    GLuint color_frag = compile_shader("shaders/color.frag", GL_FRAGMENT_SHADER);
    GLuint flat_color_frag = compile_shader("shaders/flat_color.frag", GL_FRAGMENT_SHADER);

    if(graph_vert == 0 || line_vert == 0 || tex_frag == 0 || color_frag == 0 || flat_color_frag == 0)
    {
        // error messages are displayed by the compile_shader function
        dynamic_cast<Gtk::Window *>(get_toplevel())->hide();
        return_code = EXIT_FAILURE;
        return;
    }

    // link shaders
    _prog_tex = link_shader_prog(std::vector<GLuint> {graph_vert, tex_frag},
        {std::make_pair(0, "vert_pos"), std::make_pair(1, "vert_tex_coords"), std::make_pair(2, "vert_normal")});
    _prog_color = link_shader_prog(std::vector<GLuint> {graph_vert, color_frag},
        {std::make_pair(0, "vert_pos"), std::make_pair(1, "vert_tex_coords"), std::make_pair(2, "vert_normal")});
    _prog_line = link_shader_prog(std::vector<GLuint> {line_vert, flat_color_frag},
        {std::make_pair(0, "vert_pos")});

    if(_prog_tex == 0 || _prog_color == 0 || _prog_line == 0)
    {
        // error messages are displayed by the link_shader_prog function
        dynamic_cast<Gtk::Window *>(get_toplevel())->hide();
        return_code = EXIT_FAILURE;
        return;
    }

    check_error("build shaders");

    // free shader objects
    glDeleteShader(graph_vert);
    glDeleteShader(line_vert);
    glDeleteShader(tex_frag);
    glDeleteShader(color_frag);
    glDeleteShader(flat_color_frag);

    // get uniform locations for each shader
    _prog_tex_uniforms["view_model_perspective"] = glGetUniformLocation(_prog_tex, "view_model_perspective");
    _prog_tex_uniforms["view_model"] = glGetUniformLocation(_prog_tex, "view_model");
    _prog_tex_uniforms["normal_transform"] = glGetUniformLocation(_prog_tex, "normal_transform");
    _prog_tex_uniforms["shininess"] = glGetUniformLocation(_prog_tex, "shininess");
    _prog_tex_uniforms["specular"] = glGetUniformLocation(_prog_tex, "specular");
    _prog_tex_uniforms["ambient_color"] = glGetUniformLocation(_prog_tex, "ambient_color");
    _prog_tex_uniforms["cam_light_color"] = glGetUniformLocation(_prog_tex, "cam_light_color");
    _prog_tex_uniforms["cam_light_pos_eye"] = glGetUniformLocation(_prog_tex, "cam_light_pos_eye");
    _prog_tex_uniforms["cam_light_strength"] = glGetUniformLocation(_prog_tex, "cam_light_strength");
    _prog_tex_uniforms["const_atten"] = glGetUniformLocation(_prog_tex, "const_atten");
    _prog_tex_uniforms["linear_atten"] = glGetUniformLocation(_prog_tex, "linear_atten");
    _prog_tex_uniforms["quad_atten"] = glGetUniformLocation(_prog_tex, "quad_atten");
    _prog_tex_uniforms["dir_light_color"] = glGetUniformLocation(_prog_tex, "dir_light_color");
    _prog_tex_uniforms["dir_light_dir"] = glGetUniformLocation(_prog_tex, "dir_light_dir");
    _prog_tex_uniforms["dir_light_strength"] = glGetUniformLocation(_prog_tex, "dir_light_strength");
    _prog_tex_uniforms["dir_half_vec"] = glGetUniformLocation(_prog_tex, "dir_half_vec");
    _prog_tex_uniforms["light_forward"] = glGetUniformLocation(_prog_tex, "light_forward");
    check_error("_prog_tex GetUniformLocation");

    _prog_color_uniforms["view_model_perspective"] = glGetUniformLocation(_prog_color, "view_model_perspective");
    _prog_color_uniforms["view_model"] = glGetUniformLocation(_prog_color, "view_model");
    _prog_color_uniforms["normal_transform"] = glGetUniformLocation(_prog_color, "normal_transform");
    _prog_color_uniforms["color"] = glGetUniformLocation(_prog_color, "color");
    _prog_color_uniforms["shininess"] = glGetUniformLocation(_prog_color, "shininess");
    _prog_color_uniforms["specular"] = glGetUniformLocation(_prog_color, "specular");
    _prog_color_uniforms["ambient_color"] = glGetUniformLocation(_prog_color, "ambient_color");
    _prog_color_uniforms["cam_light_color"] = glGetUniformLocation(_prog_color, "cam_light_color");
    _prog_color_uniforms["cam_light_pos_eye"] = glGetUniformLocation(_prog_color, "cam_light_pos_eye");
    _prog_color_uniforms["cam_light_strength"] = glGetUniformLocation(_prog_color, "cam_light_strength");
    _prog_color_uniforms["const_atten"] = glGetUniformLocation(_prog_color, "const_atten");
    _prog_color_uniforms["linear_atten"] = glGetUniformLocation(_prog_color, "linear_atten");
    _prog_color_uniforms["quad_atten"] = glGetUniformLocation(_prog_color, "quad_atten");
    _prog_color_uniforms["dir_light_color"] = glGetUniformLocation(_prog_color, "dir_light_color");
    _prog_color_uniforms["dir_light_dir"] = glGetUniformLocation(_prog_color, "dir_light_dir");
    _prog_color_uniforms["dir_light_strength"] = glGetUniformLocation(_prog_color, "dir_light_strength");
    _prog_color_uniforms["dir_half_vec"] = glGetUniformLocation(_prog_color, "dir_half_vec");
    _prog_color_uniforms["light_forward"] = glGetUniformLocation(_prog_color, "light_forward");
    check_error("_prog_color GetUniformLocation");

    _prog_line_uniforms["perspective"] = glGetUniformLocation(_prog_line, "perspective");
    _prog_line_uniforms["view_model"] = glGetUniformLocation(_prog_line, "view_model");
    _prog_line_uniforms["color"] = glGetUniformLocation(_prog_line, "color");
    check_error("_prog_line GetUniformLocation");

    // set up un-changing lighting values
    glm::vec3 light_pos_eye(0.0f);
    glm::vec3 light_forward(0.0f, 0.0f, 1.0f); // in eye space

    glUseProgram(_prog_tex);
    glUniform3fv(_prog_tex_uniforms["ambient_color"], 1, &_ambient_color[0]);
    glUniform3fv(_prog_tex_uniforms["cam_light_pos_eye"], 1, &light_pos_eye[0]);
    glUniform3fv(_prog_tex_uniforms["light_forward"], 1, &light_forward[0]);
    check_error("_prog_tex uniforms static");

    glUseProgram(_prog_color);
    glUniform3fv(_prog_color_uniforms["ambient_color"], 1, &_ambient_color[0]);
    glUniform3fv(_prog_color_uniforms["cam_light_pos_eye"], 1, &light_pos_eye[0]);
    glUniform3fv(_prog_color_uniforms["light_forward"], 1, &light_forward[0]);
    check_error("_prog_color uniforms static");

    // create static geometry objects - cursor, axes
    try
    {
        _cursor.build("img/cursor.png");
    }
    catch(Glib::Exception &e)
    {
        dynamic_cast<Gtk::Window *>(get_toplevel())->hide();
        return_code = EXIT_FAILURE;
        return;
    }

    _axes.build();
    _axes.color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    invalidate(); // redraw
}

// called when window is resized
void Graph_disp::resize(Gtk::Allocation & allocation)
{
    if(m_refGdkWindow)
    {
        // set the GL viewport dimensions and perspective matrix
        glViewport(0, 0, allocation.get_width(), allocation.get_height());
        _perspective = glm::perspective((float)M_PI / 6.0f,
            (float)allocation.get_width() / (float)allocation.get_height(),
            0.1f, 1000.0f);
        invalidate(); // redraw
    }
}

// main drawing code
bool Graph_disp::draw(const Cairo::RefPtr<Cairo::Context> & unused)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set up viewmodel matrices
    glm::mat4 view_model;
    if(use_orbit_cam)
    {
        view_model = glm::rotate(glm::rotate(glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, -_orbit_cam.r)),
            -_orbit_cam.phi, glm::vec3(1.0f, 0.0f, 0.0f)), -_orbit_cam.theta, glm::vec3(0.0f, 0.0f, 1.0f));
    }
    else
    {
        view_model = _cam.view_mat();
    }
    view_model = glm::scale(view_model, glm::vec3(_scale));

    glm::mat4 view_model_perspective = _perspective * view_model;
    glm::mat3 normal_transform = glm::transpose(glm::inverse(glm::mat3(view_model)));

    glm::vec3 light_forward(0.0f, 0.0f, 1.0f); // in eye space

    // directional light
    glm::vec3 dir_light_dir = normal_transform * glm::normalize(-dir_light.pos);
    glm::vec3 dir_half_vec = glm::normalize(light_forward + dir_light_dir);

    // draw axes
    if(draw_axes_flag)
    {
        glUseProgram(_prog_line);

        glUniformMatrix4fv(_prog_line_uniforms["perspective"], 1, GL_FALSE, &_perspective[0][0]);
        glUniformMatrix4fv(_prog_line_uniforms["view_model"], 1, GL_FALSE, &view_model[0][0]);
        glUniform4fv(_prog_line_uniforms["color"], 1, &_axes.color[0]);

        _axes.draw();

        check_error("draw axes");
    }

    // draw graphs
    for(auto &graph: _graphs)
    {
        // draw geometry
        if(graph->draw_flag)
        {
            if(graph->use_tex && graph->valid_tex)
            {
                // draw with texture
                glUseProgram(_prog_tex);

                glUniformMatrix4fv(_prog_tex_uniforms["view_model_perspective"], 1, GL_FALSE, &view_model_perspective[0][0]);
                glUniformMatrix4fv(_prog_tex_uniforms["view_model"], 1, GL_FALSE, &view_model[0][0]);
                glUniformMatrix3fv(_prog_tex_uniforms["normal_transform"], 1, GL_FALSE, &normal_transform[0][0]);

                // material properties
                glUniform1f(_prog_tex_uniforms["shininess"], graph->shininess);
                glUniform3fv(_prog_tex_uniforms["specular"], 1, &graph->specular[0]);
                glUniform3fv(_prog_tex_uniforms["dir_light_dir"], 1, &dir_light_dir[0]);
                glUniform3fv(_prog_tex_uniforms["dir_half_vec"], 1, &dir_half_vec[0]);

                // light properties
                glUniform3fv(_prog_tex_uniforms["cam_light_color"], 1, &cam_light.color[0]);
                glUniform1f(_prog_tex_uniforms["cam_light_strength"], cam_light.strength);
                glUniform1f(_prog_tex_uniforms["const_atten"], cam_light.const_atten);
                glUniform1f(_prog_tex_uniforms["linear_atten"], cam_light.linear_atten);
                glUniform1f(_prog_tex_uniforms["quad_atten"], cam_light.quad_atten);
                glUniform3fv(_prog_tex_uniforms["dir_light_color"], 1, &dir_light.color[0]);
                glUniform1f(_prog_tex_uniforms["dir_light_strength"], dir_light.strength);
            }
            else
            {
                // draw with one color
                glUseProgram(_prog_color);

                glUniformMatrix4fv(_prog_color_uniforms["view_model_perspective"], 1, GL_FALSE, &view_model_perspective[0][0]);
                glUniformMatrix4fv(_prog_color_uniforms["view_model"], 1, GL_FALSE, &view_model[0][0]);
                glUniformMatrix3fv(_prog_color_uniforms["normal_transform"], 1, GL_FALSE, &normal_transform[0][0]);

                // material properties
                glUniform4fv(_prog_color_uniforms["color"], 1, &graph->color[0]);
                glUniform1f(_prog_color_uniforms["shininess"], graph->shininess);
                glUniform3fv(_prog_color_uniforms["specular"], 1, &graph->specular[0]);
                glUniform3fv(_prog_color_uniforms["dir_light_dir"], 1, &dir_light_dir[0]);
                glUniform3fv(_prog_color_uniforms["dir_half_vec"], 1, &dir_half_vec[0]);

                // light properties
                glUniform3fv(_prog_color_uniforms["cam_light_color"], 1, &cam_light.color[0]);
                glUniform1f(_prog_color_uniforms["cam_light_strength"], cam_light.strength);
                glUniform1f(_prog_color_uniforms["const_atten"], cam_light.const_atten);
                glUniform1f(_prog_color_uniforms["linear_atten"], cam_light.linear_atten);
                glUniform1f(_prog_color_uniforms["quad_atten"], cam_light.quad_atten);
                glUniform3fv(_prog_color_uniforms["dir_light_color"], 1, &dir_light.color[0]);
                glUniform1f(_prog_color_uniforms["dir_light_strength"], dir_light.strength);
            }
            check_error("draw geometry");

            graph->draw();
        }

        // draw grid
        if(graph->draw_grid_flag)
        {
            // switch to line shader
            glUseProgram(_prog_line);
            glUniformMatrix4fv(_prog_line_uniforms["perspective"], 1, GL_FALSE, &_perspective[0][0]);
            glUniformMatrix4fv(_prog_line_uniforms["view_model"], 1, GL_FALSE, &view_model[0][0]);
            glUniform4fv(_prog_line_uniforms["color"], 1, &graph->grid_color[0]);

            graph->draw_grid();
            check_error("draw grid");
        }

        // draw normal vectors
        if(graph->draw_normals_flag)
        {
            // switch to line shader
            glUseProgram(_prog_line);
            glUniformMatrix4fv(_prog_line_uniforms["perspective"], 1, GL_FALSE, &_perspective[0][0]);
            glUniformMatrix4fv(_prog_line_uniforms["view_model"], 1, GL_FALSE, &view_model[0][0]);
            glUniform4fv(_prog_line_uniforms["color"], 1, &graph->normal_color[0]);

            graph->draw_normals();
            check_error("draw normals");
        }
    }

    // draw cursor
    if(draw_cursor_flag && _active_graph && _active_graph->cursor_defined())
    {
        glUseProgram(_prog_tex);

        view_model = glm::scale(glm::translate(view_model, _active_graph->cursor_pos()), glm::vec3(0.25f / _scale));
        view_model_perspective = _perspective * view_model;
        normal_transform = glm::transpose(glm::inverse(glm::mat3(view_model)));

        glUniformMatrix4fv(_prog_tex_uniforms["view_model_perspective"], 1, GL_FALSE, &view_model_perspective[0][0]);
        glUniformMatrix4fv(_prog_tex_uniforms["view_model"], 1, GL_FALSE, &view_model[0][0]);
        glUniformMatrix3fv(_prog_tex_uniforms["normal_transform"], 1, GL_FALSE, &normal_transform[0][0]);

        // material properties
        glUniform1f(_prog_tex_uniforms["shininess"], _cursor.shininess);
        glUniform3fv(_prog_tex_uniforms["specular"], 1, &_cursor.specular[0]);
        glUniform3fv(_prog_tex_uniforms["dir_light_dir"], 1, &dir_light_dir[0]);
        glUniform3fv(_prog_tex_uniforms["dir_half_vec"], 1, &dir_half_vec[0]);

        _cursor.draw();
        check_error("draw cursor");
    }

    display(); // swap display buffers
    return true;
}

// main input processing
bool Graph_disp::input()
{
    // state vars
    static std::unordered_map<sf::Keyboard::Key, bool, std::hash<int>> key_lock;
    static sf::Vector2i old_mouse_pos = sf::Mouse::getPosition(glWindow);
    static sf::Clock cursor_delay;
    static sf::Clock zoom_delay;

    // the neat thing about having this in a timeout func is that we
    // don't need to calculate dt for movement controls.
    // it is always (almost) exactly 10ms

    // only process when the window is active and display is focused
    if(dynamic_cast<Gtk::Window *>(get_toplevel())->is_active())
    {
        sf::Vector2i new_mouse_pos = sf::Mouse::getPosition(glWindow);

        if(!has_focus() && new_mouse_pos.x >= 0 && new_mouse_pos.y >= 0 &&
            new_mouse_pos.x < (int)glWindow.getSize().x && new_mouse_pos.y < (int)glWindow.getSize().y)
        {
            grab_focus();
        }

        if(has_focus())
        {
            // Camera controls

            float mov_scale = 0.1f;

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
            {
                mov_scale *= 2.0f;
            }

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt))
            {
                mov_scale *= 0.1f;
            }

            // orbiting rotational cam
            if(use_orbit_cam)
            {
                // reset
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::R) && !key_lock[sf::Keyboard::R])
                {
                    key_lock[sf::Keyboard::R] = true;
                    reset_cam();
                }
                else if(!sf::Keyboard::isKeyPressed(sf::Keyboard::R))
                    key_lock[sf::Keyboard::R] = false;

                // tilt up
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                {
                    _orbit_cam.phi += (float)M_PI / 90.0f * mov_scale;
                    invalidate();
                }

                // tilt down
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                {
                    _orbit_cam.phi -= (float)M_PI / 90.0f * mov_scale;
                    invalidate();
                }

                // rotate clockwise
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
                {
                    _orbit_cam.theta += (float)M_PI / 90.0f * mov_scale;
                    invalidate();
                }

                // rotate counter-clockwise
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
                {
                    _orbit_cam.theta -= (float)M_PI / 90.0f * mov_scale;
                    invalidate();
                }

                // move camera in
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
                {
                    _orbit_cam.r -= mov_scale;
                    invalidate();
                }

                // move camera out
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::E))
                {
                    _orbit_cam.r += mov_scale;
                    invalidate();
                }

                // rotate w/ mouse click & drag
                if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
                {
                    int d_x = new_mouse_pos.x - old_mouse_pos.x;
                    int d_y = new_mouse_pos.y - old_mouse_pos.y;

                    _orbit_cam.theta -= 0.005f * d_x;
                    _orbit_cam.phi -= 0.005f * d_y;

                    invalidate();
                }

                // wrap theta
                if(_orbit_cam.theta > (float)M_PI * 2.0f)
                    _orbit_cam.theta -= (float)M_PI * 2.0f;
                if(_orbit_cam.theta < 0.0f)
                    _orbit_cam.theta += (float)M_PI * 2.0f;

                // clamp phi
                if(_orbit_cam.phi > (float)M_PI)
                    _orbit_cam.phi = (float)M_PI;
                if(_orbit_cam.phi < 0.0f)
                    _orbit_cam.phi = 0.0f;
            }
            else // free camera
            {
                // reset
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::R) && !key_lock[sf::Keyboard::R])
                {
                    key_lock[sf::Keyboard::R] = true;
                    reset_cam();
                }
                else if(!sf::Keyboard::isKeyPressed(sf::Keyboard::R))
                    key_lock[sf::Keyboard::R] = false;

                // move forward
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                {
                    _cam.translate(mov_scale * _cam.forward());
                    invalidate();
                }

                // move backwards
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
                {
                    _cam.translate(-mov_scale * _cam.forward());
                    invalidate();
                }

                // move left
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
                {
                    _cam.translate(-mov_scale * _cam.right());
                    invalidate();
                }

                // move right
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
                {
                    _cam.translate(mov_scale * _cam.right());
                    invalidate();
                }

                // move up
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
                {
                    _cam.translate(mov_scale * glm::vec3(0.0f, 0.0f, 1.0f));
                    invalidate();
                }

                // move down
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::E))
                {
                    _cam.translate(-mov_scale * glm::vec3(0.0f, 0.0f, 1.0f));
                    invalidate();
                }

                // rotate view with mouse click & drag
                if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
                {
                    int d_x = new_mouse_pos.x - old_mouse_pos.x;
                    int d_y = new_mouse_pos.y - old_mouse_pos.y;

                    _cam.rotate(0.001f * d_y, _cam.right());
                    _cam.rotate(0.001f * d_x, glm::vec3(0.0f, 0.0f, 1.0f));

                    invalidate();
                }
            }

            const int zoom_timeout = 200;

            // zoom in
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Z) && zoom_delay.getElapsedTime().asMilliseconds() >= zoom_timeout)
            {
                _scale *= 2.0f;
                zoom_delay.restart();
                invalidate();
            }

            // zoom out
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::X) && zoom_delay.getElapsedTime().asMilliseconds() >= zoom_timeout)
            {
                _scale *= 0.5f;
                zoom_delay.restart();
                invalidate();
            }

            // move cursor with arrow keys
            if(draw_cursor_flag && _active_graph)
            {
                const int cursor_timeout = 200;
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && cursor_delay.getElapsedTime().asMilliseconds() >= cursor_timeout)
                {
                    _active_graph->move_cursor(Graph::UP);
                    cursor_delay.restart();
                    invalidate();
                }

                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down) && cursor_delay.getElapsedTime().asMilliseconds() >= cursor_timeout)
                {
                    _active_graph->move_cursor(Graph::DOWN);
                    cursor_delay.restart();
                    invalidate();
                }

                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && cursor_delay.getElapsedTime().asMilliseconds() >= cursor_timeout)
                {
                    _active_graph->move_cursor(Graph::LEFT);
                    cursor_delay.restart();
                    invalidate();
                }

                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && cursor_delay.getElapsedTime().asMilliseconds() >= cursor_timeout)
                {
                    _active_graph->move_cursor(Graph::RIGHT);
                    cursor_delay.restart();
                    invalidate();
                }
            }
        }
        old_mouse_pos = new_mouse_pos;
    }
    return true;
}

// GTK key press handler
bool Graph_disp::key_press(GdkEventKey * e)
{
    // returning true means we don't propagate key presses up - keep them in this widget
    // (and disregard them entirely because we're using SFML for handling the keyboard, not GTK)
    return true;
}
