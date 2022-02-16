
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
std::thread* Game::decoration_thread;
bool Game::done;
UIErrorMessage* Game::new_error_message;

extern Settings user_settings;
extern SDL_Window* window;

bool Game::init(std::string new_mapname)
{
	SpineManager::LoadData("buildings");
	SpineManager::LoadData("caterpillar");
	SpineManager::LoadData("spine");
	SpineManager::LoadData("grass");
	SpineManager::LoadData("tree");
	SpineManager::LoadData("cattail");

	// this is so when a projectile is created by the client thread,
	// the thread doesn't end up calling any GL methods because the
	// texture is found and not loaded
	PaintBrush::get_texture("data/images/arrow.png");

	// basic stuff
	mapname = new_mapname;
	done = false;

	// music?
	AudioController::play_music();

	// add some stuff to the UI
	UserInterface::add_widget(new UIImage(0.5, 0.9, 1.01, 0.2, PaintBrush::Soil_Load_Texture("data/images/HUD.png", TEXTURE_CLAMP)));

	GreenBox* new_greenbox = new GreenBox();
	UserInterface::add_widget((UIWidget*)new_greenbox);

	new_error_message = new UIErrorMessage();
	UserInterface::add_widget((UIWidget*)new_error_message);

	UITimer *new_timer = new UITimer();
	UserInterface::add_widget((UIWidget*)new_timer);

	FOWBuilding::progress_bar = new UIProgressBar(t_vertex(0.5, 0.95, 0.0f), t_vertex(0.0f,1.0f,0.0f));
	UserInterface::add_widget((UIWidget*)FOWBuilding::progress_bar);
	FOWSelectable::hp_bar = new UIProgressBar(t_vertex(0.5, 0.9, 0.0f), t_vertex(1.0f, 0.0f, 0.0f));
	UserInterface::add_widget((UIWidget*)FOWSelectable::hp_bar);

	FOWPlayer::green_box = new_greenbox;

	game_state = PLAY_MODE;

	// init other stuff
	GridManager::init(mapname);

	GridManager::make_decorations();
	decoration_thread = new std::thread(GridManager::update);
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
	GridManager::game_update();

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

	PaintBrush::set_camera_location(glm::vec3(camera_transform.x, camera_transform.y, camera_transform.z));

	glDisable(GL_DEPTH_TEST);
	GridManager::draw_autotile();
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	// this is the decorations
	// don't draw them in edit mode
	if (game_state == PLAY_MODE)
	{
		GridManager::draw_vao();
	}

	if (game_state == EDIT_MODE)
	{
		FOWEditor::draw();
	}

	// draw entities
	for (auto entityItr : Game::entities)
	{
		entityItr->draw();
	}
	glDisable(GL_BLEND);
}

void Game::draw_ui()
{
	UserInterface::draw();
}

// this could be replaced with the new paintbrush drawquad that uses a vao
void Game::draw_plane()
{
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glColor3f(1.0f, 1.0f, 1.0f);
	glPushMatrix();
	glBegin(GL_QUADS);
	glVertex3f(-1000, -1000, -FOWPlayer::camera_pos.z);
	glVertex3f(1000, -1000, -FOWPlayer::camera_pos.z);
	glVertex3f(1000, 1000, -FOWPlayer::camera_pos.z);
	glVertex3f(-1000, 1000, -FOWPlayer::camera_pos.z);
	glEnd();
	glPopMatrix();
	glEnable(GL_TEXTURE_2D);
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
	real_mouse_position.x = posX + FOWPlayer::camera_pos.x;
	real_mouse_position.y = posY + FOWPlayer::camera_pos.y;
	real_mouse_position.z = posZ;

	coord_mouse_position.x = std::min((int)GridManager::size.x, std::max(int(Game::real_mouse_position.x + 0.5), 0));
	coord_mouse_position.y = std::min((int)GridManager::size.y, std::max(int(-Game::real_mouse_position.y + 0.5), 0));
}

void Game::send_error_message(std::string message, int team_id)
{
	// this message is for us, just display it
	if (FOWPlayer::team_id == team_id)
	{
		new_error_message->set_message(message);
	}
	else
	{
		ServerHandler::error_messages.push_back(t_error_message(message, team_id));
	}
}

int Game::get_supply_for_team(int team_id)
{
	auto townhalls = GridManager::get_entities_of_type(FOW_TOWNHALL, team_id);
	auto farms = GridManager::get_entities_of_type(FOW_FARM, team_id);

	// only include farms that aren't under construction or destroyed
	int built_farms = 0;
	for (auto farm : farms)
	{
		if (!((FOWBuilding*)farm)->under_construction && !((FOWBuilding*)farm)->destroyed)
			built_farms++;
	}

	// only include farms that aren't under construction or destroyed
	int built_townhalls = 0;
	for (auto townhall : townhalls)
	{
		if (!((FOWBuilding*)townhall)->under_construction && !((FOWBuilding*)townhall)->destroyed)
			built_townhalls++;
	}

	return (built_townhalls) + (built_farms * 4);
}

int Game::get_used_supply_for_team(int team_id)
{
	int total = 0;
	for (auto entity : Game::entities)
	{
		if (is_unit(entity->type))
		{
			if (((FOWSelectable*)entity)->team_id == team_id)
			{
				if(((FOWCharacter*)entity)->state != GRID_DEAD)
					total++;
			}
		}
	}
	return total;
}
