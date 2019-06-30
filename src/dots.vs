#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aOffset;

out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform float zoom;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(aPos/200 * zoom + aOffset, 1.0f); 
}