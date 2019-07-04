#pragma once
#include <memory>
#include <SFML/Network.hpp>

enum FactionName
{
	eYellow = 0,
	eBlue = 1,
	eGreen = 2,
	eRed = 3,
	eTotal = eRed + 1
};

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