#include "VRIK.h"
#include "IKSolverVR.h"

namespace RootMotion
{
	std::vector<Transform*> VRIK::References::GetTransforms()
	{
		std::vector<Transform*> transforms =
		{
			root, pelvis, spine, chest, neck, head, leftShoulder, leftUpperArm, leftForearm, leftHand, rightShoulder, rightUpperArm, rightForearm, rightHand, leftThigh, leftCalf, leftFoot, leftToes, rightThigh, rightCalf, rightFoot, rightToes
		};
		return transforms;
	}

	bool VRIK::References::isFilled() const
	{
		if (
			root == nullptr ||
			pelvis == nullptr ||
			spine == nullptr ||
			head == nullptr ||
			leftUpperArm == nullptr ||
			leftForearm == nullptr ||
			leftHand == nullptr ||
			rightUpperArm == nullptr ||
			rightForearm == nullptr ||
			rightHand == nullptr
			) return false;

		// If all leg bones are null, it is valid
		bool noLegBones =
			leftThigh == nullptr &&
			leftCalf == nullptr &&
			leftFoot == nullptr &&
			rightThigh == nullptr &&
			rightCalf == nullptr &&
			rightFoot == nullptr;

		if (noLegBones) return true;

		bool atLeastOneLegBoneMissing =
			leftThigh == nullptr ||
			leftCalf == nullptr ||
			leftFoot == nullptr ||
			rightThigh == nullptr ||
			rightCalf == nullptr ||
			rightFoot == nullptr;

		if (atLeastOneLegBoneMissing) return false;

		// Shoulder, toe and neck bones are optional
		return true;
	};

	bool VRIK::References::isEmpty()
	{
		if (
			root != nullptr ||
			pelvis != nullptr ||
			spine != nullptr ||
			chest != nullptr ||
			neck != nullptr ||
			head != nullptr ||
			leftShoulder != nullptr ||
			leftUpperArm != nullptr ||
			leftForearm != nullptr ||
			leftHand != nullptr ||
			rightShoulder != nullptr ||
			rightUpperArm != nullptr ||
			rightForearm != nullptr ||
			rightHand != nullptr ||
			leftThigh != nullptr ||
			leftCalf != nullptr ||
			leftFoot != nullptr ||
			leftToes != nullptr ||
			rightThigh != nullptr ||
			rightCalf != nullptr ||
			rightFoot != nullptr ||
			rightToes != nullptr
			) return false;

		return true;
	};

	//bool VRIK::References::AutoDetectReferences(Transform& root, References* references)
	//{
		/*references = new VRIK::References();

		var animator = root.GetComponentInChildren<Animator>();
		if (animator == null || !animator.isHuman)
		{
			qDebug() << "VRIK needs a Humanoid Animator to auto-detect biped references. Please assign references manually.";
			return false;
		}

		references.root = root;
		references.pelvis = animator.GetBoneTransform(HumanBodyBones.Hips);
		references.spine = animator.GetBoneTransform(HumanBodyBones.Spine);
		references.chest = animator.GetBoneTransform(HumanBodyBones.Chest);
		references.neck = animator.GetBoneTransform(HumanBodyBones.Neck);
		references.head = animator.GetBoneTransform(HumanBodyBones.Head);
		references.leftShoulder = animator.GetBoneTransform(HumanBodyBones.LeftShoulder);
		references.leftUpperArm = animator.GetBoneTransform(HumanBodyBones.LeftUpperArm);
		references.leftForearm = animator.GetBoneTransform(HumanBodyBones.LeftLowerArm);
		references.leftHand = animator.GetBoneTransform(HumanBodyBones.LeftHand);
		references.rightShoulder = animator.GetBoneTransform(HumanBodyBones.RightShoulder);
		references.rightUpperArm = animator.GetBoneTransform(HumanBodyBones.RightUpperArm);
		references.rightForearm = animator.GetBoneTransform(HumanBodyBones.RightLowerArm);
		references.rightHand = animator.GetBoneTransform(HumanBodyBones.RightHand);
		references.leftThigh = animator.GetBoneTransform(HumanBodyBones.LeftUpperLeg);
		references.leftCalf = animator.GetBoneTransform(HumanBodyBones.LeftLowerLeg);
		references.leftFoot = animator.GetBoneTransform(HumanBodyBones.LeftFoot);
		references.leftToes = animator.GetBoneTransform(HumanBodyBones.LeftToes);
		references.rightThigh = animator.GetBoneTransform(HumanBodyBones.RightUpperLeg);
		references.rightCalf = animator.GetBoneTransform(HumanBodyBones.RightLowerLeg);
		references.rightFoot = animator.GetBoneTransform(HumanBodyBones.RightFoot);
		references.rightToes = animator.GetBoneTransform(HumanBodyBones.RightToes);*/

		//return true;
	//}

	void VRIK::OpenUserManual()
	{
		//Application.OpenURL("http://www.root-motion.com/finalikdox/html/page16.html");
	}

	void VRIK::OpenScriptReference()
	{
		//Application.OpenURL("http://www.root-motion.com/finalikdox/html/class_root_motion_1_1_final_i_k_1_1_v_r_i_k.html");
	}

	void VRIK::OpenSetupTutorial()
	{
		//Application.OpenURL("https://www.youtube.com/watch?v=6Pfx7lYQiIA&feature=youtu.be");
	}

	void VRIK::GuessHandOrientations()
	{
		solver->GuessHandOrientations(references, false);
	};

	IKSolver* VRIK::GetIKSolver()
	{
		return dynamic_cast<IKSolver*>(solver);
	};

	void VRIK::InitiateSolver()
	{
		solver = new IKSolverVR();

		//if (references.isEmpty()) AutoDetectReferences();
		if (references.isFilled()) solver->SetToReferences(references);

		IK::InitiateSolver();
	};

	void VRIK::UpdateSolver(float deltaTime)
	{
		if (references.root != nullptr && references.root->LocalScale() == Vector3::zero)
		{
			qDebug() << "VRIK Root Transform's scale is zero, can not update VRIK. Make sure you have not calibrated the character to a zero scale.";
			enabled = false;
			return;
		}

		IK::UpdateSolver(deltaTime);
	};
}