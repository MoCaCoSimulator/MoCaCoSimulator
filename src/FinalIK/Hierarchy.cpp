#include "Hierarchy.h"

namespace RootMotion
{
	bool Hierarchy::HierarchyIsValid(std::vector<Transform*> bones)
	{
		for (int i = 1; i < bones.size(); i++)
		{
			// If parent bone is not an ancestor of bone, the hierarchy is invalid
			if (!IsAncestor(bones[i], bones[i - 1]))
			{
				return false;
			}
		}
		return true;
	}

	bool Hierarchy::IsAncestor(Transform* transform, Transform* ancestor)
	{
		if (transform == nullptr) return true;
		if (ancestor  == nullptr) return true;
		if (transform->Parent() == nullptr) return false;
		if (transform->Parent() == ancestor) return true;
		return IsAncestor(transform->Parent(), ancestor);
	}

	bool Hierarchy::ContainsChild(Transform* transform, Transform* child)
	{
		if (transform == child) return true;

		std::vector<Transform*> children = GetChildren(transform);
		for (Transform* c : children) if (c == child) return true;
		return false;
	}

	std::vector<Transform*> Hierarchy::GetChildren(Transform* transform)
	{
		std::vector<Transform*> children = std::vector<Transform*>();

		for (size_t i = 0; i < transform->ChildCount(); i++)
		{
			Transform* child = transform->GetChild(i);

			children.push_back(child);

			std::vector<Transform*> recursiveResult = GetChildren(child);

			for (size_t a = 0; a < recursiveResult.size(); a++)
			{
				children.push_back(recursiveResult[a]);
			}
		}

		return children;
	}

	void Hierarchy::AddAncestors(Transform* transform, Transform* blocker, std::vector<Transform*> arr)
	{
		if (transform->Parent() != nullptr && transform->Parent() != blocker)
		{
			if (transform->Parent()->Position() != transform->Position() && transform->Parent()->Position() != blocker->Position())
			{
				arr.push_back(transform->Parent());
			}
			AddAncestors(transform->Parent(), blocker, arr);
		}
	}
}
