
#ifndef ConstantShader_hpp
#define ConstantShader_hpp

#include <stdio.h>
#include <iostream>
#include <assert.h>
#include "color.h"
#include "vector.h"
#include "matrix.h"
#include "camera.h"
#include "baseshader.h"


class ConstantShader : public BaseShader
{
public:
    ConstantShader();
    void color( const Color& c);
    const Color& color() const { return Col; }
    virtual void activate(const BaseCamera& Cam) const;
private:
    Color Col;
    GLuint ShaderProgram;
    GLint ColorLoc;
    GLint ModelViewProjLoc;
    
};

#endif /* ConstantShader_hpp */
