#version 400
layout(location=0) in vec4 VertexPos;
layout(location=2) in vec2 VertexTexcoord0;

uniform mat4 ModelMat;
uniform mat4 ModelViewProjMat;

out vec3 Position;
out vec2 Texcoord;

void main() {
    Texcoord = VertexTexcoord0;
    Position = (ModelMat * VertexPos).xyz;
    gl_Position = mat4( ModelViewProjMat[0], ModelViewProjMat[1], ModelViewProjMat[2], vec4(0.8,0.8,0.8,1) ) * VertexPos;
}
