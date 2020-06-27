
#include "CharacterListItem.h"

#include <QtWidgets>
#include <filesystem>
#include "../EventManager.h"
#include "../Utils.h"
#include "../Paths.h"

CharacterListItem::CharacterListItem(std::string path) : QWidget(nullptr), path(path)
{
	std::string foldername = Utils::FoldernameFromPath(path, "/\\");

	QHBoxLayout* layout = new QHBoxLayout();
	QPushButton* loadButton = new QPushButton("Load");
	loadButton->setMaximumWidth(50);
	loadButton->setMinimumWidth(50);
	connect(loadButton, SIGNAL(clicked()), this, SLOT(OnLoadButtonPressed()));
	layout->addWidget(loadButton);
	layout->addWidget(new QLabel(foldername.c_str()));
	setLayout(layout);
	name = foldername.c_str();
}

void CharacterListItem::OnLoadButtonPressed()
{
	EventManager::instance().FireEvent("OnLoadCharacterButtonPressed", path);
}