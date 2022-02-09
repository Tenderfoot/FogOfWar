#pragma once

#include "fow_character.h"

class FOWBuilding;

typedef struct
{
	int gold_cost;
	int wood_cost;
}t_building_cost;


class FOWGatherer : public FOWCharacter
{
public:

	FOWGatherer();
	FOWGatherer(t_vertex initial_position);

	bool has_gold;
	bool has_trees;
	t_vertex current_tree;
	float collecting_time;

	std::map<std::string, t_building_cost> building_costs;

	FOWSelectable* target_mine;
	FOWSelectable* target_town_hall;
	bool build_mode;
	bool good_spot;

	FOWBuilding* to_build;
	entity_types building_type;

	float chop_start_time;

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