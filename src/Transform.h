#pragma once
#include "Matrix.h"
#include "Quaternion.h"
#include <vector>
#include <qDebug.h>
#include "Utils.h"
#include "MeshModel.h"

class Transform
{
public:
	enum class Space
	{
		Self,
		World
	};

	Transform();
	Transform(Matrix mat);
	Transform(Matrix mat, Transform* parent);
	~Transform() {}

	Transform* Parent() { return parent; }
	void SetParent(Transform* parent);
	const Transform* Root() const;
	void Name(const std::string& name) { this->name = name; }
	std::string Name() const { return name; }
	bool HasChanged() const { return hasChanged; }

	//Matrix LocalMatrix() const { return mat; }
	Matrix LocalMatrix() const;
	Matrix GlobalMatrix() const;
	Matrix LocalToWorldMatrix() const;
	Matrix WorldToLocalMatrix() const;
	void SetLocalMatrix(const Matrix& localMat);
	void SetGlobalMatrix(const Matrix& globalMat) { SetLocalMatrix(WorldToLocalMatrix() * globalMat); }
	
	Matrix LocalToWorldMatrixReversed() const;
	Matrix WorldToLocalMatrixReversed() const;
	Matrix LocalMatrixReversed() const;
	Matrix GlobalMatrixReversed() const;
	void Reverse();
	
	Vector3 LocalPosition() const { return localPos; }
	Quaternion LocalRotation() const { return localRot; }
	Vector3 LocalScale() const { return localScale; }
	void LocalPosition(const Vector3& pos);
	void LocalRotation(const Quaternion& rot);
	void LocalScale(const Vector3& scale);
	Vector3 LocalEulerAngles() const { return LocalRotation().eulerAngles() * Utils::RAD2DEG; }

	Vector3 Position() const { return (LocalToWorldMatrix() * localPos); }
	Quaternion Rotation() const { return (LocalToWorldMatrix() * localRot); }
	Vector3 EulerAngles() const { return Rotation().eulerAngles() * Utils::RAD2DEG; }
	Vector3 Scale() const { return LocalToWorldMatrix() * localScale; }
	void Position(const Vector3& pos) { LocalPosition(WorldToLocalMatrix() * pos); }
	void Rotation(const Quaternion& rot) { LocalRotation(WorldToLocalMatrix() * rot); }
	void Scale(const Vector3& scale) { LocalScale(WorldToLocalMatrix() * scale); }
	void SetPositionAndRotation(const Vector3& pos, const Quaternion& rot);

	Vector3 Forward() const { return GlobalMatrix().forward().normalize(); }
	Vector3 Right() const { return GlobalMatrix().right().normalize(); }
	Vector3 Up() const { return GlobalMatrix().up().normalize(); }

	size_t ChildCount() const { return children.size(); }

	bool operator==(const Transform& rhs) const { return this == &rhs; }
	bool operator!=(const Transform& rhs) const { return this != &rhs; }

	//int HierarchyCapacity() const;
	//int HierarchyCount() const;

	void DetachChildren();
	Transform* Find(const std::string& name) const;
	Transform* GetChild(const int& id) const;
	std::vector<Transform*> GetChildren() const { return children; }
	//GetSiblingIndex()
	bool IsChildOf(const Transform* parent) const { return parent == this->parent; }
	void LookAt(const Transform& target);
	void LookAt(const Transform& target, const Vector3& up);
	void Rotate(const Vector3& axis, float angle, Space relativeTo = Space::Self);
	//RotateAround
	//SetAsFirstSibling
	//SetAsLastSibling
	//SetSiblingIndex

	Vector3 TransformDirection(const Vector3& dir) const { return LocalToWorldMatrix().rotationMatrix() * dir; }
	Vector3 TransformPoint(const Vector3& point) const { return LocalToWorldMatrix() * point; }
	Vector3 TransformVector(const Vector3& vec) const { Matrix ltw = LocalToWorldMatrix(); return (ltw.rotationMatrix() * ltw.scaleMatrix()) * vec; }
	Vector3 InverseTransformDirection(const Vector3& dir) const { return WorldToLocalMatrix().rotationMatrix() * dir; }
	Vector3 InverseTransformPoint(const Vector3& point) const { return WorldToLocalMatrix() * point; }
	Vector3 InverseTransformVector(const Vector3& vec) const { Matrix wtl = WorldToLocalMatrix(); return (wtl.rotationMatrix() * wtl.scaleMatrix()) * vec; }

	void Translate(const Vector3& pos) { LocalPosition(localPos + pos); }
private:
	void UpdateLocalMatrix() const;

	mutable Matrix localMat;
	mutable bool hasChanged;
	Transform* parent;
	std::vector<Transform*> children = std::vector<Transform*>();
	std::string name = "";

	Quaternion localRot;
	Vector3 localScale;
	Vector3 localPos;

	MeshModel* jointModel;
};
