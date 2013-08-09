#version 330 core

out vec4 frag_color;

uniform sampler2D tex;

in vec2 tex_coords;
in vec3 normal_vec;

void main()
{
    frag_color = texture(tex, tex_coords);
}
