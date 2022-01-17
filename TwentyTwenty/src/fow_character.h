#pragma once

#include "fow_selectable.h"

class FOWBuilding;
class FOWPlayer;

class FOWCharacter : public FOWSelectable
{
public:

	FOWCharacter();

	void set_position(t_vertex position);

	// spine animation callback
	void callback(spine::AnimationState* state, spine::EventType type, spine::TrackEntry* entry, spine::Event* event);
	
	void char_init();
	void die();
	void set_idle();
	void process_command(FOWCommand next_command);
	void give_command(FOWCommand command);
	void set_moving(t_vertex new_position);
	void set_moving(FOWSelectable *move_target);
	void find_path_to_target(FOWSelectable *target);
	void move_entity_on_grid();
	bool check_attack();
	bool check_attack_move(bool use_far);

	void attack();
	void hard_set_position(t_vertex new_position);

	virtual void take_damage(int amount);

	virtual void make_new_path();
	virtual void draw();
	virtual void OnReachNextSquare();
	virtual void OnReachDestination();
	virtual void PathBlocked();
	virtual void update(float time_delta);
	virtual void take_input(SDL_Keycode input, bool type, bool queue_add_toggle);
	virtual void think();
	FOWSelectable* get_hit_target();
	FOWSelectable* get_attack_target();

	FOWSelectable* attack_move_target;
	t_vertex desired_position;
	t_vertex entity_position;
	int blocked_retry_count;
	float blocked_time;
	bool dirty_visibiltiy;
	std::vector<t_tile*> current_path;
	FOWPlayer *owner;
	bool dir;

};