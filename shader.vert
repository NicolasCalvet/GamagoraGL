#version 450

uniform mat4 trans;

in vec3 position;
out vec3 positionWorld;

void main()
{
    vec3 lightPos;

    gl_Position = trans * vec4(position * 0.01, 1.0);
    positionWorld = (gl_Position / gl_Position.w).xyz;
}
