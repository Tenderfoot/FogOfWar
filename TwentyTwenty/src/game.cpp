
#include "game.h"
#include <gl/GLU.h>

std::vector<GameEntity*> Game::entities;
t_transform Game::real_mouse_position;
t_transform Game::relative_mouse_position;
bool sort_layers(Entity* i, Entity* j);	// this is in level.cpp

bool Game::init()
{
	SpineManager::LoadData();

	game_state = PLAY_MODE;
	
	// This is leftover code from the merge and its gross
	grid_manager.entities = &entities;
	grid_manager.init();
	player.grid_manager = &grid_manager;
	player.entities = &entities;
	editor.grid_manager = &grid_manager;


	// this should go somewhere
	for (int i = 0; i < entities.size(); i++)
	{
		Entity* current_entity = entities.at(i);
		if (current_entity->type == GRID_SPAWNPOINT)
		{
			new_character = new FOWGatherer();
			new_character->grid_manager = &grid_manager;
			new_character->position = current_entity->position;
			new_character->set_idle();
			entities.push_back(new_character);
		}
	}

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
	player.mouse_pos.x = real_mouse_position.x;
	player.mouse_pos.y = real_mouse_position.y;
	player.mouse_in_space = t_vertex(real_mouse_position.x, real_mouse_position.y, real_mouse_position.w);
	player.update();

	grid_manager.mouse_x = int(real_mouse_position.x);
	grid_manager.mouse_y = int(-real_mouse_position.y);

	editor.mouse_pos.x = real_mouse_position.x;
	editor.mouse_pos.y = real_mouse_position.y;
	editor.update();

	// update entities
	for (std::vector<GameEntity*>::iterator it = entities.begin(); it != entities.end(); ++it) 
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

		gluLookAt(camera_transform.x, camera_transform.y, camera_transform.w, camera_transform.x, camera_transform.y, 0, 0, 1, 0);
	}
	else
	{
		camera_transform.x = editor.camera_pos.x;
		camera_transform.y = editor.camera_pos.y;
		camera_transform.w = editor.camera_distance;

		gluLookAt(camera_transform.x, camera_transform.y, camera_transform.w, camera_transform.x, camera_transform.y, GAME_PLANE, 0, 1, 0);
	}
	
	grid_manager.draw_autotile();

	// draw entities
	for (std::vector<GameEntity*>::iterator it = entities.begin(); it != entities.end(); ++it)
	{
		(*it)->draw();
		//if(level_editor.is_selected((*it)))
			//draw_aabb(((GameEntity*)(*it))->get_aabb());
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