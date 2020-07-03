#pragma once

#include <QWidget>
#include <string>

class CharacterListItem : public QWidget
{
	Q_OBJECT
private:
	std::string path;
	std::string name;
public:
	CharacterListItem(std::string path);

	std::string GetPath() { return path; }
	std::string GetName() { return name; }
public slots:
	void OnLoadButtonPressed();
};