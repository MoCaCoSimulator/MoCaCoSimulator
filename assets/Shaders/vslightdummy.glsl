#version 400

layout(location=0) in vec4 VertexPos;



void main()
{

    gl_Position =  VertexPos;
}