#include <SFML/Network.hpp> 
#include "Utilities/XMLParser.h"
#include "Global.h"
#include <iostream>
#include <array>
#include <assert.h>
#include "Server.h"

int main()
{
	std::unique_ptr<Server> server = Server::create("level3.tmx", 55001);
	if (!server)
	{
		std::cout << "Server failed to start\n";
		return -1;
	}

	server->listen();

	return 0;
}