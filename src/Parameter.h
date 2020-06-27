#pragma once

#include <string>
#include <typeinfo>
#include <vector>
#include <map>
#include <type_traits>
#include <QDebug>

class BaseParameter
{
public:
	virtual ~BaseParameter() {}

	template<class T, class U> void SetValue(const U& newValue);
	template<class T> const T& GetValue() const;
	const std::string& GetName() const { return name; }
	void SetName(const std::string& newName) { name = newName; }
	const std::type_info& GetType() const { return type; }
protected:
	BaseParameter(std::string name, const std::type_info& type) : name(name), type(type) {};
	BaseParameter(const BaseParameter& parameter) : name(parameter.name), type(parameter.type) {};
	std::string name;
	const std::type_info& type;
};

template <typename T>
class Parameter : public BaseParameter
{
public:
	virtual ~Parameter() {}

	Parameter(const std::string& n, const T& v) : value(v), BaseParameter(n, typeid(T)) { }
	Parameter(const Parameter<T>& parameter) : value(parameter.value), BaseParameter(parameter.name, typeid(T)) { }
	const T& GetValue() const { return value; }
	void SetValue(const T& newValue) { value = newValue; }
protected:
	T value;
};

//Here's the trick: dynamic_cast rather than virtual
template<class T> const T& BaseParameter::GetValue() const
{
	return dynamic_cast<const Parameter<T>&>(*this).GetValue();
}
template<class T, class U> void BaseParameter::SetValue(const U& newValue)
{
	return dynamic_cast<Parameter<T>&>(*this).SetValue(newValue);
}