#include "WeightedRotationErrorMetric.h"
#include "../../Enumerations.h"

RegisterBaseErrorMetric<WeightedRotationErrorMetric> WeightedRotationErrorMetric::Register;

WeightedRotationErrorMetric::WeightedRotationErrorMetric() :
	BaseErrorMetric("Weighted Rotation Difference"),
	longestBone(0)
{
}

bool WeightedRotationErrorMetric::CalculateDifference(const Pose& groundTruthPose, const Pose& solvedPose, float& result)
{
	if (longestBone == 0)
		for (const std::pair<std::string, MeshModel::JointInfo>& pair : groundTruthPose.skinnedModel->GetJointMapping())
		{
			if (pair.first == "Hips")
				continue;

			double boneLength = pair.second.localTransform.translation().length();
			boneLength *= pair.second.globalTransform.scale().x;
			if (boneLength > longestBone)
				longestBone = boneLength;
		}

	// Use double just to be safe here
	double combinedAngle = 0.0f;
	int counter = 0;

	std::map<std::string, MeshModel::JointInfo> solvedJointMapping = solvedPose.skinnedModel->GetJointMapping();
	for (const std::pair<std::string, MeshModel::JointInfo>& pair : groundTruthPose.skinnedModel->GetJointMapping())
	{
		double boneWeight = pair.second.localTransform.translation().length() / longestBone;
		boneWeight *= pair.second.globalTransform.scale().x;
		if (pair.first == "Hips")
			boneWeight = 1.0f;

		Quaternion groundTruthPosition = pair.second.globalTransform.rotation();
		Quaternion solvedPosition = solvedJointMapping[pair.first].globalTransform.rotation();
		combinedAngle += Quaternion::Angle(groundTruthPosition, solvedPosition) * boneWeight;
		++counter;
	}
	result = combinedAngle / counter;

	return true;
}

BaseErrorMetric* WeightedRotationErrorMetric::Clone() const
{
	return new WeightedRotationErrorMetric();
}
