#include <SFML/Network.hpp>
#include "Client.h"
#include "Utilities/XMLParser.h"
#include <iostream>
#include <array>
#include <assert.h>

constexpr size_t MAX_CLIENTS = 4;

struct Faction
{
	Faction(FactionName factionName)
		:factionName(factionName),
		occupied(false)
	{}
	
	const FactionName factionName;
	sf::Vector2i spawnPosition;
	std::vector<eShipType> ships;
	bool occupied;
};

void broadcastMessage(std::vector<std::unique_ptr<Client>>& clients, sf::Packet& packetToSend)
{
	for (auto& client : clients)
	{
		client->getTcpSocket().send(packetToSend);
	}
}

void broadcastMessage(std::vector<std::unique_ptr<Client>>& clients, const ServerMessage& messageToSend)
{
	sf::Packet packetToSend;
	packetToSend << messageToSend;
	for (auto& client : clients)
	{
		client->getTcpSocket().send(packetToSend);
	}
}

void resetFaction(std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& factions, FactionName factionName)
{
	auto iter = std::find_if(factions.begin(), factions.end(), [factionName](const auto& faction) { return faction.factionName == factionName; });
	assert(iter != factions.end());

	iter->occupied = false;
}

FactionName getAvaiableFactionName(std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& factions)
{
	auto iter = std::find_if(factions.begin(), factions.end(), [](const auto& faction) { return !faction.occupied; });
	assert(iter != factions.end());
	iter->occupied = true;

	return iter->factionName;
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
	std::vector<std::unique_ptr<Client>> clients;
	clients.reserve(MAX_CLIENTS);
	std::array<Faction, static_cast<size_t>(FactionName::eTotal)> factions
	{
		FactionName::eYellow,
		FactionName::eBlue,
		FactionName::eGreen,
		FactionName::eRed
	};

	std::string levelName = "Level1.tmx";
	for (int i = 0; i < factions.size(); ++i)
	{
		factions[i].spawnPosition = XMLParser::parseFactionSpawnPoints(levelName)[i];
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
								existingFactions.emplace_back(faction.factionName, faction.ships);
							}
						}
						ServerMessage messageToSend(eMessageType::eEstablishConnection, availableFaction);
						messageToSend.existingFactions = existingFactions;
						sf::Packet packetToSend;
						packetToSend << messageToSend;
						if (newClient->send(packetToSend) != sf::Socket::Done)
						{
							std::cout << "Failed to establish connection\n";
							resetFaction(factions, availableFaction);
							continue;
						}

						std::cout << "New Client Added.\n";
						clients.push_back(std::make_unique<Client>(std::move(newClient), availableFaction));
						socketSelector.add(clients.back()->getTcpSocket());
					}
				}
			}
			else
			{
				for (auto& client : clients)
				{
					if (socketSelector.isReady(client->getTcpSocket()))
					{
						sf::Packet receivedPacket;
						std::cout << "received Packet From Client\n";

						if (client->getTcpSocket().receive(receivedPacket) == sf::Socket::Done)
						{
							ServerMessage receivedServerMessage;
							receivedPacket >> receivedServerMessage;
							if (receivedServerMessage.type == eMessageType::ePlayerReady)
							{
								++playersReady;
								if (playersReady == clients.size())
								{
									playersReady = 0;
									ServerMessage messageToSend(eMessageType::eStartOnlineGame);
									messageToSend.levelName = levelName;

									for (const auto& faction : factions)
									{
										messageToSend.spawnPositions.emplace_back(faction.factionName, 
											faction.spawnPosition.x, faction.spawnPosition.y);
									}

									broadcastMessage(clients, messageToSend);
								}
							}
							else if (receivedServerMessage.type == eMessageType::eNewPlayer)
							{
								addFactionShips(factions, receivedServerMessage.faction, receivedServerMessage.shipsToAdd);
								broadcastMessage(clients, receivedServerMessage);
							}
							else if (receivedServerMessage.type == eMessageType::eDisconnect)
							{
								resetFaction(factions, receivedServerMessage.faction);

								for (auto iter = clients.begin(); iter != clients.end(); ++iter)
								{
									if (receivedServerMessage.faction == iter->get()->getFactionName())
									{
										std::cout << "Client Removed\n";
										socketSelector.remove(client->getTcpSocket());
										clients.erase(iter);
										break;
									}
								}
							}
							else if (receivedServerMessage.type == eMessageType::eDeployShipAtPosition ||
								receivedServerMessage.type == eMessageType::eMoveShipToPosition ||
								receivedServerMessage.type == eMessageType::eAttackShipAtPosition)
							{
								broadcastMessage(clients, receivedServerMessage);
							}					
						}
					}
				}
			}
		}
	}

	return 0;
}