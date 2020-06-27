#pragma once

#include "BaseErrorMetric.h"

class PositionDifferenceErrorMetric : public BaseErrorMetric
{
protected:
	virtual bool CalculateDifference(const Pose& groundTruthPose, const Pose& solvedPose, float& result);
public:
	PositionDifferenceErrorMetric();

	static RegisterBaseErrorMetric<PositionDifferenceErrorMetric> Register;

	virtual BaseErrorMetric* Clone() const;
};