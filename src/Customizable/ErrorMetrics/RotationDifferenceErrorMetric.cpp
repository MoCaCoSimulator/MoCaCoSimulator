#include "RotationDifferenceErrorMetric.h"
#include "../../Enumerations.h"

RegisterBaseErrorMetric<RotationDifferenceErrorMetric> RotationDifferenceErrorMetric::Register;

RotationDifferenceErrorMetric::RotationDifferenceErrorMetric() : BaseErrorMetric("Rotation Difference") 
{
	AddParameter(new Parameter<Enums::HumanJointType>("Joint", Enums::HumanJointType::All));
}

bool RotationDifferenceErrorMetric::CalculateDifference(const Pose& groundTruthPose, const Pose& solvedPose, float& result)
{
	Enums::HumanJointType selectedJoint = dynamic_cast<Parameter<Enums::HumanJointType>*>(parameters["Joint"])->GetValue();

	if (selectedJoint == Enums::HumanJointType::All)
	{
		// Use double just to be safe here
		double combinedAngle = 0.0f;
		int counter = 0;

		std::map<std::string, MeshModel::JointInfo> truthJointMapping = groundTruthPose.skinnedModel->GetJointMapping();
		std::map<std::string, MeshModel::JointInfo> solvedJointMapping = solvedPose.skinnedModel->GetJointMapping();
		for (int humanJointType = (int)Enums::HumanJointType::Hips; humanJointType != (int)Enums::HumanJointType::All; humanJointType++)
		{
			Enums::HumanJointType jointEnum = static_cast<Enums::HumanJointType>(humanJointType);
			std::string jointString = QVariant::fromValue(jointEnum).toString().toStdString();

			Quaternion groundTruthPosition = truthJointMapping[jointString].globalTransform.rotation();
			Quaternion solvedPosition = solvedJointMapping[jointString].globalTransform.rotation();
			combinedAngle += Quaternion::Angle(groundTruthPosition, solvedPosition);
			++counter;
		}
		result = combinedAngle / counter;
	}
	else
	{
		std::string selectedJointString = QVariant::fromValue(selectedJoint).toString().toStdString();
		Quaternion solvedPos = solvedPose.skinnedModel->GetJointMapping()[selectedJointString].globalTransform.rotation();
		Quaternion groundTruthPos = groundTruthPose.skinnedModel->GetJointMapping()[selectedJointString].globalTransform.rotation();
		result = Quaternion::Angle(groundTruthPos, solvedPos);
	}

	return true;
}

BaseErrorMetric* RotationDifferenceErrorMetric::Clone() const
{
	return new RotationDifferenceErrorMetric();
}
