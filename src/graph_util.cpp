// graph_util.cpp
// generic graphing class

// Copyright 2014 Matthew Chandler

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

#include "graph.hpp"

Graph_exception::Graph_exception(const mu::Parser::exception_type & mu_e, const Location l):
    mu::Parser::exception_type(mu_e), _location(l)
{}

Graph_exception::Location Graph_exception::GetLocation() const
{
    return _location;
}

// calculate the normal of a point given surrounding points
glm::vec3 get_normal (glm::vec3 center,
    glm::vec3 up, bool up_def,
    glm::vec3 ur, bool ur_def,
    glm::vec3 rt, bool rt_def,
    glm::vec3 lr, bool lr_def,
    glm::vec3 dn, bool dn_def,
    glm::vec3 ll, bool ll_def,
    glm::vec3 lf, bool lf_def,
    glm::vec3 ul, bool ul_def)
{
    const float epsilon = std::numeric_limits<double>::epsilon() / 2.0f;

    std::vector<glm::dvec3> surrounding;

    // temporary vars
    glm::dvec3 cr;
    double length;

    // get tangents through surrounding points
    glm::dvec3 t_up, t_ur, t_rt, t_lr, t_dn, t_ll, t_lf, t_ul;

    // check to make sure we have no 0-length vectors, and then normalize
    if(up_def)
    {
        t_up = up - center;
        length = glm::length(t_up);
        if(length <= epsilon)
            up_def = false;
    }
    if(ur_def)
    {
        t_ur = ur - center;
        length = glm::length(t_ur);
        if(length <= epsilon)
            ur_def = false;
    }
    if(rt_def)
    {
        t_rt = rt - center;
        length = glm::length(t_rt);
        if(length <= epsilon)
            rt_def = false;
    }
    if(lr_def)
    {
        t_lr = lr - center;
        length = glm::length(t_lr);
        if(length <= epsilon)
            lr_def = false;
    }
    if(dn_def)
    {
        t_dn = dn - center;
        length = glm::length(t_dn);
        if(length <= epsilon)
            dn_def = false;
    }
    if(ll_def)
    {
        t_ll = ll - center;
        length = glm::length(t_ll);
        if(length <= epsilon)
            ll_def = false;
    }
    if(lf_def)
    {
        t_lf = lf - center;
        length = glm::length(t_lf);
        if(length <= epsilon)
            lf_def = false;
    }
    if(ul_def)
    {
        t_ul = ul - center;
        length = glm::length(t_ul);
        if(length <= epsilon)
            ul_def = false;
    }

    // get cross-products from combinations of surrounding points
    // check for colinearity
    if(up_def)
    {
        if(ur_def)
        {
            cr = glm::cross(t_ur, t_up);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(rt_def)
        {
            cr = glm::cross(t_rt, t_up);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(lr_def)
        {
            cr = glm::cross(t_lr, t_up);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
    }

    if(ur_def)
    {
        if(rt_def)
        {
            cr = glm::cross(t_rt, t_ur);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(lr_def)
        {
            cr = glm::cross(t_lr, t_ur);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(dn_def)
        {
            cr = glm::cross(t_dn, t_ur);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
    }

    if(rt_def)
    {
        if(lr_def)
        {
            cr = glm::cross(t_lr, t_rt);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(dn_def)
        {
            cr = glm::cross(t_dn, t_rt);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(ll_def)
        {
            cr = glm::cross(t_ll, t_rt);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
    }

    if(lr_def)
    {
        if(dn_def)
        {
            cr = glm::cross(t_dn, t_lr);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(ll_def)
        {
            cr = glm::cross(t_ll, t_lr);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(lf_def)
        {
            cr = glm::cross(t_lf, t_lr);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
    }

    if(dn_def)
    {
        if(ll_def)
        {
            cr = glm::cross(t_ll, t_dn);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(lf_def)
        {
            cr = glm::cross(t_lf, t_dn);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(ul_def)
        {
            cr = glm::cross(t_ul, t_dn);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
    }

    if(ll_def)
    {
        if(lf_def)
        {
            cr = glm::cross(t_lf, t_ll);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(ul_def)
        {
            cr = glm::cross(t_ul, t_ll);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(up_def)
        {
            cr = glm::cross(t_up, t_ll);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
    }

    if(lf_def)
    {
        if(ul_def)
        {
            cr = glm::cross(t_ul, t_lf);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(up_def)
        {
            cr = glm::cross(t_up, t_lf);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(ur_def)
        {
            cr = glm::cross(t_ur, t_lf);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
    }

    if(ul_def)
    {
        if(up_def)
        {
            cr = glm::cross(t_up, t_ul);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(ur_def)
        {
            cr = glm::cross(t_ur, t_ul);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
        if(rt_def)
        {
            cr = glm::cross(t_rt, t_ul);
            length = glm::length(cr);
            if(length > epsilon)
                surrounding.push_back(cr / length);
        }
    }

    // add surrounding vectors together (for averaging)
    glm::dvec3 normal(0.0f);
    for(auto &i: surrounding)
    {
        // invert inverted normals
        if(glm::length(normal + i) > glm::length(normal))
            normal += i;
        else
            normal -= i;
    }

    // check to see if we have a good vector before normalizing (to prevent div by 0)
    if(surrounding.size() > 0 && glm::length(normal) > epsilon)
        return glm::vec3(glm::normalize(normal));
    else
        // fall back to up vector
        return glm::vec3(0.0f, 0.0f, 1.0f);
}
