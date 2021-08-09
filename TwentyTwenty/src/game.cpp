
#include "game.h"
#include <gl/GLU.h>

#include "gatherer.h"

std::vector<GameEntity*> Game::entities;
t_transform Game::real_mouse_position;
t_transform Game::relative_mouse_position;
GridManager *FOWSelectable::grid_manager = nullptr;

bool Game::init()
{
	SpineManager::LoadData("buildings");
	SpineManager::LoadData("caterpillar");
	SpineManager::LoadData("spine");

	PaintBrush::setup_extensions();

	// this is so units can access and manupulate the player
	GridManager::player = &player;
	FOWSelectable::grid_manager = &grid_manager;

	game_state = PLAY_MODE;

	grid_manager.entities = &entities;
	grid_manager.init();
	player.grid_manager = &grid_manager;
	editor.grid_manager = &grid_manager;

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

	if(game_state == PLAY_MODE)
		player.update(deltatime);
	else
		editor.update(deltatime);

	// update entities
	// I have to use the size_type iterator and not the normal vector iterator
	// because update on GameEntity can spawn new entities, and adds them to the vector,
	// which invalidates the iterator
	std::vector<GameEntity*>::size_type size = entities.size();
	for (std::vector<GameEntity*>::size_type i = 0; i < size; ++i)
	{
		entities[i]->update(deltatime);
	}
}

void Game::take_input(SDL_Keycode input, bool keydown)
{

	if (keymap[EDIT_KEY] == input)
		game_state = EDIT_MODE;

	if (keymap[PLAY_KEY] == input)
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
		camera_transform = player.camera_pos;
	else
		camera_transform = editor.camera_pos;

	gluLookAt(camera_transform.x, camera_transform.y, camera_transform.w, camera_transform.x, camera_transform.y, GAME_PLANE, 0, 1, 0);
	
	grid_manager.draw_autotile();

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