#pragma once

#include "Scene.h"
#include "AvatarSystem/Avatar.h"
#include "FinalIK/IKSolverVR.h"
#include "FinalIK/VRIK.h"
#include "Tracker.h"

class SetupScene : public Scene
{
public:
	SetupScene();
	virtual ~SetupScene();

	virtual void start();
	virtual void update(float dtime);
	virtual void draw();
	virtual void end();
	Tracker* generateTracker(std::string& file, const bool& pushToModels = true);
	const BaseModel* GetCharacter() const { return findModel("MarkerMan"); }

	void OnProgressSliderValueChanged(float value);
	//void OnPlayButtonPressed();
	//void OnPauseButtonPressed();
	void OnStopButtonPressed();
	void OnTPoseButtonPressed();
	void OnLoadAnimationButtonPressed(std::string path);
	void OnTrackerRemoved(Tracker* tracker);
	void OnTrackerSelected(Tracker* tracker);
	void OnTrackerHovered(Tracker* tracker);

	void ToggleDrawTrackerOffsets();
	void ToggleDrawSkeleton();
	void ToggleDrawTrackerOffsets(const bool& state);
	void ToggleDrawSkeleton(const bool& state);
	void TogglePlay();

	void ReloadScene(std::string characterFile);
	std::map<std::string, Tracker*> GetTrackers() const;
protected:
	virtual void ClearScene();
private:
	struct DragInfo
	{
		DragInfo() {
			active = false;
			selection = NULL;
			position = Vector3::zero;
			origin = Vector3::zero;
		}
		bool active;
		BaseModel* selection;
		Vector3 position;
		Vector3 origin;
	};

	void handleInput();
	void placeTracker();
	void select();
	void drag();
	void updateSelection();
	const bool IsDragControl(BaseModel* model) const;
	const bool ComputeDragPosition(const Vector3& position, Vector3& intersection) const;

	void LoadTrackerOffsetRepresentation(Tracker* tracker);
	void UpdateTrackerOffsetRepresentations();
	void LoadTransformRepresentations();
	void UpdateTransformRepresentations();

	//void SelectTracker(AttachedModel* tracker);
	//void HoverTracker(AttachedModel* tracker);
	/*bool dragMode = false;
	Vector3 dragPosition;
	BaseModel* dragSelection;*/

	DragInfo dragInfo;

	Tracker* selection;
	MeshModel** arrowsMove;
	MeshModel* arrowRotate;

	bool drawSkeleton;
	bool drawTrackerOffset;

	// DEBUG VARIABLES
	Avatar* avatar;
	RootMotion::IKSolverVR* ikSolver;
	RootMotion::VRIK::References references;
	std::vector<Transform*> transforms;
	std::vector<Transform*> targetTransforms;
};

