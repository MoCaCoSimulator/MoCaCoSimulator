#include "Terrain.h"
#include "rgbimage.h"
#include "Terrainshader.h"
#include <algorithm>

Terrain::Terrain(Vector3 size, const char* HeightMap, const char* DetailMap1, const char* DetailMap2, const char* TopMap, const char* BotMap)
{
	BoundingBox = AABB::unitBox();
	Size = size;
    if(HeightMap && DetailMap1 && DetailMap2 && TopMap && BotMap)
    {
        bool loaded = load( HeightMap, DetailMap1, DetailMap2, TopMap, BotMap);
        if(!loaded)
            throw std::exception();
    }
}

Terrain::~Terrain()
{
	if (Heights != NULL)
		delete[] Heights;
	if (VBs != NULL)
		delete[] VBs;
	if (IBs != NULL)
		delete[] IBs;
}

void Terrain::loadChunk(int chunkX, int chunkZ, const Vector3* vertz, const Vector3* normals, int k) {
	int chunk = chunkX * chunksSide + chunkZ;
	printf("loadChunk x - %d y - %d ( %d )\n", chunkX, chunkZ, chunk);
	VertexBuffer* VB = &VBs[chunk];
	IndexBuffer* IB = &IBs[chunk];

	printf("VB begin");

	int chunkSegXstart = chunkX * chunk_size;
	int chunkSegZstart = chunkZ * chunk_size;

	int chunkSegXsize = std::min( NumSegX - chunkX * chunk_size, chunk_size + 1);
	int chunkSegZsize = std::min( NumSegZ - chunkZ * chunk_size, chunk_size + 1);

	VB->begin();
	for (int i = chunkSegXstart; i < chunkSegXstart + chunkSegXsize; i++)
		for (int j = chunkSegZstart; j < chunkSegZstart + chunkSegZsize; j++)
		{
			unsigned int idx = i * NumSegX + j;

			VB->addNormal(Vector3(normals[idx]).normalize());

			float s0 = (float)i / (float)(NumSegX);
			float t0 = (float)j / (float)(NumSegZ);
			VB->addTexcoord0(s0, t0);

			float s1 = s0 * (float)k;
			float t1 = t0 * (float)k;
			VB->addTexcoord1(s1, t1);

			VB->addVertex(vertz[idx]);
		}
	VB->end();
	printf(" xSize: %d ySize %d count: %d - end\n", chunkSegXsize, chunkSegZsize, VB->vertexCount());

	printf("IB begin");
	IB->begin();
	for (int i = 0; i < chunkSegXsize - 1; i++)
		for (int j = 0; j < chunkSegZsize - 1; j++)
		{
			unsigned int idx_a = i * chunkSegZsize + j;
			unsigned int idx_b = idx_a + 1;
			unsigned int idx_c = idx_a + chunkSegZsize;
			unsigned int idx_d = idx_b + chunkSegZsize;

			IB->addIndex(idx_a);
			IB->addIndex(idx_b);
			IB->addIndex(idx_c);

			IB->addIndex(idx_c);
			IB->addIndex(idx_b);
			IB->addIndex(idx_d);
		}
	IB->end();
	printf(" count %d - end\n", IB->indexCount());
}

