
#ifndef AnimatedModelShader_hpp
#define AnimatedModelShader_hpp

#include <stdio.h>
#include <iostream>
#include <assert.h>
#include "color.h"
#include "vector.h"
#include "matrix.h"
#include "camera.h"
#include "PhongShader.h"
#include "texture.h"
#include "ShaderLightMapper.h"

class SkinnedModelShader : public PhongShader
{
public:
	SkinnedModelShader();
	virtual ~SkinnedModelShader();
};

#endif /* AnimatedModelShader_hpp */
