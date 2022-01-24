#include <sstream>
#include <fstream>
#include "grid_manager.h"
#include "gatherer.h"
#include "knight.h"
#include "undead.h"
#include "fow_player.h"
#include "fow_building.h"
#include "game.h"

t_vertex  GridManager::size;
std::map<int, std::map<int, t_tile>> GridManager::tile_map;
t_VBO GridManager::new_vbo;
GLuint GridManager::tile_atlas[4];
t_tile* GridManager::last_path;
float GridManager::game_speed;
extern lua_State* state;
static std::thread* script_thread{ nullptr };
bool GridManager::tile_map_dirty = false;

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

bool GridManager::position_visible(const t_vertex& check_position)
{
	return tile_map[check_position.x][check_position.y].visible;
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
				int team_id;
				entity_data.at("team_id").get_to(team_id);
				// I'd use a std::unique_ptr or std::shared_ptr here
				// this doesn't use build_and_add_entity because that requires
				// the entire tile map to have been assembled
				GameEntity* new_entity = GridManager::create_entity((entity_types)type, t_vertex(widthItr, heightItr, 0));
				((FOWSelectable*)new_entity)->team_id = team_id;
				Game::entities.push_back(new_entity);
			}

			// I'd add { } even around 1 line statements
			else
				new_tile_map[widthItr][heightItr].entity_on_position = nullptr;
		}
	}
}

void GridManager::init()
{
	load_map("data/gardenofwar_mp.json");

	game_speed = 1;

	tile_atlas[3] = PaintBrush::Soil_Load_Texture("data/images/autotile_textureatlas_sophie.png", TEXTURE_CLAMP);
	tile_atlas[1] = PaintBrush::Soil_Load_Texture("data/images/autotile_textureatlas_snow.png", TEXTURE_CLAMP);
	tile_atlas[2] = PaintBrush::Soil_Load_Texture("data/images/autotile_textureatlas_wasteland.png", TEXTURE_CLAMP);
	tile_atlas[0] = PaintBrush::Soil_Load_Texture("data/images/autotile_textureatlas_marsh.png", TEXTURE_CLAMP);

	// this needs to happen after the texture is set now
	calc_all_tiles();

	// can this get removed?
	last_path = &tile_map[0][0];
}

GameEntity* GridManager::create_entity(const entity_types& type, const t_vertex& position)
{
	// I'd use a std::unique_ptr or std::shared_ptr here to prevent memory leaks
	GameEntity *new_entity = nullptr;
	if (type == FOW_GATHERER) {
		new_entity = new FOWGatherer(position);
	}
	if (type == FOW_SKELETON) {
		new_entity = new FOWUndead(position);
	}
	if (type == FOW_KNIGHT) {
		new_entity = new FOWKnight(position);
	}
	if (type == FOW_TOWNHALL) {
		new_entity = new FOWTownHall(position);
	}
	if (type == FOW_GOLDMINE) {
		new_entity = new FOWGoldMine(position);
	}
	if (type == FOW_FARM) {
		new_entity = new FOWFarm(position);
	}
	if (type == FOW_BARRACKS) {
		new_entity = new FOWBarracks(position);
	}
	if (type == FOW_ENEMYSPAWNER) {
		new_entity = new FOWEnemySpawner(position);
	}
	return new_entity;
}


GameEntity *GridManager::build_and_add_entity(const entity_types& type, const t_vertex& position)
{
	GameEntity* new_entity = create_entity(type, position);
	((FOWSelectable*)new_entity)->dirty_tile_map();
	Game::entities.push_back(new_entity);
	return new_entity;
}

int GridManager::build_and_add_entity(lua_State* state)
{
	// The number of function arguments will be on top of the stack.
	int args = lua_gettop(state);

	printf("build_and_add_entity() was called with %d arguments:\n", args);

	for (int n = 1; n <= args; ++n) {
		printf("  argument %d: '%s'\n", n, lua_tostring(state, n));
	}
	int type = lua_tointeger(state, 1);
	int x = lua_tointeger(state, 2);
	int y = lua_tointeger(state, 3);

	build_and_add_entity((entity_types)type, t_vertex(x, y, 0.0f));

	return 0;
}

static void run_script_thread()
{
	lua_pcall(state, 0, LUA_MULTRET, 0);
}

