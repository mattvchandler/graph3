// graph_cartesian.cpp
// cartesian coordinate system graph class (Z(X, Y))

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

#include "graph_cartesian.hpp"

Graph_cartesian::Graph_cartesian(const std::string & eqn, float x_min, float x_max, int x_res,
    float y_min, float y_max, int y_res): Graph(eqn),
    _x_min(x_min), _x_max(x_max), _x_res(x_res), _y_min(y_min), _y_max(y_max), _y_res(y_res)

{
    _p.DefineVar("x", &_x);
    _p.DefineVar("y", &_y);
    _p.SetExpr(eqn);

    build_graph();
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

// OpenGL needs to be initialized before this is run, hence it's not in the ctor
void Graph_cartesian::build_graph()
{
    std::vector<glm::vec3> coords(_y_res * _x_res);
    std::vector<glm::vec2> tex_coords(_y_res * _x_res);
    std::vector<glm::vec3> normals(_y_res * _x_res);
    std::vector<bool> defined(_y_res * _x_res);

    float h_x = 1e-3f * (_x_max - _x_min) / (float)_x_res;
    float h_y = 1e-3f * (_y_max - _y_min) / (float)_y_res;

    // calculate coords, texture cords, and normals
    double y = _y_max;
    for(size_t y_i = 0; y_i < _y_res; ++y_i, y -= (_y_max - _y_min) / (double)(_y_res - 1))
    {
        double x = _x_min;
        for(size_t x_i = 0; x_i < _x_res; ++x_i,  x += (_x_max - _x_min) / (double)(_x_res - 1))
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

            coords[y_i * _x_res + x_i] = glm::vec3((float)x, (float)y, (float)z);
            tex_coords[y_i * _x_res + x_i] = glm::vec2((float)((x - _x_min) / (_x_max - _x_min)), (float)((_y_max - y) / (_y_max - _y_min)));
            defined[y_i * _x_res + x_i] = true;

            // calculate surrounding points for normal calculation
            glm::vec3 u, d, l, r, ul, ur, ll, lr;
            bool u_def = false, d_def = false, l_def = false, r_def = false,
                 ul_def = false, ur_def = false, ll_def = false, lr_def = false;

            float l_x = (float)x - h_x;
            float r_x = (float)x + h_x;
            float u_y = (float)y + h_y;
            float d_y = (float)y - h_y;

            // ul
            z = eval(l_x, u_y);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                ul_def = true;
                ul = glm::vec3(l_x, u_y, z);
            }

            // u
            z = eval(x, u_y);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                u_def = true;
                u = glm::vec3(x, u_y, z);
            }

            // ur
            z = eval(r_x, u_y);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                ur_def = true;
                ur = glm::vec3(r_x, u_y, z);
            }

            // r
            z = eval(r_x, y);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                r_def = true;
                r = glm::vec3(r_x, y, z);
            }

            // lr
            z = eval(r_x, d_y);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                lr_def = true;
                lr = glm::vec3(r_x, d_y, z);
            }

            // d
            z = eval(x, d_y);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                d_def = true;
                d = glm::vec3(x, d_y, z);
            }

            // ll
            z = eval(l_x, d_y);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                ll_def = true;
                ll = glm::vec3(l_x, d_y, z);
            }

            // l
            z = eval(l_x, y);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                l_def = true;
                l = glm::vec3(l_x, y, z);
            }

            normals[y_i * _x_res + x_i] = get_normal(coords[y_i * _x_res + x_i],
                u, u_def,
                ur, ur_def,
                r, r_def,
                lr, lr_def,
                d, d_def,
                ll, ll_def,
                l, l_def,
                ul, ul_def);
        }
    }

    build_graph_geometry(_y_res, _x_res, coords, tex_coords, normals, defined);

    // initialize cursor
    _cursor_pos.x = (_x_max - _x_min) / 2.0 + _x_min;
    _cursor_pos.y = (_y_max - _y_min) / 2.0 + _y_min;
    _cursor_pos.z = eval(_cursor_pos.x, _cursor_pos.y);
    _cursor_defined = std::fpclassify(_cursor_pos.z) == FP_NORMAL || std::fpclassify(_cursor_pos.z) == FP_ZERO;
    _signal_cursor_moved.emit();
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
    _signal_cursor_moved.emit();
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
