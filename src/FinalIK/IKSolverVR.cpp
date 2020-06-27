#include "IKSolverVR.h"

namespace RootMotion
{
	void IKSolverVR::SetToReferences(VRIK::References& references)
	{
		if (!references.isFilled())
		{
			qDebug() << "Invalid references, one or more Transforms are missing.";
			return;
		}

		solverTransforms = references.GetTransforms(); 

		hasChest = solverTransforms[3] != nullptr;
		hasNeck = solverTransforms[4] != nullptr;
		hasShoulders = solverTransforms[6] != nullptr && solverTransforms[10] != nullptr;
		hasToes = solverTransforms[17] != nullptr && solverTransforms[21] != nullptr;
		hasLegs = solverTransforms[14] != nullptr;

		readPositions = std::vector<Vector3>(solverTransforms.size());
		readRotations = std::vector<Quaternion>(solverTransforms.size());

		DefaultAnimationCurves();
		GuessHandOrientations(references, true);
	}

	void IKSolverVR::GuessHandOrientations(VRIK::References& references, bool onlyIfZero)
	{
		if (!references.isFilled())
		{
			qDebug() << "VRIK References are not filled in, can not guess hand orientations. Right-click on VRIK header and slect 'Guess Hand Orientations' when you have filled in the References.";
			return;
		}

		if (leftArm->wristToPalmAxis == Vector3::zero || !onlyIfZero)
		{
			leftArm->wristToPalmAxis = GuessWristToPalmAxis(references.leftHand, references.leftForearm);
		}

		if (leftArm->palmToThumbAxis == Vector3::zero || !onlyIfZero)
		{
			leftArm->palmToThumbAxis = GuessPalmToThumbAxis(references.leftHand, references.leftForearm);
		}

		if (rightArm->wristToPalmAxis == Vector3::zero || !onlyIfZero)
		{
			rightArm->wristToPalmAxis = GuessWristToPalmAxis(references.rightHand, references.rightForearm);
		}

		if (rightArm->palmToThumbAxis == Vector3::zero || !onlyIfZero)
		{
			rightArm->palmToThumbAxis = GuessPalmToThumbAxis(references.rightHand, references.rightForearm);
		}
	}

	IKSolverVR::VirtualBone::VirtualBone(Vector3 position, Quaternion rotation)
	{
		Read(position, rotation);
	}

	void IKSolverVR::VirtualBone::Read(Vector3 position, Quaternion rotation)
	{
		readPosition = position;
		readRotation = rotation;
		solverPosition = position;
		solverRotation = rotation;
	}

	void IKSolverVR::VirtualBone::SwingRotation(std::vector<VirtualBone*> bones, int index, Vector3 swingTarget, float weight)
	{
		if (weight <= 0.0f) return;

		Vector3 from = bones[index]->solverRotation * bones[index]->axis;
		Vector3 to = swingTarget - bones[index]->solverPosition;

		Quaternion r = Quaternion::FromToRotation(from, to);
		if (weight < 1.0f) r = Quaternion::Lerp(Quaternion::identity, r, weight);

		for (int i = index; i < bones.size(); i++)
		{
			bones[i]->solverRotation = r * bones[i]->solverRotation;
		}
	}

