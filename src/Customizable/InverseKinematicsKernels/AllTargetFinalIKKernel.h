#pragma once

#include "BaseFinalIKKernel.h"
#include <vector>

class AllTargetFinalIKKernel : public BaseFinalIKKernel
{
public:
	AllTargetFinalIKKernel();

	static RegisterIKSolver<AllTargetFinalIKKernel> Register;

	virtual Animation* Solve(const Animation& groundTruthAnimation, const std::map<std::string, Tracker*>& trackers, SkinnedModel& model, const Animation& endEffectorsAnimation);
	virtual std::vector<std::string> InputNames();
};

