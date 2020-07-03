#include "TrackingVirtualizerListWidget.h"
#include "../EventManager.h"
#include "../QJsonSerializer.h"
#include "../Scene.h"
#include "../AttachedModelShader.h"
#include "../SetupScene.h"
#include "TrackingVirtualizerList.h"

TrackingVirtualizerListItem::TrackingVirtualizerListItem(QWidget* parent, Tracker* tracker) :
	ParameterListWidget(parent),
	solveSlotComboBox(nullptr),
	prevSlotName("None"),
	tracker(tracker),
	selectedVirtualizer(nullptr)
{
	QGridLayout* layout = new QGridLayout();

	trackingVirtualizerComboBox = new QComboBox();
	std::vector<const BaseTrackingVirtualizer*> possibleVirtualizers = BaseTrackingVirtualizer::registry();
	for each (const BaseTrackingVirtualizer* possibleVirtualizer in possibleVirtualizers)
		trackingVirtualizerComboBox->addItem(possibleVirtualizer->GetName().c_str());
	layout->addWidget(new QLabel("Virtualizer / Slot"), 0, 0, 1, 1);
	layout->addWidget(trackingVirtualizerComboBox, 0, 1, 1, 1);
	solveSlotComboBox = new QComboBox();
	layout->addWidget(solveSlotComboBox, 0, 2, 1, 1);
	QPushButton* deleteButton = new QPushButton("Delete");
	layout->addWidget(deleteButton, 0, 3, 1, 1);

	/*	
	offsetWidgets = new QLineEdit*[6];
	QLineEdit** widgets;

	offsetPos = new Parameter<Vector3>("offsetPos", Vector3::zero);
	widgets =  GenerateVectorLineBoxes(offsetPos);
	layout->addWidget(new QLabel("Offset Position"), 1, 0, 1, 4);
	layout->addWidget(widgets[0], 1, 1, 1, 1);
	connect(widgets[0], &QLineEdit::textChanged, this, &TrackingVirtualizerListItem::OnOffsetChanged);
	layout->addWidget(widgets[1], 1, 2, 1, 1);
	connect(widgets[1], &QLineEdit::textChanged, this, &TrackingVirtualizerListItem::OnOffsetChanged);
	layout->addWidget(widgets[2], 1, 3, 1, 1);
	connect(widgets[1], &QLineEdit::textChanged, this, &TrackingVirtualizerListItem::OnOffsetChanged);
	for (int i = 0; i < 3; i++)
		offsetWidgets[i] = widgets[i];

	offsetRot = new Parameter<Vector3>("offsetPos", Vector3::zero);
	widgets = GenerateVectorLineBoxes(offsetRot);
	layout->addWidget(new QLabel("Offset Rotation"), 2, 0, 1, 4);
	layout->addWidget(widgets[0], 2, 1, 1, 1);
	connect(widgets[0], &QLineEdit::textChanged, this, &TrackingVirtualizerListItem::OnOffsetChanged);
	layout->addWidget(widgets[1], 2, 2, 1, 1);
	connect(widgets[1], &QLineEdit::textChanged, this, &TrackingVirtualizerListItem::OnOffsetChanged);
	layout->addWidget(widgets[2], 2, 3, 1, 1);
	connect(widgets[1], &QLineEdit::textChanged, this, &TrackingVirtualizerListItem::OnOffsetChanged);
	for (int i = 0; i < 3; i++)
		offsetWidgets[3 + i] = widgets[i];
	*/

	selectedVirtualizer = possibleVirtualizers[0]->Clone();
	virtualizerParameterListWidget = GenerateParameterTree(selectedVirtualizer->GetParameters());
	layout->addWidget(virtualizerParameterListWidget, 3, 0, 4, 4);
	
	// Setup the signals to slots
	connect(solveSlotComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnSolveSlotComboboxValueChanged(int)));
	connect(trackingVirtualizerComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnTrackingVirtualizerComboboxChanged(int)));
	connect(deleteButton, &QPushButton::clicked, this, &TrackingVirtualizerListItem::OnDeleteButtonPressed);

	setLayout(layout);
}

TrackingVirtualizerListItem::~TrackingVirtualizerListItem()
{
	if (selectedVirtualizer)
		delete selectedVirtualizer;
}

QJsonObject TrackingVirtualizerListItem::SaveSettings()
{
	QJsonObject json;

	AttachedModel* trackerModel = tracker->GetModel();
	//Trackermodel information
	json.insert("filename", QJsonValue(trackerModel->GetFilename().c_str()));
	json.insert("filepath", QJsonValue(trackerModel->GetFilepath().c_str()));
	json.insert("name", QJsonValue(trackerModel->getName().c_str()));
	json.insert("parentnode", QJsonValue(trackerModel->getParentNode()->Name.c_str()));
	//json.insert("attachedTo", QJsonValue(trackerModel->getAttachedTo()->getName().c_str()));
	std::map<int, float> weightMapping = trackerModel->GetWeightMapping();

	QJsonObject weights;
	for (auto const& kv : weightMapping)
		weights.insert(QString::number(kv.first), QJsonValue(kv.second));

	json.insert("weightmapping", weights);
	json.insert("transform", QJsonSerializer::MatrixToJson(trackerModel->getTransform()));

	//Virtualizer settings
	json.insert("virtualizer", trackingVirtualizerComboBox->itemText(trackingVirtualizerComboBox->currentIndex()));
	json.insert("solveslot", solveSlotComboBox->itemText(solveSlotComboBox->currentIndex()));

	//Virtualizer parameters
	json.insert("trackersettings", QJsonSerializer::ParameterMapToJson(widgetParameterMap));

	return json;
}

