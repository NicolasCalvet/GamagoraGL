#version 450

in vec3 position;
in vec3 color;
in float pointSize;

out vec3 particle_color;


void main()
{

    gl_PointSize = pointSize;

    gl_Position = vec4(position, 1.0);
    
    particle_color = color;
}