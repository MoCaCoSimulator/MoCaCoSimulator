#pragma once

#include "SolverManager.h"
#include "IKSolver.h"

namespace RootMotion
{
	/// <summary>
	/// Base abstract class for %IK solver components.
	/// </summary>
	class IK : public SolverManager
	{
		#pragma region Main Interface
	public:
		IK(Transform* root) : SolverManager(root) { }

		/// <summary>
		/// Gets the %IK component's solver as IKSolver.
		/// </summary>
		virtual IKSolver* GetIKSolver() = 0;

		#pragma endregion Main Interface

	protected:
		/*
		* Updates the solver. If you need full control of the execution order of your IK solvers, disable this script and call UpdateSolver() instead.
		* */
		virtual void UpdateSolver(float deltaTime) override
		{
			if (!GetIKSolver()->GetInitiated()) InitiateSolver();
			if (!GetIKSolver()->GetInitiated()) return;

			GetIKSolver()->Update(deltaTime);
		}

		/*
		 * Initiates the %IK solver
		 * */
		virtual void InitiateSolver() override
		{
			if (GetIKSolver()->GetInitiated()) return;

			GetIKSolver()->Initiate(transform);
		}

		virtual void FixTransforms() override
		{
			if (!GetIKSolver()->GetInitiated()) return;
			GetIKSolver()->FixTransforms();
		}

		// Open the User Manual url
		virtual void OpenUserManual() = 0;

		// Open the Script Reference url
		virtual void OpenScriptReference() = 0;
	};
}