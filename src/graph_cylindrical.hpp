// graph_cylindrical.hpp
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

#ifndef __GRAPH_CYLINDRICAL_H__
#define __GRAPH_CYLINDRICAL_H__

#include "graph.hpp"

class Graph_cylindrical final: public Graph
{
public:
    explicit Graph_cylindrical(const std::string & eqn,
        const std::string & r_min, const std::string & r_max, size_t r_res,
        const std::string & theta_min, const std::string & theta_max, size_t theta_res);

    double eval(const double r, const double theta);
    void build_graph() override;

    // cursor funcs
    void move_cursor(const Cursor_dir dir) override;
    glm::vec3 cursor_pos() const override;
    bool cursor_defined() const override;
    std::string cursor_text() const override;

private:
    mu::Parser _p;
    std::string _eqn;

    double _r, _theta;
    double _r_min, _r_max;
    size_t _r_res;
    double _theta_min, _theta_max;
    size_t _theta_res;

    double _cursor_r, _cursor_theta;
    glm::vec3 _cursor_pos;
    bool _cursor_defined;
};

#endif // __GRAPH_CYLINDRICAL_H__
