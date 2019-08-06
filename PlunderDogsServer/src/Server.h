#pragma once

#include <SFML/Network.hpp>
#include "Faction.h"
#include <array>
#include <memory>

class Server
{
public:
	static std::unique_ptr<Server> create(std::string&& startingLevel, int portNumber);

	void listen();

private:
	Server(std::string&& startingLevel);
	std::unique_ptr<sf::TcpListener> m_tcpListener;
	sf::SocketSelector m_socketSelector;
	std::array<Faction, static_cast<size_t>(FactionName::eTotal)> m_factions
	{
		FactionName::eYellow,
		FactionName::eBlue,
	   {FactionName::eGreen, eFactionControllerType::AI},
	   {FactionName::eRed, eFactionControllerType::AI}
	};
	
	bool m_running;
	int m_remoteClientsReady;
	std::string m_levelName;

	void handleFactionMessageBackLog();
	void listenForNewClient();
	void listenForServerMessages();
	void broadcastMessage(const ServerMessage& messageToSend);
	bool isServerFull() const;
	Faction& getNonOccupiedFaction();
	int numbOfRemoteClients() const;
};