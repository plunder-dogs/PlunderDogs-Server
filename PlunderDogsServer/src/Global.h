#pragma once
#include <utility>
#include <string>
#include <SFML/Graphics.hpp>
#include <vector>

constexpr int INVALID_SHIP_ID = -1;
constexpr float DRAW_OFFSET_X{ 0 };
constexpr float DRAW_OFFSET_Y{ 8 };

const sf::Vector2u SCREEN_RESOLUTION{ 1680, 1050 };
const size_t MAX_SHIPS_PER_FACTION = 6;
const std::string DATA_DIRECTORY = "Data\\";
const std::string SHIP_DATA_DIRECTORY = "Data\\Ships\\";
const std::string LEVEL_DATA_DIRECTORY = "Data\\Levels\\";
const sf::Vector2i MOUSE_POSITION_OFFSET{ 25, 45 };

//enum class eGameMessage
//{
//	eDeployAtPosition = 0,
//	eMoveToPosition,
//	eFireAtPosition
//};
//
//struct Message
//{
//	Message(eGameMessage message, ShipOnTile shipOnTile, sf::Vector2i target)
//		: message(message),
//		shipOnTile(shipOnTile),
//		target(target)
//	{}
//
//	eGameMessage message;
//	ShipOnTile shipOnTile;
//	sf::Vector2f target;
//};

enum eFactionName
{
	eYellow = 0,
	eBlue = 1,
	eGreen = 2,
	eRed = 3,
	eTotal = eRed + 1
};

enum eDirection
{
	eNorth,
	eNorthEast,
	eSouthEast,
	eSouth,
	eSouthWest,
	eNorthWest,
	Max = eNorthWest
};

enum OverWorldWindow
{
	eBattle = 0,
	eMainMenu,
	eShipSelection,
	eLevelSelection,
	ePlayerSelection,
	eUpgrade
};

enum BattlePhase
{
	Deployment = 0,
	Movement,
	Attack
};

enum eTileType
{
	eGrass = 0,
	eSparseForest,
	eForest,
	eFoothills,
	eWoodedFoothills,
	eMountain,
	eSea,
	eOcean,
	eGrasslandTown,
	eWalledGrasslandTown,
	eStoneGrasslandTown,
	eFarm,
	eWoodedSwamp,
	eSwampPools,
	eSwamp,
	eSwampWater,
	eSnow,
	eSparseSnowForest,
	eSnowForest,
	eSnowFoothills,
	eSnowWoodedFoothills,
	eIceburgs,
	eSnowTown,
	eSnowCastle,
	eSand,
	eSandFoothills,
	eSandDunes,
	eMesa,
	eOasis,
	eSandTown,
	eWalledSandTown,
	eJungle,
	eLeftPort,
	eRightPort,
	eLighthouse,
	eGrasslandRuin,
	eSwampRuins
};

enum eShipSpriteFrame
{
	eMaxHealth = 0,
	eLowDamage,
	eHighDamage,
	eDead
};

enum class eShipType
{
	eFrigate,
	eTurtle,
	eFire,
	eSniper
};

enum eLightIntensity
{
	eMaximum = 0,
	//eHigh,
	//eLow,
	eMinimum
};

struct ShipOnTile
{
	ShipOnTile()
		: factionName(),
		shipID(INVALID_SHIP_ID)
	{}
	ShipOnTile(eFactionName factionName, int shipID)
		: factionName(factionName),
		shipID(shipID)
	{}

	bool operator==(const ShipOnTile& orig)
	{
		return (factionName == orig.factionName && shipID == orig.shipID);
	}

	bool isValid() const
	{
		return (shipID != INVALID_SHIP_ID);
	}

	void clear()
	{
		shipID = INVALID_SHIP_ID;
	}

	eFactionName factionName;
	int shipID;
};

enum class eMessageType
{
	eEstablishConnection = 0,
	eRefuseConnection,
	eNewPlayer,
	ePlayerReady,
	eStartOnlineGame,
	eDeployShipAtPosition,
	eMoveShipToPosition,
	eAttackShipAtPosition,
	eDisconnect,
	eClientDisconnected,
	eRemotePlayerReady
};

struct ServerMessageShipAction
{
	ServerMessageShipAction(int shipID, int x, int y)
		: shipID(shipID),
		position(x, y)
	{}

	ServerMessageShipAction(int shipID, int x, int y, eDirection direction)
		: shipID(shipID),
		position(x, y),
		direction(direction)
	{}

	int shipID;
	sf::Vector2i position;
	eDirection direction;
};

struct ServerMessageSpawnPosition
{
	ServerMessageSpawnPosition(eFactionName factionName, int x, int y)
		:factionName(factionName),
		position(x, y)
	{}

	eFactionName factionName;
	sf::Vector2i position;
};

struct ServerMessageExistingFaction
{
	ServerMessageExistingFaction(eFactionName factionName, std::vector<eShipType>&& existingShips, bool AIControlled, bool ready)
		: factionName(factionName),
		ready(ready),
		AIControlled(AIControlled),
		existingShips(std::move(existingShips))
	{}

	ServerMessageExistingFaction(eFactionName factionName, const std::vector<eShipType>& existingShips, bool AIControlled, bool ready)
		: factionName(factionName),
		ready(ready),
		AIControlled(AIControlled),
		existingShips(existingShips)
	{}

	eFactionName factionName;
	bool ready;
	bool AIControlled;
	std::vector<eShipType> existingShips;
};

struct ServerMessage
{
	ServerMessage()
	{}

	ServerMessage(eMessageType type)
		: type(type)
	{}

	ServerMessage(eMessageType type, eFactionName factionName)
		: type(type),
		faction(factionName)
	{}

	ServerMessage(eMessageType type, eFactionName factionName, std::vector<eShipType>&& shipsToAdd)
		: type(type),
		faction(factionName),
		shipsToAdd(std::move(shipsToAdd))
	{}

	eMessageType type;
	eFactionName faction;
	std::string levelName;
	std::vector<eShipType> shipsToAdd;
	std::vector<ServerMessageShipAction> shipActions;
	std::vector<ServerMessageSpawnPosition> spawnPositions;
	std::vector<ServerMessageExistingFaction> existingFactions;
};
