
#include "fow_building.h"
#include "fow_player.h"
#include "gatherer.h"
#include "knight.h"
#include "audiocontroller.h"

FOWBuilding::FOWBuilding(int x, int y, int size)
{
	type = FOW_BUILDING;
	can_build_units = false;
	skeleton_name = "buildings";

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
		{
			printf("nowhere to put unit!");
		}
		else
		{
			if (next_command.unit_type == FOW_GATHERER || next_command.unit_type == FOW_KNIGHT)
			{
				FOWPlayer::gold--;	// this static player reference would go better on game I think
			}

			t_vertex new_unit_position = t_vertex(tiles[0].x, tiles[0].y, 0);
			last_built_unit = ((FOWCharacter*)GridManager::build_and_add_entity(entity_to_build, new_unit_position));
		}
	}
	FOWSelectable::process_command(next_command);
}

void FOWBuilding::take_input(SDL_Keycode input, bool type, bool queue_add_toggle)
{
	if (keymap[ACTION] == input && can_build_units)
	{
		if (FOWPlayer::gold > 0)
		{
			process_command(FOWCommand(BUILD_UNIT, entity_to_build));
		}
		else
		{
			if (type == true && SDL_GetTicks() - FOWPlayer::last_poor_warning > 2500)
			{
				AudioController::play_sound("data/sounds/notenough.wav");
				FOWPlayer::last_poor_warning = SDL_GetTicks();
			}
		}
	}
}

void FOWBuilding::char_init()
{
	animationState->addAnimation(0, "animation", true, 0);
}

void FOWBuilding::set_under_construction()
{
	// just in case it hasn't been drawn yet
	if (spine_initialized == false)
	{
		build_spine();
	}

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
	 {
		 if (SDL_GetTicks() - construction_start_time > time_to_build)
		 {
			 construction_finished();
		 }
	 }
}

 void FOWEnemySpawner::update(float time_delta)
{
	 if (SDL_GetTicks() - last_spawn > 5000)
	 {	
		 // find an empty tile
		 auto adjacent_tiles = get_adjacent_tiles(true);
		 if (adjacent_tiles.size() > 0)
		 {
			 FOWCharacter* new_skeleton;

			 // this is kind of hacky but also reduces repeated code so...
			 process_command(FOWCommand(BUILD_UNIT, FOW_SKELETON));
			 auto town_halls = GridManager::get_entities_of_type(FOW_TOWNHALL);
			 if (town_halls.size() > 0)
			 {
				last_built_unit->give_command(FOWCommand(ATTACK_MOVE, t_vertex(town_halls[0]->position.x+1, town_halls[0]->position.y-1, 0)));
			 }
		 }

		 last_spawn = SDL_GetTicks();
	 }

}