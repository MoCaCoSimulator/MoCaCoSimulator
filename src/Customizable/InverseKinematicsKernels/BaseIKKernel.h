#pragma once

#include "../../Animation.h"
#include "../../MeshModel.h"
#include "../../Enumerations.h"
#include "../../Parameter.h"
#include "../../Parameter.h"
#include "../../SkinnedModel.h"
#include "../../Tracker.h"

template <class T>
struct RegisterIKSolver
{
	RegisterIKSolver()
	{
		BaseIKKernel::registry().push_back(new T());
	}
};

class BaseIKKernel
{
private:
	std::string name;

	BaseIKKernel(const BaseIKKernel &);
protected:
	std::map<std::string, BaseParameter*> parameters;

public:
	BaseIKKernel(std::string name);

	// Define a virtual destructor
	virtual ~BaseIKKernel() {}

	virtual Animation* Solve(const Animation& groundTruthAnimation, const std::map<std::string, Tracker*>& trackers, SkinnedModel& model, const Animation& endEffectorsAnimation) = 0;
	virtual std::vector<std::string> InputNames() = 0;

	void AddParameter(BaseParameter* parameter);
	const std::map<std::string, BaseParameter*>& GetParameters() { return parameters; }

	static std::vector<BaseIKKernel*>& registry();
	std::string GetName() const;
};