#pragma once

#define _USE_MATH_DEFINES

#include "IKSolver.h"
#include "QuaTools.h"
#include "V3Tools.h"
#include "AxisTools.h"
#include "../Utils.h"
#include "Interp.h"
#include "../Transform.h"
#include "FloatCurve.h"
#include "Physics.h"
#include "VRIK.h"

#include <algorithm>
#include <cmath>
#include <QDebug>

namespace RootMotion
{
	class IKSolverVR : public IKSolver
	{
		/// <summary>
		/// Hybrid %IK solver designed for mapping a character to a VR headset and 2 hand controllers 
		/// </summary>
		#pragma region Port of the partial class IKSolverVRUtilities.cs

	public:
		//[System.Serializable]
		enum class PositionOffset
		{
			Pelvis,
			Chest,
			Head,
			LeftHand,
			RightHand,
			LeftFoot,
			RightFoot,
			LeftHeel,
			RightHeel
		};

		//[System.Serializable]
		enum class RotationOffset
		{
			Pelvis,
			Chest,
			Head
		};

		//[System.Serializable]
		class VirtualBone
		{
		public:
			// TODO: Check if this is correct
			bool operator==(const VirtualBone& v)
			{
				return (this == &v) ? true : false;
			}

			Vector3 readPosition = Vector3::zero;
			Quaternion readRotation = Quaternion::identity;

			Vector3 solverPosition = Vector3::zero;
			Quaternion solverRotation = Quaternion::identity;

			float length = 0.0f;
			float sqrMag = 0.0f;
			Vector3 axis = Vector3::zero;;

			VirtualBone(Vector3 position, Quaternion rotation);

			void Read(Vector3 position, Quaternion rotation);

			static void SwingRotation(std::vector<VirtualBone*> bones, int index, Vector3 swingTarget, float weight = 1.0f);

			// Calculates bone lengths and axes, returns the length of the entire chain
			static float PreSolve(std::vector<VirtualBone*> bones);

			static void RotateAroundPoint(std::vector<VirtualBone*> bones, int index, Vector3 point, Quaternion rotation);

			static void RotateBy(std::vector<VirtualBone*> bones, int index, Quaternion rotation);

			static void RotateBy(std::vector<VirtualBone*> bones, Quaternion rotation);

			static void RotateTo(std::vector<VirtualBone*> bones, int index, Quaternion rotation);

			// TODO Move to IKSolverTrigonometric
			/// <summary>
			/// Solve the bone chain virtually using both solverPositions and SolverRotations. This will work the same as IKSolverTrigonometric.Solve.
			/// </summary>
			static void SolveTrigonometric(std::vector<VirtualBone*>& bones, int first, int second, int third, Vector3 targetPosition, Vector3 bendNormal, float weight);

		private:
			//Calculates the bend direction based on the law of cosines. NB! Magnitude of the returned vector does not equal to the length of the first bone!
			static Vector3 GetDirectionToBendPoint(Vector3 direction, float directionMag, Vector3 bendDirection, float sqrMag1, float sqrMag2);

		public:
			// TODO Move to IKSolverFABRIK
			// Solves a simple FABRIK pass for a bone hierarchy, not using rotation limits or singularity breaking here
			static void SolveFABRIK(std::vector<VirtualBone*> bones, Vector3 startPosition, Vector3 targetPosition, float weight, float minNormalizedTargetDistance, int iterations, float length, Vector3 startOffset);

		private:
			// Solves a FABRIK joint between two bones.
			static Vector3 SolveFABRIKJoint(Vector3 pos1, Vector3 pos2, float length);

		public:
			static void SolveCCD(std::vector<VirtualBone*> bones, Vector3 targetPosition, float weight, int iterations);
		};

#pragma endregion

		/// <summary>
		/// A base class for all IKSolverVR body parts.
		/// </summary>
		#pragma region Port of the partial class IKSolverVRBodyPart.cs

		class BodyPart
		{
		protected:
			float sqrMag = 0.0f;
			float mag = 0.0f;
			bool initiated = false;
			virtual void OnRead(const std::vector<Vector3>& positions, const std::vector<Quaternion>& rotations, bool hasChest, bool hasNeck, bool hasShoulders, bool hasToes, bool hasLegs, int rootIndex, int index) = 0;
		public:
			virtual ~BodyPart()
			{
				for (VirtualBone* bone : bones)
					delete bone;
			}

			virtual void PreSolve() = 0;
			virtual void Write(std::vector<Vector3>& solvedPositions, std::vector<Quaternion>& solvedRotations) = 0;
			virtual void ApplyOffsets() = 0;
			virtual void ResetOffsets() = 0;

			float GetSqrMag() { return sqrMag; }

			float GetMag() { return mag; }

			bool GetInitiated() { return initiated; }

			std::vector<VirtualBone*> bones = std::vector<VirtualBone*>();
		protected:
			Vector3 rootPosition = Vector3::zero;
			Quaternion rootRotation = Quaternion::identity;
			int index = -1;
			int LOD = 0;

		public:
			void SetLOD(int LOD)
			{
				this->LOD = LOD;
			}

			void Read(std::vector<Vector3>& positions, std::vector<Quaternion>& rotations, bool hasChest, bool hasNeck, bool hasShoulders, bool hasToes, bool hasLegs, int rootIndex, int index)
			{
				this->index = index;

				rootPosition = positions[rootIndex];
				rootRotation = rotations[rootIndex];

				OnRead(positions, rotations, hasChest, hasNeck, hasShoulders, hasToes, hasLegs, rootIndex, index);

				mag = VirtualBone::PreSolve(bones);
				sqrMag = mag * mag;

				initiated = true;
			}

			void MovePosition(Vector3 position)
			{
				Vector3 delta = position - bones[0]->solverPosition;
				for(VirtualBone* bone : bones) bone->solverPosition += delta;
			}

			void MoveRotation(Quaternion rotation)
			{
				Quaternion delta = QuaTools::FromToRotation(bones[0]->solverRotation, rotation);
				VirtualBone::RotateAroundPoint(bones, 0, bones[0]->solverPosition, delta);
			}

			void Translate(Vector3 position, Quaternion rotation)
			{
				MovePosition(position);
				MoveRotation(rotation);
			}

			void TranslateRoot(Vector3 newRootPos, Quaternion newRootRot)
			{
				Vector3 deltaPosition = newRootPos - rootPosition;
				rootPosition = newRootPos;
				for(VirtualBone* bone : bones) bone->solverPosition += deltaPosition;

				Quaternion deltaRotation = QuaTools::FromToRotation(rootRotation, newRootRot);
				rootRotation = newRootRot;
				VirtualBone::RotateAroundPoint(bones, 0, newRootPos, deltaRotation);
			}

			void RotateTo(VirtualBone* bone, Quaternion rotation, float weight = 1.0f)
			{
				if (weight <= 0.0f) return;

				Quaternion q = QuaTools::FromToRotation(bone->solverRotation, rotation);

				if (weight < 1.0f) q = Quaternion::Slerp(Quaternion::identity, q, weight);

				for (int i = 0; i < bones.size(); i++)
				{
					if (bones[i] == bone)
					{
						VirtualBone::RotateAroundPoint(bones, i, bones[i]->solverPosition, q);
						return;
					}
				}
			}

			void Visualize(Color color)
			{
				/*for (int i = 0; i < bones.Length - 1; i++)
				{
					Debug.DrawLine(bones[i]->solverPosition, bones[i + 1].solverPosition, color);
				}*/
			}

			void Visualize()
			{
				//Visualize(Color.white);
			}
		};

		#pragma endregion

		/// <summary>
		/// 4-segmented analytic leg chain.
		/// </summary>
		#pragma region Port of the partial class IKSolverVRLeg.cs

		class Leg : public BodyPart
		{
		private:
			Vector3 position = Vector3::zero;
			Quaternion rotation = Quaternion::identity;
			bool hasToes = false;
		public:
			//[Tooltip("The foot/toe target. This should not be the foot tracker itself, but a child GameObject parented to it so you could adjust it's position/rotation to match the orientation of the foot/toe bone. If a toe bone is assigned in the References, the solver will match the toe bone to this target. If no toe bone assigned, foot bone will be used instead.")]
			/// <summary>
			/// The foot/toe target. This should not be the foot tracker itself, but a child GameObject parented to it so you could adjust it's position/rotation to match the orientation of the foot/toe bone. If a toe bone is assigned in the References, the solver will match the toe bone to this target. If no toe bone assigned, foot bone will be used instead.
			/// </summary>
			Transform* target = nullptr;

			//[Tooltip("The knee will be bent towards this Transform if 'Bend Goal Weight' > 0.")]
			/// <summary>
			/// The knee will be bent towards this Transform if 'Bend Goal Weight' > 0.
			/// </summary>
			Transform* bendGoal = nullptr;

			//[Tooltip("Positional weight of the toe/foot target. Note that if you have nulled the target, the foot will still be pulled to the last position of the target until you set this value to 0.")]
			/// <summary>
			/// Positional weight of the toe/foot target. Note that if you have nulled the target, the foot will still be pulled to the last position of the target until you set this value to 0.
			/// </summary>
			//[Range(0f, 1f)] 
			float positionWeight = 0.0f;

			//[Tooltip("Rotational weight of the toe/foot target. Note that if you have nulled the target, the foot will still be rotated to the last rotation of the target until you set this value to 0.")]
			/// <summary>
			/// Rotational weight of the toe/foot target. Note that if you have nulled the target, the foot will still be rotated to the last rotation of the target until you set this value to 0.
			/// </summary>
			//[Range(0f, 1f)] 
			float rotationWeight = 0.0f;

			//[Tooltip("If greater than 0, will bend the knee towards the 'Bend Goal' Transform.")]
			/// <summary>
			/// If greater than 0, will bend the knee towards the 'Bend Goal' Transform.
			/// </summary>
			//[Range(0f, 1f)] 
			float bendGoalWeight = 0.0f;

			//[Tooltip("Angular offset of knee bending direction.")]
			/// <summary>
			/// Angular offset of knee bending direction.
			/// </summary>
			//[Range(-180f, 180f)]
			float swivelOffset = 0.0f;

			//[Tooltip("If 0, the bend plane will be locked to the rotation of the pelvis and rotating the foot will have no effect on the knee direction. If 1, to the target rotation of the leg so that the knee will bend towards the forward axis of the foot. Values in between will be slerped between the two.")]
			/// <summary>
			/// If 0, the bend plane will be locked to the rotation of the pelvis and rotating the foot will have no effect on the knee direction. If 1, to the target rotation of the leg so that the knee will bend towards the forward axis of the foot. Values in between will be slerped between the two.
			/// </summary>
			//[Range(0f, 1f)]
			float bendToTargetWeight = 0.5f;

			//[Tooltip("Use this to make the leg shorter/longer. Works by displacement of foot and calf localPosition.")]
			/// <summary>
			/// Use this to make the leg shorter/longer. Works by displacement of foot and calf localPosition.
			/// </summary>
			//[Range(0.01f, 2f)]
			float legLengthMlp = 1.0f;

			//[Tooltip("Evaluates stretching of the leg by target distance relative to leg length. Value at time 1 represents stretching amount at the point where distance to the target is equal to leg length. Value at time 1 represents stretching amount at the point where distance to the target is double the leg length. Value represents the amount of stretching. Linear stretching would be achieved with a linear curve going up by 45 degrees. Increase the range of stretching by moving the last key up and right at the same amount. Smoothing in the curve can help reduce knee snapping (start stretching the arm slightly before target distance reaches leg length). To get a good optimal value for this curve, please go to the 'VRIK (Basic)' demo scene and copy the stretch curve over from the Pilot character.")]
			/// <summary>
			/// Evaluates stretching of the leg by target distance relative to leg length. Value at time 1 represents stretching amount at the point where distance to the target is equal to leg length. Value at time 1 represents stretching amount at the point where distance to the target is double the leg length. Value represents the amount of stretching. Linear stretching would be achieved with a linear curve going up by 45 degrees. Increase the range of stretching by moving the last key up and right at the same amount. Smoothing in the curve can help reduce knee snapping (start stretching the arm slightly before target distance reaches leg length). To get a good optimal value for this curve, please go to the 'VRIK (Basic)' demo scene and copy the stretch curve over from the Pilot character.
			/// </summary>
			FloatCurve stretchCurve = FloatCurve();

			/// <summary>
			/// Target position of the toe/foot. Will be overwritten if target is assigned.
			/// </summary>
			//[NonSerialized][HideInInspector]
			Vector3 IKPosition = Vector3::zero;

			/// <summary>
			/// Target rotation of the toe/foot. Will be overwritten if target is assigned.
			/// </summary>
			//[NonSerialized][HideInInspector] 
			Quaternion IKRotation = Quaternion::identity;

			/// <summary>
			/// Position offset of the toe/foot. Will be applied on top of target position and reset to Vector3.zero after each update.
			/// </summary>
			//[NonSerialized][HideInInspector] 
			Vector3 footPositionOffset = Vector3::zero;

			/// <summary>
			/// Position offset of the heel. Will be reset to Vector3.zero after each update.
			/// </summary>
			//[NonSerialized][HideInInspector] 
			Vector3 heelPositionOffset = Vector3::zero;

			/// <summary>
			/// Rotation offset of the toe/foot. Will be reset to Quaternion.identity after each update.
			/// </summary>
			//[NonSerialized][HideInInspector] 
			Quaternion footRotationOffset = Quaternion::identity;

			/// <summary>
			/// The length of the leg (calculated in last read).
			/// </summary>
			//[NonSerialized][HideInInspector] 
			float currentMag = 0.0f;

			/// <summary>
			/// If true, will sample the leg bend angle each frame from the animation.
			/// </summary>
			//[HideInInspector]
			bool useAnimatedBendNormal = false;

			Vector3 GetPosition()
			{
				return position;
			}

			Quaternion GetRotation()
			{
				return rotation;
			}

			bool GetHasToes()
			{
				return hasToes;
			}
			
			VirtualBone* thigh(){ return bones[0]; }
		private:
			VirtualBone* calf(){ return bones[1]; }
			VirtualBone* foot(){ return bones[2]; }
			VirtualBone* toes(){ return bones[3]; }
		public:
			VirtualBone* lastBone(){ return bones[bones.size() - 1]; }
			Vector3& GetThighRelativeToPelvis() { return thighRelativeToPelvis; }
		private:
			Vector3 thighRelativeToPelvis = Vector3::zero;
			Vector3 footPosition = Vector3::zero;
			Quaternion footRotation = Quaternion::identity;
			Vector3 bendNormal = Vector3::zero;
			Quaternion calfRelToThigh = Quaternion::identity;
			Quaternion thighRelToFoot = Quaternion::identity;
			Vector3 bendNormalRelToPelvis = Vector3::zero;
			Vector3 bendNormalRelToTarget = Vector3::zero;
		protected:
			void OnRead(const std::vector<Vector3>& positions, const std::vector<Quaternion>& rotations, bool hasChest, bool hasNeck, bool hasShoulders, bool hasToes, bool hasLegs, int rootIndex, int index) override
			{
				Vector3 thighPos = positions[index];
				Quaternion thighRot = rotations[index];
				Vector3 calfPos = positions[index + 1];
				Quaternion calfRot = rotations[index + 1];
				Vector3 footPos = positions[index + 2];
				Quaternion footRot = rotations[index + 2];
				Vector3 toePos = positions[index + 3];
				Quaternion toeRot = rotations[index + 3];

				if (!initiated)
				{
					this->hasToes = hasToes;
					bones = std::vector<VirtualBone*>(hasToes ? 4 : 3);

					if (hasToes)
					{
						bones[0] = new VirtualBone(thighPos, thighRot);
						bones[1] = new VirtualBone(calfPos, calfRot);
						bones[2] = new VirtualBone(footPos, footRot);
						bones[3] = new VirtualBone(toePos, toeRot);

						IKPosition = toePos;
						IKRotation = toeRot;
					}
					else
					{
						bones[0] = new VirtualBone(thighPos, thighRot);
						bones[1] = new VirtualBone(calfPos, calfRot);
						bones[2] = new VirtualBone(footPos, footRot);

						IKPosition = footPos;
						IKRotation = footRot;
					}

					bendNormal = (calfPos - thighPos).cross(footPos - calfPos);

					bendNormalRelToPelvis = rootRotation.inverse() * bendNormal;
					bendNormalRelToTarget = IKRotation.inverse() * bendNormal;

					rotation = IKRotation;
				}

				if (hasToes)
				{
					bones[0]->Read(thighPos, thighRot);
					bones[1]->Read(calfPos, calfRot);
					bones[2]->Read(footPos, footRot);
					bones[3]->Read(toePos, toeRot);
				}
				else
				{
					bones[0]->Read(thighPos, thighRot);
					bones[1]->Read(calfPos, calfRot);
					bones[2]->Read(footPos, footRot);
				}
			}

