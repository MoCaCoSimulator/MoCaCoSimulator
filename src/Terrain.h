#ifndef Terrain_hpp
#define Terrain_hpp

#include <stdio.h>
#include "basemodel.h"
#include "texture.h"
#include "vertexbuffer.h"
#include "indexbuffer.h"
#include "rgbimage.h"

class Terrain : public BaseModel
{
public:
    Terrain(Vector3 size, const char* HeightMap=NULL, const char* DetailMap1=NULL, const char* DetailMap2=NULL, const char* TopMap = NULL, const char* BotMap=NULL);
    virtual ~Terrain();
	bool load(const char* HeightMap, const char* DetailMap1, const char* DetailMap2, const char* TopMap, const char* BotMap);

    virtual void shader( BaseShader* shader, bool deleteOnDestruction=false );
    virtual void draw(const BaseCamera& Cam);
	const AABB& boundingBox() const { return BoundingBox; }
	float getHeight(float x, float z);
    
    //float width() const { return Size.X; }
    //float height() const { return Size.Y; }
    //float depth() const { return Size.Z; }

    //void width(float v) { Size.X = v; }
    //void height(float v) { Size.Y = v; }
    //void depth(float v) { Size.Z = v; }
    
    const Vector3& size() const { return Size; }
    void size(const Vector3& s) { Size = s; }
protected:
    void applyShaderParameter();
	void drawChunk(const BaseCamera& Cam, int chunk);
	void loadChunk(int chunkX, int chunkY, const Vector3* vertz, const Vector3* normals, int k);

	int NumSegX;
	int NumSegZ;
	int chunks;
	int chunksSide;
	int chunk_size = 256;
	float* Heights;
    VertexBuffer* VBs;
    IndexBuffer* IBs;
    Texture DetailTex[4];
    Texture MixTex;
    Texture HeightTex;
    Vector3 Size;
	AABB BoundingBox;
};



#endif /* Terrain_hpp */
