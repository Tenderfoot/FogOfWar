in vec4 gl_FragCoord;
varying vec2 texture_coordinate; 
uniform sampler2D my_color_texture;
uniform float depth;

void main()
{
    vec4 test = texture2D(my_color_texture, texture_coordinate);
    
    if(test.a < 0.015)
    {
       discard;
    }

    gl_FragDepth = depth;
    gl_FragColor = vec4(test.r, test.g, test.b, test.a);
}