		public:
			void PreSolve() override
			{
				if (target != nullptr)
				{
					IKPosition = target->Position();
					IKRotation = target->Rotation();
				}

				footPosition = foot()->solverPosition;
				footRotation = foot()->solverRotation;

				position = lastBone()->solverPosition;
				rotation = lastBone()->solverRotation;

				if (rotationWeight > 0.0f)
				{
					ApplyRotationOffset(QuaTools::FromToRotation(rotation, IKRotation), rotationWeight);
				}

				if (positionWeight > 0.0f)
				{
					ApplyPositionOffset(IKPosition - position, positionWeight);
				}

				thighRelativeToPelvis = rootRotation.inverse() * (thigh()->solverPosition - rootPosition);
				calfRelToThigh = thigh()->solverRotation.inverse() * calf()->solverRotation;
				thighRelToFoot = lastBone()->solverRotation.inverse() * thigh()->solverRotation;

				// Calculate bend plane normal
				if (useAnimatedBendNormal)
				{
					// This was used before version 1.8
					bendNormal = (calf()->solverPosition - thigh()->solverPosition).cross(foot()->solverPosition - calf()->solverPosition);
				}
				else
				{
					if (bendToTargetWeight <= 0.0f)
					{
						bendNormal = rootRotation * bendNormalRelToPelvis;
					}
					else if (bendToTargetWeight >= 1.0f)
					{
						bendNormal = rotation * bendNormalRelToTarget;
					}
					else
					{
						Vector3 relToPelvis = rootRotation * bendNormalRelToPelvis;
						Vector3 relToTarget = rotation * bendNormalRelToTarget;
						bendNormal = Vector3::Slerp(relToPelvis, relToTarget, bendToTargetWeight);
					}
				}
			}

			void ApplyOffsets() override
			{
				ApplyPositionOffset(footPositionOffset, 1.0f);
				ApplyRotationOffset(footRotationOffset, 1.0f);

				// Heel position offset
				Quaternion fromTo = Quaternion::FromToRotation(footPosition - position, footPosition + heelPositionOffset - position);
				footPosition = position + fromTo * (footPosition - position);
				footRotation = fromTo * footRotation;

				// Bend normal offset
				float bAngle = 0.0f;

				if (bendGoal != nullptr && bendGoalWeight > 0.0f)
				{
					Vector3 b = (bendGoal->Position() - thigh()->solverPosition).cross(position - thigh()->solverPosition);
					Quaternion l = Quaternion::LookRotation(bendNormal, thigh()->solverPosition - foot()->solverPosition);
					Vector3 bRelative = l.inverse() * b;
					bAngle = std::atan2(bRelative.x, bRelative.z) * Utils::RAD2DEG * bendGoalWeight;
				}

				float sO = swivelOffset + bAngle;

				if (sO != 0.0f)
				{
					bendNormal = Quaternion::AngleAxis(sO * Utils::DEG2RAD, thigh()->solverPosition - lastBone()->solverPosition) * bendNormal;
					thigh()->solverRotation = Quaternion::AngleAxis(-sO * Utils::DEG2RAD, thigh()->solverRotation * thigh()->axis) * thigh()->solverRotation;
				}
			}

		private:
			// Foot position offset
			void ApplyPositionOffset(Vector3 offset, float weight)
			{
				if (weight <= 0.0f) return;
				offset = offset * weight;

				// Foot position offset
				footPosition += offset;
				position += offset;
			}

			// Foot rotation offset
			void ApplyRotationOffset(Quaternion offset, float weight)
			{
				if (weight <= 0.0f) return;
				if (weight < 1.0f)
				{
					offset = Quaternion::Lerp(Quaternion::identity, offset, weight);
				}

				footRotation = offset * footRotation;
				rotation = offset * rotation;
				bendNormal = offset * bendNormal;
				footPosition = position + offset * (footPosition - position);
			}

		public:
			void Solve(bool stretch)
			{
				if (stretch && LOD < 1) Stretching();

				// Foot pass
				VirtualBone::SolveTrigonometric(bones, 0, 1, 2, footPosition, bendNormal, 1.0f);

				// Rotate foot back to where it was before the last solving
				RotateTo(foot(), footRotation);

				// Toes pass
				if (!hasToes)
				{
					FixTwistRotations();
					return;
				}

				Vector3 b = (foot()->solverPosition - thigh()->solverPosition).cross(toes()->solverPosition - foot()->solverPosition);

				VirtualBone::SolveTrigonometric(bones, 0, 2, 3, position, b, 1.0f);

				// Fix thigh twist relative to target rotation
				FixTwistRotations();

				// Keep toe rotation fixed
				toes()->solverRotation = rotation;
			}

		private:
			void FixTwistRotations()
			{
				if (LOD < 1)
				{
					if (bendToTargetWeight > 0.0f)
					{
						// Fix thigh twist relative to target rotation
						Quaternion thighRotation = rotation * thighRelToFoot;
						Quaternion f = Quaternion::FromToRotation(thighRotation * thigh()->axis, calf()->solverPosition - thigh()->solverPosition);
						if (bendToTargetWeight < 1.0f)
						{
							thigh()->solverRotation = Quaternion::Slerp(thigh()->solverRotation, f * thighRotation, bendToTargetWeight);
						}
						else
						{
							thigh()->solverRotation = f * thighRotation;
						}
					}

					// Fix calf twist relative to thigh
					Quaternion calfRotation = thigh()->solverRotation * calfRelToThigh;
					Quaternion fromTo = Quaternion::FromToRotation(calfRotation * calf()->axis, foot()->solverPosition - calf()->solverPosition);
					calf()->solverRotation = fromTo * calfRotation;
				}
			}

			void Stretching()
			{
				// Adjusting leg length
				float legLength = thigh()->length + calf()->length;
				Vector3 kneeAdd = Vector3::zero;
				Vector3 footAdd = Vector3::zero;

				if (legLengthMlp != 1.0f)
				{
					legLength *= legLengthMlp;
					kneeAdd = (calf()->solverPosition - thigh()->solverPosition) * (legLengthMlp - 1.0f) * positionWeight;
					footAdd = (foot()->solverPosition - calf()->solverPosition)  * (legLengthMlp - 1.0f) * positionWeight;
					calf()->solverPosition += kneeAdd;
					foot()->solverPosition += kneeAdd + footAdd;
					if (hasToes) toes()->solverPosition += kneeAdd + footAdd;
				}

				// Stretching
				float distanceToTarget = Vector3::Distance(thigh()->solverPosition, footPosition);
				float stretchF = distanceToTarget / legLength;

				float m = stretchCurve.Evaluate(stretchF) * positionWeight;
				//m *= positionWeight;

				kneeAdd = (calf()->solverPosition - thigh()->solverPosition) * m;
				footAdd = (foot()->solverPosition - calf()->solverPosition) * m;

				calf()->solverPosition += kneeAdd;
				foot()->solverPosition += kneeAdd + footAdd;
				if (hasToes) toes()->solverPosition += kneeAdd + footAdd;
			}

		public:
			void Write(std::vector<Vector3>& solvedPositions, std::vector<Quaternion>& solvedRotations) override
			{
				solvedRotations[index]	   = thigh()->solverRotation;
				solvedRotations[index + 1] = calf()->solverRotation;
				solvedRotations[index + 2] = foot()->solverRotation;

				solvedPositions[index] = thigh()->solverPosition;
				solvedPositions[index + 1] = calf()->solverPosition;
				solvedPositions[index + 2] = foot()->solverPosition;

				if (hasToes)
				{
					solvedRotations[index + 3] = toes()->solverRotation;
					solvedPositions[index + 3] = toes()->solverPosition;
				}
			}

			void ResetOffsets() override
			{
				footPositionOffset = Vector3::zero;
				footRotationOffset = Quaternion::identity;
				heelPositionOffset = Vector3::zero;
			}
		};

		#pragma endregion

		/// <summary>
		/// 4-segmented analytic arm chain.
		/// </summary>
		#pragma region Port of the partial class IKSolverVRArm.cs

		class Arm : public BodyPart
		{
		private:
			Vector3 position = Vector3::zero;
			Quaternion rotation = Quaternion::identity;

		public:
			//[System.Serializable]
			enum ShoulderRotationMode
			{
				YawPitch,
				FromTo
			};

			//[Tooltip("The hand target. This should not be the hand controller itself, but a child GameObject parented to it so you could adjust it's position/rotation to match the orientation of the hand bone. The best practice for setup would be to move the hand controller to the avatar's hand as it it was held by the avatar, duplicate the avatar's hand bone and parent it to the hand controller. Then assign the duplicate to this slot.")]
			/// <summary>
			/// The hand target. This should not be the hand controller itself, but a child GameObject parented to it so you could adjust it's position/rotation to match the orientation of the hand bone. The best practice for setup would be to move the hand controller to the avatar's hand as it it was held by the avatar, duplicate the avatar's hand bone and parent it to the hand controller. Then assign the duplicate to this slot.
			/// </summary>
			Transform* target = nullptr;

			//[Tooltip("The elbow will be bent towards this Transform if 'Bend Goal Weight' > 0.")]
			/// <summary>
			/// The elbow will be bent towards this Transform if 'Bend Goal Weight' > 0.
			/// </summary>
			Transform* bendGoal = nullptr;

			//[Tooltip("Positional weight of the hand target. Note that if you have nulled the target, the hand will still be pulled to the last position of the target until you set this value to 0.")]
			/// <summary>
			/// Positional weight of the hand target. Note that if you have nulled the target, the hand will still be pulled to the last position of the target until you set this value to 0.
			/// </summary>
			//[Range(0f, 1f)]
			float positionWeight = 1.0f;

			//[Tooltip("Rotational weight of the hand target. Note that if you have nulled the target, the hand will still be rotated to the last rotation of the target until you set this value to 0.")]
			/// <summary>
			/// Rotational weight of the hand target. Note that if you have nulled the target, the hand will still be rotated to the last rotation of the target until you set this value to 0.
			/// </summary>
			//[Range(0f, 1f)]
			float rotationWeight = 1.0f;

			//[Tooltip("Different techniques for shoulder bone rotation.")]
			/// <summary>
			/// Different techniques for shoulder bone rotation.
			/// </summary>
			ShoulderRotationMode shoulderRotationMode = ShoulderRotationMode::YawPitch;

			//[Tooltip("The weight of shoulder rotation")]
			/// <summary>
			/// The weight of shoulder rotation.
			/// </summary>
			//[Range(0f, 1f)] 
			float shoulderRotationWeight = 1.0f;

			//[Tooltip("The weight of twisting the shoulders backwards when arms are lifted up.")]
			/// <summary>
			/// The weight of twisting the shoulders backwards when arms are lifted up.
			/// </summary>
			//[Range(0f, 1f)] 
			float shoulderTwistWeight = 1.0f;

			//[Tooltip("If greater than 0, will bend the elbow towards the 'Bend Goal' Transform.")]
			/// <summary>
			/// If greater than 0, will bend the elbow towards the 'Bend Goal' Transform.
			/// </summary>
			//[Range(0f, 1f)]
			float bendGoalWeight = 0.0f;

			//[Tooltip("Angular offset of the elbow bending direction.")]
			/// <summary>
			/// Angular offset of the elbow bending direction.
			/// </summary>
			//[Range(-180f, 180f)] 
			float swivelOffset = 0.0f;

			//[Tooltip("Local axis of the hand bone that points from the wrist towards the palm. Used for defining hand bone orientation. If you have copied VRIK component from another avatar that has different bone orientations, right-click on VRIK header and select 'Guess Hand Orientations' from the context menu.")]
			/// <summary>
			/// Local axis of the hand bone that points from the wrist towards the palm. Used for defining hand bone orientation. If you have copied VRIK component from another avatar that has different bone orientations, right-click on VRIK header and select 'Guess Hand Orientations' from the context menu.
			/// </summary>
			Vector3 wristToPalmAxis = Vector3::zero;

			//[Tooltip("Local axis of the hand bone that points from the palm towards the thumb. Used for defining hand bone orientation. If you have copied VRIK component from another avatar that has different bone orientations, right-click on VRIK header and select 'Guess Hand Orientations' from the context menu.")]
			/// <summary>
			/// Local axis of the hand bone that points from the palm towards the thumb. Used for defining hand bone orientation If you have copied VRIK component from another avatar that has different bone orientations, right-click on VRIK header and select 'Guess Hand Orientations' from the context menu..
			/// </summary>
			Vector3 palmToThumbAxis = Vector3::zero;

			//[Tooltip("Use this to make the arm shorter/longer. Works by displacement of hand and forearm localPosition.")]
			/// <summary>
			/// Use this to make the arm shorter/longer. Works by displacement of hand and forearm localPosition.
			/// </summary>
			//[Range(0.01f, 2f)]
			float armLengthMlp = 1.0f;

			//[Tooltip("Evaluates stretching of the arm by target distance relative to arm length. Value at time 1 represents stretching amount at the point where distance to the target is equal to arm length. Value at time 2 represents stretching amount at the point where distance to the target is double the arm length. Value represents the amount of stretching. Linear stretching would be achieved with a linear curve going up by 45 degrees. Increase the range of stretching by moving the last key up and right at the same amount. Smoothing in the curve can help reduce elbow snapping (start stretching the arm slightly before target distance reaches arm length). To get a good optimal value for this curve, please go to the 'VRIK (Basic)' demo scene and copy the stretch curve over from the Pilot character.")]
			/// <summary>
			/// Evaluates stretching of the arm by target distance relative to arm length. Value at time 1 represents stretching amount at the point where distance to the target is equal to arm length. Value at time 2 represents stretching amount at the point where distance to the target is double the arm length. Value represents the amount of stretching. Linear stretching would be achieved with a linear curve going up by 45 degrees. Increase the range of stretching by moving the last key up and right at the same amount. Smoothing in the curve can help reduce elbow snapping (start stretching the arm slightly before target distance reaches arm length). To get a good optimal value for this curve, please go to the 'VRIK (Basic)' demo scene and copy the stretch curve over from the Pilot character.
			/// </summary>
			FloatCurve stretchCurve = FloatCurve();

			/// <summary>
			/// Target position of the hand. Will be overwritten if target is assigned.
			/// </summary>
			//[NonSerialized][HideInInspector] 
			Vector3 IKPosition = Vector3::zero;

			/// <summary>
			/// Target rotation of the hand. Will be overwritten if target is assigned.
			/// </summary>
			//[NonSerialized][HideInInspector] 
			Quaternion IKRotation = Quaternion::identity;

			/// <summary>
			/// The bending direction of the limb. Will be used if bendGoalWeight is greater than 0. Will be overwritten if bendGoal is assigned.
			/// </summary>
			//[NonSerialized][HideInInspector]
			Vector3 bendDirection = -Vector3::forward;

			/// <summary>
			/// Position offset of the hand. Will be applied on top of hand target position and reset to Vector3.zero after each update.
			/// </summary>
			//[NonSerialized][HideInInspector] 
			Vector3 handPositionOffset = Vector3::zero;

			// Gets the target position of the hand.
			Vector3 GetPosition()
			{
				return position;
			}

			// Gets the target rotation of the hand

			Quaternion GetRotation()
			{
				return rotation;
			}

		private:
			bool hasShoulder = false;
			VirtualBone* shoulder() { return bones[0]; }
			VirtualBone* upperArm() { return bones[hasShoulder ? 1 : 0]; }
			VirtualBone* forearm() { return bones[hasShoulder ? 2 : 1]; }
			VirtualBone* hand() { return bones[hasShoulder ? 3 : 2]; }

			Vector3 chestForwardAxis = Vector3::zero;
			Vector3 chestUpAxis = Vector3::zero;
			Quaternion chestRotation = Quaternion::identity;
			Vector3 chestForward = Vector3::zero;
			Vector3 chestUp = Vector3::zero;
			Quaternion forearmRelToUpperArm = Quaternion::identity;
			Vector3 upperArmBendAxis = Vector3::zero;

			// Const caused the copy constructor to be deleted
			// See: https://stackoverflow.com/questions/31264984/c-compiler-error-c2280-attempting-to-reference-a-deleted-function-in-visual
			/*const*/ float yawOffsetAngle = 45.0f;
			/*const*/ float pitchOffsetAngle = -30.0f;

