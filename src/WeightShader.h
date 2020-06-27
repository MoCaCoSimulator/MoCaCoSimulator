//
//  PhongShader.hpp
//  ogl4
//
//  Created by Philipp Lensing on 16.09.16.
//  Copyright © 2016 Philipp Lensing. All rights reserved.
//

#ifndef WeightShader_hpp
#define WeightShader_hpp

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

class WeightShader : public BaseShader
{
public:
	WeightShader();
    virtual void activate(const BaseCamera& Cam) const;
protected:
	GLuint ModelViewProjLoc;
};

#endif /* PhongShader_hpp */
