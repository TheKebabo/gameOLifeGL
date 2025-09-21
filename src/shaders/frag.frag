#version 430 core

out vec4 fragColor;

layout (location = 1) uniform vec4 Color = vec4(vec3(0.0), 1.0);

void main()
{
    fragColor = Color;
}