		protected:
			void OnRead(const std::vector<Vector3>& positions, const std::vector<Quaternion>& rotations, bool hasChest, bool hasNeck, bool hasShoulders, bool hasToes, bool hasLegs, int rootIndex, int index) override
			{
				Vector3 shoulderPosition = positions[index];
				Quaternion shoulderRotation = rotations[index];
				Vector3 upperArmPosition = positions[index + 1];
				Quaternion upperArmRotation = rotations[index + 1];
				Vector3 forearmPosition = positions[index + 2];
				Quaternion forearmRotation = rotations[index + 2];
				Vector3 handPosition = positions[index + 3];
				Quaternion handRotation = rotations[index + 3];

				if (!initiated)
				{
					IKPosition = handPosition;
					IKRotation = handRotation;
					rotation = IKRotation;

					this->hasShoulder = hasShoulders;

					bones = std::vector<VirtualBone*>(hasShoulder ? 4 : 3);

					if (hasShoulder)
					{
						bones[0] = new VirtualBone(shoulderPosition, shoulderRotation);
						bones[1] = new VirtualBone(upperArmPosition, upperArmRotation);
						bones[2] = new VirtualBone(forearmPosition, forearmRotation);
						bones[3] = new VirtualBone(handPosition, handRotation);
					}
					else
					{
						bones[0] = new VirtualBone(upperArmPosition, upperArmRotation);
						bones[1] = new VirtualBone(forearmPosition, forearmRotation);
						bones[2] = new VirtualBone(handPosition, handRotation);
					}

					Vector3 rootForward = rotations[0] * Vector3::forward;
					chestForwardAxis = rootRotation.inverse() * rootForward;
					chestUpAxis = rootRotation.inverse() * (rotations[0] * Vector3::up);

					// Get the local axis of the upper arm pointing towards the bend normal
					Vector3 upperArmForwardAxis = AxisTools::GetAxisVectorToDirection(upperArmRotation, rootForward);
					if ((upperArmRotation * upperArmForwardAxis).dot(rootForward) < 0.0f) upperArmForwardAxis = -upperArmForwardAxis;
					upperArmBendAxis = Vector3::Cross(Quaternion::Inverse(upperArmRotation) * (forearmPosition - upperArmPosition), upperArmForwardAxis);
				}

				if (hasShoulder)
				{
					bones[0]->Read(shoulderPosition, shoulderRotation);
					bones[1]->Read(upperArmPosition, upperArmRotation);
					bones[2]->Read(forearmPosition, forearmRotation);
					bones[3]->Read(handPosition, handRotation);
				}
				else
				{
					bones[0]->Read(upperArmPosition, upperArmRotation);
					bones[1]->Read(forearmPosition, forearmRotation);
					bones[2]->Read(handPosition, handRotation);
				}
			}

		public:
			void PreSolve() override
			{
				if (target != nullptr)
				{
					IKPosition = target->Position();
					IKRotation = target->Rotation();
				}

				position = V3Tools::Lerp(hand()->solverPosition, IKPosition, positionWeight);
				rotation = QuaTools::Lerp(hand()->solverRotation, IKRotation, rotationWeight);

				shoulder()->axis = shoulder()->axis.normalized();
				forearmRelToUpperArm = upperArm()->solverRotation.inverse() * forearm()->solverRotation;
			}

			void ApplyOffsets() override
			{
				position += handPositionOffset;
			}

		private:
			void Stretching()
			{
				// Adjusting arm length
				float armLength = upperArm()->length + forearm()->length;
				Vector3 elbowAdd = Vector3::zero;
				Vector3 handAdd = Vector3::zero;

				if (armLengthMlp != 1.0f)
				{
					armLength *= armLengthMlp;
					elbowAdd = (forearm()->solverPosition - upperArm()->solverPosition) * (armLengthMlp - 1.0f);
					handAdd = (hand()->solverPosition - forearm()->solverPosition) * (armLengthMlp - 1.0f);
					forearm()->solverPosition += elbowAdd;
					hand()->solverPosition += elbowAdd + handAdd;
				}

				// Stretching
				float distanceToTarget = Vector3::Distance(upperArm()->solverPosition, position);
				float stretchF = distanceToTarget / armLength;

				float m = stretchCurve.Evaluate(stretchF);
				m *= positionWeight;

				elbowAdd = (forearm()->solverPosition - upperArm()->solverPosition) * m;
				handAdd = (hand()->solverPosition - forearm()->solverPosition) * m;

				forearm()->solverPosition += elbowAdd;
				hand()->solverPosition += elbowAdd + handAdd;
			}

		public:
			void Solve(bool isLeft)
			{
				chestRotation = Quaternion::LookRotation(rootRotation * chestForwardAxis, rootRotation * chestUpAxis);
				chestForward = chestRotation * Vector3::forward;
				chestUp = chestRotation * Vector3::up;

				//Debug.DrawRay (Vector3.up * 2f, chestForward);
				//Debug.DrawRay (Vector3.up * 2f, chestUp);

				Vector3 bendNormal = Vector3::zero;

				if (hasShoulder && shoulderRotationWeight > 0.0f && LOD < 1)
				{
					switch (shoulderRotationMode)
					{
						case ShoulderRotationMode::YawPitch:
						{
							Vector3 sDir = position - shoulder()->solverPosition;
							sDir = sDir.normalized();

							// Shoulder Yaw
							float yOA = isLeft ? yawOffsetAngle : -yawOffsetAngle;
							Quaternion yawOffset = Quaternion::AngleAxis(((isLeft ? -90.0f : 90.0f) + yOA) * Utils::DEG2RAD, chestUp);
							Quaternion workingSpace = yawOffset * chestRotation;

							//Debug.DrawRay(Vector3.up * 2f, workingSpace * Vector3.forward);
							//Debug.DrawRay(Vector3.up * 2f, workingSpace * Vector3.up);

							Vector3 sDirWorking = workingSpace.inverse() * sDir;

							//Debug.DrawRay(Vector3.up * 2f, sDirWorking);

							float yaw = std::atan2(sDirWorking.x, sDirWorking.z) * Utils::RAD2DEG;

							float dotY = sDirWorking.dot(Vector3::up);
							dotY = 1.0f - std::abs(dotY);
							yaw *= dotY;

							yaw -= yOA;
							float yawLimitMin = isLeft ? -20.0f : -50.0f;
							float yawLimitMax = isLeft ? 50.0f : 20.0f;
							yaw = DamperValue(yaw, yawLimitMin - yOA, yawLimitMax - yOA, 0.7f); // back, forward

							Vector3 f = shoulder()->solverRotation * shoulder()->axis;
							Vector3 t = workingSpace * (Quaternion::AngleAxis(yaw * Utils::DEG2RAD, Vector3::up) * Vector3::forward);
							Quaternion yawRotation = Quaternion::FromToRotation(f, t);

							//Debug.DrawRay(Vector3.up * 2f, f, Color.red);
							//Debug.DrawRay(Vector3.up * 2f, t, Color.green);

							//Debug.DrawRay(Vector3.up * 2f, yawRotation * Vector3.forward, Color.blue);
							//Debug.DrawRay(Vector3.up * 2f, yawRotation * Vector3.up, Color.green);
							//Debug.DrawRay(Vector3.up * 2f, yawRotation * Vector3.right, Color.red);

							// Shoulder Pitch
							Quaternion pitchOffset = Quaternion::AngleAxis((isLeft ? -90.0f : 90.0f) * Utils::DEG2RAD, chestUp);
							workingSpace = pitchOffset * chestRotation;
							workingSpace = Quaternion::AngleAxis((isLeft ? pitchOffsetAngle : -pitchOffsetAngle) * Utils::DEG2RAD, chestForward) * workingSpace;

							//Debug.DrawRay(Vector3.up * 2f, workingSpace * Vector3.forward);
							//Debug.DrawRay(Vector3.up * 2f, workingSpace * Vector3.up);

							sDir = position - (shoulder()->solverPosition + chestRotation * (isLeft ? Vector3::right : Vector3::left) * mag);
							sDirWorking = workingSpace.inverse() * sDir;

							//Debug.DrawRay(Vector3.up * 2f, sDirWorking);

							float pitch = std::atan2(sDirWorking.y, sDirWorking.z) * Utils::RAD2DEG;

							pitch -= pitchOffsetAngle;
							pitch = DamperValue(pitch, -45.0f - pitchOffsetAngle, 45.0f - pitchOffsetAngle);

							Quaternion pitchRotation = Quaternion::AngleAxis(-pitch * Utils::DEG2RAD, workingSpace * Vector3::right);

							//Debug.DrawRay(Vector3.up * 2f, pitchRotation * Vector3.forward, Color.green);
							//Debug.DrawRay(Vector3.up * 2f, pitchRotation * Vector3.up, Color.green);

							// Rotate bones
							Quaternion sR = pitchRotation * yawRotation;
							if (shoulderRotationWeight * positionWeight < 1.0f) sR = Quaternion::Lerp(Quaternion::identity, sR, shoulderRotationWeight * positionWeight);

							VirtualBone::RotateBy(bones, sR);

							Stretching();

							// Solve trigonometric
							bendNormal = GetBendNormal(position - upperArm()->solverPosition);
							VirtualBone::SolveTrigonometric(bones, 1, 2, 3, position, bendNormal, positionWeight);

							float p = std::clamp(pitch * positionWeight * shoulderRotationWeight * shoulderTwistWeight * 2.0f, 0.0f, 180.0f);
							shoulder()->solverRotation = Quaternion::AngleAxis(p * Utils::DEG2RAD, shoulder()->solverRotation * (isLeft ? shoulder()->axis : -shoulder()->axis)) * shoulder()->solverRotation;
							upperArm()->solverRotation = Quaternion::AngleAxis(p * Utils::DEG2RAD, upperArm()->solverRotation * (isLeft ? upperArm()->axis : -upperArm()->axis)) * upperArm()->solverRotation;

							// Additional pass to reach with the shoulders
							//VirtualBone.SolveTrigonometric(bones, 0, 1, 3, position, Vector3.Cross(upperArm.solverPosition - shoulder.solverPosition, hand.solverPosition - shoulder.solverPosition), positionWeight * 0.5f);
						}
						break;
						case ShoulderRotationMode::FromTo:
						{
							Quaternion shoulderRotation = shoulder()->solverRotation;

							Quaternion r = Quaternion::FromToRotation((upperArm()->solverPosition - shoulder()->solverPosition).normalized() + chestForward, position - shoulder()->solverPosition);
							r = Quaternion::Slerp(Quaternion::identity, r, 0.5f * shoulderRotationWeight * positionWeight);
							VirtualBone::RotateBy(bones, r);

							Stretching();

							VirtualBone::SolveTrigonometric(bones, 0, 2, 3, position, Vector3::Cross(forearm()->solverPosition - shoulder()->solverPosition, hand()->solverPosition - shoulder()->solverPosition), 0.5f * shoulderRotationWeight * positionWeight);
							bendNormal = GetBendNormal(position - upperArm()->solverPosition);
							VirtualBone::SolveTrigonometric(bones, 1, 2, 3, position, bendNormal, positionWeight);

							// Twist shoulder and upper arm bones when holding hands up
							Quaternion q = Quaternion::Inverse(Quaternion::LookRotation(chestUp, chestForward));
							Vector3 vBefore = q * (shoulderRotation * shoulder()->axis);
							Vector3 vAfter = q * (shoulder()->solverRotation * shoulder()->axis);
							float angleBefore = std::atan2(vBefore.x, vBefore.z) * Utils::RAD2DEG;
							float angleAfter = std::atan2(vAfter.x, vAfter.z) * Utils::RAD2DEG;
							float pitchAngle = Utils::DeltaAngle(angleBefore, angleAfter);
							if (isLeft) pitchAngle = -pitchAngle;
							pitchAngle = std::clamp(pitchAngle * shoulderRotationWeight * shoulderTwistWeight * 2.0f * positionWeight, 0.0f, 180.0f);

							shoulder()->solverRotation = Quaternion::AngleAxis(pitchAngle * Utils::DEG2RAD, shoulder()->solverRotation * (isLeft ? shoulder()->axis : -shoulder()->axis)) * shoulder()->solverRotation;
							upperArm()->solverRotation = Quaternion::AngleAxis(pitchAngle * Utils::DEG2RAD, upperArm()->solverRotation * (isLeft ? upperArm()->axis : -upperArm()->axis)) * upperArm()->solverRotation;
						}
						break;
					}
				}
				else
				{
					if (LOD < 1) Stretching();

					bendNormal = GetBendNormal(position - upperArm()->solverPosition);
					// Solve arm trigonometric
					if (hasShoulder)
					{
						VirtualBone::SolveTrigonometric(bones, 1, 2, 3, position, bendNormal, positionWeight);
					}
					else
					{
						VirtualBone::SolveTrigonometric(bones, 0, 1, 2, position, bendNormal, positionWeight);
					}
				}

				if (LOD < 1)
				{
					// Fix upperarm twist relative to bend normal
					Quaternion space = Quaternion::LookRotation(upperArm()->solverRotation * upperArmBendAxis, forearm()->solverPosition - upperArm()->solverPosition);
					Vector3 upperArmTwist = Quaternion::Inverse(space) * bendNormal;
					float angle = std::atan2(upperArmTwist.x, upperArmTwist.z) * Utils::RAD2DEG;
					upperArm()->solverRotation = Quaternion::AngleAxis(angle * Utils::DEG2RAD, forearm()->solverPosition - upperArm()->solverPosition) * upperArm()->solverRotation;

					// Fix forearm twist relative to upper arm
					Quaternion forearmFixed = upperArm()->solverRotation * forearmRelToUpperArm;
					Quaternion fromTo = Quaternion::FromToRotation(forearmFixed * forearm()->axis, hand()->solverPosition - forearm()->solverPosition);
					RotateTo(forearm(), fromTo * forearmFixed, positionWeight);
				}

				// Set hand rotation
				if (rotationWeight >= 1.0f)
				{
					hand()->solverRotation = rotation;
				}
				else if (rotationWeight > 0.0f)
				{
					hand()->solverRotation = Quaternion::Lerp(hand()->solverRotation, rotation, rotationWeight);
				}
			}

			void ResetOffsets() override
			{
				handPositionOffset = Vector3::zero;
			}

			void Write(std::vector<Vector3>& solvedPositions, std::vector<Quaternion>& solvedRotations) override
			{
				if (hasShoulder)
				{
					solvedPositions[index] = shoulder()->solverPosition;
					solvedRotations[index] = shoulder()->solverRotation;
				}

				solvedPositions[index + 1] = upperArm()->solverPosition;
				solvedPositions[index + 2] = forearm()->solverPosition;
				solvedPositions[index + 3] = hand()->solverPosition;

				solvedRotations[index + 1] = upperArm()->solverRotation;
				solvedRotations[index + 2] = forearm()->solverRotation;
				solvedRotations[index + 3] = hand()->solverRotation;
			}

		private:
			float DamperValue(float value, float min, float max, float weight = 1.0f)
			{
				float range = max - min;

				if (weight < 1.0f)
				{
					float mid = max - range * 0.5f;
					float v = value - mid;
					v *= 0.5f;
					value = mid + v;
				}

				value -= min;

				float t = std::clamp(value / range, 0.0f, 1.0f);
				float tEased = Interp::Float(t, InterpolationMode::InOutQuintic);
				return Utils::Lerp(min, max, tEased);
			}

			Vector3 GetBendNormal(Vector3 dir)
			{
				if (bendGoal != nullptr) 
					bendDirection = bendGoal->Position() - bones[1]->solverPosition;

				Vector3 armDir = bones[0]->solverRotation * bones[0]->axis;

				Vector3 f = Vector3::down;
				Vector3 t = Quaternion::Inverse(chestRotation) * dir.normalized() + Vector3::forward;
				Quaternion q = Quaternion::FromToRotation(f, t);

				Vector3 b = q * Vector3::back;

				f = Quaternion::Inverse(chestRotation) * armDir;
				t = Quaternion::Inverse(chestRotation) * dir;
				q = Quaternion::FromToRotation(f, t);
				b = q * b;

				b = chestRotation * b;

				b += armDir;
				b -= rotation * wristToPalmAxis;
				b -= rotation * palmToThumbAxis * 0.5f;

				if (bendGoalWeight > 0.0f)
				{
					b = Vector3::Slerp(b, bendDirection, bendGoalWeight);
				}

				if (swivelOffset != 0.0f) b = Quaternion::AngleAxis(swivelOffset * Utils::DEG2RAD, -dir) * b;

				return Vector3::Cross(b, dir);
			}

