#version 450

layout (binding = 0) uniform sampler2D tex;

in vec3 ourColor;
in vec2 TexCoord;

out vec4 color;

void main()
{
//    color = texture(tex, TexCoord) * vec4(ourColor, 1.0); 
//    color = vec4(ourColor, 1.0); 
    color = texture(tex, TexCoord); 
}