#include <SFML/Network.hpp> 
#include "Utilities/XMLParser.h"
#include "Global.h"
#include <iostream>
#include <array>
#include <assert.h>

constexpr size_t MAX_CLIENTS = 4;

struct Faction
{
	Faction(FactionName factionName, bool AIControlled = false)
		:factionName(factionName),
		occupied(false),
		AIControlled(AIControlled)
	{
		if(AIControlled)
		{
			occupied = true;
		}
	}
	
	const FactionName factionName;
	std::unique_ptr<sf::TcpSocket> m_tcpSocket;
	sf::Vector2i spawnPosition;
	std::vector<eShipType> ships;
	bool occupied;
	bool AIControlled;
};

void loadAIShips(Faction& faction)
{
	assert(faction.AIControlled);

	int randomNumber{ std::rand() % 8 };
	int numSideCannons{ 0 };
	int numTurtle{ 0 };
	int numFlame{ 0 };
	int numSniper{ 0 };

	switch (randomNumber)
	{
	case 0:
		numFlame = 4;
		numSideCannons = 2;
		break;
	case 1:
		numTurtle = 2;
		numSideCannons = 2;
		numSniper = 2;
		break;
	case 2:
		numTurtle = 3;
		numSideCannons = 3;
		break;
	case 3:
		numFlame = 2;
		numSideCannons = 2;
		numTurtle = 2;
		break;
	case 4:
		numSniper = 4;
		numSideCannons = 2;
		break;
	case 5:
		numFlame = 3;
		numSideCannons = 3;
		break;
	case 6:
		numSideCannons = 2;
		numTurtle = 1;
		numFlame = 2;
		numSniper = 1;
		break;
	case 7:
		numFlame = 4;
		numSniper = 2;
		break;
	}

	assert(numSideCannons + numTurtle + numFlame + numSniper < 7);
	for (int i = 0; i < numFlame; ++i)
	{
		faction.ships.push_back(eShipType::eFire);
	}
	for (int i = 0; i < numSideCannons; ++i)
	{
		faction.ships.push_back(eShipType::eFrigate);
	}
	for (int i = 0; i < numTurtle; ++i)
	{
		faction.ships.push_back(eShipType::eTurtle);
	}
	for (int i = 0; i < numSniper; ++i)
	{
		faction.ships.push_back(eShipType::eSniper);
	}
}

void broadcastMessage(std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& factions, const ServerMessage& messageToSend)
{
	sf::Packet packetToSend;
	packetToSend << messageToSend;
	for (auto& faction : factions)
	{
		if (faction.occupied && !faction.AIControlled)
		{
			faction.m_tcpSocket->send(packetToSend);
		}
	}
}

void resetFaction(std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& factions, FactionName factionName,
	sf::SocketSelector& socketSelector)
{
	auto iter = std::find_if(factions.begin(), factions.end(), [factionName](const auto& faction) { return faction.factionName == factionName; });
	assert(iter != factions.end());

	iter->occupied = false;
	socketSelector.remove(*iter->m_tcpSocket);
	iter->m_tcpSocket.reset();
}

FactionName getAvaiableFactionName(std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& factions)
{
	auto iter = std::find_if(factions.begin(), factions.end(), [](const auto& faction) { return !faction.occupied; });
	assert(iter != factions.end());
	iter->occupied = true;

	return iter->factionName;
}

int numbOfHumanPlayers(const std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& factions)
{
	int humanCount = 0;
	for (const auto& faction : factions)
	{
		if (!faction.AIControlled && !faction.ships.empty())
		{
			++humanCount;
		}
	}

	return humanCount;
}

bool isServerFull(const std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& factions)
{
	bool serverFull = true;
	for (const auto& faction : factions)
	{
		if (!faction.occupied)
		{
			serverFull = false;
		}
	}

	return serverFull;
}

void addFactionShips(std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& factions, FactionName factionName, 
	std::vector<eShipType>& shipsToAdd)
{
	factions[static_cast<int>(factionName)].ships = shipsToAdd;
}

