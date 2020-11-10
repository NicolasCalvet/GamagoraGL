#version 450

uniform float time;
uniform vec3 colorA;
uniform vec3 colorB;
uniform float mouseX;

in vec3 position;
in vec3 color;
in float pointSize;

out vec3 particle_color;


void main()
{

    float r = length(position.xy);
    float time_temp = mouseX;
    float speed = 10.f;

    gl_PointSize = r * 20;

    /*
    float x = cos(time * 1/r * speed) * r;
    float y = sin(time * 1/r * speed) * r;
    */

    /*
    float x = cos(time * 1/r * speed) * sin(r);
    float y = sin(time * 1/r * speed) * sin(r);
    */

    /*
    float x = (cos(time * 1/r * speed) + 1.0f) / 2.0f * r;
    float y = (sin(time * 1/r * speed) + 1.0f) / 2.0f * r;
    */

    /*
    float x = cos(time * 1/r * speed) * sin(r + 1.0f) / 2.0f;
    float y = sin(time * 1/r * speed) * sin(r + 1.0f) / 2.0f;
    */

    
    float x = cos(time_temp * 1/r * speed) * (sin(r) + 1.0f) / 2.0f;
    float y = sin(time_temp * 1/r * speed) * (sin(r) + 1.0f) / 2.0f;
    

    /*
    float x = cos(time) * (sin(r) + 1.0f) / 2.0f;
    float y = sin(time) * (sin(r) + 1.0f) / 2.0f;
    x += position.x;
    y += position.y;
    */

    gl_Position = vec4(x, y, 1.0, 1.0);

    // gl_Position = vec4(position.x * sin(time), position.y * cos(time), position.z, 1.0);
    // gl_Position = vec4(position, 1.0);
    
    particle_color = mix(colorA, colorB, r);
}