
#include "fow_building.h"
#include "fow_player.h"
#include "gatherer.h"
#include "knight.h"
#include "audiocontroller.h"

FOWBuilding::FOWBuilding(int x, int y, int size)
{
	type = FOW_BUILDING;
	can_build_units = false;

	this->position.x = (float)x;
	this->position.y = (float)y;
	this->size = size;
	color = t_vertex(1, 1, 1);

	maximum_hp = 60;
	current_hp = maximum_hp;
	draw_position = position;
	draw_offset = t_vertex(-0.5f, +0.5f, 0);
}

void FOWBuilding::construction_finished()
{
	// TODO: This class specific code shouldn't be in the parent
	skin_name = base_skin;
	reset_skin();

	AudioController::play_sound("data/sounds/workcomplete.ogg");
	under_construction = false;

	std::vector<t_tile> tiles = get_adjacent_tiles(true);
	t_vertex new_position = t_vertex(tiles[0].x, tiles[0].y, 0.0f);
	((FOWCharacter*)builder)->hard_set_position(new_position);
	builder->visible = true;
}

void FOWBuilding::process_command(FOWCommand next_command)
{
	if (next_command.type == BUILD_UNIT)
	{
		printf("Build Unit command recieved\n");
		std::vector<t_tile> tiles = get_adjacent_tiles(true);
		if (tiles.size() < 1)
			printf("nowhere to put unit!");
		else
		{
			t_vertex new_unit_position = t_vertex(tiles[0].x, tiles[0].y, 0);
			grid_manager->build_and_add_entity(entity_to_build, new_unit_position);
		}
	}
	FOWSelectable::process_command(next_command);
}

void FOWBuilding::take_input(SDL_Keycode input, bool type, bool queue_add_toggle)
{
	FOWPlayer* player = GridManager::player;
	if (keymap[ACTION] == input && can_build_units)
	{
		if (player->gold > 0)
		{
			player->gold--;
			process_command(FOWCommand(BUILD_UNIT, entity_to_build));
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

void FOWBuilding::shared_init()
{
	load_spine_data("buildings", base_skin);
	make_vbo();
}

void FOWBuilding::make_vbo()
{
	VBO = SpineManager::make_vbo(skeleton);
	animationState = new spine::AnimationState(SpineManager::stateData["buildings"]);
	animationState->addAnimation(0, "animation", true, 0);
}

void FOWBuilding::set_under_construction()
{
	under_construction = true;
	construction_start_time = SDL_GetTicks();
	skin_name = skin_name.append("_UC");
	reset_skin();
}

void FOWBuilding::take_damage(int amount)
{
	printf("in building take damage\n");
}

 void FOWBuilding::update(float time_delta)
{
	if (under_construction)
		if (SDL_GetTicks() - construction_start_time > time_to_build)
			construction_finished();
}

 void FOWEnemySpawner::update(float time_delta)
{
	 if (SDL_GetTicks() - last_spawn > 5000)
	 {	
		 // find an empty tile
		 auto adjacent_tiles = get_adjacent_tiles(true);
		 if (adjacent_tiles.size() > 0)
		 {
			 t_tile chosen_tile = adjacent_tiles[0];
			 FOWCharacter* new_skeleton;
			 new_skeleton = ((FOWCharacter*)grid_manager->build_and_add_entity(FOW_SKELETON, t_vertex(chosen_tile.x, chosen_tile.y, 0.0f)));

			 auto town_halls = grid_manager->get_entities_of_type(FOW_TOWNHALL);
			 if (town_halls.size() > 0)
			 {
				 new_skeleton->give_command(FOWCommand(ATTACK, (FOWSelectable*)town_halls[0]));
			 }
		 }

		 last_spawn = SDL_GetTicks();
	 }
}