#include "JointTrackingVirtualizer.h"

RegisterVirtualizer<JointTrackingVirtualizer> JointTrackingVirtualizer::Register;

JointTrackingVirtualizer::JointTrackingVirtualizer() : BaseTrackingVirtualizer("JointTrackingVirtualizer")
{
	AddParameter(new Parameter("Joint", Enums::HumanJointType::Hips));
}

bool JointTrackingVirtualizer::CreateOutputAnimation(TrackerHandle& trackerHandle, AnimationCurve& output)
{
	Enums::HumanJointType selectedJoint = dynamic_cast<Parameter<Enums::HumanJointType>*>(parameters["Joint"])->GetValue();
	if (selectedJoint == Enums::HumanJointType::All)
	{
		qDebug() << "JointTrackingVirtualizer: All joints cant be used as a option";
		return false;
	}
	std::string selectedJointString = QVariant::fromValue(selectedJoint).toString().toStdString();
	
	AnimationCurve jointCurve = trackerHandle.GetSkinnedModelAnimation()->GetAnimationCurve(selectedJointString);
	float maxTime = jointCurve.positions[jointCurve.positions.size() - 1].time;
	SkinnedModel* model = trackerHandle.GetSkinnedModel();
	output.name = selectedJointString;
	for (AnimationCurve::VectorAnimationKey key : jointCurve.positions)
	{
		float normalizedTime = key.time / maxTime;

		trackerHandle.SetNormalizedAnimationTime(normalizedTime);
		Matrix matrix = model->GetJointMapping()[selectedJointString].globalTransform;
		output.positions.push_back(AnimationCurve::VectorAnimationKey(key.time, matrix.translation()));
		output.rotations.push_back(AnimationCurve::QuaternionAnimationKey(key.time, matrix.rotation()));
		output.scalings.push_back(AnimationCurve::VectorAnimationKey(key.time, matrix.scale()));
	}

	return true;
}

BaseTrackingVirtualizer* JointTrackingVirtualizer::Clone() const
{
	return new JointTrackingVirtualizer();
}
