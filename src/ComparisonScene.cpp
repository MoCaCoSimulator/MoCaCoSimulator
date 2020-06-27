#include "ComparisonScene.h"
#include "AnimatedModelShader.h"
#include "EventManager.h"
#include <QDebug>
#include <QtWidgets>
#include <tinyxml2.h>
#include <filesystem>
#include <ctime>

ComparisonScene::ComparisonScene(
	std::string modelfile,
	std::vector<BaseErrorMetric*> selectedErrorMetrics,
	std::vector<std::string> groundTruthAnimationPaths,
	std::vector<std::string> solvedAnimationPaths,
	int errorMetricsSampleRate)
	:
	modelfile(modelfile),
	selectedErrorMetrics(selectedErrorMetrics),
	groundTruthAnimationPaths(groundTruthAnimationPaths),
	solvedAnimationPaths(solvedAnimationPaths),
	errorMetricsSampleRate(errorMetricsSampleRate),
	Scene()
{
	std::function<void()> voidCallback = std::function([this]() { return this->OnStopButtonPressed(); });
	EventManager::instance().SubscribeToEvent("OnStopButtonPressed", voidCallback);
	voidCallback = std::function([this]() { return this->TogglePlay(); });
	EventManager::instance().SubscribeToEvent("OnTogglePlay", voidCallback);
	std::function<void(float)> floatCallback = std::function([this](float value) { return this->OnProgressSliderValueChanged(value); });
	EventManager::instance().SubscribeToEvent("OnProgressSliderValueChanged", floatCallback);
}

ComparisonScene::~ComparisonScene()
{
}

void ComparisonScene::start()
{
	qDebug() << "Comparision Scene start";

	Scene::start();

	sun = new DirectionalLight();
	float sunStrength = 0.75f;
	float ambientStrength = 1.0f;//0.25f;
	sun->color(Color(sunStrength, sunStrength, sunStrength));
	sun->direction(Vector3(0.2f, 0, -1));

	ShaderLightMapper::instance().addLight(sun);

	// Spawn the ground truth model
	SkinnedModelShader* shader = new SkinnedModelShader();
	shader->alpha(0.5f);
	// Red
	shader->color(Color(1, 0, 0));

	const char* modelfile_cstr = modelfile.c_str();

	SkinnedModel* groundTruthSkinnedModel = new SkinnedModel(modelfile_cstr, false);
	groundTruthSkinnedModel->shader(shader, false);
	groundTruthSkinnedModel->setTransform(Matrix().translation(0, 0, 0));
	groundTruthSkinnedModel->setName("GroundTruthModel");
	groundTruthSkinnedModel->setTransparency(TRANSPARENCY_FULL);
	models.push_back(groundTruthSkinnedModel);

	Animator* groundTruthAnimator = new Animator(*groundTruthSkinnedModel);
	animators.push_back(groundTruthAnimator);

	// Spawn the calculated avatar
	shader = new SkinnedModelShader();
	shader->alpha(0.5f);
	// Green
	shader->color(Color(0, 1, 0));

	SkinnedModel* solvedSkinnedModel = new SkinnedModel(modelfile_cstr, false);
	solvedSkinnedModel->shader(shader, false);
	solvedSkinnedModel->setTransform(Matrix().translation(0, 0, 0));
	solvedSkinnedModel->setName("CalculatedModel");
	solvedSkinnedModel->setTransparency(TRANSPARENCY_FULL);
	models.push_back(solvedSkinnedModel);

	Animator* solvedAnimator = new Animator(*solvedSkinnedModel);
	animators.push_back(solvedAnimator);

	int maxX = selectedErrorMetrics.size();
	int maxY = solvedAnimationPaths.size();
	std::vector<float> resultsMatrix = std::vector<float>(maxX * maxY);
	std::vector<std::string> columnNames;
	std::vector<std::string> rowNames;

	std::vector<AnimationResults> combinedResults;
	for (int y = 0; y < groundTruthAnimationPaths.size(); ++y)
	{
		AnimationResults results;

		Animation* groundTruthAnimation = Animation::LoadFromPath(groundTruthAnimationPaths[y]);
		Animation* solvedAnimation = Animation::LoadFromPath(solvedAnimationPaths[y]);

		if (!groundTruthAnimation || !solvedAnimation)
			continue;

		groundTruthAnimator->SetAnimation(groundTruthAnimation);
		solvedAnimator->SetAnimation(solvedAnimation);

		// Calulate sample times for the error metrics
		std::vector<float> sampleTimes;
		float animationLength = groundTruthAnimator->GetAnimationLength();
		int frameCount = animationLength * errorMetricsSampleRate;
		for (size_t i = 0; i < frameCount; i++)
			sampleTimes.push_back(animationLength * (i / (float)frameCount));

		std::string animationName = groundTruthAnimation->name;
		rowNames.push_back(animationName);
		qDebug() << "Animation: " << animationName.c_str();

		std::map<std::string, std::vector<float>> resultsMap;
		for (int x = 0; x < selectedErrorMetrics.size(); ++x)
		{
			std::vector<float> resultsVector = selectedErrorMetrics[x]->CalculateValues(groundTruthSkinnedModel, solvedSkinnedModel, groundTruthAnimator, solvedAnimator, sampleTimes);
			resultsMap[selectedErrorMetrics[x]->GetName()] = resultsVector;

			float combined = 0.0f;
			int resultCount = 0;
			for (size_t i = 0; i < resultsVector.size(); i++)
			{
				float result = resultsVector[i];
				if (std::isnan(result))
					continue;
				combined += result;
				resultCount++;
			}

			resultsMatrix[y * maxX + x] = combined / resultCount;
		}

		results.name = animationName;
		results.timestamps = sampleTimes;
		results.errorMetricsResultsMap = resultsMap;

		combinedResults.push_back(results);

		delete solvedAnimation;
		delete groundTruthAnimation;
	}

	for (int x = 0; x < selectedErrorMetrics.size(); ++x)
	{
		columnNames.push_back(selectedErrorMetrics[x]->GetName());
	}

	groundTruthAnimator->RemoveAnimation();
	solvedAnimator->RemoveAnimation();

	qDebug() << "END CALCULATION";

	SaveErrorMetricResults(combinedResults);

	ResultsMatrix matrix = ResultsMatrix(rowNames, columnNames, resultsMatrix, maxX, combinedResults.size());
	EventManager::instance().FireEvent("OnMatrixCalculated", matrix);
}

