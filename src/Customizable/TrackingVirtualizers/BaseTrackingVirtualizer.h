#pragma once

#include<string>
#include "../../Animation.h"
#include "../../Animator.h"
#include "../../AttachedModel.h"
#include "../../Enumerations.h"
#include "../CustomEnumerations.h"
#include <typeindex>
#include "QDebug"
#include "../../Parameter.h"
#include "../../Tracker.h"

struct TrackerHandle
{
private:
	mutable Animator* animator;
	Tracker* tracker;
	std::string inputName;
public:
	TrackerHandle(Animator* animator, Tracker* tracker, std::string inputName) :
		animator(animator),
		tracker(tracker),
		inputName(inputName)
	{
	}

	Vector3 GetPositionOffset()
	{
		return tracker->GetOffsetPosition();
	}

	Quaternion GetRotationOffset()
	{
		return tracker->GetOffsetRotation();
	}

	Vector3 GetPosition(float time) const
	{
		animator->SetNormalizedAnimationTime(time);
		// Maybe a draw is needed?
		Vector3 position = tracker->GetModel()->getAnimationTransform().translation();
		return position;
	}

	Quaternion GetRotation(float time) const
	{
		animator->SetNormalizedAnimationTime(time);
		// Maybe a draw is needed?
		Quaternion rotation = tracker->GetModel()->getAnimationTransform().rotation();
		return rotation;
	}

	Matrix GetTransform(float time) const
	{
		animator->SetNormalizedAnimationTime(time);
		return tracker->GetModel()->getAnimationTransform();
	}

	std::string GetName() const
	{
		return inputName;
	}

	float GetAnimationLength()
	{
		return animator->GetAnimationLength();
	}

	SkinnedModel* GetSkinnedModel()
	{
		return animator->GetModel();
	}

	const Animation* GetSkinnedModelAnimation()
	{
		return animator->GetAnimation();
	}

	void SetNormalizedAnimationTime(float normalizedTime)
	{
		animator->SetNormalizedAnimationTime(normalizedTime);
	}
};

// Weird but works
template <class T>
struct RegisterVirtualizer
{
	RegisterVirtualizer()
	{
		BaseTrackingVirtualizer::registry().push_back(new T());
	}
	~RegisterVirtualizer()
	{
		auto& virtualizers = BaseTrackingVirtualizer::registry();
		for (const BaseTrackingVirtualizer* virtualizer : virtualizers)
			delete virtualizer;
		virtualizers.clear();
	}
};

class BaseTrackingVirtualizer
{
private:

	BaseTrackingVirtualizer(const BaseTrackingVirtualizer&);
protected:
	std::string name;
	std::map<std::string, BaseParameter*> parameters;

public:
	BaseTrackingVirtualizer(std::string name);
	virtual ~BaseTrackingVirtualizer();

	// Define a virtual destructor
	void AddParameter(BaseParameter* parameter);
	const std::map<std::string, BaseParameter*>& GetParameters() { return parameters; }

	virtual bool CreateOutputAnimation(TrackerHandle& trackerHandle, AnimationCurve& output) = 0;

	static std::vector<const BaseTrackingVirtualizer*>& registry();
	std::string GetName() const;

	// Use the virtual constructor idiom to create copies of subtypes
	virtual BaseTrackingVirtualizer* Clone() const = 0;
};

