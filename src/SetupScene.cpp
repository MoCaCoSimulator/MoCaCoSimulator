#include "SetupScene.h"
#include "Paths.h"
#include "Quaternion.h"
#include "SkinnedModel.h"
#include "WeightShader.h"
#include "AnimatedModelShader.h"
#include "InputManager.h"
#include <qlineedit.h>
#include "SkinnedModel.h"
#include "EventManager.h"
#include "AttachedModel.h"
#include "AttachedModelShader.h"
#include <QDebug>
#include "Parameter.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <filesystem>

struct TransformRepresentation
{
public:
	float scale = 0.01f;
	//Transform* transform;
	MeshModel* jointModel;
	std::map<Transform*, MeshModel*> boneModels;

	void Toggle(bool state)
	{
		jointModel->Disabled(!state);
		for (const auto& kv : boneModels)
			kv.second->Disabled(!state);
	}
};

struct TrackerOffsetRepresentation
{
public:
	float scale = 0.01f;
	MeshModel* pointModel;
	MeshModel* lineModel;

	void Toggle(bool state)
	{
		pointModel->Disabled(!state);
		lineModel->Disabled(!state);
	}
};

std::map<Transform*, TransformRepresentation*> transformRepresentations;
std::map<Tracker*, TrackerOffsetRepresentation*> trackerOffsetRepresentations;
std::map<AttachedModel*, Tracker*> modelTrackerMap;

SetupScene::SetupScene() :
	Scene(),
	avatar(NULL),
	selection(&Tracker::Invalid),
	arrowRotate(NULL),
	arrowsMove(NULL),
	ikSolver(NULL),
	drawSkeleton(false),
	drawTrackerOffset(false)
{
	std::function<void()> voidCallback = std::function([this]() { return this->OnStopButtonPressed(); });
	EventManager::instance().SubscribeToEvent("OnStopButtonPressed", voidCallback);
	voidCallback = std::function([this]() { return this->OnTPoseButtonPressed(); });
	EventManager::instance().SubscribeToEvent("OnTPoseButtonPressed", voidCallback);
	voidCallback = std::function([this]() { return this->TogglePlay(); });
	EventManager::instance().SubscribeToEvent("OnTogglePlay", voidCallback);

	voidCallback = std::function([this]() { return this->ToggleDrawSkeleton(); });
	EventManager::instance().SubscribeToEvent("OnToggleSkeleton", voidCallback);
	voidCallback = std::function([this]() { return this->ToggleDrawTrackerOffsets(); });
	EventManager::instance().SubscribeToEvent("OnToggleTrackerOffsets", voidCallback);

	std::function<void(float)> floatCallback = std::function([this](float value) { return this->OnProgressSliderValueChanged(value); });
	EventManager::instance().SubscribeToEvent("OnProgressSliderValueChanged", floatCallback);

	std::function<void(std::string)> stringCallback = std::function([this](std::string value) { return this->OnLoadAnimationButtonPressed(value); });
	EventManager::instance().SubscribeToEvent("OnLoadAnimationButtonPressed", stringCallback);

	std::function<void(Tracker*)> trackerCallback = std::function([this](Tracker* m) { return this->OnTrackerRemoved(m); });
	EventManager::instance().SubscribeToEvent("OnTrackerRemoved", trackerCallback);
	trackerCallback = std::function([this](Tracker* m) { return this->OnTrackerSelected(m); });
	EventManager::instance().SubscribeToEvent("OnTrackerSelected", trackerCallback);
	trackerCallback = std::function([this](Tracker* m) { return this->OnTrackerHovered(m); });
	EventManager::instance().SubscribeToEvent("OnTrackerHovered", trackerCallback);
	trackerCallback = std::function([this](Tracker* m) { return this->OnTrackerHovered(NULL); });
	EventManager::instance().SubscribeToEvent("OnTrackerUnhovered", trackerCallback);
}

SetupScene::~SetupScene()
{
	if (avatar)
		delete avatar;

	delete sun;

	for (int i = 0; i < 4; i++)
		models.remove(arrowsMove[i]);
	models.remove(arrowRotate);

	delete arrowsMove[0]->shader();
	delete[] arrowsMove;
	delete arrowRotate;

	ClearScene();
}