			void Visualize(VirtualBone bone1, VirtualBone bone2, VirtualBone bone3, Color color)
			{
				//Debug.DrawLine(bone1.solverPosition, bone2.solverPosition, color);
				//Debug.DrawLine(bone2.solverPosition, bone3.solverPosition, color);
			}
		};

		#pragma endregion

		/// <summary>
		/// Hybrid %IK solver designed for mapping a character to a VR headset and 2 hand controllers 
		/// </summary>
		#pragma region Part of the partial class IKSolverVRSpine.cs

		class Spine : public BodyPart
		{
		public:
			//[Tooltip("The head target. This should not be the camera Transform itself, but a child GameObject parented to it so you could adjust it's position/rotation  to match the orientation of the head bone. The best practice for setup would be to move the camera to the avatar's eyes, duplicate the avatar's head bone and parent it to the camera. Then assign the duplicate to this slot.")]
			/// <summary>
			/// The head target. This should not be the camera Transform itself, but a child GameObject parented to it so you could adjust it's position/rotation to match the orientation of the head bone. The best practice for setup would be to move the camera to the avatar's eyes, duplicate the avatar's head bone and parent it to the camera. Then assign the duplicate to this slot.
			/// </summary>
			Transform* headTarget = nullptr;

			//[Tooltip("The pelvis target (optional), useful for seated rigs or if you had an additional tracker on the backpack or belt are. The best practice for setup would be to duplicate the avatar's pelvis bone and parenting it to the pelvis tracker. Then assign the duplicate to this slot.")]
			/// <summary>
			/// The pelvis target (optional), useful for seated rigs or if you had an additional tracker on the backpack or belt are. The best practice for setup would be to duplicate the avatar's pelvis bone and parenting it to the pelvis tracker. Then assign the duplicate to this slot.
			/// </summary>
			Transform* pelvisTarget = nullptr;

			//[Tooltip("Positional weight of the head target. Note that if you have nulled the headTarget, the head will still be pulled to the last position of the headTarget until you set this value to 0.")]
			/// <summary>
			/// Positional weight of the head target. Note that if you have nulled the headTarget, the head will still be pulled to the last position of the headTarget until you set this value to 0.
			/// </summary>
			//[Range(0f, 1f)] 
			float positionWeight = 1.0f;

			//[Tooltip("Rotational weight of the head target. Note that if you have nulled the headTarget, the head will still be rotated to the last rotation of the headTarget until you set this value to 0.")]
			/// <summary>
			/// Rotational weight of the head target. Note that if you have nulled the headTarget, the head will still be rotated to the last rotation of the headTarget until you set this value to 0.
			/// </summary>
			//[Range(0f, 1f)] 
			float rotationWeight = 1.0f;

			//[Tooltip("Positional weight of the pelvis target. Note that if you have nulled the pelvisTarget, the pelvis will still be pulled to the last position of the pelvisTarget until you set this value to 0.")]
			/// <summary>
			/// Positional weight of the pelvis target. Note that if you have nulled the pelvisTarget, the pelvis will still be pulled to the last position of the pelvisTarget until you set this value to 0.
			/// </summary>
			//[Range(0f, 1f)] 
			float pelvisPositionWeight = 0.0f;

			//[Tooltip("Rotational weight of the pelvis target. Note that if you have nulled the pelvisTarget, the pelvis will still be rotated to the last rotation of the pelvisTarget until you set this value to 0.")]
			/// <summary>
			/// Rotational weight of the pelvis target. Note that if you have nulled the pelvisTarget, the pelvis will still be rotated to the last rotation of the pelvisTarget until you set this value to 0.
			/// </summary>
			//[Range(0f, 1f)] 
			float pelvisRotationWeight = 0.0f;

			//[Tooltip("If 'Chest Goal Weight' is greater than 0, the chest will be turned towards this Transform.")]
			/// <summary>
			/// If chestGoalWeight is greater than 0, the chest will be turned towards this Transform.
			/// </summary>
			Transform* chestGoal = nullptr;

			//[Tooltip("Weight of turning the chest towards the 'Chest Goal'.")]
			/// <summary>
			/// Weight of turning the chest towards the chestGoal.
			/// </summary>
			//[Range(0f, 1f)] 
			float chestGoalWeight = 0.0f;

			//[Tooltip("Minimum height of the head from the root of the character.")]
			/// <summary>
			/// Minimum height of the head from the root of the character.
			/// </summary>
			float minHeadHeight = 0.8f;

			//[Tooltip("Determines how much the body will follow the position of the head.")]
			/// <summary>
			/// Determines how much the body will follow the position of the head.
			/// </summary>
			//[Range(0f, 1f)]
			float bodyPosStiffness = 0.55f;

			//[Tooltip("Determines how much the body will follow the rotation of the head.")]
			/// <summary>
			/// Determines how much the body will follow the rotation of the head.
			/// </summary>
			//[Range(0f, 1f)] 
			float bodyRotStiffness = 0.1f;

			//[Tooltip("Determines how much the chest will rotate to the rotation of the head.")]
			/// <summary>
			/// Determines how much the chest will rotate to the rotation of the head.
			/// </summary>
			//[FormerlySerializedAs("chestRotationWeight")]
			//[Range(0f, 1f)] 
			float neckStiffness = 0.2f;

			//[Tooltip("The amount of rotation applied to the chest based on hand positions.")]
			/// <summary>
			/// The amount of rotation applied to the chest based on hand positions.
			/// </summary>
			//[Range(0f, 1f)] 
			float rotateChestByHands = 1.0f;

			//[Tooltip("Clamps chest rotation. Value of 0.5 allows 90 degrees of rotation for the chest relative to the head. Value of 0 allows 180 degrees and value of 1 means the chest will be locked relative to the head.")]
			/// <summary>
			/// Clamps chest rotation. Value of 0.5 allows 90 degrees of rotation for the chest relative to the head. Value of 0 allows 180 degrees and value of 1 means the chest will be locked relative to the head.
			/// </summary>
			//[Range(0f, 1f)]
			float chestClampWeight = 0.5f;

			//[Tooltip("Clamps head rotation. Value of 0.5 allows 90 degrees of rotation for the head relative to the headTarget. Value of 0 allows 180 degrees and value of 1 means head rotation will be locked to the target.")]
			/// <summary>
			/// Clamps head rotation. Value of 0.5 allows 90 degrees of rotation for the head relative to the headTarget. Value of 0 allows 180 degrees and value of 1 means head rotation will be locked to the target.
			/// </summary>
			//[Range(0f, 1f)] 
			float headClampWeight = 0.6f;

			//[Tooltip("Moves the body horizontally along -character.forward axis by that value when the player is crouching.")]
			/// <summary>
			/// Moves the body horizontally along -character.forward axis by that value when the player is crouching.
			/// </summary>
			float moveBodyBackWhenCrouching = 0.5f;

			//[Tooltip("How much will the pelvis maintain it's animated position?")]
			/// <summary>
			/// How much will the pelvis maintain it's animated position?
			/// </summary>
			//[Range(0f, 1f)] 
			float maintainPelvisPosition = 0.2f;

			//[Tooltip("Will automatically rotate the root of the character if the head target has turned past this angle.")]
			/// <summary>
			/// Will automatically rotate the root of the character if the head target has turned past this angle.
			/// </summary>
			//[Range(0f, 180f)]
			float maxRootAngle = 25.0f;

			//[Tooltip("Angular offset for root heading. Adjust this value to turn the root relative to the HMD around the vertical axis. Usefulf for fighting or shooting games where you would sometimes want the avatar to stand at an angled stance.")]
			/// <summary>
			/// Angular offset for root heading. Adjust this value to turn the root relative to the HMD around the vertical axis. Usefulf for fighting or shooting games where you would sometimes want the avatar to stand at an angled stance.
			/// </summary>
			//[Range(-180f, 180f)]
			float rootHeadingOffset = 0.0f;

			/// <summary>
			/// Target position of the head. Will be overwritten if target is assigned.
			/// </summary>
			//[NonSerialized][HideInInspector] 
			Vector3 IKPositionHead = Vector3::zero;

			/// <summary>
			/// Target rotation of the head. Will be overwritten if target is assigned.
			/// </summary>
			//[NonSerialized][HideInInspector]
			Quaternion IKRotationHead = Quaternion::identity;

			/// <summary>
			/// Target position of the pelvis. Will be overwritten if target is assigned.
			/// </summary>
			//[NonSerialized][HideInInspector]
			Vector3 IKPositionPelvis = Vector3::zero;

			/// <summary>
			/// Target rotation of the pelvis. Will be overwritten if target is assigned.
			/// </summary>
			//[NonSerialized][HideInInspector] 
			Quaternion IKRotationPelvis = Quaternion::identity;

			/// <summary>
			/// The goal position for the chest. If chestGoalWeight > 0, the chest will be turned towards this position.
			/// </summary>
			//[NonSerialized][HideInInspector] 
			Vector3 goalPositionChest = Vector3::zero;

			/// <summary>
			/// Position offset of the pelvis. Will be applied on top of pelvis target position and reset to Vector3.zero after each update.
			/// </summary>
			//[NonSerialized][HideInInspector] 
			Vector3 pelvisPositionOffset = Vector3::zero;

			/// <summary>
			/// Position offset of the chest. Will be reset to Vector3.zero after each update.
			/// </summary>
			//[NonSerialized][HideInInspector] 
			Vector3 chestPositionOffset = Vector3::zero;

			/// <summary>
			/// Position offset of the head. Will be applied on top of head target position and reset to Vector3.zero after each update.
			/// </summary>
			//[NonSerialized][HideInInspector] 
			Vector3 headPositionOffset = Vector3::zero;

			/// <summary>
			/// Rotation offset of the pelvis. Will be reset to Quaternion.identity after each update.
			/// </summary>
			//[NonSerialized][HideInInspector]
			Quaternion pelvisRotationOffset = Quaternion::identity;

			/// <summary>
			/// Rotation offset of the chest. Will be reset to Quaternion.identity after each update.
			/// </summary>
			//[NonSerialized][HideInInspector] 
			Quaternion chestRotationOffset = Quaternion::identity;

			/// <summary>
			/// Rotation offset of the head. Will be applied on top of head target rotation and reset to Quaternion.identity after each update.
			/// </summary>
			//[NonSerialized][HideInInspector] 
			Quaternion headRotationOffset = Quaternion::identity;

			VirtualBone* pelvis() { return bones[pelvisIndex]; }

			VirtualBone* firstSpineBone() { return bones[spineIndex]; }

			VirtualBone* chest()
			{
				if (hasChest) return bones[chestIndex];
				return bones[spineIndex];
			}
		private:
			VirtualBone* neck() { return bones[neckIndex]; }

		public:
			VirtualBone* head() { return bones[headIndex]; }

			//[NonSerialized][HideInInspector] 
			Vector3 faceDirection = Vector3::zero;
			//[NonSerialized] [HideInInspector] 
			Vector3 locomotionHeadPositionOffset = Vector3::zero;
			//[NonSerialized] [HideInInspector] 
			Vector3 headPosition = Vector3::zero;

			Quaternion GetAnchorRotation() { return anchorRotation; }

			Quaternion GetAnchorRelativeToHead() { return anchorRelativeToHead; }

		private:
			Quaternion anchorRotation = Quaternion::identity;
			Quaternion anchorRelativeToHead = Quaternion::identity;

			Quaternion headRotation = Quaternion::identity;
			Quaternion pelvisRotation = Quaternion::identity;
			Quaternion anchorRelativeToPelvis = Quaternion::identity;
			Quaternion pelvisRelativeRotation = Quaternion::identity;
			Quaternion chestRelativeRotation = Quaternion::identity;
			Vector3 headDeltaPosition = Vector3::zero;
			Quaternion pelvisDeltaRotation = Quaternion::identity;
			Quaternion chestTargetRotation = Quaternion::identity;
			int pelvisIndex = 0, spineIndex = 1, chestIndex = -1, neckIndex = -1, headIndex = -1;
			float length = 0.0f;
			bool hasChest = false;
			bool hasNeck = false;
			bool hasLegs = false;
			float headHeight = 0.0f;
			float sizeMlp = 0.0f;
			Vector3 chestForward = Vector3::zero;

		protected:
			void OnRead(const std::vector<Vector3>& positions, const std::vector<Quaternion>& rotations, bool hasChest, bool hasNeck, bool hasShoulders, bool hasToes, bool hasLegs, int rootIndex, int index) override
			{
				Vector3 pelvisPos = positions[index];
				Quaternion pelvisRot = rotations[index];
				Vector3 spinePos = positions[index + 1];
				Quaternion spineRot = rotations[index + 1];
				Vector3 chestPos = positions[index + 2];
				Quaternion chestRot = rotations[index + 2];
				Vector3 neckPos = positions[index + 3];
				Quaternion neckRot = rotations[index + 3];
				Vector3 headPos = positions[index + 4];
				Quaternion headRot = rotations[index + 4];

				this->hasLegs = hasLegs;

				if (!hasChest)
				{
					chestPos = spinePos;
					chestRot = spineRot;
				}

				if (!initiated)
				{
					this->hasChest = hasChest;
					this->hasNeck = hasNeck;
					headHeight = V3Tools::ExtractVertical(headPos - positions[0], rotations[0] * Vector3::up, 1.0f).length();

					int boneCount = 3;
					if (hasChest) boneCount++;
					if (hasNeck) boneCount++;
					bones = std::vector<VirtualBone*>(boneCount);

					chestIndex = hasChest ? 2 : 1;

					neckIndex = 1;
					if (hasChest) neckIndex++;
					if (hasNeck) neckIndex++;

					headIndex = 2;
					if (hasChest) headIndex++;
					if (hasNeck) headIndex++;

					bones[0] = new VirtualBone(pelvisPos, pelvisRot);
					bones[1] = new VirtualBone(spinePos, spineRot);
					if (hasChest) bones[chestIndex] = new VirtualBone(chestPos, chestRot); // ERROR
					if (hasNeck) bones[neckIndex]   = new VirtualBone(neckPos, neckRot);
					bones[headIndex] = new VirtualBone(headPos, headRot);

					pelvisRotationOffset = Quaternion::identity;
					chestRotationOffset = Quaternion::identity;
					headRotationOffset = Quaternion::identity;

					anchorRelativeToHead = Quaternion::Inverse(headRot) * rotations[0];
					anchorRelativeToPelvis = Quaternion::Inverse(pelvisRot) * rotations[0];

					faceDirection = rotations[0] * Vector3::forward;

					IKPositionHead = headPos;
					IKRotationHead = headRot;
					IKPositionPelvis = pelvisPos;
					IKRotationPelvis = pelvisRot;
					goalPositionChest = chestPos + rotations[0] * Vector3::forward;
				}

				// Forward and up axes
				pelvisRelativeRotation = Quaternion::Inverse(headRot) * pelvisRot;
				chestRelativeRotation = Quaternion::Inverse(headRot) * chestRot;

				chestForward = Quaternion::Inverse(chestRot) * (rotations[0] * Vector3::forward);

				bones[0]->Read(pelvisPos, pelvisRot);
				bones[1]->Read(spinePos, spineRot);
				if (hasChest) bones[chestIndex]->Read(chestPos, chestRot);
				if (hasNeck)  bones[neckIndex]->Read(neckPos, neckRot);
				bones[headIndex]->Read(headPos, headRot);

				float spineLength = Vector3::Distance(pelvisPos, headPos);
				sizeMlp = spineLength / 0.7f;
			}

		public:
			void PreSolve() override
			{
				if (headTarget != nullptr)
				{
					IKPositionHead = headTarget->Position();
					IKRotationHead = headTarget->Rotation();
				}

				if (chestGoal != nullptr)
				{
					goalPositionChest = chestGoal->Position();
				}

				if (pelvisTarget != nullptr)
				{
					IKPositionPelvis = pelvisTarget->Position();
					IKRotationPelvis = pelvisTarget->Rotation();
				}

				headPosition = V3Tools::Lerp(head()->solverPosition, IKPositionHead, positionWeight);
				headRotation = QuaTools::Lerp(head()->solverRotation, IKRotationHead, rotationWeight);

				pelvisRotation = QuaTools::Lerp(pelvis()->solverRotation, IKRotationPelvis, rotationWeight);
			}

