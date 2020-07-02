#include "Qt/OpenGLWindow.h"
#include "MainWindow.h"
#include "EventManager.h"
#include <QDebug>
#include "Customizable/TrackingVirtualizers/BaseTrackingVirtualizer.h"
#include "Customizable/InverseKinematicsKernels/BaseIKKernel.h"
#include "Customizable/ErrorMetrics/BaseErrorMetric.h"
#include "ResultsWindow.h"
#include <filesystem>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
	qDebug("Init: Mainwindow");

	ui.setupUi(this);

	// Fill the dropdowns with values
	QComboBox* comboBox = ui.choosableMetricsComboBox;
	for (const BaseErrorMetric* possibleVirtualizer : BaseErrorMetric::registry())
		comboBox->addItem(possibleVirtualizer->GetName().c_str());

	comboBox = ui.ikKernelComboBox;
	for (BaseIKKernel* possibleVirtualizer : BaseIKKernel::registry())
		comboBox->addItem(possibleVirtualizer->GetName().c_str());

	std::function<void(float)> floatCallback = std::function([this](float value) { return SetProgressSliderValue(value); });
	EventManager::instance().SubscribeToEvent("SetProgressSliderValue", floatCallback);
	std::function<void(std::string)> stringCallback = std::function([this](std::string value) { return this->OnLoadCharacterButtonPressed(value); });
	EventManager::instance().SubscribeToEvent("OnLoadCharacterButtonPressed", stringCallback);

	OpenGLWindow* openGLWindow = static_cast<OpenGLWindow*>(ui.openGLWindow);
	setupScene = new SetupScene();
	openGLWindow->SetCurrentScene(setupScene);
}

void MainWindow::OnVirtualizerDropdownValueChanged(int value)
{

}

MainWindow::~MainWindow()
{
	qDebug() << "DESTRUCTOR CALLED";
}

bool MainWindow::GenerateTrackingVirtualizerAnimations(Animator& animator, const Animation& groundTruthAnimation, std::map<std::string, AnimationCurve>& result)
{
	// Create tracking virtualizer animations
	for (TrackingVirtualizerListItem* widget : ui.trackerList->itemWidgets)
	{
		animator.GetModel()->SetDefaultPose();
		BaseTrackingVirtualizer* virtualizerToUse = widget->GetVirtualizer();

		TrackerHandle trackerHandle = TrackerHandle(&animator, widget->tracker, widget->solveSlotComboBox->currentText().toStdString());

		std::string solveSlotName = widget->solveSlotComboBox->currentText().toStdString();
		qDebug() << "Solveslot name: " << solveSlotName.c_str();
		AnimationCurve trackerAnimationCurve;
		bool success = virtualizerToUse->CreateOutputAnimation(trackerHandle, trackerAnimationCurve);
		if (success)
		{
			trackerAnimationCurve.name = solveSlotName;
			result[solveSlotName] = trackerAnimationCurve;
		}
		else
		{
			std::stringstream ss;
			ss << virtualizerToUse->GetName().c_str() << " could not create a tracker animation for " << solveSlotName.c_str() << " of " << groundTruthAnimation.name.c_str() << ". No further calculation possible";
			qDebug() << ss.str().c_str();

			/*QMessageBox msgBox;
			msgBox.setText(ss.str().c_str());
			msgBox.exec();*/

			return false;
		}
	}

	return true;
}

Animation* MainWindow::CombineTrackerAnimations(const Animation& groundTruthAnimation, const std::map<std::string, AnimationCurve>& trackerAnimations)
{
	Animation* combinedAnimation = new Animation();
	for (auto& trackerAnimation : trackerAnimations)
		combinedAnimation->animNodeMapping.insert(trackerAnimation);
	combinedAnimation->name = groundTruthAnimation.name;
	combinedAnimation->duration = groundTruthAnimation.duration;
	combinedAnimation->ticksPerSecond = groundTruthAnimation.ticksPerSecond;
	return combinedAnimation;
}

