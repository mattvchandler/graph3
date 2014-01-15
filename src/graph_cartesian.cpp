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

#include <iomanip>
#include <sstream>

#include "graph_cartesian.hpp"

Graph_cartesian::Graph_cartesian(const std::string & eqn,
    const std::string & x_min, const std::string & x_max, size_t x_res,
    const std::string & y_min, const std::string & y_max, size_t y_res):
    _eqn(eqn), _x(0.0), _y(0.0), _x_res(x_res), _y_res(y_res)
{
    _p.DefineConst("pi", M_PI);
    _p.DefineConst("e", M_E);
    double min, max;

    _p.SetExpr(x_min);
    try
    {
        min = _p.Eval();
    }
    catch(const mu::Parser::exception_type & e)
    {
        Graph_exception ge(e, Graph_exception::ROW_MIN);
        throw ge;
    }

    _p.SetExpr(x_max);
    try
    {
        max = _p.Eval();
    }
    catch(const mu::Parser::exception_type & e)
    {
        Graph_exception ge(e, Graph_exception::ROW_MAX);
        throw ge;
    }

    _x_min = std::min(min, max);
    _x_max = std::max(min, max);

    _p.SetExpr(y_min);
    try
    {
        min = _p.Eval();
    }
    catch(const mu::Parser::exception_type & e)
    {
        Graph_exception ge(e, Graph_exception::COL_MIN);
        throw ge;
    }

    _p.SetExpr(y_max);
    try
    {
        max = _p.Eval();
    }
    catch(const mu::Parser::exception_type & e)
    {
        Graph_exception ge(e, Graph_exception::COL_MAX);
        throw ge;
    }

    _y_min = std::min(min, max);
    _y_max = std::max(min, max);

    _p.DefineVar("x", &_x);
    _p.DefineVar("y", &_y);
    _p.SetExpr(eqn);

    build_graph();
}

double Graph_cartesian::eval(const double x, const double y)
{
    _x = x; _y = y;
    try
    {
        return _p.Eval();
    }
    catch(const mu::Parser::exception_type & e)
    {
        Graph_exception ge(e, Graph_exception::EQN);
        throw ge;
    }
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
            glm::vec3 up, dn, lf, rt, ul, ur, ll, lr;
            bool up_def = false, dn_def = false, lf_def = false, rt_def = false,
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

            // up
            z = eval(x, u_y);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                up_def = true;
                up = glm::vec3(x, u_y, z);
            }

            // ur
            z = eval(r_x, u_y);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                ur_def = true;
                ur = glm::vec3(r_x, u_y, z);
            }

            // rt
            z = eval(r_x, y);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                rt_def = true;
                rt = glm::vec3(r_x, y, z);
            }

            // lr
            z = eval(r_x, d_y);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                lr_def = true;
                lr = glm::vec3(r_x, d_y, z);
            }

            // dn
            z = eval(x, d_y);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                dn_def = true;
                dn = glm::vec3(x, d_y, z);
            }

            // ll
            z = eval(l_x, d_y);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                ll_def = true;
                ll = glm::vec3(l_x, d_y, z);
            }

            // lf
            z = eval(l_x, y);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                lf_def = true;
                lf = glm::vec3(l_x, y, z);
            }

            normals[y_i * _x_res + x_i] = get_normal(coords[y_i * _x_res + x_i],
                up, up_def,
                ur, ur_def,
                rt, rt_def,
                lr, lr_def,
                dn, dn_def,
                ll, ll_def,
                lf, lf_def,
                ul, ul_def);
        }
    }

    build_graph_geometry(_y_res, _x_res, coords, tex_coords, normals, defined);

    // initialize cursor
    _cursor_pos.x = (_x_max - _x_min) / 2.0 + _x_min;
    _cursor_pos.y = (_y_max - _y_min) / 2.0 + _y_min;
    _cursor_pos.z = eval(_cursor_pos.x, _cursor_pos.y);
    _cursor_defined = std::fpclassify(_cursor_pos.z) == FP_NORMAL || std::fpclassify(_cursor_pos.z) == FP_ZERO;
    _signal_cursor_moved.emit(cursor_text());
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
    _signal_cursor_moved.emit(cursor_text());
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
    std::string eqn = _eqn;

    if(_eqn.size() > 50)
        eqn = _eqn.substr(0, 49) + "…";

    str<<std::setprecision(4)<<"z(x, y) = "<<eqn<<"; z("<<_cursor_pos.x<<", "<<_cursor_pos.y<<") = "<<_cursor_pos.z;
    return str.str();
}
