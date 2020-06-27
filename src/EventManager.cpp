#include "EventManager.h"
#include "Utils.h"

EventManager::EventManager() { }

EventManager& EventManager::instance()
{
	static EventManager* instance = new EventManager();
	return *instance;
}