#pragma once

#include "BaseTrackingVirtualizer.h"
#include "../../Animation.h"
#include <string>
#include <vector>

class JointTrackingVirtualizer : public BaseTrackingVirtualizer
{
public:
	static RegisterVirtualizer<JointTrackingVirtualizer> Register;

	JointTrackingVirtualizer();

	virtual bool CreateOutputAnimation(TrackerHandle& trackerHandle, AnimationCurve& output);

	virtual BaseTrackingVirtualizer* Clone() const;
};