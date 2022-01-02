#pragma once

#include "common.h"
#include "spine_entity.h"
#include "grid_manager.h"

class FOWSelectable;

class FOWCommand
{
public:

	FOWCommand()
	{
	}

	FOWCommand(t_ability_enum type, t_vertex position)
	{
		this->type = type;
		this->position = position;
		this->target = nullptr;
	}

	FOWCommand(t_ability_enum type, FOWSelectable *target)
	{
		this->type = type;
		this->target = target;
	}
	
	FOWCommand(t_ability_enum type, entity_types unit_type)
	{
		this->type = type;
		this->unit_type = unit_type;
	}

	bool operator==(const FOWCommand& rhs)
	{
		return (this->type == rhs.type && this->position.x == rhs.position.x &&  this->position.y == rhs.position.y);
	}

	t_vertex position;
	t_ability_enum type;
	entity_types unit_type;
	FOWSelectable *target;
};

class MyListener : public spine::AnimationStateListenerObject
{
public:
};

class FOWSelectable : public SpineEntity, public spine::AnimationStateListenerObject
{
public:

	FOWSelectable() : SpineEntity()
	{
		team_id = 0;
	}

	virtual void load_spine_data(std::string spine_file, std::string skin_name);
	virtual void callback(spine::AnimationState* state, spine::EventType type, spine::TrackEntry* entry, spine::Event* event) {};
	virtual void process_command(FOWCommand next_command);
	virtual void clear_selection();

	// references to future classes... should just have flags on this class
	bool is_selectable(entity_types type);
	bool is_unit(entity_types type);
	bool is_unit();

	void draw_selection_box();

	// this is probably cacheable if it becomes a problem
	std::vector<t_tile> get_adjacent_tiles(bool position_empty);
	virtual void take_input(SDL_Keycode input, bool type, bool queue_add_toggle) {};
	void dirty_tile_map();
	virtual void take_damage(int amount);

	FOWCommand current_command;
	std::vector<FOWCommand> command_queue;
	GridCharacterState state;
	static GridManager *grid_manager;
	
	// this is all stuff that characters and buildings share
	bool selected;
	int size;
	int team_id;
	int sight;
	int current_hp;
	int maximum_hp;


};