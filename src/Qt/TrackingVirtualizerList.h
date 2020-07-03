#pragma once

#include <QListWidget>

#include "../AttachedModel.h"
#include "../Customizable/InverseKinematicsKernels/BaseIKKernel.h"
#include "TrackingVirtualizerListWidget.h"
#include "../Scene.h"
#include "../Tracker.h"

typedef std::pair<std::string, bool> SolveSlot;

class TrackingVirtualizerList : public QListWidget
{
	Q_OBJECT
private:
	int missingSlots;
	BaseIKKernel* currentKernel;
	//std::vector<SolveSlot> solveSlots;
	std::map<std::string, bool> solveSlotsInUse;

	void UpdateWidgetSlotComboBox(TrackingVirtualizerListItem& itemWidget);
public:
	std::vector<TrackingVirtualizerListItem*> itemWidgets;
	TrackingVirtualizerList(QWidget* parent);
	QJsonObject SaveTrackers();
	void LoadTrackers(const QJsonObject& json, Scene& scene);

	TrackingVirtualizerListItem* OnTrackerGotPlaced(Tracker* tracker);
	int GetMissingSlots();
	void RemoveItem(TrackingVirtualizerListItem* item);
	void Reset();
	void OnTrackerSelected(Tracker* tracker);
public slots:
	// Callbacks from items
	void OnIKKernelChanged(int kernelIndex);
	void OnItemSolveSlotValueChanged(TrackingVirtualizerListItem* itemWidget);
	void OnItemVirtualizerValueChanged(TrackingVirtualizerListItem* itemWidget);
	void OnItemClicked(QListWidgetItem* item);
	void OnItemHovered(QListWidgetItem* item);
signals:
	void UpdateInformationText(QString value);
	void ValidateCalculationButton();
};