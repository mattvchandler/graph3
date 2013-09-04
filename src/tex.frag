#version 330 core

out vec4 frag_color;

// material vars
uniform sampler2D tex;

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
    if(gl_FrontFacing)
    {
        diffuse_mul = max(0.0, dot(normal_vec, light_dir));
        specular_mul = max(0.0, dot(normal_vec, half_vec));
    }
    else
    {
        diffuse_mul = max(0.0, dot(-normal_vec, light_dir));
        specular_mul = max(0.0, dot(-normal_vec, half_vec));
    }

    if(diffuse_mul <= 0.0001)
        specular_mul = 0.0;
    else
        specular_mul = pow(specular_mul, shininess) * light_strength;

    vec3 scattered = ambient_color + light_color * diffuse_mul * atten;
    vec3 reflected = light_color * specular_mul * atten * specular;
    vec3 rgb = min(texture(tex, tex_coords).rgb * scattered + reflected, vec3(1.0));
    frag_color = vec4(rgb, texture(tex, tex_coords).a);
}
