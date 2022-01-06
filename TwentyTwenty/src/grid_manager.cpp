#include <sstream>
#include <fstream>
#include "grid_manager.h"
#include "gatherer.h"
#include "knight.h"
#include "undead.h"
#include "fow_player.h"
#include "fow_building.h"


// Nice, nullptr is the way to go for handling pointers
FOWPlayer* GridManager::player = nullptr;

static const int war2_autotile_map[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
										-1, -1, -1, -1, 13, 13, -1, -1, -1, -1,
										-1, -1, -1, -1, -1, -1, -1, 13, -1, -1,
										13, 13, -1, -1, -1, -1, -1, -1, -1, -1,
										-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
										-1, -1, -1, -1, -1, -1, 7, 7, -1, 7,
										7, 7, 5, 5, -1, -1, -1, -1, -1, -1,
										-1, -1, -1, -1, -1, -1, -1, -1, 13, 13,
										-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
										-1, -1, -1, -1, 13, 13, -1, -1, -1, -1,
										-1, -1, -1, -1, -1, -1, -1, -1, 7, -1,
										13, 13, -1, -1, -1, -1, -1, -1, -1, -1,
										7, 7, -1, 7, 7, 7, 5, 5, -1, -1,
										-1, 14, -1, -1, -1, 14, -1, -1, -1, -1,
										-1, -1, -1, 12, -1, -1, -1, 14, -1, -1,
										-1, 14, -1, -1, -1, 14, -1, -1, -1, 12,
										-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
										-1, -1, -1, -1, -1, -1, -1, 11, -1, 14,
										-1, -1, -1, 14, -1, 7, -1, 6, -1, 7,
										-1, 4, -1, -1, -1, 14, -1, -1, 14, 14,
										-1, -1, -1, -1, -1, -1, 13, 12, -1, -1,
										-1, 14, -1, -1, -1, 14, -1, -1, -1, 14,
										-1, -1, 13, 12, 11, 11, -1, 10, 11, 11,
										11, 10, -1, -1, -1, -1, 11, 11, 9, 8,
										11, 11, -1, 10, 11, 11, 11, 10, 3, 3,
										-1, 2, 3, 3, 1, 0 };

// I'd pass these as const t_tile&, it means you aren't worried about null
float heuristic_cost_estimate(const t_tile& a, const t_tile& b)
{
	return (abs(b.x - a.x) + abs(b.y - a.y));
}

// I'd pass these as const t_tile&, it means you aren't worried about null
bool are_equal(const t_tile& a, const t_tile& b)
{
	return ((a.x == b.x) && (a.y == b.y));
}

// I'd pass these as std::vector<t_tile*>& and const t_tile&
bool in_set(std::vector<t_tile*>& set, const t_tile& vertex)
{
	for (auto item : set)
	{
		if (are_equal(*item, vertex))
		{
			return true;
		}
	}
	return false;
}

void GridManager::draw_path(const t_vertex &start_pos)
{
	auto path_to_draw = find_path(start_pos, t_vertex(mouse_x, 0, mouse_y));

	if (path_to_draw.size() > 0)
	{
		for (auto pathItr : path_to_draw)
		{
			pathItr->in_path = true;
		}
	}
}

int GridManager::num_path(const t_vertex& start_pos)
{
	auto new_path = find_path(start_pos, t_vertex(mouse_x, 0, mouse_y));
	return new_path.size();
}

bool GridManager::position_visible(int x, int z)
{
	return tile_map[x][z].visible;
}

GameEntity* GridManager::entity_on_position(t_vertex entity_pos)
{
	return tile_map[(int)entity_pos.x][(int)entity_pos.y].entity_on_position;
}

