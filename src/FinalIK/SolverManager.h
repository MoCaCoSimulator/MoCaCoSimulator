#pragma once

#include <QDebug>

#include "../Animator.h"
#include "../Animation.h"
#include "../Transform.h"

namespace RootMotion
{
	/// <summary>
	/// Manages solver initiation and updating
	/// </summary>
	class SolverManager // : MonoBehaviour
	{
		#pragma region Main Interface

	public:
		SolverManager(Transform* root) : transform(root), animator(NULL), componentInitiated(false), legacy(NULL), skipSolverUpdate(false), updateFrame(false) {}

		/// <summary>
		/// 
		/// </summary>
		/// NOTE: Added to emulate MonoBehaviour
		Transform* transform = nullptr;

		/// <summary>
		/// If true, will fix all the Transforms used by the solver to their initial state in each Update. This prevents potential problems with unanimated bones and animator culling with a small cost of performance. Not recommended for CCD and FABRIK solvers.
		/// </summary>
		//[Tooltip("If true, will fix all the Transforms used by the solver to their initial state in each Update. This prevents potential problems with unanimated bones and animator culling with a small cost of performance. Not recommended for CCD and FABRIK solvers.")]
		bool fixTransforms = true;

		/// <summary>
		/// [DEPRECATED] Use "enabled = false" instead.
		/// </summary>
		void Disable()
		{
			qDebug() << "IK.Disable() is deprecated. Use enabled = false instead";

			enabled = false;
		}

		void UpdateSolverExternal(float deltaTime)
		{
			if (!enabled) return;

			skipSolverUpdate = true;

			UpdateSolver(deltaTime);
		}

		// Call this after the constructor to initiate the system
		void Initiate()
		{
			if (componentInitiated) return;

			//FindAnimatorRecursive(transform, true);

			InitiateSolver();
			componentInitiated = true;
		}

		#pragma endregion

	protected:
		// Dummy for enabled of Unity's GameObject
		bool enabled = true;

		virtual void InitiateSolver() = 0;
		virtual void UpdateSolver(float deltaTime) = 0;
		virtual void FixTransforms() = 0;

	private:
		Animator* animator = nullptr;
		Animation* legacy = nullptr;
		bool updateFrame = false;
		bool componentInitiated = false;

		void OnDisable()
		{
			//if (!Application.isPlaying) return;
			Initiate();
		}

		// Was: animatePhysics
		bool animatePhysics()
		{
			//if (animator != nullptr) return animator->updateMode == AnimatorUpdateMode.AnimatePhysics;
			//if (legacy != null) return legacy.animatePhysics;
			return false;
		}

		void Update()
		{
			if (skipSolverUpdate) return;
			if (animatePhysics()) return;

			if (fixTransforms) FixTransforms();
		}

		// Finds the first Animator/Animation up the hierarchy
		/*void FindAnimatorRecursive(Transform t, bool findInChildren)
		{
			if (isAnimated) return;

			animator = t.GetComponent<Animator>();
			legacy = t.GetComponent<Animation>();

			if (isAnimated) return;

			if (animator == null && findInChildren) animator = t.GetComponentInChildren<Animator>();
			if (legacy == null && findInChildren) legacy = t.GetComponentInChildren<Animation>();

			if (!isAnimated && t.parent != null) FindAnimatorRecursive(t.parent, false);
		}*/

		bool isAnimated() 
		{
			return animator != nullptr || legacy != nullptr;
		}

		// Workaround hack for the solver to work with animatePhysics
		void FixedUpdate()
		{
			if (skipSolverUpdate)
			{
				skipSolverUpdate = false;
			}

			updateFrame = true;

			if (animatePhysics() && fixTransforms) FixTransforms();
		}

		// Updating
		void LateUpdate(float deltaTime)
		{
			if (skipSolverUpdate) return;

			// Check if either animatePhysics is false or FixedUpdate has been called
			if (!animatePhysics()) updateFrame = true;
			if (!updateFrame) return;
			updateFrame = false;

			UpdateSolver(deltaTime);
		}

		// This enables other scripts to update the Animator on in FixedUpdate and the solvers with it
		bool skipSolverUpdate = false;
	};
}

