#include <sstream>
#include <fstream>
#include <SDL.h>
#include <SDL_opengl.h>
#include <gl/GLU.h>
#include <gl/gl.h>     // The GL Header File
#include "grid_manager.h"
#include "gatherer.h"
#include "knight.h"
#include "undead.h"
#include "fow_player.h"
#include "fow_building.h"
#include "game.h"
#include "fow_decoration.h"
#include "archer.h"
#include "fow_projectile.h"
#include "server_handler.h"
#include "fow_decoration_manager.h"
#include "script_manager.h"

t_vertex  GridManager::size;
std::map<int, std::map<int, t_tile>> GridManager::tile_map;
t_VBO GridManager::new_vbo;
std::vector<GLuint> GridManager::tile_atlas;
t_tile* GridManager::last_path;
float GridManager::game_speed;
bool GridManager::tile_map_dirty = false;
extern bool sort_by_y(GameEntity* i, GameEntity* j);
std::string script_name;

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
	j.at("script").get_to(script_name);

	printf("script name was: %s\n", script_name.c_str());

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
			else
			{
				new_tile_map[widthItr][heightItr].entity_on_position = nullptr;
			}
		}
	}
}

void GridManager::init(std::string mapname)
{
	load_map(std::string("data/maps/") + mapname);

	game_speed = 1;

	tile_atlas.push_back(PaintBrush::Soil_Load_Texture("data/images/autotile_textureatlas.png", TEXTURE_CLAMP));
	tile_atlas.push_back(PaintBrush::Soil_Load_Texture("data/images/autotile_textureatlas_marsh.png", TEXTURE_CLAMP));
	tile_atlas.push_back(PaintBrush::Soil_Load_Texture("data/images/autotile_textureatlas_sophie.png", TEXTURE_CLAMP));
	tile_atlas.push_back(PaintBrush::Soil_Load_Texture("data/images/autotile_textureatlas_snow.png", TEXTURE_CLAMP));
	tile_atlas.push_back(PaintBrush::Soil_Load_Texture("data/images/autotile_textureatlas_wasteland.png", TEXTURE_CLAMP));

	// this needs to happen after the texture is set now
	calc_all_tiles();
	// generate the VBO
	generate_autotile_vbo();

	// can this get removed?
	last_path = &tile_map[0][0];
}

t_tile* GridManager::get_tile(int x, int y)
{
	return &tile_map[x][y];
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
	if (type == FOW_ARCHER) {
		new_entity = new FOWArcher(position);
	}
	if (type == FOW_PROJECTILE) {
		new_entity = new FOWProjectile(position);
	}
	return new_entity;
}

GameEntity *GridManager::build_and_add_entity(const entity_types& type, const t_vertex& position)
{
	GameEntity* new_entity = create_entity(type, position);
	Game::entities.push_back(new_entity);
	return new_entity;
}


std::vector<GameEntity*> GridManager::get_entities_of_type(entity_types type, int team_id)
{
	std::vector<GameEntity*> to_return;

	for (auto entity : Game::entities)
	{
		if (entity->type == type)
		{
			if (team_id == -1 || ((FOWSelectable*)entity)->team_id == team_id)
			{
				to_return.push_back(entity);
			}
		}
	}

	return to_return;
}