void ComparisonScene::OnProgressSliderValueChanged(float value)
{
	std::list<Animator*>::iterator it = animators.begin();
	Animator* animator = *it;
	animator->Pause();
	animator->SetNormalizedAnimationTime(value);
	it++;
	animator = *it;
	animator->Pause();
	animator->SetNormalizedAnimationTime(value);
}

void ComparisonScene::OnStopButtonPressed()
{
	std::list<Animator*>::iterator it = animators.begin();
	Animator* animator = *it;
	animator->Stop();
	it++;
	animator = *it;
	animator->Stop();
}

void ComparisonScene::TogglePlay()
{
	std::list<Animator*>::iterator it = animators.begin();
	Animator* animator = *it;
	if (animator->IsPlaying())
	{
		animator->Pause();
		it++;
		animator = *it;
		animator->Pause();
	}
	else
	{
		animator->Play();
		it++;
		animator = *it;
		animator->Play();
	}
}

void ComparisonScene::LoadAnimations(int index)
{
	Animation* groundTruthAnimation = Animation::LoadFromPath(groundTruthAnimationPaths[index]);
	Animation* solvedAnimation = Animation::LoadFromPath(solvedAnimationPaths[index]);
	std::list<Animator*>::iterator it = animators.begin();
	Animator* animator = *it;
	animator->RemoveAnimation(true);
	animator->SetAnimation(groundTruthAnimation);
	it++;
	animator = *it;
	animator->RemoveAnimation(true);
	animator->SetAnimation(solvedAnimation);
}

