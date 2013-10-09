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

#include <glm/gtc/matrix_transform.hpp>

#include "gl_helpers.hpp"
#include "graph_disp.hpp"

Cursor::Cursor(): shininess(90.0f), specular(1.0f),
    _tex(0), _vao(0), _vbo(0), _num_indexes(0)
{
}

Cursor::~Cursor()
{
    if(_tex)
        glDeleteTextures(1, &_tex);
    if(_vao)
        glDeleteVertexArrays(1, &_vao);
    if(_vbo)
        glDeleteBuffers(1, &_vbo);
}

void Cursor::draw() const
{
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBindTexture(GL_TEXTURE_2D, _tex);

    glDrawArrays(GL_TRIANGLES, 0, _num_indexes);
}

void Cursor::build(const std::string & tex_file_name)
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
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * coords.size(), coords.data());
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * coords.size(), sizeof(glm::vec2) * tex_coords.size(), tex_coords.data());
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * coords.size() + sizeof(glm::vec2) * tex_coords.size(),
        sizeof(glm::vec3) * normals.size(), normals.data());

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
        exit(EXIT_FAILURE);
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
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    glDrawArrays(GL_LINES, 0, _num_indexes);
}

void Axes::build()
{
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

    // OpenGL structs
    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * coords.size(), coords.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    _num_indexes = coords.size();
}

