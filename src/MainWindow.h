#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MainWindow.h"
#include "SetupScene.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow(QWidget *parent = Q_NULLPTR);
	~MainWindow();
	Ui::MainWindowClass GetUi() { return ui; }
protected:
	//void closeEvent(QCloseEvent* event) override;

public slots:
	void OnVirtualizerDropdownValueChanged(int value);
	void OnCalculationButtonPressed();
	void ValidateCalculationButton();
	void OnSaveLayoutButtonPressed();
	void OnLoadLayoutButtonPressed();

	void ReloadUI();

private:
	SetupScene* setupScene;

	void OnLoadCharacterButtonPressed(std::string path);
	void SetProgressSliderValue(float normalizedValue);
	Ui::MainWindowClass ui;
	void saveJson(QJsonDocument document, QString fileName);
	QJsonDocument loadJson(QString fileName);
	bool GenerateTrackingVirtualizerAnimations(Animator& animator, const Animation& groundTruthAnimation, std::map<std::string, AnimationCurve>& output);
	Animation* CombineTrackerAnimations(const Animation& groundTruthAnimation, const std::map<std::string, AnimationCurve>& trackerAnimations);
	void SkipIKSolver(const std::string& affix, std::string& modelfile, std::vector<std::string>& solvedAnimationPaths, std::vector<std::string>& truthAnimationPaths);
	void GenerateAnimations(std::string& model, std::vector<std::string>& solvedAnimationPaths, std::vector<std::string>& truthAnimationPaths);
};