void MainWindow::SkipIKSolver(const std::string& affix, std::string& modelfile, std::vector<std::string>& solvedAnimationPaths, std::vector<std::string>& truthAnimationPaths)
{
	std::vector<std::string> animationPaths = ui.animationList->GetSelectedAnimationPaths();

	SetupScene* currentScene = dynamic_cast<SetupScene*>(ui.openGLWindow->GetCurrentScene());
	if (!currentScene)
		return;

	Animator* animator = currentScene->GetAnimator();
	SkinnedModel* model = animator->GetModel();
	modelfile = model->getPath();

	int counter = 0;
	std::string dir = "";
	for (std::string path : animationPaths)
	{
		Animation* groundTruthAnimation = Animation::LoadFromPath(path);

		if (counter == 0)
			dir = groundTruthAnimation->path + "../animations_solved_" + affix + "/";

		std::string solvedPath = dir + groundTruthAnimation->filename;
		solvedAnimationPaths.push_back(solvedPath);
		truthAnimationPaths.push_back(path);
	}
}

void MainWindow::GenerateAnimations(std::string& modelfile, std::vector<std::string>& solvedAnimationPaths, std::vector<std::string>& truthAnimationPaths)
{
	qDebug() << "Generate Animations";
	std::vector<std::string> animationPaths = ui.animationList->GetSelectedAnimationPaths();
	SetupScene* currentScene = dynamic_cast<SetupScene*>(ui.openGLWindow->GetCurrentScene());
	if (!currentScene)
		return;

	Animator* animator = currentScene->GetAnimator();
	SkinnedModel* model = animator->GetModel();
	modelfile = model->getPath();
	std::map<std::string, Tracker*> trackers = currentScene->GetTrackers();

	int numFiles = animationPaths.size();
	QProgressDialog progress("Starting comparision process..", "Abort", 0, numFiles, this);
	progress.setWindowModality(Qt::WindowModal);


	BaseIKKernel* usedKernel = BaseIKKernel::registry()[ui.ikKernelComboBox->currentIndex()];

	int counter = 0;
	std::string dir = "";
	for (std::string path : animationPaths)
	{
		// Load ground truth animation
		Animation* groundTruthAnimation = Animation::LoadFromPath(path);
		animator->SetAnimation(groundTruthAnimation);

		if (counter == 0)
		{
			time_t seconds = time(nullptr);
			std::stringstream ss;
			ss << seconds;
			std::string ts = ss.str();

			dir = groundTruthAnimation->path + "../animations_solved_" + ts + "/";
			if (!std::filesystem::exists(dir))
				std::filesystem::create_directory(dir);
		}

		std::stringstream ss;
		ss << "Animation " << counter << "/" << numFiles << ": " << groundTruthAnimation->name;
		progress.setLabelText(ss.str().c_str());
		progress.setValue(counter);
		ss.clear();

		counter++;

		if (progress.wasCanceled())
			break;

		animator->SetAnimation(groundTruthAnimation);

		qDebug() << "Generate Tracker Animations";
		std::map<std::string, AnimationCurve> trackerCurves;
		bool success = GenerateTrackingVirtualizerAnimations(*animator, *groundTruthAnimation, trackerCurves);

		if (!success)
		{
			animator->RemoveAnimation(true);
			continue;
		}

		qDebug() << "Combine Tracker Animations";
		Animation* trackerAnimation = CombineTrackerAnimations(*groundTruthAnimation, trackerCurves);
		trackerCurves.clear();
		qDebug() << "Solve Animation";

		// Let the IK solver do its job
		animator->GetModel()->SetDefaultPose();
		Animation* solvedAnimation = usedKernel->Solve(*groundTruthAnimation, trackers, *model, *trackerAnimation);

		std::string solvedPath = dir + groundTruthAnimation->filename;
		Animation::SaveToPath(path, *solvedAnimation, solvedPath);

		// Delete ground truth and solved animation from memory
		animator->RemoveAnimation(true); //handles gt destruction
		delete solvedAnimation;

		solvedAnimationPaths.push_back(solvedPath);
		truthAnimationPaths.push_back(path);
	}

	progress.setValue(numFiles);
}

void MainWindow::OnCalculationButtonPressed()
{
	std::vector<std::string> animationPaths = ui.animationList->GetSelectedAnimationPaths();
	std::vector<std::string> truthAnimationPaths;
	std::vector<std::string> solvedAnimationPaths;
	std::string characterModel;

	GenerateAnimations(characterModel, solvedAnimationPaths, truthAnimationPaths);
	//SkipIKSolver("dances_10", characterModel, solvedAnimationPaths, truthAnimationPaths);

	ErrorMetricList* errorList = static_cast<ErrorMetricList*>(ui.errorMetricsList);

	std::vector<BaseErrorMetric*> selectedErrorMetrics;
	for (auto widget : errorList->itemWidgets)
		selectedErrorMetrics.push_back(widget->errorMetric);

	int errorMetricsSampleRate = ui.errorMetricsSampleRateSpinBox->value();

	ResultsWindow* rw = new ResultsWindow(nullptr, characterModel, selectedErrorMetrics, truthAnimationPaths, solvedAnimationPaths, errorMetricsSampleRate);

	rw->show();
	this->close();
}