			void ApplyOffsets() override
			{
				headPosition += headPositionOffset;

				Vector3 rootUp = rootRotation * Vector3::up;
				if (rootUp == Vector3::up)
				{
					headPosition.y = std::max(rootPosition.y + minHeadHeight, headPosition.y);
				}
				else
				{
					Vector3 toHead = headPosition - rootPosition;
					Vector3 hor = V3Tools::ExtractHorizontal(toHead, rootUp, 1.0f);
					Vector3 ver = toHead - hor;
					float dot = Vector3::Dot(ver, rootUp);
					if (dot > 0.0f)
					{
						if (ver.length() < minHeadHeight) ver = ver.normalized() * minHeadHeight;
					}
					else
					{
						ver = -ver.normalized() * minHeadHeight;
					}

					headPosition = rootPosition + hor + ver;
				}

				headRotation = headRotationOffset * headRotation;

				headDeltaPosition = headPosition - head()->solverPosition;
				pelvisDeltaRotation = QuaTools::FromToRotation(pelvis()->solverRotation, headRotation * pelvisRelativeRotation);

				if (pelvisRotationWeight <= 0.0f) anchorRotation = headRotation * anchorRelativeToHead;
				else if (pelvisRotationWeight > 0.0f && pelvisRotationWeight < 1.0f) anchorRotation = Quaternion::Lerp(headRotation * anchorRelativeToHead, pelvisRotation * anchorRelativeToPelvis, pelvisRotationWeight);
				else if (pelvisRotationWeight >= 1.0f) anchorRotation = pelvisRotation * anchorRelativeToPelvis;
			}

		private:
			void CalculateChestTargetRotation(VirtualBone* rootBone, std::vector<Arm*> arms)
			{
				chestTargetRotation = headRotation * chestRelativeRotation;

				// Use hands to adjust c
				AdjustChestByHands(chestTargetRotation, arms);

				faceDirection = Vector3::Cross(anchorRotation * Vector3::right, rootBone->readRotation * Vector3::up) + anchorRotation * Vector3::forward;
			}

		public:
			void Solve(VirtualBone* rootBone, std::vector<Leg*> legs, std::vector<Arm*> arms)
			{
				CalculateChestTargetRotation(rootBone, arms);

				// Root rotation
				if (maxRootAngle < 180.0f)
				{
					Vector3 f = faceDirection;
					if (rootHeadingOffset != 0.0f) f = Quaternion::AngleAxis(rootHeadingOffset * Utils::DEG2RAD, Vector3::up) * f;
					Vector3 faceDirLocal = Quaternion::Inverse(rootBone->solverRotation) * f;
					float angle = std::atan2(faceDirLocal.x, faceDirLocal.z) * Utils::RAD2DEG;

					float rotation = 0.0f;
					float maxAngle = maxRootAngle;

					if (angle > maxAngle)
					{
						rotation = angle - maxAngle;
					}
					if (angle < -maxAngle)
					{
						rotation = angle + maxAngle;
					}

					rootBone->solverRotation = Quaternion::AngleAxis(rotation * Utils::DEG2RAD, rootBone->readRotation * Vector3::up) * rootBone->solverRotation;
				}

				Vector3 animatedPelvisPos = pelvis()->solverPosition;
				Vector3 rootUp = rootBone->solverRotation * Vector3::up;

				// Translate pelvis to make the head's position & rotation match with the head target
				TranslatePelvis(legs, headDeltaPosition, pelvisDeltaRotation);

				FABRIKPass(animatedPelvisPos, rootUp, positionWeight);

				// Bend the spine to look towards chest target rotation
				Bend(bones, pelvisIndex, chestIndex, chestTargetRotation, chestRotationOffset, chestClampWeight, false, neckStiffness * rotationWeight);

				if (LOD < 1 && chestGoalWeight > 0.0f)
				{
					Quaternion c = Quaternion::FromToRotation(bones[chestIndex]->solverRotation * chestForward, goalPositionChest - bones[chestIndex]->solverPosition) * bones[chestIndex]->solverRotation;
					Bend(bones, pelvisIndex, chestIndex, c, chestRotationOffset, chestClampWeight, false, chestGoalWeight * rotationWeight);
				}

				InverseTranslateToHead(legs, false, false, Vector3::zero, positionWeight);

				if (LOD < 1) FABRIKPass(animatedPelvisPos, rootUp, positionWeight);

				Bend(bones, neckIndex, headIndex, headRotation, headClampWeight, true, rotationWeight);

				SolvePelvis();
			}

		private:
			void FABRIKPass(Vector3 animatedPelvisPos, Vector3 rootUp, float weight)
			{
				Vector3 startPos = Vector3::Lerp(pelvis()->solverPosition, animatedPelvisPos, maintainPelvisPosition) + pelvisPositionOffset;// - chestPositionOffset;
				Vector3 endPos = headPosition - chestPositionOffset;
				//Vector3 startOffset = rootUp * (bones[bones.Length - 1].solverPosition - bones[0].solverPosition).magnitude;
				Vector3 startOffset = Vector3::zero;// (bones[bones.Length - 1].solverPosition - bones[0].solverPosition) * weight;

				float dist = Vector3::Distance(bones[0]->solverPosition, bones[bones.size() - 1]->solverPosition);

				VirtualBone::SolveFABRIK(bones, startPos, endPos, weight, 1.0f, 1, dist, startOffset);
			}

			void SolvePelvis()
			{
				// Pelvis target
				if (pelvisPositionWeight > 0.0f)
				{
					Quaternion headSolverRotation = head()->solverRotation;

					Vector3 delta = ((IKPositionPelvis + pelvisPositionOffset) - pelvis()->solverPosition) * pelvisPositionWeight;
					for (VirtualBone* bone : bones) bone->solverPosition += delta;

					Vector3 bendNormal = anchorRotation * Vector3::right;

					if (hasChest && hasNeck)
					{
						VirtualBone::SolveTrigonometric(bones, pelvisIndex, spineIndex, headIndex, headPosition, bendNormal, pelvisPositionWeight * 0.6f);
						VirtualBone::SolveTrigonometric(bones, pelvisIndex, chestIndex, headIndex, headPosition, bendNormal, pelvisPositionWeight * 0.6f);
						VirtualBone::SolveTrigonometric(bones, pelvisIndex, neckIndex, headIndex, headPosition, bendNormal, pelvisPositionWeight * 1.0f);
					}
					else if (hasChest && !hasNeck)
					{
						VirtualBone::SolveTrigonometric(bones, pelvisIndex, spineIndex, headIndex, headPosition, bendNormal, pelvisPositionWeight * 0.75f);
						VirtualBone::SolveTrigonometric(bones, pelvisIndex, chestIndex, headIndex, headPosition, bendNormal, pelvisPositionWeight * 1.0f);
					}
					else if (!hasChest && hasNeck)
					{
						VirtualBone::SolveTrigonometric(bones, pelvisIndex, spineIndex, headIndex, headPosition, bendNormal, pelvisPositionWeight * 0.75f);
						VirtualBone::SolveTrigonometric(bones, pelvisIndex, neckIndex, headIndex, headPosition, bendNormal, pelvisPositionWeight * 1.0f);
					}
					else if (!hasNeck && !hasChest)
					{
						VirtualBone::SolveTrigonometric(bones, pelvisIndex, spineIndex, headIndex, headPosition, bendNormal, pelvisPositionWeight);
					}

					head()->solverRotation = headSolverRotation;
				}
			}

		public:
			void Write(std::vector<Vector3>& solvedPositions, std::vector<Quaternion>& solvedRotations) override
			{
				// Pelvis
				solvedPositions[index] = bones[0]->solverPosition;
				solvedRotations[index] = bones[0]->solverRotation;

				// Spine
				solvedRotations[index + 1] = bones[1]->solverRotation;

				// Chest
				if (hasChest) solvedRotations[index + 2] = bones[chestIndex]->solverRotation;

				// Neck
				if (hasNeck) solvedRotations[index + 3] = bones[neckIndex]->solverRotation;

				// Head
				solvedRotations[index + 4] = bones[headIndex]->solverRotation;
			}

			void ResetOffsets() override
			{
				// Reset offsets to zero
				pelvisPositionOffset = Vector3::zero;
				chestPositionOffset = Vector3::zero;
				headPositionOffset = locomotionHeadPositionOffset;// Vector3.zero;
				pelvisRotationOffset = Quaternion::identity;
				chestRotationOffset = Quaternion::identity;
				headRotationOffset = Quaternion::identity;
			}

		private:
			void AdjustChestByHands(Quaternion& chestTargetRotation, std::vector<Arm*> arms)
			{
				if (LOD > 0) return;

				Quaternion h = Quaternion::Inverse(anchorRotation);

				Vector3 pLeft = h * (arms[0]->GetPosition() - headPosition) / sizeMlp;
				Vector3 pRight = h * (arms[1]->GetPosition() - headPosition) / sizeMlp;

				Vector3 c = Vector3::forward;
				c.x += pLeft.x * std::abs(pLeft.x);
				c.x += pLeft.z * std::abs(pLeft.z);
				c.x += pRight.x * std::abs(pRight.x);
				c.x -= pRight.z * std::abs(pRight.z);
				c.x *= 5.0f * rotateChestByHands;

				float angle = std::atan2(c.x, c.z) * Utils::RAD2DEG;
				Quaternion q = Quaternion::AngleAxis(angle * Utils::DEG2RAD, rootRotation * Vector3::up);

				chestTargetRotation = q * chestTargetRotation;

				Vector3 t = Vector3::up;
				t.x += pLeft.y;
				t.x -= pRight.y;
				t.x *= 0.5f * rotateChestByHands;

				angle = std::atan2(t.x, t.y) * Utils::RAD2DEG;
				q = Quaternion::AngleAxis(angle * Utils::DEG2RAD, rootRotation * Vector3::back);

				chestTargetRotation = q * chestTargetRotation;
			}

		public:
			// Move the pelvis so that the head would remain fixed to the anchor
			void InverseTranslateToHead(std::vector<Leg*> legs, bool limited, bool useCurrentLegMag, Vector3 offset, float w)
			{
				Vector3 delta = (headPosition + offset - head()->solverPosition) * w;// * (1f - pelvisPositionWeight); This makes the head lose it's target when pelvisPositionWeight is between 0 and 1.

				Vector3 p = pelvis()->solverPosition + delta;
				MovePosition(limited ? LimitPelvisPosition(legs, p, useCurrentLegMag) : p);
			}

		private:
			// Move and rotate the pelvis
			void TranslatePelvis(std::vector<Leg*> legs, Vector3 deltaPosition, Quaternion deltaRotation)
			{
				// Rotation
				Vector3 p = head()->solverPosition;

				deltaRotation = QuaTools::ClampRotation(deltaRotation, chestClampWeight, 2);

				Quaternion r = Quaternion::Slerp(Quaternion::identity, deltaRotation, bodyRotStiffness * rotationWeight);
				r = Quaternion::Slerp(r, QuaTools::FromToRotation(pelvis()->solverRotation, IKRotationPelvis), pelvisRotationWeight);
				VirtualBone::RotateAroundPoint(bones, 0, pelvis()->solverPosition, pelvisRotationOffset * r);

				deltaPosition -= head()->solverPosition - p;

				// Position
				// Move the body back when head is moving down
				Vector3 m = rootRotation * Vector3::forward;
				float deltaY = V3Tools::ExtractVertical(deltaPosition, rootRotation * Vector3::up, 1.0f).length();
				float backOffset = deltaY * -moveBodyBackWhenCrouching * headHeight;
				deltaPosition += m * backOffset;

				MovePosition(LimitPelvisPosition(legs, pelvis()->solverPosition + deltaPosition * bodyPosStiffness * positionWeight, false));
			}

			// Limit the position of the pelvis so that the feet/toes would remain fixed
			Vector3 LimitPelvisPosition(std::vector<Leg*> legs, Vector3 pelvisPosition, bool useCurrentLegMag, int it = 2)
			{
				if (!hasLegs) return pelvisPosition;

				// Cache leg current mag
				if (useCurrentLegMag)
				{
					for (Leg* leg : legs)
					{
						leg->currentMag = Vector3::Distance(leg->thigh()->solverPosition, leg->lastBone()->solverPosition);
					}
				}

				// Solve a 3-point constraint
				for (int i = 0; i < it; i++)
				{
					for (Leg* leg : legs)
					{
						Vector3 delta = pelvisPosition - pelvis()->solverPosition;
						Vector3 wantedThighPos = leg->thigh()->solverPosition + delta;
						Vector3 toWantedThighPos = wantedThighPos - leg->GetPosition();
						float maxMag = useCurrentLegMag ? leg->currentMag : leg->GetMag();
						Vector3 limitedThighPos = leg->GetPosition() + Vector3::ClampMagnitude(toWantedThighPos, maxMag);
						pelvisPosition += limitedThighPos - wantedThighPos;

						// TODO rotate pelvis to accommodate, rotate the spine back then
					}
				}

				return pelvisPosition;
			}

			// Bending the spine to the head effector
			void Bend(std::vector<VirtualBone*> bones, int firstIndex, int lastIndex, Quaternion targetRotation, float clampWeight, bool uniformWeight, float w)
			{
				if (w <= 0.0f) return;
				if (bones.size() == 0) return;
				int bonesCount = (lastIndex + 1) - firstIndex;
				if (bonesCount < 1) return;

				Quaternion r = QuaTools::FromToRotation(bones[lastIndex]->solverRotation, targetRotation);
				r = QuaTools::ClampRotation(r, clampWeight, 2);

				float step = uniformWeight ? 1.0f / bonesCount : 0.0f;

				for (int i = firstIndex; i < lastIndex + 1; i++)
				{
					if (!uniformWeight) step = std::clamp(((i - firstIndex) + 1) / bonesCount * 1.0f, 0.0f, 1.0f);
					VirtualBone::RotateAroundPoint(bones, i, bones[i]->solverPosition, Quaternion::Slerp(Quaternion::identity, r, step * w));
				}
			}

			// Bending the spine to the head effector
			void Bend(std::vector<VirtualBone*> bones, int firstIndex, int lastIndex, Quaternion targetRotation, Quaternion rotationOffset, float clampWeight, bool uniformWeight, float w)
			{
				if (w <= 0.0f) return;
				if (bones.size() == 0) return;
				int bonesCount = (lastIndex + 1) - firstIndex;
				if (bonesCount < 1) return;

				Quaternion r = QuaTools::FromToRotation(bones[lastIndex]->solverRotation, targetRotation);
				r = QuaTools::ClampRotation(r, clampWeight, 2);
				float step = uniformWeight ? 1.0f / bonesCount : 0.0f;

				for (int i = firstIndex; i < lastIndex + 1; i++)
				{

					if (!uniformWeight)
					{
						if (bonesCount == 1)
						{
							step = 1.0f;
						}
						else if (bonesCount == 2)
						{
							step = i == 0 ? 0.2f : 0.8f;
						}
						else if (bonesCount == 3)
						{
							if (i == 0) step = 0.15f;
							else if (i == 1) step = 0.4f;
							else step = 0.45f;
						}
						else if (bonesCount > 3)
						{
							step = 1.0f / bonesCount;
						}
					}

					//if (!uniformWeight) step = Mathf.Clamp(((i - firstIndex) + 1) / bonesCount, 0, 1f);
					VirtualBone::RotateAroundPoint(bones, i, bones[i]->solverPosition, Quaternion::Slerp(Quaternion::Slerp(Quaternion::identity, rotationOffset, step), r, step * w));
				}
			}
		};

#pragma endregion

		/// <summary>
		/// There was no summary in Final IK for this class
		/// </summary>
		#pragma region Port of the partial class IKSolverVRFootstep.cs

		class Footstep
		{
		public:
			float stepSpeed = 3.0f;
			Vector3 characterSpaceOffset = Vector3::zero;

			Vector3 position = Vector3::zero;
			Quaternion rotation = Quaternion::identity;
			Quaternion stepToRootRot = Quaternion::identity;
			bool isStepping() { return stepProgress < 1.0f; }
			bool isSupportLeg = false;

			float GetStepProgress() { return stepProgress; }
			Vector3 stepFrom = Vector3::zero;
			Vector3 stepTo = Vector3::zero;
			Quaternion stepFromRot = Quaternion::identity;
			Quaternion stepToRot = Quaternion::identity;
		private:
			float stepProgress = 0.0f;
			Quaternion footRelativeToRoot = Quaternion::identity;
			float supportLegW = 0.0f;
			float supportLegWV = 0.0f;

		public:
			Footstep(Quaternion rootRotation, Vector3 footPosition, Quaternion footRotation, Vector3 characterSpaceOffset)
			{
				this->characterSpaceOffset = characterSpaceOffset;
				Reset(rootRotation, footPosition, footRotation);
				footRelativeToRoot = Quaternion::Inverse(rootRotation) * rotation;
			}

