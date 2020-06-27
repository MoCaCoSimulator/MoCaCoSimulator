#pragma once

#include "../vector.h"
#include "../Quaternion.h"
#include "../Utils.h"
#include "Hierarchy.h"
#include "RotationLimit.h"
#include "../Transform.h"

#include <string>
#include <QDebug>
#include "../MeshModel.h"

namespace RootMotion
{
	/// <summary>
	/// The base abstract class for all %IK solvers
	/// </summary>
	//[System.Serializable]
	class IKSolver
	{
#pragma region Main Interface
	public:
		/// <summary>
		/// The most basic element type in the %IK chain that all other types extend from.
		/// </summary>
		//[System.Serializable]
		class Point
		{
		public:
			Point() : transform(nullptr) {};

			Point(Transform* transform)
			: transform(transform)
			{
			}

			Point(Transform* transform, float weight)
			: transform(transform), 
			  weight(weight)
			{
			}

			/// <summary>
			/// The transform.
			/// </summary>
			Transform* transform = nullptr;

			/// <summary>
			/// The weight of this bone in the solver.
			/// </summary>
			//[Range(0f, 1f)]
			float weight = 1.0f;

			/// <summary>
			/// Virtual position in the %IK solver.
			/// </summary>
			Vector3 solverPosition = Vector3::zero;

			/// <summary>
			/// Virtual rotation in the %IK solver.
			/// </summary>
			Quaternion solverRotation = Quaternion::identity;

			/// <summary>
			/// The default local position of the Transform.
			/// </summary>
			Vector3 defaultLocalPosition = Vector3::zero;

			/// <summary>
			/// The default local rotation of the Transform.
			/// </summary>
			Quaternion defaultLocalRotation = Quaternion::identity;

			/// <summary>
			/// Stores the default local state of the point.
			/// </summary>
			void StoreDefaultLocalState()
			{
				defaultLocalPosition = transform->LocalPosition();
				defaultLocalRotation = transform->LocalRotation();
			}

			/// <summary>
			/// Fixes the transform to it's default local state.
			/// </summary>
			void FixTransform()
			{
				if (transform->LocalPosition() != defaultLocalPosition) transform->LocalPosition(defaultLocalPosition);
				if (transform->LocalRotation() != defaultLocalRotation) transform->LocalRotation(defaultLocalRotation);
			}

			/// <summary>
			/// Updates the solverPosition (in world space).
			/// </summary>
			void UpdateSolverPosition()
			{
				solverPosition = transform->Position();
			}

			/// <summary>
			/// Updates the solverPosition (in local space).
			/// </summary>
			void UpdateSolverLocalPosition()
			{
				solverPosition = transform->LocalPosition();
			}

			/// <summary>
			/// Updates the solverPosition/Rotation (in world space).
			/// </summary>
			void UpdateSolverState()
			{
				solverPosition = transform->Position();
				solverRotation = transform->Rotation();
			}

			/// <summary>
			/// Updates the solverPosition/Rotation (in local space).
			/// </summary>
			void UpdateSolverLocalState()
			{
				solverPosition = transform->LocalPosition();
				solverRotation = transform->LocalRotation();
			}
		};

		/// <summary>
		/// %Bone type of element in the %IK chain. Used in the case of skeletal Transform hierarchies.
		/// </summary>
		//[System.Serializable]
		class Bone : public Point
		{
		public:
			/// <summary>
			/// The length of the bone.
			/// </summary>
			float length = 0.0f;
			/// <summary>
			/// The sqr mag of the bone.
			/// </summary>
			float sqrMag = 0.0f;
			/// <summary>
			/// Local axis to target/child bone.
			/// </summary>
			Vector3 axis = -Vector3::right;

			/// <summary>
			/// Gets the rotation limit component from the Transform if there is any.
			/// </summary>
			RotationLimit* GetRotationLimit()
			{
				if (!isLimited) return nullptr;
				//if (_rotationLimit == nullptr) _rotationLimit = transform.GetComponent<RotationLimit>();
				isLimited = _rotationLimit != nullptr;
				return _rotationLimit;
			}

			void SetRotationLimit(RotationLimit* value)
			{
				_rotationLimit = value;
				isLimited = value != nullptr;
			}

			/// <summary>
			/// Swings the Transform's axis towards the swing target
			/// </summary>
			void Swing(Vector3 swingTarget, float weight = 1.0f)
			{
				if (weight <= 0.0f) return;

				Quaternion r = Quaternion::FromToRotation(transform->Rotation() * axis, swingTarget - transform->Position());

				if (weight >= 1.0f)
				{
					transform->Rotation(r * transform->Rotation());
					return;
				}

				transform->Rotation(Quaternion::Lerp(Quaternion::identity, r, weight) * transform->Rotation());
			}

