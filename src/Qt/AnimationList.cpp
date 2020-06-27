#include "AnimationList.h"
#include "QDebug"
#include <filesystem>
#include <iostream>
#include "../Paths.h"
#include <set>
#include "AnimationListWidget.h"
#include <qjsonarray.h>
#include <qjsonobject.h>
#include "../QJsonSerializer.h"
#include "../EventManager.h"
#include <QtWidgets>

AnimationList::AnimationList(QWidget* parent) : QListWidget(parent)
{
	// Make multiple items selectable
	setSelectionMode(QAbstractItemView::SelectionMode::MultiSelection);

	LoadAnimations(CHARACTER_DIRECTORY "David/animations");

	std::function<void(std::string)> stringCallback = std::function([this](std::string value) { return this->OnLoadCharacterButtonPressed(value); });
	EventManager::instance().SubscribeToEvent("OnLoadCharacterButtonPressed", stringCallback);
}

void AnimationList::LoadAnimations(std::string path)
{
	if (loadedPath == path)
		return;

	loadedPath = path;
	Reset();
	setUniformItemSizes(true);

	setLayoutMode(QListWidget::Batched);
	setBatchSize(10);
	
	auto dirIter = std::filesystem::directory_iterator(path);
	int numFiles = std::distance(dirIter, std::filesystem::directory_iterator{});
	QProgressDialog progress = QProgressDialog("Loading animations...", "Abort", 0, numFiles, window());
	progress.setWindowModality(Qt::WindowModal);
	QCoreApplication::processEvents();

	static QTime rateTimer;
	rateTimer.start();

	int counter = 0;
	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		if (rateTimer.elapsed() > 10000 / 60) {
			std::stringstream ss;
			ss << "Loading animation " << counter << "/" << numFiles << ": " << entry.path();
			progress.setLabelText(ss.str().c_str());
			progress.setValue(counter);
			QCoreApplication::processEvents();
			rateTimer.restart();
		}

		QListWidgetItem* item = new QListWidgetItem(this);
		AnimationListWidget* customItem = new AnimationListWidget(entry.path().string());
		item->setSizeHint(customItem->minimumSizeHint());
		setItemWidget(item, customItem);

		itemWidgets.push_back(customItem);
		items.push_back(item);
		counter++;
	}
}

QJsonObject AnimationList::SaveSelected() const
{
	QJsonObject json;

	QJsonArray selected;
	for (auto& selectedIndex : selectedIndexes()) {
		qDebug() << selectedIndex.row();
		qDebug() << itemWidgets.size();
		qDebug() << itemWidgets[selectedIndex.row()];
		selected.append(QString(itemWidgets[selectedIndex.row()]->GetPath().c_str()));
	}
	json.insert("selected", selected);

	return json;
}

void AnimationList::LoadSelected(QJsonObject json)
{
	for (auto& j : json["selected"].toArray())
	{
		std::string path = j.toString().toStdString();
		for (const auto& item : items)
		{
			if (path != dynamic_cast<AnimationListWidget*>(itemWidget(item))->GetPath())
				continue;

			setItemSelected(item, true);
			break;
		}
	}
}

std::vector<std::string> AnimationList::GetSelectedAnimationPaths()
{
	std::vector<std::string> paths;
	for (auto& selectedIndex : selectedIndexes())
		paths.push_back(itemWidgets[selectedIndex.row()]->GetPath());
	return paths;
}

void AnimationList::Reset()
{
	clearSelection();
	clear();
	itemWidgets.clear();
	items.clear();
}

void AnimationList::OnLoadCharacterButtonPressed(std::string path)
{
	LoadAnimations(path + "/animations");
}