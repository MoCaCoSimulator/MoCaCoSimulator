
#include "WeightShader.h"
#include "Paths.h"

WeightShader::WeightShader()
{
	bool loaded = load(SHADER_DIRECTORY "vsweight.glsl", SHADER_DIRECTORY "fsweight.glsl");
	if (!loaded)
		throw std::exception();

	ModelViewProjLoc = glGetUniformLocation(ShaderProgram, "ModelViewProjMat");
	//assert(ModelViewProjLoc>=0);

}

void WeightShader::activate(const BaseCamera& Cam) const
{
	BaseShader::activate(Cam);

	// always update matrices
	Matrix ModelView = Cam.getViewMatrix() * ModelTransform;
	Matrix ModelViewProj = Cam.getProjectionMatrix() * ModelView;
	glUniformMatrix4fv(ModelViewProjLoc, 1, GL_FALSE, ModelViewProj.m);
}
