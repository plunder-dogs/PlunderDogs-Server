#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include "Client.h"
#include <vector>
#include <iostream>

constexpr size_t MAX_CLIENTS = 4;

int main()
{
	bool serverRunning = true;
	sf::TcpListener tcpListener;
	tcpListener.listen(55001);
	sf::SocketSelector socketSelector;
	socketSelector.add(tcpListener);
	std::vector<std::unique_ptr<Client>> clients;
	clients.reserve(MAX_CLIENTS);
	std::cout << "Started Listening\n";

	while (serverRunning)
	{
		if (socketSelector.wait())
		{
			if (socketSelector.isReady(tcpListener))
			{
				std::unique_ptr<sf::TcpSocket> newClient = std::make_unique<sf::TcpSocket>();
				if (tcpListener.accept(*newClient) == sf::Socket::Done)
				{
					std::cout << "New Client Added.\n";
					clients.push_back(std::make_unique<Client>(std::move(newClient)));
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
						if(client->getTcpSocket().receive(receivedPacket) == sf::Socket::Done)
						{
							std::cout << "Message Received\n";
						}
					}
				}
			}

		}
	}

	return 0;
}