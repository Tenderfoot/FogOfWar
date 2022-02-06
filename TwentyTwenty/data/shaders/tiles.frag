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
    vec4 old = gl_FragColor;

    vec2 water_uv_offset;
    water_uv_offset.x = sin(ourTime);
    water_uv_offset.y = sin(ourTime);
    
    float r;
    float g;
    float b;

    vec4 tex;
    if(ourType > 1.9 && ourType < 2.1) // water tile
    {
        tex = texture(ourTexture, TexCoord);
        if(tex[2] > (tex[0]+tex[1]))	// actual water (its blue!)
        {
            vec2 tiled_coords = TexCoord * 2;
            //tex = texture(waterTexture, tiled_coords + water_uv_offset);

             r =  ((sin(ourTime)*0.5))+(sin(gl_FragCoord.x/100)*0.5+sin(gl_FragCoord.y/100)*0.5);
             g =  ((sin(ourTime)*0.5))+(sin(gl_FragCoord.x/100)*0.5+sin(gl_FragCoord.y/100)*0.5);
             b = 1.0;

             r = tex[0]*0.8;
             g = tex[1]*0.8;
             b = tex[2]*0.8;
        }
        else  // not blue
        {
             r = tex[0]*0.8;
             g = tex[1]*0.8;
             b = tex[2]*0.8;
        }
    }
    else // not water
    {
        tex = texture(ourTexture, TexCoord);
        r = tex[0]*0.8;
        g = tex[1]*0.8;
        b = tex[2]*0.8;
    }

    gl_FragDepth = ourPos.z;
    FragColor = vec4(r,g,b,tex.a);
}