// Note: load_map invokes this
void from_json(const nlohmann::json& j, std::map<int, std::map<int, t_tile>>& new_tile_map)
{
	int width, height;

	j.at("width").get_to(width);
	j.at("height").get_to(height);

	printf("%d %d\n", width, height);

	// Is it possible "tiles" won't be there?
	nlohmann::json tile_data = j.at("tiles");

	int widthItr, heightItr;
	for (widthItr = 0; widthItr < width; widthItr++)
	{
		for (heightItr = 0; heightItr < height; heightItr++)
		{
			new_tile_map[widthItr][heightItr] = t_tile();
			tile_data.at(std::to_string(widthItr)).at(std::to_string(heightItr)).at("type").get_to(new_tile_map[widthItr][heightItr].type);

			new_tile_map[widthItr][heightItr].x = widthItr;
			new_tile_map[widthItr][heightItr].y = heightItr;
			new_tile_map[widthItr][heightItr].gscore = INFINITY;
			new_tile_map[widthItr][heightItr].fscore = INFINITY;

			if (new_tile_map[widthItr][heightItr].type > 1)
			{
				new_tile_map[widthItr][heightItr].wall = 1;
			}

			if (tile_data.at(std::to_string(widthItr)).at(std::to_string(heightItr)).at("entities").is_null() == false)
			{
				nlohmann::json entity_data;
				tile_data.at(std::to_string(widthItr)).at(std::to_string(heightItr)).at("entities").get_to(entity_data);
				int type;
				entity_data.at("type").get_to(type);
				// I'd use a std::unique_ptr or std::shared_ptr here
				GameEntity* new_entity = GridManager::create_entity((entity_types)type, t_vertex(widthItr, heightItr, 0));
				if (new_entity != nullptr)
					new_tile_map[widthItr][heightItr].entity_on_position = new_entity;
			}

			// I'd add { } even around 1 line statements
			else
				new_tile_map[widthItr][heightItr].entity_on_position = nullptr;
		}
	}
}

GameEntity* GridManager::create_entity(const entity_types& type, const t_vertex& position)
{

	// I'd use a std::unique_ptr or std::shared_ptr here to prevent memory leaks
	FOWCharacter* new_character;
	FOWBuilding* new_building;

	if (type == FOW_GATHERER)
	{
		new_character = new FOWGatherer(t_vertex(position.x, position.y, 0));
		new_character->owner = player;
		new_character->team_id = 0;
		return new_character;
	}
	
	if (type == FOW_SKELETON)
	{
		new_character = new FOWUndead(t_vertex(position.x, position.y, 0));
		new_character->team_id = 1;
		return new_character;
	}
	
	if (type == FOW_KNIGHT)
	{
		new_character = new FOWKnight(t_vertex(position.x, position.y, 0));
		new_character->owner = player;
		new_character->team_id = 0;
		return new_character;
	}
	
	if (type == FOW_TOWNHALL)
	{
		new_building = new FOWTownHall(position.x, position.y);
		return new_building;
	}
	
	if (type == FOW_GOLDMINE)
	{
		new_building = new FOWGoldMine(position.x, position.y);
		return new_building;
	}

	if (type == FOW_FARM)
	{
		new_building = new FOWFarm(position.x, position.y);
		return new_building;
	}

	if (type == FOW_BARRACKS)
	{
		new_building = new FOWBarracks(position.x, position.y);
		return new_building;
	}
	
	if (type == FOW_ENEMYSPAWNER)
	{
		new_building = new FOWEnemySpawner(position.x, position.y);
		new_building->team_id = 1;
		return new_building;
	}

	return nullptr;
}


GameEntity *GridManager::build_and_add_entity(const entity_types& type, const t_vertex& position)
{
	GameEntity* new_entity = create_entity(type, position);
	((FOWSelectable*)new_entity)->dirty_tile_map();
	entities->push_back(new_entity);
	return new_entity;
}

void GridManager::save_map(const std::string& mapname)
{

	nlohmann::json j =
	{
		{"name", "test"},
		{"width", width},
		{"height", height},
		{"tiles", nlohmann::json({}) },
	};

	std::vector<GameEntity*> used_entities;

	for (int widthItr = 0; widthItr < width; widthItr++)
	{
		for (int heightItr = 0; heightItr < height; heightItr++)
		{
			t_tile* current_tile = &tile_map[widthItr][heightItr];

			j["tiles"][std::to_string(widthItr)][std::to_string(heightItr)] = nlohmann::json({ {"type", current_tile->type},
														{"x", current_tile->x},
														{"y", current_tile->y},
														{"entities", {}} });

			if (current_tile->entity_on_position != nullptr && std::find(used_entities.begin(), used_entities.end(), current_tile->entity_on_position) == used_entities.end())
			{
				GameEntity* current_entity = current_tile->entity_on_position;
				used_entities.push_back(current_entity);
				j["tiles"][std::to_string(widthItr)][std::to_string(heightItr)]["entities"] = nlohmann::json({ {"type", current_entity->type},
														{"x", current_entity->position.x},
														{"y", current_entity->position.y} });
			}
		}
	}

	// I'd recommend some error handling here incase the file can't be openned, otherwise the code
	// will throw an error
	std::ofstream o(mapname);
	o << std::setw(4) << j << std::endl;
}

