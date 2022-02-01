#version 330 core
out vec4 FragColor;
out float FragDepth;
  
in vec3 ourColor;
in vec2 TexCoord;
in vec3 ourPos;
uniform sampler2D ourTexture;

void main()
{
    vec4 frag_pos = gl_FragCoord;
    vec3 light_pos = vec3(0.0,0.0,0.0);

    float x_dist = light_pos[0] - frag_pos[0];
    float y_dist = light_pos[1] - frag_pos[1];

    float dist = (x_dist*x_dist) + (y_dist*y_dist);

    vec4 tex =  texture(ourTexture, TexCoord);
    vec4 old = gl_FragColor;

    if(tex.a < 0.015)
        discard;

    float r = tex[0]*(3000000/dist);
    float g = tex[1]*(3000000/dist);
    float b = tex[2]*(3000000/dist);

    gl_FragDepth = ourPos.z;
    FragColor = vec4(r,g,b,tex.a);
}