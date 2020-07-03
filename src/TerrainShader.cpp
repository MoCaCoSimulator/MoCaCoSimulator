#include "TerrainShader.h"
#include "Paths.h"
#include <string>

TerrainShader::TerrainShader() : PhongShader(false), Scaling(1,1,1), MixTex(NULL)
{
    std::string VSFile = SHADER_DIRECTORY "vsterrain.glsl";
    std::string FSFile = SHADER_DIRECTORY "fsterrain.glsl";
    if( !load(VSFile, FSFile))
        throw std::exception();
    PhongShader::assignLocations();
    specularColor(Color(0,0,0));

	MixTexLoc = getParameterID("MixTex");
    for(int i=0; i<DETAILTEX_COUNT; i++)
    {
        DetailTex[i] =NULL;
        std::string s;
        s += "DetailTex[" + std::to_string(i) + "]";
        DetailTexLoc[i] = getParameterID( s.c_str());
    }
	ScalingLoc = getParameterID("Scaling");
}

TerrainShader::~TerrainShader()
{
	//Detail Tex gets deleted in Terrain (Model)
	//if (MixTex != NULL)
	//	delete MixTex;
	//if (DetailTex != NULL)
	//	delete[] DetailTex;
}

void TerrainShader::activate(const BaseCamera& Cam) const
{
    PhongShader::activate(Cam);

	int TexSlotsIdx = TexCount;
	for (int i = 0; i < MaxLightCount; ++i)
		if (ShadowMapTexture[i] && (ShadowMapMatLoc[i] != -1))
			TexSlotsIdx++;

	TexSlotsIdx = activateTex(MixTex, MixTexLoc, TexSlotsIdx);
    for(int i=0; i<DETAILTEX_COUNT; i++)
		TexSlotsIdx = activateTex(DetailTex[i], DetailTexLoc[i], TexSlotsIdx);
    setParameter(ScalingLoc, Scaling);
}

void TerrainShader::deactivate() const
{
    for(int i=DETAILTEX_COUNT-1; i>=0; i--)
		deactivateTex(DetailTex[i],DetailTexLoc[i]);
	deactivateTex(MixTex, MixTexLoc);
	PhongShader::deactivate();
}
