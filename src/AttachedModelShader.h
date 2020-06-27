
#ifndef AttachedModelShader_hpp
#define AttachedModelShader_hpp

#include "PhongShader.h"
#include "VertexBuffer.h"

class AttachedModelShader : public PhongShader
{
public:
	AttachedModelShader();
	virtual ~AttachedModelShader();
	//virtual void activate(const BaseCamera& Cam) const;
	//virtual void deactivate() const;
protected:
};

#endif /* AttachedModelShader_hpp */
