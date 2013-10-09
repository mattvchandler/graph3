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

#include <limits>
#include <sstream>

#include "gl_helpers.hpp"
#include "graph.hpp"

// TODO: remove these debug funcs
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

Graph_exception::Graph_exception(const mu::Parser::exception_type & mu_e, const Location l):
    mu::Parser::exception_type(mu_e), _location(l)
{
}

Graph_exception::Location Graph_exception::GetLocation() const
{
    return _location;
}

// calculate the normal of a point given surrounding points
glm::vec3 get_normal (glm::vec3 center,
    glm::vec3 up, bool up_def,
    glm::vec3 ur, bool ur_def,
    glm::vec3 rt, bool rt_def,
    glm::vec3 lr, bool lr_def,
    glm::vec3 dn, bool dn_def,
    glm::vec3 ll, bool ll_def,
    glm::vec3 lf, bool lf_def,
    glm::vec3 ul, bool ul_def)
{
    const float epsilon = std::numeric_limits<double>::epsilon() / 2.0f;

    std::vector<glm::dvec3> surrounding;

    // temporary vars
    glm::dvec3 cr;
    double length;

    // get tangents through surrounding points
    glm::dvec3 t_up, t_ur, t_rt, t_lr, t_dn, t_ll, t_lf, t_ul; 

    // check to make sure we have no 0-length vectors, and then normalize
    if(up_def)
    {
        t_up = up - center;
        length = glm::length(t_up);
        if(length <= epsilon)
            up_def = false;
    }
    if(ur_def)
    {
        t_ur = ur - center;
        length = glm::length(t_ur);
        if(length <= epsilon)
            ur_def = false;
    }
    if(rt_def)
    {
        t_rt = rt - center;
        length = glm::length(t_rt);
        if(length <= epsilon)
            rt_def = false;
    }
    if(lr_def)
    {
        t_lr = lr - center;
        length = glm::length(t_lr);
        if(length <= epsilon)
            lr_def = false;
    }
    if(dn_def)
    {
        t_dn = dn - center;
        length = glm::length(t_dn);
        if(length <= epsilon)
            dn_def = false;
    }
    if(ll_def)
    {
        t_ll = ll - center;
        length = glm::length(t_ll);
        if(length <= epsilon)
            ll_def = false;
    }
    if(lf_def)
    {
        t_lf = lf - center;
        length = glm::length(t_lf);
        if(length <= epsilon)
            lf_def = false;
    }
    if(ul_def)
    {
        t_ul = ul - center;
        length = glm::length(t_ul);
        if(length <= epsilon)
            ul_def = false;
    }

    // get cross-products from combinations of surrounding points
    // check for colinearity

    if(up_def)
    {
        if(ur_def)
        {
            cr = glm::cross(t_ur, t_up);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(rt_def)
        {
            cr = glm::cross(t_rt, t_up);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(lr_def)
        {
            cr = glm::cross(t_lr, t_up);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
    }

    if(ur_def)
    {
        if(rt_def)
        {
            cr = glm::cross(t_rt, t_ur);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(lr_def)
        {
            cr = glm::cross(t_lr, t_ur);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(dn_def)
        {
            cr = glm::cross(t_dn, t_ur);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
    }

    if(rt_def)
    {
        if(lr_def)
        {
            cr = glm::cross(t_lr, t_rt);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(dn_def)
        {
            cr = glm::cross(t_dn, t_rt);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(ll_def)
        {
            cr = glm::cross(t_ll, t_rt);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
    }

    if(lr_def)
    {
        if(dn_def)
        {
            cr = glm::cross(t_dn, t_lr);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(ll_def)
        {
            cr = glm::cross(t_ll, t_lr);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(lf_def)
        {
            cr = glm::cross(t_lf, t_lr);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
    }

    if(dn_def)
    {
        if(ll_def)
        {
            cr = glm::cross(t_ll, t_dn);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(lf_def)
        {
            cr = glm::cross(t_lf, t_dn);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(ul_def)
        {
            cr = glm::cross(t_ul, t_dn);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
    }

    if(ll_def)
    {
        if(lf_def)
        {
            cr = glm::cross(t_lf, t_ll);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(ul_def)
        {
            cr = glm::cross(t_ul, t_ll);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(up_def)
        {
            cr = glm::cross(t_up, t_ll);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
    }

    if(lf_def)
    {
        if(ul_def)
        {
            cr = glm::cross(t_ul, t_lf);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(up_def)
        {
            cr = glm::cross(t_up, t_lf);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(ur_def)
        {
            cr = glm::cross(t_ur, t_lf);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
    }

    if(ul_def)
    {
        if(up_def)
        {
            cr = glm::cross(t_up, t_ul);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(ur_def)
        {
            cr = glm::cross(t_ur, t_ul);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(rt_def)
        {
            cr = glm::cross(t_rt, t_ul);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
    }

    glm::dvec3 normal(0.0f);
    for(auto &i: surrounding)
    {
        // invert inverted normals
        if(glm::length(normal + i) > glm::length(normal))
            normal += i;
        else
            normal -= i;
    }

    // check to see if we have a good vector before normalizing (to prevent div by 0)
    if(surrounding.size() > 0 && glm::length(normal) > epsilon)
        return glm::vec3(glm::normalize(normal));
    else
        return glm::vec3(0.0f, 0.0f, 1.0f);
}

Graph::Graph():
    use_tex(false), valid_tex(false), color(1.0f), shininess(50.0f), specular(1.0f),
    grid_color(0.1f, 0.1f, 0.1f, 1.0f), normal_color(0.0f, 1.0f, 1.0f, 1.0f),
    draw_flag(true), draw_grid_flag(true), draw_normals_flag(false),
    _tex(0), _ebo(0), _vao(0), _vbo(0), _num_indexes(0),
    _grid_ebo(0), _grid_num_indexes(0),
    _normal_vao(0), _normal_vbo(0), _normal_num_indexes(0)
{
}

Graph::~Graph()
{
    if(_tex)
        glDeleteTextures(1, &_tex);

    if(_ebo)
        glDeleteBuffers(1, &_ebo);
    if(_vao)
        glDeleteVertexArrays(1, &_vao);
    if(_vbo)
        glDeleteBuffers(1, &_vbo);

    if(_grid_ebo)
        glDeleteBuffers(1, &_grid_ebo);

    if(_normal_vao)
        glDeleteVertexArrays(1, &_normal_vao);
    if(_normal_vbo)
        glDeleteBuffers(1, &_normal_vbo);
}

// drawing code
void Graph::draw() const
{
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBindTexture(GL_TEXTURE_2D, _tex);

    glDrawElements(GL_TRIANGLE_STRIP, _num_indexes, GL_UNSIGNED_INT, NULL);
}

void Graph::draw_grid() const
{
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _grid_ebo);

    glDrawElements(GL_LINE_STRIP, _grid_num_indexes, GL_UNSIGNED_INT, NULL);
}

void Graph::draw_normals() const
{
    glBindVertexArray(_normal_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _normal_vbo);

    glDrawArrays(GL_LINES, 0, _normal_num_indexes);
}

void Graph::set_texture(const std::string & filename)
{
    if(_tex)
        glDeleteTextures(1, &_tex);

    // so we're in a good state if the texture creation line throws
    _tex = 0;
    valid_tex = false;

    if(filename.size() == 0)
        return;

    _tex = create_texture_from_file(filename);
    valid_tex = true;
}

sigc::signal<void, const std::string &> Graph::signal_cursor_moved()
{
    return _signal_cursor_moved;
}

void Graph::build_graph_geometry(size_t num_rows, size_t num_columns,
    const std::vector<glm::vec3> & coords,
    const std::vector<glm::vec2> & tex_coords,
    const std::vector<glm::vec3> & normals,
    const std::vector<bool> & defined)
{
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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * index.size(), index.data(), GL_STATIC_DRAW);

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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * grid_index.size(), grid_index.data(), GL_STATIC_DRAW);

    _grid_num_indexes = grid_index.size();

    // lines for normal vectors
    std::vector<glm::vec3> normal_coords;

    for(size_t i = 0; i < coords.size(); ++i)
    {
        if(defined[i])
        {
            normal_coords.push_back(coords[i]);
            normal_coords.push_back(coords[i] + 0.1f * normals[i]);
        }
    }

    glGenVertexArrays(1, &_normal_vao);
    glBindVertexArray(_normal_vao);

    glGenBuffers(1, &_normal_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _normal_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normal_coords.size(), normal_coords.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    _normal_num_indexes = normal_coords.size();
}
