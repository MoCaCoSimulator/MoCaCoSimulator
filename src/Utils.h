#pragma once

#include <map>
#include <string>
#include <vector>
#include <algorithm>

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

namespace Utils
{
    static const float RAD2DEG = 180.0 / M_PI;
    static const float DEG2RAD = M_PI / 180.0;

    // Functions
    std::string FilenameFromPath(const std::string& path, bool withExtension, const std::string& delims);

    std::string FoldernameFromPath(const std::string& path, const std::string& delims);

    inline bool FileExists(const std::string& name)
    {
        struct stat buffer;
        return (stat(name.c_str(), &buffer) == 0);
    }

    // Template functions
    template<typename T, typename D>
    bool Contains(std::map<T, D> map, T value)
    {
        std::map<T, D>::iterator it = map.find(value);
        if (it != map.end())
            return true;
        return false;
    }

    template <typename T>
    int FindIndexInVector(const std::vector<T>& vecOfElements, const T& element)
    {
        // Find given element in vector
        auto it = std::find(vecOfElements.begin(), vecOfElements.end(), element);

        if (it != vecOfElements.end())
            return distance(vecOfElements.begin(), it);
        else
            return -1;
    }

    template <typename T> 
    inline int Sign(T val)
    {
        return (T(0) < val) - (val < T(0));
    }

    inline float Lerp(float lhs, float rhs, float value)
    {
        if (value < 0.0f)
            value = 0.0f;
        if (value > 1.0f)
            value = 1.0f;
        return lhs + value * (rhs - lhs);
    }

    inline float Repeat(float t, float length)
    {
        return std::clamp(t - std::floor(t / length) * length, 0.0f, length);
    }

    inline float DeltaAngle(float current, float target)
    {
        float delta = Repeat((target - current), 360.0f);
        if (delta > 180.0f)
            delta -= 360.0f;
        return delta;
    }

    // Source: https://github.com/Unity-Technologies/UnityCsReference/blob/master/Runtime/Export/Math/Mathf.cs
    // Gradually changes a value towards a desired goal over time.
    static float SmoothDamp(float current, float target, float& currentVelocity, float smoothTime, float deltaTime /* = Time.deltaTime*/, float maxSpeed = std::numeric_limits<float>::infinity())
    {
        // Based on Game Programming Gems 4 Chapter 1.10
        smoothTime = std::max(0.0001F, smoothTime);
        float omega = 2.0F / smoothTime;

        float x = omega * deltaTime;
        float exp = 1.0F / (1.0F + x + 0.48F * x * x + 0.235F * x * x * x);
        float change = current - target;
        float originalTo = target;

        // Clamp maximum speed
        float maxChange = maxSpeed * smoothTime;
        change = std::clamp(change, -maxChange, maxChange);
        target = current - change;

        float temp = (currentVelocity + omega * change) * deltaTime;
        currentVelocity = (currentVelocity - omega * temp) * exp;
        float output = target + (change + temp) * exp;

        // Prevent overshooting
        if (originalTo - current > 0.0F == output > originalTo)
        {
            output = originalTo;
            currentVelocity = (output - originalTo) / deltaTime;
        }

        return output;
    }

    // Source: https://github.com/Unity-Technologies/UnityCsReference/blob/master/Runtime/Export/Math/Mathf.cs
    // Moves a value /current/ towards /target/.
    inline float MoveTowards(float current, float target, float maxDelta)
    {
        if (std::abs(target - current) <= maxDelta)
            return target;
        return current + Sign(target - current) * maxDelta;
    }

    // Source: https://stackoverflow.com/questions/5289613/generate-random-float-between-two-floats/5289624
    inline float RandomFloat(float a, float b)
    {
        float random = ((float)rand()) / (float)RAND_MAX;
        float diff = b - a;
        float r = random * diff;
        return a + r;
    }
};