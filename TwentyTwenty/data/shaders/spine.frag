#version 330 core
out vec4 FragColor;
out float FragDepth;
  
in vec3 ourColor;
in vec2 TexCoord;
in vec3 ourPos;
uniform sampler2D ourTexture;

void main()
{
    vec4 new_color =  texture(ourTexture, TexCoord);
    if(new_color.a < 0.015)
        discard;

    gl_FragDepth = ourPos.z;
    FragColor = texture(ourTexture, TexCoord);
}