void TrackingVirtualizerListItem::LoadSettings(const QJsonObject& json, Scene& scene)
{
	//Trackermodel information
	//validate avatar
	std::string file = json["filepath"].toString().toStdString() + json["filename"].toString().toStdString();
	SetupScene& setupScene = dynamic_cast<SetupScene&>(scene);
	tracker = setupScene.generateTracker(file);
	tracker->GetModel()->setName(json["name"].toString().toStdString());

	std::string nodeName = json["parentnode"].toString().toStdString();
	QJsonObject weights = json["weightmapping"].toObject();
	std::map<int, float> weightMapping;
	for (const QString& key : weights.keys())
		weightMapping[key.toInt()] = (float)weights[key].toDouble();
	Matrix transform = QJsonSerializer::JsonToMatrix(json["transform"].toArray());

	tracker->GetModel()->attachToMesh(setupScene.GetCharacter(), nodeName, weightMapping, transform);

	//Virtualizer settings
	trackingVirtualizerComboBox->setCurrentIndex(trackingVirtualizerComboBox->findText(json["virtualizer"].toString()));
	solveSlotComboBox->setCurrentIndex(solveSlotComboBox->findText(json["solveslot"].toString()));

	//Virtualizer information
	QJsonSerializer::JsonToParameterMap(widgetParameterMap, json["trackersettings"].toObject());
	UpdateWidgetStates();
}

void TrackingVirtualizerListItem::SelectVirtualizer(const BaseTrackingVirtualizer* bp)
{
	if (selectedVirtualizer)
		delete selectedVirtualizer;

	selectedVirtualizer = bp->Clone();
	UpdateParameterTree(selectedVirtualizer->GetParameters());
}

BaseTrackingVirtualizer* TrackingVirtualizerListItem::GetVirtualizer()
{
	return selectedVirtualizer;
}

void TrackingVirtualizerListItem::OnTrackingVirtualizerComboboxChanged(int value)
{
	EventManager::instance().FireEvent("OnVirtualizerChanged", this);

	SelectVirtualizer(BaseTrackingVirtualizer::registry()[value]);
}

void TrackingVirtualizerListItem::OnSolveSlotComboboxValueChanged(int value)
{
	EventManager::instance().FireEvent("OnSolveSlotChanged", this);

	prevSlotName = solveSlotComboBox->itemText(value).toStdString();

	//Calculate offset
	//if (prevSlotName == "None")
		//return;

	tracker->SetSlot(prevSlotName);

	/*
	Vector3 pos = tracker->GetOffsetPosition();
	Vector3 rot = tracker->GetOffsetRotation().eulerAngles();
	offsetPos->SetValue(pos);
	offsetRot->SetValue(rot);

	blockSignals(true);
	for (int i = 0; i < 6; i++)
	{
		QLineEdit* widget = offsetWidgets[i];
		Vector3 vec = i < 3 ? pos : rot;
		int id = i % 3;
		float value;
		switch (id)
		{
		case 0:
			value = vec.x;
		case 1:
			value = vec.y;
		case 2:
			value = vec.z;
		}

		offsetWidgets[i]->setText(QString::number(value));
	}
	blockSignals(false);
	*/
}

void TrackingVirtualizerListItem::OnDeleteButtonPressed()
{
	TrackingVirtualizerList* parent = dynamic_cast<TrackingVirtualizerList*>(parentWidget()->parentWidget());
	QListWidgetItem* thisItem = NULL;

	for (int i = 0; i < parent->count(); i++)
	{
		QListWidgetItem* item = parent->item(i);
		if (parent->itemWidget(item) == this)
		{
			thisItem = item;
			break;
		}
	}

	EventManager::instance().FireEvent("OnTrackerRemoved", tracker);
	parent->RemoveItem(this);

	delete thisItem;
}
/*
void TrackingVirtualizerListItem::OnOffsetChanged()
{
	qDebug() << "offset change";
	tracker->SetOffsetPosition(offsetPos->GetValue());
	tracker->SetOffsetRotation(Quaternion::Euler(offsetRot->GetValue()));
}
*/
std::string TrackingVirtualizerListItem::GetCurrentSlotName()
{
	return solveSlotComboBox->itemText(solveSlotComboBox->currentIndex()).toStdString();
}

std::string TrackingVirtualizerListItem::GetPreviousSlotName()
{
	return prevSlotName;
}

void TrackingVirtualizerListItem::enterEvent(QEvent* event)
{
	EventManager::instance().FireEvent("OnTrackerHovered", tracker);
}

void TrackingVirtualizerListItem::leaveEvent(QEvent* event)
{
	EventManager::instance().FireEvent("OnTrackerUnhovered", tracker);
}

bool operator==(const TrackingVirtualizerListItem& lhs, const TrackingVirtualizerListItem& rhs)
{
	return &lhs == &rhs;
}
