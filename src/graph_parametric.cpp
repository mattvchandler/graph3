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

#include <utility>

#include "graph_parametric.hpp"

Graph_parametric::Graph_parametric(const std::string & eqn,
    const std::string & u_min, const std::string & u_max, size_t u_res,
    const std::string & v_min, const std::string & v_max, size_t v_res):
    Graph(eqn), _u(0.0), _v(0.0), _u_res(u_res),_v_res(v_res)
{
    // TODO: error checks
    _p.SetExpr(u_min);
    double min = _p.Eval();
    _p.SetExpr(u_max);
    double max = _p.Eval();

    _u_min = std::min(min, max);
    _u_max = std::max(min, max);

    _p.SetExpr(v_min);
    min = _p.Eval();
    _p.SetExpr(v_max);
    max = _p.Eval();

    _v_min = std::min(min, max);
    _v_max = std::max(min, max);

    _p.DefineVar("u", &_u);
    _p.DefineVar("v", &_v);
    _p.SetExpr(eqn);

    build_graph();
}

glm::vec3 Graph_parametric::eval(const double u, const double v)
{
    _u = u; _v = v;
    glm::vec3 result(0.0f);
    int num_eqns;

    mu::value_type * result_v = _p.Eval(num_eqns);
    if(num_eqns != 3)
    {
        _p.Error(mu::EErrorCodes::ecUNEXPECTED_EOF, _eqn.size()-1, _eqn);
    }

    result.x = result_v[0];
    result.y = result_v[1];
    result.z = result_v[2];
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
            tex_coords[v_i * _u_res + u_i] = glm::vec2((float)((u - _u_min) / (_u_max - _u_min)), (float)((_v_max - v) / (_v_max - _v_min)));
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
    _signal_cursor_moved.emit(cursor_text());
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

    _signal_cursor_moved.emit(cursor_text());
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
    std::string eqn = _eqn;

    if(_eqn.size() > 20)
        eqn = _eqn.substr(0, 19) + "…";

    str<<"(x, y, z)(u ,v) = "<<eqn<<" (x, y, z)("<<_cursor_u<<", "<<_cursor_v<<") = (";
    str<<_cursor_pos.x<<", "<<_cursor_pos.y<<", "<<_cursor_pos.z<<")"<<std::endl;
    return str.str();
}
