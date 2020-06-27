#pragma once

#include "../MeshModel.h"
#include "../Transform.h"

#include <vector>

namespace RootMotion
{
	/// <summary>
	/// Contains tools for working on Transform hierarchies.
	/// </summary>
	class Hierarchy
	{
	public:
		/// <summary>
		/// Make sure the bones are in valid %Hierarchy
		/// </summary>
		static bool HierarchyIsValid(std::vector<Transform*> bones);

		/// <summary>
		/// Checks if an array of objects contains any duplicates.
		/// </summary>
		/*static UnityEngine.Object ContainsDuplicate(UnityEngine.Object[] objects)
		{
			for (int i = 0; i < objects.Length; i++)
			{
				for (int i2 = 0; i2 < objects.Length; i2++)
				{
					if (i != i2 && objects[i] == objects[i2]) return objects[i];
				}
			}
			return null;
		}*/

		/// <summary>
		/// Determines whether the second Transform is an ancestor to the first Transform.
		/// </summary>
		static bool IsAncestor(Transform* transform, Transform* ancestor);

		/// <summary>
		/// Returns true if the transforms contains the child
		/// </summary>
		static bool ContainsChild(Transform* transform, Transform* child);

		static std::vector<Transform*> GetChildren(Transform* transform);

		/// <summary>
		/// Adds all Transforms until the blocker to the array
		/// </summary>
		static void AddAncestors(Transform* transform, Transform* blocker, std::vector<Transform*> arr);

		/// <summary>
		/// Gets the last ancestor that has more than minChildCount number of child Transforms 
		/// </summary>
		static Transform* GetAncestor(Transform* transform, int minChildCount)
		{
			if (transform == nullptr) return nullptr;

			if (transform->Parent() != nullptr)
			{
				if (transform->Parent()->ChildCount() >= minChildCount) return transform->Parent();
				return GetAncestor(transform->Parent(), minChildCount);
			}
			return nullptr;
		}

		/// <summary>
		/// Gets the first common ancestor up the hierarchy
		/// </summary>
		static const Transform* GetFirstCommonAncestor(Transform* t1, Transform* t2)
		{
			if (t1			== nullptr)	return nullptr;
			if (t2			== nullptr)	return nullptr;
			if (t1->Parent() == nullptr) return nullptr;
			if (t2->Parent() == nullptr) return nullptr;

			if (IsAncestor(t2, t1->Parent())) return t1->Parent();
			return GetFirstCommonAncestor(t1->Parent(), t2);
		}

		/// <summary>
		/// Gets the first common ancestor of the specified transforms.
		/// </summary>
		static const Transform* GetFirstCommonAncestor(std::vector<Transform*> transforms)
		{
			if (&transforms == nullptr)
			{
				qDebug() << "RootMotion::Hierarchy: Warning -> Transforms is null.";
				return nullptr;
			}
			if (transforms.size() == 0)
			{
				qDebug() << "RootMotion::Hierarchy: Warning -> Transforms.Length is 0.";
				return nullptr;
			}

			for (int i = 0; i < transforms.size(); i++)
			{
				if (transforms[i] == nullptr) return nullptr;

				if (IsCommonAncestor(transforms[i], transforms)) return transforms[i];
			}

			return GetFirstCommonAncestorRecursive(transforms[0], transforms);
		}

		/// <summary>
		/// Gets the first common ancestor recursively.
		/// </summary>
		static const Transform* GetFirstCommonAncestorRecursive(Transform* transform, std::vector<Transform*> transforms)
		{
			if (transform == nullptr)
			{
				qDebug() << "RootMotion::Hierarchy: Warning -> Transform is null.";
				return nullptr;
			}

			if (&transforms == nullptr)
			{
				qDebug() << "RootMotion::Hierarchy: Warning -> Transforms is null.";
				return nullptr;
			}
			if (transforms.size() == 0)
			{
				qDebug() << "RootMotion::Hierarchy: Warning -> Transforms.Length is 0.";
				return nullptr;
			}

			if (IsCommonAncestor(transform, transforms)) return transform;
			if (transform->Parent() == nullptr) return nullptr;
			return GetFirstCommonAncestorRecursive(transform->Parent(), transforms);
		}

		/// <summary>
		/// Determines whether the first parameter is the common ancestor of all the other specified transforms.
		/// </summary>
		 static bool IsCommonAncestor(Transform* transform, std::vector<Transform*> transforms)
		{
			if (transform == nullptr)
			{
				qDebug() << "RootMotion::Hierarchy: Warning -> Transform is null.";
				return false;
			}

			for (int i = 0; i < transforms.size(); i++)
			{
				if (transforms[i] == nullptr)
				{
					qDebug() << "RootMotion::Hierarchy: Log -> Transforms[" << i << "] is null.";
					return false;
				}
				if (!IsAncestor(transforms[i], transform) && transforms[i] != transform) return false;
			}
			return true;
		}
	};
}