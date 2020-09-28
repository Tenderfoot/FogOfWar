
#include "game.h"

bool Game::init()
{
	SpineManager::LoadData();

	SpineEntity *test = new SpineEntity("witch");
	entities.push_back(test);

	return true;
}

void Game::run(float deltatime)
{
	// update entities
	for (std::vector<Entity*>::iterator it = entities.begin(); it != entities.end(); ++it) 
	{
		(*it)->update(deltatime);
	}
}

void Game::draw()
{
	// draw entities
	for (std::vector<Entity*>::iterator it = entities.begin(); it != entities.end(); ++it)
	{
		(*it)->draw();
	}
}