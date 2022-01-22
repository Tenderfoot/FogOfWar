
#include "common.h"
#include "spine_entity.h"

SpineEntity::SpineEntity() : GameEntity()
{
	skeleton = nullptr;
	animationState = nullptr;
	color = t_vertex(1, 1, 1);
	draw_offset = t_vertex(0, 0, 0);
	visible = true;
	dirty_vbo = false;
}


// Skins (the next 3 methods) can be added and removed by the clienthandler
// but the clienthandler thread can't do anything OpenGL related (opengl context)
// so dirty_vbo is required to initiate a reset on the main thread
void SpineEntity::set_skin(std::string skin_name)
{
	current_skin = new spine::Skin(skin_name.c_str());
	current_skin->addSkin(SpineManager::skeletonData[skeleton_name.c_str()]->findSkin(skin_name.c_str()));
	skeleton->setSkin(current_skin);
}

void SpineEntity::add_to_skin(std::string skin_name)
{
	spine::Skin* sp_current_skin = skeleton->getSkin();
	sp_current_skin->addSkin(SpineManager::skeletonData[skeleton_name.c_str()]->findSkin(skin_name.c_str()));
	skeleton->setSkin(sp_current_skin);
	skeleton->setToSetupPose();
	skeleton->updateWorldTransform();
	dirty_vbo = true;
}

void SpineEntity::reset_skin()
{
	current_skin = new spine::Skin(skin_name.c_str());
	current_skin->addSkin(SpineManager::skeletonData[skeleton_name.c_str()]->findSkin(skin_name.c_str()));
	skeleton->setSkin(current_skin);
	skeleton->setToSetupPose();
	skeleton->updateWorldTransform();
	dirty_vbo = true;
}

void SpineEntity::update(float timedelta) 
{
	// this if is for the OpenGL thread problem
	if (skeleton != nullptr)
	{
		// this dirty_vbo stuff works when I put it here
		// but failed where I intended to put it which was draw
		// I'm not 100% sure why
		if (dirty_vbo == true)
		{
			SpineManager::reset_vbo(skeleton, &VBO);
			dirty_vbo = false;
		}

		animationState->update(timedelta);
		animationState->apply(*skeleton);
	}
};

void SpineEntity::build_vbo()
{
}

t_transform SpineEntity::get_aabb()
{
	t_transform aabb;
	float x1 = draw_position.x - 0.5;
	float y1 = draw_position.y - 1;
	float x2 = draw_position.x + 0.5;
	float y2 = draw_position.y;

	aabb.x = std::min(x1, x2);
	aabb.w = std::max(x1, x2);
	aabb.y = std::min(y1, y2);
	aabb.h = std::max(y1, y2);

	return aabb;
}

void SpineEntity::set_animation(std::string animation_name)
{
	if (spine_initialized)
	{
		animationState->setAnimation(0, animation_name.c_str(), true);
	}
	else
	{
		printf("Tried to set animation on uninitialized entity!\n");
	}
}

void SpineEntity::draw() 
{
	if (visible)
	{
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glPushMatrix();
		glTranslatef(draw_position.x + draw_offset.x, -draw_position.y + draw_offset.y, 0.1f);
		if (flip)
			glRotatef(180, 0.0f, 1.0f, 0.0f);
		PaintBrush::draw_vbo(VBO);
		glPopMatrix();
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
	}
};