// graph_cylindrical.cpp
// cylindrical coordinate system graph class (Z(r, theta))

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

#include "graph_cylindrical.h"

Graph_cylindrical::Graph_cylindrical(const std::string & eqn, float r_min, float r_max, int r_res,
    float theta_min, float theta_max, int theta_res): Graph(eqn),
    _r_min(r_min), _r_max(r_max), _r_res(r_res), _theta_min(theta_min), _theta_max(theta_max), _theta_res(theta_res)

{
    _p.DefineVar("r", &_r);
    _p.DefineVar("theta", &_theta);
    _p.SetExpr(eqn);

    build_graph();
}

double Graph_cylindrical::eval(const double r, const double theta)
{
    _r = r; _theta = theta;
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
void Graph_cylindrical::build_graph()
{
    std::vector<glm::vec3> coords(_theta_res * _r_res);
    std::vector<glm::vec2> tex_coords(_theta_res * _r_res);
    std::vector<glm::vec3> normals(_theta_res * _r_res);
    std::vector<bool> defined(_theta_res * _r_res);

    float h_r = 1e-3f * (_r_max - _r_min) / (float)_r_res;
    float h_theta = 1e-3f * (_theta_max - _theta_min) / (float)_theta_res;

    // calculate coords, texture cords, and normals
    double theta = _theta_max;
    for(size_t theta_i = 0; theta_i < _theta_res; ++theta_i, theta -= (_theta_max - _theta_min) / (double)(_theta_res - 1))
    {
        double radius = _r_min;
        for(size_t r_i = 0; r_i < _r_res; ++r_i,  radius += (_r_max - _r_min) / (double)(_r_res - 1))
        {
            double z = eval(radius, theta);

            if(std::fpclassify(z) != FP_NORMAL &&
                std::fpclassify(z) != FP_ZERO)
            {
                coords[theta_i * _r_res + r_i] = glm::vec3(0.0f);
                tex_coords[theta_i * _r_res + r_i] = glm::vec2(0.0f);
                normals[theta_i * _r_res + r_i] = glm::vec3(0.0f, 0.0f, 1.0f);
                defined[theta_i * _r_res + r_i] = false;
                continue;
            }

            // convert into cartesian coordinates
            coords[theta_i * _r_res + r_i] = glm::vec3((float)radius * cosf(theta), (float)radius * sinf(theta), (float)z);
            tex_coords[theta_i * _r_res + r_i] = glm::vec2((float)((radius - _r_min) / (_r_max - _r_min)), (float)((_theta_max - theta) / (_theta_max - _theta_min)));
            defined[theta_i * _r_res + r_i] = true;

            // calculate surrounding points for normal calculation
            glm::vec3 u, d, l, r, ul, ur, ll, lr;
            bool u_def = false, d_def = false, l_def = false, r_def = false,
                 ul_def = false, ur_def = false, ll_def = false, lr_def = false;

            float l_r = (float)radius - h_r;
            float r_r = (float)radius + h_r;
            float u_theta = (float)theta + h_theta;
            float d_theta = (float)theta - h_theta;

            // ul
            z = eval(l_r, u_theta);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                ul_def = true;
                ul = glm::vec3(l_r * cosf(u_theta), l_r * sinf(u_theta), z);
            }

            // u
            z = eval(radius, u_theta);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                u_def = true;
                u = glm::vec3(radius * cosf(u_theta), radius * sinf(u_theta), z);
            }

            // ur
            z = eval(r_r, u_theta);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                ur_def = true;
                ur = glm::vec3(r_r * cosf(u_theta), r_r * sinf(u_theta), z);
            }

            // r
            z = eval(r_r, theta);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                r_def = true;
                r = glm::vec3(r_r * cosf(theta), r_r * sinf(theta), z);
            }

            // lr
            z = eval(r_r, d_theta);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                lr_def = true;
                lr = glm::vec3(r_r * cosf(d_theta), r_r * sinf(d_theta), z);
            }

            // d
            z = eval(radius, d_theta);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                d_def = true;
                d = glm::vec3(radius * cosf(d_theta), radius * sinf(d_theta), z);
            }

            // ll
            z = eval(l_r, d_theta);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                ll_def = true;
                ll = glm::vec3(l_r * cosf(d_theta), l_r * sinf(d_theta), z);
            }

            // l
            z = eval(l_r, theta);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                l_def = true;
                l = glm::vec3(l_r * cosf(theta), l_r * sinf(theta), z);
            }

            normals[theta_i * _r_res + r_i] = get_normal(coords[theta_i * _r_res + r_i],
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

    build_graph_geometry(_theta_res, _r_res, coords, tex_coords, normals, defined);

    // initialize cursor
    cursor_r =  (_r_max - _r_min) / 2.0 + _r_min;
    cursor_theta =  (_theta_max - _theta_min) / 2.0 + _theta_min;
    _cursor_pos.x = cursor_r * cosf(cursor_theta);
    _cursor_pos.y = cursor_r * sinf(cursor_theta);
    _cursor_pos.z = eval(cursor_r, cursor_theta);
    _cursor_defined = std::fpclassify(_cursor_pos.z) == FP_NORMAL || std::fpclassify(_cursor_pos.z) == FP_ZERO;
    _signal_cursor_moved.emit();
}

// TODO: revist cursor movement - current setup doesn't work with cyl/sphere/parametric coords
void Graph_cylindrical::move_cursor(const Cursor_dir dir)
{
    switch(dir)
    {
    case UP:
        break;
    case DOWN:
        break;
    case LEFT:
        break;
    case RIGHT:
        break;
    default:
        break;
    }

    _signal_cursor_moved.emit();
}

glm::vec3 Graph_cylindrical::cursor_pos() const
{
    return _cursor_pos;
}

bool Graph_cylindrical::cursor_defined() const
{
    return _cursor_defined;
}

std::string Graph_cylindrical::cursor_text() const
{
    std::ostringstream str;
    str<<"Z(X: "<<_cursor_pos.x<<", Y: "<<_cursor_pos.y<<") = "<<_cursor_pos.z;
    return str.str();
}
