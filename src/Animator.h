#pragma once

#include "vector.h"
#include "Quaternion.h"
#include <vector>
#include "Animation.h"
#include <assimp\scene.h>
#include "SkinnedModel.h"

class Animator
{
	//const MeshModel::Node* RootNode;
	//std::map<std::string, SkinnedModel::JointInfo>* jointMapping; //TODO switch to bones
	SkinnedModel* model;
	const Animation* animation;

	float animationTime;
	//Matrix globalInverseTransform;
	bool isPlaying;
	bool loopAnimation;
public:
	float speed;

	// Constructors
	//Animator(const MeshModel::Node& rootNode, std::map<std::string, SkinnedModel::JointInfo>& jointMapping, Animation* animation = nullptr);
	Animator(SkinnedModel& model, const Animation* animation = nullptr);
	~Animator();

	void Update(float deltaTime);
	const Animation* GetAnimation() const;
	void SetAnimation(Animation* newAnimation);
	void RemoveAnimation(bool destroy = false);
	void SetAnimationTime(float time, bool force = false);
	void SetNormalizedAnimationTime(float time);
	void Play(const bool loop = false);
	void Pause();
	void Stop();

	const MeshModel::Node* GetRootNode();

	float GetAnimationLength();
	bool IsPlaying();
	bool HasAnimation() { return animation != NULL; };
	float NormalizedTime();
	SkinnedModel* GetModel() { return model; }
private:
	static float GetInterpolationTime(const AnimationCurve::AnimationKey& from, const AnimationCurve::AnimationKey& to, const float time);
	Matrix GetNodeTransform(float time, std::string nodeName);
	void UpdateAnimation(float time, const MeshModel::Node* node, const Matrix& parentTransform);
	void DefaultPose();
	Quaternion InterpolateQuaternion(float time, const std::vector<AnimationCurve::QuaternionAnimationKey>& quatKey) const;
	Vector3 InterpolateVector(float time, const std::vector<AnimationCurve::VectorAnimationKey>& vectorKey) const;
	int FindActiveQuaternionKeyIndex(float time, const std::vector<AnimationCurve::QuaternionAnimationKey>& keys) const;
	int FindActiveVectorKeyIndex(float time, const std::vector<AnimationCurve::VectorAnimationKey>& keys) const;
	void Reset();
};