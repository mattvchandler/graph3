// graph_disp.h
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
#include <string>
#include <unordered_map>
#include <vector>

#include <GL/glew.h>

#include <glibmm/main.h>

#include <SFML/OpenGL.hpp>
#include <SFML/Window.hpp>

#include <glm/glm.hpp>

#include "SFMLWidget/SFMLWidget.h"

#include "camera.h"
#include "graph.h"
#include "graph_cartesian.h"
#include "graph_cylindrical.h"

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
    Cursor();
    ~Cursor();

    void draw() const;
    void build();

    // material properties
    GLuint tex;
    float shininess;
    glm::vec3 specular;

private:
    // OpenGL stuff
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
    Graph_disp(const sf::VideoMode & mode, const int size_reqest = - 1, const sf::ContextSettings & context_settings = sf::ContextSettings());
    ~Graph_disp();

    // key press handler
    bool key_press(GdkEventKey * e);

    // openGL initialization should go here
    void realize();
    void resize(Gtk::Allocation & allocation);
    bool draw(const Cairo::RefPtr<Cairo::Context> & cr);

    bool input();

    std::vector<std::unique_ptr<Graph>> graphs;

    size_t active_graph;
    std::vector<GLuint> textures;

private:
    GLuint prog_tex;
    GLuint prog_color;
    GLuint prog_line;
    std::unordered_map<std::string, GLuint> prog_tex_uniforms;
    std::unordered_map<std::string, GLuint> prog_color_uniforms;
    std::unordered_map<std::string, GLuint> prog_line_uniforms;

    Cursor cursor;
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

#endif // __GRAPH_DISP_H__