			void Reset(Quaternion rootRotation, Vector3 footPosition, Quaternion footRotation)
			{
				position = footPosition;
				rotation = footRotation;
				stepFrom = position;
				stepTo = position;
				stepFromRot = rotation;
				stepToRot = rotation;
				stepToRootRot = rootRotation;
				stepProgress = 1.0f;
			}

			void StepTo(Vector3 p, Quaternion rootRotation, float stepThreshold)
			{
				if (Vector3::Magnitude(p - stepTo) < stepThreshold && Quaternion::Angle(rootRotation, stepToRootRot) < 25.0f) return;
				stepFrom = position;
				stepTo = p;
				stepFromRot = rotation;
				stepToRootRot = rootRotation;
				stepToRot = rootRotation * footRelativeToRoot;
				stepProgress = 0.0f;
			}

			void UpdateStepping(Vector3 p, Quaternion rootRotation, float speed, float deltaTime)
			{
				stepTo = Vector3::Lerp(stepTo, p, deltaTime * speed);
				stepToRot = Quaternion::Lerp(stepToRot, rootRotation * footRelativeToRoot, deltaTime * speed);

				stepToRootRot = stepToRot * Quaternion::Inverse(footRelativeToRoot);
			}

			void UpdateStanding(Quaternion rootRotation, float minAngle, float speed, float deltaTime)
			{
				if (speed <= 0.0f || minAngle >= 180.0f) return;

				Quaternion r = rootRotation * footRelativeToRoot;
				float angle = Quaternion::Angle(rotation, r);
				if (angle > minAngle) 
					rotation = Quaternion::RotateTowards(rotation, r, std::min(deltaTime * speed * (1.0f - supportLegW), angle - minAngle) * Utils::DEG2RAD);
			}

			void Update(InterpolationMode interpolation, std::function<void()> onStep, float deltaTime)
			{
				float supportLegWTarget = isSupportLeg ? 1.0f : 0.0f;
				supportLegW = Utils::SmoothDamp(supportLegW, supportLegWTarget, supportLegWV, 0.2f, deltaTime);

				if (!isStepping()) return;

				stepProgress = Utils::MoveTowards(stepProgress, 1.0f, deltaTime * stepSpeed);

				if (stepProgress >= 1.0f) onStep();

				float stepProgressSmooth = Interp::Float(stepProgress, interpolation);

				position = Vector3::Lerp(stepFrom, stepTo, stepProgressSmooth);
				rotation = Quaternion::Lerp(stepFromRot, stepToRot, stepProgressSmooth);
			}
		};

		#pragma endregion

		/// <summary>
		/// Hybrid %IK solver designed for mapping a character to a VR headset and 2 hand controllers 
		/// </summary>
		#pragma region Port of the partial class IKSolverVRLocomotion.cs

		class Locomotion
		{
		public:
			~Locomotion()
			{
				delete stepHeight;
				delete heelHeight;
			}

			//[Tooltip("Used for blending in/out of procedural locomotion.")]
			/// <summary>
			/// Used for blending in/out of procedural locomotion.
			/// </summary>
			//[Range(0f, 1f)] 
			float weight = 1.0f;

			//[Tooltip("Tries to maintain this distance between the legs.")]
			/// <summary>
			/// Tries to maintain this distance between the legs.
			/// </summary>
			float footDistance = 0.3f;

			//[Tooltip("Makes a step only if step target position is at least this far from the current footstep or the foot does not reach the current footstep anymore or footstep angle is past the 'Angle Threshold'.")]
			/// <summary>
			/// Makes a step only if step target position is at least this far from the current footstep or the foot does not reach the current footstep anymore or footstep angle is past the 'Angle Threshold'.
			/// </summary>
			float stepThreshold = 0.4f;

			//[Tooltip("Makes a step only if step target position is at least 'Step Threshold' far from the current footstep or the foot does not reach the current footstep anymore or footstep angle is past this value.")]
			/// <summary>
			/// Makes a step only if step target position is at least 'Step Threshold' far from the current footstep or the foot does not reach the current footstep anymore or footstep angle is past this value.
			/// </summary>
			float angleThreshold = 60.0f;

			//[Tooltip("Multiplies angle of the center of mass - center of pressure vector. Larger value makes the character step sooner if losing balance.")]
			/// <summary>
			/// Multiplies angle of the center of mass - center of pressure vector. Larger value makes the character step sooner if losing balance.
			/// </summary>
			float comAngleMlp = 1.0f;

			//[Tooltip("Maximum magnitude of head/hand target velocity used in prediction.")]
			/// <summary>
			/// Maximum magnitude of head/hand target velocity used in prediction.
			/// </summary>
			float maxVelocity = 0.4f;

			//[Tooltip("The amount of head/hand target velocity prediction.")]
			/// <summary>
			/// The amount of head/hand target velocity prediction.
			/// </summary>
			float velocityFactor = 0.4f;

			//[Tooltip("How much can a leg be extended before it is forced to step to another position? 1 means fully stretched.")]
			/// <summary>
			/// How much can a leg be extended before it is forced to step to another position? 1 means fully stretched.
			/// </summary>
			//[Range(0.9f, 1f)]
			float maxLegStretch = 1.0f;

			//[Tooltip("The speed of lerping the root of the character towards the horizontal mid-point of the footsteps.")]
			/// <summary>
			/// The speed of lerping the root of the character towards the horizontal mid-point of the footsteps.
			/// </summary>
			float rootSpeed = 20.0f;

			//[Tooltip("The speed of moving a foot to the next position.")]
			/// <summary>
			/// The speed of moving a foot to the next position.
			/// </summary>
			float stepSpeed = 3.0f;

			//[Tooltip("The height of the foot by normalized step progress (0 - 1).")]
			/// <summary>
			/// The height of the foot by normalized step progress (0 - 1).
			/// </summary>
			FloatCurve* stepHeight = nullptr;

			//[Tooltip("The height offset of the heel by normalized step progress (0 - 1).")]
			/// <summary>
			/// The height offset of the heel by normalized step progress (0 - 1).
			/// </summary>
			FloatCurve* heelHeight = nullptr;

			//[Tooltip("Rotates the foot while the leg is not stepping to relax the twist rotation of the leg if ideal rotation is past this angle.")]
			/// <summary>
			/// Rotates the foot while the leg is not stepping to relax the twist rotation of the leg if ideal rotation is past this angle.
			/// </summary>
			//[Range(0f, 180f)] 
			float relaxLegTwistMinAngle = 20.0f;

			//[Tooltip("The speed of rotating the foot while the leg is not stepping to relax the twist rotation of the leg.")]
			/// <summary>
			/// The speed of rotating the foot while the leg is not stepping to relax the twist rotation of the leg.
			/// </summary>
			float relaxLegTwistSpeed = 400.0f;

			//[Tooltip("Interpolation mode of the step.")]
			/// <summary>
			/// Interpolation mode of the step.
			/// </summary>
			InterpolationMode stepInterpolation = InterpolationMode::InOutSine;

			//[Tooltip("Offset for the approximated center of mass.")]
			/// <summary>
			/// Offset for the approximated center of mass.
			/// </summary>
			Vector3 offset = Vector3::zero;

			//[HideInInspector] 
			//bool blockingEnabled;
			//[HideInInspector] 
			//LayerMask blockingLayers;
			//[HideInInspector] 
			float raycastRadius = 0.2f;
			//[HideInInspector] 
			float raycastHeight = 0.2f;

			//[Tooltip("Called when the left foot has finished a step.")]
			/// <summary>
			/// Called when the left foot has finished a step.
			/// </summary>
			std::function<void()> onLeftFootstep = std::function<void()>();

			//[Tooltip("Called when the right foot has finished a step")]
			/// <summary>
			/// Called when the right foot has finished a step
			/// </summary>
			std::function<void()> onRightFootstep = std::function<void()>();

			/// <summary>
			/// Gets the approximated center of mass.
			/// </summary>
			Vector3& GetCenterOfMass() { return centerOfMass; }

		private:
			Vector3 centerOfMass = Vector3::zero;
			std::vector<Footstep> footsteps = std::vector<Footstep>();
			Vector3 lastComPosition = Vector3::zero;
			Vector3 comVelocity = Vector3::zero;
			int leftFootIndex = 0;
			int rightFootIndex = 0;

		public:
			void Initiate(std::vector<Vector3> positions, std::vector<Quaternion> rotations, bool hasToes)
			{
				leftFootIndex = hasToes ? 17 : 16;
				rightFootIndex = hasToes ? 21 : 20;

				footsteps.push_back(Footstep(rotations[0], positions[leftFootIndex], rotations[leftFootIndex], Vector3::left * footDistance));
				footsteps.push_back(Footstep(rotations[0], positions[rightFootIndex], rotations[rightFootIndex], Vector3::right * footDistance));
			}

			void Reset(std::vector<Vector3> positions, std::vector<Quaternion> rotations)
			{
				lastComPosition = Vector3::Lerp(positions[1], positions[5], 0.25f) + rotations[0] * offset;
				comVelocity = Vector3::zero;

				footsteps[0].Reset(rotations[0], positions[leftFootIndex], rotations[leftFootIndex]);
				footsteps[1].Reset(rotations[0], positions[rightFootIndex], rotations[rightFootIndex]);
			}

			void AddDeltaRotation(Quaternion delta, Vector3 pivot)
			{
				Vector3 toLastComPosition = lastComPosition - pivot;
				lastComPosition = pivot + delta * toLastComPosition;

				for(Footstep f : footsteps)
				{
					f.rotation = delta * f.rotation;
					f.stepFromRot = delta * f.stepFromRot;
					f.stepToRot = delta * f.stepToRot;
					f.stepToRootRot = delta * f.stepToRootRot;

					Vector3 toF = f.position - pivot;
					f.position = pivot + delta * toF;

					Vector3 toStepFrom = f.stepFrom - pivot;
					f.stepFrom = pivot + delta * toStepFrom;

					Vector3 toStepTo = f.stepTo - pivot;
					f.stepTo = pivot + delta * toStepTo;
				}
			}

			void AddDeltaPosition(Vector3 delta)
			{
				lastComPosition += delta;

				for(Footstep f : footsteps)
				{
					f.position += delta;
					f.stepFrom += delta;
					f.stepTo += delta;
				}
			}

			void Solve(VirtualBone* rootBone, Spine* spine, Leg* leftLeg, Leg* rightLeg, Arm* leftArm, Arm* rightArm, int supportLegIndex, Vector3& leftFootPosition, Vector3& rightFootPosition, Quaternion& leftFootRotation, Quaternion& rightFootRotation, float& leftFootOffset, float& rightFootOffset, float& leftHeelOffset, float& rightHeelOffset, float deltaTime)
			{
				if (weight <= 0.0f)
				{
					leftFootPosition  = Vector3::zero;
					rightFootPosition = Vector3::zero;
					leftFootRotation  = Quaternion::identity;
					rightFootRotation = Quaternion::identity;
					leftFootOffset = 0.0f;
					rightFootOffset = 0.0f;
					leftHeelOffset = 0.0f;
					rightHeelOffset = 0.0f;
					return;
				}

				Vector3 rootUp = rootBone->solverRotation * Vector3::up;

				Vector3 leftThighPosition = spine->pelvis()->solverPosition + spine->pelvis()->solverRotation  * leftLeg->GetThighRelativeToPelvis();
				Vector3 rightThighPosition = spine->pelvis()->solverPosition + spine->pelvis()->solverRotation * rightLeg->GetThighRelativeToPelvis();

				footsteps[0].characterSpaceOffset = Vector3::left  * footDistance;
				footsteps[1].characterSpaceOffset = Vector3::right * footDistance;

				Vector3 forward = spine->faceDirection;
				Vector3 forwardY = V3Tools::ExtractVertical(forward, rootUp, 1.0f);
				forward -= forwardY;
				Quaternion forwardRotation = Quaternion::LookRotation(forward, rootUp);
				if (spine->rootHeadingOffset != 0.0f) forwardRotation = Quaternion::AngleAxis(spine->rootHeadingOffset * Utils::DEG2RAD, rootUp) * forwardRotation;

				//centerOfMass = Vector3.Lerp(spine.pelvis.solverPosition, spine.head.solverPosition, 0.25f) + rootBone.solverRotation * offset;

				float pelvisMass = 1.0f;
				float headMass = 1.0f;
				float armMass = 0.2f;
				float totalMass = pelvisMass + headMass + 2.0f * armMass;

				centerOfMass = Vector3::zero;
				centerOfMass += spine->pelvis()->solverPosition * pelvisMass;
				centerOfMass += spine->head()->solverPosition * headMass;
				centerOfMass += leftArm->GetPosition() * armMass;
				centerOfMass += rightArm->GetPosition() * armMass;
				centerOfMass = centerOfMass / totalMass;

				centerOfMass += rootBone->solverRotation * offset;

				comVelocity = deltaTime > 0.0f ? (centerOfMass - lastComPosition) / deltaTime : Vector3::zero;
				lastComPosition = centerOfMass;
				comVelocity = Vector3::ClampMagnitude(comVelocity, maxVelocity) * velocityFactor;
				Vector3 centerOfMassV = centerOfMass + comVelocity;

				Vector3 pelvisPositionGroundLevel = V3Tools::PointToPlane(spine->pelvis()->solverPosition, rootBone->solverPosition, rootUp);
				Vector3 centerOfMassVGroundLevel  = V3Tools::PointToPlane(centerOfMassV, rootBone->solverPosition, rootUp);

				Vector3 centerOfPressure = Vector3::Lerp(footsteps[0].position, footsteps[1].position, 0.5f);

				Vector3 comDir = centerOfMassV - centerOfPressure;
				float comAngle = Vector3::AngleDegree(comDir, rootBone->solverRotation * Vector3::up) * comAngleMlp;

				// Set support leg
				for (int i = 0; i < footsteps.size(); i++)
				{
					footsteps[i].isSupportLeg = supportLegIndex == i;
				}

				// Update stepTo while stepping
				for (int i = 0; i < footsteps.size(); i++)
				{

					if (footsteps[i].isStepping())
					{
						Vector3 stepTo = centerOfMassVGroundLevel + rootBone->solverRotation * footsteps[i].characterSpaceOffset;

						if (!StepBlocked(footsteps[i].stepFrom, stepTo, rootBone->solverPosition))
						{
							footsteps[i].UpdateStepping(stepTo, forwardRotation, 10.0f, deltaTime);
						}
					}
					else
					{
						footsteps[i].UpdateStanding(forwardRotation, relaxLegTwistMinAngle, relaxLegTwistSpeed, deltaTime);
					}
				}

				// Triggering new footsteps
				if (CanStep())
				{
					int stepLegIndex = -1;
					float bestValue = -std::numeric_limits<float>::infinity();

					for (int i = 0; i < footsteps.size(); i++)
					{
						if (!footsteps[i].isStepping())
						{
							Vector3 stepTo = centerOfMassVGroundLevel + rootBone->solverRotation * footsteps[i].characterSpaceOffset;

							float legLength = i == 0 ? leftLeg->GetMag() : rightLeg->GetMag();
							Vector3 thighPos = i == 0 ? leftThighPosition : rightThighPosition;

							float thighDistance = Vector3::Distance(footsteps[i].position, thighPos);

							bool lengthStep = false;
							if (thighDistance >= legLength * maxLegStretch)
							{// * 0.95f) {
								stepTo = pelvisPositionGroundLevel + rootBone->solverRotation * footsteps[i].characterSpaceOffset;
								lengthStep = true;
							}

							bool collision = false;
							for (int n = 0; n < footsteps.size(); n++)
							{
								if (n != i && !lengthStep)
								{
									if (Vector3::Distance(footsteps[i].position, footsteps[n].position) < 0.25f && (footsteps[i].position - stepTo).lengthSquared() < (footsteps[n].position - stepTo).lengthSquared())
									{
									}
									else collision = GetLineSphereCollision(footsteps[i].position, stepTo, footsteps[n].position, 0.25f);
									if (collision) break;
								}
							}

							float angle = Quaternion::Angle(forwardRotation, footsteps[i].stepToRootRot);

							if (!collision || angle > angleThreshold)
							{
								float stepDistance = Vector3::Distance(footsteps[i].position, stepTo);
								float sT = Utils::Lerp(stepThreshold, stepThreshold * 0.1f, comAngle * 0.015f);
								if (lengthStep) sT *= 0.5f;
								if (i == 0) sT *= 0.9f;

								if (!StepBlocked(footsteps[i].position, stepTo, rootBone->solverPosition))
								{
									if (stepDistance > sT || angle > angleThreshold)
									{
										float value = 0.0f;

										value -= stepDistance;

										if (value > bestValue)
										{
											stepLegIndex = i;
											bestValue = value;
										}
									}
								}
							}
						}

					}

					if (stepLegIndex != -1)
					{
						Vector3 stepTo = centerOfMassVGroundLevel + rootBone->solverRotation * footsteps[stepLegIndex].characterSpaceOffset;
						footsteps[stepLegIndex].stepSpeed = Utils::RandomFloat(stepSpeed, stepSpeed * 1.5f);
						footsteps[stepLegIndex].StepTo(stepTo, forwardRotation, stepThreshold);
					}
				}

				footsteps[0].Update(stepInterpolation, onLeftFootstep, deltaTime);
				footsteps[1].Update(stepInterpolation, onRightFootstep, deltaTime);

				leftFootPosition = footsteps[0].position;
				rightFootPosition = footsteps[1].position;

				leftFootPosition  = V3Tools::PointToPlane(leftFootPosition, leftLeg->lastBone()->readPosition, rootUp);
				rightFootPosition = V3Tools::PointToPlane(rightFootPosition, rightLeg->lastBone()->readPosition, rootUp);

				leftFootOffset = stepHeight->Evaluate(footsteps[0].GetStepProgress());
				rightFootOffset = stepHeight->Evaluate(footsteps[1].GetStepProgress());

				leftHeelOffset = heelHeight->Evaluate(footsteps[0].GetStepProgress());
				rightHeelOffset = heelHeight->Evaluate(footsteps[1].GetStepProgress());

				leftFootRotation = footsteps[0].rotation;
				rightFootRotation = footsteps[1].rotation;
			}

