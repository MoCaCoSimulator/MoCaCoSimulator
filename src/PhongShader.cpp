//
//  PhongShader.cpp
//  ogl4
//
//  Created by Philipp Lensing on 16.09.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#include "PhongShader.h"
#include "Paths.h"
#include <string>

PhongShader::PhongShader(bool LoadStaticShaderCode) :
	DiffuseColor(0.8f,0.8f,0.8f),
	SpecularColor(0.5f,0.5f,0.5f),
	AmbientColor(0.2f,0.2f,0.2f),
	SpecularExp(20.0f),
	LightPos(20.0f,20.0f,20.0f),
	LightColor(1,1,1),
	DiffuseTexture(Texture::defaultTex()),
	NormalTexture(Texture::defaultNormalTex()),
	UpdateState(0xFFFFFFFF),
	TexCount(0)
{
    if(!LoadStaticShaderCode)
        return;

	bool loaded = load(SHADER_DIRECTORY "vsphong.glsl", SHADER_DIRECTORY "fsphong.glsl");
	if (!loaded)
		throw std::exception();
    assignLocations();
}

PhongShader::~PhongShader()
{

}

void PhongShader::assignLocations()
{
    DiffuseColorLoc = glGetUniformLocation(ShaderProgram, "DiffuseColor");
    AmbientColorLoc = glGetUniformLocation(ShaderProgram, "AmbientColor");
    SpecularColorLoc = glGetUniformLocation(ShaderProgram, "SpecularColor");
    SpecularExpLoc = glGetUniformLocation(ShaderProgram, "SpecularExp");
    DiffuseTexLoc = glGetUniformLocation(ShaderProgram, "DiffuseTexture");
	if (DiffuseTexLoc >= 0)
		TexCount++;
	NormalTexLoc = glGetUniformLocation(ShaderProgram, "NormalTexture");
	if (NormalTexLoc >= 0)
		TexCount++;
    LightPosLoc = glGetUniformLocation(ShaderProgram, "LightPos");
    LightColorLoc = glGetUniformLocation(ShaderProgram, "LightColor");
    EyePosLoc = glGetUniformLocation(ShaderProgram, "EyePos");
    ModelMatLoc = glGetUniformLocation(ShaderProgram, "ModelMat");
	ModelViewProjLoc = glGetUniformLocation(ShaderProgram, "ModelViewProjMat");
	
	AlphaLoc = glGetUniformLocation(ShaderProgram, "Alpha");
	ColorLoc = glGetUniformLocation(ShaderProgram, "GlobalDiffuseColor");

	for (int i = 0; i < MaxShadowCount; ++i)
	{
		std::string smt = "ShadowMapTexture[" + std::to_string(i) + "]";
		std::string smm = "ShadowMapMat[" + std::to_string(i) + "]";
		ShadowMapTextureLoc[i] = getParameterID(smt.c_str());
		ShadowMapMatLoc[i] = getParameterID(smm.c_str());
		ShadowMapTexture[i] = NULL;
		ShadowMapMat[i].setIdentity();
	}
}

