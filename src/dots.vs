#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec2 aOffset;

out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform float dot_scale;
uniform float x_scale;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = projection * view * vec4(aPos.x * dot_scale + aOffset.x * x_scale, 
                                           aPos.y * dot_scale + aOffset.y, 0.0f, 1.0f); 
}