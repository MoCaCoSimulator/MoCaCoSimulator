//
//  PhongShader.hpp
//  ogl4
//
//  Created by Philipp Lensing on 16.09.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#ifndef PhongShader_hpp
#define PhongShader_hpp

#include <stdio.h>
#include <iostream>
#include <assert.h>
#include "color.h"
#include "vector.h"
#include "matrix.h"
#include "camera.h"
#include "baseshader.h"
#include "texture.h"
#include "ShaderLightMapper.h"

class PhongShader : public BaseShader
{
public:
    PhongShader(bool LoadStaticShaderCode=true);
	virtual ~PhongShader();
	int activateTex(const Texture* pTex, GLint Loc, int slot) const;
	void deactivateTex(const Texture* pTex, GLint Loc) const;
    // setter
    void diffuseColor( const Color& c);
    void ambientColor( const Color& c);
    void specularColor( const Color& c);
    void specularExp( float exp);
    void diffuseTexture(const Texture* pTex);
	void normalTexture(const Texture* pTex);
    void lightPos( const Vector3& pos);
    void lightColor(const Color& c);
	void shadowMap(unsigned int slot, const Texture* pTex, const Matrix& Mtx);
    //getter
    const Color& diffuseColor() const { return DiffuseColor; }
    const Color& ambientColor() const { return AmbientColor; }
    const Color& specularColor() const { return SpecularColor; }
    float specularExp() const { return SpecularExp; }
    const Texture* diffuseTexture() const { return DiffuseTexture; }
	const Texture* normalTexture() const { return NormalTexture; }
    const Vector3& lightPos() const { return LightPos; }
    const Color& lightColor() const { return LightColor; }

    virtual void activate(const BaseCamera& Cam) const;
	virtual void deactivate() const;
protected:
    void assignLocations();
	unsigned int TexCount;

	const Texture* ShadowMapTexture[MaxShadowCount];
	Matrix ShadowMapMat[MaxShadowCount];

	GLint ShadowMapTextureLoc[MaxShadowCount];
	GLint ShadowMapMatLoc[MaxShadowCount];
	GLint DiffuseTexLoc;
	GLint NormalTexLoc;

private:
    Color DiffuseColor;
    Color SpecularColor;
    Color AmbientColor;
    float SpecularExp;
    Vector3 LightPos;
    Color LightColor;
    const Texture* DiffuseTexture;
	const Texture* NormalTexture;

    GLint DiffuseColorLoc;
    GLint SpecularColorLoc;
    GLint AmbientColorLoc;
    GLint SpecularExpLoc;
    GLint LightPosLoc;
    GLint LightColorLoc;
    GLint ModelMatLoc;
    GLint ModelViewProjLoc;
    GLint EyePosLoc;
	GLint AlphaLoc;
    
    mutable unsigned int UpdateState;
    
    enum UPDATESTATES
    {
        DIFF_COLOR_CHANGED = 1<<0,
        AMB_COLOR_CHANGED = 1<<1,
        SPEC_COLOR_CHANGED = 1<<2,
        SPEC_EXP_CHANGED = 1<<3,
        LIGHT_POS_CHANGED = 1<<4,
        LIGHT_COLOR_CHANGED = 1<<5,
        DIFF_TEX_CHANGED = 1<<6,
		NORM_TEX_CHANGED = 1<<7,
    };
    
};

#endif /* PhongShader_hpp */
