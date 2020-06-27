#pragma once

#include "IK.h"
#include "../MeshModel.h"
#include "../Transform.h"

#include <array>

namespace RootMotion
{
	// Forward declaration to fix circular dependency between VRIK <-> IKSolverVR
	class IKSolverVR;

	/// <summary>
	/// A full-body IK solver designed specifically for a VR HMD and hand controllers.
	/// </summary>
	class VRIK : public IK
	{
	public:
		VRIK(Transform* root) : IK(root) {}

		/// <summary>
		/// VRIK-specific definition of a humanoid biped.
		/// </summary>
		//[System.Serializable]
		class References
		{
		public:
			Transform* root = nullptr;			// 0
			Transform* pelvis = nullptr;		// 1
			Transform* spine = nullptr;         // 24

			//[Tooltip("Optional")]
			Transform* chest = nullptr;         // 3 Optional

			//[Tooltip("Optional")]
			Transform* neck = nullptr; 			// 4 Optional
			Transform* head = nullptr;          // 5

			//[Tooltip("Optional")]
			Transform* leftShoulder = nullptr;	// 6 Optional
			Transform* leftUpperArm = nullptr;	// 7
			Transform* leftForearm = nullptr;	// 8
			Transform* leftHand = nullptr;      // 9

			//[Tooltip("Optional")]
			Transform* rightShoulder = nullptr;	// 10 Optional
			Transform* rightUpperArm = nullptr;	// 11
			Transform* rightForearm = nullptr;	// 12
			Transform* rightHand = nullptr;     // 13

			//[Tooltip("VRIK also supports legless characters.If you do not wish to use legs, leave all leg references empty.")]
			Transform* leftThigh = nullptr;     // 14 Optional

			//[Tooltip("VRIK also supports legless characters.If you do not wish to use legs, leave all leg references empty.")]
			Transform* leftCalf = nullptr;      // 15 Optional

			//[Tooltip("VRIK also supports legless characters.If you do not wish to use legs, leave all leg references empty.")]
			Transform* leftFoot = nullptr;      // 16 Optional

			//[Tooltip("Optional")]
			Transform* leftToes = nullptr;      // 17 Optional

			//[Tooltip("VRIK also supports legless characters.If you do not wish to use legs, leave all leg references empty.")]
			Transform* rightThigh = nullptr;    // 18 Optional

			//[Tooltip("VRIK also supports legless characters.If you do not wish to use legs, leave all leg references empty.")]
			Transform* rightCalf = nullptr;     // 19 Optional

			//[Tooltip("VRIK also supports legless characters.If you do not wish to use legs, leave all leg references empty.")]
			Transform* rightFoot = nullptr;     // 20 Optional

			//[Tooltip("Optional")]
			Transform* rightToes = nullptr;		// 21 Optional

			/// <summary>
			/// Returns an array of all the Transforms in the definition.
			/// </summary>
			std::vector<Transform*> GetTransforms();

			/// <summary>
			/// Returns true if all required Transforms have been assigned (shoulder, toe and neck bones are optional).
			/// </summary>
			bool isFilled() const;

			/// <summary>
			/// Returns true if none of the Transforms have been assigned.
			/// </summary>
			bool isEmpty();

			/// <summary>
			/// Auto-detects VRIK references. Works with a Humanoid Animator on the root gameobject only.
			/// </summary>
			//static bool AutoDetectReferences(Transform& root, References* references);
		};

	protected:
		// Open the User Manual URL
		//[ContextMenu("User Manual")]
		void OpenUserManual() override;

		// Open the Script Reference URL
		//[ContextMenu("Scrpt Reference")]
		void OpenScriptReference() override;

		// Open a video tutorial about setting up the component
		//[ContextMenu("TUTORIAL VIDEO (STEAMVR SETUP)")]
	private:
		void OpenSetupTutorial();

	public:
		/// <summary>
		/// Bone mapping. Right-click on the component header and select 'Auto-detect References' of fill in manually if not a Humanoid character. Chest, neck, shoulder and toe bones are optional. VRIK also supports legless characters. If you do not wish to use legs, leave all leg references empty.
		/// </summary>
		//[ContextMenuItem("Auto-detect References", "AutoDetectReferences")]
		//[Tooltip("Bone mapping. Right-click on the component header and select 'Auto-detect References' of fill in manually if not a Humanoid character. Chest, neck, shoulder and toe bones are optional. VRIK also supports legless characters. If you do not wish to use legs, leave all leg references empty.")]
		References references = References();

		/// <summary>
		/// The solver.
		/// </summary>
		//[Tooltip("The VRIK solver.")]
		// NOTE: Moved construction to InitiateSolver(), because of the circular dependency
		IKSolverVR* solver = nullptr;

		/// <summary>
		/// Auto-detects bone references for this VRIK. Works with a Humanoid Animator on the gameobject only.
		/// </summary>
		//[ContextMenu("Auto-detect References")]
		//void AutoDetectReferences();

		/// <summary>
		/// Fills in arm wristToPalmAxis and palmToThumbAxis.
		/// </summary>
		//[ContextMenu("Guess Hand Orientations")]
		void GuessHandOrientations();

		IKSolver* GetIKSolver() override;

	protected:
		void InitiateSolver() override;

		void UpdateSolver(float deltaTime) override;
	};
}