void SetupScene::ClearScene()
{
	Scene::ClearScene();

	for (const auto& kv : transformRepresentations)
		delete kv.second;
	transformRepresentations.clear();
	for (const auto& kv : trackerOffsetRepresentations)
		delete kv.second;
	trackerOffsetRepresentations.clear();
	for (const auto& kv : modelTrackerMap)
		delete kv.second;
	modelTrackerMap.clear();

	delete ikSolver;

	selection = &Tracker::Invalid;
}

void SetupScene::LoadTrackerOffsetRepresentation(Tracker* tracker)
{
	PhongShader* pointShader = new PhongShader();
	pointShader->color(Color(0, 0, 1));
	pointShader->alpha(0.5f);
	PhongShader* lineShader = new PhongShader();
	lineShader->color(Color(0, 1, 1));
	lineShader->alpha(0.5f);

	TrackerOffsetRepresentation* representation = new TrackerOffsetRepresentation();

	MeshModel* pointModel = new MeshModel(MODEL_DIRECTORY "sphere.dae", false);
	pointModel->shader(pointShader, true);
	pointModel->setName("Point " + tracker->GetModel()->getName());
	pointModel->AlwaysOnTop(true);
	MeshModel* lineModel = new MeshModel(MODEL_DIRECTORY "cone.dae", false);
	lineModel->shader(lineShader, true);
	lineModel->setName("Line " + tracker->GetModel()->getName());
	lineModel->AlwaysOnTop(true);

	representation->pointModel = pointModel;
	representation->lineModel = lineModel;

	models.push_back(pointModel);
	models.push_back(lineModel);

	trackerOffsetRepresentations[tracker] = representation;

	ToggleDrawTrackerOffsets(drawTrackerOffset);
}

void SetupScene::UpdateTrackerOffsetRepresentations()
{
	for (const auto& kv : trackerOffsetRepresentations)
	{
		Tracker* tracker = kv.first;
		TrackerOffsetRepresentation* representation = kv.second;
		Matrix globalAnimatedTransform = tracker->GetModel()->getAnimationTransform();
		globalAnimatedTransform = globalAnimatedTransform.translationMatrix() * globalAnimatedTransform.rotationMatrix();
		Vector3 offsetPos = tracker->GetOffsetPosition();
		Quaternion offsetRot = tracker->GetOffsetRotation();

		Matrix pointTransform = Matrix().translation(offsetPos) * offsetRot.toRotationMatrix() * Matrix().scale(representation->scale);
		Matrix lineTransform = Matrix().translation(offsetPos) * Matrix().lookAt(offsetPos) * Matrix().scale(representation->scale, representation->scale, offsetPos.length());

		representation->pointModel->setTransform(globalAnimatedTransform * pointTransform);
		representation->lineModel->setTransform(globalAnimatedTransform * lineTransform);
	}
}

void SetupScene::LoadTransformRepresentations()
{
	PhongShader* jointShader = new PhongShader();
	jointShader->color(Color(0, 1, 0));
	jointShader->alpha(0.5f);
	PhongShader* boneShader = new PhongShader();
	boneShader->color(Color(1, 1, 0));
	boneShader->alpha(0.5f);

	int transformCount = targetTransforms.size();

	for (int i = 0; i < transformCount; i++)
	{
		Transform* transform = targetTransforms[i];
		if (transform->Name() == "RootNode")
			continue;

		TransformRepresentation* representation = new TransformRepresentation();

		MeshModel* jointModel = new MeshModel(MODEL_DIRECTORY "sphere.dae", false);
		jointModel->shader(jointShader, i == 0);
		jointModel->setName("Joint " + transform->Name());
		jointModel->AlwaysOnTop(true);

		representation->jointModel = jointModel;
		models.push_back(jointModel);

		for (int j = 0; j < transform->ChildCount(); j++)
		{
			Transform* child = transform->GetChild(j);

			MeshModel* boneModel = new MeshModel(MODEL_DIRECTORY "cone.dae", false);
			boneModel->shader(boneShader, i == 0 && j == 0);
			boneModel->setName("Bone " + transform->Name() + " to " + child->Name());
			boneModel->AlwaysOnTop(true);

			representation->boneModels[child] = boneModel;
			models.push_back(boneModel);
		}

		transformRepresentations[transform] = representation;
	}

	ToggleDrawSkeleton(drawSkeleton);
}

