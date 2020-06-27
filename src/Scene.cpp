
#define SPEED_LR 0.3f;
#define SPEED_F 1.f;

#include "Scene.h"
#include "AttachedModel.h"
#include "InputManager.h"
#include <QDebug>
#include <filesystem>

Scene::Scene() :
	cam(),
	sun(),
	time(0)
{
}

Scene::~Scene()
{
	ClearScene();
}

void Scene::ClearScene()
{
	for (ModelList::iterator it = models.begin(); it != models.end(); ++it)
		delete (*it);

	for (AnimatorList::iterator it = animators.begin(); it != animators.end(); ++it)
		delete (*it);

	models.clear();
	animators.clear();
}

void Scene::start()
{
	time = 0;
}

void Scene::update(float dtime)
{
	cam.handleInput();
    cam.update();
	time += dtime;

	// Update the animators
	for each (Animator* animator in animators)
		animator->Update(dtime);
}

void Scene::draw()
{
	ShaderLightMapper::instance().activate();

	//Todo check for NULL ptr in modellist

    //setup shaders and Draw models
    for( ModelList::iterator it = models.begin(); it != models.end(); ++it )
    {
		BaseModel* model = *it;
		if (model->Disabled())
			continue;
		if (model->getTransparency() == TRANSPARENCY_FULL)
			continue;
		if (model->AlwaysOnTop())
			continue;
		model->draw(cam);
    }

	//Draw transparent models | Everything above will be seen through transparent models
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	for (ModelList::iterator it = models.begin(); it != models.end(); ++it)
	{
		BaseModel* model = *it;
		if (model->Disabled())
			continue;
		if (model->getTransparency() == TRANSPARENCY_NONE)
			continue;
		model->draw(cam);
	}
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);

	//Draw models on top of previously drawn models (see through)
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (ModelList::iterator it = models.begin(); it != models.end(); ++it)
	{
		BaseModel* model = *it;
		if (model->Disabled())
			continue;
		if (!model->AlwaysOnTop())
			continue;
		model->draw(cam);
	}
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);

	ShaderLightMapper::instance().deactivate();
}

void Scene::end()
{
}

void Scene::camRayCollision(const float& distance, HitInfo& info)
{
	Vector3 ray = cam.screenPosToRay(InputManager::mousePosition.x, InputManager::mousePosition.y);
	rayCollision(cam.position(), ray, distance, info);
}

void Scene::rayCollision(const Vector3& start, const Vector3& direction, const float& range, HitInfo& info)
{
	for each (BaseModel * model in models)
	{
		if (model->Disabled())
			continue;

		HitInfo modelInfo;
		model->rayCollision(start, direction, range, modelInfo);

		//continue if model was not hit
		if (!modelInfo.hit)
			continue;

		//continue if already hit another model and distance of new hit is further away
		if (info.hit && info.distance < modelInfo.distance) {
			qDebug() << "new hit on " << modelInfo.model->getName().c_str() << " but " << info.model->getName().c_str() << " distance lower " << info.distance << " (new " << modelInfo.distance << " ) ";
			continue;
		}

		info = modelInfo;
	}
}

void Scene::setSize(int width, int height)
{
	cam.setSize(width, height);
}

Animator* Scene::GetAnimator(int animatorIndex) const
{
	if (animators.front() == nullptr)
		return nullptr;
	auto it = animators.begin();
	std::advance(it, animatorIndex);
	return *it;
}

const BaseModel* Scene::findModel(std::string name) const
{
	for (const BaseModel* model : models)
		if (model->getName() == name)
			return model;
	return NULL;
}

void Scene::DeleteModel(BaseModel* model)
{
	models.remove(model);
	delete model;
}

std::string Scene::FindCharacterFileInPath(std::string path)
{
	std::string file = "";
	qDebug() << path.c_str();
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		std::string entryPath = entry.path().string();
		typename std::string::size_type const p(entryPath.find_last_of('.'));
		if (p == 0 || p == std::string::npos)
			continue;

		std::string type = entryPath.substr(p, entryPath.length() - 1);

		if (type != ".dae")
			continue;

		file = entryPath;
		break;
	}

	std::replace(file.begin(), file.end(), '\\', '/');

	return file;
}
