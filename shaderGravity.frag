#version 450

uniform float time;

in vec3 particle_color;

out vec4 color;


void main()
{
    color = vec4(particle_color, 1.0f);
}