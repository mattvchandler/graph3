// graph.cpp
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

Graph::Graph(const std::string & eqn):
    tex(0), shininess(50.0f), specular(1.0f),
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
    if(tex)
        glDeleteTextures(1, &tex);
}

// drawing code
void Graph::draw() const
{
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBindTexture(GL_TEXTURE_2D, tex);

    glDrawElements(GL_TRIANGLE_STRIP, _num_indexes, GL_UNSIGNED_SHORT, NULL);
}

void Graph::draw_grid() const
{
    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _grid_ebo);

    glDrawElements(GL_LINE_STRIP, _grid_num_indexes, GL_UNSIGNED_SHORT, NULL);
}

Graph_cartesian::Graph_cartesian(const std::string & eqn, float x_min, float x_max, int x_res,
    float y_min, float y_max, int y_res): Graph(eqn),
    _x_min(x_min), _x_max(x_max), _x_res(x_res), _y_min(y_min), _y_max(y_max), _y_res(y_res)

{
    _p.DefineVar("x", &_x);
    _p.DefineVar("y", &_y);
    _p.SetExpr(eqn);
}

double Graph_cartesian::eval(const double x, const double y)
{
    _x = x; _y = y;
    double result = 0.0;
    try
    {
        result = _p.Eval();
    }
    catch(mu::Parser::exception_type &e)
    {
        std::cerr<<"Error evaluating equation:"<<std::endl;
        std::cerr<<"Message:  "<< e.GetMsg()<<std::endl;
        std::cerr<<"Formula:  "<< e.GetExpr()<<std::endl;
        std::cerr<<"Token:    "<< e.GetToken() <<std::endl;
        std::cerr<<"Position: "<< e.GetPos()<<std::endl;
        std::cerr<<"Errc:     "<< e.GetCode()<<std::endl;
        throw;
    }
    return result;
}

