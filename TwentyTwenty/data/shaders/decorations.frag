#version 330 core
out vec4 FragColor;
out float FragDepth;
  
in vec3 ourColor;
in vec2 TexCoord;
in vec3 ourPos;
uniform sampler2D ourTexture;

void main()
{
    vec4 tex =  texture(ourTexture, TexCoord);
    vec4 old = gl_FragColor;

    if(tex.a < 0.015)
        discard;

    float r = tex[0]*0.8;
    float g = tex[1]*0.8;
    float b = tex[2]*0.8;


    gl_FragDepth = ourPos.z;
    FragColor = vec4(r,g,b,tex.a);
}