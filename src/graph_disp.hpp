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

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "SFMLWidget/SFMLWidget.hpp"

#include "camera.hpp"
#include "graph.hpp"

struct Light
{
    glm::vec3 pos;
    glm::vec3 color;
    float strength;
    // attenuation properties
    float const_atten;
    float linear_atten;
    float quad_atten;
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

// main OpenGL display class
// all graphics code is done here or in sub-classes
// is a hybrid of a GTK widget and SFML window
class Graph_disp final: public SFMLWidget
{
public:
    Graph_disp(const sf::VideoMode & mode, const int size_request = - 1);
    ~Graph_disp();

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
    // called when OpenGL context is ready and GTK widget is ready
    void realize();
    // called when window is resized
    void resize(Gtk::Allocation & allocation);
    // main drawing code
    bool draw(const Cairo::RefPtr<Cairo::Context> & cr);
    // main input processing
    bool input();
    // GTK key press handler
    bool key_press(GdkEventKey * e);

    // shader vars
    GLuint _prog_tex;
    GLuint _prog_color;
    GLuint _prog_line;
    std::unordered_map<std::string, GLuint> _prog_tex_uniforms;
    std::unordered_map<std::string, GLuint> _prog_color_uniforms;
    std::unordered_map<std::string, GLuint> _prog_line_uniforms;

    // static geometry
    Cursor _cursor;
    Axes _axes;

    // cameras
    Camera _cam;
    Orbit_cam _orbit_cam;
    float _scale;

    // lighting vars
    glm::mat4 _perspective;
    Light _light;
    Light _dir_light;
    glm::vec3 _ambient_color;

    // storage for graphs (we do not own them here)
    Graph * _active_graph;
    std::set<const Graph *> _graphs;

    // make non-copyable
    Graph_disp(const Graph_disp &) = delete;
    Graph_disp(const Graph_disp &&) = delete;
    Graph_disp & operator =(const Graph_disp &) = delete;
    Graph_disp & operator =(const Graph_disp &&) = delete;
};

#endif // __GRAPH_DISP_H__
