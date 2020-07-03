#pragma once

#include "BaseErrorMetric.h"

class PositionDifferenceErrorMetric : public BaseErrorMetric
{
public:
	PositionDifferenceErrorMetric();

	virtual bool CalculateDifference(const Pose& groundTruthPose, const Pose& solvedPose, float& result);

	static RegisterBaseErrorMetric<PositionDifferenceErrorMetric> Register;

	virtual BaseErrorMetric* Clone() const;
};