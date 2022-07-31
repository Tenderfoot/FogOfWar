
#include "fow_decoration_manager.h"
#include "fow_decoration.h"

std::vector<GameEntity*> FOWDecorationManager::decorations;
extern bool sort_by_y(GameEntity* i, GameEntity* j);

void FOWDecorationManager::make_decorations()
{
	GameEntity* new_decoration;
	for (int widthItr = 0; widthItr < GridManager::size.x; widthItr++)
	{
		for (int heightItr = 0; heightItr < GridManager::size.y; heightItr++)
		{
			t_tile *tile_ref = GridManager::get_tile(widthItr, heightItr);

			if (tile_ref->type == TILE_GRASS)
			{
				if (tile_ref->entity_on_position != nullptr)
				{
				}
				else
				{
					if (tile_ref->tex_wall == 0)
					{
						for (int i = 0; i < 25; i++)
						{
							new_decoration = new FOWDecoration("grass", t_vertex(widthItr + (((rand() % 2) == 0 ? -1 : 1) * (((float)(rand() % 50)) / 100)), heightItr + (((rand() % 2) == 0 ? -1 : 1) * (((float)(rand() % 50))) / 100), 0), tile_ref);
							tile_ref->decorations.push_back(new_decoration);
							FOWDecorationManager::decorations.push_back(new_decoration);
						}
					}
				}
			}
			if (tile_ref->type == TILE_TREES)
			{
				if (tile_ref->entity_on_position != nullptr)
				{
				}
				else
				{
					if (tile_ref->tex_wall == 3 || tile_ref->tex_wall == 7 || tile_ref->tex_wall == 11)
					{
					}
					else
					{
						new_decoration = new FOWDecoration("tree", t_vertex(widthItr + 0.5, heightItr - 0.5, 0), tile_ref);
						FOWDecorationManager::decorations.push_back(new_decoration);
						tile_ref->decorations.push_back(new_decoration);
						new_decoration = new FOWDecoration("tree", t_vertex(widthItr + 0.5, heightItr, 0), tile_ref);
						FOWDecorationManager::decorations.push_back(new_decoration);
						tile_ref->decorations.push_back(new_decoration);
						new_decoration = new FOWDecoration("tree", t_vertex(widthItr, heightItr - 0.5, 0), tile_ref);
						FOWDecorationManager::decorations.push_back(new_decoration);
						tile_ref->decorations.push_back(new_decoration);
					}
					new_decoration = new FOWDecoration("tree", t_vertex(widthItr, heightItr, 0), tile_ref);
					FOWDecorationManager::decorations.push_back(new_decoration);
					tile_ref->decorations.push_back(new_decoration);
				}
			}
			if (tile_ref->type == TILE_WATER)
			{
				if (tile_ref->tex_wall == 0)
				{
					if (GridManager::tile_map[widthItr + 1][heightItr].tex_wall != 0 ||
						GridManager::tile_map[widthItr - 1][heightItr].tex_wall != 0 || 
						GridManager::tile_map[widthItr][heightItr + 1].tex_wall != 0 || 
						GridManager::tile_map[widthItr][heightItr - 1].tex_wall != 0)
					{
						FOWDecorationManager::decorations.push_back(new FOWDecoration("cattail", t_vertex(widthItr + (((rand() % 2) == 0 ? -1 : 1) * (((float)(rand() % 50)) / 100)), heightItr + (((rand() % 2) == 0 ? -1 : 1) * (((float)(rand() % 50))) / 100), 0), tile_ref));
						FOWDecorationManager::decorations.push_back(new FOWDecoration("cattail", t_vertex(widthItr + (((rand() % 2) == 0 ? -1 : 1) * (((float)(rand() % 50)) / 100)), heightItr + (((rand() % 2) == 0 ? -1 : 1) * (((float)(rand() % 50))) / 100), 0), tile_ref));
					}
				}
			}
		}
	}

	std::sort(FOWDecorationManager::decorations.begin(), FOWDecorationManager::decorations.end(), sort_by_y);

	for (std::string type : FOWDecoration::decoration_types)
	{
		FOWDecoration::assemble_megatron(type);
	}
}