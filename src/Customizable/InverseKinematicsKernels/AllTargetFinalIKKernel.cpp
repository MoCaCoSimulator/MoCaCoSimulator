#include "AllTargetFinalIKKernel.h"
#include "../../FinalIK/VRIK.h"
#include "../../FinalIK/IKSolverVR.h"

RegisterIKSolver<AllTargetFinalIKKernel> AllTargetFinalIKKernel::Register;

AllTargetFinalIKKernel::AllTargetFinalIKKernel() : BaseFinalIKKernel("All Target Final IK Kernel")
{

}

Animation* AllTargetFinalIKKernel::Solve(const Animation& groundTruthAnimation, const std::map<std::string, Tracker*>& trackers, SkinnedModel& model, const Animation& endEffectorsAnimation)
{
	InitSolver(model);

	Animation unconstEndEffectorsAnimation = endEffectorsAnimation;
	Animation unconstGroundTruthAnimation = groundTruthAnimation;
	std::map<std::string, AnimationCurve> curveMapping = unconstEndEffectorsAnimation.animNodeMapping;

	AddTargetCurve(ikSolver->spine->headTarget, curveMapping["Head"]);
	AddTargetCurve(ikSolver->spine->pelvisTarget, curveMapping["Hips"]);
	AddTargetCurve(ikSolver->leftArm->target, curveMapping["LeftHand"]);
	AddTargetCurve(ikSolver->rightArm->target, curveMapping["RightHand"]);
	AddTargetCurve(ikSolver->leftLeg->target, curveMapping["LeftToeBase"]);
	AddTargetCurve(ikSolver->rightLeg->target, curveMapping["RightToeBase"]);

	AddTargetCurve(ikSolver->leftArm->bendGoal, curveMapping["LeftForeArm"]);
	AddTargetCurve(ikSolver->rightArm->bendGoal, curveMapping["RightForeArm"]);
	AddTargetCurve(ikSolver->leftLeg->bendGoal, curveMapping["LeftLeg"]);
	AddTargetCurve(ikSolver->rightLeg->bendGoal, curveMapping["RightLeg"]);
	AddTargetCurve(ikSolver->spine->chestGoal, curveMapping["Spine1"]);
	
	GenerateOffsets(trackers);

	ikSolver->spine->positionWeight = 1.0f;
	ikSolver->spine->rotationWeight = 1.0f;
	ikSolver->spine->pelvisPositionWeight = 1.0f;
	ikSolver->spine->pelvisRotationWeight = 1.0f;
	ikSolver->leftArm->positionWeight = 1.0f;
	ikSolver->leftArm->rotationWeight = 1.0f;
	ikSolver->rightArm->positionWeight = 1.0f;
	ikSolver->rightArm->rotationWeight = 1.0f;
	ikSolver->leftLeg->positionWeight = 1.0f;
	ikSolver->leftLeg->rotationWeight = 1.0f;
	ikSolver->rightLeg->positionWeight = 1.0f;
	ikSolver->rightLeg->rotationWeight = 1.0f;

	//ikSolver->spine->chestGoalWeight = 1.0f;
	ikSolver->leftArm->bendGoalWeight = 1.0f;
	ikSolver->rightArm->bendGoalWeight = 1.0f;
	ikSolver->leftLeg->bendGoalWeight = 1.0f;
	ikSolver->rightLeg->bendGoalWeight = 1.0f;

	Animation* solvedAnimation = GenerateSolvedAnimation(groundTruthAnimation.duration);
	IterateData(endEffectorsAnimation, solvedAnimation);

	ReleaseTransforms();
	ReleaseTargets();

	return solvedAnimation;
}

std::vector<std::string> AllTargetFinalIKKernel::InputNames()
{
	std::vector<std::string> inputNames;

	//Targets
	inputNames.push_back("Head");
	inputNames.push_back("Hips");
	inputNames.push_back("LeftHand");
	inputNames.push_back("RightHand");
	inputNames.push_back("LeftToeBase");
	inputNames.push_back("RightToeBase");

	//Bends
	inputNames.push_back("LeftForeArm");
	inputNames.push_back("RightForeArm");
	inputNames.push_back("LeftLeg");
	inputNames.push_back("RightLeg");
	inputNames.push_back("Spine1");

	return inputNames;
}
