
#include "fow_building.h"
#include "fow_player.h"
#include "gatherer.h"
#include "audiocontroller.h"

FOWBuilding::FOWBuilding()
{
}

FOWBuilding::FOWBuilding(int x, int y, int size)
{
	type = FOW_BUILDING;
	position.x = x;
	position.y = y;
	this->size = size;
	color = t_vertex(1, 1, 1);
	this->size = size;
}

void FOWBuilding::draw()
{
	// important for selection box
	draw_position = position;

	if (selected)
		draw_selection_box();
	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glPushMatrix();
		glTranslatef(position.x - 0.5, -position.y + 0.5, 0.01f);
		PaintBrush::draw_vbo(VBO);
		glColor3f(1, 1, 1);
	glPopMatrix();
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
}

FOWTownHall::FOWTownHall()
{
}

void FOWBuilding::construction_finished()
{
	if(type == FOW_TOWNHALL)
		skin_name = "TownHall";
	if (type == FOW_FARM)
		skin_name = "Farm";
	if (type == FOW_BARRACKS)
		skin_name = "Barracks";
	reset_skin();
	AudioController::play_sound("data/sounds/workcomplete.ogg");
	under_construction = false;
	std::vector<t_tile> tiles = get_adjacent_tiles(true);
	t_vertex new_position = t_vertex(tiles[0].x, tiles[0].y, 0.0f);
	((FOWCharacter*)builder)->hard_set_position(new_position);
	builder->visible = true;
}

FOWTownHall::FOWTownHall(int x, int y, int size) : FOWBuilding(x, y, size)
{
	type = FOW_TOWNHALL;
	load_spine_data("buildings", "TownHall");
	make_vbo();
	time_to_build = 5000;
}

void FOWTownHall::process_command(FOWCommand next_command)
{

	if (next_command.type == BUILD_UNIT)
	{
		printf("Build Unit command recieved\n");
		std::vector<t_tile> tiles = get_adjacent_tiles(true);
		if (tiles.size() < 1)
			printf("can't do that!");
		else
		{
			t_vertex new_unit_position = t_vertex(tiles[0].x, tiles[0].y, 0);
			FOWGatherer* new_gatherer = new FOWGatherer(new_unit_position);
			new_gatherer->owner = GridManager::player;
			new_gatherer->team_id = 0;
			grid_manager->entities->push_back((GameEntity*)new_gatherer);
			grid_manager->tile_map[tiles[0].x][tiles[0].y].entity_on_position = new_gatherer;
		}
	}

	FOWSelectable::process_command(next_command);
};

void FOWTownHall::take_input(SDL_Keycode input, bool type, bool queue_add_toggle)
{
	FOWPlayer* player = GridManager::player;
	if (keymap[ACTION] == input)
	{
		if (player->gold > 0)
		{
			player->gold--;
			process_command(FOWCommand(BUILD_UNIT, FOW_GATHERER));
		}
		else
		{
			if (type == true && SDL_GetTicks() - player->last_poor_warning > 2500)
			{
				AudioController::play_sound("data/sounds/notenough.wav");
				player->last_poor_warning = SDL_GetTicks();
			}
		}
	}
}