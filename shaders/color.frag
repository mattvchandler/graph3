// color.frag
// phong lighting fragment shader - no texture

// Copyright 2018 Matthew Chandler

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

#version 330 core

out vec4 frag_color;

// material vars
struct Material
{
    vec3 specular;
    float shininess;
};
uniform Material material;
uniform vec3 color;

// lighting vars
uniform vec3 ambient_color;

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
uniform Point_light cam_light;

struct Dir_light
{
    Base_light base;
    vec3 dir;
    vec3 half_vec;
};
uniform Dir_light dir_light;

// camera facing direction
uniform vec3 light_forward;

in vec2 tex_coords;
in vec3 normal_vec;
in vec3 pos;

void calc_point_lighting(in vec3 pos, in vec3 forward, in vec3 normal_vec,
    in Material material, in Point_light point_light, out vec3 scattered, out vec3 reflected);

void calc_dir_lighting(in vec3 normal_vec, in Material material, in Dir_light dir_light,
    out vec3 scattered, out vec3 reflected);

void main()
{
    vec3 scattered = ambient_color, reflected = vec3(0.0, 0.0, 0.0);
    vec3 tmp_scattered, tmp_reflected;

    calc_point_lighting(pos, light_forward, normal_vec, material, cam_light,
        tmp_scattered, tmp_reflected);
    scattered += tmp_scattered;
    reflected += tmp_reflected;

    calc_dir_lighting(normal_vec, material, dir_light, tmp_scattered, tmp_reflected);
    scattered += tmp_scattered;
    reflected += tmp_reflected;

    // add to material color (from material color) to lighting for final color
    vec3 rgb = min(color.rgb * scattered + reflected, vec3(1.0));
    frag_color = vec4(rgb, 1.0);
}
