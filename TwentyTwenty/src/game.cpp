
#include <SDL.h>
#include <SDL_opengl.h>
#include "game.h"
#include "gatherer.h"
#include "audiocontroller.h"
#include "settings.h"
#include "user_interface.h"
#include "server_handler.h"
#include "client_handler.h"
#include "fow_decoration.h"

std::vector<GameEntity*> Game::entities;
t_vertex Game::raw_mouse_position;
t_vertex Game::real_mouse_position;
t_vertex Game::relative_mouse_position;
t_vertex Game::coord_mouse_position;
MapWidget* Game::minimap = nullptr;
e_gamestate Game::game_state;
std::string Game::mapname = "";
bool Game::initialized = false;
UIProgressBar* Game::new_bar = nullptr;
std::vector<GameEntity*> Game::combined_vector;

extern Settings user_settings;
extern SDL_Window* window;

bool Game::init(std::string new_mapname)
{
	SpineManager::LoadData("buildings");
	SpineManager::LoadData("caterpillar");
	SpineManager::LoadData("spine");
	SpineManager::LoadData("grass");
	SpineManager::LoadData("tree");

	mapname = new_mapname;

	// music?
	AudioController::play_music();

	// add some stuff to the UI
	UserInterface::add_widget(new UIImage(0.5, 0.9, 1.01, 0.2, PaintBrush::Soil_Load_Texture("data/images/HUD.png", TEXTURE_CLAMP)));

	GreenBox* new_greenbox = new GreenBox();
	UserInterface::add_widget((UIWidget*)new_greenbox);

	FOWBuilding::progress_bar = new UIProgressBar();
	UserInterface::add_widget((UIWidget*)FOWBuilding::progress_bar);

	FOWPlayer::green_box = new_greenbox;

	game_state = PLAY_MODE;

	// init other stuff
	GridManager::init(mapname);
	GridManager::make_decorations();
	make_combined();

	FOWPlayer::init();
	FOWEditor::init();

	// build minimap (needs gridmanager initialized)
	minimap = new MapWidget();
	UserInterface::add_widget((UIWidget*)minimap);

	// this isn't doing anything right now
	// built entities aren't having init called I think this is dead code
	for (auto entityItr : entities)
	{
		entityItr->init();
	}

	initialized = true;

	return true;
}

void Game::run(float deltatime)
{
	if (game_state == PLAY_MODE)
	{
		FOWPlayer::update(deltatime);
	}
	else
	{
		FOWEditor::update(deltatime);
	}

	// here is where I'm going to update grid decorations
	
	// the goal:
	/******************************
	for (auto entityItr : entities)
	{
	    entityItr->update(deltatime);
	}
	******************************/

	// Decoration stuff
	FOWDecoration::reset_decorations();
	GridManager::update(deltatime);

	// so I am changing this set while I iterate over it
	// so if I use the auto iterator it breaks
	std::vector<GameEntity*>::size_type size = entities.size();
	for (std::vector<GameEntity*>::size_type i = 0; i < size; ++i)
	{
		std::unique_lock<std::mutex> lock(entities[i]->entity_mutex);
		entities[i]->update(deltatime);
		lock.unlock();
	}
}

void Game::take_input(SDL_Keycode input, bool keydown)
{
	UserInterface::take_input(input, keydown);

	if (initialized)
	{
		if (keymap[DISABLE_SIDESCROLL] == input && keydown == true)
		{
			user_settings.toggleScroll();
		}

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
			FOWPlayer::take_input(input, keydown);
		}
		else
		{
			FOWEditor::take_input(input, keydown);
		}
	}
}

bool sort_by_y(GameEntity *i, GameEntity *j) { return (i->draw_position.y < j->draw_position.y); }

void Game::make_combined()
{
	combined_vector.clear();
	combined_vector.insert(combined_vector.end(), Game::entities.begin(), Game::entities.end());
	combined_vector.insert(combined_vector.end(), GridManager::decorations.begin(), GridManager::decorations.end());
	std::sort(combined_vector.begin(), combined_vector.end(), sort_by_y);
}

void Game::draw()
{
	t_vertex camera_transform;

	if (game_state == PLAY_MODE)
	{
		camera_transform = FOWPlayer::camera_pos;
	}
	else
	{
		camera_transform = FOWEditor::camera_pos;
	}

	gluLookAt(camera_transform.x, camera_transform.y, camera_transform.z, camera_transform.x, camera_transform.y, GAME_PLANE, 0, 1, 0);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	GridManager::draw_autotile();

	if (game_state == EDIT_MODE)
	{
		FOWEditor::draw();
	}

	// want to get rid of this
	make_combined();

	glEnable(GL_BLEND);
	// draw entities
	t_transform red_box = minimap->get_red_box();
	for (auto entityItr : combined_vector)
	{
		if (entityItr->position.x > (red_box.x - red_box.w) && entityItr->position.x < (red_box.x + red_box.w) &&
			entityItr->position.y >(red_box.y - red_box.h) && entityItr->position.y < (red_box.y + red_box.h))
		{
			entityItr->draw();
		}
	}

	glDisable(GL_BLEND);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
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

	// conversion from GLdouble to float
	real_mouse_position.x = posX;
	real_mouse_position.y = posY;
	real_mouse_position.z = posZ;

	coord_mouse_position.x = std::min((int)GridManager::size.x, std::max(int(Game::real_mouse_position.x + 0.5), 0));
	coord_mouse_position.y = std::min((int)GridManager::size.y, std::max(int(-Game::real_mouse_position.y + 0.5), 0));
}