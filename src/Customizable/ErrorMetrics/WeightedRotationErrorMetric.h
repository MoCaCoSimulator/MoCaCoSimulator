#pragma once

#include "BaseErrorMetric.h"

class WeightedRotationErrorMetric : public BaseErrorMetric
{
public:
	WeightedRotationErrorMetric();

	virtual bool CalculateDifference(const Pose& groundTruthPose, const Pose& solvedPose, float& result);

	static RegisterBaseErrorMetric<WeightedRotationErrorMetric> Register;

	virtual BaseErrorMetric* Clone() const;
private:
	double longestBone;
};