bool Terrain::load(const char* HeightMap, const char* DetailMap1, const char* DetailMap2, const char* TopMap, const char* BotMap)
{
	Texture tex = Texture(HeightMap);
	const RGBImage* hmap_image = tex.getRGBImage();
	NumSegX = hmap_image->width();
	NumSegZ = hmap_image->height();

	//Pixels divided by chunk (pixel) size ( +1 because floor )
	chunksSide = NumSegX / chunk_size + 1;
	//squared
	chunks = chunksSide * chunksSide;
	printf("NumSegX %d chunks %d \n", NumSegX, chunks);

	RGBImage HeightImage(NumSegX, NumSegZ);
	RGBImage::GaussFilter(HeightImage, *hmap_image, 2);

	if (!HeightTex.create(HeightImage))
		return false;

	RGBImage mmap(NumSegX, NumSegZ);
	RGBImage::SobelFilter(mmap, HeightImage, 10);
	
	if (!MixTex.create(mmap))
		return false;
	if (!DetailTex[0].load(DetailMap1))
		return false;
	if (!DetailTex[1].load(DetailMap2))
		return false;
	if (!DetailTex[2].load(TopMap))
		return false;
	if (!DetailTex[3].load(BotMap))
		return false;
		
	float startx = -0.5f;
	float startz = -0.5f;
	float stepx = 1 / (float)(NumSegX);
	float stepz = 1 / (float)(NumSegZ);

	// 1. setup vertex buffer
	int terrainSize = NumSegX*NumSegZ;
	Vector3* vertz = new Vector3[terrainSize];
	Vector3* normals = new Vector3[terrainSize];
	Heights = new float[terrainSize];

	Vector3 min, max;
	for (int i = 0; i < NumSegX; i++)
		for (int j = 0; j < NumSegZ; j++)
		{
			unsigned int idx = i * NumSegX + j;
			float py = HeightImage.getPixelColor(i, j).R * Size.y;
			float px = ( startx + i*stepx ) * Size.x;
			float pz = ( startz + j*stepz ) * Size.z;

			if (px < min.x || (j == 0 && i == 0)) min.x = px;
			if (py < min.y || (j == 0 && i == 0)) min.y = py;
			if (pz < min.z || (j == 0 && i == 0)) min.z = pz;
			if (px > max.x || (j == 0 && i == 0)) max.x = px;
			if (py > max.y || (j == 0 && i == 0)) max.y = py;
			if (pz > max.z || (j == 0 && i == 0)) max.z = pz;
			
			Heights[idx] = py;
			vertz[idx] = Vector3(px, py, pz);
			normals[idx] = Vector3(0,0,0);
		}
	BoundingBox = AABB(min, max);
	//why no pointers ?!
	Vector3 a, b, c, d, n;
	for (int i = 0; i < NumSegX - 1; i++)
		for (int j = 0; j < NumSegZ - 1; j++)
		{
			unsigned int idx_a = i * NumSegX + j;
			unsigned int idx_b = idx_a + 1;
			unsigned int idx_c = idx_a + NumSegX;
			unsigned int idx_d = idx_b + NumSegX;
			a = Vector3(vertz[idx_a]);
			b = Vector3(vertz[idx_b]);
			c = Vector3(vertz[idx_c]);
			d = Vector3(vertz[idx_d]);

			n = (b - a).normalize().cross((c - a).normalize());
			normals[idx_a] += n;
			normals[idx_b] += n;
			normals[idx_c] += n;
			n = (b - c).normalize().cross((d - c).normalize());
			normals[idx_c] += n;
			normals[idx_b] += n;
			normals[idx_d] += n;
		}

	int k = 100;

	//generate buffers based on chunk count
	VBs = new VertexBuffer[chunks];
	IBs = new IndexBuffer[chunks];

	for (int chunkX = 0; chunkX < chunksSide; chunkX++) {
		for (int chunkZ = 0; chunkZ < chunksSide; chunkZ++) {
			loadChunk(chunkX, chunkZ, vertz, normals, k);
		}
	}

	//delete HeightImage;
	delete normals;
	delete vertz;
    return true;
}

void Terrain::shader( BaseShader* shader, bool deleteOnDestruction )
{
    BaseModel::shader(shader, deleteOnDestruction);
}

void Terrain::drawChunk(const BaseCamera& Cam, int chunk)
{
	VertexBuffer* VB = &VBs[chunk];
	IndexBuffer* IB = &IBs[chunk];

	VB->activate();
	IB->activate();

	glDrawElements(GL_TRIANGLES, IB->indexCount(), IB->indexFormat(), 0);

	IB->deactivate();
	VB->deactivate();
}

void Terrain::draw(const BaseCamera& Cam)
{
	applyShaderParameter();
    BaseModel::draw(Cam);

	for (int chunkX = 0; chunkX < chunksSide; chunkX++) {
		for (int chunkZ = 0; chunkZ < chunksSide; chunkZ++) {
			int chunkID = chunkX * chunksSide + chunkZ;
			drawChunk(Cam, chunkID);
		}
	}

	pShader->deactivate();
}

float Terrain::getHeight(float x, float z)
{
	int xPos = (int)((x * NumSegX) / Size.x + NumSegX * 0.5);
	int zPos = (int)((z * NumSegZ) / Size.z + NumSegZ * 0.5);
	if (xPos >= NumSegX-1 || xPos < 0 || zPos >= NumSegZ-1 || zPos < 0)
		return 0;
	return Heights[xPos * NumSegX + zPos];
}

void Terrain::applyShaderParameter()
{
    TerrainShader* Shader = dynamic_cast<TerrainShader*>(BaseModel::shader());
    if(!Shader)
        return;
    
    Shader->mixTex(&MixTex);
    for(int i=0; i<4; i++)
        Shader->detailTex(i,&DetailTex[i]);
    Shader->scaling(Size);
}
