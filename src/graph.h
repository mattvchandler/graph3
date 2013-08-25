// graph.h
//

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

#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <iostream>
#include <string>
#include <vector>

#include <cmath>

#include <GL/glew.h>

#include <SFML/OpenGL.hpp>

#include <glm/glm.hpp>

#include <muParser.h>

#ifndef M_PI
#define M_PI 3.141592654
#endif

#ifndef M_E
#define M_E 2.718281828
#endif

// TODO: remove debug funcs

inline std::ostream & operator<<(std::ostream & out, const glm::vec2 & v)
{
    out<<"("<<v.x<<", "<<v.y<<")";
    return out;
}

inline std::ostream & operator<<(std::ostream & out, const glm::vec3 & v)
{
    out<<"("<<v.x<<", "<<v.y<<", "<<v.z<<")";
    return out;
}

class Graph
{
public:
    Graph(const std::string & eqn = ""): _eqn(eqn)
    {
        std::cout<<"Base: "<<_eqn<<std::endl;
        _p.DefineConst("pi", M_PI);
        _p.DefineConst("e", M_E);
    }
    // remove as many of these as possible
    // probably best to make non-copyable
    // Graph(const Graph & a);
    // Graph(Graph && a);
    ~Graph()
    {

    }
    // Graph & operator =(const Graph & a);
    // Graph & operator =(Graph && a);

    virtual double eval(const double, const double) = 0;
    virtual void build_graph() = 0;

protected:
    std::string _eqn;
    mu::Parser _p;
    GLuint _vao, _wire_vao;
    GLuint _tex;
    // TODO: maybe have each have its own shader program?
    enum {vert_coords = 0, tet_coords = 1, normals = 2};
};

class Graph_cartesian final: public Graph
{
public:
    Graph_cartesian(const std::string & eqn = ""): Graph(eqn)
    {
        std::cout<<"Derived: "<<_eqn<<std::endl;
        _p.DefineVar("x", &_x);
        _p.DefineVar("y", &_y);
        _p.SetExpr(eqn);

        // TODO: remove debug stuff
        _x_min = _y_min = -1.0;
        _x_max = _y_max = 1.0;
        _x_res = _y_res = 3;
    }

