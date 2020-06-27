#include "BaseErrorMetric.h"
#include "../../Animator.h"

BaseErrorMetric::BaseErrorMetric(std::string name) : name(name)
{
}

std::vector<float> BaseErrorMetric::CalculateValues
(
    SkinnedModel* groundTruthSkinnedModel, 
    SkinnedModel* solvedSkinnedModel,
    Animator* groundTruthAnimator,
    Animator* solvedAnimator,
    std::vector<float> sampleTimes
)
{
    std::vector<float> results;

    float animationLength = groundTruthAnimator->GetAnimationLength();

    Pose groundTruthPose;
    Pose solvedPose;

    groundTruthPose.skinnedModel = groundTruthSkinnedModel;
    solvedPose.skinnedModel = solvedSkinnedModel;

    std::map<std::string, Vector3> prevGroundTruthPositions;
    std::map<std::string, Vector3> prevGroundTruthVelocities;
    std::map<std::string, Vector3> prevSolvedPositions;
    std::map<std::string, Vector3> prevSolvedVelocities;

    for (size_t i = 0; i < sampleTimes.size(); i++)
    {
        groundTruthPose.velocities.clear();
        groundTruthPose.accelerations.clear();
        solvedPose.velocities.clear();
        solvedPose.accelerations.clear();

        float time = sampleTimes[i];

        groundTruthAnimator->SetNormalizedAnimationTime(time / animationLength);
        solvedAnimator->SetNormalizedAnimationTime(time / animationLength);

        std::map<std::string, Vector3> groundTruthVelocities = std::map<std::string, Vector3>();
        std::map<std::string, Vector3> solvedVelocities = std::map<std::string, Vector3>();

        // Calculate velocity
        if (i > 0)
        {
            for (std::pair<std::string, MeshModel::JointInfo> pair : groundTruthSkinnedModel->GetJointMapping())
                groundTruthVelocities[pair.first] = pair.second.transform.translation() - prevGroundTruthPositions[pair.first];

            groundTruthPose.velocities = groundTruthVelocities;

            for (std::pair<std::string, MeshModel::JointInfo> pair : solvedSkinnedModel->GetJointMapping())
                solvedVelocities[pair.first] = pair.second.transform.translation() - prevSolvedPositions[pair.first];

            solvedPose.velocities = solvedVelocities;
        }

        // Calculate acceleration
        if (i > 1)
        {
            std::map<std::string, Vector3> groundTruthAccelerations = std::map<std::string, Vector3>();
            std::map<std::string, Vector3> solvedAccelerations = std::map<std::string, Vector3>();

            for (std::pair<std::string, MeshModel::JointInfo> pair : groundTruthSkinnedModel->GetJointMapping())
                groundTruthAccelerations[pair.first] = groundTruthVelocities[pair.first] - prevGroundTruthVelocities[pair.first];

            groundTruthPose.accelerations = groundTruthAccelerations;

            for (std::pair<std::string, MeshModel::JointInfo> pair : solvedSkinnedModel->GetJointMapping())
                solvedAccelerations[pair.first] = solvedVelocities[pair.first] - prevGroundTruthVelocities[pair.first];

            solvedPose.accelerations = solvedAccelerations;
        }

        // Check if a value can be calculated
        float result = 0.0f;
        if (CalculateDifference(groundTruthPose, solvedPose, result))
            results.push_back(result);
        else
            results.push_back(NAN);

        for (std::pair<std::string, MeshModel::JointInfo> pair : groundTruthSkinnedModel->GetJointMapping())
            prevGroundTruthPositions[pair.first] = pair.second.transform.translation();

        for (std::pair<std::string, MeshModel::JointInfo> pair : solvedSkinnedModel->GetJointMapping())
            prevSolvedPositions[pair.first] = pair.second.transform.translation();

        prevGroundTruthVelocities = groundTruthVelocities;
        prevSolvedVelocities = solvedVelocities;
    }

    return results;
}

std::vector<const BaseErrorMetric*>& BaseErrorMetric::registry()
{
    static std::vector<const BaseErrorMetric*> impl;
    return impl;
}

std::string BaseErrorMetric::GetName() const
{
    return name;
}

int BaseErrorMetric::GetSamplerate()
{
	return dynamic_cast<Parameter<int>*>(parameters["SampleRate"])->GetValue();
}

void BaseErrorMetric::AddParameter(BaseParameter* parameter)
{
    parameters[parameter->GetName()] = parameter;
}