void SetupScene::UpdateTransformRepresentations()
{
	for (Transform* transform : targetTransforms)
	{
		if (transform->Name() == "RootNode")
			continue;

		TransformRepresentation* representation = transformRepresentations[transform];

		Matrix mat = transform->GlobalMatrixReversed();
		Matrix jointMat = mat.translationMatrix() * mat.rotationMatrix() * Matrix().scale(representation->scale);
		representation->jointModel->setTransform(jointMat);

		for (Transform* child : transform->GetChildren())
		{
			Matrix childMat = child->GlobalMatrixReversed();
			Vector3 jointDir = mat.translation() - childMat.translation();
			float jointScale = jointDir.length();

			Matrix boneMat = mat.translationMatrix() * Matrix().lookAt(jointDir) * Matrix().scale(representation->scale, representation->scale, jointScale);
			representation->boneModels[child]->setTransform(boneMat);
		}
	}
}

void SetupScene::start()
{
	Scene::start();

	sun = new DirectionalLight();
	float sunStrength = 0.5f;
	float ambientStrength = 1.0f;//0.25f;
	sun->color(Color(sunStrength, sunStrength, sunStrength));
	sun->direction(Vector3(0.2f, 0, -1));

	ShaderLightMapper::instance().addLight(sun);

	ReloadScene(CHARACTER_DIRECTORY "David/David.dae");
}

