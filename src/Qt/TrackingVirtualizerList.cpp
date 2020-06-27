#include "TrackingVirtualizerList.h"
#include "../EventManager.h"
#include "TrackingVirtualizerListWidget.h"

#include <QDebug>
#include <sstream>

TrackingVirtualizerList::TrackingVirtualizerList(QWidget* parent) : 
	QListWidget(parent), 
	currentKernel()
{
	// On tracker called callback
	std::function<void(Tracker*)> trackerPlacedCallback = std::function([this](Tracker* value) { this->OnTrackerGotPlaced(value); return; });
	EventManager::instance().SubscribeToEvent("OnTrackerPlaced", trackerPlacedCallback);
	
	// On virtualizer changed feedback
	std::function<void(TrackingVirtualizerListItem*)> itemVirtualizerChanged = std::function(
		[this](TrackingVirtualizerListItem* value)
		{
			return this->OnItemVirtualizerValueChanged(value);
		});
	EventManager::instance().SubscribeToEvent("OnVirtualizerChanged", itemVirtualizerChanged);
	
	// On solveslot changed feedback
	std::function<void(TrackingVirtualizerListItem*)> itemSolveSlotChanged = std::function(
	[this](TrackingVirtualizerListItem* value) 
	{
		return this->OnItemSolveSlotValueChanged(value);
	});
	EventManager::instance().SubscribeToEvent("OnSolveSlotChanged", itemSolveSlotChanged);


	std::function<void(Tracker*)> trackerCallback = std::function([this](Tracker* m) { return this->OnTrackerSelected(m); });
	EventManager::instance().SubscribeToEvent("OnTrackerSelected", trackerCallback);
	//trackerCallback = [this](Tracker* m) { return this->OnTrackerHovered(m); };
	//EventManager::instance().SubscribeToEvent("OnTrackerHovered", trackerCallback);

	//connect(this, &QListWidget::itemSelectionChanged, this, &TrackingVirtualizerList::OnSelectionChanged);
	connect(this, &QListWidget::itemEntered, this, &TrackingVirtualizerList::OnItemHovered);
	connect(this, &QListWidget::itemClicked, this, &TrackingVirtualizerList::OnItemClicked);

	OnIKKernelChanged(0);
}

QJsonObject TrackingVirtualizerList::SaveTrackers()
{
	QJsonObject json;

	QJsonArray tracker;
	int i = 0;
	for (TrackingVirtualizerListItem* item : itemWidgets)
		tracker.append(item->SaveSettings());
	json.insert("tracker", tracker);

	return json;
}

void TrackingVirtualizerList::LoadTrackers(const QJsonObject& json, Scene& scene)
{
	QJsonArray tracker = json.value(QString("tracker")).toArray();
	for (int i = 0; i < tracker.size(); i++)
	{ 
		QJsonObject object = tracker[i].toObject();
		TrackingVirtualizerListItem* widget = OnTrackerGotPlaced(NULL);
		widget->LoadSettings(object, scene);
	}
}

TrackingVirtualizerListItem* TrackingVirtualizerList::OnTrackerGotPlaced(Tracker* tracker)
{
	qDebug() << "Tracker placed";

	QListWidgetItem* item = new QListWidgetItem(this);
	TrackingVirtualizerListItem* customItem = new TrackingVirtualizerListItem(this, tracker);
	itemWidgets.push_back(customItem);
	item->setSizeHint(customItem->minimumSizeHint());
	setItemWidget(item, customItem);
	UpdateWidgetSlotComboBox(*customItem);
	addItem(item);
	return customItem;
}

int TrackingVirtualizerList::GetMissingSlots()
{
	return missingSlots;
}

void TrackingVirtualizerList::RemoveItem(TrackingVirtualizerListItem* item)
{
	itemWidgets.erase(std::remove(itemWidgets.begin(), itemWidgets.end(), item), itemWidgets.end());
}

