#include "Server.h"
#include "Utilities/XMLParser.h"
#include <algorithm>
#include <assert.h>
#include <iostream>

void loadAIShips(Faction& faction)
{
	assert(faction.controllerType == eFactionControllerType::AI);

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

Server::Server(std::string&& startingLevel)
	: m_tcpListener(std::make_unique<sf::TcpListener>()),
	m_socketSelector(),
	m_running(),
	m_remoteClientsReady(0),
	m_levelName(std::move(startingLevel))
{
	for (int i = 0; i < m_factions.size(); ++i)
	{
		m_factions[i].spawnPosition = XMLParser::parseFactionSpawnPoints(m_levelName)[i];
		if (m_factions[i].controllerType == eFactionControllerType::AI)
		{
			loadAIShips(m_factions[i]);
		}
	}
}

std::unique_ptr<Server> Server::create(std::string&& startingLevel, int portNumber)
{
	Server server(std::move(startingLevel));
	if (server.m_tcpListener->listen(portNumber) == sf::Socket::Done)
	{
		server.m_socketSelector.add(*server.m_tcpListener);
		server.m_running = true;
		std::cout << "Server on port:" << portNumber << " started listening.\n";
		return std::make_unique<Server>(std::move(server));
	}
	else
	{
		return std::unique_ptr<Server>();
	}
}

void Server::listen()
{
	while (m_running)
	{
		handleFactionMessageBackLog();

		if (m_socketSelector.wait())
		{
			if (m_socketSelector.isReady(*m_tcpListener))
			{
				listenForNewClient();
			}
			else
			{
				listenForServerMessages();
			}
		}
	}
}

void Server::handleFactionMessageBackLog()
{
	for (auto& faction : m_factions)
	{
		if (faction.controllerType == eFactionControllerType::RemoteClient && !faction.m_messageBackLog.empty())
		{
			for (auto iter = faction.m_messageBackLog.begin(); iter != faction.m_messageBackLog.end();)
			{
				sf::Packet packetToSend;
				packetToSend << (*iter);
				if (faction.m_tcpSocket->send(packetToSend) == sf::Socket::Done)
				{
					iter = faction.m_messageBackLog.erase(iter);
				}
				else
				{
					break;
				}
			}
		}
	}
}

void Server::listenForNewClient()
{
	std::unique_ptr<sf::TcpSocket> newClient = std::make_unique<sf::TcpSocket>();
	if (m_tcpListener->accept(*newClient) == sf::Socket::Done)
	{
		if (isServerFull())
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
			Faction& newlyOccupiedFaction = getNonOccupiedFaction();
			newlyOccupiedFaction.controllerType = eFactionControllerType::RemoteClient;
			std::vector<ServerMessageExistingFaction> existingFactions;

			for (const auto& faction : m_factions)
			{
				if (faction.factionName != newlyOccupiedFaction.factionName && faction.controllerType == eFactionControllerType::AI)
				{
					existingFactions.emplace_back(faction.factionName, faction.ships, true);
				}
				else if (faction.factionName != newlyOccupiedFaction.factionName && faction.controllerType != eFactionControllerType::AI)
				{
					existingFactions.emplace_back(faction.factionName, faction.ships, false);
				}
			}

			ServerMessage messageToSend(eMessageType::eEstablishConnection, newlyOccupiedFaction.factionName);
			messageToSend.existingFactions = existingFactions;
			sf::Packet packetToSend;
			packetToSend << messageToSend;
			if (newClient->send(packetToSend) != sf::Socket::Done)
			{
				std::cout << "Failed to establish connection\n";
				newlyOccupiedFaction.controllerType = eFactionControllerType::None;
			}
			else
			{
				std::cout << "Message sent to client\n";
			}

			std::cout << "New Client Added.\n";
			m_factions[static_cast<int>(newlyOccupiedFaction.factionName)].m_tcpSocket = std::move(newClient);
			m_socketSelector.add(*m_factions[static_cast<int>(newlyOccupiedFaction.factionName)].m_tcpSocket);
		}
	}
}