void SetupScene::ReloadScene(std::string characterFile)
{
	ClearScene();

	ConstantShader* arrowMoveShader = new ConstantShader();
	arrowMoveShader->color(Color(1, 0, 0));
	arrowMoveShader->alpha(0.3f);

	arrowsMove = new MeshModel * [4];
	for (int i = 0; i < 4; i++)
	{
		MeshModel* arrowMove = new MeshModel(MODEL_DIRECTORY "ArrowMove.dae");
		arrowMove->shader(arrowMoveShader, false);
		arrowMove->SetSelectable(true);
		arrowMove->setName("arrowMove" + i);
		arrowMove->setTransparency(TRANSPARENCY_FULL);
		arrowsMove[i] = arrowMove;
		models.push_back(arrowMove);
	}

	ConstantShader* arrowRotateShader = new ConstantShader();
	arrowRotateShader->color(Color(0, 1, 0));
	arrowRotateShader->alpha(0.3f);
	arrowRotate = new MeshModel(MODEL_DIRECTORY "ArrowRotate.dae");
	arrowRotate->setTransparency(TRANSPARENCY_FULL);
	arrowRotate->shader(arrowRotateShader, true);
	arrowRotate->SetSelectable(true);
	arrowRotate->setName("arrowRotate");
	models.push_back(arrowRotate);

	const char* modelfile = characterFile.c_str();

	SkinnedModel* skinnedModel = new SkinnedModel(modelfile, false);
	skinnedModel->shader(new SkinnedModelShader(), true);
	skinnedModel->setName("MarkerMan");

	models.push_back(skinnedModel);

	avatar = new Avatar(skinnedModel);
	Animator* idleAnimator = new Animator(*skinnedModel);

	animators.push_back(idleAnimator);

	targetTransforms = skinnedModel->ConvertRepresentationToTransforms();
	LoadTransformRepresentations();

	//idleAnimator->SetAnimation(Animation::LoadFromPath(ANIMATION_DIRECTORY "Mixamo/Gangnam Style.dae", 0));
	//idleAnimator->Pause();
	//idleAnimator->SetNormalizedAnimationTime(0.433f);
/*
	SkinnedModelShader* shader = new SkinnedModelShader();
	shader->color(Color(1.0f, 0.5f, 0.5f));
	SkinnedModel* finalIKSkinnedModel = new SkinnedModel(modelfile, false);
	finalIKSkinnedModel->shader(shader, true);
	finalIKSkinnedModel->setName("FinalIKMan");

	models.push_back(finalIKSkinnedModel);

	transforms = finalIKSkinnedModel->ConvertRepresentationToTransforms();

	Transform* root = nullptr;
	for (Transform* transform : transforms)
		if (transform->Name() == "RootNode")
			root = transform;

	if (root == nullptr)
		qDebug() << "ERROR";

	ikSolver = new RootMotion::IKSolverVR();

	references = RootMotion::VRIK::References();

	for (Transform* transform : transforms)
	{
		std::string name = transform->Name();
		if (name == "RootNode")
			references.root = transform; // 0
		else if (name == "Hips")
			references.pelvis = transform; // 1
		else if (name == "Spine")
			references.spine = transform; // 24
		else if (name == "Spine1")
			references.chest = transform; // 3 Optional
		else if (name == "Neck")
			references.neck = transform; // 4 Optional
		else if (name == "Head")
			references.head = transform; // 5
		else if (name == "LeftShoulder")
			references.leftShoulder = transform; // 6 Optional
		else if (name == "LeftArm")
			references.leftUpperArm = transform; // 7
		else if (name == "LeftForeArm")
			references.leftForearm = transform; // 8
		else if (name == "LeftHand")
			references.leftHand = transform; // 9
		else if (name == "RightShoulder")
			references.rightShoulder = transform; // 10 Optional
		else if (name == "RightArm")
			references.rightUpperArm = transform; // 11
		else if (name == "RightForeArm")
			references.rightForearm = transform; // 12
		else if (name == "RightHand")
			references.rightHand = transform; // 13
		else if (name == "LeftUpLeg")
			references.leftThigh = transform; // 14 Optional
		else if (name == "LeftLeg")
		{
			// Failsafe action for perfect line extrimities
			transform->Rotate(Vector3::right, 5.0f, Transform::Space::World);
			references.leftCalf = transform; // 15 Optional
		}
		else if (name == "LeftFoot")
			references.leftFoot = transform; // 16 Optional
		else if (name == "LeftToeBase")
			references.leftToes = transform; // 17 Optional
		else if (name == "RightUpLeg")
			references.rightThigh = transform; // 18 Optional
		else if (name == "RightLeg")
		{
			// Failsafe action for perfect line extrimities
			transform->Rotate(Vector3::right, 5.0f, Transform::Space::World);
			references.rightCalf = transform; // 19 Optional
		}
		else if (name == "RightFoot")
			references.rightFoot = transform; // 20 Optional
		else if (name == "RightToeBase")
			references.rightToes = transform; // 21 Optional
	}

	// Set the references to the solver
	ikSolver->SetToReferences(references);
	ikSolver->Initiate(root);

	// Setup the targets
	for (Transform* transform : targetTransforms)
	{
		if (transform->Name() == "Head")
			ikSolver->spine->headTarget = transform;
		else if (transform->Name() == "Hips")
			ikSolver->spine->pelvisTarget = transform;
		else if (transform->Name() == "LeftHand")
			ikSolver->leftArm->target = transform;
		else if (transform->Name() == "RightHand")
			ikSolver->rightArm->target = transform;
		else if (transform->Name() == "LeftToeBase")
			ikSolver->leftLeg->target = transform;
		else if (transform->Name() == "RightToeBase")
			ikSolver->rightLeg->target = transform;
		else
			continue;
	}

	// Setup the weights
	ikSolver->plantFeet = false;
	ikSolver->locomotion->weight = 0.0f;

	ikSolver->spine->minHeadHeight = 0.0f;
	ikSolver->spine->bodyPosStiffness = 0.0f;
	ikSolver->spine->bodyRotStiffness = 0.0f;
	ikSolver->spine->maintainPelvisPosition = 0.0f;

	ikSolver->spine->positionWeight = 1.0f;
	ikSolver->spine->rotationWeight = 1.0f;
	ikSolver->spine->pelvisPositionWeight = 0.0f;
	ikSolver->spine->pelvisRotationWeight = 0.0f;
	ikSolver->leftArm->positionWeight = 1.0f;
	ikSolver->leftArm->rotationWeight = 1.0f;
	ikSolver->rightArm->positionWeight = 1.0f;
	ikSolver->rightArm->rotationWeight = 1.0f;
	ikSolver->leftLeg->positionWeight = 1.0f;
	ikSolver->leftLeg->rotationWeight = 1.0f;
	ikSolver->rightLeg->positionWeight = 1.0f;
	ikSolver->rightLeg->rotationWeight = 1.0f;

	ikSolver->spine->bodyPosStiffness = 0.5f;
	ikSolver->spine->bodyRotStiffness = 0.5f;
	ikSolver->spine->maintainPelvisPosition = 0.0f;*/
}