void GridManager::load_map(const std::string &mapname)
{
	nlohmann::json level_data;
	// I'd confirm that the file open worked
	std::ifstream i(mapname);
	i >> level_data;

	// import settings
	tile_map = level_data.get<std::map<int, std::map<int, t_tile>>>();

	width = tile_map.size();
	height = tile_map[0].size();

	int widthItr, heightItr;
	for (widthItr = 0; widthItr < width; widthItr++)
	{
		for (heightItr = 0; heightItr < height; heightItr++)
		{
			if (tile_map[widthItr][heightItr].entity_on_position != nullptr)
			{
				entities->push_back(tile_map[widthItr][heightItr].entity_on_position);
				((FOWSelectable*)tile_map[widthItr][heightItr].entity_on_position)->dirty_tile_map();
			}
		}
	}

	printf("Level dimensions: %d x %d\n", width, height);
}

void GridManager::init()
{
	load_map("data/gardenofwar.json");
	calc_all_tiles();

	game_speed = 1;

	fake_tex[0] = PaintBrush::Soil_Load_Texture("data/images/war2autotile_grasstodirt.png", TEXTURE_CLAMP);
	fake_tex[1] = PaintBrush::Soil_Load_Texture("data/images/war2autotile_watertodirt.png", TEXTURE_CLAMP);
	fake_tex[2] = PaintBrush::Soil_Load_Texture("data/images/war2autotile_rockstodirt.png", TEXTURE_CLAMP);
	fake_tex[3] = PaintBrush::Soil_Load_Texture("data/images/war2autotile_treestograss.png", TEXTURE_CLAMP);
	real_tex[0] = PaintBrush::Soil_Load_Texture("data/images/war2autotile_grasstodirt_real.png", TEXTURE_CLAMP);
	real_tex[1] = PaintBrush::Soil_Load_Texture("data/images/war2autotile_watertodirt_real.png", TEXTURE_CLAMP);
	real_tex[2] = PaintBrush::Soil_Load_Texture("data/images/war2autotile_rockstodirt_real.png", TEXTURE_CLAMP);
	real_tex[3] = PaintBrush::Soil_Load_Texture("data/images/war2autotile_treestograss_real.png", TEXTURE_CLAMP);

	last_path = &tile_map[x][y];
}

void GridManager::set_mouse_coords(t_transform mouse_position)
{
	mouse_x = int(real_mouse_position.x + 0.5);
	mouse_y = int(-real_mouse_position.y + 0.5);
	real_mouse_position = mouse_position;
	mouse_x = std::min(width, std::max(mouse_x, 0));
	mouse_y = std::min(height, std::max(mouse_y, 0));
}

// this is dead code
t_vertex GridManager::convert_mouse_coords(t_vertex mouse_space)
{
	return mouse_space;
}

void GridManager::clear_path()
{
	for (int widthItr = 0; widthItr < width; widthItr++)
	{
		for (int heightItr = 0; heightItr < height; heightItr++)
		{
			tile_map[widthItr][heightItr].gscore = INFINITY;
			tile_map[widthItr][heightItr].fscore = INFINITY;
			tile_map[widthItr][heightItr].cameFrom.x = 0;
			tile_map[widthItr][heightItr].cameFrom.y = 0;
			tile_map[widthItr][heightItr].in_path = false;
		}
	}
}

