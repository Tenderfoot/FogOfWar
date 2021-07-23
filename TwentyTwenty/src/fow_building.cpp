
#include "fow_building.h"

FOWBuilding::FOWBuilding()
{
}

FOWBuilding::FOWBuilding(int x, int y, int size)
{
	type = FOW_BUILDING;
	position.x = x;
	position.y = y;
	this->size = size;
	color = t_vertex(1, 1, 1);

	skeleton = new spine::Skeleton(SpineManager::skeletonData["buildings"]);
	skeleton->setToSetupPose();
	skeleton->updateWorldTransform();

	animationState = new spine::AnimationState(SpineManager::stateData["buildings"]);
}

void FOWBuilding::draw()
{
	glPushMatrix();
		glTranslatef(position.x - 0.5, -position.y + 0.5, 0.01f);
		glColor3f(color.x, color.y, color.z);
		SpineManager::drawSkeleton(skeleton);
		glColor3f(1, 1, 1);
	glPopMatrix();
}