#pragma once
#include <QListWidget>
#include "../Customizable/ErrorMetrics/BaseErrorMetric.h"
#include "ErrorMetricListWidget.h"

class ErrorMetricList : public QListWidget
{
	Q_OBJECT
private:
	int errorMetricIndex;
public:
	std::vector<ErrorMetricListWidget*> itemWidgets;

	ErrorMetricList(QWidget* parent);

	QJsonObject SaveSettings() const;
	void LoadSettings(QJsonObject json);
	void Reset();
public slots:
	void SetErrorMetricIndex(int index);
	ErrorMetricListWidget* AddNewErrorMetric();
signals:
	void ValidateCalculationButton();
};