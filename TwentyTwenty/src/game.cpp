
#include <SDL.h>
#include <SDL_opengl.h>
#include "game.h"
#include "gatherer.h"
#include "audiocontroller.h"
#include "settings.h"
#include "user_interface.h"

std::vector<GameEntity*> Game::entities;
t_vertex Game::raw_mouse_position;
t_vertex Game::real_mouse_position;
t_vertex Game::relative_mouse_position;
t_vertex Game::coord_mouse_position;
GridManager *FOWSelectable::grid_manager = nullptr;
MapWidget* Game::minimap = nullptr;

extern Settings user_settings;
extern SDL_Window* window;

bool Game::init()
{
	SpineManager::LoadData("buildings");
	SpineManager::LoadData("caterpillar");
	SpineManager::LoadData("spine");

	PaintBrush::setup_extensions();

	// music?
	AudioController::play_sound("data/sounds/music.mp3");

	// this is so units can access and manupulate the player
	GridManager::player = &player;
	FOWSelectable::grid_manager = &grid_manager;
	UserInterface::grid_manager = &grid_manager;

	// add some stuff to the UI
	UserInterface::add_widget(new UIImage(0.5, 0.9, 1.01, 0.2, PaintBrush::Soil_Load_Texture("data/images/HUD.png", TEXTURE_CLAMP)));

	GreenBox* new_greenbox = new GreenBox();
	UserInterface::add_widget((UIWidget*)new_greenbox);
	minimap = new MapWidget();
	UserInterface::add_widget((UIWidget*)minimap);

	player.green_box = new_greenbox;

	game_state = PLAY_MODE;

	grid_manager.entities = &entities;
	grid_manager.init();
	player.grid_manager = &grid_manager;
	editor.grid_manager = &grid_manager;
	editor.init();

	for (auto entityItr : entities)
	{
		entityItr->init();
	}
	
	return true;
}

void Game::run(float deltatime)
{
	if (game_state == PLAY_MODE)
	{
		player.update(deltatime);
	}
	else
	{
		editor.update(deltatime);
	}

	// the goal:
	/******************************
	for (auto entityItr : entities)
	{
	    entityItr->update(deltatime);
	}
	******************************/
	if (user_settings.isDirty())
	{
		if (window != nullptr)
		{
			SDL_SetWindowFullscreen(window, (SDL_WINDOW_FULLSCREEN & user_settings.fullscreen));
			user_settings.clearDirty();
			save_settings_to_file(user_settings, DEFAULT_SETTINGS_PATH);
		}
	}
	// so I am changing this set while I iterate over it
	// so if I use the auto iterator it breaks
	std::vector<GameEntity*>::size_type size = entities.size();
	for (std::vector<GameEntity*>::size_type i = 0; i < size; ++i)
	{
		entities[i]->update(deltatime);
	}
}

void Game::take_input(SDL_Keycode input, bool keydown)
{
	UserInterface::take_input(input, keydown);

	if (keymap[EDIT_KEY] == input)
	{
		game_state = EDIT_MODE;
	}
	if (keymap[PLAY_KEY] == input)
	{
		game_state = PLAY_MODE;
	}
	if (game_state == PLAY_MODE)
	{
		player.take_input(input, keydown);
	}
	else
	{
		editor.take_input(input, keydown);
	}
}

bool sort_by_y(GameEntity *i, GameEntity *j) { return (i->draw_position.y < j->draw_position.y); }

void Game::draw()
{
	t_vertex camera_transform;

	if (game_state == PLAY_MODE)
	{
		camera_transform = player.camera_pos;
	}
	else
	{
		camera_transform = editor.camera_pos;
	}

	gluLookAt(camera_transform.x, camera_transform.y, camera_transform.z, camera_transform.x, camera_transform.y, GAME_PLANE, 0, 1, 0);
	
	grid_manager.draw_autotile();

	if (game_state == EDIT_MODE)
	{
		editor.draw();
	}

	// using function as comp
	std::sort(entities.begin(), entities.end(), sort_by_y);

	// draw entities
	for (auto entityItr : entities)
	{
		entityItr->draw();
	}
}

void Game::draw_ui()
{
	UserInterface::draw();
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
	real_mouse_position.z = posZ;

	coord_mouse_position.x = std::min(grid_manager.width, std::max(int(Game::real_mouse_position.x + 0.5), 0));
	coord_mouse_position.y = std::min(grid_manager.height, std::max(int(-Game::real_mouse_position.y + 0.5), 0));
}