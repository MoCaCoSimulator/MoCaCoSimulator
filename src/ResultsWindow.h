#pragma once

#include <QWidget>
#include "ui_ResultsWindow.h"
#include "ComparisonScene.h"
#include "Customizable/ErrorMetrics/BaseErrorMetric.h"

class ResultsWindow : public QWidget
{
	Q_OBJECT
private:
	ComparisonScene::ResultsMatrix resultsMatrix;
	int currentIndex;
	ComparisonScene* comparisonScene;

	void SetProgressSliderValue(float normalizedValue);
public:
	ResultsWindow(QWidget* parent, std::string modelfile, std::vector<BaseErrorMetric*> selectedErrorMetrics, std::vector<std::string> groundTruthAnimationPaths, std::vector<std::string> solvedAnimationPaths, int errorMetricsSampleRate);
	~ResultsWindow();
	Ui::ResultsWindow ui;

	void OnMatrixCalculated(ComparisonScene::ResultsMatrix resultsMatrix);
public slots:
	void CellSelected(int row, int column);
	void HeaderSelected(int row);
};
