#version 450

uniform float time;

in vec3 position;
in vec3 color;
in float pointSize;

out vec3 particle_color;


void main()
{
    gl_Position = vec4(position.x * sin(time), position.y * cos(time), position.z, 1.0);

    gl_PointSize = pointSize;
    
    particle_color = color;
}