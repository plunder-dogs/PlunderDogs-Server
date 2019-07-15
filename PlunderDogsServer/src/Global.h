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

enum FactionName
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

enum eControllerType
{
	eLocalPlayer = 0,
	eAI,
	eRemotePlayer,
	None
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
	ShipOnTile(FactionName factionName, int shipID)
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

	FactionName factionName;
	int shipID;
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

struct ServerMessageShipAction
{
	ServerMessageShipAction(int shipID, int x, int y)
		: shipID(shipID),
		position(x, y)
	{}

	int shipID;
	sf::Vector2i position;
};

struct ServerMessageSpawnPosition
{
	ServerMessageSpawnPosition(FactionName factionName, int x, int y)
		:factionName(factionName),
		position(x, y)
	{}

	FactionName factionName;
	sf::Vector2i position;
};

struct ServerMessage
{
	ServerMessage()
	{}

	ServerMessage(eMessageType type)
		: type(type)
	{}

	ServerMessage(eMessageType type, FactionName factionName)
		: type(type)
	{}

	ServerMessage(eMessageType type, FactionName factionName, std::vector<eShipType>&& shipsToAdd)
		: type(type),
		faction(factionName),
		shipsToAdd(std::move(shipsToAdd))
	{}


	eMessageType type;
	FactionName faction;
	std::string levelName;
	std::vector<eShipType> shipsToAdd;
	std::vector<ServerMessageShipAction> shipActions;
	std::vector<ServerMessageSpawnPosition> spawnPositions;
};