#include "Client.h"

Client::Client(std::unique_ptr<sf::TcpSocket>&& tcpSocket, FactionName factionName)
	: m_tcpSocket(std::move(tcpSocket)),
	m_factionName(factionName)
{

}

sf::TcpSocket & Client::getTcpSocket()
{
	return *m_tcpSocket;
}