void ComparisonScene::update(float dtime)
{
	Scene::update(dtime);

	std::list<Animator*>::iterator it = animators.begin();
	Animator* animator = *it;
	if (animator->HasAnimation())
		EventManager::instance().FireEvent("SetProgressSliderValue", animator->NormalizedTime());
}

void ComparisonScene::draw()
{
	Scene::draw();
}

void ComparisonScene::end()
{
	Scene::end();
}

void ComparisonScene::SaveErrorMetricResults(std::vector<AnimationResults> combinedResults)
{
	namespace tx = tinyxml2;
	tx::XMLDocument doc;

	// XML Declaration
	auto decl = doc.NewDeclaration();
	doc.InsertFirstChild(decl);

	tx::XMLElement* root = doc.NewElement("Workbook");
	// Identify as an xml excel file
	root->SetAttribute("xmlns", "urn:schemas-microsoft-com:office:spreadsheet");
	root->SetAttribute("xmlns:o", "urn:schemas-microsoft-com:office:office");
	root->SetAttribute("xmlns:x", "urn:schemas-microsoft-com:office:excel");
	root->SetAttribute("xmlns:ss", "urn:schemas-microsoft-com:office:spreadsheet");
	root->SetAttribute("xmlns:html", "http://www.w3.org/TR/REC-html40");

	doc.InsertEndChild(root);

	for (size_t animationIndex = 0; animationIndex < combinedResults.size(); animationIndex++)
	{
		AnimationResults result = combinedResults[animationIndex];

		tx::XMLElement* sheet = doc.NewElement("Worksheet");
		sheet->SetAttribute("ss:Name", result.name.c_str());

		tx::XMLElement* table = doc.NewElement("Table");

		// Headerrow
		tx::XMLElement* headerRow = doc.NewElement("Row");
		tx::XMLElement* cell = doc.NewElement("Cell");
		tx::XMLElement* data = doc.NewElement("Data");
		data->SetAttribute("ss:Type", "String");
		data->SetText("Time");
		cell->InsertEndChild(data);
		headerRow->InsertEndChild(cell);
		for (auto resultMap : result.errorMetricsResultsMap)
		{
			tx::XMLElement* cell = doc.NewElement("Cell");
			tx::XMLElement* data = doc.NewElement("Data");
			data->SetAttribute("ss:Type", "String");
			data->SetText(resultMap.first.c_str());
			cell->InsertEndChild(data);
			headerRow->InsertEndChild(cell);
		}
		table->InsertEndChild(headerRow);

		for (size_t i = 0; i < result.timestamps.size(); i++)
		{
			tx::XMLElement* row = doc.NewElement("Row");

			tx::XMLElement* cell = doc.NewElement("Cell");
			tx::XMLElement* data = doc.NewElement("Data");
			data->SetAttribute("ss:Type", "Number");
			data->SetText(result.timestamps[i]);
			cell->InsertEndChild(data);
			row->InsertEndChild(cell);

			for (auto resultMap : result.errorMetricsResultsMap)
			{
				tx::XMLElement* cell = doc.NewElement("Cell");
				tx::XMLElement* data = doc.NewElement("Data");
				data->SetAttribute("ss:Type", "Number");
				data->SetText(resultMap.second[i]);
				cell->InsertEndChild(data);
				row->InsertEndChild(cell);
			}

			table->InsertEndChild(row);
		}
		
		sheet->InsertEndChild(table);
		root->InsertEndChild(sheet);
	}

	std::filesystem::path cwd = std::filesystem::current_path();
	std::string dirName = cwd.string() + "/results";

	if (!std::filesystem::exists(dirName))
		std::filesystem::create_directory(dirName);

	time_t seconds = std::time(nullptr);
	std::stringstream filenameSS;
	filenameSS << dirName << "/results_" << seconds << ".xml";

	qDebug() << "Saving results to path: " << filenameSS.str().c_str();

	tx::XMLError errorCode = doc.SaveFile(filenameSS.str().c_str());
	if (errorCode == 0)
		qDebug() << "Results save was a success";
	else
		qDebug() << "Results save failed with state " << errorCode;
}