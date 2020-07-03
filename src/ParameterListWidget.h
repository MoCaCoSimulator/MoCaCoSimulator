#pragma once

#include <QWidget>
#include <QtWidgets>
#include <string>
#include "Parameter.h"
#include "vector.h"

class ParameterListWidget : public QWidget
{
	Q_OBJECT
protected:
	std::map<QWidget*, BaseParameter*> widgetParameterMap;
	QTreeWidget* parameterTree;
public:
	ParameterListWidget(QWidget* parent) : QWidget(parent), parameterTree(NULL) {}
	~ParameterListWidget();
private:
	void GenerateParameterItem(BaseParameter* parameter);
protected:
	void GenerateParameters(const std::map<std::string, BaseParameter*>& parameters);
	QTreeWidget* GenerateParameterTree(const std::map<std::string, BaseParameter*>& parameters);
	void UpdateParameterTree(const std::map<std::string, BaseParameter*>& parameters);
	void UpdateWidgetState(QWidget* widget, BaseParameter* parameter);
	void UpdateWidgetStates();
	QMetaEnum GetMetaEnum(const std::string& enumName) const;
	QWidget* GenerateCheckBox(Parameter<bool>* parameter);
	QWidget* GenerateSpinBox(Parameter<int>* parameter);
	QWidget* GenerateLineBox(Parameter<float>* parameter);
	QWidget* GenerateLineBox(Parameter<double>* parameter);
	QWidget* GenerateLineBox(Parameter<std::string>* parameter);
	QWidget* GenerateComboBox(Parameter<int>* parameter, const type_info& paramType);
	QLineEdit** GenerateVectorLineBoxes(Parameter<Vector3>* parameter);
public slots:
	void OnSpinBoxChange(int value);
	void OnFloatLineChange(const QString& value);
	void OnVectorFloatLineChange(const QString& value);
	void OnDoubleLineChange(const QString& value);
	void OnCheckBoxChange(int value);
	void OnLineEditChange(const QString& value);
	void OnComboBoxChange(int value);
};