
#ifndef LineCubeModel_hpp
#define LineCubeModel_hpp

#include <stdio.h>
#include "basemodel.h"
#include "vertexbuffer.h"

class LineBoxModel : public BaseModel
{
public:
	LineBoxModel(float Width = 1, float Height = 1, float Depth = 1);
    virtual ~LineBoxModel() {}
    virtual void draw(const BaseCamera& Cam);
protected:
	VertexBuffer VB;
    
};

#endif /* LineCubeModel_hpp */
