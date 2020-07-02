#pragma once

#include "BaseErrorMetric.h"

class AnatomicAngleErrorMetric : public BaseErrorMetric
{
public:
	AnatomicAngleErrorMetric();

	virtual bool CalculateDifference(const Pose& groundTruthPose, const Pose& solvedPose, float& result);

	static RegisterBaseErrorMetric<AnatomicAngleErrorMetric> Register;

	virtual BaseErrorMetric* Clone() const;
};