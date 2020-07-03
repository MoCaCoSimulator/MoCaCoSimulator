#include "ParameterListWidget.h"
#include "Enumerations.h"
#include "Customizable/CustomEnumerations.h"

ParameterListWidget::~ParameterListWidget()
{
	widgetParameterMap.clear();
}

QWidget* ParameterListWidget::GenerateCheckBox(Parameter<bool>* parameter)
{
	//parameter = new SpecificParameter<bool>(*parameter);
	QCheckBox* checkBox = new QCheckBox();
	checkBox->setChecked(parameter->GetValue());
	connect(checkBox, &QCheckBox::stateChanged, this, &ParameterListWidget::OnCheckBoxChange);
	widgetParameterMap[checkBox] = parameter;
	return checkBox;
}

QWidget* ParameterListWidget::GenerateSpinBox(Parameter<int>* parameter)
{
	//parameter = new SpecificParameter<int>(*parameter);
	QSpinBox* spinBox = new QSpinBox();
	spinBox->setMinimum(std::numeric_limits<int>::min());
	spinBox->setMaximum(std::numeric_limits<int>::max());
	spinBox->setValue(parameter->GetValue());
	connect(spinBox, qOverload<int>(&QSpinBox::valueChanged), this, &ParameterListWidget::OnSpinBoxChange);
	widgetParameterMap[spinBox] = parameter;
	return spinBox;
}

QWidget* ParameterListWidget::GenerateLineBox(Parameter<float>* parameter)
{
	//parameter = new SpecificParameter<float>(*parameter);
	QLineEdit* lineBox = new QLineEdit();
	lineBox->setText(QString::number(parameter->GetValue()));
	lineBox->setValidator(new QDoubleValidator(lineBox));
	connect(lineBox, &QLineEdit::textChanged, this, &ParameterListWidget::OnFloatLineChange);
	widgetParameterMap[lineBox] = parameter;
	return lineBox;
}

QWidget* ParameterListWidget::GenerateLineBox(Parameter<double>* parameter)
{
	//parameter = new SpecificParameter<double>(*parameter);
	QLineEdit* lineBox = new QLineEdit();
	lineBox->setText(QString::number(parameter->GetValue()));
	lineBox->setValidator(new QDoubleValidator(lineBox));
	connect(lineBox, &QLineEdit::textChanged, this, &ParameterListWidget::OnDoubleLineChange);
	widgetParameterMap[lineBox] = parameter;
	return lineBox;
}

QWidget* ParameterListWidget::GenerateLineBox(Parameter<std::string>* parameter)
{
	//parameter = new SpecificParameter<std::string>(*parameter);
	QLineEdit* lineBox = new QLineEdit();
	lineBox->setText(QString(parameter->GetValue().c_str()));
	connect(lineBox, &QLineEdit::textChanged, this, &ParameterListWidget::OnLineEditChange);
	widgetParameterMap[lineBox] = parameter;
	return lineBox;
}

QMetaEnum ParameterListWidget::GetMetaEnum(const std::string& enumName) const
{
	QMetaEnum metaEnum;
	const QMetaObject* mo;
	int index;

	//try build in enums
	mo = &Enums::staticMetaObject;
	index = mo->indexOfEnumerator(enumName.c_str());
	if (index != -1)
		return mo->enumerator(index);
	//try custom enums
	mo = &CustomEnums::staticMetaObject;
	index = mo->indexOfEnumerator(enumName.c_str());
	if (index != -1)
		return mo->enumerator(index);

	return metaEnum;
}

QWidget* ParameterListWidget::GenerateComboBox(Parameter<int>* parameter, const type_info& paramType)
{
	//parameter = new SpecificParameter<int>(*parameter);

	std::string paramName = paramType.name();
	int delimIndex = paramName.find("::");
	std::string enumName = paramName.substr(delimIndex + 2);
	QComboBox* comboBox = new QComboBox();

	QMetaEnum metaEnum = GetMetaEnum(enumName);
	for (int i = 0; i < metaEnum.keyCount(); i++)
	{
		const char* val = metaEnum.valueToKey(i);
		comboBox->addItem(val);
	}

	comboBox->blockSignals(true);
	comboBox->setCurrentIndex(parameter->GetValue());
	comboBox->blockSignals(false);

	connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnComboBoxChange(int)));
	widgetParameterMap[comboBox] = parameter;
	return comboBox;
}

