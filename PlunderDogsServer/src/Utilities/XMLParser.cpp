#include "XMLParser.h"
#include "Base64.h"
#include "tinyxml.h"
#include <assert.h>

std::vector<sf::Vector2i> XMLParser::parseFactionSpawnPoints(const std::string & levelName)
{
	TiXmlDocument mapFile;
	bool mapLoaded = mapFile.LoadFile("Data//" + levelName);
	assert(mapLoaded);

	const auto& rootElement = mapFile.RootElement();

	std::vector<sf::Vector2i> factionStartingPositions;
	factionStartingPositions.reserve(4);
	for (const auto* entityElementRoot = rootElement->FirstChildElement(); entityElementRoot != nullptr; entityElementRoot = entityElementRoot->NextSiblingElement())
	{
		if (entityElementRoot->Value() != std::string("objectgroup") || entityElementRoot->Attribute("name") != std::string("SpawnPositionLayer"))
		{
			continue;
		}

		for (const auto* entityElement = entityElementRoot->FirstChildElement(); entityElement != nullptr; entityElement = entityElement->NextSiblingElement())
		{
			sf::Vector2i startingPosition;
			entityElement->Attribute("x", &startingPosition.x);
			entityElement->Attribute("y", &startingPosition.y);
			//startingPosition.y -= tileSize; //Tiled Hack
			startingPosition.x /= 24;
			startingPosition.y /= 28;
			factionStartingPositions.push_back(startingPosition);
		}
	}
	assert(!factionStartingPositions.empty());
	return factionStartingPositions;
}