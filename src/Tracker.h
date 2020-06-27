#pragma once
#include "AttachedModel.h"
#include "Parameter.h"

class Tracker
{
public:
	Tracker() : slot(""), model(NULL), offsetPos(Vector3::zero), offsetRot(Quaternion::identity), hovered(false), selected(false) {};
	Tracker(AttachedModel* model) : model(model), offsetPos(Vector3::zero), offsetRot(Quaternion::identity), hovered(false), selected(false) {};
	virtual ~Tracker() {};

	void Select(const bool& state);
	const bool& IsSelected() const { return selected; }
	void Hover(const bool& state);
	const bool& IsHovered() const { return hovered; }
	void SetOffsetPosition(const Vector3& state) { offsetPos = state; }
	const Vector3& GetOffsetPosition() const { return offsetPos; }
	void SetOffsetRotation(const Quaternion& state) { offsetRot = state; }
	const Quaternion& GetOffsetRotation() const { return offsetRot; }
	const Matrix& GetOffsetMatrix() const { return Matrix().translation(offsetPos) * offsetRot.toRotationMatrix(); }
	void SetModel(AttachedModel* newModel) { model = newModel; }
	AttachedModel* GetModel() const { return model; }
	void SetSlot(const std::string& slotName);
	void UpdateOffset();
	std::string GetSlot() const { return slot; }
	void UpdateColor();

	static Tracker Invalid;
private:
	AttachedModel* model;
	Vector3 offsetPos;
	Quaternion offsetRot;
	bool hovered;
	bool selected;
	std::string slot;
};