void TrackingVirtualizerList::Reset()
{
	clearSelection();
	clear();
	itemWidgets.clear();
}

void TrackingVirtualizerList::OnIKKernelChanged(int kernelIndex)
{
	qDebug() << "Kernel changed";

	currentKernel = BaseIKKernel::registry()[kernelIndex];

	solveSlotsInUse.clear();
	std::vector<std::string> names = currentKernel->InputNames();
	missingSlots = names.size();

	for (std::string name : names)
		solveSlotsInUse[name] = false;

	for (TrackingVirtualizerListItem* widget : itemWidgets)
		UpdateWidgetSlotComboBox(*widget);
}

void TrackingVirtualizerList::UpdateWidgetSlotComboBox(TrackingVirtualizerListItem& itemWidget)
{
	itemWidget.solveSlotComboBox->blockSignals(true);

	std::string currentSlot = itemWidget.GetCurrentSlotName();
	if (currentSlot.empty())
		currentSlot = "None";

	itemWidget.solveSlotComboBox->clear();
	itemWidget.solveSlotComboBox->addItem("None");
	for (auto& solveSlot : solveSlotsInUse)
		if (!solveSlot.second || solveSlot.first == currentSlot)
			itemWidget.solveSlotComboBox->addItem(QString::fromStdString(solveSlot.first));

	int newIndex = itemWidget.solveSlotComboBox->findText(QString::fromStdString(currentSlot));
	itemWidget.solveSlotComboBox->setCurrentIndex(newIndex);

	itemWidget.solveSlotComboBox->blockSignals(false);
}

void TrackingVirtualizerList::OnItemVirtualizerValueChanged(TrackingVirtualizerListItem* itemWidget)
{
	qDebug() << "Virtualizer changed";
}

void TrackingVirtualizerList::OnTrackerSelected(Tracker* tracker)
{
	blockSignals(true);

	for (int i = 0; i < count(); i++)
	{
		QListWidgetItem* qItem = item(i);
		const QWidget* widget = itemWidget(qItem);
		const TrackingVirtualizerListItem* tv = dynamic_cast<const TrackingVirtualizerListItem*>(widget);

		setItemSelected(qItem, tv->tracker == tracker);
	}

	blockSignals(false);
}

void TrackingVirtualizerList::OnItemClicked(QListWidgetItem* item)
{
	//const auto& items = selectedItems();
	//for (const auto& item : items)
	//{
		qDebug() << "item clicked";
		const QWidget* widget = itemWidget(item);
		const TrackingVirtualizerListItem* selection = dynamic_cast<const TrackingVirtualizerListItem*>(widget);
		EventManager::instance().FireEvent("OnTrackerSelected", selection->tracker);
		qDebug() << "event fired";
	//break;
	//}
}

void TrackingVirtualizerList::OnItemHovered(QListWidgetItem* item)
{
	qDebug() << "item hovered";
	const QWidget* widget = itemWidget(item);
	const TrackingVirtualizerListItem* selection = dynamic_cast<const TrackingVirtualizerListItem*>(widget);
	EventManager::instance().FireEvent("OnTrackerHovered", selection->tracker);
}

void TrackingVirtualizerList::OnItemSolveSlotValueChanged(TrackingVirtualizerListItem* itemWidget)
{
	std::string prevSlotName = itemWidget->GetPreviousSlotName();
	std::string slotName = itemWidget->GetCurrentSlotName();

	if (prevSlotName != "None")
		solveSlotsInUse[prevSlotName] = false;
	if (slotName != "None") 
		solveSlotsInUse[slotName] = true;

	for (TrackingVirtualizerListItem* widget : itemWidgets) 
		if (widget != itemWidget)
			UpdateWidgetSlotComboBox(*widget);

	missingSlots = 0;
	for (auto& solveSlot : solveSlotsInUse)
		if (!solveSlot.second)
			++missingSlots;

	ValidateCalculationButton();
}