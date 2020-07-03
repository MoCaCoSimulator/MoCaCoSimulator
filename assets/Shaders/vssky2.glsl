#version 400
layout(location=0) in vec4 VertexPos;
layout(location=2) in vec2 VertexTexcoord0;

uniform mat4 ModelViewProjMat;

out vec3 Position;
out vec2 Texcoord;

void main() {
    Texcoord = VertexTexcoord0;
    Position = VertexPos.xyz;
    gl_Position = ModelViewProjMat * VertexPos;
}
