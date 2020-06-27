#pragma once

#include "BaseIKKernel.h"
#include <vector>
#include "../../FinalIK/VRIK.h"
#include "../../FinalIK/IKSolverVR.h"

class BaseFinalIKKernel : public BaseIKKernel
{
public:
	BaseFinalIKKernel(std::string name);

	//static RegisterIKSolver<FinalIKKernel> Register;

	virtual Animation* Solve(const Animation& groundTruthAnimation, const std::map<std::string, Tracker*>& trackers, SkinnedModel& model, const Animation& endEffectorsAnimation) = 0;
	virtual std::vector<std::string> InputNames() = 0;

protected:
	RootMotion::IKSolverVR* ikSolver;
	RootMotion::VRIK::References references;
	std::vector<Transform*> transforms;
	std::map<Transform*, AnimationCurve> targetCurveMap;
	std::map<std::string, Matrix> slotOffsetMap;

	void GenerateOffsets(std::map<std::string, Tracker*> trackers);
	void InitSolver(SkinnedModel& model);
	Animation* GenerateSolvedAnimation(float duration);
	void ApplyCurveDataToTarget(Transform* target, const AnimationCurve& curve, const float& time);
	void ApplyCurveDataToTarget(Transform* target, const AnimationCurve& curve, const int& index);
	void RunSolver(float deltaTime);
	void IterateData(const Animation& endEffectorsAnimation, Animation*& solvedAnimation);
	void AddTargetCurve(Transform*& target, const AnimationCurve& source);
	void ApplyCurveDataToTargets(const float& time);
	void ApplyCurveDataToTargets(const int& index);
	void SaveSolvedData(Animation*& solvedAnimation, const float& time);
	void ReleaseTargets();
	void ReleaseTransforms();
};

