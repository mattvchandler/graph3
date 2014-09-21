// tex.frag
// textured phong lighting fragment shader

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

out vec4 frag_color;

// material vars
uniform sampler2D tex;
uniform float shininess;
uniform vec3 specular;

// lighting vars
uniform vec3 ambient_color;

uniform vec3 cam_light_color;
uniform vec3 cam_light_pos_eye;
uniform float cam_light_strength;

uniform float const_atten;
uniform float linear_atten;
uniform float quad_atten;

uniform vec3 dir_light_color;
uniform vec3 dir_light_dir;
uniform float dir_light_strength;
uniform vec3 dir_half_vec;

// camera facing direction
uniform vec3 light_forward;

in vec2 tex_coords;
in vec3 normal_vec;
in vec3 pos;

void calc_lighting(in vec3 pos, in vec3 cam_light_pos_eye, in float const_atten,
    in float linear_atten, in float quad_atten, in vec3 light_forward, in vec3 normal_vec,
    in vec3 dir_light_dir, in vec3 dir_half_vec, in float shininess,
    in float cam_light_strength, in float dir_light_strength, in vec3 specular,
    in vec3 cam_light_color, in vec3 dir_light_color, in vec3 ambient_color,
    out vec3 scattered, out vec3 reflected);

void main()
{
    vec3 scattered, reflected;

    calc_lighting(pos, cam_light_pos_eye, const_atten, linear_atten, quad_atten,
        light_forward, normal_vec, dir_light_dir, dir_half_vec, shininess,
        cam_light_strength, dir_light_strength, specular, cam_light_color,
        dir_light_color, ambient_color, scattered, reflected);

    // add to material color (from texture) to lighting for final color
    vec3 rgb = min(texture(tex, tex_coords).rgb * scattered + reflected, vec3(1.0));
    frag_color = vec4(rgb, texture(tex, tex_coords).a);
}
