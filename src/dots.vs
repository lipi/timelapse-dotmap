#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec2 aOffset;

out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform float zoom;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(aPos.x/200 * zoom + aOffset.x, 
                                           aPos.y/200 * zoom + aOffset.y, 0.0f, 1.0f); 
}