void MainWindow::ValidateCalculationButton()
{
	bool isValid = true;

	// Check if the tracker list is filled properly
	TrackingVirtualizerList* trackerList = static_cast<TrackingVirtualizerList*>(ui.trackerList);
	if (trackerList->GetMissingSlots() > 0)
		isValid = false;

	// Check if at least one animation was selected for processing
	if (ui.animationList->selectedItems().size() == 0)
		isValid = false;

	// Check if at least one error metric was selected for processing
	if (ui.errorMetricsList->count() == 0)
		isValid = false;

	if (isValid)
		ui.runSimulationButton->setDisabled(false);
	else
		ui.runSimulationButton->setDisabled(true);
}

QJsonDocument MainWindow::loadJson(QString fileName) {
	QFile jsonFile(fileName);
	jsonFile.open(QFile::ReadOnly);
	return QJsonDocument().fromJson(jsonFile.readAll());
}

void MainWindow::saveJson(QJsonDocument document, QString fileName) {
	QFile jsonFile(fileName);
	jsonFile.open(QFile::WriteOnly);
	jsonFile.write(document.toJson());
}

void MainWindow::OnSaveLayoutButtonPressed()
{
	QJsonObject json;
	QJsonDocument jsonDocument;

	//Read avatar model
	json.insert("character", ui.characterList->SaveSelected());// SaveAvatar(ui.openGLWindow->GetCurrentScene()));
	//Read Selected Animations
	json.insert("animations", ui.animationList->SaveSelected());
	//Read IK Kernel
	json.insert("ikkernel", ui.ikKernelOptionsList->SaveSettings());
	//Read IK Options
	json.insert("trackerList", ui.trackerList->SaveTrackers());
	//Read ErrorMetrics
	json.insert("metricList", ui.errorMetricsList->SaveSettings());

	//Save
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Layout"), "layouts", tr("JSON files (*.json)"));
	jsonDocument.setObject(json);
	saveJson(jsonDocument, fileName);
}

void MainWindow::OnLoadLayoutButtonPressed()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Load Layout"), "layouts", tr("JSON files (*.json)"));

	ui.openGLWindow->makeCurrent();
	QJsonDocument jsonDocument = loadJson(fileName);
	QJsonObject json = jsonDocument.object();
	Scene* scene = ui.openGLWindow->GetCurrentScene();
	//Check avatar
	//SkinnedModel* model = LoadAvatar(json["avatar"].toObject(), scene);
	ui.characterList->LoadSelected(json["character"].toObject());
	//Read Selected Animations
	ui.animationList->LoadSelected(json["animations"].toObject());
	//Read IK Kernel
	ui.ikKernelOptionsList->LoadSettings(json["ikkernel"].toObject(), ui.ikKernelComboBox);
	//Read IK Options
	ui.trackerList->LoadTrackers(json["trackerList"].toObject(), *scene);
	//Read ErrorMetrics
	ui.errorMetricsList->LoadSettings(json["metricList"].toObject());
	ui.openGLWindow->doneCurrent();
}

void MainWindow::ReloadUI()
{
	//ui.animationList->clear(); (must be) done within animationlist class
	ui.ikKernelComboBox->setCurrentIndex(0);
	ui.trackerList->Reset();
	ui.errorMetricsList->Reset();
}

void MainWindow::OnLoadCharacterButtonPressed(std::string path)
{
	qDebug() << "set opengl context";
	ui.openGLWindow->makeCurrent();
	qDebug() << "get scene";
	SetupScene* scene = dynamic_cast<SetupScene*>(ui.openGLWindow->GetCurrentScene());
	if (!scene)
		return;
	qDebug() << "reload UI";
	ReloadUI();
	qDebug() << "reload scene";
	scene->ReloadScene(scene->FindCharacterFileInPath(path));
}

void MainWindow::SetProgressSliderValue(float normalizedValue)
{
	int realValue = ui.progressSlider->maximum() * normalizedValue;

	// Stop slider from invoking OnValueChanged
	ui.progressSlider->blockSignals(true);
	ui.progressSlider->setValue(realValue);
	ui.progressSlider->blockSignals(false);
}
