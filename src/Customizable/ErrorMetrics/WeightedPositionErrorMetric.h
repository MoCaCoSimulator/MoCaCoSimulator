#pragma once

#include "BaseErrorMetric.h"

class WeightedPositionErrorMetric : public BaseErrorMetric
{
protected:
	virtual bool CalculateDifference(const Pose& groundTruthPose, const Pose& solvedPose, float& result);
public:
	WeightedPositionErrorMetric();

	static RegisterBaseErrorMetric<WeightedPositionErrorMetric> Register;

	virtual BaseErrorMetric* Clone() const;
private:
	double longestBone;
};