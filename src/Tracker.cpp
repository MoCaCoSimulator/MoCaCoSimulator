#include "Tracker.h"

Tracker Tracker::Invalid = Tracker();

void Tracker::Select(const bool& state)
{
	selected = state;
	UpdateColor();
}

void Tracker::Hover(const bool& state)
{
	hovered = state;
	UpdateColor();
}

void Tracker::UpdateColor()
{
	if (selected)
		model->shader()->color(Color::red);
	else if (hovered)
		model->shader()->color(Color::orange);
	else
		model->shader()->color(Color::white);
}

void Tracker::SetSlot(const std::string& slotName)
{
	const SkinnedModel* skin = dynamic_cast<const SkinnedModel*>(model->getAttachedTo());
	if (!skin)
		return;

	const MeshModel::JointInfo* info = skin->GetJointInfo(slotName);
	if (!info)
	{
		SetOffsetPosition(Vector3::zero);
		SetOffsetRotation(Quaternion::identity);
		return;
	}

	slot = slotName;

	((SkinnedModel*)skin)->SetDefaultPose();

	Matrix globalJointTransform = info->globalTransform; 
	Matrix localJointTransform = Matrix(model->getAnimationTransform()).invert() * globalJointTransform;

	//bring offset to world scale
	SetOffsetPosition(localJointTransform.translation().multiplyElements(localJointTransform.scale()));
	SetOffsetRotation(localJointTransform.rotation());
}

void Tracker::UpdateOffset()
{
	SetSlot(slot);
}
