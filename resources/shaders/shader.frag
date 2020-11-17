#version 450

uniform float time;

in vec3 particle_color;

out vec4 color;


void main()
{
    // color = vec4((sin(time * 10) + 1.0f) / 2.0f * particle_color, 1.0f);
    color = vec4(particle_color, 1.0f);
}