
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

void Game::take_input(boundinput input, bool keydown)
{
	witch->take_input(input, keydown);
}


void Game::draw()
{
	// draw entities
	for (std::vector<Entity*>::iterator it = entities.begin(); it != entities.end(); ++it)
	{
		(*it)->draw();
		draw_aabb(((GameEntity*)(*it))->get_aabb());
	}
}

void Game::draw_aabb(t_transform aabb)
{
	glPushMatrix();
		glTranslatef(0.0f, 0.0f, GAME_PLANE);
		glDisable(GL_TEXTURE_2D);
		glLineWidth(1.0f);
		glColor3f(0.0f, 1.0f, 0.0f);
		glBegin(GL_LINES);
			glVertex2f(aabb.x, aabb.y);
			glVertex2f(aabb.x, aabb.h);
			glVertex2f(aabb.x, aabb.y);
			glVertex2f(aabb.w, aabb.y);
			glVertex2f(aabb.w, aabb.y);
			glVertex2f(aabb.w, aabb.h);
			glVertex2f(aabb.x, aabb.h);
			glVertex2f(aabb.w, aabb.h);
		glEnd();
		glEnable(GL_TEXTURE_2D);
		glColor3f(1.0f, 1.0f, 1.0f);
	glPopMatrix();
}