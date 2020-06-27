#pragma once

#include "BaseFinalIKKernel.h"
#include <vector>

class FiveTargetFinalIKKernel : public BaseFinalIKKernel
{
public:
	FiveTargetFinalIKKernel();

	static RegisterIKSolver<FiveTargetFinalIKKernel> Register;

	virtual Animation* Solve(const Animation& groundTruthAnimation, const std::map<std::string, Tracker*>& trackers, SkinnedModel& model, const Animation& endEffectorsAnimation);
	virtual std::vector<std::string> InputNames();
};

