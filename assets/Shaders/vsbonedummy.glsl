#version 400

layout(location=0) in vec4 VertexPos;

const int MAX_BONES = 100;
uniform Bones
{
	int BoneCount;
	mat4 bones[MAX_BONES];
};

void main()
{
    gl_Position =  VertexPos;
}