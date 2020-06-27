#pragma once

#include "BaseTrackingVirtualizer.h"
#include "../../PythonInclude.h"

class IMUSimTrackingVirtualizer : public BaseTrackingVirtualizer
{
private:
	static int referenceCounter;
	static bool pythonEnvironmentActive;
	static PyObject* pFunc;
public:
	static RegisterVirtualizer<IMUSimTrackingVirtualizer> Register;

	IMUSimTrackingVirtualizer();
	~IMUSimTrackingVirtualizer();

	virtual bool CreateOutputAnimation(TrackerHandle& trackerHandle, AnimationCurve& output);

	virtual BaseTrackingVirtualizer* Clone() const;
};