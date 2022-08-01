
#include "fow_decoration_manager.h"
#include "fow_decoration.h"
#include "game.h"

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
							decorations.push_back(new_decoration);
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
						decorations.push_back(new_decoration);
						tile_ref->decorations.push_back(new_decoration);
						new_decoration = new FOWDecoration("tree", t_vertex(widthItr + 0.5, heightItr, 0), tile_ref);
						decorations.push_back(new_decoration);
						tile_ref->decorations.push_back(new_decoration);
						new_decoration = new FOWDecoration("tree", t_vertex(widthItr, heightItr - 0.5, 0), tile_ref);
						decorations.push_back(new_decoration);
						tile_ref->decorations.push_back(new_decoration);
					}
					new_decoration = new FOWDecoration("tree", t_vertex(widthItr, heightItr, 0), tile_ref);
					decorations.push_back(new_decoration);
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
						decorations.push_back(new FOWDecoration("cattail", t_vertex(widthItr + (((rand() % 2) == 0 ? -1 : 1) * (((float)(rand() % 50)) / 100)), heightItr + (((rand() % 2) == 0 ? -1 : 1) * (((float)(rand() % 50))) / 100), 0), tile_ref));
						decorations.push_back(new FOWDecoration("cattail", t_vertex(widthItr + (((rand() % 2) == 0 ? -1 : 1) * (((float)(rand() % 50)) / 100)), heightItr + (((rand() % 2) == 0 ? -1 : 1) * (((float)(rand() % 50))) / 100), 0), tile_ref));
					}
				}
			}
		}
	}

	std::sort(decorations.begin(), decorations.end(), sort_by_y);

	for (std::string type : FOWDecoration::decoration_types)
	{
		FOWDecoration::assemble_megatron(type);
	}
}

void FOWDecorationManager::draw_vao()
{
	for (std::string type : FOWDecoration::decoration_types)
	{
		PaintBrush::draw_vao(FOWDecoration::megatron_vbo[type]);
	}
}

// This function doesn't run on update
// it actually runs in its own thread and updates the decoration vertex positions
void FOWDecorationManager::update()
{
	float timedelta = 0;
	float previous_time = 0;
	while (!Game::done)
	{
		timedelta = (SDL_GetTicks() - previous_time) / 1000;
		previous_time = SDL_GetTicks();

		for (std::string type : FOWDecoration::decoration_types)
		{
			FOWDecoration::clear_totals(type);
		}

		if (FOWDecorationManager::decorations.size() > 0)
		{
			for (std::string type : FOWDecoration::decoration_types)
			{
				((FOWDecoration*)FOWDecorationManager::decorations.at(0))->update_skeleton(type, timedelta);
			}
		}

		// this throws a read error sometimes on close
		// this thread needs to be able to be cleaned up
		for (auto thing : FOWDecorationManager::decorations)
		{
			((FOWDecoration*)thing)->make_totals();
		}
	}
}

// This just rebinds the updated data
void FOWDecorationManager::game_update()
{
	FOWDecoration::reset_decorations();

	for (std::string type : FOWDecoration::decoration_types)
	{
		FOWDecoration::update_megatron(type);
	}
}