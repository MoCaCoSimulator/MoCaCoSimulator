#include "AnatomicAngleErrorMetric.h"
#include "../../Enumerations.h"

RegisterBaseErrorMetric<AnatomicAngleErrorMetric> AnatomicAngleErrorMetric::Register;

AnatomicAngleErrorMetric::AnatomicAngleErrorMetric() : BaseErrorMetric("Anatomic Angle Difference")
{
	//AddParameter(new Parameter<CustomEnums::AvatarJointType>("Joint", CustomEnums::AvatarJointType::));
}

bool AnatomicAngleErrorMetric::CalculateDifference(const Pose& groundTruthPose, const Pose& solvedPose, float& result)
{
	// Use double just to be safe here
	double combinedAngle = 0.0f;
	int counter = 0;

	for (int jointType = 0; jointType <= (int)CustomEnums::AvatarJointType::Head; jointType++)
	{
		AvatarJoint* joint = groundTruthPose.avatar->GetAvatarJoint((CustomEnums::AvatarJointType)jointType);
		std::vector<CustomEnums::HumanAnatomicAngleType> angleTypes = groundTruthPose.avatar->GetAnatomicAngleTypesOfJoint(joint->humanJointType);
		for (CustomEnums::HumanAnatomicAngleType angleType : angleTypes)
		{
			float groundTruthAngle = groundTruthPose.avatar->GetAnatomicAngle((CustomEnums::AvatarJointType)jointType, angleType);
			float solvedAngle = solvedPose.avatar->GetAnatomicAngle((CustomEnums::AvatarJointType)jointType, angleType);
			combinedAngle += std::abs(groundTruthAngle - solvedAngle);
			++counter;
		}
	}

	result = combinedAngle / counter;

	return true;
}

BaseErrorMetric* AnatomicAngleErrorMetric::Clone() const
{
	return new AnatomicAngleErrorMetric();
}
