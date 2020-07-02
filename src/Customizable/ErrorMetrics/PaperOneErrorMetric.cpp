#include "PaperOneErrorMetric.h"

RegisterBaseErrorMetric<PaperOneErrorMetric> PaperOneErrorMetric::Register;

PaperOneErrorMetric::PaperOneErrorMetric() : BaseErrorMetric("Paper One")
{
	needsVelocities = true;
	needsAccelerations = true;
}

bool PaperOneErrorMetric::CalculateDifference(const Pose& groundTruthPose, const Pose& solvedPose, float& result)
{
	if (groundTruthPose.velocities.size() == 0)
		return false;

	// There was no value given for v in the paper
	float v = 1.0f;

	// Setup the joint weights as given by the paper
	std::map<std::string, float> jointWeights;
	for (auto joint : groundTruthPose.skinnedModel->GetJointMapping())
	{
		std::string name = joint.first;
		if(name == "LeftForeArm"
		|| name == "RightForeArm"
		|| name == "LeftLeg"
		|| name == "RightLeg"
		|| name == "LeftUpLeg"
		|| name == "RightUpLeg"
		|| name == "LeftElbow"
		|| name == "RightElbow"
		|| name == "LeftArm"
		|| name == "RightArm"
		|| name == "Hips"
		|| name == "Spine"
		|| name == "Spine1"
		|| name == "Spine2"
		)
			jointWeights[name] = 1.0f;
		else
			jointWeights[name] = 0.0f;
	}

	// Weighted difference between joint velocities
	float velocityResult = 0.0f;
	for (std::pair<std::string, Vector3> groundTruthVelocityPair : groundTruthPose.velocities)
	{
		Vector3 solvedVelocity = solvedPose.velocities.at(groundTruthVelocityPair.first);
		float jointWeight = jointWeights[groundTruthVelocityPair.first];
		velocityResult += jointWeight * Vector3::Distance(groundTruthVelocityPair.second, solvedVelocity);
	}
	result += v * velocityResult;

	// Difference between the translational position
	Vector3 groundTruthPosition = groundTruthPose.skinnedModel->GetJointMapping().at("Hips").transform.translation();
	Vector3 solvedPosition = solvedPose.skinnedModel->GetJointMapping().at("Hips").transform.translation();
	result += std::pow(Vector3::Distance(groundTruthPosition, solvedPosition), 2);

	// Weighted difference between joint rotations
	float rotationResult = 0.0f;
	for (std::pair<std::string, MeshModel::JointInfo> groundTruthJointInfoPair : groundTruthPose.skinnedModel->GetJointMapping())
	{
		MeshModel::JointInfo solvedJointInfo = solvedPose.skinnedModel->GetJointMapping().at(groundTruthJointInfoPair.first);
		Quaternion groundTruthRotation = groundTruthJointInfoPair.second.localTransform.rotation();
		Quaternion solvedRotation = solvedJointInfo.localTransform.rotation();

		//rotationResult += jointWeights[groundTruthJointInfoPair.first] * std::pow(std::abs(std::log(groundTruthRotation.inverse() * solvedRotation)), 2);
		rotationResult += jointWeights[groundTruthJointInfoPair.first] * std::pow(Quaternion::Angle(groundTruthRotation, solvedRotation), 2);
	}
	result += rotationResult;

	return true;
}

BaseErrorMetric* PaperOneErrorMetric::Clone() const
{
	return new PaperOneErrorMetric();
}
