#include "ShaderLightMapper.h"
#include "BaseShader.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "paths.h"

ShaderLightMapper* ShaderLightMapper::pMapper = NULL;

float toRad(float deg)
{
	return deg * (float)M_PI / 180.0f;
}

ShaderLightMapper& ShaderLightMapper::instance()
{
	if (!pMapper)
		pMapper = new ShaderLightMapper();
	return *pMapper;
}

ShaderLightMapper::ShaderLightMapper()
{
	BaseShader Shader;
	bool loaded = Shader.load(SHADER_DIRECTORY "vslightdummy.glsl", SHADER_DIRECTORY "fslightdummy.glsl");
	assert(loaded);

	GLuint BlockIndex = glGetUniformBlockIndex(Shader.openGLProgramID(), "Lights");
	assert(BlockIndex != GL_INVALID_INDEX);

	GLint BlockSize = 0;
	glGetActiveUniformBlockiv(Shader.openGLProgramID(), BlockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &BlockSize);

    // If the following assertion fails:
    // You have to adjust the shaderLightBlock structure.
    // The ShaderLightBlock structure does not match the structure size of the compiled shader block.
    // Please check BlockSize to obtain the actual size of the shader structure and update
    // the ShaderLightBlock structure of this class. The size of ShaderLightBlock should fit to BlockSize!
	assert(sizeof(ShaderLightBlock) == BlockSize);

	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(ShaderLightBlock), NULL, GL_DYNAMIC_DRAW);
	
}

ShaderLightMapper::~ShaderLightMapper()
{
	clear();
}

void ShaderLightMapper::addLight(BaseLight* Light)
{
	assert(Light);

	if (lights().size() >= MaxLightCount)
		return;

	Lights.push_back(Light);
}


void ShaderLightMapper::activate()
{
	ShaderLights.LightCount = (int)lights().size();
	int ShadowCasterIndex = 0;

	for (int i = 0; i<ShaderLights.LightCount; i++)
	{
		BaseLight* L = lights()[i];
		ShaderLight* SL = &ShaderLights.lights[i];

		SL->Color = L->color();
		SL->Attenuation = L->attenuation();
		SL->Position = L->position();
		SL->Type = L->type();
		SL->ShadowIndex = L->castShadows() ? ShadowCasterIndex++ : -1;

		switch (L->type())
		{
			case BaseLight::DIRECTIONAL:
			{
				DirectionalLight* dl = dynamic_cast<DirectionalLight*>(L);
				SL->Direction = dl->direction();
				SL->Direction.normalize();
				break;
			}
			case BaseLight::SPOT:
			{
				SpotLight* sl = dynamic_cast<SpotLight*>(L);
				SL->Direction = sl->direction();
				SL->SpotRadius = Vector3( toRad(sl->innerRadius()), toRad(sl->outerRadius()), 0);
				SL->Direction.normalize();
				break;
			}
            case BaseLight::POINT:
                break;
		}
	}

	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ShaderLightBlock), &ShaderLights);
}

void ShaderLightMapper::deactivate()
{

}

void ShaderLightMapper::clear()
{
	for (BaseLight* pLight : Lights)
		delete pLight;

	Lights.clear();
}