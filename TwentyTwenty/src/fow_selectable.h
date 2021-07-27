#pragma once

#include "common.h"
#include "spine_entity.h"
#include "grid_manager.h"

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
	}

	FOWCommand(t_ability_enum type, GameEntity *target)
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
	GameEntity *target;
};

class FOWSelectable : public SpineEntity
{
public:

	FOWSelectable() : SpineEntity("witch")
	{
	}

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
			glColor3f(0.5f, 1.0f, 0.5f);
			glDisable(GL_TEXTURE_2D);
			glLineWidth(1.0f);
			glBegin(GL_LINES);
			glVertex3f((draw_position.x) - 0.5, -(draw_position.y) - 0.5, 0.01f);
			glVertex3f((draw_position.x) - 0.5, -(draw_position.y) + 0.5, 0.01f);
			glVertex3f((draw_position.x) - 0.5, -(draw_position.y) - 0.5, 0.01f);
			glVertex3f((draw_position.x) + 0.5, -(draw_position.y) - 0.5, 0.01f);
			glVertex3f((draw_position.x) - 0.5, -(draw_position.y) + 0.5, 0.01f);
			glVertex3f((draw_position.x) + 0.5, -(draw_position.y) + 0.5, 0.01f);
			glVertex3f((draw_position.x) + 0.5, -(draw_position.y) - 0.5, 0.01f);
			glVertex3f((draw_position.x) + 0.5, -(draw_position.y) + 0.5, 0.01f);
			glEnd();
			glColor3f(1.0f, 1.0f, 1.0f);
			glEnable(GL_TEXTURE_2D);
		glPopMatrix();
	}

	virtual void take_input(boundinput input, bool type, bool queue_add_toggle) {};

	FOWCommand current_command;
	t_vertex draw_position;
	std::vector<FOWCommand> command_queue;
	GridCharacterState state;
	static GridManager *grid_manager;
	bool selected;

};