void Graph_cartesian::build_graph()
{
    std::vector<glm::vec3> coords(_y_res * _x_res);
    std::vector<glm::vec2> tex_coords(_y_res * _x_res);
    std::vector<glm::vec3> normals(_y_res * _x_res);
    std::vector<bool> defined(_y_res * _x_res);

    // coordinate pass
    double y = _y_max;
    for(int y_i = 0; y_i < _y_res; ++y_i, y -= (_y_max - _y_min) / (double)(_y_res - 1))
    {
        double x = _x_min;
        for(int x_i = 0; x_i < _x_res; ++x_i,  x += (_x_max - _x_min) / (double)(_x_res - 1))
        {
            double z = eval(x, y);

            if(std::fpclassify(z) != FP_NORMAL &&
                std::fpclassify(z) != FP_ZERO)
            {
                coords[y_i * _x_res + x_i] = glm::vec3(0.0f);
                tex_coords[y_i * _x_res + x_i] = glm::vec2(0.0f);
                normals[y_i * _x_res + x_i] = glm::vec3(0.0f, 0.0f, 1.0f);
                defined[y_i * _x_res + x_i] = false;
                continue;
            }

            // re-arranged into OpenGL's coordinate system
            coords[y_i * _x_res + x_i] = glm::vec3((float)x, (float)y, (float)z);
            tex_coords[y_i * _x_res + x_i] = glm::vec2((float)((x - _x_min) / (_x_max - _x_min)), (float)((_y_max - y) / (_y_max - _y_min)));
            defined[y_i * _x_res + x_i] = true;
        }
    }

    // normal pass
    for(int y_i = 0; y_i < _y_res; ++y_i)
    {
        for(int x_i = 0; x_i < _x_res; ++x_i)
        {
            if(!defined[y_i * _x_res + x_i])
                continue;

            // get / calculate coords of surrounding verts
            glm::vec3 u, d, l, r;
            glm::vec3 ul, ur, ll, lr;
            bool u_def = false, d_def = false, l_def = false, r_def = false;
            bool ul_def = false, ur_def = false, ll_def = false, lr_def = false;

            double l_x, r_x, u_y, d_y, x, y, z;

            x = coords[y_i * _x_res + x_i].x;
            y = coords[y_i * _x_res + x_i].y;

            if(x_i == 0)
                l_x = _x_min - (_x_max - _x_min) / (double)_x_res;
            else
                l_x = coords[y_i * _x_res + x_i - 1].x;

            if(x_i == _x_res - 1)
                r_x = _x_max + (_x_max - _x_min) / (double)_x_res;
            else
                r_x = coords[y_i * _x_res + x_i + 1].x;

            if(y_i == 0)
                u_y = _y_max + (_y_max - _y_min) / (double)_y_res;
            else
                u_y = coords[(y_i - 1) * _x_res + x_i].y;

            if(y_i == _y_res - 1)
                d_y = _y_min - (_y_max - _y_min) / (double)_y_res;
            else
                d_y = coords[(y_i + 1) * _x_res + x_i].y;

            // ul
            if(x_i == 0 || y_i == 0)
            {
                z = eval(l_x, u_y);
                if(std::fpclassify(z) == FP_NORMAL ||
                    std::fpclassify(z) == FP_ZERO)
                {
                    ul_def = true;
                    ul = glm::vec3(l_x, u_y, z);
                }
            }
            else
            {
                ul_def = defined[(y_i - 1) * _x_res + x_i - 1];
                ul = coords[(y_i - 1) * _x_res + x_i - 1];
            }

            // u
            if(y_i == 0)
            {
                z = eval(x, u_y);
                if(std::fpclassify(z) == FP_NORMAL ||
                    std::fpclassify(z) == FP_ZERO)
                {
                    u_def = true;
                    u = glm::vec3(x, u_y, z);
                }
            }
            else
            {
                u_def = defined[(y_i - 1) * _x_res + x_i];
                u = coords[(y_i - 1) * _x_res + x_i];
            }

            // ur
            if(x_i == _x_res - 1 || y_i == 0)
            {
                z = eval(r_x, u_y);
                if(std::fpclassify(z) == FP_NORMAL ||
                    std::fpclassify(z) == FP_ZERO)
                {
                    ur_def = true;
                    ur = glm::vec3(r_x, u_y, z);
                }
            }
            else
            {
                ur_def = defined[(y_i - 1) * _x_res + x_i + 1];
                ur = coords[(y_i - 1) * _x_res + x_i + 1];
            }

            // r
            if(x_i == _x_res - 1)
            {
                z = eval(r_x, y);
                if(std::fpclassify(z) == FP_NORMAL ||
                    std::fpclassify(z) == FP_ZERO)
                {
                    r_def = true;
                    r = glm::vec3(r_x, y, z);
                }
            }
            else
            {
                r_def = defined[y_i * _x_res + x_i + 1];
                r = coords[y_i * _x_res + x_i + 1];
            }

            // lr
            if(x_i == _x_res - 1 || y_i == _y_res - 1)
            {
                z = eval(r_x, d_y);
                if(std::fpclassify(z) == FP_NORMAL ||
                    std::fpclassify(z) == FP_ZERO)
                {
                    lr_def = true;
                    lr = glm::vec3(r_x, d_y, z);
                }
            }
            else
            {
                lr_def = defined[(y_i + 1) * _x_res + x_i + 1];
                lr = coords[(y_i + 1) * _x_res + x_i + 1];
            }

            // d
            if(y_i == _y_res - 1)
            {
                z = eval(x, d_y);
                if(std::fpclassify(z) == FP_NORMAL ||
                    std::fpclassify(z) == FP_ZERO)
                {
                    d_def = true;
                    d = glm::vec3(x, d_y, z);
                }
            }
            else
            {
                d_def = defined[(y_i + 1) * _x_res + x_i];
                d = coords[(y_i + 1) * _x_res + x_i];
            }

            // ll
            if(x_i == 0 || y_i == _y_res - 1)
            {
                z = eval(l_x, d_y);
                if(std::fpclassify(z) == FP_NORMAL ||
                    std::fpclassify(z) == FP_ZERO)
                {
                    ll_def = true;
                    ll = glm::vec3(l_x, d_y, z);
                }
            }
            else
            {
                ll_def = defined[(y_i + 1) * _x_res + x_i - 1];
                ll = coords[(y_i + 1) * _x_res + x_i - 1];
            }

            // l
            if(x_i == 0)
            {
                z = eval(l_x, y);
                if(std::fpclassify(z) == FP_NORMAL ||
                    std::fpclassify(z) == FP_ZERO)
                {
                    l_def = true;
                    l = glm::vec3(l_x, y, z);
                }
            }
            else
            {
                l_def = defined[y_i * _x_res + x_i - 1];
                l = coords[y_i * _x_res + x_i - 1];
            }

            std::vector<glm::vec3> surrounding;

            // std::cout<<ul<<u<<ur<<r<<lr<<d<<ll<<l<<std::endl; // TODO: remove debug
            // std::cout<<ul_def<<u_def<<ur_def<<r_def<<lr_def<<d_def<<ll_def<<l_def<<std::endl; // TODO: remove debug
            glm::vec3 center = coords[y_i * _x_res + x_i];

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

            for(auto &i: surrounding)
                normals[y_i * _x_res + x_i] += i;

            normals[y_i * _x_res + x_i] = glm::normalize(normals[y_i * _x_res + x_i]);
        }
    }

    // TODO: remove debug
    /*
    for(auto &i: coords)
        std::cout<<i<<" ";
    std::cout<<std::endl;

    for(auto &i: tex_coords)
        std::cout<<i<<" ";
    std::cout<<std::endl;

    for(auto &i: normals)
        std::cout<<i<<" ";
    std::cout<<std::endl;

    for(size_t i = 0; i < defined.size(); ++i)
        std::cout<<(defined[i]?"def":"undef")<<" ";
    std::cout<<std::endl;
    */

    // TODO: remove
    // print 'random' samples
    // std::cout<<verts[6][6]<<std::endl;
    // std::cout<<verts[6][5].coords<<" "<<verts[6][7].coords<<" "<<verts[5][6].coords<<" "<<verts[7][6].coords<<std::endl;

    std::vector<GLushort> index;

    bool break_flag = true;


    // IDeas: indexed, use restart index to start new row / skip undefined? rework above to build separate arrays that we can use directly? unless undefined throws us off

    // arrange verts as a triangle strip
    for(int y_i = 0; y_i < _y_res - 1; ++y_i)
    {
        for(int x_i = 0; x_i < _x_res - 1; ++x_i)
        {
            int ul = y_i * _x_res + x_i;
            int ur = y_i * _x_res + x_i + 1;
            int ll = (y_i + 1) * _x_res + x_i;
            int lr = (y_i + 1) * _x_res + x_i + 1;

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
                index.push_back(0xFFFF);
                break_flag = true;
            }
            else if(defined[ul] && defined[ur] && defined[lr])
            {
                if(!break_flag)
                    index.push_back(0xFFFF);
                index.push_back(ul);
                index.push_back(lr);
                index.push_back(ur);
                index.push_back(0xFFFF);
                break_flag = true;
            }
            else if(defined[ul] && defined[ll] && defined[lr])
            {
                index.push_back(ul);
                index.push_back(ll);
                index.push_back(lr);
                index.push_back(0xFFFF);
                break_flag = true;
            }
            else if(defined[ur] && defined[ll] && defined[lr])
            {
                if(!break_flag)
                    index.push_back(0xFFFF);
                index.push_back(ur);
                index.push_back(ll);
                index.push_back(lr);
                index.push_back(0xFFFF);
                break_flag = true;
            }
            else
            {
                if(!break_flag)
                    index.push_back(0xFFFF);
                break_flag = true;
            }
        }
        // finish row
        int ul = y_i * _x_res + _x_res - 1;
        int ll = (y_i + 1) * _x_res + _x_res - 1;

        if(!break_flag && defined[ul] && defined[ll])
        {
            index.push_back(ul);
            index.push_back(ll);
        }

        if(!break_flag)
            index.push_back(0xFFFF);
        break_flag = true;
    }

    // TODO: remove debug
    /*
    std::cout<<std::endl;
    for(auto &i: index)
    {
        if(i != 0xFFFF)
            std::cout<<i<<":"<<coords[i]<<"/"<<normals[i]<<" ";
        else
            std::cout<<std::endl;
    }
    */

    // generate required OpenGL structures
    glGenBuffers(1, &_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * index.size(), &index[0], GL_STATIC_DRAW);

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
    std::vector<GLushort> grid_index;

    // horizontal pass
    for(int i = 1; i < 10; ++i)
    {
        for(int x_i = 0; x_i < _x_res; ++x_i)
        {
            GLushort ind = (int)((float)_y_res * (float)i / 10.0f) * _x_res + x_i;
            if(defined[ind])
                grid_index.push_back(ind);
            else
                grid_index.push_back(0xFFFF);
        }
        grid_index.push_back(0xFFFF);
    }

    //vertical pass
    for(int i = 1; i < 10; ++i)
    {
        for(int y_i = 0; y_i < _y_res; ++y_i)
        {
            GLushort ind = y_i * _x_res + (int)((float)_x_res * (float)i / 10.0f);
            if(defined[ind])
                grid_index.push_back(ind);
            else
                grid_index.push_back(0xFFFF);
        }
        grid_index.push_back(0xFFFF);
    }

    // generate required OpenGL structures
    glGenBuffers(1, &_grid_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _grid_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * grid_index.size(), &grid_index[0], GL_STATIC_DRAW);

    _grid_num_indexes = grid_index.size();

    // initialize cursor
    _cursor_pos.x = (_x_max - _x_min) / 2.0 + _x_min;
    _cursor_pos.y = (_y_max - _y_min) / 2.0 + _y_min;
    _cursor_pos.z = eval(_cursor_pos.x, _cursor_pos.y);
    _cursor_defined = std::fpclassify(_cursor_pos.z) == FP_NORMAL || std::fpclassify(_cursor_pos.z) == FP_ZERO;
}