			static void SolverSwing(std::vector<Bone*> bones, int index, Vector3 swingTarget, float weight = 1.0f)
			{
				if (weight <= 0.0f) return;

				Quaternion r = Quaternion::FromToRotation(bones[index]->solverRotation * bones[index]->axis, swingTarget - bones[index]->solverPosition);

				if (weight >= 1.0f)
				{
					for (int i = index; i < bones.size(); i++)
					{
						bones[i]->solverRotation = r * bones[i]->solverRotation;
					}
					return;
				}

				for (int i = index; i < bones.size(); i++)
				{
					bones[i]->solverRotation = Quaternion::Lerp(Quaternion::identity, r, weight) * bones[i]->solverRotation;
				}
			}

			/*
			 * Swings the Transform's axis towards the swing target on the XY plane only
			 * */
			void Swing2D(Vector3 swingTarget, float weight = 1.0f)
			{
				if (weight <= 0.0f) return;

				Vector3 from = transform->Rotation() * axis;
				Vector3 to = swingTarget - transform->Position();

				float angleFrom = std::atan2(from.x, from.y) * Utils::RAD2DEG;
				float angleTo = std::atan2(to.x, to.y) * Utils::RAD2DEG;

				transform->Rotation(Quaternion::AngleAxis(Utils::DeltaAngle(angleFrom, angleTo) * weight * Utils::DEG2RAD, -Vector3::forward) * transform->Rotation());
			}

			/*
			 * Moves the bone to the solver position
			 * */
			void SetToSolverPosition()
			{
				transform->Position(solverPosition);
			}

		private:
			RotationLimit* _rotationLimit = nullptr;
			bool isLimited = true;
		};

		/// <summary>
		/// %Node type of element in the %IK chain. Used in the case of mixed/non-hierarchical %IK systems
		/// </summary>
		//[System.Serializable]
		class Node : public Point
		{
		public:
			/// <summary>
			/// Distance to child node.
			/// </summary>
			float length = 0.0f;
			/// <summary>
			/// The effector position weight.
			/// </summary>
			float effectorPositionWeight = 0.0f;
			/// <summary>
			/// The effector rotation weight.
			/// </summary>
			float effectorRotationWeight = 0.0f;
			/// <summary>
			/// Position offset.
			/// </summary>
			Vector3 offset = Vector3::zero;
		};

		/// <summary>
		/// Determines whether this instance is valid or not.
		/// </summary>
		bool IsValid()
		{
			std::string message = "";
			return IsValid(message);
		}

		/// <summary>
		/// Determines whether this instance is valid or not. If returns false, also fills in an error message.
		/// </summary>
		virtual bool IsValid(std::string& message) = 0;

		/// <summary>
		/// Initiate the solver with specified root Transform. Use only if this %IKSolver is not a member of an %IK component.
		/// </summary>
		void Initiate(Transform* root)
		{
			if (OnPreInitiate != nullptr) OnPreInitiate();

			if (root == nullptr) qDebug() << "Initiating IKSolver with null root Transform.";
			this->root = root;
			initiated = false;

			std::string message = "";
			if (!IsValid(message))
			{
				qDebug() << "Error happened: " << message.c_str();
				return;
			}

			OnInitiate();
			StoreDefaultLocalState();
			initiated = true;
			firstInitiation = false;

			if (OnPostInitiate != nullptr) OnPostInitiate();
		}

		/// <summary>
		/// Updates the %IK solver. Use only if this %IKSolver is not a member of an %IK component or the %IK component has been disabled and you intend to manually control the updating.
		/// </summary>
		void Update(float deltaTime)
		{
			if (OnPreUpdate != nullptr) OnPreUpdate();

			if (firstInitiation) Initiate(root); // when the IK component has been disabled in Awake, this will initiate it.
			if (!initiated) return;

			OnUpdate(deltaTime);

			if (OnPostUpdate != nullptr) OnPostUpdate();
		}

		/// <summary>
		/// The %IK position.
		/// </summary>
		Vector3 IKPosition = Vector3::zero;

		//[Tooltip("The positional or the master weight of the solver.")]
		/// <summary>
		/// The %IK position weight or the master weight of the solver.
		/// </summary>
		//[Range(0f, 1f)]
		float IKPositionWeight = 1.0f;

		/// <summary>
		/// Gets the %IK position. NOTE: You are welcome to read IKPosition directly, this method is here only to match the Unity's built in %IK API.
		/// </summary>
		virtual Vector3 GetIKPosition()
		{
			return IKPosition;
		}

