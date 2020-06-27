#pragma once

#define _USE_MATH_DEFINES

#include "../vector.h"

#include <algorithm>
#include <cmath>

namespace RootMotion
{
	/// <summary>
	/// Interpolation mode.
	/// </summary>
	//[System.Serializable]
	enum InterpolationMode
	{
		None,
		InOutCubic,
		InOutQuintic,
		InOutSine,
		InQuintic,
		InQuartic,
		InCubic,
		InQuadratic,
		InElastic,
		InElasticSmall,
		InElasticBig,
		InSine,
		InBack,
		OutQuintic,
		OutQuartic,
		OutCubic,
		OutInCubic,
		OutInQuartic,
		OutElastic,
		OutElasticSmall,
		OutElasticBig,
		OutSine,
		OutBack,
		OutBackCubic,
		OutBackQuartic,
		BackInCubic,
		BackInQuartic,
	};

	/// <summary>
	/// Class for various interpolation methods.
	/// </summary>
	class Interp
	{
		#pragma region Public methods

	public:
		/// <summary>
		/// Interpolate the specified t by InterpolationMode mode.
		/// </summary>
		/// <param name='t'>
		/// T.
		/// </param>
		/// <param name='mode'>
		/// InterpolationMode.
		/// </param>
		static float Float(float t, InterpolationMode mode)
		{
			float interpT = 0;

			switch (mode)
			{
			case InterpolationMode::None:
				interpT = Interp::None(t, 0, 1);
				break;
			case InterpolationMode::InOutCubic:
				interpT = Interp::InOutCubic(t, 0, 1);
				break;
			case InterpolationMode::InOutQuintic:
				interpT = Interp::InOutQuintic(t, 0, 1);
				break;
			case InterpolationMode::InQuintic:
				interpT = Interp::InQuintic(t, 0, 1);
				break;
			case InterpolationMode::InQuartic:
				interpT = Interp::InQuartic(t, 0, 1);
				break;
			case InterpolationMode::InCubic:
				interpT = Interp::InCubic(t, 0, 1);
				break;
			case InterpolationMode::InQuadratic:
				interpT = Interp::InQuadratic(t, 0, 1);
				break;
			case InterpolationMode::OutQuintic:
				interpT = Interp::OutQuintic(t, 0, 1);
				break;
			case InterpolationMode::OutQuartic:
				interpT = Interp::OutQuartic(t, 0, 1);
				break;
			case InterpolationMode::OutCubic:
				interpT = Interp::OutCubic(t, 0, 1);
				break;
			case InterpolationMode::OutInCubic:
				interpT = Interp::OutInCubic(t, 0, 1);
				break;
			case InterpolationMode::OutInQuartic:
				interpT = Interp::OutInCubic(t, 0, 1);
				break;
			case InterpolationMode::BackInCubic:
				interpT = Interp::BackInCubic(t, 0, 1);
				break;
			case InterpolationMode::BackInQuartic:
				interpT = Interp::BackInQuartic(t, 0, 1);
				break;
			case InterpolationMode::OutBackCubic:
				interpT = Interp::OutBackCubic(t, 0, 1);
				break;
			case InterpolationMode::OutBackQuartic:
				interpT = Interp::OutBackQuartic(t, 0, 1);
				break;
			case InterpolationMode::OutElasticSmall:
				interpT = Interp::OutElasticSmall(t, 0, 1);
				break;
			case InterpolationMode::OutElasticBig:
				interpT = Interp::OutElasticBig(t, 0, 1);
				break;
			case InterpolationMode::InElasticSmall:
				interpT = Interp::InElasticSmall(t, 0, 1);
				break;
			case InterpolationMode::InElasticBig:
				interpT = Interp::InElasticBig(t, 0, 1);
				break;
			case InterpolationMode::InSine:
				interpT = Interp::InSine(t, 0, 1);
				break;
			case InterpolationMode::OutSine:
				interpT = Interp::OutSine(t, 0, 1);
				break;
			case InterpolationMode::InOutSine:
				interpT = Interp::InOutSine(t, 0, 1);
				break;
			case InterpolationMode::InElastic:
				interpT = Interp::OutElastic(t, 0, 1);
				break;
			case InterpolationMode::OutElastic:
				interpT = Interp::OutElastic(t, 0, 1);
				break;
			case InterpolationMode::InBack:
				interpT = Interp::InBack(t, 0, 1);
				break;
			case InterpolationMode::OutBack:
				interpT = Interp::OutBack(t, 0, 1);
				break;
			default: interpT = 0;
				break;
			}

			return interpT;
		}

		/// <summary>
		/// Interpolate between two verctors by InterpolationMode mode
		/// </summary>
		static Vector3 V3(Vector3 v1, Vector3 v2, float t, InterpolationMode mode)
		{
			float interpT = Interp::Float(t, mode);
			return (v1 * (1 - interpT)) + (v2 * interpT);
		}

		/// <summary>
		/// Linear interpolation of value towards target.
		/// </summary>
		static float LerpValue(float value, float target, float increaseSpeed, float decreaseSpeed, float deltaTime)
		{
			if (value == target) return target;
			if (value < target) return std::clamp(value + deltaTime * increaseSpeed, -std::numeric_limits<float>::infinity(), target);
			else return std::clamp(value - deltaTime * decreaseSpeed, target, std::numeric_limits<float>::infinity());
		}

		#pragma endregion

		#pragma region Interpolation modes

	private:
		static float None(float t, float b, float c)
		{ // time, b, distance,
			return b + c * (t);
		}