std::vector<t_tile*> GridManager::find_path(t_vertex start_pos, t_vertex end_pos, bool use_teams, int team)
{
	t_tile* start = &tile_map[start_pos.x][start_pos.y];
	t_tile* goal = &tile_map[end_pos.x][end_pos.y];

	clear_path();

	std::vector<t_tile*> return_vector;

	if (are_equal(*start, *goal))
		return return_vector;

	// The set of nodes already evaluated.
	std::vector<t_tile*> closedSet = {};
	// The set of currently discovered nodes still to be evaluated.
	// Initially, only the start node is known.
	std::vector<t_tile*> openSet = { start };
	// For each node, which node it can most efficiently be reached from.
	// If a node can be reached from many nodes, cameFrom will eventually contain the
	// most efficient previous step.

	int i;
	int j;

	t_tile* current = start;
	t_tile* neighbour;

	current->gscore = 0;
	current->fscore = heuristic_cost_estimate(*start, *goal);

	int recursion_depth = 0;

	while (openSet.size() > 0)
	{
		recursion_depth++;
		float current_fscore = INFINITY;
		for (i = 0; i < openSet.size(); i++)
		{
			if (openSet.at(i)->fscore < current_fscore)
			{
				current = openSet.at(i);
				current_fscore = current->fscore;
			}
		}

		if (are_equal(*current, *goal))
		{
			// success
			while (current != start)
			{
				return_vector.push_back(current);
				current = &tile_map[current->cameFrom.x][current->cameFrom.y];
			}
			return return_vector;
		}

		for (i = 0; i < openSet.size(); i++)
		{
			if (are_equal(*current, *openSet.at(i)))
			{
				openSet.erase(openSet.begin() + i);
			}
		}

		closedSet.push_back(current);

		for (j = 0; j < 8; j++)
		{
			int new_x, new_y;
			bool valid = true;
			switch (j)
			{
			case 0:
				new_x = current->x - 1;
				new_y = current->y - 1;
				if (tile_map[current->x - 1][current->y].wall == 1 || tile_map[current->x][current->y - 1].wall == 1)
					valid = false;
				break;
			case 1:
				new_x = current->x;
				new_y = current->y - 1;
				break;
			case 2:
				new_x = current->x + 1;
				new_y = current->y - 1;
				if (tile_map[current->x + 1][current->y].wall == 1 || tile_map[current->x][current->y - 1].wall == 1)
					valid = false;
				break;
			case 3:
				new_x = current->x - 1;
				new_y = current->y;
				break;
			case 4:
				new_x = current->x + 1;
				new_y = current->y;
				break;
			case 5:
				new_x = current->x - 1;
				new_y = current->y + 1;
				if (tile_map[current->x - 1][current->y].wall == 1 || tile_map[current->x][current->y + 1].wall == 1)
					valid = false;
				break;
			case 6:
				new_x = current->x;
				new_y = current->y + 1;
				break;
			case 7:
				new_x = current->x + 1;
				new_y = current->y + 1;
				if (tile_map[current->x + 1][current->y].wall == 1 || tile_map[current->x][current->y + 1].wall == 1)
					valid = false;
				break;
			}

			// this helps with ignoring enemies while attack moving
			bool condition2 = false;
			if (tile_map[new_x][new_y].entity_on_position != nullptr)
			{
				condition2 = use_teams && ((FOWSelectable*)tile_map[new_x][new_y].entity_on_position)->is_unit() && ((FOWSelectable*)tile_map[new_x][new_y].entity_on_position)->team_id != team;
			}

			if ((new_x >= 0 && new_x < width && new_y >= 0 && new_y < height) && tile_map[new_x][new_y].wall == 0 && (tile_map[new_x][new_y].entity_on_position == nullptr || condition2) && valid)
			{
				neighbour = &tile_map[new_x][new_y];
			}
			else
				continue;

			if (in_set(closedSet, *neighbour))
				continue;		// Ignore the neighbor which is already evaluated. 

			float tentative_gScore;
			tentative_gScore = current->gscore + 1;

			if (!in_set(openSet, *neighbour))	// Discover a new node
				openSet.push_back(neighbour);
			else if (tentative_gScore >= neighbour->gscore)
				continue;		// This is not a better path.

			// This path is the best until now. Record it!
			neighbour->cameFrom.x = current->x;
			neighbour->cameFrom.y = current->y;
			neighbour->gscore = tentative_gScore;
			neighbour->fscore = neighbour->gscore + heuristic_cost_estimate(*neighbour, *goal);
		}

		if (recursion_depth > MAXIMUM_RECUSION_DEPTH)
			return return_vector;
	}

	return return_vector;
}

void GridManager::dropblob(int i, int j, tiletype_t blobtype)
{
	int wall = 0;
	if (blobtype == TILE_WATER || blobtype == TILE_ROCKS)
	{
		wall = 1;
	}

	tile_map[i][j].type = blobtype;
	tile_map[i][j].wall = wall;
	tile_map[i + 1][j].type = blobtype;
	tile_map[i + 1][j].wall = wall;
	tile_map[i][j + 1].type = blobtype;
	tile_map[i][j + 1].wall = wall;
	tile_map[i + 1][j + 1].type = blobtype;
	tile_map[i + 1][j + 1].wall = wall;

}

