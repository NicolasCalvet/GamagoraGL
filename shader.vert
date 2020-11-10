#version 450

uniform float time;

in vec3 position;
in vec3 color;
in float pointSize;

out vec3 particle_color;


void main()
{
    gl_Position = vec4(position, 1.0);
    // gl_Position = vec4(position * sin(time), 1.0);

    gl_PointSize = 40 * pointSize;
    
    particle_color = color;
}