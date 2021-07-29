
#include "fow_building.h"
#include "fow_player.h"
#include "gatherer.h"
#include "audiocontroller.h"

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

	size = 3;
}

void FOWBuilding::draw()
{
	// important for selection box
	draw_position = position;

	if (selected)
		draw_selection_box();

	glPushMatrix();
		glTranslatef(position.x - 0.5, -position.y + 0.5, 0.01f);
		glColor3f(color.x, color.y, color.z);
		SpineManager::drawSkeleton(skeleton);
		glColor3f(1, 1, 1);
	glPopMatrix();
}

FOWTownHall::FOWTownHall()
{
}

FOWTownHall::FOWTownHall(int x, int y, int size) : FOWBuilding(x, y, size)
{
	type = FOW_TOWNHALL;
	current_skin = new spine::Skin(*SpineManager::skeletonData["buildings"]->findSkin("TownHall"));
	skeleton->setSkin(current_skin);
}

void FOWTownHall::process_command(FOWCommand next_command)
{

	if (next_command.type == BUILD_UNIT)
	{
		printf("Build Unit command recieved\n");
		FOWGatherer* new_gatherer = new FOWGatherer();
		new_gatherer->owner = GridManager::player;
		new_gatherer->position = t_vertex(position.x + 4, position.y, 0.0f);
		grid_manager->entities->push_back((GameEntity*)new_gatherer);
	}

	FOWSelectable::process_command(next_command);
};

void FOWTownHall::take_input(boundinput input, bool type, bool queue_add_toggle)
{
	FOWPlayer* player = GridManager::player;
	if (input == ACTION)
	{
		if (player->gold > 0)
		{
			player->gold--;
			process_command(FOWCommand(BUILD_UNIT, FOW_GATHERER));
		}
		else
		{
			if (type == true && SDL_GetTicks() - player->last_poor_warning > 2500)
			{
				AudioController::play_sound("data/sounds/notenough.wav");
				player->last_poor_warning = SDL_GetTicks();
			}
		}
	}
}