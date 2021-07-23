
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

	virtual void update(float time_delta);
	virtual void OnReachDestination();
	virtual void PathBlocked();
	virtual void process_command(FOWCommand next_command);

	void clear_selection()
	{
		build_mode = false;
		FOWSelectable::clear_selection();
	}

};