			Vector3 leftFootstepPosition()
			{
				return footsteps[0].position;
			}

			Vector3 rightFootstepPosition()
			{
				return footsteps[1].position;
			}

			Quaternion leftFootstepRotation()
			{
				return footsteps[0].rotation;
			}

			Quaternion rightFootstepRotation()
			{
				return footsteps[1].rotation;
			}

		private:
			// NOTE: We have no blocking mask, therefore this function always is fals
			bool StepBlocked(Vector3 fromPosition, Vector3 toPosition, Vector3 rootPosition)
			{
				return false;

				/*if (blockingLayers == -1 || !blockingEnabled) return false;

				Vector3 origin = fromPosition;
				origin.y = rootPosition.y + raycastHeight + raycastRadius;

				Vector3 direction = toPosition - origin;
				direction.y = 0.0f;

				RaycastHit hit;

				if (raycastRadius <= 0.0f)
				{
					return Physics.Raycast(origin, direction, out hit, direction.magnitude, blockingLayers);
				}
				else
				{
					return Physics.SphereCast(origin, raycastRadius, direction, out hit, direction.magnitude, blockingLayers);
				}*/
			}

			bool CanStep()
			{
				for(Footstep f : footsteps) if (f.isStepping() && f.GetStepProgress() < 0.8f) return false;

				return true;
			}

			static bool GetLineSphereCollision(Vector3 lineStart, Vector3 lineEnd, Vector3 sphereCenter, float sphereRadius)
			{
				Vector3 line = lineEnd - lineStart;
				Vector3 toSphere = sphereCenter - lineStart;
				float distToSphereCenter = toSphere.length();
				float d = distToSphereCenter - sphereRadius;

				if (d > line.length()) return false;

				Quaternion q = Quaternion::LookRotation(line, toSphere);

				Vector3 toSphereRotated = Quaternion::Inverse(q) * toSphere;

				if (toSphereRotated.z < 0.0f)
				{
					return d < 0.0f;
				}

				return toSphereRotated.y - sphereRadius < 0.0f;
			}
		};

		#pragma endregion

		/// <summary>
		/// Hybrid %IK solver designed for mapping a character to a VR headset and 2 hand controllers 
		/// </summary>
		#pragma region Port of the partial class IKSolverVR.cs

		#pragma region Wrapper

		public:
			~IKSolverVR()
			{
				delete leftArm;
				delete rightArm;
				delete spine;
				delete leftLeg;
				delete rightLeg;
				delete locomotion;
				delete rootBone;
			}

			/// <summary>
			/// Sets this VRIK up to the specified bone references.
			/// </summary>
			void SetToReferences(VRIK::References& references);

			/// <summary>
			/// Guesses the hand bones orientations ('Wrist To Palm Axis' and "Palm To Thumb Axis" of the arms) based on the provided references. if onlyIfZero is true, will only guess an orientation axis if it is Vector3.zero.
			/// </summary>
			void GuessHandOrientations(VRIK::References& references, bool onlyIfZero);

			/// <summary>
			/// Set default values for the animation curves if they have no keys.
			/// </summary>
			void DefaultAnimationCurves()
			{
				if (locomotion->stepHeight == nullptr) locomotion->stepHeight = new FloatCurve();
				if (locomotion->heelHeight == nullptr) locomotion->heelHeight = new FloatCurve();

				if (locomotion->stepHeight->keys.size() == 0)
				{
					locomotion->stepHeight->keys = GetSineKeyframes(0.03f);
				}

				if (locomotion->heelHeight->keys.size() == 0)
				{
					locomotion->heelHeight->keys = GetSineKeyframes(0.03f);
				}
			}

			/// <summary>
			/// Adds position offset to a body part. Position offsets add to the targets in VRIK.
			/// </summary>
			void AddPositionOffset(PositionOffset positionOffset, Vector3 value)
			{
				switch (positionOffset)
				{
				case PositionOffset::Pelvis: spine->pelvisPositionOffset += value; return;
				case PositionOffset::Chest: spine->chestPositionOffset += value; return;
				case PositionOffset::Head: spine->headPositionOffset += value; return;
				case PositionOffset::LeftHand: leftArm->handPositionOffset += value; return;
				case PositionOffset::RightHand: rightArm->handPositionOffset += value; return;
				case PositionOffset::LeftFoot: leftLeg->footPositionOffset += value; return;
				case PositionOffset::RightFoot: rightLeg->footPositionOffset += value; return;
				case PositionOffset::LeftHeel: leftLeg->heelPositionOffset += value; return;
				case PositionOffset::RightHeel: rightLeg->heelPositionOffset += value; return;
				}
			}

			/// <summary>
			/// Adds rotation offset to a body part. Rotation offsets add to the targets in VRIK
			/// </summary>
			void AddRotationOffset(RotationOffset rotationOffset, Vector3 value)
			{
				AddRotationOffset(rotationOffset, Quaternion::Euler(value));
			}

			/// <summary>
			/// Adds rotation offset to a body part. Rotation offsets add to the targets in VRIK
			/// </summary>
			void AddRotationOffset(RotationOffset rotationOffset, Quaternion value)
			{
				switch (rotationOffset)
				{
					case RotationOffset::Pelvis: spine->pelvisRotationOffset = value * spine->pelvisRotationOffset; return;
					case RotationOffset::Chest: spine->chestRotationOffset = value * spine->chestRotationOffset; return;
					case RotationOffset::Head: spine->headRotationOffset = value * spine->headRotationOffset; return;
				}
			}

			/// <summary>
			/// Call this in each Update if your avatar is standing on a moving platform
			/// </summary>
			void AddPlatformMotion(Vector3 deltaPosition, Quaternion deltaRotation, Vector3 platformPivot)
			{
				locomotion->AddDeltaPosition(deltaPosition);
				raycastOriginPelvis += deltaPosition;

				locomotion->AddDeltaRotation(deltaRotation, platformPivot);
				spine->faceDirection = deltaRotation * spine->faceDirection;
			}

			/// <summary>
			/// Resets all tweens, blendings and lerps. Call this after you have teleported the character.
			/// </summary>
			void Reset()
			{
				if (!initiated) return;

				UpdateSolverTransforms();
				Read(readPositions, readRotations, hasChest, hasNeck, hasShoulders, hasToes, hasLegs);

				spine->faceDirection = rootBone->readRotation * Vector3::forward;

				if (hasLegs)
				{
					locomotion->Reset(readPositions, readRotations);
					raycastOriginPelvis = spine->pelvis()->readPosition;
				}
			}

			void StoreDefaultLocalState() override
			{
				for (int i = 1; i < solverTransforms.size(); i++)
				{
					if (solverTransforms[i] != nullptr)
					{
						defaultLocalPositions[i - 1] = solverTransforms[i]->LocalPosition();
						defaultLocalRotations[i - 1] = solverTransforms[i]->LocalRotation();
					}
				}
			}

			void FixTransforms() override
			{
				if (!initiated) return;
				if (LOD >= 2) return;

				for (int i = 1; i < solverTransforms.size(); i++)
				{
					if (solverTransforms[i] != nullptr)
					{
						bool isPelvis = i == 1;

						bool isArmStretchable = i == 8 || i == 9 || i == 12 || i == 13;
						bool isLegStretchable = (i >= 15 && i <= 17) || (i >= 19 && i <= 21);

						if (isPelvis || isArmStretchable || isLegStretchable)
						{
							solverTransforms[i]->LocalPosition(defaultLocalPositions[i - 1]);
						}
						solverTransforms[i]->LocalRotation(defaultLocalRotations[i - 1]);
					}
				}
			}

			std::vector<IKSolver::Point> GetPoints() override
			{
				qDebug() << "GetPoints() is not applicable to IKSolverVR.";
				return std::vector<IKSolver::Point>();
			}

			IKSolver::Point GetPoint(Transform transform) override
			{
				qDebug() << "GetPoint is not applicable to IKSolverVR.";
				return IKSolver::Point();
			}

			bool IsValid(std::string& message) override
			{
				if (solverTransforms.size() == 0)
				{
					message = "Trying to initiate IKSolverVR with invalid bone references.";
					return false;
				}

				if (leftArm->wristToPalmAxis == Vector3::zero)
				{
					message = "Left arm 'Wrist To Palm Axis' needs to be set in VRIK. Please select the hand bone, set it to the axis that points from the wrist towards the palm. If the arrow points away from the palm, axis must be negative.";
					return false;
				}

				if (rightArm->wristToPalmAxis == Vector3::zero)
				{
					message = "Right arm 'Wrist To Palm Axis' needs to be set in VRIK. Please select the hand bone, set it to the axis that points from the wrist towards the palm. If the arrow points away from the palm, axis must be negative.";
					return false;
				}

				if (leftArm->palmToThumbAxis == Vector3::zero)
				{
					message = "Left arm 'Palm To Thumb Axis' needs to be set in VRIK. Please select the hand bone, set it to the axis that points from the palm towards the thumb. If the arrow points away from the thumb, axis must be negative.";
					return false;
				}

				if (rightArm->palmToThumbAxis == Vector3::zero)
				{
					message = "Right arm 'Palm To Thumb Axis' needs to be set in VRIK. Please select the hand bone, set it to the axis that points from the palm towards the thumb. If the arrow points away from the thumb, axis must be negative.";
					return false;
				}

				return true;
			}

		private:
			std::vector<Transform*> solverTransforms = std::vector<Transform*>();
			bool hasChest = false, hasNeck = false, hasShoulders = false, hasToes = false, hasLegs = false;
			std::vector<Vector3> readPositions = std::vector<Vector3>();
			std::vector<Quaternion> readRotations = std::vector<Quaternion>();
			std::vector<Vector3> solvedPositions = std::vector<Vector3>(22); // NOTE: Was an array
			std::vector<Quaternion> solvedRotations = std::vector<Quaternion>(22); // NOTE: Was an array
			// Vector3 defaultPelvisLocalPosition;
			std::vector<Quaternion> defaultLocalRotations = std::vector<Quaternion>(21); // NOTE: Was an array
			std::vector<Vector3> defaultLocalPositions = std::vector<Vector3>(21); // NOTE: Was an array

			Vector3 GetNormal(std::vector<Transform*>& transforms)
			{
				Vector3 normal = Vector3::zero;

				Vector3 centroid = Vector3::zero;
				for (int i = 0; i < transforms.size(); i++)
				{
					centroid += transforms[i]->Position();
				}
				centroid = centroid / transforms.size();

				for (int i = 0; i < transforms.size() - 1; i++)
				{
					normal += Vector3::Cross(transforms[i]->Position() - centroid, transforms[i + 1]->Position() - centroid).normalized();
				}

				return normal;
			}

			Vector3 GuessWristToPalmAxis(const Transform* hand, const Transform* forearm)
			{
				Vector3 toForearm = forearm->Position() - hand->Position();
				Vector3 axis = AxisTools::ToVector3(AxisTools::GetAxisToDirection(hand, toForearm));
				if (Vector3::Dot(toForearm, hand->Rotation() * axis) > 0.0f) axis = -axis;
				return axis;
			}

			Vector3 GuessPalmToThumbAxis(const Transform* hand, const Transform* forearm)
			{
				if (hand->ChildCount() == 0)
				{
					qDebug() << "Warning -> IKSolverVR -> Hand " << hand->Name().c_str() << " does not have any fingers, VRIK can not guess the hand bone's orientation. Please assign 'Wrist To Palm Axis' and 'Palm To Thumb Axis' manually for both arms in VRIK settings.";
					return Vector3::zero;
				}

				float closestSqrMag = std::numeric_limits<float>::infinity();
				int thumbIndex = 0;

				for (int i = 0; i < hand->ChildCount(); i++)
				{
					float sqrMag = Vector3::SqrMagnitude(hand->GetChild(i)->Position() - hand->Position());
					if (sqrMag < closestSqrMag)
					{
						closestSqrMag = sqrMag;
						thumbIndex = i;
					}
				}

				Vector3 handNormal = Vector3::Cross(hand->Position() - forearm->Position(), hand->GetChild(thumbIndex)->Position() - hand->Position());
				Vector3 toThumb = Vector3::Cross(handNormal, hand->Position() - forearm->Position());
				Vector3 axis = AxisTools::ToVector3(AxisTools::GetAxisToDirection(hand, toThumb));
				if (Vector3::Dot(toThumb, hand->Rotation() * axis) < 0.0f) axis = -axis;
				return axis;
			}

			static std::vector<Keyframe> GetSineKeyframes(float mag)
			{
				std::vector<Keyframe> keys = std::vector<Keyframe>(3);
				keys[0].time = 0.0f;
				keys[0].value = 0.0f;
				keys[1].time = 0.5f;
				keys[1].value = mag;
				keys[2].time = 1.0f;
				keys[2].value = 0.0f;
				return keys;
			}

			void UpdateSolverTransforms()
			{
				for (int i = 0; i < solverTransforms.size(); i++)
				{
					if (solverTransforms[i] != nullptr)
					{
						readPositions[i] = solverTransforms[i]->Position();
						readRotations[i] = solverTransforms[i]->Rotation();
					}
				}
			}

		protected:

			void OnInitiate() override
			{
				UpdateSolverTransforms();
				Read(readPositions, readRotations, hasChest, hasNeck, hasShoulders, hasToes, hasLegs);
			}

			void OnUpdate(float deltaTime) override
			{
				if (IKPositionWeight > 0.0f)
				{
					if (LOD < 2)
					{
						bool read = false;

						if (lastLOD != LOD)
						{
							if (lastLOD == 2)
							{
								spine->faceDirection = rootBone->readRotation * Vector3::forward;

								if (hasLegs)
								{
									// Teleport to the current position/rotation if resuming from culled LOD with locomotion enabled
									if (locomotion->weight > 0.0f)
									{
										root->Position(Vector3(spine->headTarget->Position().x, root->Position().y, spine->headTarget->Position().z));
										Vector3 forward = spine->faceDirection;
										forward.y = 0.0f;
										root->Rotation(Quaternion::LookRotation(forward, root->Up()));

										UpdateSolverTransforms();
										Read(readPositions, readRotations, hasChest, hasNeck, hasShoulders, hasToes, hasLegs);
										read = true;

										locomotion->Reset(readPositions, readRotations);
									}

									raycastOriginPelvis = spine->pelvis()->readPosition;
								}
							}
						}

						if (!read)
						{
							UpdateSolverTransforms();
							Read(readPositions, readRotations, hasChest, hasNeck, hasShoulders, hasToes, hasLegs);
						}

						Solve(deltaTime);
						Write();

						WriteTransforms();
					}
					else
					{
						// Culled
						if (locomotion->weight > 0.0f)
						{
							root->Position(Vector3(spine->headTarget->Position().x, root->Position().y, spine->headTarget->Position().z));
							Vector3 forward = spine->headTarget->Rotation() * spine->GetAnchorRelativeToHead() * Vector3::forward;
							forward.y = 0.0f;
							root->Rotation(Quaternion::LookRotation(forward, root->Up()));
						}
					}
				}

				lastLOD = LOD;
			}

