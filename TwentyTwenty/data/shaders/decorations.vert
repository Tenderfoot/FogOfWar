#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

out vec3 ourColor;
out vec2 TexCoord;
out vec3 ourPos;

void main()
{
    gl_Position = projection * view * model * vec4(aPos[0], aPos[1], 0.0, 1.0);
    ourColor = aColor;
    TexCoord = aTexCoord;
    ourPos = vec3(0,0,aPos[2]);
}