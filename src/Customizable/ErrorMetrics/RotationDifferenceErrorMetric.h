#pragma once

#include "BaseErrorMetric.h"

class RotationDifferenceErrorMetric : public BaseErrorMetric
{
protected:
	virtual bool CalculateDifference(const Pose& groundTruthPose, const Pose& solvedPose, float& result);
public:
	RotationDifferenceErrorMetric();

	static RegisterBaseErrorMetric<RotationDifferenceErrorMetric> Register;

	virtual BaseErrorMetric* Clone() const;
};

