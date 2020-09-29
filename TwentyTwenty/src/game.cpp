
#include "game.h"
#include <gl/GLU.h>

std::vector<Entity*> Game::entities;

bool Game::init()
{
	SpineManager::LoadData();

	load_level("data/level.json");

	witch = new Player();
	entities.push_back(witch);

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
	t_transform camera_transform = witch->transform;
	gluLookAt(camera_transform.x, camera_transform.y, 0.0f, camera_transform.x, camera_transform.y, GAME_PLANE, 0, 1, 0);

	// draw entities
	for (std::vector<Entity*>::iterator it = entities.begin(); it != entities.end(); ++it)
	{
		(*it)->draw();
		//draw_aabb(((GameEntity*)(*it))->get_aabb());
	}
}

bool Game::load_level(std::string filename) {
	nlohmann::json level_data;
	std::ifstream i(filename);
	i >> level_data;

	// import settings
	new_level = level_data.get<Level>();

	printf("Level Name: %s\n", new_level.name.c_str());

	return true;
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