void SetupScene::OnLoadAnimationButtonPressed(std::string path)
{
	std::list<Animator*>::iterator it = animators.begin();
	Animator* animator = *it;
	animator->SetAnimation(Animation::LoadFromPath(path));
}

void SetupScene::OnTrackerRemoved(Tracker* tracker)
{
	DeleteModel(tracker->GetModel());
	delete trackerOffsetRepresentations[tracker];
	trackerOffsetRepresentations.erase(tracker);
	modelTrackerMap.erase(tracker->GetModel());
	if (tracker == selection)
		selection = &Tracker::Invalid;
	delete tracker;
}

void SetupScene::OnTrackerSelected(Tracker* tracker)
{
	qDebug() << "on tracker sel";
	for (const auto& kv : modelTrackerMap)
		kv.second->Select(kv.second == tracker);
	selection = tracker;
}

void SetupScene::OnTrackerHovered(Tracker* tracker)
{
	for (const auto& kv : modelTrackerMap)
		kv.second->Hover(kv.second == tracker);
}

void SetupScene::ToggleDrawTrackerOffsets()
{
	ToggleDrawTrackerOffsets(!drawTrackerOffset);
}

void SetupScene::ToggleDrawSkeleton()
{
	ToggleDrawSkeleton(!drawSkeleton);
}

void SetupScene::ToggleDrawTrackerOffsets(const bool& state)
{
	drawTrackerOffset = state;

	for (const auto& kv : trackerOffsetRepresentations)
		kv.second->Toggle(drawTrackerOffset);
}

void SetupScene::ToggleDrawSkeleton(const bool& state)
{
	drawSkeleton = state;

	for (const auto& kv : transformRepresentations)
		kv.second->Toggle(drawSkeleton);
}

void SetupScene::OnProgressSliderValueChanged(float value)
{
	std::list<Animator*>::iterator it = animators.begin();
	Animator* animator = *it;
	animator->Pause();
	animator->SetNormalizedAnimationTime(value);
}

void SetupScene::TogglePlay()
{
	std::list<Animator*>::iterator it = animators.begin();
	Animator* animator = *it;

	if (animator->IsPlaying())
		animator->Pause();
	else
		animator->Play();
}

void SetupScene::OnStopButtonPressed()
{
	std::list<Animator*>::iterator it = animators.begin();
	Animator* animator = *it;
	animator->Stop();
}

void SetupScene::OnTPoseButtonPressed()
{
	std::list<Animator*>::iterator it = animators.begin();
	Animator* animator = *it;
	animator->RemoveAnimation();
}

void SetupScene::update(float dtime)
{
	handleInput();
	updateSelection();

	Scene::update(dtime);

	std::list<Animator*>::iterator it = animators.begin();
	Animator* animator = *it;
	if (animator->HasAnimation())//&& animator->IsPlaying()) doesnt resent when pressing stop
		EventManager::instance().FireEvent("SetProgressSliderValue", animator->NormalizedTime());

	SkinnedModel* sourceModel = (SkinnedModel*)findModel("MarkerMan");
	if (!sourceModel)
		return;

	sourceModel->UpdateTransformsFromJointMapping(targetTransforms);

	if (drawSkeleton)
		UpdateTransformRepresentations();
	if (drawTrackerOffset)
		UpdateTrackerOffsetRepresentations();

	SkinnedModel* targetModel = (SkinnedModel*)findModel("FinalIKMan");

	if (!targetModel)
		return;

	ikSolver->FixTransforms();
	ikSolver->Update(dtime);

	targetModel->UpdateJointMappingFromTransforms(transforms);
	targetModel->UpdateBoneAnimation();

	if (InputManager::GetKeyDown(InputManager::Keycode::Alt))
	{
		for (int jointType = 0; jointType < (int)CustomEnums::AvatarJointType::Head; jointType++)
		{
			AvatarJoint* joint = avatar->GetAvatarJoint((CustomEnums::AvatarJointType)jointType);
			qDebug() << "Joint: " << QVariant::fromValue((CustomEnums::AvatarJointType)jointType).toString();

			Vector3 forward = joint->globalOffsetMatrix.rotation() * joint->transform->Forward();
			Vector3 right = joint->globalOffsetMatrix.rotation() * joint->transform->Right();
			Vector3 up = joint->globalOffsetMatrix.rotation() * joint->transform->Up();
			qDebug() << "Forward: " << forward.toString().c_str();
			qDebug() << "Up: " << up.toString().c_str();
			qDebug() << "Right: " << right.toString().c_str();

			std::vector<CustomEnums::HumanAnatomicAngleType> angleTypes = avatar->GetAnatomicAngleTypesOfJoint(joint->humanJointType);

			for (CustomEnums::HumanAnatomicAngleType angleType : angleTypes)
			{
				float angle = avatar->GetAnatomicAngle((CustomEnums::AvatarJointType)jointType, angleType);

				qDebug() << QVariant::fromValue(angleType).toString() << " Value: " << angle;
			}

			qDebug() << "";
		}
	}
}

