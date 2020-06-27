#pragma once

#include "Scene.h"
#include "Customizable/ErrorMetrics/BaseErrorMetric.h"

class ComparisonScene : public Scene
{
public:
	struct ResultsMatrix
	{
		ResultsMatrix() : x(0), y(0), matrix(NULL) {}
		ResultsMatrix(std::vector<std::string> rowLabels, std::vector<std::string> columnLabels, std::vector<float> matrix, int x, int y) : rowLabels(rowLabels), columnLabels(columnLabels), matrix(matrix), x(x), y(y) {}
		std::vector<std::string> rowLabels;
		std::vector<std::string> columnLabels;
		std::vector<float> matrix;
		int x;
		int y;
	};

	struct AnimationResults
	{
		std::string name;
		std::vector<float> timestamps;
		std::map<std::string, std::vector<float>> errorMetricsResultsMap;
	};

private:
	std::vector<BaseErrorMetric*> selectedErrorMetrics;
	std::vector<std::string> groundTruthAnimationPaths;
	std::vector<std::string> solvedAnimationPaths;
	std::string modelfile = "";
	int errorMetricsSampleRate = 0;

	void SaveErrorMetricResults(std::vector<AnimationResults> combinedResults);
public:
	ComparisonScene(std::string modelfile,std::vector<BaseErrorMetric*> selectedErrorMetrics, std::vector<std::string> groundTruthAnimationPaths, std::vector<std::string> solvedAnimationPaths, int errorMetricsSampleRate);
	~ComparisonScene();

	virtual void start();
	virtual void update(float dtime);
	virtual void draw();
	virtual void end();

	void OnProgressSliderValueChanged(float value);
	//void OnPlayButtonPressed();
	//void OnPauseButtonPressed();
	void OnStopButtonPressed();
	void TogglePlay();

	void LoadAnimations(int index);
};