#include "Client.h"

Client::Client(std::unique_ptr<sf::TcpSocket>&& tcpSocket)
	: m_tcpSocket(std::move(tcpSocket))
{

}

sf::TcpSocket & Client::getTcpSocket()
{
	return *m_tcpSocket;
}
