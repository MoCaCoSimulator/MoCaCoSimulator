#pragma once

#include "BaseIKKernel.h"
#include <vector>

class PerfectIKKernel : public BaseIKKernel
{
public:
	PerfectIKKernel();

	static RegisterIKSolver<PerfectIKKernel> Register;

	virtual Animation* Solve(const Animation& groundTruthAnimation, const std::map<std::string, Tracker*>& trackers, SkinnedModel& model, const Animation& endEffectorsAnimation);
	virtual std::vector<std::string> InputNames();
};

