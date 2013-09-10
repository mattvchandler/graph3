// graph.h
// generic graphing class

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

#include <string>

#include <GL/glew.h>

#include <sigc++/sigc++.h>

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
std::ostream & operator<<(std::ostream & out, const glm::dvec3 & v);

// calculate the normal of a point given surrounding points
glm::vec3 get_normal (glm::vec3 center,
    glm::vec3 u, bool u_def,
    glm::vec3 ur, bool ur_def,
    glm::vec3 r, bool r_def,
    glm::vec3 lr, bool lr_def,
    glm::vec3 d, bool d_def,
    glm::vec3 ll, bool ll_def,
    glm::vec3 l, bool l_def,
    glm::vec3 ul, bool ul_def);

class Graph
{
public:
    explicit Graph(const std::string & eqn = "");
    virtual ~Graph();

    void draw() const;
    void draw_grid() const;

    virtual double eval(const double, const double) = 0;

    // cursor funcs
    typedef enum {UP, DOWN, LEFT, RIGHT} Cursor_dir;
    virtual void move_cursor(const Cursor_dir dir) = 0;
    virtual glm::vec3 cursor_pos() const = 0;
    virtual bool cursor_defined() const = 0;
    virtual std::string cursor_text() const = 0;
    sigc::signal<void> signal_cursor_moved();

    // material properties
    GLuint tex;
    glm::vec4 color;
    float shininess;
    glm::vec3 specular;

    glm::vec4 grid_color;

protected:
    virtual void build_graph() = 0;
    void build_graph_geometry(size_t num_rows, size_t num_columns,
        const std::vector<glm::vec3> & coords,
        const std::vector<glm::vec2> & tex_coords,
        const std::vector<glm::vec3> & normals,
        const std::vector<bool> & defined);

    std::string _eqn;
    mu::Parser _p;

    GLuint _ebo;
    GLuint _vao;
    GLuint _vbo;
    GLuint _num_indexes;

    GLuint _grid_ebo;
    GLuint _grid_num_indexes;

    sigc::signal<void> _signal_cursor_moved;

private:
    // make non-copyable
    Graph(const Graph &) = delete;
    Graph(const Graph &&) = delete;
    Graph & operator=(const Graph &) = delete;
    Graph & operator=(const Graph &&) = delete;
};

#endif // __GRAPH_H__
