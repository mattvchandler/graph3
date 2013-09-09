// graph.cpp
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

#include <sstream>

#include "graph.h"

std::ostream & operator<<(std::ostream & out, const glm::vec2 & v)
{
    out<<"("<<v.x<<", "<<v.y<<")";
    return out;
}

std::ostream & operator<<(std::ostream & out, const glm::vec3 & v)
{
    out<<"("<<v.x<<", "<<v.y<<", "<<v.z<<")";
    return out;
}

std::ostream & operator<<(std::ostream & out, const glm::dvec3 & v)
{
    out<<"("<<v.x<<", "<<v.y<<", "<<v.z<<")";
    return out;
}

// calculate the normal of a point given surrounding points
glm::vec3 get_normal (glm::vec3 center,
    glm::vec3 u, bool u_def,
    glm::vec3 ur, bool ur_def,
    glm::vec3 r, bool r_def,
    glm::vec3 lr, bool lr_def,
    glm::vec3 d, bool d_def,
    glm::vec3 ll, bool ll_def,
    glm::vec3 l, bool l_def,
    glm::vec3 ul, bool ul_def)
{
    std::vector<glm::vec3> surrounding;

    // get cross-products from combinations of surrounding points
    if(u_def && ur_def)
        surrounding.push_back(glm::normalize(glm::cross(ur - center, u - center)));
    if(u_def && r_def)
        surrounding.push_back(glm::normalize(glm::cross(r - center, u - center)));
    if(u_def && lr_def)
        surrounding.push_back(glm::normalize(glm::cross(lr - center, u - center)));

    if(ur_def && r_def)
        surrounding.push_back(glm::normalize(glm::cross(r - center, ur - center)));
    if(ur_def && lr_def)
        surrounding.push_back(glm::normalize(glm::cross(lr - center, ur - center)));
    if(ur_def && d_def)
        surrounding.push_back(glm::normalize(glm::cross(d - center, ur - center)));

    if(r_def && lr_def)
        surrounding.push_back(glm::normalize(glm::cross(lr - center, r - center)));
    if(r_def && d_def)
        surrounding.push_back(glm::normalize(glm::cross(d - center, r - center)));
    if(r_def && ll_def)
        surrounding.push_back(glm::normalize(glm::cross(ll - center, r - center)));

    if(lr_def && d_def)
        surrounding.push_back(glm::normalize(glm::cross(d - center, lr - center)));
    if(lr_def && ll_def)
        surrounding.push_back(glm::normalize(glm::cross(ll - center, lr - center)));
    if(lr_def && l_def)
        surrounding.push_back(glm::normalize(glm::cross(l - center, lr - center)));

    if(d_def && ll_def)
        surrounding.push_back(glm::normalize(glm::cross(ll - center, d - center)));
    if(d_def && l_def)
        surrounding.push_back(glm::normalize(glm::cross(l - center, d - center)));
    if(d_def && ul_def)
        surrounding.push_back(glm::normalize(glm::cross(ul - center, d - center)));

    if(ll_def && l_def)
        surrounding.push_back(glm::normalize(glm::cross(l - center, ll - center)));
    if(ll_def && ul_def)
        surrounding.push_back(glm::normalize(glm::cross(ul - center, ll - center)));
    if(ll_def && u_def)
        surrounding.push_back(glm::normalize(glm::cross(u - center, ll - center)));

    if(l_def && ul_def)
        surrounding.push_back(glm::normalize(glm::cross(ul - center, l - center)));
    if(l_def && u_def)
        surrounding.push_back(glm::normalize(glm::cross(u - center, l - center)));
    if(l_def && ur_def)
        surrounding.push_back(glm::normalize(glm::cross(ur - center, l - center)));

    if(ul_def && u_def)
        surrounding.push_back(glm::normalize(glm::cross(u - center, ul - center)));
    if(ul_def && ur_def)
        surrounding.push_back(glm::normalize(glm::cross(ur - center, ul - center)));
    if(ul_def && r_def)
        surrounding.push_back(glm::normalize(glm::cross(r - center, ul - center)));

    glm::vec3 normal(0.0f);
    for(auto &i: surrounding)
        normal += i;

    return glm::normalize(normal);
}

Graph::Graph(const std::string & eqn):
    tex(0), color(1.0f), shininess(50.0f), specular(1.0f),
    grid_color(0.1f, 0.1f, 0.1f, 1.0f), grid_shininess(100.0f), grid_specular(1.0f),
     _eqn(eqn), _ebo(0), _vao(0), _vbo(0), _num_indexes(0)

{
    _p.DefineConst("pi", M_PI);
    _p.DefineConst("e", M_E);
}

Graph::~Graph()
{
    if(_ebo)
        glDeleteBuffers(1, &_ebo);
    if(_vao)
        glDeleteVertexArrays(1, &_vao);
    if(_vbo)
        glDeleteBuffers(1, &_vbo);
}

// drawing code
void Graph::draw() const
{
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBindTexture(GL_TEXTURE_2D, tex);

    glDrawElements(GL_TRIANGLE_STRIP, _num_indexes, GL_UNSIGNED_INT, NULL);
}

void Graph::draw_grid() const
{
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _grid_ebo);

    glDrawElements(GL_LINE_STRIP, _grid_num_indexes, GL_UNSIGNED_INT, NULL);
}

sigc::signal<void> Graph::signal_cursor_moved()
{
    return _signal_cursor_moved;
}
