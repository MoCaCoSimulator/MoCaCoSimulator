#pragma once
#include <string>
#include <functional>
#include <map>
#include <vector>
#include "Utils.h"
#include "QDebug"

// Singleton
class EventManager
{
public:
	static EventManager& instance();

	template <typename T>
	void FireEvent(std::string eventName, T value)
	{
		std::map<std::string, std::vector<std::function<void(T)>>>* eventMap = get_listeners<T>();

		for each (std::pair<std::string, std::vector<std::function<void(T)>>> callback in (*eventMap))
		{
			if (callback.first == eventName)
			{
				for each (std::function<void(T)> function in callback.second)
				{
					function(value);
				}
			}
		}
	}

	void FireEvent(std::string eventName)
	{
		for each (std::pair<std::string, std::vector<std::function<void()>>> callback in voidEvents)
			if (callback.first == eventName)
				for each (std::function<void()> function in callback.second)
					function();
	}

	void SubscribeToEvent(std::string eventName, std::function<void()> callback)
	{
		// Map already contains event name
		if (Utils::Contains(voidEvents, eventName))
		{
			voidEvents[eventName].push_back(callback);
		}
		// Map does not have event name, creating it
		else
		{
			std::pair<std::string, std::vector<std::function<void()>>>* newEventList = new std::pair<std::string, std::vector<std::function<void()>>>();
			newEventList->first = eventName;
			newEventList->second.push_back(callback);
			voidEvents.insert(*newEventList);
		}
	}

	template <typename T>
	void SubscribeToEvent(std::string eventName, std::function<void(T)> callback)
	{
		std::map<std::string, std::vector<std::function<void(T)>>>* eventMap = get_listeners<T>();

		// Map already contains event name
		if (Utils::Contains(*eventMap, eventName))
		{
			(*eventMap)[eventName].push_back(callback);
		}
		// Map does not have event name, creating it
		else
		{
			std::pair<std::string, std::vector<std::function<void(T)>>>* newEventList = new std::pair<std::string, std::vector<std::function<void(T)>>>();
			newEventList->first = eventName;
			newEventList->second.push_back(callback);
			eventMap->insert(*newEventList);
		}
	}
private:
	EventManager();

	std::map<std::string, std::vector<std::function<void()>>> voidEvents;

	template <typename T>
	static std::map<std::string, std::vector<std::function<void(T)>>>* get_listeners()
	{
		static std::map<std::string, std::vector<std::function<void(T)>>> listeners;
		return &listeners;
	}
};
