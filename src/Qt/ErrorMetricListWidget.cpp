#include "ErrorMetricListWidget.h"
#include "../Customizable/ErrorMetrics/PositionDifferenceErrorMetric.h"
#include "../Enumerations.h"
#include "../QJsonSerializer.h"
#include "../EventManager.h"

ErrorMetricListWidget::ErrorMetricListWidget(QWidget* parent, const BaseErrorMetric* errorMetric) : 
	ParameterListWidget(parent)
{
	QGridLayout* layout = new QGridLayout();

	QLabel* label = new QLabel(errorMetric->GetName().c_str());
	layout->addWidget(label, 0, 0, 1, 2);

	QPushButton* deleteButton = new QPushButton("Delete");
	layout->addWidget(deleteButton, 0, 2, 1, 1);
	connect(deleteButton, &QPushButton::clicked, this, &ErrorMetricListWidget::OnDeleteButtonPressed);

	this->errorMetric = errorMetric->Clone();
	QTreeWidget* tree = GenerateParameterTree(this->errorMetric->GetParameters());
	layout->addWidget(tree, 1, 0, 4, 3); 

	setLayout(layout);
}

ErrorMetricListWidget::~ErrorMetricListWidget()
{
	delete errorMetric;
}

QJsonObject ErrorMetricListWidget::SaveSettings() const
{
	QJsonObject json;

	//Virtualizer parameters
	json.insert("metricsettings", QJsonSerializer::ParameterMapToJson(widgetParameterMap));

	return json;
}

void ErrorMetricListWidget::LoadSettings(const QJsonObject& json)
{
	//Virtualizer information
	QJsonSerializer::JsonToParameterMap(widgetParameterMap, json["metricsettings"].toObject());
	UpdateWidgetStates();
}

bool operator==(const ErrorMetricListWidget& lhs, const ErrorMetricListWidget& rhs)
{
	return &lhs == &rhs;
}

void ErrorMetricListWidget::OnDeleteButtonPressed()
{
	QListWidget* parent = dynamic_cast<QListWidget*>(parentWidget()->parentWidget());
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

	delete thisItem;
}