void Server::listenForServerMessages()
{
	for (auto& faction : m_factions)
	{
		if (faction.m_tcpSocket && m_socketSelector.isReady(*faction.m_tcpSocket))
		{
			sf::Packet receivedPacket;
			std::cout << "received Packet From Client\n";

			if (faction.m_tcpSocket->receive(receivedPacket) == sf::Socket::Done)
			{
				ServerMessage receivedServerMessage;
				receivedPacket >> receivedServerMessage;
				if (receivedServerMessage.type == eMessageType::ePlayerReady)
				{
					++m_remoteClientsReady;
					if (m_remoteClientsReady == numbOfRemoteClients())
					{
						m_remoteClientsReady = 0;
						ServerMessage messageToSend(eMessageType::eStartOnlineGame);
						messageToSend.levelName = m_levelName;
						std::cout << "Start Online Game\n";
						for (const auto& faction : m_factions)
						{
							messageToSend.spawnPositions.emplace_back(faction.factionName,
								faction.spawnPosition.x, faction.spawnPosition.y);
						}

						broadcastMessage(messageToSend);
					}
				}
				else if (receivedServerMessage.type == eMessageType::eNewPlayer)
				{
					m_factions[static_cast<int>(receivedServerMessage.faction)].ships = receivedServerMessage.shipsToAdd;
					broadcastMessage(receivedServerMessage);
				}
				else if (receivedServerMessage.type == eMessageType::eDisconnect)
				{
					std::cout << "Client Disconnected\n";
					m_factions[static_cast<int>(receivedServerMessage.faction)].controllerType = eFactionControllerType::None;
					m_socketSelector.remove(*m_factions[static_cast<int>(receivedServerMessage.faction)].m_tcpSocket);

					ServerMessage messageToSend(eMessageType::eClientDisconnected, receivedServerMessage.faction);
					broadcastMessage(messageToSend);
				}
				else if (receivedServerMessage.type == eMessageType::eDeployShipAtPosition ||
					receivedServerMessage.type == eMessageType::eMoveShipToPosition ||
					receivedServerMessage.type == eMessageType::eAttackShipAtPosition)
				{
					broadcastMessage(receivedServerMessage);
				}
			}
			else
			{
				std::cout << "Failed to receive packet\n";
			}
		}
	}
}

void Server::broadcastMessage(const ServerMessage & messageToSend)
{
	sf::Packet packetToSend;
	packetToSend << messageToSend;
	for (auto& faction : m_factions)
	{
		if (faction.controllerType == eFactionControllerType::RemoteClient)
		{
			if (faction.m_messageBackLog.empty())
			{
				if (faction.m_tcpSocket->send(packetToSend) != sf::Socket::Done)
				{
					std::cout << "Failed to send message to client\n";
					faction.m_messageBackLog.push_back(messageToSend);
				}
			}
			else
			{
				faction.m_messageBackLog.push_back(messageToSend);
			}
		}
	}
}

bool Server::isServerFull() const
{
	bool serverFull = true;
	for (const auto& faction : m_factions)
	{
		if (faction.controllerType == eFactionControllerType::None)
		{
			serverFull = false;
		}
	}

	return serverFull;
}

Faction & Server::getNonOccupiedFaction()
{
	auto iter = std::find_if(m_factions.begin(), m_factions.end(), [](const auto& faction) 
		{ return faction.controllerType == eFactionControllerType::None; });
	
	assert(iter != m_factions.end());
	return *iter;
}

int Server::numbOfRemoteClients() const
{
	int remoteClientCount = 0;
	for (const auto& faction : m_factions)
	{
		if (faction.controllerType == eFactionControllerType::RemoteClient)
		{
			++remoteClientCount;
		}
	}

	return remoteClientCount;
}