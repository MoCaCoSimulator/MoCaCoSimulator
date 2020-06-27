#ifndef LIGHTMANAGER_H
#define LIGHTMANAGER_H

#include <vector>
#include <GL/glew.h>
#include "lights.h"

#define MaxLightCount 14
#define MaxShadowCount 1

class ShaderLightMapper
{
public:
	typedef std::vector<BaseLight*> LightList;

	void addLight(BaseLight* Light);
	const LightList& lights() const { return Lights; }
	void clear();

	void activate();
	void deactivate();
	static ShaderLightMapper& instance();
	GLuint uniformBlockID() { return UBO;  }
protected:
	struct ShaderLight
	{
		int Type; Vector3 padding5;
		Color Color; float padding0;
		Vector3 Position; float padding1;
		Vector3 Direction; float padding2;
		Vector3 Attenuation; float padding3;
		Vector3 SpotRadius;
		int ShadowIndex;
	};

	struct ShaderLightBlock
	{
		int LightCount; Vector3 padding0;
		ShaderLight lights[MaxLightCount];

	};
protected:
	ShaderLightMapper();
	ShaderLightMapper(const ShaderLightMapper& m) {}
	~ShaderLightMapper();
	LightList Lights;
	ShaderLightBlock ShaderLights;
	GLuint UBO;
	static ShaderLightMapper* pMapper;
};

#endif

