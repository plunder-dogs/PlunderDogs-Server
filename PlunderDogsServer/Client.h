#pragma once
#include <memory>
#include <SFML/Network.hpp>

class Client
{
public:
	Client(std::unique_ptr<sf::TcpSocket>&& tcpSocket);

	sf::TcpSocket& getTcpSocket();

private:
	std::unique_ptr<sf::TcpSocket> m_tcpSocket;
};