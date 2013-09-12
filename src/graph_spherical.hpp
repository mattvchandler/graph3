// graph_spherical.hpp
// spherical coordinate system graph class (Z(r, theta))

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

#ifndef __GRAPH_SPHERICAL_H__
#define __GRAPH_SPHERICAL_H__

#include "graph.hpp"

class Graph_spherical final: public Graph
{
public:
    explicit Graph_spherical(const std::string & eqn = "",
        float theta_min = -1.0f, float theta_max = 1.0f, int theta_res = 50,
        float phi_min = -1.0f, float phi_max = 1.0f, int phi_res = 50);

    double eval(const double theta, const double phi) override;
    void build_graph() override;

    // cursor funcs
    void move_cursor(const Cursor_dir dir) override;
    glm::vec3 cursor_pos() const override;
    bool cursor_defined() const override;
    std::string cursor_text() const override;

private:
    double  _theta, _phi;
    double _theta_min, _theta_max;
    size_t _theta_res;
    double _phi_min, _phi_max;
    size_t _phi_res;

    double _cursor_theta, _cursor_phi, _cursor_r;
    glm::vec3 _cursor_pos;
    bool _cursor_defined;
};

#endif // __GRAPH_SPHERICAL_H__
