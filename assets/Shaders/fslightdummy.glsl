#version 400
out vec4 FragColor;

const int MAX_LIGHTS=14;
struct Light
{
	int Type;
	vec3 Color;
	vec3 Position;
	vec3 Direction;
	vec3 Attenuation;
	vec3 SpotRadius;
	int ShadowIndex;
};

uniform Lights
{
	int LightCount;
	Light lights[MAX_LIGHTS];
};

void main()
{
	vec3 Color = vec3(0,0,0);
	for(int i=0; i<LightCount;i++)
		Color += lights[i].Color;

	FragColor = vec4(Color,1);
}
