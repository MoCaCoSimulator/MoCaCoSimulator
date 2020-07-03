#version 400
const int MAX_LIGHTS=14;

in vec2 Texcoord;
in vec3 Position;

//Phong Shader
uniform sampler2D DiffuseTexture; //only used
//remove diffuse maybe

uniform sampler2D DayTexture;
uniform sampler2D NightTexture;
uniform vec3 SunDirection;
uniform vec3 SunColor;

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

const vec3 up = vec3( 0, 1, 0 );

float sat( in float a)
{
    return clamp(a, 0.0, 1.0);
}

//Calc horizon color containing all directional light colors
vec3 lighting() {
  float intensity = 1.f;
  vec3 horizonComponent = vec3(0);
  for(int i = 0; i < LightCount; i++)
	{
		Light light = lights[i];

		//Directional light like sun
		if (light.Type == 1)
      horizonComponent += light.Color * intensity;
	}
  return horizonComponent;
}

float spread(float min, float max, float value) {
	float range = max - min;
	float factor = 1 / range;

	return sat((value - min) * factor);
}

void main() {
    //Keeping it within 0 - 1
    vec3 sunDir = normalize(SunDirection);
    vec3 dayColor = texture( DayTexture, Texcoord ).rgb;
    vec3 nightColor = texture( NightTexture, Texcoord ).rgb;
    
    //sun angle
    float sunAng = dot(up, sunDir) * 0.5f + 0.5f;
    //generic horizon color level
    float horizonAngle = -dot( normalize(Position), up);
    //smooth horizon
    float horizonLevel = sat( (horizonAngle < 0 ? pow ( horizonAngle + 1, 2 ) : 1) );

    //dont display sun if under horizon
    float sunLevelGlow = 0;
    float sunLevelWhite = 0;
    if( horizonAngle < 0 )
    {
      //calc where to display the sun
      float sunLevel = dot( normalize(Position), -sunDir );
      //smooth sun edges
      sunLevelGlow = pow(spread(0.9993f, 0.9997f, sunLevel), 30);
      sunLevelWhite = spread(0.9996f, 0.9997f, sunLevel);
    }

    vec3 finalColor = vec3(0);
    //mix day and night texture
    finalColor = mix( dayColor, nightColor, sunAng);
    //mix generic horizon color on bottom
    finalColor = mix(finalColor, lighting(), horizonLevel);
    //mix sun into texture
    finalColor = mix(finalColor, SunColor, sunLevelGlow);
    finalColor = mix(finalColor, vec3(1), sunLevelWhite);

    FragColor = vec4( finalColor, 1 );
}
