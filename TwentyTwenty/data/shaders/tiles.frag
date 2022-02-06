#version 330 core
out vec4 FragColor;
out float FragDepth;
  
in vec3 ourColor;
in vec2 TexCoord;
in vec3 ourPos;
in float ourType;
in float ourTime;
uniform sampler2D ourTexture;

void main()
{
    vec4 old = gl_FragColor;

    vec2 water_uv_offset;
    water_uv_offset.x = sin(ourTime);
    water_uv_offset.y = sin(ourTime);
    
    vec4 tex;
    if(ourType == 2)
    {
        tex = texture(ourTexture, TexCoord + water_uv_offset);
    }
    else
    {
        tex = texture(ourTexture, TexCoord);
    }

    float r = tex[0]*0.8;
    float g = tex[1]*0.8;
    float b = tex[2]*0.8;

    gl_FragDepth = ourPos.z;
    FragColor = vec4(r,g,b,tex.a);
}