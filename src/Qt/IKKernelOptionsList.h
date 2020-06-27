#pragma once

#include "../Customizable/InverseKinematicsKernels/BaseIKKernel.h"
#include "../ParameterListWidget.h"

class IKKernelOptionsList : public ParameterListWidget
{
	Q_OBJECT
private:
	BaseIKKernel* currentKernel;
public:
	IKKernelOptionsList(QWidget* parent);
	QJsonObject SaveSettings() const;
	void LoadSettings(QJsonObject json, QComboBox* ikComboBox);
public slots:
	void OnIKKernelChanged(int kernelIndex);
};

