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
