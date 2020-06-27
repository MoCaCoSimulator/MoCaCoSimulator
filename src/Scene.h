

#ifndef Application_hpp
#define Application_hpp

#include <stdio.h>
#include <list>
#include <time.h>
#include <stdlib.h> 
#include <algorithm>
#include "paths.h"
#include "camera.h"
#include "phongshader.h"
#include "constantshader.h"
#include "vertexbuffer.h"
#include "indexbuffer.h"
#include "basemodel.h"
#include "ShaderLightmapper.h"
#include "terrain.h"
#include "lineplanemodel.h"
#include "lineboxmodel.h"
#include "triangleplanemodel.h"
#include "trianglespheremodel.h"
#include "triangleboxmodel.h"
#include "terrainshader.h"
#include "MouseInput.h"
#include "Animator.h"

class Scene
{
public:
    typedef std::list<BaseModel*> ModelList;
	typedef std::list<Animator*> AnimatorList;
    Scene();
	virtual ~Scene();
    virtual void start();
	virtual void update(float dtime);
	virtual void draw();
	virtual void end();
	virtual void camRayCollision(const float& range, HitInfo& info);
	virtual void rayCollision(const Vector3& start, const Vector3& direction, const float& range, HitInfo& info);
	void setSize(int width, int height);

	Animator* GetAnimator(int animatorIndex = 0) const;
	void AddModel(BaseModel* model) { models.push_back(model); }
	Camera* getCamera() { return &cam; };
	const BaseModel* findModel(std::string name) const;
	void DeleteModel(BaseModel* model);
	std::string FindCharacterFileInPath(std::string path);
protected:
	virtual void ClearScene();

    Camera cam;
    ModelList models;
	AnimatorList animators;
	DirectionalLight* sun;

	float time;
	Vector3 vdir;

	//ShadowMapGenerator ShadowGenerator;
};

#endif /* Application_hpp */
