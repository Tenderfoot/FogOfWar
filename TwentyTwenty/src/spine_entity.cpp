
#include "common.h"
#include "spine_entity.h"

SpineEntity::SpineEntity() : GameEntity()
{
	skeleton = nullptr;
	animationState = nullptr;
	color = t_vertex(1, 1, 1);
	draw_offset = t_vertex(0, 0, 0);
	visible = true;
}

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
	SpineManager::reset_vbo(skeleton, &VBO);
}

void SpineEntity::reset_skin()
{
	current_skin = new spine::Skin(skin_name.c_str());
	current_skin->addSkin(SpineManager::skeletonData[skeleton_name.c_str()]->findSkin(skin_name.c_str()));
	skeleton->setSkin(current_skin);
	skeleton->setToSetupPose();
	skeleton->updateWorldTransform();
	SpineManager::reset_vbo(skeleton, &VBO);
}

void SpineEntity::update(float timedelta) 
{
	// this if is for the OpenGL thread problem
	if (skeleton != nullptr)
	{
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
	float x1 = transform.x - (transform.w / 2);
	float y1 = transform.y;
	float x2 = transform.x + (transform.w / 2);
	float y2 = transform.y + transform.h;

	aabb.x = std::min(x1, x2);
	aabb.w = std::max(x1, x2);
	aabb.y = std::min(y1, y2);
	aabb.h = std::max(y1, y2);

	return aabb;
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