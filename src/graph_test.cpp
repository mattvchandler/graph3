// graph_test.cpp
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

#include <iostream>

#include "graph.h"

int main()
{
    // init glew
    // TODO: verify: can we do this before GL context is initialized?
    // NOPE!
    if(glewInit() != GLEW_OK)
    {
        std::cerr<<"Error loading glew. Aborting"<<std::endl;
        exit(EXIT_FAILURE);
    }

    // Graph_cartesian a("0");
    // Graph_cartesian a("sqrt(1.0 - (x*x + y*y))");
    // Graph_cartesian a("sqrt(1.5625 - (x*x + y*y))");
    Graph_cartesian a("x");
    Graph * g = &a;

    try
    {
        double ret = g->eval(0,0);
        if(std::fpclassify(ret) == FP_NORMAL || std::fpclassify(ret) == FP_ZERO)
            std::cout<<g->eval(0,0)<<std::endl;
        else
            std::cout<<"Undefined!"<<std::endl;

        g->build_graph();
    }
    catch(...)
    {
        std::cerr<<"could not evaluate given equation"<<std::endl;
    }
    return 0;
}

