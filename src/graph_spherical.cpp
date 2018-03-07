// graph_spherical.cpp
// spherical coordinate system graph class (r(phi, theta))

// Copyright 2018 Matthew Chandler

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

#include "graph_spherical.hpp"

Graph_spherical::Graph_spherical(const std::string & eqn,
    const std::string & theta_min, const std::string & theta_max, size_t theta_res,
    const std::string & phi_min, const std::string & phi_max, size_t phi_res):
    _eqn(eqn), _theta(0.0), _phi(0.0), _theta_res(theta_res), _phi_res(phi_res),
    _cursor_theta(0.0f), _cursor_phi(0.0f), _cursor_r(0.0f), _cursor_defined(false)
{
    _p.DefineConst("pi", M_PI);
    _p.DefineConst("e", M_E);
    double min, max;

    // try to evaluate mins and maxes strings
    _p.SetExpr(theta_min);
    try
    {
        min = _p.Eval();
    }
    catch(const mu::Parser::exception_type & e)
    {
        Graph_exception ge(e, Graph_exception::ROW_MIN);
        throw ge;
    }

    _p.SetExpr(theta_max);
    try
    {
        max = _p.Eval();
    }
    catch(const mu::Parser::exception_type & e)
    {
        Graph_exception ge(e, Graph_exception::ROW_MAX);
        throw ge;
    }

    _theta_min = std::min(min, max);
    _theta_max = std::max(min, max);

    _p.SetExpr(phi_min);
    try
    {
        min = _p.Eval();
    }
    catch(const mu::Parser::exception_type & e)
    {
        Graph_exception ge(e, Graph_exception::COL_MIN);
        throw ge;
    }

    _p.SetExpr(phi_max);
    try
    {
        max = _p.Eval();
    }
    catch(const mu::Parser::exception_type & e)
    {
        Graph_exception ge(e, Graph_exception::COL_MAX);
        throw ge;
    }

    _phi_min = std::min(min, max);
    _phi_max = std::max(min, max);

    _p.DefineVar("theta", &_theta);
    _p.DefineVar("phi", &_phi);
    _p.SetExpr(eqn);

    build_graph();
}

