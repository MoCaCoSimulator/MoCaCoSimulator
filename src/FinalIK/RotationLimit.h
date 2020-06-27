#pragma once

#include "../vector.h"
#include "../Quaternion.h"
#include "../Transform.h"

#include <QDebug>

namespace RootMotion
{
	class RotationLimit
	{
        #pragma region Main Interface

    public:
        /// <summary>
        /// 
        /// </summary>
        /// NOTE: Newly added
        Transform* transform = nullptr;

        /// <summary>
        /// The main axis of the rotation limit.
        /// </summary>
        Vector3 axis = Vector3::forward;

        /// <summary>
        /// Map the zero rotation point to the current local rotation of this gameobject.
        /// </summary>
        void SetDefaultLocalRotation()
        {
            defaultLocalRotation = transform->LocalRotation();
            defaultLocalRotationSet = true;
            defaultLocalRotationOverride = false;
        }

        /// <summary>
        /// Map the zero rotation point to the specified rotation.
        /// </summary>
        void SetDefaultLocalRotation(Quaternion localRotation)
        {
            defaultLocalRotation = localRotation;
            defaultLocalRotationSet = true;
            defaultLocalRotationOverride = true;
        }

        /// <summary>
        /// Returns the limited local rotation.
        /// </summary>
        Quaternion GetLimitedLocalRotation(Quaternion localRotation, bool& changed)
        {
            // Making sure the Rotation Limit is initiated
            if (!initiated) Awake();

            // Subtracting defaultLocalRotation
            Quaternion rotation = defaultLocalRotation.inverse() * localRotation;

            Quaternion limitedRotation = LimitRotation(rotation);
#if UNITY_2018_3_OR_NEWER
            limitedRotation = Quaternion.Normalize(limitedRotation);
#endif
            changed = limitedRotation != rotation;

            if (!changed) return localRotation;

            // Apply defaultLocalRotation back on
            return defaultLocalRotation * limitedRotation;
        }

        /// <summary>
        /// Apply the rotation limit to transform.localRotation. Returns true if the limit has changed the rotation.
        /// </summary>
        bool Apply()
        {
            bool changed = false;

            transform->LocalRotation(GetLimitedLocalRotation(transform->LocalRotation(), changed));

            return changed;
        }

        /// <summary>
        /// Disable this instance making sure it is initiated. Use this if you intend to manually control the updating of this Rotation Limit.
        /// </summary>
        void Disable()
        {
            if (initiated)
            {
                enabled = false;
                return;
            }

            Awake();
            enabled = false;
        }

        #pragma endregion

        /*
        * An arbitrary secondary axis that we get by simply switching the axes
        * */
        Vector3 secondaryAxis()
        {
            return Vector3(axis.y, axis.z, axis.x);
        }

        /*
        * Cross product of axis and secondaryAxis
        * */
        Vector3 crossAxis() 
        {
            return Vector3::Cross(axis, secondaryAxis());
        }

        /*
        * The default local rotation of the gameobject. By default stored in Awake.
        * */
        //[HideInInspector] 
        Quaternion defaultLocalRotation = Quaternion::identity;

        bool GetDefaultLocalRotationOverride()
        {
            return defaultLocalRotationOverride;
        }

    protected:
        virtual Quaternion LimitRotation(Quaternion rotation) = 0;

    private:
        // NOTE: Replacement for GameObject.enabled
        bool enabled = true;

        bool defaultLocalRotationOverride = false;
        bool initiated = false;
        bool applicationQuit = false;
        bool defaultLocalRotationSet = false;

        /*
         * Initiating the Rotation Limit
         * */
        void Awake()
        {
            // Store the local rotation to map the zero rotation point to the current rotation
            if (!defaultLocalRotationSet) SetDefaultLocalRotation();

            if (axis == Vector3::zero) qDebug() << "RootMotion::RotationLimit: Error -> Axis is Vector3.zero.";
            initiated = true;
        }

        /*
         * Using LateUpdate here because you most probably want to apply the limits after animation.
         * If you need precise control over the execution order, disable this script and call Apply() whenever you need
         * */
        void LateUpdate()
        {
            Apply();
        }

    public:
        /*
         * Logs the warning if no other warning has beed logged in this session.
         * */
        void LogWarning(std::string message)
        {
            qDebug() << "Warning -> RotationLimit ->" << message.c_str();
        }

        #pragma region Static helper methods for all Rotation Limits

    protected:
        /*
        * Limits rotation to a single degree of freedom (along axis)
        * */
        static Quaternion Limit1DOF(Quaternion rotation, Vector3 axis)
        {
            return Quaternion::FromToRotation(rotation * axis, axis) * rotation;
        }

        /*
         * Applies uniform twist limit to the rotation
         * */
        static Quaternion LimitTwist(Quaternion rotation, Vector3 axis, Vector3 orthoAxis, float twistLimit)
        {
            twistLimit = std::clamp(twistLimit, 0.0f, 180.0f);
            if (twistLimit >= 180) return rotation;

            Vector3 normal = rotation * axis;
            Vector3 orthoTangent = orthoAxis;
            Vector3::OrthoNormalize(normal, orthoTangent);

            Vector3 rotatedOrthoTangent = rotation * orthoAxis;
            Vector3::OrthoNormalize(normal, rotatedOrthoTangent);

            Quaternion fixedRotation = Quaternion::FromToRotation(rotatedOrthoTangent, orthoTangent) * rotation;

            if (twistLimit <= 0) return fixedRotation;

            // Rotate from zero twist to free twist by the limited angle
            return Quaternion::RotateTowards(fixedRotation, rotation, twistLimit * Utils::DEG2RAD);
        }

        /*
         * Returns the angle between two vectors on a plane with the specified normal
         * */
        static float GetOrthogonalAngle(Vector3 v1, Vector3 v2, Vector3 normal)
        {
            Vector3::OrthoNormalize(normal, v1);
            Vector3::OrthoNormalize(normal, v2);
            return Vector3::AngleDegree(v1, v2);
        }

        #pragma endregion
	};
}