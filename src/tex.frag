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

void main()
{
    // light location
    vec3 light_dir = cam_light_pos_eye - pos;
    float light_dist = length(light_dir);

    light_dir = light_dir / light_dist;

    // calculate light falloff
    float atten = 1.0 / (const_atten
        + linear_atten * light_dist
        + quad_atten * light_dist * light_dist);

    // midway between light and camera - for reflection calc
    vec3 half_vec = normalize(light_dir + light_forward);

    // calculate ammt of diffuse and specular shading
    float diffuse_mul, specular_mul;
    float dir_diffuse_mul, dir_specular_mul;
    if(gl_FrontFacing)
    {
        diffuse_mul = max(0.0, dot(normal_vec, light_dir));
        specular_mul = max(0.0, dot(normal_vec, half_vec));

        dir_diffuse_mul = max(0.0, dot(normal_vec, normalize(dir_light_dir)));
        dir_specular_mul = max(0.0, dot(normal_vec, half_vec));
    }
    else
    {
        diffuse_mul = max(0.0, dot(-normal_vec, light_dir));
        specular_mul = max(0.0, dot(-normal_vec, half_vec));

        dir_diffuse_mul = max(0.0, dot(-normal_vec, normalize(dir_light_dir)));
        dir_specular_mul = max(0.0, dot(-normal_vec, dir_half_vec));
    }

    // calculate specular shine strength
    if(diffuse_mul <= 0.0001)
        specular_mul = 0.0;
    else
        specular_mul = pow(specular_mul, shininess) * cam_light_strength;

    if(dir_diffuse_mul <= 0.0001)
        dir_specular_mul = 0.0;
    else
        dir_specular_mul = pow(dir_specular_mul, shininess) * dir_light_strength;

    // diffuse light color
    vec3 scattered = ambient_color + cam_light_color * diffuse_mul * atten + dir_light_color * dir_diffuse_mul;
    // specular light color
    vec3 reflected = cam_light_color * specular_mul * atten * specular + dir_light_color * dir_specular_mul * specular;
    // add to material color (from texture) to lighting for final color
    vec3 rgb = min(texture(tex, tex_coords).rgb * scattered + reflected, vec3(1.0));
    frag_color = vec4(rgb, texture(tex, tex_coords).a);
}
