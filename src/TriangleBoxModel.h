
#ifndef TriangleBoxModel_hpp
#define TriangleBoxModel_hpp

#include <stdio.h>

#include <stdio.h>
#include "basemodel.h"
#include "vertexbuffer.h"
#include "indexbuffer.h"

class TriangleBoxModel : public BaseModel
{
public:
    TriangleBoxModel(float Width=1, float Height=1, float Depth=1 );
    virtual ~TriangleBoxModel() {}
    virtual void draw(const BaseCamera& Cam);
protected:
    VertexBuffer VB;
    IndexBuffer IB;
};


#endif /* TriangleBoxModel_hpp */
