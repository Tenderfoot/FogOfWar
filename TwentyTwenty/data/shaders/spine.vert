in  vec3 in_Position;
in  vec2 in_TexCoord;
in  vec3 in_Color;

void main()
{
    // Transforming The Vertex
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
 
    // Passing The Texture Coordinate Of Texture Unit 0 To The Fragment Shader
    //texture_coordinate = vec2(gl_MultiTexCoord0);
}
