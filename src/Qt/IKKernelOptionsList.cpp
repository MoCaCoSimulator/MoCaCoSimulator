#include "IKKernelOptionsList.h"
#include "../QJsonSerializer.h"

IKKernelOptionsList::IKKernelOptionsList(QWidget* parent) : ParameterListWidget(parent)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setMargin(0);

	currentKernel = BaseIKKernel::registry()[0];
	QTreeWidget* tree = GenerateParameterTree(currentKernel->GetParameters());
	layout->addWidget(tree);

	setLayout(layout);
}

QJsonObject IKKernelOptionsList::SaveSettings() const
{
	QJsonObject json;

	json.insert("currentkernel", QJsonValue(currentKernel->GetName().c_str()));
	json.insert("kernelsettings", QJsonSerializer::ParameterMapToJson(widgetParameterMap));

	return json;
}

void IKKernelOptionsList::LoadSettings(QJsonObject json, QComboBox* ikComboBox)
{
	std::string kernelName = json["currentkernel"].toString().toStdString();
	const auto& registry = BaseIKKernel::registry();
	for (int i = 0; i < registry.size(); i++)
		if (registry[i]->GetName() == kernelName)
			ikComboBox->setCurrentIndex(i);

	QJsonSerializer::JsonToParameterMap(widgetParameterMap, json["kernelsettings"].toObject());
	UpdateWidgetStates();
}

void IKKernelOptionsList::OnIKKernelChanged(int kernelIndex)
{
	currentKernel = BaseIKKernel::registry()[kernelIndex];

	qDebug() << "IKKernelOptionsList: Kernel changed to " << currentKernel->GetName().c_str();

	UpdateParameterTree(currentKernel->GetParameters());
}