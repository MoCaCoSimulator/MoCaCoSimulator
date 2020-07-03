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

	Avatar* groundTruthAvatar = new Avatar(groundTruthSkinnedModel);

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

	Avatar* solvedAvatar = new Avatar(solvedSkinnedModel);

	Animator* solvedAnimator = new Animator(*solvedSkinnedModel);
	animators.push_back(solvedAnimator);

	int maxX = selectedErrorMetrics.size();
	int maxY = solvedAnimationPaths.size();
	std::vector<float> resultsMatrix = std::vector<float>(maxX * maxY);
	std::vector<std::string> columnNames;
	std::vector<std::string> rowNames;
	std::vector<std::string> finalSolvedAnimationPaths;
	std::vector<std::string> finalTruthAnimationPaths;

	std::vector<AnimationResults> combinedResults;
	int row = 0;
	for (int y = 0; y < groundTruthAnimationPaths.size(); ++y)
	{
		AnimationResults results;

		Animation* solvedAnimation = Animation::LoadFromPath(solvedAnimationPaths[y]);
		if (!solvedAnimation)
			continue;
		Animation* groundTruthAnimation = Animation::LoadFromPath(groundTruthAnimationPaths[y]);
		if (!groundTruthAnimation || !solvedAnimation)
			continue;

		finalSolvedAnimationPaths.push_back(solvedAnimationPaths[y]);
		finalTruthAnimationPaths.push_back(groundTruthAnimationPaths[y]);

		groundTruthAnimator->SetAnimation(groundTruthAnimation);
		solvedAnimator->SetAnimation(solvedAnimation);

		bool velocitiesNeeded = false;
		bool accelerationsNeeded = false;
		for (int a = 0; a < selectedErrorMetrics.size(); ++a)
		{
			if (selectedErrorMetrics[a]->needsVelocities)
				velocitiesNeeded = true;
			if (selectedErrorMetrics[a]->needsAccelerations)
				accelerationsNeeded = true;
			if (velocitiesNeeded && accelerationsNeeded)
				break;
		}

		// Calulate sample times for the error metrics
		std::vector<float> sampleTimes;
		std::map<std::string, Vector3> prevGroundTruthPositions;
		std::map<std::string, Vector3> prevGroundTruthVelocities;
		std::map<std::string, Vector3> prevSolvedPositions;
		std::map<std::string, Vector3> prevSolvedVelocities;
		float animationLength = groundTruthAnimator->GetAnimationLength();
		int frameCount = animationLength * errorMetricsSampleRate;
		std::map<std::string, std::vector<float>> resultsMap;
		for (size_t i = 0; i < frameCount; i++)
		{
			float sampleTime = animationLength * (i / (float)frameCount);
			sampleTimes.push_back(sampleTime);

			BaseErrorMetric::Pose groundTruthPose;
			BaseErrorMetric::Pose solvedPose;

			groundTruthPose.skinnedModel = groundTruthSkinnedModel;
			solvedPose.skinnedModel = solvedSkinnedModel;

			groundTruthPose.avatar = groundTruthAvatar;
			solvedPose.avatar = solvedAvatar;

			groundTruthPose.velocities.clear();
			groundTruthPose.accelerations.clear();
			solvedPose.velocities.clear();
			solvedPose.accelerations.clear();

			groundTruthAnimator->SetNormalizedAnimationTime(sampleTime / animationLength);
			solvedAnimator->SetNormalizedAnimationTime(sampleTime / animationLength);

			if (velocitiesNeeded || accelerationsNeeded)
			{
				std::map<std::string, Vector3> groundTruthVelocities = std::map<std::string, Vector3>();
				std::map<std::string, Vector3> solvedVelocities = std::map<std::string, Vector3>();

				// Calculate velocity
				if (i > 0 && (velocitiesNeeded || accelerationsNeeded))
				{
					for (std::pair<std::string, MeshModel::JointInfo> pair : groundTruthSkinnedModel->GetJointMapping())
						groundTruthVelocities[pair.first] = pair.second.transform.translation() - prevGroundTruthPositions[pair.first];

					groundTruthPose.velocities = groundTruthVelocities;

					for (std::pair<std::string, MeshModel::JointInfo> pair : solvedSkinnedModel->GetJointMapping())
						solvedVelocities[pair.first] = pair.second.transform.translation() - prevSolvedPositions[pair.first];

					solvedPose.velocities = solvedVelocities;
				}

				// Calculate acceleration
				if (i > 1 && accelerationsNeeded)
				{
					std::map<std::string, Vector3> groundTruthAccelerations = std::map<std::string, Vector3>();
					std::map<std::string, Vector3> solvedAccelerations = std::map<std::string, Vector3>();

					for (std::pair<std::string, MeshModel::JointInfo> pair : groundTruthSkinnedModel->GetJointMapping())
						groundTruthAccelerations[pair.first] = groundTruthVelocities[pair.first] - prevGroundTruthVelocities[pair.first];

					groundTruthPose.accelerations = groundTruthAccelerations;

					for (std::pair<std::string, MeshModel::JointInfo> pair : solvedSkinnedModel->GetJointMapping())
						solvedAccelerations[pair.first] = solvedVelocities[pair.first] - prevGroundTruthVelocities[pair.first];

					solvedPose.accelerations = solvedAccelerations;
				}

				for (std::pair<std::string, MeshModel::JointInfo> pair : groundTruthSkinnedModel->GetJointMapping())
					prevGroundTruthPositions[pair.first] = pair.second.transform.translation();

				for (std::pair<std::string, MeshModel::JointInfo> pair : solvedSkinnedModel->GetJointMapping())
					prevSolvedPositions[pair.first] = pair.second.transform.translation();

				prevGroundTruthVelocities = groundTruthVelocities;
				prevSolvedVelocities = solvedVelocities;
			}

			for (int x = 0; x < selectedErrorMetrics.size(); ++x)
			{
				std::string metricName = dynamic_cast<Parameter<std::string>*>(selectedErrorMetrics[x]->GetParameters().at("Name"))->GetValue();

				if (metricName == "")
					metricName = "ErrorMetric " + x;

				float result;
				bool success = selectedErrorMetrics[x]->CalculateDifference(groundTruthPose, solvedPose, result);
				if (success)
					resultsMap[metricName].push_back(result);
				else
					resultsMap[metricName].push_back(NAN);
			}
		}

		std::string animationName = groundTruthAnimation->name;
		qDebug() << animationName.c_str();
		rowNames.push_back(animationName);

		for (int x = 0; x < selectedErrorMetrics.size(); ++x)
		{
			std::string metricName = dynamic_cast<Parameter<std::string>*>(selectedErrorMetrics[x]->GetParameters().at("Name"))->GetValue();

			if (metricName == "")
				metricName = "ErrorMetric " + x;

			float combined = 0.0f;
			int resultCount = 0;
			for (size_t i = 0; i < resultsMap[metricName].size(); i++)
			{
				float result = resultsMap[metricName][i];
				if (std::isnan(result))
					continue;
				combined += result;
				resultCount++;
			}

			resultsMatrix[row * maxX + x] = combined / resultCount;
		}

		results.name = animationName;
		results.timestamps = sampleTimes;
		results.errorMetricsResultsMap = resultsMap;

		combinedResults.push_back(results);

		//removeanimation handles animation destruction
		groundTruthAnimator->RemoveAnimation(true);
		solvedAnimator->RemoveAnimation(true);

		row++;
	}

	solvedAnimationPaths = finalSolvedAnimationPaths;
	groundTruthAnimationPaths = finalTruthAnimationPaths;

	for (int x = 0; x < selectedErrorMetrics.size(); ++x)
	{
		std::string metricName = dynamic_cast<Parameter<std::string>*>(selectedErrorMetrics[x]->GetParameters().at("Name"))->GetValue();
		qDebug() << metricName.c_str();
		columnNames.push_back(metricName);
	}

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
		sheet->SetAttribute("ss:Name", animationIndex);//result.name.c_str());

		tx::XMLElement* table = doc.NewElement("Table");

		// Namerow
		tx::XMLElement* nameRow = doc.NewElement("Row");
		tx::XMLElement* cell = doc.NewElement("Cell");
		tx::XMLElement* data = doc.NewElement("Data");
		data->SetAttribute("ss:Type", "String");
		data->SetText(result.name.c_str());
		cell->InsertEndChild(data);
		nameRow->InsertEndChild(cell);
		table->InsertEndChild(nameRow);

		// Headerrow
		tx::XMLElement* headerRow = doc.NewElement("Row");
		cell = doc.NewElement("Cell");
		data = doc.NewElement("Data");
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