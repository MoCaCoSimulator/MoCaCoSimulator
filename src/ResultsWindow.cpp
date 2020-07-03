#include "ResultsWindow.h"
#include "Qt\OpenGLWindow.h"
#include "ComparisonScene.h"
#include "EventManager.h"

#include <iostream>
#include <fstream>
#include <sstream>

ResultsWindow::ResultsWindow(
	QWidget *parent, 
	std::string modelfile,
	std::vector<BaseErrorMetric*> selectedErrorMetrics,
	std::vector<std::string> groundTruthAnimationPaths, 
	std::vector<std::string> solvedAnimations,
	int errorMetricsSampleRate)
	:
	QWidget(parent),
	currentIndex(-1)
{
	ui.setupUi(this);

	// Set callback for selections on header
	QHeaderView* header = ui.resultsTable->verticalHeader();
	connect(header, SIGNAL(sectionClicked(int)), this, SLOT(HeaderSelected(int)));

	std::function<void(float)> floatFunction = std::function([this](float value) { return SetProgressSliderValue(value); });
	EventManager::instance().SubscribeToEvent("SetProgressSliderValue", floatFunction);
	std::function<void(ComparisonScene::ResultsMatrix)> matrixFunction = std::function([this](ComparisonScene::ResultsMatrix value) { return OnMatrixCalculated(value); });
	EventManager::instance().SubscribeToEvent("OnMatrixCalculated", matrixFunction);

	OpenGLWindow* openGLWindow = static_cast<OpenGLWindow*>(ui.openGLWindow);
	comparisonScene = new ComparisonScene(modelfile,selectedErrorMetrics, groundTruthAnimationPaths ,solvedAnimations, errorMetricsSampleRate);
	openGLWindow->SetCurrentScene(comparisonScene);

}

ResultsWindow::~ResultsWindow() 
{
}

void ResultsWindow::OnMatrixCalculated(ComparisonScene::ResultsMatrix resultsMatrix)
{
	this->resultsMatrix = resultsMatrix;

	QTableWidget* table = ui.resultsTable;
	table->setRowCount(resultsMatrix.y);
	table->setColumnCount(resultsMatrix.x);

	for (int x = 0; x < resultsMatrix.x; x++)
	{
		std::string name = resultsMatrix.columnLabels[x];
		QTableWidgetItem* item = new QTableWidgetItem(name.c_str());
		table->setHorizontalHeaderItem(x, item);
	}

	for (int y = 0; y < resultsMatrix.y; y++)
	{
		std::string name = resultsMatrix.rowLabels[y];
		QTableWidgetItem* item = new QTableWidgetItem(name.c_str());
		table->setVerticalHeaderItem(y, item);
	}

	for (int x = 0; x < resultsMatrix.x; x++)
	{
		for (int y = 0; y < resultsMatrix.y; y++)
		{
			float meanResult = resultsMatrix.matrix[y * resultsMatrix.x + x];
			QTableWidgetItem* item = new QTableWidgetItem(QString::number(meanResult));
			table->setItem(y, x, item);
		}
	}
}

void ResultsWindow::HeaderSelected(int row)
{
	qDebug() << "Header selected";

	if (currentIndex == row)
		return;

	comparisonScene->LoadAnimations(row);

	currentIndex = row;
}

void ResultsWindow::CellSelected(int row, int column)
{
	qDebug() << "Cell selected: " << row;

	if (currentIndex == row)
		return;

	comparisonScene->LoadAnimations(row);

	currentIndex = row;
}

void ResultsWindow::SetProgressSliderValue(float normalizedValue)
{
	int realValue = ui.progressSlider->maximum() * normalizedValue;

	// Stop slider from invoking OnValueChanged
	ui.progressSlider->blockSignals(true);
	ui.progressSlider->setValue(realValue);
	ui.progressSlider->blockSignals(false);
}