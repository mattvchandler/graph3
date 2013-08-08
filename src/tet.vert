#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec4 color;

uniform mat4 view_model_perspective;

out vec4 vertex_color;

void main()
{
    vertex_color = color;
    gl_Position = view_model_perspective * vec4(pos, 1.0);
}
