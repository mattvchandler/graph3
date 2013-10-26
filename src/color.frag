// color.frag
// phong lighting fragment shader - no texture

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

#version 130

out vec4 frag_color;

// material vars
uniform vec4 color;

uniform float shininess;
uniform vec3 specular;

// lighting vars
uniform vec3 ambient_color;

uniform vec3 light_color;
uniform vec3 light_pos;
uniform float light_strength;

uniform float const_atten;
uniform float linear_atten;
uniform float quad_atten;

uniform vec3 dir_light_color;
uniform vec3 dir_light_dir;
uniform float dir_light_strength;
uniform vec3 dir_half_vec;

uniform vec3 cam_forward;

in vec2 tex_coords;
in vec3 normal_vec;
in vec3 pos;

void main()
{
    vec3 light_dir = light_pos - pos;
    float light_dist = length(light_dir);

    light_dir = light_dir / light_dist;

    float atten = 1.0 / (const_atten
        + linear_atten * light_dist
        + quad_atten * light_dist * light_dist);

    vec3 half_vec = normalize(light_dir + cam_forward);

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

    if(diffuse_mul <= 0.0001)
        specular_mul = 0.0;
    else
        specular_mul = pow(specular_mul, shininess) * light_strength;

    if(dir_diffuse_mul <= 0.0001)
        dir_specular_mul = 0.0;
    else
        dir_specular_mul = pow(dir_specular_mul, shininess) * dir_light_strength;

    vec3 scattered = ambient_color + light_color * diffuse_mul * atten + dir_light_color * dir_diffuse_mul;
    vec3 reflected = light_color * specular_mul * atten * specular + dir_light_color * dir_specular_mul * specular;
    vec3 rgb = min(color.rgb * scattered + reflected, vec3(1.0));
    frag_color = vec4(rgb, color.a);
}
