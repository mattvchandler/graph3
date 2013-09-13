// graph_parametric.cpp
// parametric coordinate system graph class (x(up,v), y(up,v), z(up,v))

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

#include "graph_parametric.hpp"

Graph_parametric::Graph_parametric(const std::string & eqn, float u_min, float u_max, int u_res,
    float v_min, float v_max, int v_res): Graph(eqn),
    _u_min(u_min), _u_max(u_max), _u_res(u_res), _v_min(v_min), _v_max(v_max), _v_res(v_res)

{
    _p.DefineVar("u", &_u);
    _p.DefineVar("v", &_v);
    _p.SetExpr(eqn);

    build_graph();
}

glm::vec3 Graph_parametric::eval(const double u, const double v)
{
    _u = u; _v = v;
    glm::vec3 result(0.0f);
    try
    {
        int num_eqns;
        mu::value_type * v = _p.Eval(num_eqns); 
        if(num_eqns != 3)
        {
            _p.Error(mu::EErrorCodes::ecUNEXPECTED_EOF, _eqn.size()-1, _eqn);
        }
        result.x = v[0];
        result.y = v[1];
        result.z = v[2];
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
void Graph_parametric::build_graph()
{
    std::vector<glm::vec3> coords(_v_res * _u_res);
    std::vector<glm::vec2> tex_coords(_v_res * _u_res);
    std::vector<glm::vec3> normals(_v_res * _u_res);
    std::vector<bool> defined(_v_res * _u_res);

    float h_u = 1e-3f * (_u_max - _u_min) / (float)_u_res;
    float h_v = 1e-3f * (_v_max - _v_min) / (float)_v_res;

    // calculate coords, texture cords, and normals
    double v = _v_max;
    for(size_t v_i = 0; v_i < _v_res; ++v_i, v -= (_v_max - _v_min) / (double)(_v_res - 1))
    {
        double u = _u_min;
        for(size_t u_i = 0; u_i < _u_res; ++u_i,  u += (_u_max - _u_min) / (double)(_u_res - 1))
        {
            glm::vec3 pos = eval(u, v);

            if((std::fpclassify(pos.x) != FP_NORMAL &&
                std::fpclassify(pos.x) != FP_ZERO) ||
                (std::fpclassify(pos.y) != FP_NORMAL &&
                std::fpclassify(pos.y) != FP_ZERO) ||
                (std::fpclassify(pos.z) != FP_NORMAL &&
                std::fpclassify(pos.z) != FP_ZERO))
            {
                coords[v_i * _u_res + u_i] = glm::vec3(0.0f);
                tex_coords[v_i * _u_res + u_i] = glm::vec2(0.0f);
                normals[v_i * _u_res + u_i] = glm::vec3(0.0f, 0.0f, 1.0f);
                defined[v_i * _u_res + u_i] = false;
                continue;
            }

            coords[v_i * _u_res + u_i] = pos;
            tex_coords[v_i * _u_res + u_i] = glm::vec2((float)((pos.x - _u_min) / (_u_max - _u_min)), (float)((_v_max - pos.y) / (_v_max - _v_min)));
            defined[v_i * _u_res + u_i] = true;

            // calculate surrounding points for normal calculation
            glm::vec3 up, dn, lf, rt, ul, ur, ll, lr;
            bool up_def = false, dn_def = false, lf_def = false, rt_def = false,
                 ul_def = false, ur_def = false, ll_def = false, lr_def = false;

            float l_u = (float)u - h_u;
            float r_u = (float)u + h_u;
            float u_v = (float)v + h_v;
            float d_v = (float)v - h_v;

            // ul
            pos = eval(l_u, u_v);
            if((std::fpclassify(pos.x) == FP_NORMAL ||
                std::fpclassify(pos.x) == FP_ZERO) &&
                (std::fpclassify(pos.y) == FP_NORMAL ||
                std::fpclassify(pos.y) == FP_ZERO) &&
                (std::fpclassify(pos.z) == FP_NORMAL ||
                std::fpclassify(pos.z) == FP_ZERO))
            {
                ul_def = true;
                ul = pos;
            }

            // up
            pos = eval(u, u_v);
            if((std::fpclassify(pos.x) == FP_NORMAL ||
                std::fpclassify(pos.x) == FP_ZERO) &&
                (std::fpclassify(pos.y) == FP_NORMAL ||
                std::fpclassify(pos.y) == FP_ZERO) &&
                (std::fpclassify(pos.z) == FP_NORMAL ||
                std::fpclassify(pos.z) == FP_ZERO))
            {
                up_def = true;
                up = pos;
            }

            // ur
            pos = eval(r_u, u_v);
            if((std::fpclassify(pos.x) == FP_NORMAL ||
                std::fpclassify(pos.x) == FP_ZERO) &&
                (std::fpclassify(pos.y) == FP_NORMAL ||
                std::fpclassify(pos.y) == FP_ZERO) &&
                (std::fpclassify(pos.z) == FP_NORMAL ||
                std::fpclassify(pos.z) == FP_ZERO))
            {
                ur_def = true;
                ur = pos;
            }

            // rt
            pos = eval(r_u, v);
            if((std::fpclassify(pos.x) == FP_NORMAL ||
                std::fpclassify(pos.x) == FP_ZERO) &&
                (std::fpclassify(pos.y) == FP_NORMAL ||
                std::fpclassify(pos.y) == FP_ZERO) &&
                (std::fpclassify(pos.z) == FP_NORMAL ||
                std::fpclassify(pos.z) == FP_ZERO))
            {
                rt_def = true;
                rt = pos;
            }

            // lr
            pos = eval(r_u, d_v);
            if((std::fpclassify(pos.x) == FP_NORMAL ||
                std::fpclassify(pos.x) == FP_ZERO) &&
                (std::fpclassify(pos.y) == FP_NORMAL ||
                std::fpclassify(pos.y) == FP_ZERO) &&
                (std::fpclassify(pos.z) == FP_NORMAL ||
                std::fpclassify(pos.z) == FP_ZERO))
            {
                lr_def = true;
                lr = pos;
            }

            // dn
            pos = eval(u, d_v);
            if((std::fpclassify(pos.x) == FP_NORMAL ||
                std::fpclassify(pos.x) == FP_ZERO) &&
                (std::fpclassify(pos.y) == FP_NORMAL ||
                std::fpclassify(pos.y) == FP_ZERO) &&
                (std::fpclassify(pos.z) == FP_NORMAL ||
                std::fpclassify(pos.z) == FP_ZERO))
            {
                dn_def = true;
                dn = pos;
            }

            // ll
            pos = eval(l_u, d_v);
            if((std::fpclassify(pos.x) == FP_NORMAL ||
                std::fpclassify(pos.x) == FP_ZERO) &&
                (std::fpclassify(pos.y) == FP_NORMAL ||
                std::fpclassify(pos.y) == FP_ZERO) &&
                (std::fpclassify(pos.z) == FP_NORMAL ||
                std::fpclassify(pos.z) == FP_ZERO))
            {
                ll_def = true;
                ll = pos;
            }

            // lf
            pos = eval(l_u, v);
            if((std::fpclassify(pos.x) == FP_NORMAL ||
                std::fpclassify(pos.x) == FP_ZERO) &&
                (std::fpclassify(pos.y) == FP_NORMAL ||
                std::fpclassify(pos.y) == FP_ZERO) &&
                (std::fpclassify(pos.z) == FP_NORMAL ||
                std::fpclassify(pos.z) == FP_ZERO))
            {
                lf_def = true;
                lf = pos;
            }

            normals[v_i * _u_res + u_i] = get_normal(coords[v_i * _u_res + u_i],
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

    build_graph_geometry(_v_res, _u_res, coords, tex_coords, normals, defined);

    // initialize cursor
    _cursor_u = (_u_max - _u_min) / 2.0 + _u_min;
    _cursor_v = (_v_max - _v_min) / 2.0 + _v_min;
    _cursor_pos = eval(_cursor_u, _cursor_v);
    _cursor_defined = (std::fpclassify(_cursor_pos.x) == FP_NORMAL || std::fpclassify(_cursor_pos.x) == FP_ZERO) &&
        (std::fpclassify(_cursor_pos.y) == FP_NORMAL || std::fpclassify(_cursor_pos.y) == FP_ZERO) &&
        (std::fpclassify(_cursor_pos.z) == FP_NORMAL || std::fpclassify(_cursor_pos.z) == FP_ZERO);
    _signal_cursor_moved.emit();
}

void Graph_parametric::move_cursor(const Cursor_dir dir)
{
    switch(dir)
    {
    case UP:
        _cursor_v += (_v_max - _v_min) / (double)_v_res;
        if(_cursor_v > _v_max)
            _cursor_v -= _v_max - _v_min;
        break;
    case DOWN:
        _cursor_v -= (_v_max - _v_min) / (double)_v_res;
        if(_cursor_v < _v_min)
            _cursor_v += _v_max - _v_min;
        break;
    case LEFT:
        _cursor_u -= (_u_max - _u_min) / (double)_u_res;
        if(_cursor_u < _u_min)
            _cursor_u += _u_max - _u_min;
        break;
    case RIGHT:
        _cursor_u += (_u_max - _u_min) / (double)_u_res;
        if(_cursor_u > _u_max)
            _cursor_u -= _u_max - _u_min;
        break;
    default:
        break;
    }

    _cursor_pos = eval(_cursor_u, _cursor_v);
    _cursor_defined = (std::fpclassify(_cursor_pos.x) == FP_NORMAL || std::fpclassify(_cursor_pos.x) == FP_ZERO) &&
        (std::fpclassify(_cursor_pos.y) == FP_NORMAL || std::fpclassify(_cursor_pos.y) == FP_ZERO) &&
        (std::fpclassify(_cursor_pos.z) == FP_NORMAL || std::fpclassify(_cursor_pos.z) == FP_ZERO);

    _signal_cursor_moved.emit();
}

glm::vec3 Graph_parametric::cursor_pos() const
{
    return _cursor_pos;
}

bool Graph_parametric::cursor_defined() const
{
    return _cursor_defined;
}

std::string Graph_parametric::cursor_text() const
{
    std::ostringstream str;
    str<<"x(u: "<<_cursor_u<<", v: "<<_cursor_v<<") = "<<_cursor_pos.x<<", ";
    str<<"y(u: "<<_cursor_u<<", v: "<<_cursor_v<<") = "<<_cursor_pos.y<<", ";
    str<<"z(u: "<<_cursor_u<<", v: "<<_cursor_v<<") = "<<_cursor_pos.z<<std::endl;
    return str.str();
}
