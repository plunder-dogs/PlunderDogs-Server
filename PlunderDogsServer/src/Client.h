#pragma once
#include <memory>
#include <SFML/Network.hpp>
#include "Global.h"

class Client
{
public:
	Client(std::unique_ptr<sf::TcpSocket>&& tcpSocket, FactionName factionName);

	FactionName getFactionName() const;
	sf::TcpSocket& getTcpSocket();

private:
	const FactionName m_factionName;
	std::unique_ptr<sf::TcpSocket> m_tcpSocket;
};