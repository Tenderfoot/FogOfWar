#pragma once

#include "fow_character.h"

class FOWBuilding;

class FOWGatherer : public FOWCharacter
{
public:

	FOWGatherer();
	FOWGatherer(t_vertex initial_position);

	bool has_gold;
	bool has_trees;
	t_vertex current_tree;
	float collecting_time;

	FOWSelectable* target_mine;
	FOWSelectable* target_town_hall;
	bool build_mode;
	bool good_spot;

	FOWBuilding* to_build;
	entity_types building_type;

	std::vector<std::string> chop_sounds;

	virtual void draw();
	virtual void update(float time_delta);
	virtual void OnReachDestination();
	virtual void make_new_path();
	void char_init();
	void set_collecting(t_vertex new_position);
	void set_chopping(t_vertex new_position);
	FOWSelectable* get_entity_of_entity_type(entity_types type, int team_id = -1);
	virtual void process_command(FOWCommand next_command);
	virtual void take_input(SDL_Keycode input, bool type, bool queue_add_toggle);
	virtual void callback(spine::AnimationState* state, spine::EventType type, spine::TrackEntry* entry, spine::Event* event);

	void clear_selection()
	{
		build_mode = false;
		FOWSelectable::clear_selection();
	}

};