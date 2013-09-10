// graph_cartesian.h
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

#ifndef __GRAPH_CARTESIAN_H__
#define __GRAPH_CARTESIAN_H__

#include "graph.h"

class Graph_cartesian final: public Graph
{
public:
    explicit Graph_cartesian(const std::string & eqn = "",
        float x_min = -1.0f, float x_max = 1.0f, int x_res = 50,
        float y_min = -1.0f, float y_max = 1.0f, int y_res = 50);

    double eval(const double x, const double y) override;
    void build_graph() override;

    // cursor funcs
    void move_cursor(const Cursor_dir dir) override;
    glm::vec3 cursor_pos() const override;
    bool cursor_defined() const override;
    std::string cursor_text() const override;

private:
    double _x, _y;
    double _x_min, _x_max;
    size_t _x_res;
    double _y_min, _y_max;
    size_t _y_res;

    glm::vec3 _cursor_pos;
    bool _cursor_defined;
};

#endif // __GRAPH_CARTESIAN_H__
