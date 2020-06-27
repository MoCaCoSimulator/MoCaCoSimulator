#pragma once

#include "BaseTrackingVirtualizer.h"
#include "../../Animation.h"
#include <string>
#include <vector>

class PerfectTrackingVirtualizer : public BaseTrackingVirtualizer
{
public:
	static RegisterVirtualizer<PerfectTrackingVirtualizer> Register;

	PerfectTrackingVirtualizer();

	virtual bool CreateOutputAnimation(TrackerHandle& trackerHandle, AnimationCurve& output);

	virtual BaseTrackingVirtualizer* Clone() const;
};