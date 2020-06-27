#include "ErrorMetricList.h"

ErrorMetricList::ErrorMetricList(QWidget* parent) : QListWidget(parent), errorMetricIndex(0)
{
}

ErrorMetricListWidget* ErrorMetricList::AddNewErrorMetric()
{
	QListWidgetItem* item = new QListWidgetItem(this);
	ErrorMetricListWidget* customItem = new ErrorMetricListWidget(this, BaseErrorMetric::registry()[errorMetricIndex]);
	itemWidgets.push_back(customItem);
	item->setSizeHint(customItem->minimumSizeHint());
	setItemWidget(item, customItem);
	addItem(item);

	ValidateCalculationButton();

	return customItem;
}

void ErrorMetricList::SetErrorMetricIndex(int index)
{
	errorMetricIndex = index;
}

QJsonObject ErrorMetricList::SaveSettings() const
{
	QJsonObject json;

	QJsonArray metrics;
	for (ErrorMetricListWidget* item : itemWidgets) 
	{
		QJsonObject object = item->SaveSettings();
		object.insert("metricname", QJsonValue(QString(item->errorMetric->GetName().c_str())));
		metrics.append(object);
	}
	json.insert("metrics", metrics);

	return json;
}

void ErrorMetricList::LoadSettings(QJsonObject json)
{
	QJsonArray metrics = json.value(QString("metrics")).toArray();
	for (int i = 0; i < metrics.size(); i++)
	{
		QJsonObject object = metrics[i].toObject();
		std::string name = object["metricname"].toString().toStdString();

		std::vector<const BaseErrorMetric*>& metricList = BaseErrorMetric::registry();
		for (int i = 0; i < metricList.size(); i++)
		{
			if (metricList[i]->GetName() != name)
				continue;

			errorMetricIndex = i;
			break;
		}
		ErrorMetricListWidget* widget = AddNewErrorMetric();
		widget->LoadSettings(object);
	}
}

void ErrorMetricList::Reset()
{
	clearSelection();
	clear();
	itemWidgets.clear();
}
