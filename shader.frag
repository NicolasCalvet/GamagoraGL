#version 450

uniform float time;

in vec3 particle_color;

out vec4 color;


void main()
{
    color = vec4(fract(time) * particle_color, 1);
    // color = vec4((sin(time) + 1.f) / 2.f * particle_color, 0.f);
}