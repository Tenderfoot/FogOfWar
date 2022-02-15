#version 330 core
out vec4 FragColor;
out float FragDepth;
  
in vec3 ourColor;
in vec2 TexCoord;
in vec3 ourPos;
uniform sampler2D ourTexture;
uniform float team_id;

void main()
{
    vec4 tex =  texture(ourTexture, TexCoord);
    vec4 old = gl_FragColor;

    if(tex.a < 0.015)
        discard;

    float r = tex[0]*0.8;
    float g = tex[1]*0.8;
    float b = tex[2]*0.8;

    if(team_id == 0)
    {
       r = tex[0]*1.0;
       g = tex[1]*0.5;
       b = tex[2]*0.5;
    }

    if(team_id > 0.9 && team_id < 1.1)
    {
       r = tex[0]*0.5;
       g = tex[1]*1.0;
       b = tex[2]*0.5;
    }

    if(team_id > 1.9 && team_id < 2.1)
    {
       r = tex[0]*0.5;
       g = tex[1]*0.5;
       b = tex[2]*1.0;
    }

    gl_FragDepth = ourPos.z;
    FragColor = vec4(r,g,b,tex.a);
}