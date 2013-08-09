#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 tex;

uniform mat4 view_model_perspective;

out vec2 tex_coords;

void main()
{
    tex_coords = tex;
    gl_Position = view_model_perspective * vec4(pos, 1.0);
}
