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

#include "config.hpp"
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
    bkg_color(0.25f, 0.25f, 0.25f), ambient_color(0.4f, 0.4f, 0.4f),
    _prog_tex(0), _prog_color(0), _prog_line(0),
    _cam(glm::vec3(0.0f, -10.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
    _orbit_cam({10.0f, 0.0f, (float)M_PI / 2.0f}), _scale(1.0f), _perspective(1.0f),
    _active_graph(nullptr)

{
    // All OpenGL initialization has to wait until the drawing context actually exists
    // we do this in the initialize method
    // connect it to the draw signal, to be run only on the first drawing request
    _draw_connection = signal_draw().connect(sigc::mem_fun(*this, &Graph_disp::initiaize));

    // connect event signals
    signal_size_allocate().connect(sigc::mem_fun(*this, &Graph_disp::resize));
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
bool Graph_disp::initiaize(const Cairo::RefPtr<Cairo::Context> & unused)
{
    // init glew
    if(glewInit() != GLEW_OK)
    {
        std::cerr<<"Error loading glew. Aborting"<<std::endl;
        dynamic_cast<Gtk::Window *>(get_toplevel())->hide();
        return_code = EXIT_FAILURE;
        return true;
    }

    // check for required OpenGL version
    if(!GLEW_VERSION_3_0)
    {
        std::cerr<<"OpenGL version too low. Version 3.0 required"<<std::endl;
        std::cerr<<"Installed version is: "<<glGetString(GL_VERSION)<<std::endl;
        dynamic_cast<Gtk::Window *>(get_toplevel())->hide();
        return_code = EXIT_FAILURE;
        return true;
    }

    // init GL state vars
    glClearColor(bkg_color.r, bkg_color.g, bkg_color.b, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthRangef(0.0f, 1.0f);
    glLineWidth(5.0f);

    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_DONT_CARE);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendColor(1.0f, 1.0f, 1.0f, 0.1f);
    glEnable(GL_BLEND);

    // build shader programs
    GLuint graph_vert = compile_shader(check_in_pwd("shaders/graph.vert"), GL_VERTEX_SHADER);
    GLuint line_vert = compile_shader(check_in_pwd("shaders/line.vert"), GL_VERTEX_SHADER);
    GLuint common_frag = compile_shader(check_in_pwd("shaders/common.frag"), GL_FRAGMENT_SHADER);
    GLuint tex_frag = compile_shader(check_in_pwd("shaders/tex.frag"), GL_FRAGMENT_SHADER);
    GLuint color_frag = compile_shader(check_in_pwd("shaders/color.frag"), GL_FRAGMENT_SHADER);
    GLuint flat_color_frag = compile_shader(check_in_pwd("shaders/flat_color.frag"), GL_FRAGMENT_SHADER);

    if(graph_vert == 0 || line_vert == 0 || tex_frag == 0 || color_frag == 0 || flat_color_frag == 0)
    {
        // error messages are displayed by the compile_shader function
        dynamic_cast<Gtk::Window *>(get_toplevel())->hide();
        return_code = EXIT_FAILURE;
        return true;
    }

    // link shaders
    _prog_tex = link_shader_prog(std::vector<GLuint> {graph_vert, tex_frag, common_frag},
        {std::make_pair(0, "vert_pos"), std::make_pair(1, "vert_tex_coords"), std::make_pair(2, "vert_normal")});
    _prog_color = link_shader_prog(std::vector<GLuint> {graph_vert, color_frag, common_frag},
        {std::make_pair(0, "vert_pos"), std::make_pair(1, "vert_tex_coords"), std::make_pair(2, "vert_normal")});
    _prog_line = link_shader_prog(std::vector<GLuint> {line_vert, flat_color_frag},
        {std::make_pair(0, "vert_pos")});

    if(_prog_tex == 0 || _prog_color == 0 || _prog_line == 0)
    {
        // error messages are displayed by the link_shader_prog function
        dynamic_cast<Gtk::Window *>(get_toplevel())->hide();
        return_code = EXIT_FAILURE;
        return true;
    }

    check_error("build shaders");

    // free shader objects
    glDeleteShader(graph_vert);
    glDeleteShader(line_vert);
    glDeleteShader(common_frag);
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
    glUniform3fv(_prog_tex_uniforms["cam_light_pos_eye"], 1, &light_pos_eye[0]);
    glUniform3fv(_prog_tex_uniforms["light_forward"], 1, &light_forward[0]);
    check_error("_prog_tex uniforms static");

    glUseProgram(_prog_color);
    glUniform3fv(_prog_color_uniforms["cam_light_pos_eye"], 1, &light_pos_eye[0]);
    glUniform3fv(_prog_color_uniforms["light_forward"], 1, &light_forward[0]);
    check_error("_prog_color uniforms static");

    // create static geometry objects - cursor, axes
    try
    {
        _cursor.build(check_in_pwd("img/cursor.png"));
    }
    catch(Glib::Exception &e)
    {
        dynamic_cast<Gtk::Window *>(get_toplevel())->hide();
        return_code = EXIT_FAILURE;
        return true;
    }

    _axes.build();
    _axes.color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    invalidate(); // redraw

    _draw_connection.disconnect();
    _draw_connection = signal_draw().connect(sigc::mem_fun(*this, &Graph_disp::draw));

    return draw(unused);
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
