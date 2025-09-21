#version 430 core
layout (location = 0) in vec3 aPos;

uniform mat4 transl;
uniform mat4 scale;

void main()
{
    // gl_Position = transl * scale * vec4(aPos, 1.0);
    gl_Position = vec4(aPos, 1.0);
}