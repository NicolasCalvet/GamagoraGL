#version 450

in vec3 position;

out vec3 particle_color;

void main()
{

    vec3 position_temp = position / 50.0f;

    gl_Position = vec4(position_temp, 1.0);
    
    particle_color = position_temp;
}