//QWidget* ParameterListWidget::GenerateVectorLineBoxes(Parameter<Vector3>* parameter)
QLineEdit** ParameterListWidget::GenerateVectorLineBoxes(Parameter<Vector3>* parameter)
{
	QLineEdit** components = new QLineEdit*[3];
	const Vector3& vec = parameter->GetValue();

	for (int i = 0; i < 3; i++)
	{
		QLineEdit* lineBox = new QLineEdit();

		switch (i)
		{
		case 0:
			lineBox->setText(QString::number(vec.x));
			lineBox->setObjectName("X");
			break;
		case 1:
			lineBox->setText(QString::number(vec.y));
			lineBox->setObjectName("Y");
			break;
		case 2:
			lineBox->setText(QString::number(vec.z));
			lineBox->setObjectName("Z");
			break;
		}

		lineBox->setValidator(new QDoubleValidator(lineBox));
		connect(lineBox, &QLineEdit::textChanged, this, &ParameterListWidget::OnVectorFloatLineChange);
		qDebug() << "vector add" << lineBox << parameter;
		widgetParameterMap[lineBox] = parameter;
		qDebug() << "wpm" << widgetParameterMap[lineBox];

		components[i] = lineBox;
	}

	return components;
}

void ParameterListWidget::GenerateParameterItem(BaseParameter* parameter)
{
	QTreeWidgetItem* item = new QTreeWidgetItem(parameterTree);
	item->setText(0, QString(parameter->GetName().c_str()));

	const type_info& paramType = parameter->GetType();
	QWidget* widget = NULL;
	if (paramType == typeid(bool))
		widget = GenerateCheckBox(dynamic_cast<Parameter<bool>*>(parameter));
	else if (paramType == typeid(int))
		widget = GenerateSpinBox(dynamic_cast<Parameter<int>*>(parameter));
	else if (paramType == typeid(float))
		widget = GenerateLineBox(dynamic_cast<Parameter<float>*>(parameter));
	else if (paramType == typeid(double))
		widget = GenerateLineBox(dynamic_cast<Parameter<double>*>(parameter));
	else if (paramType == typeid(std::string))
		widget = GenerateLineBox(dynamic_cast<Parameter<std::string>*>(parameter));
	else if (std::string(paramType.name()).find("enum ") != std::string::npos)
		widget = GenerateComboBox(static_cast<Parameter<int>*>(parameter), paramType);

	parameterTree->setItemWidget(item, 1, widget);
}

void ParameterListWidget::UpdateWidgetState(QWidget* widget, BaseParameter* parameter)
{
	const type_info& paramType = parameter->GetType();
	
	QCheckBox* checkBoxWidget = dynamic_cast<QCheckBox*>(widget);
	if (checkBoxWidget) 
	{
		checkBoxWidget->setCheckState(dynamic_cast<Parameter<bool>*>(parameter)->GetValue() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
		return;
	}

	QSpinBox* spinBoxWidget = dynamic_cast<QSpinBox*>(widget);
	if (spinBoxWidget)
	{
		spinBoxWidget->setValue(dynamic_cast<Parameter<int>*>(parameter)->GetValue());
		return;
	}

	QLineEdit* lineEditWidget = dynamic_cast<QLineEdit*>(widget);
	if (lineEditWidget)
	{
		if (paramType == typeid(float))
			lineEditWidget->setText(QString(std::to_string(dynamic_cast<Parameter<float>*>(parameter)->GetValue()).c_str()));
		else if (paramType == typeid(double))
			lineEditWidget->setText(QString(std::to_string(dynamic_cast<Parameter<double>*>(parameter)->GetValue()).c_str()));
		else if (paramType == typeid(std::string))
			lineEditWidget->setText(dynamic_cast<Parameter<std::string>*>(parameter)->GetValue().c_str());
		else
			throw std::exception("could not parse QLineEdit");

		return;
	}

	QComboBox* comboBoxWidget = dynamic_cast<QComboBox*>(widget);
	if (comboBoxWidget)
	{
		comboBoxWidget->setCurrentIndex(static_cast<Parameter<int>*>(parameter)->GetValue());
		return;
	}

	throw std::exception("could not parse QWidget");
}

void ParameterListWidget::GenerateParameters(const std::map<std::string, BaseParameter*>& parameters)
{
	parameterTree->blockSignals(true);

	// NOTE: These got deleted beforehand so the pointer is invalid at this point
	//for (const auto& kv : widgetParameterMap)
	//	delete kv.second;
	widgetParameterMap.clear();
	parameterTree->clear();

	for (std::pair<std::string, BaseParameter*> element : parameters)
		GenerateParameterItem(element.second);

	parameterTree->blockSignals(false);
}

QTreeWidget* ParameterListWidget::GenerateParameterTree(const std::map<std::string, BaseParameter*>& parameters)
{
	parameterTree = new QTreeWidget();
	parameterTree->setColumnCount(2);
	parameterTree->setHeaderLabels(QStringList({ "Name", "Value" }));
	parameterTree->setSizePolicy(QSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding));
	UpdateParameterTree(parameters);
	return parameterTree;
}

