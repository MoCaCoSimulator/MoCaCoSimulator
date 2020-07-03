#include "AttachedModelShader.h"
#include "Paths.h"

AttachedModelShader::AttachedModelShader() : PhongShader(false)
{
	std::string VSFile = SHADER_DIRECTORY "vsphongattached.glsl";
	std::string FSFile = SHADER_DIRECTORY "fsphong.glsl";
	if (!load(VSFile, FSFile))
		throw std::exception();
	PhongShader::assignLocations();
}

AttachedModelShader::~AttachedModelShader()
{
}

/*
void AttachedModelShader::activate(const BaseCamera& Cam) const
{
	PhongShader::activate(Cam);
}

void AttachedModelShader::deactivate() const
{
	PhongShader::deactivate();
}
*/
