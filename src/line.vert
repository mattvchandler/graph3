// line.vert
// vertex shader for drawing lines. draws slightly in front of actual coords

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

#version 330 core

layout(location = 0) in vec3 vert_pos;
layout(location = 1) in vec2 tex;
layout(location = 2) in vec3 normal;

uniform mat4 perspective;
uniform mat4 view_model;
uniform mat3 normal_transform;

out vec2 tex_coords;
out vec3 normal_vec;
out vec3 pos;

void main()
{
    tex_coords = tex;
    normal_vec = normalize(normal_transform * normal);
    pos = vec3(view_model * vec4(vert_pos, 1.0) + vec4(0.0, 0.0, 0.01, 0.0));
    gl_Position = perspective * (view_model * vec4(vert_pos, 1.0) + vec4(0.0, 0.0, 0.01, 0.0));
}