		private:
			void WriteTransforms()
			{
				for (int i = 0; i < solverTransforms.size(); i++)
				{
					if (solverTransforms[i] != nullptr)
					{
						bool isRootOrPelvis = i < 2;
						bool isArmStretchable = i == 8 || i == 9 || i == 12 || i == 13;
						bool isLegStretchable = (i >= 15 && i <= 17) || (i >= 19 && i <= 21);
						if (LOD > 0)
						{
							isArmStretchable = false;
							isLegStretchable = false;
						}

						if (isRootOrPelvis)
						{
							Vector3 solverTransformPosition = solverTransforms[i]->Position();
							Vector3 solvedPosition = GetPosition(i);
							Vector3 lerpedPosition = V3Tools::Lerp(solverTransformPosition, solvedPosition, IKPositionWeight);
							solverTransforms[i]->Position(lerpedPosition);
						}

						if (isArmStretchable || isLegStretchable)
						{
							if (IKPositionWeight < 1.0f)
							{
								Vector3 localPosition = solverTransforms[i]->LocalPosition();
								solverTransforms[i]->Position(V3Tools::Lerp(solverTransforms[i]->Position(), GetPosition(i), IKPositionWeight));
								solverTransforms[i]->LocalPosition(Vector3::Project(solverTransforms[i]->LocalPosition(), localPosition));
							}
							else
							{
								solverTransforms[i]->Position(V3Tools::Lerp(solverTransforms[i]->Position(), GetPosition(i), IKPositionWeight));
							}
						}
						
						Quaternion solverTransformRotation = solverTransforms[i]->Rotation();
						Quaternion solvedRotation = GetRotation(i);
						Quaternion lerpedRotation = QuaTools::Lerp(solverTransformRotation, solvedRotation, IKPositionWeight);

						solverTransforms[i]->Rotation(lerpedRotation);
					}
				}
			}

			#pragma endregion Wrapper

			#pragma region Generic API

		private:
			Vector3 rootV = Vector3::zero;
			Vector3 rootVelocity = Vector3::zero;
			Vector3 bodyOffset = Vector3::zero;
			int supportLegIndex = 0;

			void Read(std::vector<Vector3> positions, std::vector<Quaternion> rotations, bool hasChest, bool hasNeck, bool hasShoulders, bool hasToes, bool hasLegs)
			{
				if (rootBone == nullptr)
				{
					rootBone = new VirtualBone(positions[0], rotations[0]);
				}
				else
				{
					rootBone->Read(positions[0], rotations[0]);
				}

				spine->Read(positions, rotations, hasChest, hasNeck, hasShoulders, hasToes, hasLegs, 0, 1);
				leftArm->Read(positions, rotations, hasChest, hasNeck, hasShoulders, hasToes, hasLegs, hasChest ? 3 : 2, 6);
				rightArm->Read(positions, rotations, hasChest, hasNeck, hasShoulders, hasToes, hasLegs, hasChest ? 3 : 2, 10);

				if (hasLegs)
				{
					leftLeg->Read(positions, rotations, hasChest, hasNeck, hasShoulders, hasToes, hasLegs, 1, 14);
					rightLeg->Read(positions, rotations, hasChest, hasNeck, hasShoulders, hasToes, hasLegs, 1, 18);
				}

				for (int i = 0; i < rotations.size(); i++)
				{
					this->solvedPositions[i] = positions[i];
					this->solvedRotations[i] = rotations[i];
				}

				if (!initiated)
				{
					if (hasLegs) legs = { leftLeg, rightLeg };
					arms = { leftArm, rightArm };

					if (hasLegs) locomotion->Initiate(positions, rotations, hasToes);
					raycastOriginPelvis = spine->pelvis()->readPosition;
					spine->faceDirection = readRotations[0] * Vector3::forward;
				}
			}

			int lastLOD = 0;

			void Solve(float deltaTime)
			{
				spine->SetLOD(LOD);
				for(Arm* arm : arms) arm->SetLOD(LOD);
				if (hasLegs) for(Leg* leg : legs) leg->SetLOD(LOD);

				// Pre-Solving
				spine->PreSolve();
				for(Arm* arm : arms) arm->PreSolve();
				if (hasLegs) for(Leg* leg : legs) leg->PreSolve();

				// Applying spine and arm offsets
				for(Arm* arm : arms) arm->ApplyOffsets();
				spine->ApplyOffsets();

				// Spine
				spine->Solve(rootBone, legs, arms);

				if (hasLegs && spine->pelvisPositionWeight > 0.0f && plantFeet)
				{
					qDebug() << "IKSolverVR -> Warning -> If VRIK 'Pelvis Position Weight' is > 0, 'Plant Feet' should be disabled to improve performance and stability.";
				}

				// Locomotion
				if (hasLegs && locomotion->weight > 0.0f)
				{
					Vector3 leftFootPosition = Vector3::zero;
					Vector3 rightFootPosition = Vector3::zero;
					Quaternion leftFootRotation = Quaternion::identity;
					Quaternion rightFootRotation = Quaternion::identity;
					float leftFootOffset = 0.0f;
					float rightFootOffset = 0.0f;
					float leftHeelOffset = 0.0f;
					float rightHeelOffset = 0.0f;

					locomotion->Solve(rootBone, spine, leftLeg, rightLeg, leftArm, rightArm, supportLegIndex, leftFootPosition, rightFootPosition, leftFootRotation, rightFootRotation, leftFootOffset, rightFootOffset, leftHeelOffset, rightHeelOffset, deltaTime);

					leftFootPosition += root->Up() * leftFootOffset;
					rightFootPosition += root->Up() * rightFootOffset;

					leftLeg->footPositionOffset += (leftFootPosition - leftLeg->lastBone()->solverPosition) * IKPositionWeight * (1.0f - leftLeg->positionWeight) * locomotion->weight;
					rightLeg->footPositionOffset += (rightFootPosition - rightLeg->lastBone()->solverPosition) * IKPositionWeight * (1.0f - rightLeg->positionWeight) * locomotion->weight;

					leftLeg->heelPositionOffset += root->Up() * leftHeelOffset * locomotion->weight;
					rightLeg->heelPositionOffset += root->Up() * rightHeelOffset * locomotion->weight;

					Quaternion rotationOffsetLeft = QuaTools::FromToRotation(leftLeg->lastBone()->solverRotation, leftFootRotation);
					Quaternion rotationOffsetRight = QuaTools::FromToRotation(rightLeg->lastBone()->solverRotation, rightFootRotation);

					rotationOffsetLeft = Quaternion::Lerp(Quaternion::identity, rotationOffsetLeft, IKPositionWeight * (1.0f - leftLeg->rotationWeight) * locomotion->weight);
					rotationOffsetRight = Quaternion::Lerp(Quaternion::identity, rotationOffsetRight, IKPositionWeight * (1.0f - rightLeg->rotationWeight) * locomotion->weight);

					leftLeg->footRotationOffset = rotationOffsetLeft * leftLeg->footRotationOffset;
					rightLeg->footRotationOffset = rotationOffsetRight * rightLeg->footRotationOffset;

					Vector3 footPositionC = Vector3::Lerp(leftLeg->GetPosition() + leftLeg->footPositionOffset, rightLeg->GetPosition() + rightLeg->footPositionOffset, 0.5f);
					footPositionC = V3Tools::PointToPlane(footPositionC, rootBone->solverPosition, root->Up());

					Vector3 p = rootBone->solverPosition + rootVelocity * deltaTime * 2.0f * locomotion->weight;
					p = Vector3::Lerp(p, footPositionC, deltaTime * locomotion->rootSpeed * locomotion->weight);
					rootBone->solverPosition = p;

					rootVelocity += (footPositionC - rootBone->solverPosition) * deltaTime * 10.0f;
					Vector3 rootVelocityV = V3Tools::ExtractVertical(rootVelocity, root->Up(), 1.0f);
					rootVelocity -= rootVelocityV;

					float bodyYOffset = leftFootOffset + rightFootOffset;
					bodyOffset = Vector3::Lerp(bodyOffset, root->Up() * bodyYOffset, deltaTime * 3.0f);
					bodyOffset = Vector3::Lerp(Vector3::zero, bodyOffset, locomotion->weight);
				}

				// Legs
				if (hasLegs)
				{
					for(Leg* leg : legs)
					{
						leg->ApplyOffsets();
					}
					if (!plantFeet || LOD > 0)
					{
						spine->InverseTranslateToHead(legs, false, false, bodyOffset, 1.0f);

						for(Leg* leg : legs) leg->TranslateRoot(spine->pelvis()->solverPosition, spine->pelvis()->solverRotation);
						for(Leg* leg : legs)
						{
							leg->Solve(true);
						}
					}
					else
					{
						for (int i = 0; i < 2; i++)
						{
							spine->InverseTranslateToHead(legs, true, true, bodyOffset, 1.0f);

							for(Leg* leg : legs) leg->TranslateRoot(spine->pelvis()->solverPosition, spine->pelvis()->solverRotation);
							for(Leg* leg : legs)
							{
								leg->Solve(i == 0);
							}
						}
					}
				}
				else
				{
					spine->InverseTranslateToHead(legs, false, false, bodyOffset, 1.0f);
				}

				// Arms
				for (int i = 0; i < arms.size(); i++)
				{
					arms[i]->TranslateRoot(spine->chest()->solverPosition, spine->chest()->solverRotation);
				}

				for (int i = 0; i < arms.size(); i++)
				{
					arms[i]->Solve(i == 0);
				}

				// Reset offsets
				spine->ResetOffsets();
				if (hasLegs) for(Leg* leg : legs) leg->ResetOffsets();
				for(Arm* arm : arms) arm->ResetOffsets();

				if (hasLegs)
				{
					spine->pelvisPositionOffset += GetPelvisOffset(deltaTime);
					spine->chestPositionOffset += spine->pelvisPositionOffset;
					//spine.headPositionOffset += spine.pelvisPositionOffset;
				}

				Write();

				// Find the support leg
				if (hasLegs)
				{
					supportLegIndex = -1;
					float shortestMag = std::numeric_limits<float>::infinity();
					for (int i = 0; i < sizeof(legs) / sizeof(Leg); i++)
					{
						float mag = Vector3::SqrMagnitude(legs[i]->lastBone()->solverPosition - legs[i]->bones[0]->solverPosition);
						if (mag < shortestMag)
						{
							supportLegIndex = i;
							shortestMag = mag;
						}
					}
				}
			};

			Vector3 GetPosition(int index)
			{
				return solvedPositions[index];
			};

			Quaternion GetRotation(int index)
			{
				return solvedRotations[index];
			};

			#pragma endregion Generic API

		public:
			//[Tooltip("LOD 0: Full quality solving. LOD 1: Shoulder solving, stretching plant feet disabled, spine solving quality reduced. This provides about 30% of performance gain. LOD 2: Culled, but updating root position and rotation if locomotion is enabled.")]
			/// <summary>
			/// LOD 0: Full quality solving. LOD 1: Shoulder solving, stretching plant feet disabled, spine solving quality reduced. This provides about 30% of performance gain. LOD 2: Culled, but updating root position and rotation if locomotion is enabled.
			/// </summary>
			//[Range(0, 2)] 
			int LOD = 0;

			//[Tooltip("If true, will keep the toes planted even if head target is out of reach, so this can cause the camera to exit the head if it is too high for the model to reach. Enabling this increases the cost of the solver as the legs will have to be solved multiple times.")]
			/// <summary>
			/// If true, will keep the toes planted even if head target is out of reach, so this can cause the camera to exit the head if it is too high for the model to reach. Enabling this increases the cost of the solver as the legs will have to be solved multiple times.
			/// </summary>
			bool plantFeet = true;

			/// <summary>
			/// Gets the root bone.
			/// </summary>
			//[HideInInspector] 
			VirtualBone* GetRootBone(){ return rootBone; }

			//[Tooltip("The spine solver.")]
			/// <summary>
			/// The spine solver.
			/// </summary>
			Spine* spine = new Spine();

			//[Tooltip("The left arm solver.")]
			/// <summary>
			/// The left arm solver.
			/// </summary>
			Arm* leftArm = new Arm();

			//[Tooltip("The right arm solver.")]
			/// <summary>
			/// The right arm solver.
			/// </summary>
			Arm* rightArm = new Arm();

			//[Tooltip("The left leg solver.")]
			/// <summary>
			/// The left leg solver.
			/// </summary>
			Leg* leftLeg = new Leg();


			//[Tooltip("The right leg solver.")]
			/// <summary>
			/// The right leg solver.
			/// </summary>
			Leg* rightLeg = new Leg();

			//[Tooltip("Procedural leg shuffling for stationary VR games. Not designed for roomscale and thumbstick locomotion. For those it would be better to use a strafing locomotion blend tree to make the character follow the horizontal direction towards the HMD by root motion or script.")]
			/// <summary>
			/// Procedural leg shuffling for stationary VR games. Not designed for roomscale and thumbstick locomotion. For those it would be better to use a strafing locomotion blend tree to make the character follow the horizontal direction towards the HMD by root motion or script.
			/// </summary>
			Locomotion* locomotion = new Locomotion();

		private:
			VirtualBone* rootBone = nullptr;
			std::vector<Leg*> legs = std::vector<Leg*>(2); // NOTE: Was an array
			std::vector<Arm*> arms = std::vector<Arm*>(2); // NOTE: Was an array
			Vector3 headPosition = Vector3::zero;
			Vector3 headDeltaPosition = Vector3::zero;
			Vector3 raycastOriginPelvis = Vector3::zero;
			Vector3 lastOffset = Vector3::zero;
			Vector3 debugPos1 = Vector3::zero;
			Vector3 debugPos2 = Vector3::zero;
			Vector3 debugPos3 = Vector3::zero;
			Vector3 debugPos4 = Vector3::zero;

			void Write()
			{
				solvedPositions[0] = rootBone->solverPosition;
				solvedRotations[0] = rootBone->solverRotation;
				spine->Write(solvedPositions, solvedRotations);

				if (hasLegs)
				{
					for(Leg* leg : legs) leg->Write(solvedPositions, solvedRotations);
				}
				for(Arm* arm : arms) arm->Write(solvedPositions, solvedRotations);
			}

			Vector3 GetPelvisOffset(float deltaTime)
			{
				if (locomotion->weight <= 0.0f) return Vector3::zero;
				//if (locomotion.blockingLayers == -1) return Vector3::zero;

				// Origin to pelvis transform position
				Vector3 sampledOrigin = raycastOriginPelvis;
				sampledOrigin.y = spine->pelvis()->solverPosition.y;
				Vector3 origin = spine->pelvis()->readPosition;
				origin.y = spine->pelvis()->solverPosition.y;
				Vector3 direction = origin - sampledOrigin;
				Physics::RaycastHit hit;

				//debugPos4 = sampledOrigin;

				if (locomotion->raycastRadius <= 0.0f)
				{
					if (Physics::Raycast(sampledOrigin, direction, hit, direction.length() * 1.1f))
					{
						origin = hit.point;
					}
				}
				else
				{
					if (Physics::SphereCast(sampledOrigin, locomotion->raycastRadius * 1.1f, direction, hit, direction.length()))
					{
						origin = sampledOrigin + direction.normalized() * hit.distance / 1.1f;
					}
				}

				Vector3 position = spine->pelvis()->solverPosition;
				direction = position - origin;

				//debugPos1 = origin;
				//debugPos2 = position;

				if (locomotion->raycastRadius <= 0.0f)
				{
					if (Physics::Raycast(origin, direction, hit, direction.length()))
					{
						position = hit.point;
					}

				}
				else
				{
					if (Physics::SphereCast(origin, locomotion->raycastRadius, direction, hit, direction.length()))
					{
						position = origin + direction.normalized() * hit.distance;
					}
				}

				lastOffset = Vector3::Lerp(lastOffset, Vector3::zero, deltaTime * 3.0f);
				position += Vector3::ClampMagnitude(lastOffset, 0.75f);
				position.y = spine->pelvis()->solverPosition.y;

				//debugPos3 = position;

				lastOffset = Vector3::Lerp(lastOffset, position - spine->pelvis()->solverPosition, deltaTime * 15.0f);
				return lastOffset;
			}

	#pragma endregion
	};
}