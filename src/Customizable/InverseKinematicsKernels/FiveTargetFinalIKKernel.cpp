#include "FiveTargetFinalIKKernel.h"
#include "../../FinalIK/VRIK.h"
#include "../../FinalIK/IKSolverVR.h"

RegisterIKSolver<FiveTargetFinalIKKernel> FiveTargetFinalIKKernel::Register;

FiveTargetFinalIKKernel::FiveTargetFinalIKKernel() : BaseFinalIKKernel("Five Target Final IK Kernel")
{
	//default values match VRIK
	AddParameter(new Parameter<float>("BodyPosStiffness", 0.55f));
	AddParameter(new Parameter<float>("BodyRotStiffness", 0.1f));
	AddParameter(new Parameter<float>("MaintainPelvisPosition", 0.2f));
}

Animation* FiveTargetFinalIKKernel::Solve(const Animation& groundTruthAnimation, const std::map<std::string, Tracker*>& trackers, SkinnedModel& model, const Animation& endEffectorsAnimation)
{
	InitSolver(model);

	Animation unconstEndEffectorsAnimation = endEffectorsAnimation;
	Animation unconstGroundTruthAnimation = groundTruthAnimation;
	std::map<std::string, AnimationCurve> curveMapping = unconstEndEffectorsAnimation.animNodeMapping;

	AddTargetCurve(ikSolver->spine->headTarget, curveMapping["Head"]);
	AddTargetCurve(ikSolver->leftArm->target, curveMapping["LeftHand"]);
	AddTargetCurve(ikSolver->rightArm->target, curveMapping["RightHand"]);
	AddTargetCurve(ikSolver->leftLeg->target, curveMapping["LeftToeBase"]);
	AddTargetCurve(ikSolver->rightLeg->target, curveMapping["RightToeBase"]);
	GenerateOffsets(trackers);

	ikSolver->spine->positionWeight = 1.0f;
	ikSolver->spine->rotationWeight = 1.0f;
	ikSolver->spine->pelvisPositionWeight = 0.0f;
	ikSolver->spine->pelvisRotationWeight = 0.0f;
	ikSolver->leftArm->positionWeight = 1.0f;
	ikSolver->leftArm->rotationWeight = 1.0f;
	ikSolver->rightArm->positionWeight = 1.0f;
	ikSolver->rightArm->rotationWeight = 1.0f;
	ikSolver->leftLeg->positionWeight = 1.0f;
	ikSolver->leftLeg->rotationWeight = 1.0f;
	ikSolver->rightLeg->positionWeight = 1.0f;
	ikSolver->rightLeg->rotationWeight = 1.0f;

	// Specific options for solving without a pelvis
	//Todo Make parameters for those
	auto* bodyPos = dynamic_cast<Parameter<float>*>(parameters["BodyPosStiffness"]);
	auto* bodyRot = dynamic_cast<Parameter<float>*>(parameters["BodyRotStiffness"]);
	auto* pelvisPos = dynamic_cast<Parameter<float>*>(parameters["MaintainPelvisPosition"]);

	ikSolver->spine->bodyPosStiffness = bodyPos->GetValue();
	ikSolver->spine->bodyRotStiffness = bodyRot->GetValue();
	ikSolver->spine->maintainPelvisPosition = pelvisPos->GetValue();

	Animation* solvedAnimation = GenerateSolvedAnimation(groundTruthAnimation.duration);
	IterateData(endEffectorsAnimation, solvedAnimation);

	ReleaseTransforms();
	ReleaseTargets();

	return solvedAnimation;
}

std::vector<std::string> FiveTargetFinalIKKernel::InputNames()
{
	std::vector<std::string> inputNames;

	inputNames.push_back("Head");
	inputNames.push_back("LeftHand");
	inputNames.push_back("RightHand");
	inputNames.push_back("LeftToeBase");
	inputNames.push_back("RightToeBase");

	return inputNames;
}
