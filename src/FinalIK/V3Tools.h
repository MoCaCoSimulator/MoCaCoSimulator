#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>

#include "../vector.h"
#include "../Quaternion.h"
#include "../MeshModel.h"
#include "../Transform.h"

namespace RootMotion
{
	/// <summary>
	/// Helper methods for dealing with 3-dimensional vectors.
	/// </summary>
	class V3Tools
	{
	public:
		/// <summary>
		/// Optimized Vector3.Lerp
		/// </summary>
		static Vector3 Lerp(Vector3 fromVector, Vector3 toVector, float weight)
		{
			if (weight <= 0.0f) return fromVector;
			if (weight >= 1.0f) return toVector;

			return Vector3::Lerp(fromVector, toVector, weight);
		}

		/// <summary>
		/// Optimized Vector3.Slerp
		/// </summary>
		static Vector3 Slerp(Vector3 fromVector, Vector3 toVector, float weight)
		{
			if (weight <= 0.f) return fromVector;
			if (weight >= 1.f) return toVector;

			return Vector3::Slerp(fromVector, toVector, weight);
		}

		/// <summary>
		/// Returns vector projection on axis multiplied by weight.
		/// </summary>
		static Vector3 ExtractVertical(Vector3 v, Vector3 verticalAxis, float weight)
		{
			if (weight == 0.0f) return Vector3::zero;
			return Vector3::Project(v, verticalAxis) * weight;
		}

		/// <summary>
		/// Returns vector projected to a plane and multiplied by weight.
		/// </summary>
		static Vector3 ExtractHorizontal(Vector3 v, Vector3 normal, float weight)
		{
			if (weight == 0.0f) return Vector3::zero;

			Vector3 tangent = v;
			Vector3::OrthoNormalize(normal, tangent);
			return Vector3::Project(v, tangent) * weight;
		}

		/// <summary>
		/// Clamps the direction to clampWeight from normalDirection, clampSmoothing is the number of sine smoothing iterations applied on the result.
		/// </summary>
		static Vector3 ClampDirection(Vector3 direction, Vector3 normalDirection, float clampWeight, int clampSmoothing)
		{
			if (clampWeight <= 0) return direction;

			if (clampWeight >= 1.0f) return normalDirection;

			// Getting the angle between direction and normalDirection
			float angle = Vector3::AngleDegree(normalDirection, direction);
			float dot = 1.0f - (angle / 180.0f);

			if (dot > clampWeight) return direction;

			// Clamping the target
			float targetClampMlp = clampWeight > 0 ? std::clamp(1.0f - ((clampWeight - dot) / (1.0f - dot)), 0.0f, 1.0f) : 1.0f;

			// Calculating the clamp multiplier
			float clampMlp = clampWeight > 0 ? std::clamp(dot / clampWeight, 0.0f, 1.0f) : 1.0f;

			// Sine smoothing iterations
			for (int i = 0; i < clampSmoothing; i++)
			{
				float sinF = clampMlp * M_PI * 0.5f;
				clampMlp = std::sin(sinF);
			}

			// Slerping the direction (don't use Lerp here, it breaks it)
			return Vector3::Slerp(normalDirection, direction, clampMlp * targetClampMlp);
		}

