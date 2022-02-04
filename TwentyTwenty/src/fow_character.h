#pragma once

#include "fow_selectable.h"

class FOWBuilding;
class FOWPlayer;

typedef enum
{
	ATTACK_NONCOMBATANT,
	ATTACK_MELEE,
	ATTACK_RANGED,
	ATTACK_HYBRID
}t_attack_type;

class FOWCharacter : public FOWSelectable
{
public:

	// constructor...
	FOWCharacter();

	// spine animation callback
	virtual void callback(spine::AnimationState* state, spine::EventType type, spine::TrackEntry* entry, spine::Event* event);
	
	// basic character stuff
	void set_position(t_vertex position);
	virtual void char_init();
	void die();
	void set_idle();
	void process_command(FOWCommand next_command);
	void give_command(FOWCommand command);
	void set_moving(t_vertex new_position);
	void set_moving(FOWSelectable *move_target);
	bool check_attack(bool use_far);
	void handle_attack();
	void handle_attack_move();
	void attack();
	void hard_set_position(t_vertex new_position);
	virtual void take_damage(int amount);
	FOWSelectable* get_hit_target();
	FOWSelectable* get_attack_target();

	// pathfinding
	void find_path_to_target(FOWSelectable* target);
	void move_entity_on_grid();
	virtual void make_new_path();
	virtual void OnReachNextSquare();
	virtual void OnReachDestination();
	virtual void PathBlocked();

	// Entity stuff
	virtual void draw();
	virtual void update(float time_delta);
	virtual void take_input(SDL_Keycode input, bool type, bool queue_add_toggle);
	virtual void think();


	// attack variables
	t_attack_type attack_type;
	FOWSelectable* attack_move_target;	// attack_move_target is set when attack_move finds a target
	FOWSelectable* network_target;		// this target is recieved from the server so the client knows who to hit

	// other variables
	t_vertex desired_position;
	t_vertex entity_position;
	int blocked_retry_count;
	float blocked_time;
	bool dirty_visibiltiy;
	std::vector<t_tile*> current_path;
	FOWPlayer *owner;
	bool dir;
	float time_reached_last_square;
	float speed;	// time needed to go from one square to another

};