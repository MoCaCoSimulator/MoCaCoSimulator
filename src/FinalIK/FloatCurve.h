#pragma once

#include "Keyframe.h"

#include <vector>

namespace RootMotion
{
	class FloatCurve
	{
	public:
		std::vector<Keyframe> keys;

		float Evaluate(float time) const;

		bool operator==(const FloatCurve& rhs);

		static float GetInterpolationTime(const Keyframe& from, const Keyframe& to, const float& time);
		int FindActiveKeyIndex(float time) const;
	};
}