void GridManager::save_map(const std::string& mapname)
{

	nlohmann::json j =
	{
		{"name", "test"},
		{"width", GridManager::size.x},
		{"height", GridManager::size.y},
		{"tiles", nlohmann::json({}) },
	};

	std::vector<GameEntity*> used_entities;

	for (int widthItr = 0; widthItr < size.x; widthItr++)
	{
		for (int heightItr = 0; heightItr < size.y; heightItr++)
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
																												{"y", current_entity->position.y},
																												{"team_id", ((FOWSelectable*)current_entity)->team_id} 
				});
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
	/************ JSON Level Data ****************/
	nlohmann::json level_data;
	// I'd confirm that the file open worked
	std::ifstream i(mapname);
	i >> level_data;

	// import settings
	tile_map = level_data.get<std::map<int, std::map<int, t_tile>>>();

	size.x = tile_map.size();
	size.y = tile_map[0].size();

	// this has to happen after all the tiles are generated
	for (auto entity : Game::entities)
	{
		((FOWSelectable*)entity)->dirty_tile_map();
	}

	printf("Level dimensions: %d x %d\n", size.x, size.y);

	/************ LUA SCRIPT STUFF ****************/
	// register stuff to the API
	lua_register(state, "build_and_add_entity", build_and_add_entity);

	// load the script
	int result;
	// Load the program; this supports both source code and bytecode files.
	result = luaL_loadfile(state, "data/gardenofwar.lua");
	if (result != LUA_OK) 
	{
		const char* message = lua_tostring(state, -1);
		printf(message);
		lua_pop(state, 1);
		return;
	}

	// execute the script
	script_thread = new std::thread(run_script_thread);
	//lua_pcall(state, 0, LUA_MULTRET, 0);
}

