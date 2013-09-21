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

#include "graph_cylindrical.hpp"

Graph_cylindrical::Graph_cylindrical(const std::string & eqn,
    const std::string & r_min, const std::string & r_max, int r_res,
    const std::string & theta_min, const std::string & theta_max, int theta_res):
    Graph(eqn), _r(0.0), _theta(0.0), _r_res(r_res), _theta_res(theta_res)

{
    // TODO: error checks
    _p.SetExpr(r_min);
    _r_min = _p.Eval();
    _p.SetExpr(r_max);
    _r_max = _p.Eval();

    _p.SetExpr(theta_min);
    _theta_min = _p.Eval();
    _p.SetExpr(theta_max);
    _theta_max = _p.Eval();

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
        double r = _r_min;
        for(size_t r_i = 0; r_i < _r_res; ++r_i,  r += (_r_max - _r_min) / (double)(_r_res - 1))
        {
            double z = eval(r, theta);

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
            coords[theta_i * _r_res + r_i] = glm::vec3((float)r * cosf(theta), (float)r * sinf(theta), (float)z);
            tex_coords[theta_i * _r_res + r_i] = glm::vec2((coords[theta_i * _r_res + r_i].x + _r_max) / (float)(2 * _r_max),
                (_r_max - coords[theta_i * _r_res + r_i].y) / (float)(2 * _r_max));
            defined[theta_i * _r_res + r_i] = true;

            // calculate surrounding points for normal calculation
            glm::vec3 up, dn, lf, rt, ul, ur, ll, lr;
            bool up_def = false, dn_def = false, lf_def = false, rt_def = false,
                 ul_def = false, ur_def = false, ll_def = false, lr_def = false;

            float l_r = (float)r - h_r;
            float r_r = (float)r + h_r;
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

            // up
            z = eval(r, u_theta);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                up_def = true;
                up = glm::vec3(r * cosf(u_theta), r * sinf(u_theta), z);
            }

            // ur
            z = eval(r_r, u_theta);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                ur_def = true;
                ur = glm::vec3(r_r * cosf(u_theta), r_r * sinf(u_theta), z);
            }

            // rt
            z = eval(r_r, theta);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                rt_def = true;
                rt = glm::vec3(r_r * cosf(theta), r_r * sinf(theta), z);
            }

            // lr
            z = eval(r_r, d_theta);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                lr_def = true;
                lr = glm::vec3(r_r * cosf(d_theta), r_r * sinf(d_theta), z);
            }

            // dn
            z = eval(r, d_theta);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                dn_def = true;
                dn = glm::vec3(r * cosf(d_theta), r * sinf(d_theta), z);
            }

            // ll
            z = eval(l_r, d_theta);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                ll_def = true;
                ll = glm::vec3(l_r * cosf(d_theta), l_r * sinf(d_theta), z);
            }

            // lf
            z = eval(l_r, theta);
            if(std::fpclassify(z) == FP_NORMAL ||
                std::fpclassify(z) == FP_ZERO)
            {
                lf_def = true;
                lf = glm::vec3(l_r * cosf(theta), l_r * sinf(theta), z);
            }

            normals[theta_i * _r_res + r_i] = get_normal(coords[theta_i * _r_res + r_i],
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

    build_graph_geometry(_theta_res, _r_res, coords, tex_coords, normals, defined);

    // initialize cursor
    _cursor_r =  (_r_max - _r_min) / 2.0 + _r_min;
    _cursor_theta =  (_theta_max - _theta_min) / 2.0 + _theta_min;
    _cursor_pos.x = _cursor_r * cosf(_cursor_theta);
    _cursor_pos.y = _cursor_r * sinf(_cursor_theta);
    _cursor_pos.z = eval(_cursor_r, _cursor_theta);
    _cursor_defined = std::fpclassify(_cursor_pos.z) == FP_NORMAL || std::fpclassify(_cursor_pos.z) == FP_ZERO;
    _signal_cursor_moved.emit(cursor_text());
}

void Graph_cylindrical::move_cursor(const Cursor_dir dir)
{
    switch(dir)
    {
    case UP:
        _cursor_theta += (_theta_max - _theta_min) / (double)_theta_res;
        if(_cursor_theta > _theta_max)
            _cursor_theta -= _theta_max - _theta_min;
        break;
    case DOWN:
        _cursor_theta -= (_theta_max - _theta_min) / (double)_theta_res;
        if(_cursor_theta < _theta_min)
            _cursor_theta += _theta_max - _theta_min;
        break;
    case LEFT:
        _cursor_r -= (_r_max - _r_min) / (double)_r_res;
        if(_cursor_r > _r_max)
            _cursor_r += _r_max - _r_min;
        break;
    case RIGHT:
        _cursor_r += (_r_max - _r_min) / (double)_r_res;
        if(_cursor_r > _r_max)
            _cursor_r -= _r_max - _r_min;
        break;
    default:
        break;
    }

    _cursor_pos.x = _cursor_r * cosf(_cursor_theta);
    _cursor_pos.y = _cursor_r * sinf(_cursor_theta);
    _cursor_pos.z = eval(_cursor_r, _cursor_theta);
    _cursor_defined = std::fpclassify(_cursor_pos.z) == FP_NORMAL || std::fpclassify(_cursor_pos.z) == FP_ZERO;
    _signal_cursor_moved.emit(cursor_text());
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
    std::string eqn = _eqn;

    if(_eqn.size() > 20)
        eqn = _eqn.substr(0, 19) + "…";

    str<<u8"z(r, θ)  = "<<eqn<<"; z("<<_cursor_r<<", "<<_cursor_theta<<") = "<<_cursor_pos.z;
    return str.str();
}