		/// <summary>
		/// Clamps the direction to clampWeight from normalDirection, clampSmoothing is the number of sine smoothing iterations applied on the result.
		/// </summary>
		static Vector3 ClampDirection(Vector3 direction, Vector3 normalDirection, float clampWeight, int clampSmoothing, bool& changed)
		{
			changed = false;

			if (clampWeight <= 0.0) return direction;

			if (clampWeight >= 1.0f)
			{
				changed = true;
				return normalDirection;
			}

			// Getting the angle between direction and normalDirection
			float angle = Vector3::AngleDegree(normalDirection, direction);
			float dot = 1.0f - (angle / 180.0f);

			if (dot > clampWeight) return direction;
			changed = true;

			// Clamping the target
			float targetClampMlp = clampWeight > 0 ? std::clamp(1.0f - ((clampWeight - dot) / (1.0f - dot)), 0.0f, 1.0f) : 1.0f;

			// Calculating the clamp multiplier
			float clampMlp = clampWeight > 0 ? std::clamp(dot / clampWeight, 0.0f, 1.0f) : 1.0f;

			// Sine smoothing iterations
			for (int i = 0; i < clampSmoothing; i++)
			{
				float sinF = clampMlp * M_PI * 0.5f;
				clampMlp = std::sin(sinF);
			}

			// Slerping the direction (don't use Lerp here, it breaks it)
			return Vector3::Slerp(normalDirection, direction, clampMlp * targetClampMlp);
		}

		/// <summary>
		/// Clamps the direction to clampWeight from normalDirection, clampSmoothing is the number of sine smoothing iterations applied on the result.
		/// </summary>
		static Vector3 ClampDirection(Vector3 direction, Vector3 normalDirection, float clampWeight, int clampSmoothing, float& clampValue)
		{
			clampValue = 1.0f;

			if (clampWeight <= 0) return direction;

			if (clampWeight >= 1.0f)
			{
				return normalDirection;
			}

			// Getting the angle between direction and normalDirection
			float angle = Vector3::AngleDegree(normalDirection, direction);
			float dot = 1.0f - (angle / 180.0f);

			if (dot > clampWeight)
			{
				clampValue = 0.0f;
				return direction;
			}

			// Clamping the target
			float targetClampMlp = clampWeight > 0 ? std::clamp(1.0f - ((clampWeight - dot) / (1.0f - dot)), 0.0f, 1.0f) : 1.0f;

			// Calculating the clamp multiplier
			float clampMlp = clampWeight > 0 ? std::clamp(dot / clampWeight, 0.0f, 1.0f) : 1.0f;

			// Sine smoothing iterations
			for (int i = 0; i < clampSmoothing; i++)
			{
				float sinF = clampMlp * M_PI * 0.5f;
				clampMlp = std::sin(sinF);
			}

			// Slerping the direction (don't use Lerp here, it breaks it)
			float slerp = clampMlp * targetClampMlp;
			clampValue = 1.0f - slerp;
			return Vector3::Slerp(normalDirection, direction, slerp);
		}

		/// <summary>
		/// Get the intersection point of line and plane
		/// </summary>
		static Vector3 LineToPlane(Vector3 origin, Vector3 direction, Vector3 planeNormal, Vector3 planePoint)
		{
			float dot = (planePoint - origin).dot(planeNormal);
			float normalDot = direction.dot(planeNormal);

			if (normalDot == 0.0f) return Vector3::zero;

			float dist = dot / normalDot;
			return origin + direction.normalized() * dist;
		}

		/// <summary>
		/// Projects a point to a plane.
		/// </summary>
		static Vector3 PointToPlane(Vector3 point, Vector3 planePosition, Vector3 planeNormal)
		{
			if (planeNormal == Vector3::up)
			{
				return Vector3(point.x, planePosition.y, point.z);
			}

			Vector3 tangent = point - planePosition;
			Vector3 normal = planeNormal;
			Vector3::OrthoNormalize(normal, tangent);

			return planePosition + Vector3::Project(point - planePosition, tangent);
		}

		/// <summary>
		/// Same as Transform.TransformPoint(), but not using scale.
		/// </summary>
		static Vector3 TransformPointUnscaled(Transform& t, Vector3 point)
		{
			return t.Position() + t.Rotation() * point;
		}

		/// <summary>
		/// Same as Transform.InverseTransformPoint(), but not using scale.
		/// </summary>
		static Vector3 InverseTransformPointUnscaled(Transform& t, Vector3 point)
		{
			return Quaternion::Inverse(t.Rotation()) * (point - t.Position());
		}
	};
}