#pragma once

#include "paintbrush.h"
#include "game_entity.h"
#include <map>
#include <algorithm>
#include "json.hpp"
#include <iomanip>

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
	void init();
	void save_map(const std::string& mapname);
	void load_map(const std::string& mapname);
	static GameEntity* create_entity(const entity_types& type, const t_vertex& position);	// this one is static
	GameEntity* build_and_add_entity(const entity_types& type, const t_vertex& position); // this one is not
	void randomize_map();

	// Autotile stuff
	void draw_autotile();
	int calculate_tile(int i, int j, tiletype_t current_type);
	int include_perimeter(int i, int j); // this is just to split some code out and keep calculate_tile pretty...
	void calc_all_tiles();
	bool check_compatible(int i, int j, tiletype_t current_type);
	void dropblob(int i, int j, tiletype_t blobtype);
	void cull_orphans();

	// Pathfinding and grid utility
	void compute_visibility_raycast(int i, int j, bool discover);
	bool position_visible(int x, int z);
	void reset_visibility();
	bool point_can_be_seen(int i, int j, int i2, int j2);
	void set_mouse_coords(t_transform mouse_position);
	GameEntity* entity_on_position(t_vertex entity_pos);
	void draw_path(const t_vertex& start_pos);
	int num_path(const t_vertex& start_pos);
	bool space_free(const t_vertex& position, const int& size);

	t_vertex convert_mouse_coords(t_vertex mouse_space);
	t_transform mouse_coordinates()
	{
		t_transform return_value(mouse_x, mouse_y, 0, 0);
		return return_value;
	}
	std::vector<GameEntity*> get_entities_of_type(entity_types type);

	// pathfinding stuff
	std::vector<t_tile*> find_path(t_vertex start_pos, t_vertex end_pos, bool use_teams = false, int team = 0);
	std::vector<GameEntity*> *entities;
	void clear_path();
	t_tile *last_path;

	// other stuff - some of this needs to be culled
	int width, height;
	float game_speed;
	int x, y;
	std::map<int, std::map<int, t_tile>> tile_map;
	int mouse_x, mouse_y;
	t_transform real_mouse_position;
	bool lookmode;
	bool good_spot;
	bool use_tex;
	GLuint fake_tex[4];
	GLuint real_tex[4];
	static FOWPlayer* player;

};