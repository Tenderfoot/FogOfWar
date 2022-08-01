#version 330 core
out vec4 FragColor;
out float FragDepth;
  
in vec3 ourColor;
in vec2 TexCoord;
in vec3 ourPos;
in float ourType;
in float ourTime;

uniform sampler2D ourTexture;  
uniform sampler2D waterTexture;

void main()
{
    gl_FragDepth = 0.0f;
    FragColor = vec4(1.0f,1.0f,1.0f,0.0f);
}