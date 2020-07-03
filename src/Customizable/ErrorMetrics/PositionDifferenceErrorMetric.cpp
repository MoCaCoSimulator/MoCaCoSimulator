#include "PositionDifferenceErrorMetric.h"
#include "../../Enumerations.h"

RegisterBaseErrorMetric<PositionDifferenceErrorMetric> PositionDifferenceErrorMetric::Register;

PositionDifferenceErrorMetric::PositionDifferenceErrorMetric() : BaseErrorMetric("Position Difference") 
{
	AddParameter(new Parameter<Enums::HumanJointType>("Joint", Enums::HumanJointType::All));
}

bool PositionDifferenceErrorMetric::CalculateDifference(const Pose& groundTruthPose, const Pose& solvedPose, float& result)
{
	Enums::HumanJointType selectedJoint = dynamic_cast<Parameter<Enums::HumanJointType>*>(parameters["Joint"])->GetValue();

	if (selectedJoint == Enums::HumanJointType::All)
	{
		// Use double just to be safe here
		double combinedDistance = 0.0f;
		int counter = 0;

		std::map<std::string, MeshModel::JointInfo> truthJointMapping = groundTruthPose.skinnedModel->GetJointMapping();
		std::map<std::string, MeshModel::JointInfo> solvedJointMapping = solvedPose.skinnedModel->GetJointMapping();
		for (int humanJointType = (int)Enums::HumanJointType::Hips; humanJointType != (int)Enums::HumanJointType::All; humanJointType++)
		{
			Enums::HumanJointType jointEnum = static_cast<Enums::HumanJointType>(humanJointType);
			std::string jointString = QVariant::fromValue(jointEnum).toString().toStdString();

			Vector3 groundTruthPosition = truthJointMapping[jointString].globalTransform.translation();
			Vector3 solvedPosition = solvedJointMapping[jointString].globalTransform.translation();
			combinedDistance += Vector3::Distance(groundTruthPosition, solvedPosition);
			++counter;
		}
		result = combinedDistance / counter;
	}
	else
	{
		std::string selectedJointString = QVariant::fromValue(selectedJoint).toString().toStdString();
		Vector3 solvedPos = solvedPose.skinnedModel->GetJointMapping()[selectedJointString].globalTransform.translation();
		Vector3 groundTruthPos = groundTruthPose.skinnedModel->GetJointMapping()[selectedJointString].globalTransform.translation();
		result = Vector3::Distance(groundTruthPos, solvedPos);
	}

	return true;
}

BaseErrorMetric* PositionDifferenceErrorMetric::Clone() const
{
	return new PositionDifferenceErrorMetric();
}
