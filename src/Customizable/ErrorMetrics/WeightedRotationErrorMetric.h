#pragma once

#include "BaseErrorMetric.h"

class WeightedRotationErrorMetric : public BaseErrorMetric
{
protected:
	virtual bool CalculateDifference(const Pose& groundTruthPose, const Pose& solvedPose, float& result);
public:
	WeightedRotationErrorMetric();

	static RegisterBaseErrorMetric<WeightedRotationErrorMetric> Register;

	virtual BaseErrorMetric* Clone() const;
private:
	double longestBone;
};

