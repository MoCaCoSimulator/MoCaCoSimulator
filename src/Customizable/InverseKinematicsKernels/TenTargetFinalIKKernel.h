
#pragma once

#include "BaseFinalIKKernel.h"
#include <vector>

class TenTargetFinalIKKernel : public BaseFinalIKKernel
{
public:
	TenTargetFinalIKKernel();

	static RegisterIKSolver<TenTargetFinalIKKernel> Register;

	virtual Animation* Solve(const Animation& groundTruthAnimation, const std::map<std::string, Tracker*>& trackers, SkinnedModel& model, const Animation& endEffectorsAnimation);
	virtual std::vector<std::string> InputNames();
};

