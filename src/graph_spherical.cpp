// graph_spherical.cpp
// spherical coordinate system graph class (r(phi, theta))
// TODO: check order

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

#include "graph_spherical.h"

Graph_spherical::Graph_spherical(const std::string & eqn,
    float theta_min, float theta_max, int theta_res,
    float phi_min, float phi_max, int phi_res): Graph(eqn),
    _theta_min(theta_min), _theta_max(theta_max), _theta_res(theta_res),
    _phi_min(phi_min), _phi_max(phi_max), _phi_res(phi_res)

{
    _p.DefineVar("theta", &_theta);
    _p.DefineVar("phi", &_phi);
    _p.SetExpr(eqn);

    build_graph();
}

double Graph_spherical::eval(const double theta, const double phi)
{
    _theta = theta; _phi = phi;
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
void Graph_spherical::build_graph()
{
    std::vector<glm::vec3> coords(_theta_res * _phi_res);
    std::vector<glm::vec2> tex_coords(_theta_res * _phi_res);
    std::vector<glm::vec3> normals(_theta_res * _phi_res);
    std::vector<bool> defined(_theta_res * _phi_res);

    float h_phi = 1e-3f * (_phi_max - _phi_min) / (float)_phi_res;
    float h_theta = 1e-3f * (_theta_max - _theta_min) / (float)_theta_res;

    // calculate coords, texture cords, and normals
    double phi = _phi_min;
    for(size_t phi_i = 0; phi_i < _phi_res; ++phi_i,  phi += (_phi_max - _phi_min) / (double)(_phi_res - 1))
    {
        double theta = _theta_max;
        for(size_t theta_i = 0; theta_i < _theta_res; ++theta_i, theta -= (_theta_max - _theta_min) / (double)(_theta_res - 1))
        {
            double rho = eval(theta, phi);

            if(std::fpclassify(rho) != FP_NORMAL &&
                std::fpclassify(rho) != FP_ZERO)
            {
                coords[phi_i * _theta_res + theta_i] = glm::vec3(0.0f);
                tex_coords[phi_i * _theta_res + theta_i] = glm::vec2(0.0f);
                normals[phi_i * _theta_res + theta_i] = glm::vec3(0.0f, 0.0f, 1.0f);
                defined[phi_i * _theta_res + theta_i] = false;
                continue;
            }

            // convert into cartesian coordinates
            coords[phi_i * _theta_res + theta_i] = glm::vec3((float)rho * sinf(phi) * cosf(theta),
                (float)rho * sinf(phi) * sinf(theta), (float)rho * cosf(phi));
            tex_coords[phi_i * _theta_res + theta_i] = glm::vec2(
                    (float)((theta - _theta_min) / (_theta_max - _theta_min)),
                    (float)((phi - _phi_min) / (_phi_max - _phi_min)));
            defined[phi_i * _theta_res + theta_i] = true;

            // calculate surrounding points for normal calculation
            glm::vec3 u, d, l, r, ul, ur, ll, lr;
            bool u_def = false, d_def = false, l_def = false, r_def = false,
                 ul_def = false, ur_def = false, ll_def = false, lr_def = false;

            float l_theta = (float)theta - h_theta;
            float r_theta = (float)theta + h_theta;
            float u_phi = (float)phi + h_phi;
            float d_phi = (float)phi - h_phi;

            // ul
            rho = eval(l_theta, u_phi);
            if(std::fpclassify(rho) == FP_NORMAL ||
                std::fpclassify(rho) == FP_ZERO)
            {
                ul_def = true;
                ul = glm::vec3(rho * sinf(u_phi) * cosf(l_theta), rho * sinf(u_phi) * sinf(l_theta), rho * cosf(u_phi));
            }

            // u
            rho = eval(theta, u_phi);
            if(std::fpclassify(rho) == FP_NORMAL ||
                std::fpclassify(rho) == FP_ZERO)
            {
                u_def = true;
                u = glm::vec3(rho * sinf(u_phi) * cosf(theta), rho * sinf(u_phi) * sinf(theta), rho * cosf(u_phi));
            }

            // ur
            rho = eval(r_theta, u_phi);
            if(std::fpclassify(rho) == FP_NORMAL ||
                std::fpclassify(rho) == FP_ZERO)
            {
                ur_def = true;
                ur = glm::vec3(rho * sinf(u_phi) * cosf(r_theta), rho * sinf(u_phi) * sinf(r_theta), rho * cosf(u_phi));
            }

            // r
            rho = eval(r_theta, phi);
            if(std::fpclassify(rho) == FP_NORMAL ||
                std::fpclassify(rho) == FP_ZERO)
            {
                r_def = true;
                r = glm::vec3(rho * sinf(phi) * cosf(r_theta), rho * sinf(phi) * sinf(r_theta), rho * cosf(phi));
            }

            // lr
            rho = eval(r_theta, d_phi);
            if(std::fpclassify(rho) == FP_NORMAL ||
                std::fpclassify(rho) == FP_ZERO)
            {
                lr_def = true;
                lr = glm::vec3(rho * sinf(d_phi) * cosf(r_theta), rho * sinf(d_phi) * sinf(r_theta), rho * cosf(d_phi));
            }

            // d
            rho = eval(theta, d_phi);
            if(std::fpclassify(rho) == FP_NORMAL ||
                std::fpclassify(rho) == FP_ZERO)
            {
                d_def = true;
                d = glm::vec3(rho * sinf(d_phi) * cosf(theta), rho * sinf(d_phi) * sinf(theta), rho * cosf(d_phi));
            }

            // ll
            rho = eval(l_theta, d_phi);
            if(std::fpclassify(rho) == FP_NORMAL ||
                std::fpclassify(rho) == FP_ZERO)
            {
                ll_def = true;
                ll = glm::vec3(rho * sinf(d_phi) * cosf(l_theta), rho * sinf(d_phi) * sinf(l_theta), rho * cosf(d_phi));
            }

            // l
            rho = eval(l_theta, phi);
            if(std::fpclassify(rho) == FP_NORMAL ||
                std::fpclassify(rho) == FP_ZERO)
            {
                l_def = true;
                l = glm::vec3(rho * sinf(phi) * cosf(l_theta), rho * sinf(phi) * sinf(l_theta), rho * cosf(phi));
            }

            normals[phi_i * _theta_res + theta_i] = get_normal(coords[phi_i * _theta_res + theta_i],
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

    build_graph_geometry(_theta_res, _phi_res, coords, tex_coords, normals, defined);

    // initialize cursor
    _cursor_theta =  (_theta_max - _theta_min) / 2.0 + _theta_min;
    _cursor_phi =  (_phi_max - _phi_min) / 2.0 + _phi_min;
    _cursor_r = eval(_cursor_theta, _cursor_phi);
    _cursor_pos.x = _cursor_r * sinf(_cursor_phi) * cosf(_cursor_theta);
    _cursor_pos.y = _cursor_r * sinf(_cursor_phi) * sinf(_cursor_theta);
    _cursor_pos.z = _cursor_r * cosf(_cursor_phi);
    _cursor_defined = std::fpclassify(_cursor_r) == FP_NORMAL || std::fpclassify(_cursor_r) == FP_ZERO;
    _signal_cursor_moved.emit();
}

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

    _cursor_r = eval(_cursor_theta, _cursor_phi);
    _cursor_pos.x = _cursor_r * sinf(_cursor_phi) * cosf(_cursor_theta);
    _cursor_pos.y = _cursor_r * sinf(_cursor_phi) * sinf(_cursor_theta);
    _cursor_pos.z = _cursor_r * cosf(_cursor_phi);
    _cursor_defined = std::fpclassify(_cursor_r) == FP_NORMAL || std::fpclassify(_cursor_r) == FP_ZERO;
    _signal_cursor_moved.emit();
}

glm::vec3 Graph_spherical::cursor_pos() const
{
    return _cursor_pos;
}

bool Graph_spherical::cursor_defined() const
{
    return _cursor_defined;
}

std::string Graph_spherical::cursor_text() const
{
    std::ostringstream str;
    str<<"r( θ: "<<_cursor_theta<<", ϕ: "<<_cursor_phi<<") = "<<_cursor_r;
    return str.str();
}