int main()
{
	sf::TcpListener tcpListener;
	tcpListener.listen(55001);
	sf::SocketSelector socketSelector;
	socketSelector.add(tcpListener);
	std::array<Faction, static_cast<size_t>(FactionName::eTotal)> factions
	{
		FactionName::eYellow,
		FactionName::eBlue,
	   {FactionName::eGreen, true},
	   {FactionName::eRed, true}
	};

	std::string levelName = "Level1.tmx";
	for (int i = 0; i < factions.size(); ++i)
	{
		factions[i].spawnPosition = XMLParser::parseFactionSpawnPoints(levelName)[i];
	}

	for (auto& faction : factions)
	{
		if (faction.AIControlled)
		{
			loadAIShips(faction);
		}
	}

	std::cout << "Started Listening\n";
	int playersReady = 0;
	bool serverRunning = true;
	while (serverRunning)
	{
		if (socketSelector.wait())
		{
			if (socketSelector.isReady(tcpListener))
			{
				std::unique_ptr<sf::TcpSocket> newClient = std::make_unique<sf::TcpSocket>();
				if (tcpListener.accept(*newClient) == sf::Socket::Done)
				{
					if (isServerFull(factions))
					{
						std::cout << "Refused Connection\n";
						sf::Packet packetToSend;
						ServerMessage messageToSend(eMessageType::eRefuseConnection);
						packetToSend << messageToSend;
						if (newClient->send(packetToSend) != sf::Socket::Done)
						{
							std::cout << "Failed to send client message\n";
						}
					}
					else
					{
						//Assign new client to faction
						FactionName availableFaction = getAvaiableFactionName(factions);
						std::vector<ServerMessageExistingFaction> existingFactions;
						for (const auto& faction : factions)
						{
							if (faction.factionName != availableFaction)
							{
								existingFactions.emplace_back(faction.factionName, faction.ships, faction.AIControlled);
							}
						}
						ServerMessage messageToSend(eMessageType::eEstablishConnection, availableFaction);
						messageToSend.existingFactions = existingFactions;
						sf::Packet packetToSend;
						packetToSend << messageToSend;
						if (newClient->send(packetToSend) != sf::Socket::Done)
						{
							std::cout << "Failed to establish connection\n";
							resetFaction(factions, availableFaction, socketSelector);
							continue;
						}

						std::cout << "New Client Added.\n";
						factions[static_cast<int>(availableFaction)].m_tcpSocket = std::move(newClient);
						factions[static_cast<int>(availableFaction)].m_tcpSocket;
						socketSelector.add(*factions[static_cast<int>(availableFaction)].m_tcpSocket);
					}
				}
			}
			else
			{
				for (auto& faction : factions)
				{
					if (socketSelector.isReady(*faction.m_tcpSocket))
					{
						sf::Packet receivedPacket;
						std::cout << "received Packet From Client\n";

						if (faction.m_tcpSocket->receive(receivedPacket) == sf::Socket::Done)
						{
							ServerMessage receivedServerMessage;
							receivedPacket >> receivedServerMessage;
							if (receivedServerMessage.type == eMessageType::ePlayerReady)
							{
								++playersReady;
								if (playersReady == numbOfHumanPlayers(factions))
								{
									playersReady = 0;
									ServerMessage messageToSend(eMessageType::eStartOnlineGame);
									messageToSend.levelName = levelName;

									for (const auto& faction : factions)
									{
										messageToSend.spawnPositions.emplace_back(faction.factionName,
											faction.spawnPosition.x, faction.spawnPosition.y);
									}

									broadcastMessage(factions, messageToSend);
								}
							}
							else if (receivedServerMessage.type == eMessageType::eNewPlayer)
							{
								addFactionShips(factions, receivedServerMessage.faction, receivedServerMessage.shipsToAdd);
								broadcastMessage(factions, receivedServerMessage);
							}
							else if (receivedServerMessage.type == eMessageType::eDisconnect)
							{
								resetFaction(factions, receivedServerMessage.faction, socketSelector);

								ServerMessage messageToSend(eMessageType::eClientDisconnected, receivedServerMessage.faction);
								broadcastMessage(factions, messageToSend);
							}
							else if (receivedServerMessage.type == eMessageType::eDeployShipAtPosition ||
								receivedServerMessage.type == eMessageType::eMoveShipToPosition ||
								receivedServerMessage.type == eMessageType::eAttackShipAtPosition)
							{
								broadcastMessage(factions, receivedServerMessage);
							}
						}
					}
				}
			}
		}
	}

	return 0;
}