#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 tex;
layout(location = 2) in vec3 normal;

uniform mat4 view_model_perspective;
uniform mat3 normal_transform;

out vec2 tex_coords;
out vec3 normal_vec;

void main()
{
    tex_coords = tex;
    normal_vec = normal_transform * normal;
    gl_Position = view_model_perspective * vec4(pos, 1.0);
}
