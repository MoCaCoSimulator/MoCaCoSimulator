#pragma once

#include <vector>
#include <string>

#include "../../Animation.h"
#include "../../Parameter.h"
#include "../../AvatarSystem/Avatar.h"
#include "../../Animator.h"

template <class T>
struct RegisterBaseErrorMetric
{
	RegisterBaseErrorMetric()
	{
		BaseErrorMetric::registry().push_back(new T());
	}
};

class BaseErrorMetric
{
private:
	BaseErrorMetric(const BaseErrorMetric&);
protected:
	std::map<std::string, BaseParameter*> parameters;
	std::string name;
public:
	struct Pose
	{
		Avatar* avatar = nullptr;
		SkinnedModel* skinnedModel = nullptr;
		std::map<std::string, Vector3> velocities = std::map<std::string, Vector3>();
		std::map<std::string, Vector3> accelerations = std::map<std::string, Vector3>();
	};

	bool needsVelocities;
	bool needsAccelerations;

	BaseErrorMetric(std::string name);

	virtual bool CalculateDifference(const Pose& groundTruthPose, const Pose& solvedPose, float& result) = 0;

	// Define a virtual destructor
	virtual ~BaseErrorMetric() {}

	// Use the virtual constructor idiom to create copies of subtypes
	virtual BaseErrorMetric* Clone() const = 0;

	void AddParameter(BaseParameter* parameter);
	const std::map<std::string, BaseParameter*>& GetParameters() { return parameters; }

	static std::vector<const BaseErrorMetric*>& registry();
	std::string GetName() const;
	int GetSamplerate();
};

