
#include "game.h"
#include <gl/GLU.h>

std::vector<Entity*> Game::entities;
t_transform Game::real_mouse_position;
bool sort_layers(Entity* i, Entity* j);	// this is in level.cpp

bool Game::init()
{
	SpineManager::LoadData();
	load_level("data/level.json");
	game_state = PLAY_MODE;
	std::sort(entities.begin(), entities.end(), sort_layers);

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

	if (input == EDIT_KEY)
		game_state = EDIT_MODE;

	if (input == PLAY_KEY)
		game_state = PLAY_MODE;

	if (game_state == PLAY_MODE)
		witch->take_input(input, keydown);
	else
		level_editor.take_input(input, keydown);
}

void Game::draw()
{
	t_transform camera_transform;

	if (game_state == PLAY_MODE)
		camera_transform = witch->transform;
	else
		camera_transform = level_editor.camera_transform;

	gluLookAt(camera_transform.x, camera_transform.y, 0.0f, camera_transform.x, camera_transform.y, GAME_PLANE, 0, 1, 0);

	// draw entities
	for (std::vector<Entity*>::iterator it = entities.begin(); it != entities.end(); ++it)
	{
		(*it)->draw();
		//draw_aabb(((GameEntity*)(*it))->get_aabb());
	}
}

void Game::get_mouse_in_space()
{
	// refresh mouse in space - needs to happen after draw
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winX, winY, winZ;
	GLdouble posX, posY, posZ;

	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	glGetIntegerv(GL_VIEWPORT, viewport);

	winX = (float)raw_mouse_position.x;
	winY = (float)viewport[3] - (float)raw_mouse_position.y;
	glReadPixels(winX, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
	gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

	real_mouse_position.x = posX;
	real_mouse_position.y = posY;
	real_mouse_position.w = posZ;
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