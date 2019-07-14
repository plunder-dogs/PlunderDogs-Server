#pragma once

#include <vector>
#include <SFML/Graphics.hpp>
#include <utility>

enum FactionName
{
	eYellow = 0,
	eBlue = 1,
	eGreen = 2,
	eRed = 3,
	eTotal = eRed + 1
};

enum class eMessageType
{
	eEstablishConnection = 0,
	eRefuseConnection,
	eNewPlayer,
	ePlayerReady,
	eStartGame,
	eDeployShipAtPosition,
	eMoveShipToPosition,
	eAttackShipAtPosition,
	eDisconnect
};

enum class eShipType
{
	eFrigate,
	eTurtle,
	eFire,
	eSniper
};

struct ServerMessage
{
	ServerMessage()
	{}

	ServerMessage(eMessageType type)
		: type(type),
		faction(),
		ships(),
		positions()
	{}

	ServerMessage(eMessageType type, FactionName factionName)
		: type(type),
		faction(factionName),
		ships(),
		positions()
	{}

	ServerMessage(eMessageType type, FactionName factionName, std::vector<eShipType> shipsToAdd)
		: type(type),
		faction(faction),
		shipsToAdd(shipsToAdd)
	{}

	ServerMessage(eMessageType type, FactionName factionName, std::vector<int>&& ships, std::vector<sf::Vector2i>&& positions)
		: type(type),
		faction(factionName),
		ships(std::move(ships)),
		positions(std::move(positions))
	{}

	eMessageType type;
	FactionName faction;
	std::vector<int> ships;
	std::vector<sf::Vector2i> positions;
	std::vector<eShipType> shipsToAdd;
};