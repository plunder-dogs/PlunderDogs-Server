#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <memory>
#include <vector>
#include <iostream>


int main()
{
	bool serverRunning = true;
	sf::TcpListener tcpListener;
	tcpListener.listen(55001);
	sf::SocketSelector socketSelector;
	socketSelector.add(tcpListener);
	std::vector<sf::TcpSocket*> clients;

	while (serverRunning)
	{
		if (socketSelector.wait())
		{
			if (socketSelector.isReady(tcpListener))
			{
				sf::TcpSocket* newClient = new sf::TcpSocket;
				if (tcpListener.accept(*newClient) == sf::Socket::Done)
				{
					std::cout << "New Client Added.\n";
					clients.push_back(newClient);
					socketSelector.add(*newClient);
				}
				else
				{
					delete newClient;
				}
			}
			else
			{

			}

		}
	}

	return 0;
}