void GridManager::save_map(const std::string& mapname)
{
	nlohmann::json j =
	{
		{"name", "test"},
		{"width", GridManager::size.x},
		{"height", GridManager::size.y},
		{"script", ""},
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

void GridManager::load_map(const std::string& mapname)
{
	/************ JSON Level Data ****************/
	nlohmann::json level_data;
	std::ifstream i(mapname);
	i >> level_data;
	tile_map = level_data.get<std::map<int, std::map<int, t_tile>>>();

	size.x = tile_map.size();
	size.y = tile_map[0].size();

	// this has to happen after all the tiles are generated
	for (auto entity : Game::entities)
	{
		((FOWSelectable*)entity)->dirty_tile_map();
	}

	/************ LUA SCRIPT STUFF ****************/
	if (script_name != "")
	{
		ScriptManager::load_script("data/maps/" + script_name);
	}
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


bool GridManager::space_free(const t_vertex& position, const int& size)
{
	for (int widthItr = 0; widthItr < size; widthItr++)
		for (int heightItr = 0; heightItr < size; heightItr++)
			if (tile_map[widthItr+position.x][heightItr +position.y].wall || tile_map[widthItr + position.x][heightItr + position.y].entity_on_position != nullptr)
				return false;

	return true;
}

std::vector<t_tile> GridManager::get_adjacent_tiles_from_position(t_vertex position, bool position_empty, bool dont_check_passable)
{
	std::vector<t_tile> adjacent_tiles;
	for (int widthItr = position.x - 1; widthItr < position.x + 2; widthItr++)
	{
		for (int heightItr = position.y - 1; heightItr < position.y + 2; heightItr++)
		{
			if ((widthItr == position.x - 1 || widthItr == position.x + 1 || heightItr == position.y - 1 || heightItr == position.y + 1) && (GridManager::tile_map[widthItr][heightItr].entity_on_position == nullptr || position_empty == false) && (GridManager::tile_map[widthItr][heightItr].wall == 0 || dont_check_passable))
			{
				adjacent_tiles.push_back(GridManager::tile_map[widthItr][heightItr]);
			}
		}
	}
	return adjacent_tiles;
}

void GridManager::mow(int x, int y)
{
	t_tile* current_tile = &GridManager::tile_map[x][y];
	for (auto decoration : current_tile->decorations)
	{
		((FOWDecoration*)decoration)->delete_decoration();
	}

	if (current_tile->type == TILE_TREES)
	{
		if (ServerHandler::initialized)
		{
			current_tile->type = TILE_GRASS;
			current_tile->wall = 0;
			ServerHandler::tiles_dirty = true;
		}
	}
}


bool GridManager::check_compatible(int i, int j, tiletype_t current_type)
{
	if ((current_type == TILE_GRASS || current_type == TILE_TREES) && (tile_map[i][j].type == TILE_TREES || tile_map[i][j].type == TILE_GRASS))
	{
		return true;
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

	if (ServerHandler::initialized)
	{
		ServerHandler::tiles_dirty = true;
	}

	GridManager::tile_map_dirty = true;
}

void GridManager::generate_autotile_vbo()
{
	new_vbo.num_faces = size.x * size.y * 6;	// two triangles I guess
	new_vbo.verticies = std::shared_ptr<float[]>(new float[new_vbo.num_faces * 3]);
	new_vbo.colors = std::shared_ptr<float[]>(new float[new_vbo.num_faces * 3]);
	new_vbo.texcoords = std::shared_ptr<float[]>(new float[new_vbo.num_faces * 2]);
	new_vbo.tiles = std::shared_ptr<float[]>(new float[new_vbo.num_faces]);

	PaintBrush::generate_vbo(new_vbo);
	update_autotile_vbo();
	bind_autotile_vbo();

	new_vbo.texture = tile_atlas[0];
	new_vbo.shader = PaintBrush::get_shader("tiles");
}

void set_values3f(float* number_array, int offset, t_vertex values)
{
	number_array[offset] = values.x;
	number_array[offset+1] = values.y;
	number_array[offset+2] = values.z;
}

void set_values2f(float* number_array, int offset, t_vertex values)
{
	number_array[offset] = values.x;
	number_array[offset + 1] = values.y;
}

void GridManager::update_autotile_vbo()
{
	// is this ok with shared_ptr?
	float* verticies = new_vbo.verticies.get();
	float* texcoords = new_vbo.texcoords.get();
	float* colors = new_vbo.colors.get();
	float* tiles = new_vbo.tiles.get();

	for (int widthItr = 0; widthItr < size.x; widthItr++)
	{
		for (int heightItr = 0; heightItr < size.y; heightItr++)
		{
			t_tile current_tile = tile_map[widthItr][heightItr];

			int tile_index = (widthItr * size.x * 6) + (heightItr * 6);
			// This is for water I think... knowing the tile type
			for (int tileTypeItr = 0; tileTypeItr < 6; tileTypeItr++) {
				tiles[tile_index + tileTypeItr] = (float)current_tile.type;
			}

			if (current_tile.tex_wall == -1)
			{
				current_tile.tex_wall = 15;
			}

			if (current_tile.type == 0)
			{
				current_tile.tex_wall = 15;
			}

			// This short circuts trees to just appear as grass
			if (current_tile.type == 4)
			{
				current_tile.tex_wall = 0;
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
				if (Game::game_state == PLAY_MODE)
				{
					// trees does the same as grass now, tree quadrant is ignored
					x_offset = 0;
					y_offset = 0;
				}
				if (Game::game_state == EDIT_MODE)
				{
					x_offset = 1;
					y_offset = 1;

					// if we're trees, and we're going to draw that green square?
					// draw grass instead
					if (current_tile.tex_wall == 15)
					{
						x_offset = 0;
						y_offset = 0;
						current_tile.tex_wall = 0;
						xcoord = current_tile.tex_wall % 4;
						ycoord = current_tile.tex_wall / 4;
					}
				}
			}

			int vertex_offset = (widthItr * size.x * 18) + (heightItr * 18);
			int texcoord_offset = (widthItr * size.x * 12) + (heightItr * 12);
			set_values3f(verticies, vertex_offset, t_vertex(widthItr + 0.5f, -heightItr + 0.5f, 1.0f));
			set_values3f(colors, vertex_offset, t_vertex(1.0f, 0.0f, 1.0f));
			set_values2f(texcoords, texcoord_offset, t_vertex((0.5 * x_offset) + 0.125f + (0.125f * xcoord), (0.5 * y_offset) + (0.125f * ycoord), 1.0f));
			set_values3f(verticies, vertex_offset+3, t_vertex(widthItr - 0.5f, -heightItr + 0.5f, 1.0f));
			set_values3f(colors, vertex_offset+3, t_vertex(0.0f, 0.0f, 1.0f));
			set_values2f(texcoords, texcoord_offset+2, t_vertex((0.5 * x_offset) + (0.125f * xcoord), (0.5 * y_offset) + (0.125f * ycoord), 1.0f));
			set_values3f(verticies, vertex_offset + 6, t_vertex(widthItr - 0.5f, -heightItr - 0.5f, 1.0f));
			set_values3f(colors, vertex_offset+6, t_vertex(0.0f, 1.0f, 1.0f));
			set_values2f(texcoords, texcoord_offset + 4, t_vertex((0.5 * x_offset) + (0.125f * xcoord), (0.5 * y_offset) + 0.125f + (0.125f * ycoord), 1.0f));
			set_values3f(verticies, vertex_offset + 9, t_vertex(widthItr + 0.5f, -heightItr + 0.5f, 1.0f));
			set_values3f(colors, vertex_offset + 9, t_vertex(1.0f, 0.0f, 1.0f));
			set_values2f(texcoords, texcoord_offset + 6, t_vertex((0.5 * x_offset) + 0.125f + (0.125f * xcoord), (0.5 * y_offset) + (0.125f * ycoord), 1.0f));
			set_values3f(verticies, vertex_offset + 12, t_vertex(widthItr - 0.5f, -heightItr - 0.5f, 1.0f));
			set_values3f(colors, vertex_offset + 12, t_vertex(0.0f, 1.0f, 1.0f));
			set_values2f(texcoords, texcoord_offset + 8, t_vertex((0.5 * x_offset) + (0.125f * xcoord), (0.5 * y_offset) + 0.125f + (0.125f * ycoord), 1.0f));
			set_values3f(verticies, vertex_offset + 15, t_vertex(widthItr + 0.5f, -heightItr - 0.5f, 1.0f));
			set_values3f(colors, vertex_offset + 15, t_vertex(1.0f, 1.0f, 1.0f));
			set_values2f(texcoords, texcoord_offset + 10, t_vertex((0.5 * x_offset) + 0.125f + (0.125f * xcoord), (0.5 * y_offset) + 0.125f + (0.125f * ycoord), 1.0f));
		}
	}
}

void GridManager::bind_autotile_vbo()
{
	PaintBrush::bind_vbo(new_vbo);
	PaintBrush::bind_data(new_vbo);
}

void GridManager::draw_autotile()
{
	if (GridManager::tile_map_dirty)
	{
		GridManager::tile_map_dirty = false;
		update_autotile_vbo();
		bind_autotile_vbo();
	}

	GLuint texture = 0;
	if (tile_atlas.size() > 0)
	{
		texture = tile_atlas.at(0);
	}

	PaintBrush::transform_model_matrix(new_vbo.shader, glm::vec3(0), glm::vec4(0), glm::vec3(1));
	PaintBrush::draw_vao(new_vbo);
}
