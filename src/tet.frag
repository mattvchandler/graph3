#version 330 core

out vec4 frag_color;

uniform sampler2D tex;

// lighting vars
uniform vec3 ambient_color;
uniform vec3 light_color;
uniform vec3 light_pos;
uniform float light_shiny = 0.2;
uniform float light_strength = 0.8;

uniform vec3 cam_forward;

uniform float const_atten = 1.0;
uniform float linear_atten = 0.5;
uniform float quad_atten = 0.0;

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

    float diffuse = max(0.0, dot(normal_vec, light_dir));
    float specular = max(0.0, dot(normal_vec, half_vec));

    if(diffuse <= 0.0001)
        specular = 0.0;
    else
        specular = pow(specular, light_shiny) * light_strength;

    vec3 scattered = ambient_color + light_color * diffuse * atten;
    vec3 reflected = light_color * specular * atten;
    vec3 rgb = min(texture(tex, tex_coords).rgb * scattered + reflected, vec3(1.0));
    frag_color = vec4(rgb, texture(tex, tex_coords).a);
}