void SetupScene::handleInput()
{
	if (InputManager::GetKeyDown(InputManager::Keycode::Spacebar))
		placeTracker();
	if (InputManager::GetMouseButtonDown(InputManager::Mousecode::Left))
		select();
	if (dragInfo.active)
		drag();
}

const bool SetupScene::IsDragControl(BaseModel* model) const
{
	for (int i = 0; i < 4; i++)
		if (model == arrowsMove[i])
			return true;

	if (model == arrowRotate)
		return true;

	return false;
}

void SetupScene::select()
{
	if (cam.IsMoving())
		return;

	qDebug() << " select ";
	HitInfo info;
	camRayCollision(cam.farPlane(), info);

	// Return if nothing got hit
	if (!info.hit || !info.model->IsSelectable())
	{
		if (info.hit)
			qDebug() << " no hit " << info.model->getName().c_str() << " " << (info.model->IsSelectable() ? "si" : "non");

		qDebug() << selection;
		if (selection != &Tracker::Invalid)
			EventManager::instance().FireEvent("OnTrackerSelected", &Tracker::Invalid);

		//selection = NULL;
		return;
	}

	// Drag if controlarrow was hit
	if (IsDragControl(info.model))
	{
		Vector3 intersectionPos;
		Vector3 targetPos = selection->GetModel()->getGlobalTransform().translation();
		if (!ComputeDragPosition(targetPos, intersectionPos))
			return;

		dragInfo.position = intersectionPos;
		dragInfo.selection = info.model;
		dragInfo.origin = targetPos;
		dragInfo.active = true;
		return;
	}

	// Select
	//selection = info.model;

	AttachedModel* trackerModel = dynamic_cast<AttachedModel*>(info.model);
	if (trackerModel)
		EventManager::instance().FireEvent("OnTrackerSelected", modelTrackerMap[trackerModel]);
}

const bool SetupScene::ComputeDragPosition(const Vector3& position, Vector3& intersection) const
{
	Vector3 origin = cam.position();
	Vector3 dir = cam.screenPosToRay(InputManager::mousePosition.x, InputManager::mousePosition.y);
	Vector3 pos = position;
	Vector3 normal = (origin - pos).normalize();

	return origin.planeIntersection(dir, pos, normal, intersection);
}

