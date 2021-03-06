#pragma once

#include "common.h"
#include "spine_entity.h"
#include "grid_manager.h"

class FOWSelectable;
class UIProgressBar;

typedef enum
{
	SOUND_READY,
	SOUND_SELECT,
	SOUND_COMMAND,
	SOUND_DEATH
} t_audiocue;

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
	FOWSelectable* target;
	FOWSelectable* self_ref;
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
		increment_entity();
	}

	virtual void callback(spine::AnimationState* state, spine::EventType type, spine::TrackEntry* entry, spine::Event* event) {};
	virtual void process_command(FOWCommand next_command);
	virtual void clear_selection();
	virtual void select_unit();

	// references to future classes... should just have flags on this class
	bool is_selectable(entity_types type);
	bool is_unit(entity_types type);
	bool is_unit();

	virtual void char_init() {};

	virtual void draw();
	virtual void update(float deltatime);

	void draw_selection_box();
	void play_audio_queue(t_audiocue audio_cue_type);

	std::vector<t_tile> get_adjacent_tiles(bool position_empty, bool dont_check_passable=false);	// this extends out from position with size respective to top left
	std::vector<t_tile> get_adjacent_tiles_from_center(int buffer_size, bool position_empty, bool dont_check_passable = false); // this extends out from position with parameter respective to center
	virtual void take_input(SDL_Keycode input, bool type, bool queue_add_toggle) {};
	void dirty_tile_map();
	void mow_me();
	virtual void take_damage(int amount);

	FOWCommand current_command;
	std::vector<FOWCommand> command_queue;
	std::vector<FOWCommand> blocked_command_queue;
	GridCharacterState state;
	
	// Some sounds stuff
	std::vector<std::string> ready_sounds;
	std::vector<std::string> select_sounds;
	std::vector<std::string> command_sounds;
	std::vector<std::string> death_sounds;

	// this is all stuff that characters and buildings share
	bool selected;
	int size;
	int team_id;
	int sight;
	int current_hp;
	int maximum_hp;
	static UIProgressBar* hp_bar; 	// HP bar

	static float last_command_sound; // this is a hack, the sound should be played on player probably
};