		static float InOutCubic(float t, float b, float c)
		{
			float ts = t * t;
			float tc = ts * t;
			return b + c * (-2 * tc + 3 * ts);
		}

		static float InOutQuintic(float t, float b, float c)
		{
			float ts = t * t;
			float tc = ts * t;
			return b + c * (6 * tc * ts + -15 * ts * ts + 10 * tc);
		}

		static float InQuintic(float t, float b, float c)
		{
			float ts = t * t;
			float tc = ts * t;
			return b + c * (tc * ts);
		}

		static float InQuartic(float t, float b, float c)
		{
			float ts = t * t;
			return b + c * (ts * ts);
		}

		static float InCubic(float t, float b, float c)
		{
			float ts = t * t;
			float tc = ts * t;
			return b + c * (tc);
		}

		static float InQuadratic(float t, float b, float c)
		{
			float ts = t * t;
			return b + c * (ts);
		}

		static float OutQuintic(float t, float b, float c)
		{
			float ts = t * t;
			float tc = ts * t;
			return b + c * (tc * ts + -5 * ts * ts + 10 * tc + -10 * ts + 5 * t);
		}

		static float OutQuartic(float t, float b, float c)
		{
			float ts = t * t;
			float tc = ts * t;
			return b + c * (-1 * ts * ts + 4 * tc + -6 * ts + 4 * t);
		}

		static float OutCubic(float t, float b, float c)
		{
			float ts = t * t;
			float tc = ts * t;
			return b + c * (tc + -3 * ts + 3 * t);
		}

		static float OutInCubic(float t, float b, float c)
		{
			float ts = t * t;
			float tc = ts * t;
			return b + c * (4 * tc + -6 * ts + 3 * t);
		}

		static float OutInQuartic(float t, float b, float c)
		{
			float ts = t * t;
			float tc = ts * t;
			return b + c * (6 * tc + -9 * ts + 4 * t);
		}

		static float BackInCubic(float t, float b, float c)
		{
			float ts = t * t;
			float tc = ts * t;
			return b + c * (4 * tc + -3 * ts);
		}

		static float BackInQuartic(float t, float b, float c)
		{
			float ts = t * t;
			float tc = ts * t;
			return b + c * (2 * ts * ts + 2 * tc + -3 * ts);
		}

		static float OutBackCubic(float t, float b, float c)
		{
			float ts = t * t;
			float tc = ts * t;
			return b + c * (4 * tc + -9 * ts + 6 * t);
		}

		static float OutBackQuartic(float t, float b, float c)
		{
			float ts = t * t;
			float tc = ts * t;
			return b + c * (-2 * ts * ts + 10 * tc + -15 * ts + 8 * t);
		}

		static float OutElasticSmall(float t, float b, float c)
		{
			float ts = t * t;
			float tc = ts * t;
			return b + c * (33 * tc * ts + -106 * ts * ts + 126 * tc + -67 * ts + 15 * t);
		}

		static float OutElasticBig(float t, float b, float c)
		{
			float ts = t * t;
			float tc = ts * t;
			return b + c * (56 * tc * ts + -175 * ts * ts + 200 * tc + -100 * ts + 20 * t);
		}

		static float InElasticSmall(float t, float b, float c)
		{
			float ts = t * t;
			float tc = ts * t;
			return b + c * (33 * tc * ts + -59 * ts * ts + 32 * tc + -5 * ts);
		}

		static float InElasticBig(float t, float b, float c)
		{
			float ts = t * t;
			float tc = ts * t;
			return b + c * (56 * tc * ts + -105 * ts * ts + 60 * tc + -10 * ts);
		}

		static float InSine(float t, float b, float c)
		{
			c -= b;
			return -c * std::cos(t / 1 * (M_PI / 2)) + c + b;
		}

		static float OutSine(float t, float b, float c)
		{
			c -= b;
			return c * std::sin(t / 1 * (M_PI / 2)) + b;
		}

		static float InOutSine(float t, float b, float c)
		{
			c -= b;
			return -c / 2 * (std::cos(M_PI * t / 1) - 1) + b;
		}

		static float InElastic(float t, float b, float c)
		{
			c -= b;

			float d = 1.0f;
			float p = d * .3f;
			float s = 0;
			float a = 0;

			if (t == 0) return b;

			if ((t /= d) == 1) return b + c;

			if (a == 0.0f || a < std::abs(c))
			{
				a = c;
				s = p / 4;
			}
			else
			{
				s = p / (2 * M_PI) * std::asin(c / a);
			}

			return -(a * std::pow(2, 10 * (t -= 1)) * std::sin((t * d - s) * (2 * M_PI) / p)) + b;
		}

		static float OutElastic(float t, float b, float c)
		{
			c -= b;

			float d = 1.0f;
			float p = d * .3f;
			float s = 0;
			float a = 0;

			if (t == 0) return b;

			if ((t /= d) == 1) return b + c;

			if (a == 0.0f || a < std::abs(c))
			{
				a = c;
				s = p / 4;
			}
			else
			{
				s = p / (2 * M_PI) * std::asin(c / a);
			}

			return (a * std::pow(2, -10 * t) * std::sin((t * d - s) * (2 * M_PI) / p) + c + b);
		}

		static float InBack(float t, float b, float c)
		{
			c -= b;
			t /= 1;
			float s = 1.70158f;
			return c * (t)*t * ((s + 1) * t - s) + b;
		}


		static float OutBack(float t, float b, float c)
		{
			float s = 1.70158f;
			c -= b;
			t = (t / 1) - 1;
			return c * ((t)*t * ((s + 1) * t + s) + 1) + b;
		}

		#pragma endregion
	};
}