    double eval(const double x, const double y) override
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
            throw e;
        }
        return result;
    }

    void build_graph() override
    {
        std::vector<glm::vec3> coords(_y_res * _x_res);
        std::vector<glm::vec2> tex_coords(_y_res * _x_res);
        std::vector<glm::vec3> normals(_y_res * _x_res);
        std::vector<bool> defined(_y_res * _x_res);

        // coordinate pass
        double y = _y_min;
        for(int y_i = 0; y_i < _y_res; ++y_i, y += (_y_max - _y_min) / (double)(_y_res - 1))
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
                    normals[y_i * _x_res + x_i] = glm::vec3(0.0f, 1.0f, 0.0f);
                    defined[y_i * _x_res + x_i] = false;
                    continue;
                }

                // re-arranged into OpenGL's coordinate system
                coords[y_i * _x_res + x_i] = glm::vec3((float)y, (float)z, (float)x);
                tex_coords[y_i * _x_res + x_i] = glm::vec2((float)((y - _y_min) / (_y_max - _y_min)), (float)((x - _x_min) / (_x_max - _x_min)));
                defined[y_i * _x_res + x_i] = true;
            }
        }

        // normal pass
        for(int y_i = 0; y_i < _y_res; ++y_i)
        {
            for(int x_i = 0; x_i < _x_res; ++x_i)
            {
                if(!defined[y_i * _x_res + x_i])
                    continue;

                // get / calculate coords of surrounding verts
                glm::vec3 u, d, l, r;
                glm::vec3 ul, ur, ll, lr;
                bool u_def = false, d_def = false, l_def = false, r_def = false;
                bool ul_def = false, ur_def = false, ll_def = false, lr_def = false;

                double l_x, r_x, u_y, d_y, x, y, z;

                x = coords[y_i * _x_res + x_i].z;
                y = coords[y_i * _x_res + x_i].x;

                if(x_i == 0)
                    l_x = _x_min - (_x_max - _x_min) / (double)_x_res;
                else
                    l_x = coords[y_i * _x_res + x_i - 1].z;

                if(x_i == _x_res - 1)
                    r_x = _x_max + (_x_max - _x_min) / (double)_x_res;
                else
                    r_x = coords[y_i * _x_res + x_i + 1].z;

                if(y_i == 0)
                    u_y = _y_min - (_y_max - _y_min) / (double)_y_res;
                else
                    u_y = coords[(y_i - 1) * _x_res + x_i].x;

                if(y_i == _y_res - 1)
                    d_y = _y_max + (_y_max - _y_min) / (double)_y_res;
                else
                    d_y = coords[(y_i + 1) * _x_res + x_i].x;

                // ul
                if(x_i == 0 || y_i == 0)
                {
                    z = eval(l_x, u_y);
                    if(std::fpclassify(z) == FP_NORMAL ||
                        std::fpclassify(z) == FP_ZERO)
                    {
                        ul_def = true;
                        ul = glm::vec3(u_y, z, l_x);
                    }
                }
                else
                {
                    ul_def = defined[(y_i - 1) * _x_res + x_i - 1];
                    ul = coords[(y_i - 1) * _x_res + x_i - 1];
                }

                // u
                if(y_i == 0)
                {
                    z = eval(x, u_y);
                    if(std::fpclassify(z) == FP_NORMAL ||
                        std::fpclassify(z) == FP_ZERO)
                    {
                        u_def = true;
                        u = glm::vec3(u_y, z, x);
                    }
                }
                else
                {
                    u_def = defined[(y_i - 1) * _x_res + x_i];
                    u = coords[(y_i - 1) * _x_res + x_i];
                }

                // ur
                if(x_i == _x_res - 1 || y_i == 0)
                {
                    z = eval(r_x, u_y);
                    if(std::fpclassify(z) == FP_NORMAL ||
                        std::fpclassify(z) == FP_ZERO)
                    {
                        ur_def = true;
                        ur = glm::vec3(u_y, z, r_x);
                    }
                }
                else
                {
                    ur_def = defined[(y_i - 1) * _x_res + x_i + 1];
                    ur = coords[(y_i - 1) * _x_res + x_i + 1];
                }

                // r
                if(x_i == _x_res - 1)
                {
                    z = eval(r_x, y);
                    if(std::fpclassify(z) == FP_NORMAL ||
                        std::fpclassify(z) == FP_ZERO)
                    {
                        r_def = true;
                        r = glm::vec3(y, z, r_x);
                    }
                }
                else
                {
                    r_def = defined[y_i * _x_res + x_i + 1];
                    r = coords[y_i * _x_res + x_i + 1];
                }

                // lr
                if(x_i == _x_res - 1 || y_i == _y_res - 1)
                {
                    z = eval(r_x, d_y);
                    if(std::fpclassify(z) == FP_NORMAL ||
                        std::fpclassify(z) == FP_ZERO)
                    {
                        lr_def = true;
                        lr = glm::vec3(d_y, z, r_x);
                    }
                }
                else
                {
                    lr_def = defined[(y_i + 1) * _x_res + x_i + 1];
                    lr = coords[(y_i + 1) * _x_res + x_i + 1];
                }

                // d
                if(y_i == _y_res - 1)
                {
                    z = eval(x, d_y);
                    if(std::fpclassify(z) == FP_NORMAL ||
                        std::fpclassify(z) == FP_ZERO)
                    {
                        d_def = true;
                        d = glm::vec3(d_y, z, x);
                    }
                }
                else
                {
                    d_def = defined[(y_i + 1) * _x_res + x_i];
                    d = coords[(y_i + 1) * _x_res + x_i];
                }

                // ll
                if(x_i == 0 || y_i == _y_res - 1)
                {
                    z = eval(l_x, d_y);
                    if(std::fpclassify(z) == FP_NORMAL ||
                        std::fpclassify(z) == FP_ZERO)
                    {
                        ll_def = true;
                        ll = glm::vec3(d_y, z, l_x);
                    }
                }
                else
                {
                    ll_def = defined[(y_i + 1) * _x_res + x_i - 1];
                    ll = coords[(y_i + 1) * _x_res + x_i - 1];
                }

                // l
                if(x_i == 0)
                {
                    z = eval(l_x, y);
                    if(std::fpclassify(z) == FP_NORMAL ||
                        std::fpclassify(z) == FP_ZERO)
                    {
                        l_def = true;
                        l = glm::vec3(y, z, l_x);
                    }
                }
                else
                {
                    l_def = defined[y_i * _x_res + x_i - 1];
                    l = coords[y_i * _x_res + x_i - 1];
                }

                std::vector<glm::vec3> surrounding;

                std::cout<<ul<<u<<ur<<r<<lr<<d<<ll<<l<<std::endl;
                std::cout<<ul_def<<u_def<<ur_def<<r_def<<lr_def<<d_def<<ll_def<<l_def<<std::endl;
                glm::vec3 center = coords[y_i * _x_res + x_i];

                if(u_def && ur_def)
                    surrounding.push_back(glm::normalize(glm::cross(u - center, ur - center)));
                if(u_def && r_def)
                    surrounding.push_back(glm::normalize(glm::cross(u - center, r - center)));
                if(u_def && lr_def)
                    surrounding.push_back(glm::normalize(glm::cross(u - center, lr - center)));

                if(ur_def && r_def)
                    surrounding.push_back(glm::normalize(glm::cross(ur - center, r - center)));
                if(ur_def && lr_def)
                    surrounding.push_back(glm::normalize(glm::cross(ur - center, lr - center)));
                if(ur_def && d_def)
                    surrounding.push_back(glm::normalize(glm::cross(ur - center, d - center)));

                if(r_def && lr_def)
                    surrounding.push_back(glm::normalize(glm::cross(r - center, lr - center)));
                if(r_def && d_def)
                    surrounding.push_back(glm::normalize(glm::cross(r - center, d - center)));
                if(r_def && ll_def)
                    surrounding.push_back(glm::normalize(glm::cross(r - center, ll - center)));

                if(lr_def && d_def)
                    surrounding.push_back(glm::normalize(glm::cross(lr - center, d - center)));
                if(lr_def && ll_def)
                    surrounding.push_back(glm::normalize(glm::cross(lr - center, ll - center)));
                if(lr_def && l_def)
                    surrounding.push_back(glm::normalize(glm::cross(lr - center, l - center)));

                if(d_def && ll_def)
                    surrounding.push_back(glm::normalize(glm::cross(d - center, ll - center)));
                if(d_def && l_def)
                    surrounding.push_back(glm::normalize(glm::cross(d - center, l - center)));
                if(d_def && ul_def)
                    surrounding.push_back(glm::normalize(glm::cross(d - center, ul - center)));

                if(ll_def && l_def)
                    surrounding.push_back(glm::normalize(glm::cross(ll - center, l - center)));
                if(ll_def && ul_def)
                    surrounding.push_back(glm::normalize(glm::cross(ll - center, ul - center)));
                if(ll_def && u_def)
                    surrounding.push_back(glm::normalize(glm::cross(ll - center, u - center)));

                if(l_def && ul_def)
                    surrounding.push_back(glm::normalize(glm::cross(l - center, ul - center)));
                if(l_def && u_def)
                    surrounding.push_back(glm::normalize(glm::cross(l - center, u - center)));
                if(l_def && ur_def)
                    surrounding.push_back(glm::normalize(glm::cross(l - center, ur - center)));

                if(ul_def && u_def)
                    surrounding.push_back(glm::normalize(glm::cross(ul - center, u - center)));
                if(ul_def && ur_def)
                    surrounding.push_back(glm::normalize(glm::cross(ul - center, ur - center)));
                if(ul_def && r_def)
                    surrounding.push_back(glm::normalize(glm::cross(ul - center, r - center)));

                for(auto &i: surrounding)
                    normals[y_i * _x_res + x_i] += i;

                normals[y_i * _x_res + x_i] = glm::normalize(normals[y_i * _x_res + x_i]);
            }
        }

        for(auto &i: coords)
            std::cout<<i<<" ";
        std::cout<<std::endl;

        for(auto &i: tex_coords)
            std::cout<<i<<" ";
        std::cout<<std::endl;

        for(auto &i: normals)
            std::cout<<i<<" ";
        std::cout<<std::endl;

        for(size_t i = 0; i < defined.size(); ++i)
            std::cout<<(defined[i]?"def":"undef")<<" ";
        std::cout<<std::endl;

        // TODO: remove
        // print 'random' samples
        // std::cout<<verts[6][6]<<std::endl;
        // std::cout<<verts[6][5].coords<<" "<<verts[6][7].coords<<" "<<verts[5][6].coords<<" "<<verts[7][6].coords<<std::endl;

        // build data structures for OpenGL
        // glGenVertexArrays(1, _vao);
        // glBindVertexArray(_vao);

        std::vector<unsigned short> index;

        bool break_flag = true;


        // IDeas: indexed, use restart index to start new row / skip undefined? rework above to build separate arrays that we can use directly? unless undefined throws us off

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
                    index.push_back(0xFFFF);
                    break_flag = true;
                }
                else if(defined[ul] && defined[ur] && defined[lr])
                {
                    if(!break_flag)
                        index.push_back(0xFFFF);
                    index.push_back(ul);
                    index.push_back(lr);
                    index.push_back(ur);
                    index.push_back(0xFFFF);
                    break_flag = true;
                }
                else if(defined[ul] && defined[ll] && defined[lr])
                {
                    index.push_back(ul);
                    index.push_back(ll);
                    index.push_back(lr);
                    index.push_back(0xFFFF);
                    break_flag = true;
                }
                else if(defined[ur] && defined[ll] && defined[lr])
                {
                    if(!break_flag)
                        index.push_back(0xFFFF);
                    index.push_back(ur);
                    index.push_back(ll);
                    index.push_back(lr);
                    index.push_back(0xFFFF);
                    break_flag = true;
                }
                else
                {
                    if(!break_flag)
                        index.push_back(0xFFFF);
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
                index.push_back(0xFFFF);
            break_flag = true;
        }

        std::cout<<std::endl;
        for(auto &i: index)
        {
            if(i != 0xFFFF)
                std::cout<<i<<":"<<coords[i]<<" ";
            else
                std::cout<<std::endl;
        }
    }

private:
    double _x, _y;
    double _x_min, _y_min, _x_max, _y_max;
    int _x_res, _y_res;
};

#endif // __GRAPH_H__
