
#include "game.h"

bool Game::init()
{
	spine_manager.LoadData();
	return true;
}

void Game::run(float deltatime)
{
	spine_manager.updateSkeleton(deltatime);
}

void Game::draw()
{
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	spine_manager.drawSkeleton();
	glDisable(GL_BLEND);
}