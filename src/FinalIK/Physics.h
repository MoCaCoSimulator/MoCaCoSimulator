#pragma once

#include "../vector.h"

namespace RootMotion
{
	/// <summary>
	/// Helper class added to Final IK port to emulate Unity's raycasts
	/// </summary>
	class Physics
	{
	public:
		class RaycastHit
		{
		public:
			Vector3 point;
			float distance;
		};

		static bool Raycast(Vector3 origin, Vector3 direction, RaycastHit& hit, float magnitude)
		{
			// DO STUFF HERE

			return true;
		}

		static bool SphereCast(Vector3 origin, float radius, Vector3 direction, RaycastHit& hit, float magnitude)
		{
			// DO STUFF HERE

			return true;
		}
	};
}

