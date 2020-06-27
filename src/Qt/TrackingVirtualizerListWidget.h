#pragma once

#include <QWidget>
#include "../Customizable/TrackingVirtualizers/BaseTrackingVirtualizer.h"
#include <QtWidgets>
#include <string>
#include "../Scene.h"
#include "../ParameterListWidget.h"
#include "../Tracker.h"

class TrackingVirtualizerListItem : public ParameterListWidget
{
	Q_OBJECT
private:
	std::string prevSlotName;
	BaseTrackingVirtualizer* selectedVirtualizer;
	QTreeWidget* virtualizerParameterListWidget;

	//Parameter<Vector3>* offsetPos;
	//Parameter<Vector3>* offsetRot;
	//QLineEdit** offsetWidgets;
public:
	Tracker* tracker;
	QComboBox* trackingVirtualizerComboBox;
	QComboBox* solveSlotComboBox;

	TrackingVirtualizerListItem(QWidget* parent, Tracker* tracker);
	virtual ~TrackingVirtualizerListItem();
	QJsonObject SaveSettings();
	void LoadSettings(const QJsonObject& json, Scene& scene);
	void SelectVirtualizer(const BaseTrackingVirtualizer* bp);
	BaseTrackingVirtualizer* GetVirtualizer();

	std::string GetCurrentSlotName();
	std::string GetPreviousSlotName();
protected:
	virtual void enterEvent(QEvent* event);
	virtual void leaveEvent(QEvent* event);
public slots:
	void OnTrackingVirtualizerComboboxChanged(int value);
	void OnSolveSlotComboboxValueChanged(int value);
	void OnDeleteButtonPressed();
	//void OnOffsetChanged();
};

// Checks identity, not equality
bool operator==(const TrackingVirtualizerListItem& lhs, const TrackingVirtualizerListItem& rhs);