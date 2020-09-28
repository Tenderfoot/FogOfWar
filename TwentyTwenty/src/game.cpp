
#include "game.h"

bool Game::init()
{
	SpineManager::LoadData();

	test = new SpineEntity("witch");

	return true;
}

void Game::run(float deltatime)
{
	test->update(deltatime);
}

void Game::draw()
{
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	// call the static skeleton draw from the spine manager
	SpineManager::drawSkeleton(test->skeleton);
	glDisable(GL_BLEND);
}