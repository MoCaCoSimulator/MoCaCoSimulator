#version 400
layout(location=0) in vec4 VertexPos;
layout(location=1) in vec4 VertexNormal;
layout(location=2) in vec2 VertexTexcoord0;
layout(location=3) in vec2 VertexTexcoord1;
layout(location=4) in vec3 VertexTangente;
layout(location=5) in vec3 VertexBitangente;

out vec3 Position;
out vec3 Normal;
out vec2 Texcoord0;
out vec2 Texcoord1;
out vec3 Tangente;
out vec3 Bitangente;
out vec4 VertexColor;

uniform mat4 ModelMat;
uniform mat4 ModelViewProjMat;

void main()
{
    Normal = (ModelMat * vec4(VertexNormal.xyz,0)).xyz;
    Tangente = (ModelMat * vec4(VertexTangente.xyz,0)).xyz;
    Bitangente = (ModelMat * vec4(VertexBitangente.xyz,0)).xyz;
    Texcoord0 = VertexTexcoord0;
    Texcoord1 = VertexTexcoord1;
    
    vec4 worldPos = ModelMat * VertexPos;
    Position = worldPos.xyz;
    vec4 curPos = ModelViewProjMat * VertexPos;
    gl_Position = curPos;
}
