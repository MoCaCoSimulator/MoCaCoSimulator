#pragma once
#include "Parameter.h"
#include "Matrix.h"

static class QJsonSerializer
{
public:

	static QJsonValue ParameterToJson(BaseParameter* parameter)
	{
		const type_info& paramType = parameter->GetType();
		if (paramType == typeid(bool))
		{
			Parameter<bool>* boolParam = dynamic_cast<Parameter<bool>*>(parameter);
			return QJsonValue(boolParam->GetValue());
		}
		if (paramType == typeid(int))
		{
			Parameter<int>* intParam = dynamic_cast<Parameter<int>*>(parameter);
			return QJsonValue(intParam->GetValue());
		}
		if (paramType == typeid(float))
		{
			Parameter<float>* floatParam = dynamic_cast<Parameter<float>*>(parameter);
			return QJsonValue(floatParam->GetValue());
		}
		if (paramType == typeid(double))
		{
			Parameter<double>* doubleParam = dynamic_cast<Parameter<double>*>(parameter);
			return QJsonValue(doubleParam->GetValue());
		}
		if (paramType == typeid(std::string))
		{
			Parameter<std::string>* stringParam = dynamic_cast<Parameter<std::string>*>(parameter);
			return QJsonValue(stringParam->GetValue().c_str());
		}
		if (paramType == typeid(Vector3))
		{
			Parameter<Vector3>* vecParam = dynamic_cast<Parameter<Vector3>*>(parameter);
			return Vector3ToJson(vecParam->GetValue());
		}
		if (std::string(paramType.name()).find("enum ") != std::string::npos)
		{
			Parameter<int>* intParam = static_cast<Parameter<int>*>(parameter);
			return QJsonValue(intParam->GetValue());
		}
	}

	static void JsonToParameter(BaseParameter* parameter, const QJsonValue& json)
	{
		const type_info& paramType = parameter->GetType();
		if (paramType == typeid(bool))
		{
			Parameter<bool>* boolParam = dynamic_cast<Parameter<bool>*>(parameter);
			boolParam->SetValue(json.toBool());
		}
		else if (paramType == typeid(int))
		{
			Parameter<int>* intParam = dynamic_cast<Parameter<int>*>(parameter);
			intParam->SetValue(json.toInt());
		}
		else if (paramType == typeid(float))
		{
			Parameter<float>* floatParam = dynamic_cast<Parameter<float>*>(parameter);
			floatParam->SetValue(json.toDouble());
		}
		else if (paramType == typeid(double))
		{
			Parameter<double>* doubleParam = dynamic_cast<Parameter<double>*>(parameter);
			doubleParam->SetValue(json.toDouble());
		}
		else if (paramType == typeid(std::string))
		{
			Parameter<std::string>* stringParam = dynamic_cast<Parameter<std::string>*>(parameter);
			stringParam->SetValue(json.toString().toStdString());
		}
		else if (paramType == typeid(Vector3))
		{
			Parameter<Vector3>* vecParam = dynamic_cast<Parameter<Vector3>*>(parameter);
			vecParam->SetValue(JsonToVector3(json.toArray()));
		}
		else if (std::string(paramType.name()).find("enum ") != std::string::npos)
		{
			Parameter<int>* intParam = static_cast<Parameter<int>*>(parameter);
			intParam->SetValue(json.toInt());
		}
	}

	static QJsonObject ParameterMapToJson(const std::map<QWidget*, BaseParameter*>& widgetParameterMap)
	{
		QJsonObject settings;
		for (auto const& kv : widgetParameterMap)
		{
			const BaseParameter* parameter = kv.second;
			QString name = QString(parameter->GetName().c_str());
			QJsonValue value = QJsonSerializer::ParameterToJson((BaseParameter*)parameter);
			settings.insert(name, value);
		}
		return settings;
	}

	static void JsonToParameterMap(const std::map<QWidget*, BaseParameter*>& widgetParameterMap, const QJsonObject& settings)
	{
		for (auto const& kv : widgetParameterMap)
		{
			BaseParameter* parameter = kv.second;
			QJsonSerializer::JsonToParameter(parameter, settings[parameter->GetName().c_str()]);
		}
	}

	static QJsonArray MatrixToJson(const Matrix& transform)
	{
		QJsonArray json;
		json.append(QJsonValue(transform.m00));
		json.append(QJsonValue(transform.m01));
		json.append(QJsonValue(transform.m02));
		json.append(QJsonValue(transform.m03));
		json.append(QJsonValue(transform.m10));
		json.append(QJsonValue(transform.m11));
		json.append(QJsonValue(transform.m12));
		json.append(QJsonValue(transform.m13));
		json.append(QJsonValue(transform.m20));
		json.append(QJsonValue(transform.m21));
		json.append(QJsonValue(transform.m22));
		json.append(QJsonValue(transform.m23));
		json.append(QJsonValue(transform.m30));
		json.append(QJsonValue(transform.m31));
		json.append(QJsonValue(transform.m32));
		json.append(QJsonValue(transform.m33));
		return json;
	}
	static Matrix JsonToMatrix(const QJsonArray& json)
	{
		return Matrix(
			json[0].toDouble(),json[1].toDouble(),json[2].toDouble(),json[3].toDouble(),
			json[4].toDouble(),json[5].toDouble(),json[6].toDouble(),json[7].toDouble(),
			json[8].toDouble(),json[9].toDouble(),json[10].toDouble(),json[11].toDouble(),
			json[12].toDouble(),json[13].toDouble(),json[14].toDouble(),json[15].toDouble()
		);
	}
	static QJsonArray Vector3ToJson(const Vector3& vec)
	{
		QJsonArray json;
		json.append(vec.x);
		json.append(vec.y);
		json.append(vec.z);
		return json;
	}
	static Vector3 JsonToVector3(const QJsonArray& json)
	{
		return Vector3(
			json[0].toDouble(),
			json[1].toDouble(),
			json[2].toDouble()
		);
	}
	static QJsonArray Vector2ToQJsonObject(const Vector3& vec)
	{
		QJsonArray json;
		json.append(vec.x);
		json.append(vec.y);
		return json;
	}
	static Vector2 JsonToVector2(const QJsonArray& json)
	{
		return Vector2(
			json[0].toDouble(),
			json[1].toDouble()
		);
	}
	static QJsonArray QuaterionToQJsonObject(const Quaternion& vec)
	{
		QJsonArray json;
		json.append(vec.x);
		json.append(vec.y);
		json.append(vec.z);
		json.append(vec.w);
		return json;
	}
	static Quaternion JsonToQuaternion(const QJsonArray& json)
	{
		return Quaternion(
			json[0].toDouble(),
			json[1].toDouble(),
			json[2].toDouble(),
			json[3].toDouble()
		);
	}
};

