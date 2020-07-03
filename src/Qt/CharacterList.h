#pragma once

#include <QListWidget>
#include <vector>

#include "CharacterListItem.h"

class CharacterList : public QListWidget
{
	Q_OBJECT
private:
	std::vector<CharacterListItem*> itemWidgets;
	std::vector<QListWidgetItem*> items;
	std::string selectedCharacter;

	void OnLoadCharacterButtonPressed(std::string path);
public:
	CharacterList(QWidget* parent);

	QJsonObject SaveSelected() const;
	void LoadSelected(QJsonObject json);
	void Reset();
	//std::vector<std::string> GetSelectedAnimationPaths();
};