void GridManager::randomize_map()
{

	// For indexed for-loops I would suggest names like widthItr, heightItr to
	// aid in readability 
	for (int widthItr = 1; widthItr < width - 2; widthItr++)
	{
		for (int heightItr = 1; heightItr < height - 2; heightItr++)
		{
			tile_map[widthItr][heightItr].type = TILE_DIRT;
			tile_map[widthItr][heightItr].wall = 0;
		}
	}

	tiletype_t new_type = TILE_GRASS;
	for (int i = 1; i < width - 3; i++)
	{
		for (int j = 1; j < height - 3; j++)
		{
			if (rand() % 2 == 0)
			{
				dropblob(i, j, new_type);
			}
		}
	}

	new_type = TILE_WATER;
	for (int i = 1; i < width - 3; i++)
	{
		for (int j = 1; j < height - 3; j++)
		{
			if (rand() % 10 == 0)
			{
				dropblob(i, j, new_type);
			}
		}
	}

	new_type = TILE_ROCKS;
	for (int i = 1; i < width - 3; i++)
	{
		for (int j = 1; j < height - 3; j++)
		{
			if (rand() % 50 == 0)
			{
				dropblob(i, j, new_type);
			}
		}
	}

	
	new_type = TILE_TREES;
	for (int i = 2; i < width - 4; i++)
	{
		for (int j = 2; j < height - 4; j++)
		{
			if (rand() % 2 == 0)
			{
				dropblob(i, j, new_type);
			}
		}
	}

	cull_orphans();
	calc_all_tiles();
}

// Pass as const reference
std::vector<GameEntity*> GridManager::get_entities_of_type(const entity_types& type)
{
	// If these pointers are used just for inspection I'd recommend
	// a std::weak_ptr
	std::vector<GameEntity*> return_list;

	for (auto entityItr : *entities)
	{
		if (entityItr->type == type)
		{
			return_list.push_back(entityItr);
		}
	}

	return return_list;
}

// Pass as const reference
bool GridManager::space_free(const t_vertex& position, const int& size)
{
	for (int i = 0; i < size; i++)
		for (int j = 0; j < size; j++)
			if (tile_map[i+position.x][j+position.y].wall)
				return false;

	return true;
}

void GridManager::cull_orphans()
{
	for (int i = 1; i < width - 2; i++)
	{
		for (int j = 1; j < height - 2; j++)
		{
			bool found = false;
			tiletype_t current_type = tile_map[i][j].type;
			
			if (current_type != 0)
			{
				if (check_compatible(i, j-1, current_type))
				{
					if (check_compatible(i - 1, j, current_type))
					{
						if (check_compatible(i - 1, j - 1, current_type))
						{
							found = true;
						}
					}

					if (tile_map[i + 1][j].type == current_type)
					{
						if (tile_map[i + 1][j - 1].type == current_type)
						{
							found = true;
						}
					}
				}

				if (check_compatible(i, j + 1, current_type))
				{
					if (check_compatible(i - 1, j, current_type))
					{
						if (check_compatible(i - 1, j + 1, current_type))
						{
							found = true;
						}
					}

					if (check_compatible(i + 1, j, current_type))
					{
						if (check_compatible(i + 1, j + 1, current_type))
						{
							found = true;
						}
					}
				}

				if (found == false)
				{
					tile_map[i][j].type = TILE_DIRT;
				}
			}
		}
	}
}

bool GridManager::check_compatible(int i, int j, tiletype_t current_type)
{
	// I'd use an enum for the types to make handling the types
	// more readable
	if (current_type == 1)
		if (tile_map[i][j].type == 1 || tile_map[i][j].type == 4)
			return true;

	return tile_map[i][j].type == current_type;
}


int GridManager::include_perimeter(int i, int j)
{
	int tex_wall = 0;

	// I think this is just so I don't go off the end... needs a better solution
	if (!(i > 0 && j > 0))
		tex_wall = (tex_wall | 1);

	if (!(j > 0))
		tex_wall = (tex_wall | 2);

	if (!(i < width - 1 && j > 0))
		tex_wall = (tex_wall | 4);

	if (!(i < width - 1))
		tex_wall = (tex_wall | 8);

	if (!(i < width - 1 && j < height - 1))
		tex_wall = (tex_wall | 16);

	if (!(j < height - 1))
		tex_wall = (tex_wall | 32);

	if (!(i > 0 && j < height - 1))
		tex_wall = (tex_wall | 64);

	if (!(i > 0))
		tex_wall = (tex_wall | 128);

	return tex_wall;
}

