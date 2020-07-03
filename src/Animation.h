#pragma once

#include <map>
#include "AnimationCurve.h"
#include <string>
#include <assimp\scene.h>
#include <assimp\cimport.h>
#include <assimp\postprocess.h>

struct Animation
{
	Animation() : duration(0), ticksPerSecond(1), animNodeMapping(std::map<std::string, AnimationCurve>()), name("") {}
	Animation(aiAnimation* aiAnim);
	virtual ~Animation() {}

	float duration; //in seconds
	float ticksPerSecond;
	std::map<std::string, AnimationCurve> animNodeMapping; //nodename => animcurve
	std::string name;
	std::string path;
	std::string filename;

	std::string ToString();

	static Animation* LoadFromPath(const std::string& path /*, int animIndex*/);
	static void SaveToPath(const std::string& sourcePath, const Animation& animation, const std::string& path);

	AnimationCurve GetAnimationCurve(std::string name) const { return animNodeMapping.at(name); }
};