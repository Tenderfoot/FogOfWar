#pragma once

#include "fow_selectable.h"

class FOWBuilding;
class FOWPlayer;

class FOWCharacter : public FOWSelectable
{
public:

	FOWCharacter();

	void die();
	void draw();
	void set_idle();
	virtual void OnReachNextSquare();
	virtual void OnReachDestination();
	virtual void PathBlocked();
	void process_command(FOWCommand next_command);
	void give_command(FOWCommand command);
	virtual void update(float time_delta);

	t_vertex draw_position;
	t_vertex desired_position;
	bool dirty_visibiltiy;
	std::vector<t_tile*> current_path;
	FOWPlayer *owner;

};