#version 400
layout(location=0) in vec4 VertexPos;
layout(location=1) in vec4 VertexNormal;
layout(location=2) in vec2 VertexTexcoord0;
layout(location=3) in vec2 VertexTexcoord1;
layout(location=4) in vec3 VertexTangente;
layout(location=5) in vec3 VertexBitangente;
layout(location=6) in vec4 JointIDs0;
layout(location=7) in vec4 JointIDs1;
layout(location=8) in vec4 JointWeights0;
layout(location=9) in vec4 JointWeights1;

const int JOINTCOUNT = 8;
const int JOINTSIZE = 4;
const int MAX_BONES = 100;

out vec3 Position;
out vec3 Normal;
out vec2 Texcoord0;
out vec2 Texcoord1;
out vec3 Tangente;
out vec3 Bitangente;
out vec4 VertexColor;

uniform mat4 ModelMat;
uniform mat4 ModelViewProjMat;
uniform mat4 Bones[MAX_BONES];

void getJointDetails(int index, out float weight, out int id)
{
    int vectorIndex = index / JOINTSIZE;
    int componentIndex = index % JOINTSIZE;

    vec4 weights, ids;
    if (vectorIndex == 0)
    {
        weights = JointWeights0;
        ids = JointIDs0;
    }
    else if (vectorIndex == 1)
    {
        weights = JointWeights1;
        ids = JointIDs1;
    }

    weight = weights[componentIndex];
    id = int(ids[componentIndex]);
}

mat4 loadBoneTransform()
{
    mat4 boneTransform = mat4(0.0);

    for(int i = 0; i < JOINTCOUNT; i++)
    {
        float weight;
        int index;
        getJointDetails(i, weight, index);

        if (weight == 0.0f)
            break;

        boneTransform += Bones[index] * weight;
    }

    return boneTransform;
}

void main()
{
    mat4 boneTransform = loadBoneTransform();    

    Normal = (ModelMat * boneTransform * vec4(VertexNormal.xyz,0)).xyz;
    Tangente = (ModelMat * boneTransform * vec4(VertexTangente.xyz,0)).xyz;
    Bitangente = (ModelMat * boneTransform * vec4(VertexBitangente.xyz,0)).xyz;
    Texcoord0 = VertexTexcoord0;
    Texcoord1 = VertexTexcoord1;
    
    vec4 worldPos = ModelMat * boneTransform * VertexPos;
    Position = worldPos.xyz;
    vec4 curPos = ModelViewProjMat * boneTransform * VertexPos;
    gl_Position = curPos;
}