void SetupScene::drag()
{
	//stopped dragging
	if (InputManager::GetMouseButtonUp(InputManager::Mousecode::Left))
	{
		dragInfo.active = false;
		return;
	}

	Vector3 intersectionPos;
	//new intersection invalid
	if (!ComputeDragPosition(dragInfo.origin, intersectionPos))
	{
		dragInfo.active = false;
		return;
	}

	//Matrix dragMat = dragInfo.selection->getGlobalTransform();
	//Vector3 forward = selection->getGlobalTransform().forward();
	//Vector3 moveDir = dragMat.forward().normalize();
	//Vector3 moveRight = dragMat.right().normalize();
	//AttachedModel* attachedModel = dynamic_cast<AttachedModel*>(selection);
	Matrix attachedGlobalTransform = selection->GetModel()->getGlobalTransform();

	Vector3 cursorDelta = intersectionPos - dragInfo.position;
	//float dot = cursorDelta.dot(moveDir);
	//float dist = abs(dot);
	//dist = sqrt(abs(dot));

	if (cursorDelta.lengthSquared() == 0.0f)
		return;

	if (dragInfo.selection == arrowRotate)
	{
		Vector3 origin = cam.position();
		Vector3 dir = cam.screenPosToRay(InputManager::mousePosition.x, InputManager::mousePosition.y);
		Vector3 pos = attachedGlobalTransform.translation();
		Vector3 normal = attachedGlobalTransform.up().normalize();

		Vector3 intersection;
		origin.planeIntersection(dir, pos, normal, intersection);

		Vector3 forward = (intersection - pos).normalize();
		Vector3 right = normal.cross(forward).normalize();

		Matrix attachedLocalTransform = selection->GetModel()->getTransform();
		Matrix rotation = Matrix().rotation(forward, normal, right);
		selection->GetModel()->setTransform(attachedLocalTransform.translationMatrix() * rotation * attachedLocalTransform.scaleMatrix());
		selection->UpdateOffset();
	}
	else
	{
		Vector3 forward = attachedGlobalTransform.forward();
		SkinnedModel* attachedToModel = dynamic_cast<SkinnedModel*>((BaseModel*)selection->GetModel()->getAttachedTo());
		Vector3 ray = cam.screenPosToRay(InputManager::mousePosition.x, InputManager::mousePosition.y);
		HitInfo hit;
		attachedToModel->rayCollision(cam.position(), ray, cam.farPlane(), hit);

		if (hit.hit)
		{
			selection->GetModel()->attachToMesh(attachedToModel, hit.meshID, hit.position, hit.triangleInfo, forward);
			selection->UpdateOffset();
		}
	}

	dragInfo.position = intersectionPos;
}

void SetupScene::updateSelection()
{
	bool invalidTracker = selection == &Tracker::Invalid;
	arrowRotate->Disabled(invalidTracker);
	for (int i = 0; i < 4; i++)
		arrowsMove[i]->Disabled(invalidTracker);

	if (invalidTracker)
		return;

	Matrix t = selection->GetModel()->getAnimationTransform();
	Matrix offset = Matrix().translation(Vector3::up * 0.9f);

	for (int i = 0; i < 4; i++)
		arrowsMove[i]->setTransform(t * Matrix().RotationAxis(Vector3::up, M_PI_2 * i) * offset);

	offset = offset * Matrix().translation(Vector3::up * 0.1f);
	arrowRotate->setTransform(t * offset);
}

Tracker* SetupScene::generateTracker(std::string& file, const bool& pushToModels)
{
	AttachedModelShader* shader = new AttachedModelShader();
	AttachedModel* model = new AttachedModel(file.c_str());
	model->SetSelectable(true);
	model->shader(shader, true);

	Tracker* tracker = new Tracker(model);

	if (pushToModels)
	{
		models.push_back(model);
		modelTrackerMap[model] = tracker;
		LoadTrackerOffsetRepresentation(tracker);
	}

	return tracker;
}

void SetupScene::placeTracker()
{
	qDebug() << "Place tracker";

	HitInfo info;
	camRayCollision(cam.farPlane(), info);

	// Return if nothing got hit
	if (!info.hit)
		return;

	std::string file = std::string(MODEL_DIRECTORY "tracker.dae");
	Tracker* tracker = generateTracker(file, false);
	AttachedModel* trackerModel = tracker->GetModel();
	trackerModel->setTransform(Matrix().scale(0.1f));
	trackerModel->setName("attachement");

	// Check if can be attached to mesh
	if (!trackerModel->attachToMesh(info.model, info.meshID, info.position, info.triangleInfo))
	{
		delete tracker;
		return;
	}

	modelTrackerMap[trackerModel] = tracker;
	models.push_back(trackerModel);
	LoadTrackerOffsetRepresentation(tracker);
	EventManager::instance().FireEvent("OnTrackerPlaced", tracker);
}

std::map<std::string, Tracker*> SetupScene::GetTrackers() const
{
	std::map<std::string, Tracker*> trackers;
	for (const auto& kv : modelTrackerMap)
	{
		Tracker* tracker = kv.second;
		trackers[tracker->GetSlot()] = tracker;
	}
	return trackers;
}

void SetupScene::draw()
{
	Scene::draw();
}

void SetupScene::end()
{
	Scene::end();
}
