#include <SFML/Network.hpp>
#include "Client.h"
#include <vector>
#include <iostream>
#include <array>
#include <assert.h>

constexpr size_t MAX_CLIENTS = 4;

enum class eMessageType
{
	eEstablishConnection = 0,
	eNewPlayer,
	ePlayerReady,
	eStartGame,
	eDeployShip
};

struct Faction
{
	Faction(FactionName factionName)
		:factionName(factionName),
		occupied(false)
	{
	}

	const FactionName factionName;
	bool occupied;
};

void broadcastMessage(std::vector<std::unique_ptr<Client>>& clients, sf::Packet& packetToSend)
{
	for (auto& client : clients)
	{
		client->getTcpSocket().send(packetToSend);
	}
}

void broadcastMessage(std::vector<std::unique_ptr<Client>>& clients, eMessageType messageType, FactionName newFaction)
{
	for (auto& client : clients)
	{
		sf::Packet packetToSend;
		packetToSend << static_cast<int>(messageType) << static_cast<int>(newFaction);
		client->getTcpSocket().send(packetToSend);
	}
}

void broadcastMessage(std::vector<std::unique_ptr<Client>>& clients, eMessageType messageType)
{
	for (auto& client : clients)
	{
		sf::Packet packetToSend;
		packetToSend << static_cast<int>(messageType);
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

bool isFactionAvailable(std::array<Faction, static_cast<size_t>(FactionName::eTotal)>& factions)
{
	auto cIter = std::find_if(factions.cbegin(), factions.cend(), [](const auto& faction) { return !faction.occupied; });
	return cIter != factions.cend();
}

int main()
{
	bool serverRunning = true;
	int playersReady = 0;
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

	std::cout << "Started Listening\n";

	while (serverRunning)
	{
		if (socketSelector.wait())
		{
			if (socketSelector.isReady(tcpListener) && isFactionAvailable(factions))
			{
				std::unique_ptr<sf::TcpSocket> newClient = std::make_unique<sf::TcpSocket>();
				if (tcpListener.accept(*newClient) == sf::Socket::Done)
				{
					FactionName availableFaction = getAvaiableFactionName(factions);
					sf::Packet packetToSend;
					//Assign new client to faction
					packetToSend << static_cast<int>(eMessageType::eEstablishConnection) << static_cast<int>(availableFaction);
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
			else
			{
				for (auto& client : clients)
				{
					if (socketSelector.isReady(client->getTcpSocket()))
					{
						sf::Packet receivedPacket;
						if (client->getTcpSocket().receive(receivedPacket) == sf::Socket::Done)
						{
							int messageType = -1;
							receivedPacket >> messageType;
							if (static_cast<eMessageType>(messageType) == eMessageType::ePlayerReady)
							{
								int factionName = -1;
								receivedPacket >> factionName;
								++playersReady;
								if (playersReady == clients.size())
								{
									broadcastMessage(clients, eMessageType::eStartGame);
								}
							}
							else if (static_cast<eMessageType>(messageType) == eMessageType::eNewPlayer)
							{
								broadcastMessage(clients, receivedPacket);
							}
						}
					}
				}
			}
		}
	}

	return 0;
}