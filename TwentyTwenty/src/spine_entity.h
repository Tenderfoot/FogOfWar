#pragma once

#include <string>
#include "game_entity.h"
#include "spine_manager.h"

class SpineEntity : public GameEntity
{
public:

	SpineEntity();
	SpineEntity(std::string skin_name);

	virtual void update(float timedelta);
	virtual t_transform get_aabb();
	virtual void draw();
	void build_vbo();
	void set_skin(std::string skin_name);
	void add_to_skin(std::string skin_name);
	void reset_skin();
	void set_animation(std::string animation_name);
	virtual void load_spine_data(std::string spine_file, std::string skin_name);
	void build_spine();
	virtual void char_init() {};

	t_VBO VBO;

	bool dirty_vbo;
	bool spine_initialized;
	std::string skeleton_name;
	std::string skin_name;
	spine::Skin* current_skin;
	spine::Skeleton *skeleton;
	spine::AnimationState *animationState;
	bool flip;
	t_vertex color;
};