Graph_disp::Graph_disp(const sf::VideoMode & mode, const int size_reqest, const sf::ContextSettings & context_settings):
    SFMLWidget(mode, size_reqest),
    draw_cursor_flag(true), draw_axes_flag(true),
    _cam(glm::vec3(0.0f, -10.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
    _perspective_mat(1.0f),
    _light({glm::vec3(0.0f), glm::vec3(1.0f, 1.0f, 1.0f), 0.8f, 1.0f, 0.5f, 0.0f}),
    _ambient_light(0.4f, 0.4f, 0.4f),
    _active_graph(nullptr)

{
    // All OpenGL initialization has to wait until the drawing context actually exists
    // move it to the realize method
    signal_realize().connect(sigc::mem_fun(*this, &Graph_disp::realize));
    signal_size_allocate().connect(sigc::mem_fun(*this, &Graph_disp::resize));
    signal_draw().connect(sigc::mem_fun(*this, &Graph_disp::draw));
    signal_key_press_event().connect(sigc::mem_fun(*this, &Graph_disp::key_press));
    Glib::signal_timeout().connect(sigc::mem_fun(*this, &Graph_disp::input), 10);

    set_can_focus();
    set_can_default();
}

// key press handler
bool Graph_disp::key_press(GdkEventKey * e)
{
    // returning true means we don't propagate key presses up - keep them in this widget
    // (and disregard them entirely because we're using SFML for handling the keyboard, not GTK)
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

    _prog_tex = link_shader_prog(std::vector<GLuint> {graph_vert, tex_frag});
    _prog_color = link_shader_prog(std::vector<GLuint> {graph_vert, color_frag});
    _prog_line = link_shader_prog(std::vector<GLuint> {line_vert, flat_color_frag});

    if(_prog_tex == 0 || _prog_color == 0 || _prog_line == 0)
        exit(EXIT_FAILURE);

    glDeleteShader(graph_vert);
    glDeleteShader(line_vert);
    glDeleteShader(tex_frag);
    glDeleteShader(color_frag);
    glDeleteShader(flat_color_frag);

    // get uniform locations
    _prog_tex_uniforms["view_model_perspective"] = glGetUniformLocation(_prog_tex, "view_model_perspective");
    _prog_tex_uniforms["view_model"] = glGetUniformLocation(_prog_tex, "view_model");
    _prog_tex_uniforms["normal_transform"] = glGetUniformLocation(_prog_tex, "normal_transform");
    _prog_tex_uniforms["shininess"] = glGetUniformLocation(_prog_tex, "shininess");
    _prog_tex_uniforms["specular"] = glGetUniformLocation(_prog_tex, "specular");
    _prog_tex_uniforms["ambient_color"] = glGetUniformLocation(_prog_tex, "ambient_color");
    _prog_tex_uniforms["light_color"] = glGetUniformLocation(_prog_tex, "light_color");
    _prog_tex_uniforms["light_pos"] = glGetUniformLocation(_prog_tex, "light_pos");
    _prog_tex_uniforms["light_strength"] = glGetUniformLocation(_prog_tex, "light_strength");
    _prog_tex_uniforms["const_atten"] = glGetUniformLocation(_prog_tex, "const_atten");
    _prog_tex_uniforms["linear_atten"] = glGetUniformLocation(_prog_tex, "linear_atten");
    _prog_tex_uniforms["quad_atten"] = glGetUniformLocation(_prog_tex, "quad_atten");
    _prog_tex_uniforms["cam_forward"] = glGetUniformLocation(_prog_tex, "cam_forward");

    _prog_color_uniforms["view_model_perspective"] = glGetUniformLocation(_prog_color, "view_model_perspective");
    _prog_color_uniforms["view_model"] = glGetUniformLocation(_prog_color, "view_model");
    _prog_color_uniforms["normal_transform"] = glGetUniformLocation(_prog_color, "normal_transform");
    _prog_color_uniforms["color"] = glGetUniformLocation(_prog_color, "color");
    _prog_color_uniforms["shininess"] = glGetUniformLocation(_prog_color, "shininess");
    _prog_color_uniforms["specular"] = glGetUniformLocation(_prog_color, "specular");
    _prog_color_uniforms["ambient_color"] = glGetUniformLocation(_prog_color, "ambient_color");
    _prog_color_uniforms["light_color"] = glGetUniformLocation(_prog_color, "light_color");
    _prog_color_uniforms["light_pos"] = glGetUniformLocation(_prog_color, "light_pos");
    _prog_color_uniforms["light_strength"] = glGetUniformLocation(_prog_color, "light_strength");
    _prog_color_uniforms["const_atten"] = glGetUniformLocation(_prog_color, "const_atten");
    _prog_color_uniforms["linear_atten"] = glGetUniformLocation(_prog_color, "linear_atten");
    _prog_color_uniforms["quad_atten"] = glGetUniformLocation(_prog_color, "quad_atten");
    _prog_color_uniforms["cam_forward"] = glGetUniformLocation(_prog_color, "cam_forward");

    _prog_line_uniforms["perspective"] = glGetUniformLocation(_prog_line, "perspective");
    _prog_line_uniforms["view_model"] = glGetUniformLocation(_prog_line, "view_model");
    _prog_line_uniforms["color"] = glGetUniformLocation(_prog_line, "color");

    // set up un-changing lighting values
    glm::vec3 light_pos_eye(0.0f);
    glm::vec3 light_forward(0.0f, 0.0f, 1.0f); // in eye space

    glUseProgram(_prog_tex);
    glUniform3fv(_prog_tex_uniforms["ambient_color"], 1, &_ambient_light[0]);
    glUniform3fv(_prog_tex_uniforms["light_color"], 1, &_light.color[0]);
    glUniform3fv(_prog_tex_uniforms["light_pos"], 1, &light_pos_eye[0]);
    glUniform1f(_prog_tex_uniforms["light_strength"], _light.strength);
    glUniform1f(_prog_tex_uniforms["const_atten"], _light.const_attenuation);
    glUniform1f(_prog_tex_uniforms["linear_atten"], _light.linear_attenuation);
    glUniform1f(_prog_tex_uniforms["quad_atten"], _light.quad_attenuation);
    glUniform3fv(_prog_tex_uniforms["cam_forward"], 1, &light_forward[0]);

    glUseProgram(_prog_color);
    glUniform3fv(_prog_color_uniforms["ambient_color"], 1, &_ambient_light[0]);
    glUniform3fv(_prog_color_uniforms["light_color"], 1, &_light.color[0]);
    glUniform3fv(_prog_color_uniforms["light_pos"], 1, &light_pos_eye[0]);
    glUniform1f(_prog_color_uniforms["light_strength"], _light.strength);
    glUniform1f(_prog_color_uniforms["const_atten"], _light.const_attenuation);
    glUniform1f(_prog_color_uniforms["linear_atten"], _light.linear_attenuation);
    glUniform1f(_prog_color_uniforms["quad_atten"], _light.quad_attenuation);
    glUniform3fv(_prog_color_uniforms["cam_forward"], 1, &light_forward[0]);

    _cursor.build("img/cursor.png");

    _axes.build();
    _axes.color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    invalidate();
}

void Graph_disp::resize(Gtk::Allocation & allocation)
{
    if(m_refGdkWindow)
    {
        glViewport(0, 0, glWindow.getSize().x, glWindow.getSize().y);
        _perspective_mat = glm::perspective(30.0f, (float)glWindow.getSize().x / (float)glWindow.getSize().y,
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
    glm::mat4 view_model = _cam.view_mat();
    glm::mat4 view_model_perspective = _perspective_mat * view_model;
    glm::mat3 normal_transform = glm::transpose(glm::inverse(glm::mat3(view_model)));

    // draw axes
    if(draw_axes_flag)
    {
        glUseProgram(_prog_line);

        glUniformMatrix4fv(_prog_line_uniforms["perspective"], 1, GL_FALSE, &_perspective_mat[0][0]);
        glUniformMatrix4fv(_prog_line_uniforms["view_model"], 1, GL_FALSE, &view_model[0][0]);
        glUniform4fv(_prog_line_uniforms["color"], 1, &_axes.color[0]);

        _axes.draw();

        check_error("axes draw"); // TODO: clean these up
    }

    for(auto &graph: _graphs)
    {
        if(graph->draw_flag)
        {
            if(graph->use_tex && graph->valid_tex)
            {
                glUseProgram(_prog_tex);

                glUniformMatrix4fv(_prog_tex_uniforms["view_model_perspective"], 1, GL_FALSE, &view_model_perspective[0][0]);
                glUniformMatrix4fv(_prog_tex_uniforms["view_model"], 1, GL_FALSE, &view_model[0][0]);
                glUniformMatrix3fv(_prog_tex_uniforms["normal_transform"], 1, GL_FALSE, &normal_transform[0][0]);

                // material properties
                glUniform1f(_prog_tex_uniforms["shininess"], graph->shininess);
                glUniform3fv(_prog_tex_uniforms["specular"], 1, &graph->specular[0]);
            }
            else
            {
                glUseProgram(_prog_color);

                glUniformMatrix4fv(_prog_color_uniforms["view_model_perspective"], 1, GL_FALSE, &view_model_perspective[0][0]);
                glUniformMatrix4fv(_prog_color_uniforms["view_model"], 1, GL_FALSE, &view_model[0][0]);
                glUniformMatrix3fv(_prog_color_uniforms["normal_transform"], 1, GL_FALSE, &normal_transform[0][0]);

                // material properties
                glUniform4fv(_prog_color_uniforms["color"], 1, &graph->color[0]);
                glUniform1f(_prog_color_uniforms["shininess"], graph->shininess);
                glUniform3fv(_prog_color_uniforms["specular"], 1, &graph->specular[0]);
            }
            check_error("pre draw");

            graph->draw();
        }

        if(graph->draw_grid_flag)
        {
            // switch to line shader
            glUseProgram(_prog_line);
            glUniformMatrix4fv(_prog_line_uniforms["perspective"], 1, GL_FALSE, &_perspective_mat[0][0]);
            glUniformMatrix4fv(_prog_line_uniforms["view_model"], 1, GL_FALSE, &view_model[0][0]);
            glUniform4fv(_prog_line_uniforms["color"], 1, &graph->grid_color[0]);

            graph->draw_grid();
        }

        if(graph->draw_normals_flag)
        {
            // switch to line shader
            glUseProgram(_prog_line);
            glUniformMatrix4fv(_prog_line_uniforms["perspective"], 1, GL_FALSE, &_perspective_mat[0][0]);
            glUniformMatrix4fv(_prog_line_uniforms["view_model"], 1, GL_FALSE, &view_model[0][0]);
            glUniform4fv(_prog_line_uniforms["color"], 1, &graph->normal_color[0]);

            graph->draw_normals();
        }

        check_error("draw");
    }

    // draw cursor
    if(draw_cursor_flag && _active_graph && _active_graph->cursor_defined())
    {
        glUseProgram(_prog_tex);

        view_model = glm::translate(_cam.view_mat(), _active_graph->cursor_pos()) * glm::scale(glm::mat4(), glm::vec3(0.25f));
        view_model_perspective = _perspective_mat * view_model;
        normal_transform = glm::transpose(glm::inverse(glm::mat3(view_model)));

        glUniformMatrix4fv(_prog_tex_uniforms["view_model_perspective"], 1, GL_FALSE, &view_model_perspective[0][0]);
        glUniformMatrix4fv(_prog_tex_uniforms["view_model"], 1, GL_FALSE, &view_model[0][0]);
        glUniformMatrix3fv(_prog_tex_uniforms["normal_transform"], 1, GL_FALSE, &normal_transform[0][0]);

        // material properties
        glUniform1f(_prog_tex_uniforms["shininess"], _cursor.shininess);
        glUniform3fv(_prog_tex_uniforms["specular"], 1, &_cursor.specular[0]);

        _cursor.draw();
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
                _cam.set();
                invalidate();
            }
            else if(!sf::Keyboard::isKeyPressed(sf::Keyboard::R))
                key_lock[sf::Keyboard::R] = false;

            float scale = 0.1f;

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
            {
                scale *= 2.0f;
            }

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt))
            {
                scale *= 0.1f;
            }

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            {
                _cam.translate(scale * glm::normalize(glm::vec3(_cam.forward().x, _cam.forward().y, 0.0f)));
                invalidate();
            }

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            {
                _cam.translate(-scale * glm::normalize(glm::vec3(_cam.forward().x, _cam.forward().y, 0.0f)));
                invalidate();
            }

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            {
                _cam.translate(-scale * _cam.right());
                invalidate();
            }

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            {
                _cam.translate(scale * _cam.right());
                invalidate();
            }

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
            {
                _cam.translate(scale * glm::vec3(0.0f, 0.0f, 1.0f));
                invalidate();
            }

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::E))
            {
                _cam.translate(-scale * glm::vec3(0.0f, 0.0f, 1.0f));
                invalidate();
            }

            if(sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                int d_x = new_mouse_pos.x - old_mouse_pos.x;
                int d_y = new_mouse_pos.y - old_mouse_pos.y;

                _cam.rotate(0.001f * d_y, _cam.right());
                _cam.rotate(0.001f * d_x, glm::vec3(0.0f, 0.0f, 1.0f));

                invalidate();
            }

            if(draw_cursor_flag && _active_graph)
            {
                // Cursor controls
                int cursor_timeout = 200;
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

// set and get the active graph (the one w/ the cursor on it)
void Graph_disp::set_active_graph(Graph * graph)
{
    _active_graph = graph;
}

Graph * Graph_disp::get_active_graph() const
{
    return _active_graph;
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
