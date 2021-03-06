// graph_disp.hpp
// Graphics display module

// Copyright 2018 Matthew Chandler

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

#ifndef GRAPH_DISP_H
#define GRAPH_DISP_H

#include <memory>
#include <set>
#include <string>
#include <vector>

#include <GL/glew.h>

#include <glibmm/main.h>

#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "SFMLWidget/SFMLWidget.hpp"

#include "camera.hpp"
#include "gl_helpers.hpp"
#include "graph.hpp"

struct Light
{
    glm::vec3 color;
    float strength;
};

struct Point_light: public Light
{
    glm::vec3 pos;
    // attenuation properties
    float const_atten;
    float linear_atten;
    float quad_atten;
    Point_light(const glm::vec3 & color, const float strength, const glm::vec3 & pos,
    const float const_atten, const float linear_atten, const float quad_atten);
};

struct Dir_light: public Light
{
    glm::vec3 dir;
    Dir_light(const glm::vec3 & color, const float strength, const glm::vec3 & dir);
};

// 'orbital' camera
// always points to center but can rotate graph around
struct Orbit_cam
{
    float r;
    float theta;
    float phi;
};

// octahedral cursor object
class Cursor
{
public:
    Cursor();
    ~Cursor();

    void draw() const;
    // build and set text
    void build(const std::string & filename);

    // material properties
    float shininess;
    glm::vec3 specular;

private:
    // OpenGL stuff
    GLuint _tex;
    GLuint _vao;
    GLuint _vbo;
    GLuint _num_indexes;

    // make non-copyable
    Cursor(const Cursor &) = delete;
    Cursor(const Cursor &&) = delete;
    Cursor & operator=(const Cursor &) = delete;
    Cursor & operator=(const Cursor &&) = delete;
};

// X, Y, and Z axes at origin
class Axes
{
public:
    Axes();
    ~Axes();

    void draw() const;
    void build();

    glm::vec3 color;
private:
    // OpenGL stuff
    GLuint _vao;
    GLuint _vbo;
    GLuint _num_indexes;

    // make non-copyable
    Axes(const Axes &) = delete;
    Axes(const Axes &&) = delete;
    Axes & operator=(const Axes &) = delete;
    Axes & operator=(const Axes &&) = delete;
};

// main OpenGL display class
// all graphics code is done here or in sub-classes
// is a hybrid of a GTK widget and SFML window
class Graph_disp final: public SFMLWidget
{
public:
    Graph_disp(const sf::VideoMode & mode, const int size_request = - 1, const sf::ContextSettings & context_settings = sf::ContextSettings());
    ~Graph_disp();

    // openGL initialization should go here
    // set and get the active graph (the one w/ the cursor on it)
    void set_active_graph(Graph * graph);

    // give and take graphs from the display
    void add_graph(const Graph * graph);
    void remove_graph(const Graph * graph);

    // reset camera to starting position / orientation
    void reset_cam();

    // emmitted at the end of initialize method: all setup complete
    sigc::signal<void> signal_initialized() const;

    // display settings
    bool draw_cursor_flag;
    bool draw_axes_flag;
    bool use_orbit_cam;

    // lighting vars
    Point_light cam_light;
    Dir_light dir_light;

    glm::vec3 bkg_color;
    glm::vec3 ambient_color;

private:
    // called when OpenGL context is ready and GTK widget is ready
    bool initiaize(const Cairo::RefPtr<Cairo::Context> & unused);
    // called when window is resized
    void resize(Gtk::Allocation & allocation);
    // drawing setup code
    void graph_draw_setup(std::unordered_map<std::string, GLint> & uniforms,
        const Graph & graph);
    // main drawing code
    bool draw(const Cairo::RefPtr<Cairo::Context> & unused);
    // main input processing
    bool input();
    // GTK key press handler
    bool key_press(GdkEventKey * e);

    // shader vars
    Shader_prog _prog_tex;
    Shader_prog _prog_color;
    Shader_prog _prog_line;

    // static geometry
    Cursor _cursor;
    Axes _axes;

    // cameras
    Camera _cam;
    Orbit_cam _orbit_cam;
    float _scale;
    glm::mat4 _perspective;

    // storage for graphs (we do not own them here)
    Graph * _active_graph;
    std::set<const Graph *> _graphs;

    // used for initializing, and then drawing
    sigc::connection _draw_connection;

    sigc::signal<void> _signal_initialized;

    // make non-copyable
    Graph_disp(const Graph_disp &) = delete;
    Graph_disp(const Graph_disp &&) = delete;
    Graph_disp & operator =(const Graph_disp &) = delete;
    Graph_disp & operator =(const Graph_disp &&) = delete;
};

#endif // GRAPH_DISP_H