void PhongShader::activate(const BaseCamera& Cam) const
{
    BaseShader::activate(Cam);

    // update uniforms if necessary
    if(UpdateState&DIFF_COLOR_CHANGED)
		setParameter(DiffuseColorLoc, DiffuseColor);
	if (UpdateState&AMB_COLOR_CHANGED)
		setParameter(AmbientColorLoc, AmbientColor);
    if(UpdateState&SPEC_COLOR_CHANGED)
		setParameter(SpecularColorLoc, SpecularColor);
	if (UpdateState&SPEC_EXP_CHANGED)
		setParameter(SpecularExpLoc, SpecularExp);

	int TexSlotIdx = 0;
	
	if (DiffuseTexLoc >= 0)
	{
		DiffuseTexture->activate(TexSlotIdx);
		if(UpdateState&DIFF_TEX_CHANGED && DiffuseTexture)
			setParameter(DiffuseTexLoc, TexSlotIdx);
		TexSlotIdx++;
	}
	if (NormalTexLoc >= 0)
	{
		NormalTexture->activate(TexSlotIdx);
		if (UpdateState&NORM_TEX_CHANGED && NormalTexture)
			setParameter(NormalTexLoc, TexSlotIdx);
		TexSlotIdx++;
	}
	
    if(UpdateState&LIGHT_COLOR_CHANGED)
		setParameter(LightColorLoc, LightColor);
    if(UpdateState&LIGHT_POS_CHANGED)
		setParameter(LightPosLoc, LightPos);

	setParameter(AlphaLoc, Alpha);
	setParameter(ColorLoc, GlobalDiffuseColor);

    Matrix ModelViewProj = Cam.getProjectionMatrix() * Cam.getViewMatrix() * modelTransform();
	setParameter(ModelMatLoc, modelTransform());
	setParameter(ModelViewProjLoc, ModelViewProj);

    Vector3 EyePos = Cam.position();
	setParameter(EyePosLoc, EyePos);

	for (int i = 0; i < MaxShadowCount; ++i)
	{
		if (ShadowMapTexture[i] && ShadowMapTextureLoc[i] > 0)
		{
			setParameter(ShadowMapMatLoc[i], ShadowMapMat[i]);
			TexSlotIdx = activateTex(ShadowMapTexture[i], ShadowMapTextureLoc[i], TexSlotIdx);
		}
	}
	
    UpdateState = 0x0;
}

void PhongShader::deactivate() const
{
	for (int i = 0; i < MaxShadowCount; ++i)
		if (ShadowMapTexture[i] && ShadowMapTextureLoc[i] > 0)
			deactivateTex(ShadowMapTexture[i],ShadowMapTextureLoc[i]);
	deactivateTex(NormalTexture, NormalTexLoc);
	deactivateTex(DiffuseTexture, DiffuseTexLoc);

	BaseShader::deactivate();
}

void PhongShader::shadowMap(unsigned int slot, const Texture* pTex, const Matrix& Mtx)
{
	if (slot >= MaxShadowCount)
		return;

	ShadowMapTexture[slot] = pTex;
	ShadowMapMat[slot] = Mtx;
}
void PhongShader::diffuseColor( const Color& c)
{
    DiffuseColor = c;
    UpdateState |= DIFF_COLOR_CHANGED;
}
void PhongShader::ambientColor( const Color& c)
{
    AmbientColor = c;
    UpdateState |= AMB_COLOR_CHANGED;
}
void PhongShader::specularColor( const Color& c)
{
    SpecularColor = c;
    UpdateState |= SPEC_COLOR_CHANGED;
}
void PhongShader::specularExp( float exp)
{
    SpecularExp = exp;
    UpdateState |= SPEC_EXP_CHANGED;
}
void PhongShader::lightPos( const Vector3& pos)
{
    LightPos = pos;
    UpdateState |= LIGHT_POS_CHANGED;
}
void PhongShader::lightColor(const Color& c)
{
    LightColor = c;
    UpdateState |= LIGHT_COLOR_CHANGED;
}

void PhongShader::diffuseTexture(const Texture* pTex)
{
    DiffuseTexture = pTex;
    if(!DiffuseTexture)
        DiffuseTexture = Texture::defaultTex();
    
    UpdateState |= DIFF_TEX_CHANGED;
}

void PhongShader::normalTexture(const Texture* pTex)
{
	NormalTexture = pTex;
	if (!NormalTexture)
		NormalTexture = Texture::defaultNormalTex();

	UpdateState |= NORM_TEX_CHANGED;
}

int PhongShader::activateTex(const Texture* pTex, GLint Loc, int slot) const
{
	if (!pTex || Loc < 0)
		return slot;
	pTex->activate(slot);
	setParameter(Loc, slot);
	return slot + 1;
}

void PhongShader::deactivateTex(const Texture* pTex, GLint Loc) const
{
	if (!pTex || Loc < 0)
		return;
	pTex->deactivate();
}