void ParameterListWidget::UpdateParameterTree(const std::map<std::string, BaseParameter*>& parameters)
{
	GenerateParameters(parameters);
}

void ParameterListWidget::UpdateWidgetStates()
{
	for (const auto& kv : widgetParameterMap)
		UpdateWidgetState(kv.first, kv.second);
}

void ParameterListWidget::OnSpinBoxChange(int value)
{
	QSpinBox* spinBox = (QSpinBox*)sender();
	Parameter<int>* parameter = dynamic_cast<Parameter<int>*>(widgetParameterMap[spinBox]);
	parameter->SetValue(value);
	qDebug() << parameter->GetValue();
}

void ParameterListWidget::OnFloatLineChange(const QString& value)
{
	QLineEdit* lineBox = (QLineEdit*)sender();
	Parameter<float>* parameter = dynamic_cast<Parameter<float>*>(widgetParameterMap[lineBox]);
	parameter->SetValue(value.toFloat());
	qDebug() << parameter->GetValue();
}

void ParameterListWidget::OnVectorFloatLineChange(const QString& value)
{
	QLineEdit* lineBox = (QLineEdit*)sender();
	Parameter<Vector3>* parameter = dynamic_cast<Parameter<Vector3>*>(widgetParameterMap[lineBox]);

	qDebug() << "vector change";
	qDebug() << lineBox->objectName();
	qDebug() << widgetParameterMap[lineBox];
	qDebug() << parameter;
	float v = value.toFloat();
	qDebug() << v;
	Vector3 vec = parameter->GetValue();
	qDebug() << vec.toString().c_str();
	QString boxName = lineBox->objectName();
	
	if (boxName == "X")
		vec.x = v;
	else if (boxName == "Y")
		vec.y = v;
	else if (boxName == "Z")
		vec.z = v;

	parameter->SetValue(vec);
	qDebug() << parameter->GetValue().toString().c_str();
}

void ParameterListWidget::OnDoubleLineChange(const QString& value)
{
	QLineEdit* lineBox = (QLineEdit*)sender();
	Parameter<double>* parameter = dynamic_cast<Parameter<double>*>(widgetParameterMap[lineBox]);
	parameter->SetValue(value.toDouble());
	qDebug() << parameter->GetValue();
}

void ParameterListWidget::OnCheckBoxChange(int value)
{
	QCheckBox* checkBox = (QCheckBox*)sender();
	Parameter<bool>* parameter = dynamic_cast<Parameter<bool>*>(widgetParameterMap[checkBox]);
	parameter->SetValue(value != 0 ? true : false);
	qDebug() << parameter->GetValue();
}

void ParameterListWidget::OnLineEditChange(const QString& value)
{
	QLineEdit* lineBox = (QLineEdit*)sender();
	Parameter<std::string>* parameter = dynamic_cast<Parameter<std::string>*>(widgetParameterMap[lineBox]);
	parameter->SetValue(value.toStdString());
	qDebug() << parameter->GetValue().c_str();
}

void ParameterListWidget::OnComboBoxChange(int value)
{
	QComboBox* comboBox = (QComboBox*)sender();
	Parameter<int>* parameter = static_cast<Parameter<int>*>(widgetParameterMap[comboBox]);
	qDebug() << "on combobox change" << comboBox << parameter->GetName().c_str() << parameter->GetValue() << value;
	parameter->SetValue(value);
	qDebug() << parameter->GetValue();
}