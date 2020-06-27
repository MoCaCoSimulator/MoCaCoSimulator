#include "PerfectIKKernel.h"

RegisterIKSolver<PerfectIKKernel> PerfectIKKernel::Register;

PerfectIKKernel::PerfectIKKernel() : BaseIKKernel("Perfect IK Kernel")
{
}

Animation* PerfectIKKernel::Solve(const Animation& groundTruthAnimation, const std::map<std::string, Tracker*>& trackers, SkinnedModel& model, const Animation& endEffectorsAnimation)
{
	Animation* solvedAnimation = new Animation(groundTruthAnimation);
	return solvedAnimation;
}

std::vector<std::string> PerfectIKKernel::InputNames()
{
	return std::vector<std::string>();
}