void GridManager::clear_path()
{
	for (int widthItr = 0; widthItr < size.x; widthItr++)
	{
		for (int heightItr = 0; heightItr < size.y; heightItr++)
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

	GameEntity* on_end = tile_map[end_pos.x][end_pos.y].entity_on_position;

	// if they can't get on the final square, don't try to find a path
	if (are_equal(*start, *goal) || on_end != nullptr || tile_map[end_pos.x][end_pos.y].wall == 1)
	{
		if (on_end != nullptr)
		{
			if (((FOWSelectable*)on_end)->is_unit())
			{
				if (((FOWCharacter*)on_end)->team_id == team)
				{
					return return_vector;
				}
			}
		}
		else if(tile_map[end_pos.x][end_pos.y].wall == 1 || are_equal(*start, *goal))
		{
			return return_vector;
		}
	}

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

			if ((new_x >= 0 && new_x < size.x && new_y >= 0 && new_y < size.y) && tile_map[new_x][new_y].wall == 0 && (tile_map[new_x][new_y].entity_on_position == nullptr || condition2) && valid)
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
	for (int widthItr = 1; widthItr < size.x - 2; widthItr++)
	{
		for (int heightItr = 1; heightItr < size.y - 2; heightItr++)
		{
			tile_map[widthItr][heightItr].type = TILE_DIRT;
			tile_map[widthItr][heightItr].wall = 0;
		}
	}

	tiletype_t new_type = TILE_GRASS;
	for (int widthItr = 1; widthItr < size.x - 3; widthItr++)
	{
		for (int heightItr = 1; heightItr < size.y - 3; heightItr++)
		{
			if (rand() % 2 == 0)
			{
				dropblob(widthItr, heightItr, new_type);
			}
		}
	}

	new_type = TILE_WATER;
	for (int widthItr = 1; widthItr < size.x - 3; widthItr++)
	{
		for (int heightItr = 1; heightItr < size.y - 3; heightItr++)
		{
			if (rand() % 10 == 0)
			{
				dropblob(widthItr, heightItr, new_type);
			}
		}
	}

	new_type = TILE_ROCKS;
	for (int widthItr = 1; widthItr < size.x - 3; widthItr++)
	{
		for (int heightItr = 1; heightItr < size.y - 3; heightItr++)
		{
			if (rand() % 50 == 0)
			{
				dropblob(widthItr, heightItr, new_type);
			}
		}
	}

	
	new_type = TILE_TREES;
	for (int widthItr = 2; widthItr < size.x - 4; widthItr++)
	{
		for (int heightItr = 2; heightItr < size.y - 4; heightItr++)
		{
			if (rand() % 2 == 0)
			{
				dropblob(widthItr, heightItr, new_type);
			}
		}
	}

	cull_orphans();
	calc_all_tiles();
}

std::vector<GameEntity*> GridManager::get_entities_of_type(const entity_types& type)
{
	// If these pointers are used just for inspection I'd recommend
	// a std::weak_ptr
	std::vector<GameEntity*> return_list;

	for (auto entityItr : Game::entities)
	{
		if (entityItr->type == type)
		{
			return_list.push_back(entityItr);
		}
	}

	return return_list;
}

bool GridManager::space_free(const t_vertex& position, const int& size)
{
	for (int widthItr = 0; widthItr < size; widthItr++)
		for (int heightItr = 0; heightItr < size; heightItr++)
			if (tile_map[widthItr+position.x][heightItr +position.y].wall)
				return false;

	return true;
}

void GridManager::cull_orphans()
{
	for (int i = 1; i < size.x - 2; i++)
	{
		for (int j = 1; j < size.y - 2; j++)
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
	if (current_type == TILE_GRASS)
	{
		if (tile_map[i][j].type == TILE_GRASS || tile_map[i][j].type == TILE_TREES)
		{
			return true;
		}
	}

	return tile_map[i][j].type == current_type;
}


int GridManager::include_perimeter(int i, int j)
{
	int tex_wall = 0;

	if (!(i > 0 && j > 0))
	{
		tex_wall = (tex_wall | 1);
	}

	if (!(j > 0))
	{
		tex_wall = (tex_wall | 2);
	}

	if (!(i < size.x - 1 && j > 0))
	{
		tex_wall = (tex_wall | 4);
	}

	if (!(i < size.x - 1))
	{
		tex_wall = (tex_wall | 8);
	}

	if (!(i < size.x - 1 && j < size.y - 1))
	{
		tex_wall = (tex_wall | 16);
	}

	if (!(j < size.y - 1))
	{
		tex_wall = (tex_wall | 32);
	}

	if (!(i > 0 && j < size.y - 1))
	{
		tex_wall = (tex_wall | 64);
	}

	if (!(i > 0))
	{
		tex_wall = (tex_wall | 128);
	}

	return tex_wall;
}

int GridManager::calculate_tile(int i, int j, tiletype_t current_type)
{
	int tex_wall = 0;

	tex_wall = include_perimeter(i, j);

	if (check_compatible(i - 1, j - 1, current_type))
	{
		tex_wall = (tex_wall | 1);
	}

	if (check_compatible(i, j - 1, current_type))
	{
		tex_wall = (tex_wall | 2);
	}

	if (check_compatible(i + 1, j - 1, current_type))
	{
		tex_wall = (tex_wall | 4);
	}

	if (check_compatible(i + 1, j, current_type))
	{
		tex_wall = (tex_wall | 8);
	}

	if (check_compatible(i + 1, j + 1, current_type))
	{
		tex_wall = (tex_wall | 16);
	}

	if (check_compatible(i, j + 1, current_type))
	{
		tex_wall = (tex_wall | 32);
	}

	if (check_compatible(i - 1, j + 1, current_type))
	{
		tex_wall = (tex_wall | 64);
	}

	if (check_compatible(i - 1, j, current_type))
	{
		tex_wall = (tex_wall | 128);
	}

	return war2_autotile_map[tex_wall];
}


void GridManager::calc_all_tiles()
{
	for (int widthItr = 0; widthItr < size.x; widthItr++)
	{
		for (int heightItr = 0; heightItr < size.y; heightItr++)
		{
			tile_map[widthItr][heightItr].tex_wall = calculate_tile(widthItr, heightItr, tile_map[widthItr][heightItr].type);
		}
	}

	GridManager::tile_map_dirty = true;
}

void GridManager::generate_autotile_vbo()
{
	new_vbo.num_faces = size.x * size.y * 6;	// two triangles I guess
	new_vbo.verticies = std::shared_ptr<float[]>(new float[new_vbo.num_faces * 3]);
	new_vbo.colors = std::shared_ptr<float[]>(new float[new_vbo.num_faces * 3]);
	new_vbo.texcoords = std::shared_ptr<float[]>(new float[new_vbo.num_faces * 2]);

	// is this ok with shared_ptr?
	float* verticies = new_vbo.verticies.get();
	float* texcoords = new_vbo.texcoords.get();
	float* colors = new_vbo.colors.get();

	PaintBrush::generate_vbo(new_vbo);

	for (int widthItr = 0; widthItr < size.x; widthItr++)
	{
		for (int heightItr = 0; heightItr < size.y; heightItr++)
		{
			t_tile current_tile = tile_map[widthItr][heightItr];
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

			GLuint* texture_set;

			float x_offset = 0;
			float y_offset = 0;

			if (current_tile.type == 0 || current_tile.type == 1)
			{
				x_offset = 0;
				y_offset = 0;
			}
			else if (current_tile.type == 2)
			{
				x_offset = 1;
				y_offset = 0;
			}
			else if (current_tile.type == 3)
			{
				x_offset = 0;
				y_offset = 1;
			}
			else if (current_tile.type == 4)
			{
				x_offset = 1;
				y_offset = 1;
			}

			int vertex_offset = (widthItr * size.x * 18) + (heightItr * 18);
			int texcoord_offset = (widthItr * size.x * 12) + (heightItr * 12);

			verticies[vertex_offset + 0] = widthItr + 0.5f;
			verticies[vertex_offset + 1] = -heightItr + 0.5f;
			verticies[vertex_offset + 2] = 0.0f;
			texcoords[texcoord_offset + 0] = (0.5 * x_offset) + 0.125f + (0.125f * xcoord);
			texcoords[texcoord_offset + 1] = (0.5 * y_offset) + 0.0f + (0.125f * ycoord);
			colors[vertex_offset + 0] = 1.0f;
			colors[vertex_offset + 1] = 1.0f;
			colors[vertex_offset + 2] = 1.0f;

			verticies[vertex_offset + 3] = widthItr - 0.5f;
			verticies[vertex_offset + 4] = -heightItr + 0.5f;
			verticies[vertex_offset + 5] = 0.0f;
			texcoords[texcoord_offset + 2] = (0.5 * x_offset) + 0.0f + (0.125f * xcoord);
			texcoords[texcoord_offset + 3] = (0.5 * y_offset) + 0.0f + (0.125f * ycoord);
			colors[vertex_offset + 3] = 1.0f;
			colors[vertex_offset + 4] = 1.0f;
			colors[vertex_offset + 5] = 1.0f;

			verticies[vertex_offset + 6] = widthItr + -0.5f;
			verticies[vertex_offset + 7] = -heightItr - 0.5f;
			verticies[vertex_offset + 8] = 0.0f;
			texcoords[texcoord_offset + 4] = (0.5 * x_offset) + 0.0f + (0.125f * xcoord);
			texcoords[texcoord_offset + 5] = (0.5 * y_offset) + 0.125f + (0.125f * ycoord);
			colors[vertex_offset + 6] = 1.0f;
			colors[vertex_offset + 7] = 1.0f;
			colors[vertex_offset + 8] = 1.0f;

			/*********************************************************************/

			verticies[vertex_offset + 9] = widthItr + 0.5f;
			verticies[vertex_offset + 10] = -heightItr + 0.5f;
			verticies[vertex_offset + 11] = 0.0f;
			texcoords[texcoord_offset + 6] = (0.5 * x_offset) + 0.125f + (0.125f * xcoord);
			texcoords[texcoord_offset + 7] = (0.5 * y_offset) + 0.0f + (0.125f * ycoord);
			colors[vertex_offset + 9] = 1.0f;
			colors[vertex_offset + 10] = 1.0f;
			colors[vertex_offset + 11] = 1.0f;

			verticies[vertex_offset + 12] = widthItr + -0.5f;
			verticies[vertex_offset + 13] = -heightItr - 0.5f;
			verticies[vertex_offset + 14] = 0.0f;
			texcoords[texcoord_offset + 8] = (0.5 * x_offset) + 0.0f + (0.125f * xcoord);
			texcoords[texcoord_offset + 9] = (0.5 * y_offset) + 0.125f + (0.125f * ycoord);
			colors[vertex_offset + 12] = 1.0f;
			colors[vertex_offset + 13] = 1.0f;
			colors[vertex_offset + 14] = 1.0f;

			verticies[vertex_offset + 15] = widthItr + 0.5f;
			verticies[vertex_offset + 16] = -heightItr - 0.5f;
			verticies[vertex_offset + 17] = 0.0f;
			texcoords[texcoord_offset + 10] = (0.5 * x_offset) + 0.125f + (0.125f * xcoord);
			texcoords[texcoord_offset + 11] = (0.5 * y_offset) + 0.125f + (0.125f * ycoord);
			colors[vertex_offset + 15] = 1.0f;
			colors[vertex_offset + 16] = 1.0f;
			colors[vertex_offset + 17] = 1.0f;
		}
	}

	new_vbo.texture = tile_atlas[0];

	PaintBrush::bind_vbo(new_vbo);
}

void GridManager::draw_autotile()
{
	if (GridManager::tile_map_dirty)
	{
		GridManager::tile_map_dirty = false;
		generate_autotile_vbo();
	}

	PaintBrush::draw_vbo(new_vbo);
}

void GridManager::reset_visibility()
{
	for (int widthItr = 0; widthItr < size.x; widthItr++)
	{
		for (int heightItr = 0; heightItr < size.y; heightItr++)
		{
			tile_map[widthItr][heightItr].visible = false;
		}
	}
}

void GridManager::compute_visibility_raycast(int i, int j, bool discover)
{
	bool found;
	int widthItr, heightItr;

	for (widthItr = 0; widthItr < size.x; widthItr++)
	{
		for (heightItr = 0; heightItr < size.y; heightItr++)
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