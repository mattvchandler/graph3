// graph_parametric.hpp
// parametric coordinate system graph class (x(u,v), y(u,v), z(u,v))

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

#ifndef __GRAPH_PARAMETRIC_H__
#define __GRAPH_PARAMETRIC_H__

#include "graph.hpp"

class Graph_parametric final: public Graph
{
public:
    explicit Graph_parametric(const std::string & eqn,
        const std::string & u_min, const std::string & u_max, size_t u_res,
        const std::string & v_min, const std::string & v_max, size_t v_res);

    glm::vec3 eval(const double u, const double v);
    void build_graph() override;

    // cursor funcs
    void move_cursor(const Cursor_dir dir) override;
    glm::vec3 cursor_pos() const override;
    bool cursor_defined() const override;
    std::string cursor_text() const override;

private:
    double _u, _v;
    double _u_min, _u_max;
    size_t _u_res;
    double _v_min, _v_max;
    size_t _v_res;

    double _cursor_u, _cursor_v;
    glm::vec3 _cursor_pos;
    bool _cursor_defined;
};

#endif // __GRAPH_PARAMETRIC_H__
