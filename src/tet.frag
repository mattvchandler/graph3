#version 330 core

out vec4 frag_color;

uniform sampler2D tex;
uniform vec4 ambient;

in vec2 tex_coords;
in vec3 normal_vec;

void main()
{
    vec4 scattered = ambient;
    frag_color = min(texture(tex, tex_coords) * scattered, vec4(1.0));
}
