#include "AnimationListWidget.h"

#include <QtWidgets>
#include "../Utils.h"
#include "QDebug"
#include "../EventManager.h"

AnimationListWidget::AnimationListWidget(std::string path) : QWidget(nullptr), path(path)
{
	std::string filename = Utils::FilenameFromPath(path, false, "/\\");
	
	QHBoxLayout* layout = new QHBoxLayout();
	QPushButton* loadButton = new QPushButton("Load");
	loadButton->setMaximumWidth(50);
	loadButton->setMinimumWidth(50);
	connect(loadButton, SIGNAL(clicked()), this, SLOT(OnLoadButtonPressed()));
	layout->addWidget(loadButton);
	layout->addWidget(new QLabel(filename.c_str()));
	setLayout(layout);
	name = filename.c_str();
}

void AnimationListWidget::OnLoadButtonPressed()
{
	EventManager::instance().FireEvent("OnLoadAnimationButtonPressed", path);
}