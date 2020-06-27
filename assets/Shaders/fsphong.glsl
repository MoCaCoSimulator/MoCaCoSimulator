#version 400
#define M_PI 3.1415926535897932384626433832795
#define BIAS 0.00005

const int MAX_LIGHTS = 14;
const vec3 up = vec3( 0, 1, 0 );
const vec3 GlobalAmbientColor = vec3(0.5f, 0.5f, 0.5f);
const float AlphaCutoff = 0.15f;

in vec3 Position;
in vec3 Normal;
in vec2 Texcoord0;
in vec2 Texcoord1; //unused
in vec3 Tangente;
in vec3 Bitangente;
in vec4 VertexColor;

uniform vec3 DiffuseColor;
uniform vec3 AmbientColor;
uniform vec3 SpecularColor;
uniform float SpecularExp;
uniform sampler2D DiffuseTexture;
uniform sampler2D NormalTexture;
uniform vec3 LightPos; //unused
uniform vec4 LightColor; //unused
uniform vec3 EyePos;
uniform vec3 Fog;
uniform vec3 FogColor;
uniform float Alpha;
uniform vec3 GlobalDiffuseColor;

out vec4 FragColor;

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

float sat( in float a)
{
    return clamp(a, 0.f, 1.f);
}

void lighting( in vec3 E, in vec3 N, out vec3 DiffuseComponent, out vec3 SpecularComponent, out vec3 FogComponent ) {
	//Vector from fragment to light
	vec3 L;
	//intensity of light on this fragment
	float intensity;
	for(int i = 0; i < LightCount; i++)
	{
		Light light = lights[i];
		intensity = 1.f;

		//Directional light like sun
		if (light.Type == 1){
			L = -light.Direction;
		}
		else
		{
			L = normalize(light.Position - Position);
			float dist = distance(light.Position, Position);
			intensity /= light.Attenuation.x + light.Attenuation.y * dist + light.Attenuation.z * dist * dist;

			//Spot light
			if (light.Type == 2)
			{
				float sigma = acos(dot(-L, normalize(light.Direction)));
				intensity *= 1 - sat( ( sigma - light.SpotRadius.x ) / ( light.SpotRadius.y - light.SpotRadius.x ) );
			}
		}

		//Blinn components
		float ang = sat(dot(N,L));
		vec3 H = normalize(E + L);

		DiffuseComponent += light.Color * DiffuseColor * ang * intensity;
		//0 = SpecularExp
		SpecularComponent += light.Color * SpecularColor * pow(sat(dot(N,H)), SpecularExp) * intensity;
		SpecularComponent = vec3(0,0,0);
		FogComponent += light.Color * intensity;
	}
}

void main()
{
	vec4 DiffTex = texture( DiffuseTexture, Texcoord0 );

	if (DiffTex.a < AlphaCutoff)
		discard;

	vec3 NormalTex = texture( NormalTexture, Texcoord0 ).xyz;
	//normaltex colors ( 0 - 1 ) to -1 - 1 range
	NormalTex = NormalTex * 2 - 1;

	//fragment to cam direction
	vec3 E = normalize( EyePos-Position );
	vec3 N = normalize( mat3(Tangente,-Bitangente,Normal) * NormalTex );

	//Lighting
	vec3 SpecularComponent, FogComponent, DiffuseComponent;
	lighting( E, N, DiffuseComponent, SpecularComponent, FogComponent );

	vec3 color = GlobalDiffuseColor * (DiffTex.rgb * (DiffuseComponent + AmbientColor + GlobalAmbientColor) + SpecularComponent);
	FragColor = vec4(color, DiffTex.a * Alpha);
	//FragColor = VertexColor;
}
