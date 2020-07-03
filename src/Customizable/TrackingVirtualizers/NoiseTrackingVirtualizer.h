#pragma once
#include "BaseTrackingVirtualizer.h"
#include "../../Animation.h"

class NoiseTrackingVirtualizer : public BaseTrackingVirtualizer
{
public:
	static RegisterVirtualizer<NoiseTrackingVirtualizer> Register;

	NoiseTrackingVirtualizer();

	virtual bool CreateOutputAnimation(TrackerHandle& trackerHandle, AnimationCurve& output);
private:
	static void InitializeRand(int seed);
	static bool randInitialized;
	static int randSeed;

	virtual BaseTrackingVirtualizer* Clone() const;
};
