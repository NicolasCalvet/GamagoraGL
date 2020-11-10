#version 450

in vec3 positionWorld;
out vec4 color;

void main()
{
    vec3 normal = normalize(cross(dFdx(positionWorld), dFdy(positionWorld)));


    vec3 lightPos = vec3(10, 10, 10);
    vec3 lightDir = normalize(lightPos - positionWorld);
    float coef = abs(dot(lightDir, normal));

    color = vec4(coef * vec3(1, 1, 1), 1.0);
}
