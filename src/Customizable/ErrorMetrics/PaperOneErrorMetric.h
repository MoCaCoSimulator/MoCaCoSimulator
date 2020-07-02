#pragma once

#include "BaseErrorMetric.h"

// Source: Interactive Control of Avatars Animated with Human Motion Data
// https://de.wikipedia.org/wiki/Euklidischer_Abstand
// https://en.wikipedia.org/wiki/Quaternion: Geodesic norm
class PaperOneErrorMetric : public BaseErrorMetric
{
public:
	PaperOneErrorMetric();

	virtual bool CalculateDifference(const Pose& groundTruthPose, const Pose& solvedPose, float& result);

	static RegisterBaseErrorMetric<PaperOneErrorMetric> Register;

	virtual BaseErrorMetric* Clone() const;
};