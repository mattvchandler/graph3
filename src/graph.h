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

#include <muParser.h>

#ifndef M_PI
#define M_PI 3.141592654
#endif

#ifndef M_E
#define M_E 2.718281828
#endif

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
    // Graph(const Graph & a);
    // Graph(Graph && a);
    // ~Graph();
    // Graph & operator =(const Graph & a);
    // Graph & operator =(Graph && a);

    virtual double eval(const double, const double) = 0;
    virtual void build_graph() = 0;

protected:
    std::string _eqn;
    mu::Parser _p;
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
    }

private:
    double _x, _y;

};

#endif // __GRAPH_H__
