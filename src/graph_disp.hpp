// graph_disp.hpp
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

#ifndef __GRAPH_DISP_H__
#define __GRAPH_DISP_H__

#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <GL/glew.h>

#include <glibmm/main.h>

#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>

#include <glm/glm.hpp>

#include "SFMLWidget/SFMLWidget.hpp"

#include "camera.hpp"
#include "graph.hpp"

struct Light
{
    glm::vec3 pos;
    glm::vec3 color;
    float strength;
    float const_attenuation;
    float linear_attenuation;
    float quad_attenuation;
};

struct Orbit_cam
{
    float r;
    float theta;
    float phi;
};

class Cursor
{
public:
    Cursor();
    ~Cursor();

    void draw() const;
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

class Axes
{
public:
    Axes();
    ~Axes();

    void draw() const;
    void build();

    glm::vec4 color;
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

class Graph_disp final: public SFMLWidget
{
public:
    Graph_disp(const sf::VideoMode & mode, const int size_request = - 1);

    // key press handler
    bool key_press(GdkEventKey * e);

    // openGL initialization should go here
    // set and get the active graph (the one w/ the cursor on it)
    void set_active_graph(Graph * graph);
    Graph * get_active_graph() const;

    // give and take graphs from the display
    void add_graph(const Graph * graph);
    void remove_graph(const Graph * graph);

    // reset camera to starting position / orientation
    void reset_cam();

    // display settings
    bool draw_cursor_flag;
    bool draw_axes_flag;
    bool use_orbit_cam;

private:
    void realize();
    void resize(Gtk::Allocation & allocation);
    bool draw(const Cairo::RefPtr<Cairo::Context> & cr);
    bool input();

    GLuint _prog_tex;
    GLuint _prog_color;
    GLuint _prog_line;
    std::unordered_map<std::string, GLuint> _prog_tex_uniforms;
    std::unordered_map<std::string, GLuint> _prog_color_uniforms;
    std::unordered_map<std::string, GLuint> _prog_line_uniforms;

    Cursor _cursor;
    Axes _axes;

    Camera _cam;
    Orbit_cam _orbit_cam;
    float _scale;

    glm::mat4 _perspective_mat;
    Light _light;
    Light _dir_light;
    glm::vec3 _ambient_light;

    // non-owned storage for graphs
    Graph * _active_graph;
    std::set<const Graph *> _graphs;

    // make non-copyable
    Graph_disp(const Graph_disp &) = delete;
    Graph_disp(const Graph_disp &&) = delete;
    Graph_disp & operator =(const Graph_disp &) = delete;
    Graph_disp & operator =(const Graph_disp &&) = delete;
};

#endif // __GRAPH_DISP_H__