		/// <summary>
		/// Sets the %IK position. NOTE: You are welcome to set IKPosition directly, this method is here only to match the Unity's built in %IK API.
		/// </summary>
		void SetIKPosition(Vector3 position)
		{
			IKPosition = position;
		}

		/// <summary>
		/// Gets the %IK position weight. NOTE: You are welcome to read IKPositionWeight directly, this method is here only to match the Unity's built in %IK API.
		/// </summary>
		float GetIKPositionWeight()
		{
			return IKPositionWeight;
		}

		/// <summary>
		/// Sets the %IK position weight. NOTE: You are welcome to set IKPositionWeight directly, this method is here only to match the Unity's built in %IK API.
		/// </summary>
		void SetIKPositionWeight(float weight)
		{
			IKPositionWeight = std::clamp(weight, 0.0f, 1.0f);
		}

		/// <summary>
		/// Gets the root Transform.
		/// </summary>
		Transform* GetRoot()
		{
			return root;
		}

		/// <summary>
		/// Gets a value indicating whether this <see cref="IKSolver"/> has successfully initiated.
		/// </summary>
		bool GetInitiated()
		{
			return initiated;
		}

		/// <summary>
		/// Gets all the points used by the solver.
		/// </summary>
		virtual std::vector<IKSolver::Point> GetPoints() = 0;

		/// <summary>
		/// Gets the point with the specified Transform.
		/// </summary>
		virtual IKSolver::Point GetPoint(Transform transform) = 0;

		/// <summary>
		/// Fixes all the Transforms used by the solver to their initial state.
		/// </summary>
		virtual void FixTransforms() = 0;

		/// <summary>
		/// Stores the default local state for the bones used by the solver.
		/// </summary>
		virtual void StoreDefaultLocalState() = 0;
	public:
		/// <summary>
		/// Delegates solver update events.
		/// </summary>
		//delegate void UpdateDelegate();
		/// <summary>
		/// Delegates solver iteration events.
		/// </summary>
		//delegate void IterationDelegate(int i);

		/// <summary>
		/// Called before initiating the solver.
		/// </summary>
		std::function<void()> OnPreInitiate = nullptr;

		/// <summary>
		/// Called after initiating the solver.
		/// </summary>
		std::function<void()> OnPostInitiate = nullptr;

		/// <summary>
		/// Called before updating.
		/// </summary>
		std::function<void()> OnPreUpdate = nullptr;

		/// <summary>
		/// Called after writing the solved pose
		/// </summary>
		std::function<void()> OnPostUpdate = nullptr;

		#pragma endregion Main Interface

	protected:
		virtual void OnInitiate() = 0;
		virtual void OnUpdate(float deltaTime) = 0;

		bool initiated = false;
		bool firstInitiation = true;
		Transform* root = nullptr;

		void LogWarning(std::string message)
		{
			qDebug() << message.c_str();
		}

		#pragma region Class Methods

		/// <summary>
		/// Checks if an array of objects contains any duplicates.
		/// </summary>
		static Transform* ContainsDuplicateBone(std::vector<Bone*> bones)
		{
			for (int i = 0; i < bones.size(); i++)
			{
				for (int i2 = 0; i2 < bones.size(); i2++)
				{
					if (i != i2 && bones[i]->transform == bones[i2]->transform) return bones[i]->transform;
				}
			}
			return nullptr;
		}

		/*
		 * Make sure the bones are in valid Hierarchy
		 * */
		static bool HierarchyIsValid(std::vector<Bone*> bones)
		{
			for (int i = 1; i < bones.size(); i++)
			{
				// If parent bone is not an ancestor of bone, the hierarchy is invalid
				if (!Hierarchy::IsAncestor(bones[i]->transform, bones[i - 1]->transform))
				{
					return false;
				}
			}
			return true;
		}

		// Calculates bone lengths and axes, returns the length of the entire chain
		static float PreSolveBones(std::vector<Bone*> bones)
		{
			float length = 0;

			for (int i = 0; i < bones.size(); i++)
			{
				bones[i]->solverPosition = bones[i]->transform->Position();
				bones[i]->solverRotation = bones[i]->transform->Rotation();
			}

			for (int i = 0; i < bones.size(); i++)
			{
				if (i < bones.size() - 1)
				{
					bones[i]->sqrMag = (bones[i + 1]->solverPosition - bones[i]->solverPosition).lengthSquared();
					bones[i]->length = std::sqrt(bones[i]->sqrMag);
					length += bones[i]->length;

					bones[i]->axis = bones[i]->solverRotation.inverse() * (bones[i + 1]->solverPosition - bones[i]->solverPosition);
				}
				else
				{
					bones[i]->sqrMag = 0.0f;
					bones[i]->length = 0.0f;
				}
			}

			return length;
		}

	#pragma endregion Class Methods
	};
};