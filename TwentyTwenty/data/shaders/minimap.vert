#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aColor;
layout (location = 3) in float aType;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

out vec3 ourColor;
out vec2 TexCoord;
out vec3 ourPos;
out float ourType;
out float ourTime;

void main()
{
    ourColor = aColor;
    TexCoord = aTexCoord;
    ourType = aType;
    ourTime = time;
    ourPos = vec3(0,0,aPos[2]);
}