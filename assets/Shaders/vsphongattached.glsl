#version 400
layout(location=0) in vec4 VertexPos;
layout(location=1) in vec4 VertexNormal;
layout(location=2) in vec2 VertexTexcoord0;
layout(location=3) in vec2 VertexTexcoord1;
layout(location=4) in vec3 VertexTangente;
layout(location=5) in vec3 VertexBitangente;

const int JOINTCOUNT = 8;
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

uniform float AttachedJointWeights[JOINTCOUNT];
uniform int AttachedJointIDs[JOINTCOUNT];

const int JOINTCOLORS = 13;
const vec4 jointColors[JOINTCOLORS] = vec4[](
    vec4(1.00f,0.00f,0.00f,1), //red
    vec4(1.00f,1.00f,0.30f,1), //yellow
    vec4(0.00f,1.00f,0.00f,1), //green
    vec4(0.30f,0.65f,1.00f,1), //light blue
    vec4(1.00f,0.30f,0.53f,1), //pink
    vec4(0.00f,0.00f,1.00f,1), //blue
    vec4(0.30f,1.00f,0.42f,1), //light green
    vec4(1.00f,0.65f,0.30f,1), //light orange
    vec4(1.00f,0.20f,0.33f,1),
    vec4(0.00f,0.50f,0.60f,1),
    vec4(0.50f,0.00f,0.42f,1),
    vec4(0.60f,0.50f,0.00f,1),
    vec4(1.00f,0.80f,1.00f,1)
);

int computeColorID(int jointID)
{
    return int(mod(float(jointID), float(JOINTCOLORS)));
}

float greatestComponent(vec4 color)
{
    float greatest = color.r;
    if (greatest < color.g)
        greatest = color.g;
    if (greatest < color.b)
        greatest = color.b;
    return greatest;
}

vec4 normalizeColor(vec4 color)
{
    float greatest = greatestComponent(color);

    return color / greatest;
}

vec4 computeJointColor()
{
    int weightCount = 0;
    vec4 jointColor = vec4(0,0,0,0);

    float jointWeight;
    int jointID;
    for (int i = 0; i < JOINTCOUNT; i++)
    {
        float weight = AttachedJointWeights[i];
        int index = AttachedJointIDs[i];

        if (jointWeight == 0.0f)
            break;

        weightCount++;
        
        jointColor += jointColors[computeColorID(jointID)] * jointWeight;
    }

    jointColor /= weightCount;
    jointColor.a = 1.0f;
    return normalizeColor(jointColor);
}


mat4 loadBoneTransform()
{
    mat4 boneTransform = mat4(0.0);

    for(int i = 0; i < JOINTCOUNT; i++)
    {
        float weight = AttachedJointWeights[i];
        int index = AttachedJointIDs[i];

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
    
    vec4 animMat = boneTransform * VertexPos;
    vec4 worldPos = ModelMat * animMat;
    Position = worldPos.xyz;
    vec4 curPos = ModelViewProjMat * animMat;
    gl_Position = curPos;
    VertexColor = computeJointColor();
}
