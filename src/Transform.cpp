#include "Transform.h"
#include <algorithm>
#include <cmath>

Transform::Transform() :
	localMat(Matrix(Matrix::identity)),
	parent(nullptr),
	hasChanged(false),
	localPos(Vector3::zero),
	localRot(Quaternion::identity),
	localScale(Vector3::one)
{
	//UpdateLocalValues();
}

Transform::Transform(Matrix mat) :
	//mat(mat),
	parent(nullptr),
	hasChanged(false)
{
	SetLocalMatrix(mat);
	//UpdateLocalValues();
}

Transform::Transform(Matrix mat, Transform* parent) :
	//mat(mat),
	parent(nullptr),
	hasChanged(false)
{
	SetLocalMatrix(mat);
	SetParent(parent);
	//UpdateLocalValues();
}

void Transform::SetParent(Transform* parent)
{
	if (this->parent)
		this->parent->children.erase(std::remove(children.begin(), children.end(), this), children.end());

	this->parent = parent;
	parent->children.push_back(this);
}

const Transform* Transform::Root() const
{
	if (parent)
		return parent->Root();
	return this;
}

Matrix Transform::LocalMatrix() const
{
	if (hasChanged)
		UpdateLocalMatrix();
	return localMat;
}

Matrix Transform::GlobalMatrix() const
{
	return LocalToWorldMatrix() * LocalMatrix();
}

Matrix Transform::LocalToWorldMatrix() const
{
	if (parent != nullptr)
		return parent->GlobalMatrix();
	return Matrix::identity;
}

Matrix Transform::WorldToLocalMatrix() const
{
	return LocalToWorldMatrix().invert();
}

Matrix Transform::LocalMatrixReversed() const
{
	Vector3 pos = localPos.ReverseX();
	Quaternion rot = localRot.ReverseXW();
	Vector3 scale = localScale;

	return Matrix().translation(pos) * rot.toRotationMatrix() * Matrix().scale(scale);
}

Matrix Transform::GlobalMatrixReversed() const
{
	return LocalToWorldMatrixReversed() * LocalMatrixReversed();
}

Matrix Transform::LocalToWorldMatrixReversed() const
{
	Matrix globalMat = Matrix::identity;
	if (parent != nullptr)
		globalMat = parent->GlobalMatrixReversed() * globalMat;
	return globalMat;
}

Matrix Transform::WorldToLocalMatrixReversed() const
{
	return LocalToWorldMatrixReversed().invert();
}

void Transform::Reverse()
{
	LocalPosition(localPos.ReverseX());
	LocalRotation(localRot.ReverseXW());
}

void Transform::SetLocalMatrix(const Matrix& mat)
{
	/*LocalPosition(localMat.translation());
	LocalRotation(localMat.rotation());
	LocalScale(localMat.scale());*/

	localPos = mat.translation();
	localRot = mat.rotation();
	localScale = mat.scale();
	localMat = mat;
	hasChanged = false;
}

void Transform::LocalPosition(const Vector3& pos)
{
	localPos = pos;
	hasChanged = true;
}

void Transform::LocalRotation(const Quaternion& rot)
{
	localRot = rot;
	hasChanged = true;
}

void Transform::LocalScale(const Vector3& scale)
{
	localScale = scale;
	hasChanged = true;
}

void Transform::SetPositionAndRotation(const Vector3& pos, const Quaternion& rot)
{
	Position(pos);
	Rotation(rot);
}

void Transform::DetachChildren()
{
	for (Transform* child : children)
		child->parent = NULL;
	children.clear();
}

Transform* Transform::Find(const std::string& name) const
{
	if (this->name == name)
		return (Transform*)this;

	for (Transform* child : children)
		return child->Find(name);

	return NULL;
}

Transform* Transform::GetChild(const int& id) const
{
	//qDebug() << "get " << id;
	if (id >= ChildCount() || id < 0)
		return nullptr;
	//qDebug() << "id valid" << ChildCount();
	return children.at(id);
}

void Transform::LookAt(const Transform& target)
{
	Vector3 dir = target.Position() - Position();
	Matrix rot = Matrix().lookAt(dir);
	Rotation(rot.rotation());
}

void Transform::LookAt(const Transform& target, const Vector3& up)
{
	Vector3 dir = target.Position() - Position();
	Matrix rot = Matrix().lookAt(dir, up);
	Rotation(rot.rotation());
}

void Transform::Rotate(const Vector3& axis, float angle, Space relativeTo)
{
	angle *= Utils::DEG2RAD;

	Vector3 a = relativeTo == Space::Self ? TransformDirection(axis) : axis;

	Quaternion rot = Quaternion().angleAxis(angle, a);

	//Rotation(Rotation() * rot);
	Rotation(rot * Rotation());
}

void Transform::UpdateLocalMatrix() const
{
	localMat = Matrix().translation(localPos) * localRot.toRotationMatrix() * Matrix().scale(localScale);
	hasChanged = false;
}
