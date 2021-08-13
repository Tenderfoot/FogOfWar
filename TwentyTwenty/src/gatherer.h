#pragma once

#include "fow_character.h"

class FOWBuilding;

class FOWGatherer : public FOWCharacter
{
public:

	FOWGatherer();
	FOWGatherer(t_vertex initial_position);

	bool has_gold;
	float collecting_time;

	FOWSelectable* target_mine;
	FOWSelectable* target_town_hall;
	bool build_mode;
	bool good_spot;

	FOWBuilding* to_build;
	entity_types building_type;

	virtual void draw();
	virtual void update(float time_delta);
	virtual void OnReachDestination();
	virtual void make_new_path();
	void set_collecting(t_vertex new_position);
	FOWSelectable* get_entity_of_entity_type(entity_types type);
	virtual void process_command(FOWCommand next_command);
	virtual void take_input(SDL_Keycode input, bool type, bool queue_add_toggle);

	void clear_selection()
	{
		build_mode = false;
		FOWSelectable::clear_selection();
	}

};