int GridManager::calculate_tile(int i, int j, tiletype_t current_type)
{
	int tex_wall = 0;

	tex_wall = include_perimeter(i, j);

	if (check_compatible(i - 1, j - 1, current_type))
		tex_wall = (tex_wall | 1);

	if (check_compatible(i, j - 1, current_type))
		tex_wall = (tex_wall | 2);

	if (check_compatible(i + 1, j - 1, current_type))
		tex_wall = (tex_wall | 4);

	if (check_compatible(i + 1, j, current_type))
		tex_wall = (tex_wall | 8);

	if (check_compatible(i + 1, j + 1, current_type))
		tex_wall = (tex_wall | 16);

	if (check_compatible(i, j + 1, current_type))
		tex_wall = (tex_wall | 32);

	if (check_compatible(i - 1, j + 1, current_type))
		tex_wall = (tex_wall | 64);

	if (check_compatible(i - 1, j, current_type))
		tex_wall = (tex_wall | 128);

	return war2_autotile_map[tex_wall];
}


void GridManager::calc_all_tiles()
{
	int i, j;

	for (i = 0; i < width; i++)
	{
		for (j = 0; j < height; j++)
		{
			tile_map[i][j].tex_wall = calculate_tile(i, j, tile_map[i][j].type);
		}
	}
}

//183=14
void GridManager::draw_autotile()
{
	int i, j;
	bool test;

	for (i = 0; i < width; i++)
	{
		for (j = 0; j < height; j++)
		{
			t_tile current_tile = tile_map[i][j];
			if (current_tile.tex_wall == -1)
			{
				current_tile.tex_wall = 15;
			}

			if (current_tile.type == 0)
			{
				current_tile.tex_wall = 15;
			}
				
			int xcoord = current_tile.tex_wall % 4;
			int ycoord = current_tile.tex_wall / 4;

			GLuint *texture_set;

			if (use_tex)
				texture_set = fake_tex;
			else
				texture_set = real_tex;

			glEnable(GL_DEPTH_TEST);

			glPushMatrix();
				glTranslatef(i, -j, 0.0f);

				if (current_tile.type == 0 || current_tile.type == 1)
				{
					glBindTexture(GL_TEXTURE_2D, texture_set[0]);
				}
				else
				{
					glBindTexture(GL_TEXTURE_2D, texture_set[current_tile.type-1]);
				}

				glPushMatrix();
					glBegin(GL_QUADS);
						glTexCoord2f(0.25f + (0.25f * xcoord), 0.0f + (0.25f * ycoord)); 	glVertex3f(0.5f, 0.5f, 0.0f);
						glTexCoord2f(0.0f + (0.25f * xcoord), 0.0f + (0.25f * ycoord)); 	glVertex3f(-0.5f, 0.5f, 0.0f);
						glTexCoord2f(0.0f + (0.25f * xcoord), 0.25f + (0.25f * ycoord));	glVertex3f(-0.5f, -0.5f, 0.0f);
						glTexCoord2f(0.25f + (0.25f * xcoord), 0.25f + (0.25f * ycoord));	glVertex3f(0.5f, -0.5f, 0.0f);
					glEnd();
				glPopMatrix();
			glPopMatrix();
		}
	}
}

void GridManager::reset_visibility()
{
	for (int widthItr = 0; widthItr < width; widthItr++)
	{
		for (int heightItr = 0; heightItr < height; heightItr++)
		{
			tile_map[widthItr][heightItr].visible = false;
		}
	}
}

void GridManager::compute_visibility_raycast(int i, int j, bool discover)
{
	bool found;
	int widthItr, heightItr;

	for (widthItr = 0; widthItr < width; widthItr++)
	{
		for (heightItr = 0; heightItr < height; heightItr++)
		{
			if (!tile_map[widthItr][heightItr].visible)
			{
				tile_map[widthItr][heightItr].visible = point_can_be_seen(i, j, widthItr, heightItr);
			}

			if (tile_map[widthItr][heightItr].visible && discover)
			{
				tile_map[widthItr][heightItr].discovered = true;
			}
		}
	}

}

// Should be updated to use t_vertex
bool GridManager::point_can_be_seen(int i, int j, int i2, int j2)
{
	t_raycast vision_cast;
	vision_cast.init(i, j, i2, j2);

	// while raycasting
	while (vision_cast.has_next())
	{
		int ray_x = vision_cast.get_point().x;
		int ray_y = vision_cast.get_point().y;

		if (tile_map[ray_x][ray_y].wall == 1)
		{
			return false;
		}

		vision_cast.next();
	}

	return true;
}