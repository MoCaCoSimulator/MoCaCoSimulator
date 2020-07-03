#pragma once

#include "../Customizable/ErrorMetrics/BaseErrorMetric.h"
#include "../ParameterListWidget.h"

class ErrorMetricListWidget : public ParameterListWidget
{
	Q_OBJECT
public:
	BaseErrorMetric* errorMetric;

	// Copy error metric
	ErrorMetricListWidget(QWidget* parent, const BaseErrorMetric* errorMetric);
	~ErrorMetricListWidget();

	QJsonObject SaveSettings() const;
	void LoadSettings(const QJsonObject& json);
public slots:
	void OnDeleteButtonPressed();
};

// Checks identity, not equality
bool operator==(const ErrorMetricListWidget& lhs, const ErrorMetricListWidget& rhs);