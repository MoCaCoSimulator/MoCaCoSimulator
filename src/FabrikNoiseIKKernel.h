#include "BaseIKKernel.h"
#include <vector>

class FabrikNoiseIKKernel : public BaseIKKernel
{
public:
	FabrikNoiseIKKernel();

	static RegisterIKSolver<FabrikNoiseIKKernel> Register;

	virtual Animation* Solve(Animation groundTruthAnimation, Animation endEffectorsAnimation);
	virtual std::vector<std::string> InputNames();
};
