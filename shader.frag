#version 450

out vec4 color;

in vec3 particle_color;

void main()
{
    color = vec4(particle_color, 1);
}