void Graph_cartesian::move_cursor(const Cursor_dir dir)
{
    switch(dir)
    {
    case UP:
        _cursor_pos.y += (_y_max - _y_min) / (double)_y_res;
        if(_cursor_pos.y > _y_max)
            _cursor_pos.y -= _y_max - _y_min;
        break;
    case DOWN:
        _cursor_pos.y -= (_y_max - _y_min) / (double)_y_res;
        if(_cursor_pos.y < _y_min)
            _cursor_pos.y += _y_max - _y_min;
        break;
    case LEFT:
        _cursor_pos.x -= (_x_max - _x_min) / (double)_x_res;
        if(_cursor_pos.x < _x_min)
            _cursor_pos.x += _x_max - _x_min;
        break;
    case RIGHT:
        _cursor_pos.x += (_x_max - _x_min) / (double)_x_res;
        if(_cursor_pos.x > _x_max)
            _cursor_pos.x -= _x_max - _x_min;
        break;
    default:
        break;
    }
    _cursor_pos.z = eval(_cursor_pos.x, _cursor_pos.y);
    _cursor_defined = std::fpclassify(_cursor_pos.z) == FP_NORMAL || std::fpclassify(_cursor_pos.z) == FP_ZERO;
}

glm::vec3 Graph_cartesian::cursor_pos() const
{
    return _cursor_pos;
}

bool Graph_cartesian::cursor_defined() const
{
    return _cursor_defined;
}

std::string Graph_cartesian::cursor_text() const
{
    std::ostringstream str;
    str<<"Z(X: "<<_cursor_pos.x<<", Y: "<<_cursor_pos.y<<") = "<<_cursor_pos.z;
    return str.str();
}
