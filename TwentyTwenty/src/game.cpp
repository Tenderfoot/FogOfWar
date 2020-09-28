
#include "game.h"

std::vector<Entity*> Game::entities;

bool Game::init()
{
	SpineManager::LoadData();

	witch = new Player();
	entities.push_back(witch);

	GameEntity *floor = new GameEntity(0, -10, 50, 10);
	entities.push_back(floor);

	// initialize entities
	for (std::vector<Entity*>::iterator it = entities.begin(); it != entities.end(); ++it)
	{
		(*it)->init();
	}

	witch->make_floor();

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