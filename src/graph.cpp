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

    if(_grid_ebo)
        glDeleteBuffers(1, &_grid_ebo);
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

void Graph::build_graph_geometry(size_t num_rows, size_t num_columns,
    const std::vector<glm::vec3> & coords,
    const std::vector<glm::vec2> & tex_coords,
    const std::vector<glm::vec3> & normals,
    const std::vector<bool> & defined)
{
    // TODO: display normals

    std::vector<GLuint> index;

    bool break_flag = true;

    // arrange verts as a triangle strip
    for(size_t row = 0; row < num_rows - 1; ++row)
    {
        for(size_t column = 0; column < num_columns - 1; ++column)
        {
            int ul = row * num_columns + column;
            int ur = row * num_columns + column + 1;
            int ll = (row + 1) * num_columns + column;
            int lr = (row + 1) * num_columns + column + 1;

            if(defined[ul] && defined[ur] && defined[ll] && defined[lr])
            {
                index.push_back(ul);
                index.push_back(ll);
                break_flag = false;
            }
            else if(defined[ul] && defined[ur] && defined[ll])
            {
                index.push_back(ul);
                index.push_back(ll);
                index.push_back(ur);
                index.push_back(0xFFFFFFFF);
                break_flag = true;
            }
            else if(defined[ul] && defined[ur] && defined[lr])
            {
                if(!break_flag)
                    index.push_back(0xFFFFFFFF);
                index.push_back(ul);
                index.push_back(lr);
                index.push_back(ur);
                index.push_back(0xFFFFFFFF);
                break_flag = true;
            }
            else if(defined[ul] && defined[ll] && defined[lr])
            {
                index.push_back(ul);
                index.push_back(ll);
                index.push_back(lr);
                index.push_back(0xFFFFFFFF);
                break_flag = true;
            }
            else if(defined[ur] && defined[ll] && defined[lr])
            {
                if(!break_flag)
                    index.push_back(0xFFFFFFFF);
                index.push_back(ur);
                index.push_back(ll);
                index.push_back(lr);
                index.push_back(0xFFFFFFFF);
                break_flag = true;
            }
            else
            {
                if(!break_flag)
                    index.push_back(0xFFFFFFFF);
                break_flag = true;
            }
        }
        // finish row
        int ul = row * num_columns + num_columns - 1;
        int ll = (row + 1) * num_columns + num_columns - 1;

        if(!break_flag && defined[ul] && defined[ll])
        {
            index.push_back(ul);
            index.push_back(ll);
        }

        if(!break_flag)
            index.push_back(0xFFFFFFFF);
        break_flag = true;
    }

    // generate required OpenGL structures
    glGenBuffers(1, &_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * index.size(), &index[0], GL_STATIC_DRAW);

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * coords.size() + sizeof(glm::vec2) * tex_coords.size() +
        sizeof(glm::vec3) * normals.size(), NULL, GL_STATIC_DRAW);

    // store vertex data
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * coords.size(), &coords[0]);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * coords.size(), sizeof(glm::vec2) * tex_coords.size(), &tex_coords[0]);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * coords.size() + sizeof(glm::vec2) * tex_coords.size(),
        sizeof(glm::vec3) * normals.size(), &normals[0]);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(glm::vec3) * coords.size()));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid *)(sizeof(glm::vec3) * coords.size() + sizeof(glm::vec2) * tex_coords.size()));
    glEnableVertexAttribArray(2);

    _num_indexes = index.size();

    // generate grid lines
    std::vector<GLuint> grid_index;

    // horizontal pass
    for(size_t i = 1; i < 10; ++i)
    {
        for(size_t column = 0; column < num_columns; ++column)
        {
            GLuint ind = (int)((float)num_rows * (float)i / 10.0f) * num_columns + column;
            if(defined[ind])
                grid_index.push_back(ind);
            else
                grid_index.push_back(0xFFFFFFFF);
        }
        grid_index.push_back(0xFFFFFFFF);
    }

    //vertical pass
    for(size_t i = 1; i < 10; ++i)
    {
        for(size_t row = 0; row < num_rows; ++row)
        {
            GLuint ind = row * num_columns + (int)((float)num_columns * (float)i / 10.0f);
            if(defined[ind])
                grid_index.push_back(ind);
            else
                grid_index.push_back(0xFFFFFFFF);
        }
        grid_index.push_back(0xFFFFFFFF);
    }

    // generate required OpenGL structures
    glGenBuffers(1, &_grid_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _grid_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * grid_index.size(), &grid_index[0], GL_STATIC_DRAW);

    _grid_num_indexes = grid_index.size();
}
