// common.frag
// phong lighting fragment shader calculations
// to be linked with another vert shader file

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

#version 130

// material vars
struct Material
{
    vec3 specular;
    float shininess;
};

// lighting vars
struct Base_light
{
    vec3 color;
    float strength;
};

struct Point_light
{
    Base_light base;
    vec3 pos_eye;
    float const_atten;
    float linear_atten;
    float quad_atten;
};

struct Dir_light
{
    Base_light base;
    vec3 dir;
    vec3 half_vec;
};

void calc_lighting(in vec3 pos, in vec3 light_forward, in vec3 normal_vec, in vec3 ambient_color,
    in Material material, in Point_light cam_light, in Dir_light dir_light,
    out vec3 scattered, out vec3 reflected)
{
    // point light location
    vec3 cam_light_dir = cam_light.pos_eye - pos;
    float cam_light_dist = length(cam_light_dir);

    cam_light_dir = cam_light_dir / cam_light_dist; // normalize, but reuse length instead of calling normalize

    // calculate point light falloff
    float cam_atten = 1.0 / (cam_light.const_atten
        + cam_light.linear_atten * cam_light_dist
        + cam_light.quad_atten * cam_light_dist * cam_light_dist);

    // midway between light and camera - for reflection calc
    vec3 cam_half_vec = normalize(cam_light_dir + light_forward);

    // calculate ammt of diffuse and specular shading
    float cam_diffuse_mul, cam_specular_mul;
    float dir_diffuse_mul, dir_specular_mul;
    if(gl_FrontFacing)
    {
        cam_diffuse_mul = max(0.0, dot(normal_vec, cam_light_dir));
        cam_specular_mul = max(0.0, dot(normal_vec, cam_half_vec));

        dir_diffuse_mul = max(0.0, dot(normal_vec, normalize(dir_light.dir)));
        dir_specular_mul = max(0.0, dot(normal_vec, dir_light.half_vec));
    }
    else
    {
        cam_diffuse_mul = max(0.0, dot(-normal_vec, cam_light_dir));
        cam_specular_mul = max(0.0, dot(-normal_vec, cam_half_vec));

        dir_diffuse_mul = max(0.0, dot(-normal_vec, normalize(dir_light.dir)));
        dir_specular_mul = max(0.0, dot(-normal_vec, dir_light.half_vec));
    }

    // calculate specular shine strength
    if(cam_diffuse_mul <= 0.0001)
        cam_specular_mul = 0.0;
    else
        cam_specular_mul = pow(cam_specular_mul, material.shininess) * cam_light.base.strength;

    if(dir_diffuse_mul <= 0.0001)
        dir_specular_mul = 0.0;
    else
        dir_specular_mul = pow(dir_specular_mul, material.shininess) * dir_light.base.strength;

    // diffuse light color
    scattered = ambient_color + cam_light.base.color * cam_diffuse_mul * cam_atten + dir_light.base.color * dir_diffuse_mul;
    // specular light color
    reflected = cam_light.base.color * cam_specular_mul * cam_atten * material.specular + dir_light.base.color * dir_specular_mul * material.specular;
}
