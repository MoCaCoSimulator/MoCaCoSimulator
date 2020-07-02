#include "WeightedPositionErrorMetric.h"
#include "../../Enumerations.h"

RegisterBaseErrorMetric<WeightedPositionErrorMetric> WeightedPositionErrorMetric::Register;

WeightedPositionErrorMetric::WeightedPositionErrorMetric() : 
	BaseErrorMetric("Weighted Position Difference"),
	longestBone(0)
{
}

bool WeightedPositionErrorMetric::CalculateDifference(const Pose& groundTruthPose, const Pose& solvedPose, float& result)
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
	double combinedDistance = 0.0f;
	int counter = 0;

	std::map<std::string, MeshModel::JointInfo> solvedJointMapping = solvedPose.skinnedModel->GetJointMapping();
	for (const std::pair<std::string, MeshModel::JointInfo>& pair : groundTruthPose.skinnedModel->GetJointMapping())
	{
		double boneWeight = pair.second.localTransform.translation().length() / longestBone;
		boneWeight *= pair.second.globalTransform.scale().x;
		if (pair.first == "Hips")
			boneWeight = 0.0f;

		Vector3 groundTruthPosition = pair.second.globalTransform.translation();
		Vector3 solvedPosition = solvedJointMapping[pair.first].globalTransform.translation();
		combinedDistance += Vector3::Distance(groundTruthPosition, solvedPosition) * boneWeight;
		++counter;
	}
	result = combinedDistance / counter;

	return true;
}

BaseErrorMetric* WeightedPositionErrorMetric::Clone() const
{
	return new WeightedPositionErrorMetric();
}
