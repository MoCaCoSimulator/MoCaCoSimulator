#ifndef TerrainShader_hpp
#define TerrainShader_hpp

#include <stdio.h>
#include <assert.h>
#include "PhongShader.h"


class TerrainShader : public PhongShader
{
public:
    enum {
        DETAILTEX0=0,
        DETAILTEX1,
		DETAILTEX2,
		DETAILTEX3,
        DETAILTEX_COUNT
    };
    
    TerrainShader();
	virtual ~TerrainShader();
    virtual void activate(const BaseCamera& Cam) const;
    virtual void deactivate() const;
    
    const Texture* detailTex(unsigned int idx) const { return DetailTex[idx]; }
    const Texture* mixTex() const { return MixTex; }

    void detailTex(unsigned int idx, const Texture* pTex) { DetailTex[idx] = pTex; }
	void mixTex(const Texture* mixTex) { MixTex = mixTex;  }

    void scaling(const Vector3& s) { Scaling = s; }
    const Vector3& scaling() const { return Scaling; }

private:
    const Texture* MixTex;
    const Texture* DetailTex[DETAILTEX_COUNT];
    Vector3 Scaling;
    // shader locations
    GLint MixTexLoc;
    GLint DetailTexLoc[DETAILTEX_COUNT];
    GLint ScalingLoc;
};

#endif /* TerrainShader_hpp */
