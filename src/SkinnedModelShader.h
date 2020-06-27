#ifndef SkinnedModelShader_hpp
#define SkinnedModelShader_hpp

#include "PhongShader.h"
#include "VertexBuffer.h"

class SkinnedModelShader : public PhongShader
{
public:
	SkinnedModelShader();
	virtual ~SkinnedModelShader();
};

#endif /* SkinnedModelShader_hpp */