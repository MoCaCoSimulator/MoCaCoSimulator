#pragma once

#include <stdlib.h>

#include "../vector.h"
#include "../quaternion.h"
#include "../Transform.h"

namespace RootMotion
{
	/// <summary>
	/// The Cartesian axes.
	/// </summary>
	//[System.Serializable]
	enum class Axis
	{
		X,
		Y,
		Z
	};

	/// <summary>
	/// Contains tools for working with Axes that have no positive/negative directions.
	/// </summary>
	class AxisTools
	{
	public:
		/// <summary>
		/// Converts an Axis to Vector3.
		/// </summary>
		static Vector3 ToVector3(Axis axis);

		/// <summary>
		/// Converts a Vector3 to Axis.
		/// </summary>
		static Axis ToAxis(Vector3 v);

		/// <summary>
		/// Returns the Axis of the Transform towards a world space position.
		/// </summary>
		static Axis GetAxisToPoint(const Transform* t, Vector3 worldPosition);

		/// <summary>
		/// Returns the Axis of the Transform towards a world space direction.
		/// </summary>
		static Axis GetAxisToDirection(const Transform* t, Vector3 direction);

		/// <summary>
		/// Returns the local axis of the Transform towards a world space position.
		/// </summary>
		static Vector3 GetAxisVectorToPoint(const Transform* t, Vector3 worldPosition);

		/// <summary>
		/// Returns the local axis of the Transform that aligns the most with a direction.
		/// </summary>
		static Vector3 GetAxisVectorToDirection(const Transform* t, Vector3 direction);

		/// <summary>
		/// Returns the local axis of a rotation space that aligns the most with a direction.
		/// </summary>
		static Vector3 GetAxisVectorToDirection(Quaternion r, Vector3 direction);
	};
}