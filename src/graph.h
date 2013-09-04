// graph.h
//

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

#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <iostream>
#include <string>
#include <vector>

#include <cmath>

#include <GL/glew.h>

#include <SFML/OpenGL.hpp>

#include <glm/glm.hpp>

#include <muParser.h>

#ifndef M_PI
#define M_PI 3.141592654
#endif

#ifndef M_E
#define M_E 2.718281828
#endif

// TODO: remove debug funcs

std::ostream & operator<<(std::ostream & out, const glm::vec2 & v);
std::ostream & operator<<(std::ostream & out, const glm::vec3 & v);

class Graph
{
public:
    Graph(const std::string & eqn = "");
    ~Graph();

    void draw() const;
    void draw_grid() const;

    virtual double eval(const double, const double) = 0;
    virtual void build_graph() = 0;

    // cursor funcs
    typedef enum {UP, DOWN, LEFT, RIGHT} Cursor_dir;
    virtual void move_cursor(const Cursor_dir dir) = 0;
    virtual glm::vec3 cursor_pos() const = 0;
    virtual bool cursor_defined() const = 0;
    virtual std::string cursor_text() const = 0;

public:
    // material properties
    GLuint tex;
    float shininess;
    glm::vec3 specular;

    glm::vec4 grid_color;
    float grid_shininess;
    glm::vec3 grid_specular;

protected:
    std::string _eqn;
    mu::Parser _p;

    GLuint _ebo;
    GLuint _vao;
    GLuint _vbo;
    GLuint _num_indexes;

    GLuint _grid_ebo;
    GLuint _grid_num_indexes;

    // TODO: maybe have each have its own shader program?

private:
    // make non-copyable
    Graph(const Graph &) = delete;
    Graph(const Graph &&) = delete;
    Graph & operator=(const Graph &) = delete;
    Graph & operator=(const Graph &&) = delete;
};

class Graph_cartesian final: public Graph
{
public:
    Graph_cartesian(const std::string & eqn = "",
        float x_min = -1.0f, float x_max = 1.0f, int x_res = 50,
        float y_min = -1.0f, float y_max = 1.0f, int y_res = 50);

    double eval(const double x, const double y) override;
    void build_graph() override;

    // cursor funcs
    void move_cursor(const Cursor_dir dir) override;
    glm::vec3 cursor_pos() const override;
    bool cursor_defined() const override;
    std::string cursor_text() const override;

private:
    double _x, _y;
    double _x_min, _x_max;
    int _x_res;
    double _y_min, _y_max;
    int _y_res;

    glm::vec3 _cursor_pos;
    bool _cursor_defined;
};

#endif // __GRAPH_H__
