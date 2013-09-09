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

    // coordinate pass
    double theta = _theta_max;
    for(int theta_i = 0; theta_i < _theta_res; ++theta_i, theta -= (_theta_max - _theta_min) / (double)(_theta_res - 1))
    {
        double r = _r_min;
        for(int r_i = 0; r_i < _r_res; ++r_i,  r += (_r_max - _r_min) / (double)(_r_res - 1))
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
            tex_coords[theta_i * _r_res + r_i] = glm::vec2((float)((r - _r_min) / (_r_max - _r_min)), (float)((_theta_max - theta) / (_theta_max - _theta_min)));
            defined[theta_i * _r_res + r_i] = true;
        }
    }

    // TODO: look into calculating partial derivatatives to find normal
    // TODO: lol, nope. but maybe get nearby adjacent points, ie: z(x+h, y+h) instead of next point on grid
    // normal pass
    for(int theta_i = 0; theta_i < _theta_res; ++theta_i)
    {
        for(int r_i = 0; r_i < _r_res; ++r_i)
        {
            if(!defined[theta_i * _r_res + r_i])
                continue;

            // get / calculate coords of surrounding verts
            glm::vec3 u, d, l, r;
            glm::vec3 ul, ur, ll, lr;
            bool u_def = false, d_def = false, l_def = false, r_def = false;
            bool ul_def = false, ur_def = false, ll_def = false, lr_def = false;

            double l_x, r_x, u_y, d_y, x, y, z;

            x = coords[theta_i * _r_res + r_i].x;
            y = coords[theta_i * _r_res + r_i].y;

            if(r_i == 0)
                l_x = _r_min - (_r_max - _r_min) / (double)_r_res;
            else
                l_x = coords[theta_i * _r_res + r_i - 1].x;

            if(r_i == _r_res - 1)
                r_x = _r_max + (_r_max - _r_min) / (double)_r_res;
            else
                r_x = coords[theta_i * _r_res + r_i + 1].x;

            if(theta_i == 0)
                u_y = _theta_max + (_theta_max - _theta_min) / (double)_theta_res;
            else
                u_y = coords[(theta_i - 1) * _r_res + r_i].y;

            if(theta_i == _theta_res - 1)
                d_y = _theta_min - (_theta_max - _theta_min) / (double)_theta_res;
            else
                d_y = coords[(theta_i + 1) * _r_res + r_i].y;

            // ul
            if(r_i == 0 || theta_i == 0)
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
                ul_def = defined[(theta_i - 1) * _r_res + r_i - 1];
                ul = coords[(theta_i - 1) * _r_res + r_i - 1];
            }

            // u
            if(theta_i == 0)
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
                u_def = defined[(theta_i - 1) * _r_res + r_i];
                u = coords[(theta_i - 1) * _r_res + r_i];
            }

            // ur
            if(r_i == _r_res - 1 || theta_i == 0)
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
                ur_def = defined[(theta_i - 1) * _r_res + r_i + 1];
                ur = coords[(theta_i - 1) * _r_res + r_i + 1];
            }

            // r
            if(r_i == _r_res - 1)
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
                r_def = defined[theta_i * _r_res + r_i + 1];
                r = coords[theta_i * _r_res + r_i + 1];
            }

            // lr
            if(r_i == _r_res - 1 || theta_i == _theta_res - 1)
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
                lr_def = defined[(theta_i + 1) * _r_res + r_i + 1];
                lr = coords[(theta_i + 1) * _r_res + r_i + 1];
            }

            // d
            if(theta_i == _theta_res - 1)
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
                d_def = defined[(theta_i + 1) * _r_res + r_i];
                d = coords[(theta_i + 1) * _r_res + r_i];
            }

            // ll
            if(r_i == 0 || theta_i == _theta_res - 1)
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
                ll_def = defined[(theta_i + 1) * _r_res + r_i - 1];
                ll = coords[(theta_i + 1) * _r_res + r_i - 1];
            }

            // l
            if(r_i == 0)
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
                l_def = defined[theta_i * _r_res + r_i - 1];
                l = coords[theta_i * _r_res + r_i - 1];
            }

            std::vector<glm::vec3> surrounding;

            glm::vec3 center = coords[theta_i * _r_res + r_i];

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
                normals[theta_i * _r_res + r_i] += i;

            normals[theta_i * _r_res + r_i] = glm::normalize(normals[theta_i * _r_res + r_i]);
        }
    }

    std::vector<GLuint> index;

    bool break_flag = true;

    // arrange verts as a triangle strip
    for(int theta_i = 0; theta_i < _theta_res - 1; ++theta_i)
    {
        for(int r_i = 0; r_i < _r_res - 1; ++r_i)
        {
            int ul = theta_i * _r_res + r_i;
            int ur = theta_i * _r_res + r_i + 1;
            int ll = (theta_i + 1) * _r_res + r_i;
            int lr = (theta_i + 1) * _r_res + r_i + 1;

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
        int ul = theta_i * _r_res + _r_res - 1;
        int ll = (theta_i + 1) * _r_res + _r_res - 1;

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
        for(int r_i = 0; r_i < _r_res; ++r_i)
        {
            GLuint ind = (int)((float)_theta_res * (float)i / 10.0f) * _r_res + r_i;
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
        for(int theta_i = 0; theta_i < _theta_res; ++theta_i)
        {
            GLuint ind = theta_i * _r_res + (int)((float)_r_res * (float)i / 10.0f);
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
    double cursor_r =  (_r_max - _r_min) / 2.0 + _r_min;
    double cursor_theta =  (_theta_max - _theta_min) / 2.0 + _theta_min;
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
