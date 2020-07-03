
#include "ConstantShader.h"

const char *CVertexShaderCode =
"#version 400\n"
"in vec4 VertexPos;"
"uniform mat4 ModelViewProjMat;"
"void main()"
"{"
"    gl_Position = ModelViewProjMat * VertexPos;"
"}";

const char *CFragmentShaderCode =
"#version 400\n"
"uniform vec3 Color;"
"uniform float Alpha;"
"out vec4 FragColor;"
"void main()"
"{"
"    FragColor = vec4(Color, Alpha);"
"}";

ConstantShader::ConstantShader() : Col(1.0f,1.0f,1.0f)
{
    ShaderProgram = createShaderProgram( CVertexShaderCode, CFragmentShaderCode );
    
    ColorLoc = glGetUniformLocation(ShaderProgram, "Color");
	AlphaLoc = glGetUniformLocation(ShaderProgram, "Alpha");
    //assert(ColorLoc>=0);
    ModelViewProjLoc  = glGetUniformLocation(ShaderProgram, "ModelViewProjMat");
    //assert(ModelViewProjLoc>=0);
    
}
void ConstantShader::activate(const BaseCamera& Cam) const
{
    BaseShader::activate(Cam);
    
    glUniform3f(ColorLoc, Col.R, Col.G, Col.B);
	glUniform1f(AlphaLoc, Alpha);
    // always update matrices
    Matrix ModelView = Cam.getViewMatrix() * ModelTransform;
    Matrix ModelViewProj = Cam.getProjectionMatrix() * ModelView;
    glUniformMatrix4fv(ModelViewProjLoc, 1, GL_FALSE, ModelViewProj.m);
}
void ConstantShader::color( const Color& c)
{
    Col = c;
}

