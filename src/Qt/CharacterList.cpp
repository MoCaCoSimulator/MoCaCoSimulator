#include "CharacterList.h"
#include <filesystem>
#include <qjsonobject.h>
#include "../Paths.h"
#include "../EventManager.h"

CharacterList::CharacterList(QWidget* parent) : QListWidget(parent)
{
	for (const auto& entry : std::filesystem::directory_iterator(CHARACTER_DIRECTORY))
	{
		QListWidgetItem* item = new QListWidgetItem(this);
		CharacterListItem* customItem = new CharacterListItem(entry.path().string());
		item->setSizeHint(customItem->minimumSizeHint());
		setItemWidget(item, customItem);

		itemWidgets.push_back(customItem);
		items.push_back(item);
	}

	std::function<void(std::string)> stringCallback = std::function([this](std::string value) { return this->OnLoadCharacterButtonPressed(value); });
	EventManager::instance().SubscribeToEvent("OnLoadCharacterButtonPressed", stringCallback);
}

QJsonObject CharacterList::SaveSelected() const
{
	QJsonObject json;

	json.insert("path", QJsonValue(selectedCharacter.c_str()));

	return json;
}

void CharacterList::LoadSelected(QJsonObject json)
{
	std::string path = json["path"].toString().toStdString();

	for (const auto& item : itemWidgets)
	{
		if (item->GetPath() == path)
		{
			item->OnLoadButtonPressed();
			break;
		}
	}
}

void CharacterList::Reset()
{
	clearSelection();
	clear();
	itemWidgets.clear();
	items.clear();
}

void CharacterList::OnLoadCharacterButtonPressed(std::string path)
{
	selectedCharacter = path;
}