	float IKSolverVR::VirtualBone::PreSolve(std::vector<VirtualBone*> bones)
	{
		float length = 0;

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

	void IKSolverVR::VirtualBone::RotateAroundPoint(std::vector<VirtualBone*> bones, int index, Vector3 point, Quaternion rotation)
	{
		for (int i = index; i < bones.size(); i++)
		{
			if (bones[i] != nullptr)
			{
				Vector3 dir = bones[i]->solverPosition - point;
				bones[i]->solverPosition = point + rotation * dir;
				bones[i]->solverRotation = rotation * bones[i]->solverRotation;
			}
		}
	}

	void IKSolverVR::VirtualBone::RotateBy(std::vector<VirtualBone*> bones, int index, Quaternion rotation)
	{
		for (int i = index; i < bones.size(); i++)
		{
			if (bones[i] != nullptr)
			{
				Vector3 dir = bones[i]->solverPosition - bones[index]->solverPosition;
				bones[i]->solverPosition = bones[index]->solverPosition + rotation * dir;
				bones[i]->solverRotation = rotation * bones[i]->solverRotation;
			}
		}
	}

	void IKSolverVR::VirtualBone::RotateBy(std::vector<VirtualBone*> bones, Quaternion rotation)
	{
		for (int i = 0; i < bones.size(); i++)
		{
			if (bones[i] != nullptr)
			{
				if (i > 0)
				{
					Vector3 dir = bones[i]->solverPosition - bones[0]->solverPosition;
					bones[i]->solverPosition = bones[0]->solverPosition + rotation * dir;
				}

				bones[i]->solverRotation = rotation * bones[i]->solverRotation;
			}
		}
	}

	void IKSolverVR::VirtualBone::RotateTo(std::vector<VirtualBone*> bones, int index, Quaternion rotation)
	{
		Quaternion q = QuaTools::FromToRotation(bones[index]->solverRotation, rotation);

		RotateAroundPoint(bones, index, bones[index]->solverPosition, q);
	}

	void IKSolverVR::VirtualBone::SolveTrigonometric(std::vector<VirtualBone*>& bones, int first, int second, int third, Vector3 targetPosition, Vector3 bendNormal, float weight)
	{
		if (weight <= 0.0f) return;

		// Direction of the limb in solver
		targetPosition = Vector3::Lerp(bones[third]->solverPosition, targetPosition, weight);

		Vector3 dir = targetPosition - bones[first]->solverPosition;

		// Distance between the first and the last transform solver positions
		float sqrMag = dir.lengthSquared();
		if (sqrMag == 0.0f) return;
		float length = std::sqrt(sqrMag);

		float sqrMag1 = (bones[second]->solverPosition - bones[first]->solverPosition).lengthSquared();
		float sqrMag2 = (bones[third]->solverPosition - bones[second]->solverPosition).lengthSquared();

		// Get the general world space bending direction
		Vector3 bendDir = dir.cross(bendNormal);

		// Get the direction to the trigonometrically solved position of the second transform
		Vector3 toBendPoint = GetDirectionToBendPoint(dir, length, bendDir, sqrMag1, sqrMag2);

		// Position the second transform
		Quaternion q1 = Quaternion::FromToRotation(bones[second]->solverPosition - bones[first]->solverPosition, toBendPoint);
		if (weight < 1.0f) q1 = Quaternion::Lerp(Quaternion::identity, q1, weight);

		RotateAroundPoint(bones, first, bones[first]->solverPosition, q1);

		Quaternion q2 = Quaternion::FromToRotation(bones[third]->solverPosition - bones[second]->solverPosition, targetPosition - bones[second]->solverPosition);
		if (weight < 1.0f) q2 = Quaternion::Lerp(Quaternion::identity, q2, weight);

		RotateAroundPoint(bones, second, bones[second]->solverPosition, q2);
	}

	Vector3 IKSolverVR::VirtualBone::GetDirectionToBendPoint(Vector3 direction, float directionMag, Vector3 bendDirection, float sqrMag1, float sqrMag2)
	{
		float x = ((directionMag * directionMag) + (sqrMag1 - sqrMag2)) / 2.0f / directionMag;
		float y = (float)std::sqrt(std::clamp(sqrMag1 - x * x, 0.0f, std::numeric_limits<float>::infinity()));

		if (direction == Vector3::zero) return Vector3::zero;
		return Quaternion::LookRotation(direction, bendDirection) * Vector3(0.0f, y, x);
	}

	void IKSolverVR::VirtualBone::SolveFABRIK(std::vector<VirtualBone*> bones, Vector3 startPosition, Vector3 targetPosition, float weight, float minNormalizedTargetDistance, int iterations, float length, Vector3 startOffset)
	{
		if (weight <= 0.0f) return;

		if (minNormalizedTargetDistance > 0.0f)
		{
			Vector3 targetDirection = targetPosition - startPosition;
			float targetLength = targetDirection.length();
			Vector3 tP = startPosition + (targetDirection / targetLength) * std::max(length * minNormalizedTargetDistance, targetLength);
			targetPosition = Vector3::Lerp(targetPosition, tP, weight);
		}

		// Iterating the solver
		for (int iteration = 0; iteration < iterations; iteration++)
		{
			// Stage 1
			bones[bones.size() - 1]->solverPosition = Vector3::Lerp(bones[bones.size() - 1]->solverPosition, targetPosition, weight);

			for (int i = bones.size() - 2; i > -1; i--)
			{
				// Finding joint positions
				bones[i]->solverPosition = SolveFABRIKJoint(bones[i]->solverPosition, bones[i + 1]->solverPosition, bones[i]->length);
			}

			// Stage 2
			if (iteration == 0)
			{
				for (VirtualBone* bone : bones) bone->solverPosition += startOffset;
			}

			bones[0]->solverPosition = startPosition;

			for (int i = 1; i < bones.size(); i++)
			{
				bones[i]->solverPosition = SolveFABRIKJoint(bones[i]->solverPosition, bones[i - 1]->solverPosition, bones[i - 1]->length);
			}
		}

		for (int i = 0; i < bones.size() - 1; i++)
		{
			VirtualBone::SwingRotation(bones, i, bones[i + 1]->solverPosition);
		}
	}

	Vector3 IKSolverVR::VirtualBone::SolveFABRIKJoint(Vector3 pos1, Vector3 pos2, float length)
	{
		return pos2 + (pos1 - pos2).normalized() * length;
	}

	void IKSolverVR::VirtualBone::SolveCCD(std::vector<VirtualBone*> bones, Vector3 targetPosition, float weight, int iterations)
	{
		if (weight <= 0.0f) return;

		// Iterating the solver
		for (int iteration = 0; iteration < iterations; iteration++)
		{
			for (int i = bones.size() - 2; i > -1; i--)
			{
				Vector3 toLastBone = bones[bones.size() - 1]->solverPosition - bones[i]->solverPosition;
				Vector3 toTarget = targetPosition - bones[i]->solverPosition;


				Quaternion rotation = Quaternion::FromToRotation(toLastBone, toTarget);

				if (weight >= 1.0f)
				{
					//bones[i]->transform.rotation = targetRotation;
					VirtualBone::RotateBy(bones, i, rotation);
				}
				else
				{
					VirtualBone::RotateBy(bones, i, Quaternion::Lerp(Quaternion::identity, rotation, weight));
				}
			}
		}
	}
}