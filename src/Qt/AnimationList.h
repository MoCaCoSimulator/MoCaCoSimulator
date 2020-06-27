#pragma once

#include <QListWidget>
#include <vector>

#include "../Animation.h"
#include "AnimationListWidget.h"

class AnimationList : public QListWidget
{
	Q_OBJECT
private:
	std::vector<AnimationListWidget*> itemWidgets;
	std::vector<QListWidgetItem*> items;
	void OnLoadCharacterButtonPressed(std::string file);
	std::string loadedPath;
public:
	AnimationList(QWidget* parent);

	void LoadAnimations(std::string path);

	QJsonObject SaveSelected() const;
	void LoadSelected(QJsonObject json);

	std::vector<std::string> GetSelectedAnimationPaths();
	void Reset();
};