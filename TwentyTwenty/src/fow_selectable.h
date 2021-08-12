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

	virtual void load_spine_data(std::string spine_file, std::string skin_name)
	{
		skeleton_name = spine_file;

		skeleton = new spine::Skeleton(SpineManager::skeletonData[spine_file.c_str()]);

		skeleton->setToSetupPose();
		skeleton->updateWorldTransform();

		set_skin(skin_name.c_str());
	}

	virtual void callback(spine::AnimationState* state, spine::EventType type, spine::TrackEntry* entry, spine::Event* event)
	{
		// Inspect and respond to the event here.
		if (type == spine::EventType_Event)
		{
			// spine has its own string class that doesn't work with std::string
			if (std::string(event->getData().getName().buffer()) == std::string("attack_event"))
				printf("Hit attack event");
		}
	};

	virtual void process_command(FOWCommand next_command)
	{
	};

	virtual void clear_selection() 
	{
		selected = false;
	};

	bool is_selectable(entity_types type)
	{
		return (type == FOW_CHARACTER || type == FOW_GATHERER || type == FOW_BUILDING || type == FOW_TOWNHALL || type == FOW_GOLDMINE);
	}


	void draw_selection_box()
	{
		glPushMatrix();
			if (team_id == 0)
				glColor3f(0.5f, 1.0f, 0.5f);
			else
				glColor3f(1.0f, 0.0f, 0.0f);
			glDisable(GL_TEXTURE_2D);
			glLineWidth(1.0f);
			glBegin(GL_LINES);
			glVertex3f((draw_position.x) - 0.5, -(draw_position.y) - 0.5 - size + 1, 0.01f);
			glVertex3f((draw_position.x) - 0.5, -(draw_position.y) + 0.5, 0.01f);
			glVertex3f((draw_position.x) - 0.5, -(draw_position.y) - 0.5 - size + 1, 0.01f);
			glVertex3f((draw_position.x) + 0.5 + size - 1, -(draw_position.y) - 0.5 - size + 1, 0.01f);
			glVertex3f((draw_position.x) - 0.5, -(draw_position.y) + 0.5, 0.01f);
			glVertex3f((draw_position.x) + 0.5 + size - 1, -(draw_position.y) + 0.5, 0.01f);
			glVertex3f((draw_position.x) + 0.5 + size - 1, -(draw_position.y) - 0.5 - size + 1, 0.01f);
			glVertex3f((draw_position.x) + 0.5 + size -1, -(draw_position.y) + 0.5, 0.01f);
			glEnd();
			glColor3f(1.0f, 1.0f, 1.0f);
			glEnable(GL_TEXTURE_2D);
		glPopMatrix();
	}

	// this is probably cacheable if it becomes a problem
	std::vector<t_tile> get_adjacent_tiles(bool position_empty)
	{
		std::vector<t_tile> adjacent_tiles;
		int i, j;
		for (i = position.x - 1; i < position.x + (size + 1); i++)
			for (j = position.y - 1; j < position.y + (size + 1); j++)
				if ((i == position.x - 1 || i == position.x + (size + 1) || j == position.y - 1 || position.y + (size + 1)) && (grid_manager->tile_map[i][j].entity_on_position == nullptr || position_empty==false))
					adjacent_tiles.push_back(grid_manager->tile_map[i][j]);

		return adjacent_tiles;
	}

	virtual void take_input(SDL_Keycode input, bool type, bool queue_add_toggle) {};

	void dirty_tile_map()
	{
		int i, j;
		for (i = position.x; i < position.x + (size); i++)
			for (j = position.y; j < position.y + (size); j++)
				grid_manager->tile_map[i][j].entity_on_position = this;
	}

	FOWCommand current_command;
	std::vector<FOWCommand> command_queue;
	GridCharacterState state;
	static GridManager *grid_manager;
	bool selected;
	int size;
	int team_id;

};