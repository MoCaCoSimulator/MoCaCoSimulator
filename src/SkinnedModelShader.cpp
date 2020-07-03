#include "SkinnedModelShader.h"
#include "Paths.h"
#include <string>
#include <qdebug.h>

SkinnedModelShader::SkinnedModelShader() : PhongShader(false)
{
	std::string VSFile = SHADER_DIRECTORY "vsphongskinned.glsl";
	std::string FSFile = SHADER_DIRECTORY "fsphong.glsl";
	if (!load(VSFile, FSFile))
		throw std::exception();
	PhongShader::assignLocations();
}

SkinnedModelShader::~SkinnedModelShader()
{
}