#pragma once

#include <string>
#include <SFML/Graphics.hpp>
#include <vector>

namespace XMLParser
{
	std::vector<sf::Vector2i> parseFactionSpawnPoints(const std::string& levelName);
}