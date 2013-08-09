#version 330 core

out vec4 frag_color;

uniform sampler2D tex;

in vec2 tex_coords;

void main()
{
    // frag_color = vec4(gl_FragCoord.x / 800, gl_FragCoord.y / 600, 0.0, 1.0);
    frag_color = texture(tex, tex_coords);
}