// evaluate a point on the graph
double Graph_spherical::eval(const double theta, const double phi)
{
    _theta = theta; _phi = phi;
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

// calculate & build graph geometry
void Graph_spherical::build_graph()
{
    // OpenGL needs to be initialized before this is run, hence it's not in the ctor
    std::vector<glm::vec3> coords(_theta_res * _phi_res);
    std::vector<glm::vec2> tex_coords(_theta_res * _phi_res);
    std::vector<glm::vec3> normals(_theta_res * _phi_res);
    std::vector<bool> defined(_theta_res * _phi_res);

    // small offsets for calculating normals
    float h_phi = 1e-3f * (_phi_max - _phi_min) / (float)_phi_res;
    float h_theta = 1e-3f * (_theta_max - _theta_min) / (float)_theta_res;

    // calculate coords, texture cords, and normals
    double phi = _phi_min;
    for(size_t phi_i = 0; phi_i < _phi_res; ++phi_i,  phi += (_phi_max - _phi_min) / (double)(_phi_res - 1))
    {
        double theta = _theta_max;
        for(size_t theta_i = 0; theta_i < _theta_res; ++theta_i, theta -= (_theta_max - _theta_min) / (double)(_theta_res - 1))
        {
            double r = eval(theta, phi);

            // check for undefined / infinity
            if(std::fpclassify(r) != FP_NORMAL &&
                std::fpclassify(r) != FP_ZERO)
            {
                // fallback values
                coords[phi_i * _theta_res + theta_i] = glm::vec3(0.0f);
                tex_coords[phi_i * _theta_res + theta_i] = glm::vec2(0.0f);
                normals[phi_i * _theta_res + theta_i] = glm::vec3(0.0f, 0.0f, 1.0f);
                // set undefined
                defined[phi_i * _theta_res + theta_i] = false;
                continue;
            }

            // convert into cartesian coordinates
            // add vertex to lists
            coords[phi_i * _theta_res + theta_i] = glm::vec3((float)r * sinf(phi) * cosf(theta),
                (float)r * sinf(phi) * sinf(theta), (float)r * cosf(phi));
            tex_coords[phi_i * _theta_res + theta_i] = glm::vec2(
                    (float)((theta - _theta_min) / (_theta_max - _theta_min)),
                    (float)((phi - _phi_min) / (_phi_max - _phi_min)));
            defined[phi_i * _theta_res + theta_i] = true;

            // calculate surrounding points for normal calculation
            glm::vec3 up, dn, lf, rt, ul, ur, ll, lr;
            bool up_def = false, dn_def = false, lf_def = false, rt_def = false,
                 ul_def = false, ur_def = false, ll_def = false, lr_def = false;

            // add offsets
            float l_theta = (float)theta - h_theta;
            float r_theta = (float)theta + h_theta;
            float u_phi = (float)phi + h_phi;
            float d_phi = (float)phi - h_phi;

            // ul
            r = eval(l_theta, u_phi);
            if(std::fpclassify(r) == FP_NORMAL ||
                std::fpclassify(r) == FP_ZERO)
            {
                ul_def = true;
                ul = glm::vec3(r * sinf(u_phi) * cosf(l_theta), r * sinf(u_phi) * sinf(l_theta), r * cosf(u_phi));
            }

            // up
            r = eval(theta, u_phi);
            if(std::fpclassify(r) == FP_NORMAL ||
                std::fpclassify(r) == FP_ZERO)
            {
                up_def = true;
                up = glm::vec3(r * sinf(u_phi) * cosf(theta), r * sinf(u_phi) * sinf(theta), r * cosf(u_phi));
            }

            // ur
            r = eval(r_theta, u_phi);
            if(std::fpclassify(r) == FP_NORMAL ||
                std::fpclassify(r) == FP_ZERO)
            {
                ur_def = true;
                ur = glm::vec3(r * sinf(u_phi) * cosf(r_theta), r * sinf(u_phi) * sinf(r_theta), r * cosf(u_phi));
            }

            // rt
            r = eval(r_theta, phi);
            if(std::fpclassify(r) == FP_NORMAL ||
                std::fpclassify(r) == FP_ZERO)
            {
                rt_def = true;
                rt = glm::vec3(r * sinf(phi) * cosf(r_theta), r * sinf(phi) * sinf(r_theta), r * cosf(phi));
            }

            // lr
            r = eval(r_theta, d_phi);
            if(std::fpclassify(r) == FP_NORMAL ||
                std::fpclassify(r) == FP_ZERO)
            {
                lr_def = true;
                lr = glm::vec3(r * sinf(d_phi) * cosf(r_theta), r * sinf(d_phi) * sinf(r_theta), r * cosf(d_phi));
            }

            // dn
            r = eval(theta, d_phi);
            if(std::fpclassify(r) == FP_NORMAL ||
                std::fpclassify(r) == FP_ZERO)
            {
                dn_def = true;
                dn = glm::vec3(r * sinf(d_phi) * cosf(theta), r * sinf(d_phi) * sinf(theta), r * cosf(d_phi));
            }

            // ll
            r = eval(l_theta, d_phi);
            if(std::fpclassify(r) == FP_NORMAL ||
                std::fpclassify(r) == FP_ZERO)
            {
                ll_def = true;
                ll = glm::vec3(r * sinf(d_phi) * cosf(l_theta), r * sinf(d_phi) * sinf(l_theta), r * cosf(d_phi));
            }

            // lf
            r = eval(l_theta, phi);
            if(std::fpclassify(r) == FP_NORMAL ||
                std::fpclassify(r) == FP_ZERO)
            {
                lf_def = true;
                lf = glm::vec3(r * sinf(phi) * cosf(l_theta), r * sinf(phi) * sinf(l_theta), r * cosf(phi));
            }

            // get normal
            normals[phi_i * _theta_res + theta_i] = get_normal(coords[phi_i * _theta_res + theta_i],
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

    // build OpenGL geometry data from vertexes
    build_graph_geometry(_theta_res, _phi_res, coords, tex_coords, normals, defined);

    // initialize cursor
    _cursor_theta =  (_theta_max - _theta_min) / 2.0 + _theta_min;
    _cursor_phi =  (_phi_max - _phi_min) / 2.0 + _phi_min;
    _cursor_r = eval(_cursor_theta, _cursor_phi);
    _cursor_pos.x = _cursor_r * sinf(_cursor_phi) * cosf(_cursor_theta);
    _cursor_pos.y = _cursor_r * sinf(_cursor_phi) * sinf(_cursor_theta);
    _cursor_pos.z = _cursor_r * cosf(_cursor_phi);
    _cursor_defined = std::fpclassify(_cursor_r) == FP_NORMAL || std::fpclassify(_cursor_r) == FP_ZERO;
    _signal_cursor_moved.emit(cursor_text());
}

// cursor funcs
void Graph_spherical::move_cursor(const Cursor_dir dir)
{
    switch(dir)
    {
    case UP:
        _cursor_phi -= (_phi_max - _phi_min) / (double)_phi_res;
        if(_cursor_phi > _phi_max)
            _cursor_phi += _phi_max - _phi_min;
        break;
    case DOWN:
        _cursor_phi += (_phi_max - _phi_min) / (double)_phi_res;
        if(_cursor_phi > _phi_max)
            _cursor_phi -= _phi_max - _phi_min;
        break;
    case LEFT:
        _cursor_theta -= (_theta_max - _theta_min) / (double)_theta_res;
        if(_cursor_theta < _theta_min)
            _cursor_theta += _theta_max - _theta_min;
        break;
    case RIGHT:
        _cursor_theta += (_theta_max - _theta_min) / (double)_theta_res;
        if(_cursor_theta > _theta_max)
            _cursor_theta -= _theta_max - _theta_min;
        break;
    default:
        break;
    }

    // evaluate cursors new position
    _cursor_r = eval(_cursor_theta, _cursor_phi);
    _cursor_pos.x = _cursor_r * sinf(_cursor_phi) * cosf(_cursor_theta);
    _cursor_pos.y = _cursor_r * sinf(_cursor_phi) * sinf(_cursor_theta);
    _cursor_pos.z = _cursor_r * cosf(_cursor_phi);
    _cursor_defined = std::fpclassify(_cursor_r) == FP_NORMAL || std::fpclassify(_cursor_r) == FP_ZERO;

    // signal the move
    _signal_cursor_moved.emit(cursor_text());
}

glm::vec3 Graph_spherical::cursor_pos() const
{
    return _cursor_pos;
}

bool Graph_spherical::cursor_defined() const
{
    return _cursor_defined;
}

// return cursor position as a string
std::string Graph_spherical::cursor_text() const
{
    std::ostringstream str;
    std::string eqn = _eqn;

    // limit to 50 chars
    if(_eqn.size() > 50)
        eqn = _eqn.substr(0, 49) + "…";

    str<<std::setprecision(4)<<u8"r(θ, ϕ) = "<<eqn<<"; r("<<_cursor_theta<<", "<<_cursor_phi<<") = "<<_cursor_r;
    return str.str();
}
