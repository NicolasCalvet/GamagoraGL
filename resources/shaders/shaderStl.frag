#version 450

in vec3 particle_color;

out vec4 color;

void main()
{

    vec3 normal = normalize(
           cross(
             dFdx(positionWorld),
         dFdy(positionWorld)
          ));

    color = vec4(particle_color, 1.0f);
}