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

#include "graph_cartesian.h"

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

            coords[y_i * _x_res + x_i] = glm::vec3((float)x, (float)y, (float)z);
            tex_coords[y_i * _x_res + x_i] = glm::vec2((float)((x - _x_min) / (_x_max - _x_min)), (float)((_y_max - y) / (_y_max - _y_min)));
            defined[y_i * _x_res + x_i] = true;

            // calculate surrounding points for normal calculation
            glm::vec3 u, d, l, r, ul, ur, ll, lr;
            bool u_def = false, d_def = false, l_def = false, r_def = false,
                 ul_def = false, ur_def = false, ll_def = false, lr_def = false;

            float l_x = x - h_x;
            float r_x = x + h_x;
            float u_y = y + h_y;
            float d_y = y - h_y;

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

    // TODO: display normals

    std::vector<GLuint> index;

    bool break_flag = true;

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
        int ul = y_i * _x_res + _x_res - 1;
        int ll = (y_i + 1) * _x_res + _x_res - 1;

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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * index.size(), &index[0], GL_STATIC_DRAW);

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
    std::vector<GLuint> grid_index;

    // horizontal pass
    for(int i = 1; i < 10; ++i)
    {
        for(int x_i = 0; x_i < _x_res; ++x_i)
        {
            GLuint ind = (int)((float)_y_res * (float)i / 10.0f) * _x_res + x_i;
            if(defined[ind])
                grid_index.push_back(ind);
            else
                grid_index.push_back(0xFFFFFFFF);
        }
        grid_index.push_back(0xFFFFFFFF);
    }

    //vertical pass
    for(int i = 1; i < 10; ++i)
    {
        for(int y_i = 0; y_i < _y_res; ++y_i)
        {
            GLuint ind = y_i * _x_res + (int)((float)_x_res * (float)i / 10.0f);
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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * grid_index.size(), &grid_index[0], GL_STATIC_DRAW);

    _grid_num_indexes = grid_index.size();

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
