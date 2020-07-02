#pragma once

#include "BaseErrorMetric.h"

class WeightedPositionErrorMetric : public BaseErrorMetric
{
public:
	WeightedPositionErrorMetric();

	virtual bool CalculateDifference(const Pose& groundTruthPose, const Pose& solvedPose, float& result);

	static RegisterBaseErrorMetric<WeightedPositionErrorMetric> Register;

	virtual BaseErrorMetric* Clone() const;
private:
	double longestBone;
};