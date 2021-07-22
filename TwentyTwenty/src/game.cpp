
#include "game.h"
#include <gl/GLU.h>

std::vector<GameEntity*> Game::entities;
t_transform Game::real_mouse_position;
t_transform Game::relative_mouse_position;
GridManager *FOWSelectable::grid_manager = nullptr;

bool sort_layers(Entity* i, Entity* j);	// this is in level.cpp

bool Game::init()
{
	SpineManager::LoadData("spine");
	SpineManager::LoadData("caterpillar");
	SpineManager::LoadData("buildings");

	game_state = PLAY_MODE;

	grid_manager.entities = &entities;
	grid_manager.init();
	player.grid_manager = &grid_manager;
	editor.grid_manager = &grid_manager;
	FOWSelectable::grid_manager = &grid_manager;

	// this should go somewhere
	for (int i = 0; i < entities.size(); i++)
	{
		Entity* current_entity = entities.at(i);
		if (current_entity->type == GRID_SPAWNPOINT)
		{
			new_character = new FOWGatherer(current_entity->position);
			new_character->owner = &player;
			entities.push_back(new_character);
		}
	}

	// this should be part of the map
	FOWBuilding* new_building = new FOWTownHall(9, 7, 3);
	entities.push_back(new_building);
	new_building = new FOWGoldMine(22, 7, 3);
	entities.push_back(new_building);

	//std::sort(entities.begin(), entities.end(), sort_layers);
	// initialize entities
	for (std::vector<GameEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
	{
		(*it)->init();
	}
	
	return true;
}

void Game::run(float deltatime)
{
	grid_manager.set_mouse_coords(real_mouse_position);

	player.update();
	editor.update();

	std::vector<GameEntity*>::size_type size = entities.size();
	// update entities
	// I have to use the size_type iterator and not the normal vector iterator
	// because update on GameEntity can spawn new entities, and adds them to the vector,
	// which invalidates the iterator
	for (std::vector<GameEntity*>::size_type i = 0; i < size; ++i)
	{
		entities[i]->update(deltatime);
	}
}

void Game::take_input(boundinput input, bool keydown)
{

	if (input == EDIT_KEY)
		game_state = EDIT_MODE;

	if (input == PLAY_KEY)
		game_state = PLAY_MODE;

	if (game_state == PLAY_MODE)
		player.take_input(input, keydown);
	else
		editor.take_input(input, keydown);
}

void Game::draw()
{
	t_transform camera_transform;

	if (game_state == PLAY_MODE)
	{
		camera_transform.x = player.camera_pos.x;
		camera_transform.y = player.camera_pos.y;
		camera_transform.w = player.camera_distance;
	}
	else
	{
		camera_transform.x = editor.camera_pos.x;
		camera_transform.y = editor.camera_pos.y;
		camera_transform.w = editor.camera_distance;
	}

	gluLookAt(camera_transform.x, camera_transform.y, camera_transform.w, camera_transform.x, camera_transform.y, GAME_PLANE, 0, 1, 0);
	
	grid_manager.draw_autotile();
	player.draw();

	// draw entities
	for (std::vector<GameEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
	{
		(*it)->draw();
	}

	player.green_box->draw();
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