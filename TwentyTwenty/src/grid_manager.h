#pragma once

#include "paintbrush.h"
#include "game_entity.h"
#include <map>
#include <algorithm>
#include "json.hpp"
#include <iomanip>
#include <lua/lua.hpp>
#include <thread>

class FOWPlayer;

typedef struct
{
	int x, y;
} t_coords;

typedef enum
{
	TILE_DIRT,
	TILE_GRASS,
	TILE_WATER,
	TILE_ROCKS,
	TILE_TREES
}tiletype_t;

typedef struct
{
	int x;
	int y;
	int wall;

	tiletype_t type;
	
	std::vector<std::pair<int, int>> visible_tiles;
	bool visible;
	bool discovered;

	// for pathfinding
	float gscore, fscore;
	t_coords cameFrom;
	bool in_path;
	int tex_wall;
	GameEntity *entity_on_position;
	std::vector<GameEntity*> decorations;

} t_tile;

typedef struct t_raycast
{
	int dx, dy, D;
	int x, y;
	int x1, y1;
	int octant;
	int xStep, yStep;
	int error;

	void init(int x0, int y0, int x1, int y1)
	{
		dx = abs(x1 - x0);
		dy = abs(y1 - y0);
		xStep = (x1 >= x0) ? 1 : -1;
		yStep = (y1 >= y0) ? 1 : -1;

		error = dx - dy;

		D = dy - dx;
		x = x0;
		y = y0;
		this->x1 = x1;
		this->y1 = y1;
	}

	t_vertex get_point()
	{
		return t_vertex(x, y, 0);
	}

	bool has_next()
	{
		return !(x == x1 && y == y1);
	}

	void next()
	{
		int twoError = 2 * error;

		if (twoError > (-1 * dy)) {
			error -= dy;
			x += xStep;
		}
		if (twoError < dx) {
			error += dx;
			y += yStep;
		}
	}

};

class GridManager
{
public:
	// normal stuff
	static void init(std::string mapname);
	static void save_map(const std::string& mapname);
	static void load_map(const std::string& mapname);
	static GameEntity* create_entity(const entity_types& type, const t_vertex& position);	
	static GameEntity* build_and_add_entity(const entity_types& type, const t_vertex& position);
	static std::vector<GameEntity*> get_entities_of_type(entity_types type, int team_id = -1);
	void randomize_map();

	// Autotile stuff
	static void draw_autotile();
	static int calculate_tile(int i, int j, tiletype_t current_type);
	static int include_perimeter(int i, int j); // this is just to split some code out and keep calculate_tile pretty...
	static bool check_compatible(int i, int j, tiletype_t current_type);
	static void dropblob(int i, int j, tiletype_t blobtype);
	static void cull_orphans();
	static void calc_all_tiles();
	static void mow(int x, int y);
	static bool tile_map_dirty;

	// autotile VBO stuff
	static t_VBO new_vbo;
	static void generate_autotile_vbo();	// initial generation
	static void update_autotile_vbo();
	static void bind_autotile_vbo();

	// Pathfinding and grid utility
	static void compute_visibility_raycast(int i, int j, bool discover);// currently unused
	static bool position_visible(const t_vertex& check_position);		// currently unused
	static void reset_visibility();										// currently unused
	static bool point_can_be_seen(int i, int j, int i2, int j2);		// currently unused
	static 	GameEntity* entity_on_position(t_vertex entity_pos);
	static bool space_free(const t_vertex& position, const int& size);
	static std::vector<t_tile> get_adjacent_tiles_from_position(t_vertex position, bool position_empty, bool dont_check_passable);
	static t_tile *get_tile(int x, int y);

	// pathfinding stuff
	static std::vector<t_tile*> find_path(t_vertex start_pos, t_vertex end_pos, bool use_teams = false, int team = 0);
	static void clear_path();
	// should this be local to find path?
	static t_tile *last_path;

	// API for LUA
	static int howdy(lua_State* state);
	static int build_and_add_entity(lua_State* state);

	// other variables
	static t_vertex size;
	static std::map<int, std::map<int, t_tile>> tile_map;
	static std::vector<GLuint> tile_atlas;
	static float game_speed;
};