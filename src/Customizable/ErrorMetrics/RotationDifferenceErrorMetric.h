#pragma once

#include "BaseErrorMetric.h"

class RotationDifferenceErrorMetric : public BaseErrorMetric
{
public:
	RotationDifferenceErrorMetric();

	virtual bool CalculateDifference(const Pose& groundTruthPose, const Pose& solvedPose, float& result);

	static RegisterBaseErrorMetric<RotationDifferenceErrorMetric> Register;

	virtual BaseErrorMetric* Clone() const;
};

