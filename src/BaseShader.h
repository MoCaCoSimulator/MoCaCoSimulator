//
//  BaseShader.hpp
//  ogl4
//
//  Created by Philipp Lensing on 19.09.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#ifndef BaseShader_hpp
#define BaseShader_hpp

#include <stdio.h>
#include <iostream>
#include <assert.h>
#include <GL/glew.h>
#include "color.h"
#include "vector.h"
#include "matrix.h"
#include "camera.h"

class BaseShader
{
public:
    BaseShader();
	virtual ~BaseShader();
    virtual void modelTransform(const Matrix& m) { ModelTransform = m; }
    virtual const Matrix& modelTransform() const { return ModelTransform; }
    
    virtual void activate(const BaseCamera& Cam) const;
    virtual void deactivate() const;
    
    bool load( const std::string& VertexShaderFile, const std::string& FragmentShaderFile );
    GLint getParameterID(const char* ParamenterName) const;
	GLuint getBlockID(const char* BlockName) const;

	const float& alpha() const { return Alpha; }
	void alpha(const float& alpha);
	const Color& color() const { return GlobalDiffuseColor; }
	void color(const Color& color);

	void setBlock(GLuint ID, GLuint UniformBufferID) const;
    void setParameter( GLint ID, float Param) const;
    void setParameter( GLint ID, int Param) const;
    void setParameter( GLint ID, const Vector3& Param) const;
    void setParameter( GLint ID, const Color& Param) const;
    void setParameter( GLint ID, const Matrix& Param) const;

	GLuint openGLProgramID() { return ShaderProgram; }
protected:
    char* loadFile( const std::string& File, unsigned int& Filesize );
    GLuint createShaderProgram( const char* VScode, const char* FScode );
    Matrix ModelTransform;
	float Alpha;
	bool DisableZTest;
	Color GlobalDiffuseColor;
	GLuint FS;
	GLuint VS;

    GLuint ShaderProgram;
	GLuint LightUniformBuffer;
	GLint AlphaLoc;
	GLint ColorLoc;
    
    static const BaseShader* ShaderInPipe;
};

#endif /* BaseShader_hpp */
