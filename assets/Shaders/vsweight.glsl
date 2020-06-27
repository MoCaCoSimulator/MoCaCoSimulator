#version 400
layout(location=0) in vec4 VertexPos;
layout(location=6) in vec4 JointIDs0;
layout(location=7) in vec4 JointIDs1;
layout(location=8) in vec4 JointWeights0;
layout(location=9) in vec4 JointWeights1;

const int JOINTCOUNT = 8;
const int JOINTSIZE = 4;
const int JOINTCOLORS = 23;
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
    vec4(1.00f,0.80f,1.00f,1),
    vec4(0.62f, 0.322f, 0.176f,1),
    vec4(1.00f, 0.894f, 0.769f,1),
    vec4(0.498f, 1.000f, 0.831f,1),
    vec4(0.545f, 0.000f, 0.545f,1),
    vec4(0.647f, 0.165f, 0.165f,1),
    vec4(0.467f, 0.533f, 0.600f,1),
    vec4(0.961f, 0.961f, 0.961f,1),
    vec4(0.722f, 0.525f, 0.043f,1),
    vec4(0.502f, 0.502f, 0.000f,1),
    vec4(0.282f, 0.239f, 0.545f,1)
);

out vec3 Position;
out vec4 VertexColor;

uniform mat4 ModelViewProjMat;

/*uniform Bones
{
	int BoneCount;
	mat4 bones[MAX_BONES];
};*/

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

vec4 computeJointColor()
{
    int weightCount = 0;
    vec4 jointColor = vec4(0,0,0,0);

    float jointWeight;
    int jointID;
    for (int i = 0; i < JOINTCOUNT; i++)
    {
        getJointDetails(i, jointWeight, jointID);
        if (jointWeight == 0.0f)
            break;

        weightCount++;
        
        jointColor += jointColors[computeColorID(jointID)] * jointWeight;
    }

    jointColor /= weightCount;
    jointColor.a = 1.0f;
    return normalizeColor(jointColor);
}

void main()
{
    VertexColor = computeJointColor();
    gl_Position = ModelViewProjMat * VertexPos;
}
