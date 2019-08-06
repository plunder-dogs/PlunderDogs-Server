#pragma once

#include "Global.h"
#include <memory>
#include <vector>
#include <SFML/Network.hpp>

enum class eFactionControllerType
{
	None = 0,
	RemoteClient,
	AI
};

struct Faction
{
	Faction(FactionName factionName, eFactionControllerType controllerType = eFactionControllerType::None)
		:factionName(factionName),
		controllerType(controllerType)
	{}

	const FactionName factionName;
	std::unique_ptr<sf::TcpSocket> m_tcpSocket;
	sf::Vector2i spawnPosition;
	std::vector<eShipType> ships;
	std::vector<ServerMessage> m_messageBackLog;
	eFactionControllerType controllerType;
};