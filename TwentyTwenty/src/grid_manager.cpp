#include <sstream>
#include <fstream>
#include "grid_manager.h"
#include "gatherer.h"
#include "knight.h"
#include "undead.h"
#include "fow_player.h"
#include "fow_building.h"


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

float heuristic_cost_estimate(t_tile *a, t_tile *b)
{
	return (abs(b->x - a->x) + abs(b->y - a->y));
}

bool are_equal(t_tile *a, t_tile *b)
{
	return ((a->x == b->x) && (a->y == b->y));
}

bool in_set(std::vector<t_tile*> set, t_tile *vertex)
{
	int i;
	for (i = 0; i < set.size(); i++)
	{
		if (are_equal(set.at(i), vertex))
			return true;
	}
	return false;
}

void GridManager::draw_path(t_vertex start_pos)
{
	std::vector<t_tile*> test = find_path(start_pos, t_vertex(mouse_x, 0, mouse_y));

	int i;
	if (test.size() > 0)
	{
		for (i = 1; i < test.size(); i++)
		{
			test[i]->in_path = true;
		}
	}
}

int GridManager::num_path(t_vertex start_pos)
{
	int b = 0;
	std::vector<t_tile*> test = find_path(start_pos, t_vertex(mouse_x, 0, mouse_y));

	int i;
	for (i = 0; i < test.size(); i++)
	{
		b++;
		test[i]->in_path = true;
	}
	return b;
}

bool GridManager::position_visible(int x, int z)
{
	return tile_map[x][z].visible;
}

void GridManager::save_map(std::string mapname)
{

	nlohmann::json j =
	{
		{"name", "test"},
		{"width", width},
		{"height", height},
		{"tiles", nlohmann::json({}) },
	};

	int i = 0;
	int k = 0;

	std::vector<GameEntity*> used_entities;

	for(i=0; i<width; i++)
		for (k = 0; k < height; k++)
		{
			t_tile* current_tile = &tile_map[i][k];

			j["tiles"][std::to_string(i)][std::to_string(k)] = nlohmann::json({ {"type", current_tile->type},
														{"x", current_tile->x},
														{"y", current_tile->y},
														{"entities", {}} });

			if (current_tile->entity_on_position != nullptr && std::find(used_entities.begin(), used_entities.end(), current_tile->entity_on_position) == used_entities.end())
			{
				GameEntity* current_entity = current_tile->entity_on_position;
				used_entities.push_back(current_entity);
				j["tiles"][std::to_string(i)][std::to_string(k)]["entities"] = nlohmann::json({ {"type", current_entity->type},
														{"x", current_entity->position.x},
														{"y", current_entity->position.y} });
			}
		}

	std::ofstream o("data/pretty.json");
	o << std::setw(4) << j << std::endl;
}

void from_json(const nlohmann::json& j, std::map<int, std::map<int, t_tile>>& new_tile_map)
{
	int width, height;

	j.at("width").get_to(width);
	j.at("height").get_to(height);

	printf("%d %d\n", width, height);

	nlohmann::json tile_data = j.at("tiles");

	int i, k;
	for (i = 0; i < width; i++)
		for (k = 0; k < height; k++)
		{
			new_tile_map[i][k] = t_tile();
			tile_data.at(std::to_string(i)).at(std::to_string(k)).at("type").get_to(new_tile_map[i][k].type);

			new_tile_map[i][k].x = i;
			new_tile_map[i][k].y = k;
			new_tile_map[i][k].gscore = INFINITY;
			new_tile_map[i][k].fscore = INFINITY;

			if (new_tile_map[i][k].type > 1)
				new_tile_map[i][k].wall = 1;

			if (tile_data.at(std::to_string(i)).at(std::to_string(k)).at("entities").is_null() == false)
			{
				nlohmann::json entity_data;
				tile_data.at(std::to_string(i)).at(std::to_string(k)).at("entities").get_to(entity_data);
				int type;
				entity_data.at("type").get_to(type);
				GameEntity* new_entity = GridManager::create_entity((entity_types)type, t_vertex(i, k, 0));
				if (new_entity != nullptr)
					new_tile_map[i][k].entity_on_position = new_entity;
			}
			else
				new_tile_map[i][k].entity_on_position = nullptr;
		}

	FOWKnight* knight = new FOWKnight(t_vertex(20, 20, 0));
	new_tile_map[20][20].entity_on_position = knight;

}

GameEntity* GridManager::create_entity(entity_types type, t_vertex position)
{
	if (type == FOW_GATHERER)
	{
		FOWGatherer* new_character;
		new_character = new FOWGatherer(t_vertex(position.x, position.y, 0));
		new_character->owner = player;
		return new_character;
	}

	if (type == FOW_TOWNHALL)
	{
		FOWBuilding* new_building = new FOWTownHall(position.x, position.y, 3);
		return new_building;
	}

	if (type == FOW_GOLDMINE)
	{
		FOWBuilding* new_building = new FOWGoldMine(position.x, position.y, 3);
		return new_building;
	}

	return nullptr;
}

void GridManager::load_map(std::string mapname)
{
	nlohmann::json level_data;
	std::ifstream i(mapname);
	i >> level_data;

	// import settings
	tile_map = level_data.get<std::map<int, std::map<int, t_tile>>>();

	width = tile_map.size();
	height = tile_map[0].size();

	int p, k;
	for (p = 0; p < width; p++)
		for (k = 0; k < height; k++)
			if (tile_map[p][k].entity_on_position != nullptr)
			{
				entities->push_back(tile_map[p][k].entity_on_position);
				((FOWSelectable*)tile_map[p][k].entity_on_position)->dirty_tile_map();
			}

	printf("Level dimensions: %d x %d\n", width, height);
}

void GridManager::init()
{
	load_map("data/pretty.json");
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
	mouse_x = int(real_mouse_position.x+0.5);
	mouse_y = int(-real_mouse_position.y+0.5);
	real_mouse_position = mouse_position;

	if (mouse_x < 0)
		mouse_x = 0;
	if (mouse_x > width)
		mouse_x = width;

	if (mouse_y < 0)
		mouse_y = 0;
	if (mouse_y > height)
		mouse_y = height;
}

// this is dead code
t_vertex GridManager::convert_mouse_coords(t_vertex mouse_space)
{
	return mouse_space;
}

void GridManager::clear_path()
{
	int i2, j2;
	for(i2=0; i2<width; i2++)
		for (j2 = 0; j2 < height; j2++)
		{
			tile_map[i2][j2].gscore = INFINITY;
			tile_map[i2][j2].fscore = INFINITY;
			tile_map[i2][j2].cameFrom.x = 0;
			tile_map[i2][j2].cameFrom.y = 0;
			tile_map[i2][j2].in_path = false;
		}
}

std::vector<t_tile*> GridManager::find_path(t_vertex start_pos, t_vertex end_pos)
{
	t_tile *start = &tile_map[start_pos.x][start_pos.y];
	t_tile *goal = &tile_map[end_pos.x][end_pos.y];

	clear_path();

	std::vector<t_tile*> return_vector;

	if (are_equal(start, goal))
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

	t_tile *current = start;
	t_tile *neighbour;

	current->gscore = 0;
	current->fscore = heuristic_cost_estimate(start, goal);

	while (openSet.size() > 0)
	{
		float current_fscore = INFINITY;
		for (i = 0; i < openSet.size(); i++)
			if (openSet.at(i)->fscore < current_fscore)
			{
				current = openSet.at(i);
				current_fscore = current->fscore;
			}

		if (are_equal(current, goal))
		{
			// success
			while (current != start)
			{
				return_vector.push_back(current);
				// this made the path yellow
				// which is cool, but shouldn't be done here..
				//current->in_path = true;
				current = &tile_map[current->cameFrom.x][current->cameFrom.y];
			}
			return return_vector;
		}

		for (i = 0; i < openSet.size(); i++)
		{
			if (are_equal(current, openSet.at(i)))
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

			if ((new_x >= 0 && new_x < width && new_y >= 0 && new_y < height) && tile_map[new_x][new_y].wall == 0 && tile_map[new_x][new_y].entity_on_position == nullptr && valid)
			{
				neighbour = &tile_map[new_x][new_y];
			}
			else
				continue;

			if (in_set(closedSet, neighbour))
				continue;		// Ignore the neighbor which is already evaluated. 

			float tentative_gScore;
			tentative_gScore = current->gscore + 1;

			if (!in_set(openSet, neighbour))	// Discover a new node
				openSet.push_back(neighbour);
			else if (tentative_gScore >= neighbour->gscore)
				continue;		// This is not a better path.

			// This path is the best until now. Record it!
			neighbour->cameFrom.x = current->x;
			neighbour->cameFrom.y = current->y;
			neighbour->gscore = tentative_gScore;
			neighbour->fscore = neighbour->gscore + heuristic_cost_estimate(neighbour, goal);

		}
	}

	return return_vector;
}

void GridManager::dropblob(int i, int j, int blobtype)
{
	int wall = 0;
	if (blobtype == 2 || blobtype == 3)
		wall = 1;

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

	for (int i = 1; i < width - 2; i++)
	{
		for (int j = 1; j < height - 2; j++)
		{
			tile_map[i][j].type = 0;
			tile_map[i][j].wall = 0;
		}
	}

	int new_type = 1;
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

	new_type = 2;
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

	new_type = 3;
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

	/*
	new_type = 4;
	for (int i = 2; i < width - 4; i++)
	{
		for (int j = 2; j < height - 4; j++)
		{
			if (rand() % 2 == 0)
			{
				dropblob(i, j, new_type);
			}
		}
	}*/

	cull_orphans();
	calc_all_tiles();
}


std::vector<GameEntity*> GridManager::get_entities_of_type(entity_types type)
{
	std::vector<GameEntity*> return_list;
	int i;
	for (i = 0; i < entities->size(); i++)
		if (entities->at(i)->type == type)
			return_list.push_back(entities->at(i));

	return return_list;
}


bool GridManager::space_free(t_vertex position, int size)
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
			int current_type = tile_map[i][j].type;
			
			if (current_type != 0)
			{
				if (check_compatible(i, j-1, current_type))
				{
					if (check_compatible(i-1, j, current_type))
						if (check_compatible(i-1, j - 1, current_type))
							found = true;

					if (tile_map[i + 1][j].type == current_type)
						if (tile_map[i + 1][j - 1].type == current_type)
							found = true;
				}

				if (check_compatible(i, j + 1, current_type))
				{
					if (check_compatible(i-1, j, current_type))
						if (check_compatible(i - 1, j+1, current_type))
							found = true;

					if (check_compatible(i + 1, j, current_type))
						if (check_compatible(i + 1, j + 1, current_type))
							found = true;
				}

				if (found == false)
					tile_map[i][j].type = 0;
			}
		}
	}

}

bool GridManager::check_compatible(int i, int j, int current_type)
{
	if (current_type == 1)
		if (tile_map[i][j].type == 1 || tile_map[i][j].type == 4)
			return true;

	return tile_map[i][j].type == current_type;
}


int GridManager::calculate_tile(int i, int j, int current_type)
{

	int tex_wall = 0;

	if (i == 0 || i == width - 1 || j == 0 || j == height - 1)
	{
		tex_wall = 15;
	}
	else
	{
		if (check_compatible(i-1,j-1,current_type))
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
	}

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
			if (tile_map[i][j].tex_wall == -1)
				tile_map[i][j].tex_wall = 15;

			if (tile_map[i][j].type == 0)
				tile_map[i][j].tex_wall = 15;

			int xcoord = tile_map[i][j].tex_wall % 4;
			int ycoord = tile_map[i][j].tex_wall / 4;

			GLuint *texture_set;

			if (use_tex)
				texture_set = fake_tex;
			else
				texture_set = real_tex;

			glEnable(GL_DEPTH_TEST);

			glPushMatrix();
				glTranslatef(i, -j, 0.0f);

				if(tile_map[i][j].type == 0 || tile_map[i][j].type == 1)
					glBindTexture(GL_TEXTURE_2D, texture_set[0]);
				else if(tile_map[i][j].type == 2)
					glBindTexture(GL_TEXTURE_2D, texture_set[1]);
				else if (tile_map[i][j].type == 3)
					glBindTexture(GL_TEXTURE_2D, texture_set[2]);
				else if (tile_map[i][j].type == 4)
					glBindTexture(GL_TEXTURE_2D, texture_set[3]);

				if (tile_map[i][j].in_path)
					glColor3f(1.0f, 0.0f, 1.0f);
				else
					glColor3f(1.0f, 1.0f, 1.0f);

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
	for (int i2 = 0; i2 < width; i2++)
	{
		for (int j2 = 0; j2 < height; j2++)
		{
			tile_map[i2][j2].visible = false;
		}
	}
}

void GridManager::compute_visibility_raycast(int i, int j, bool discover)
{
	bool found;
	int i2, j2;

	// i and j are the current position
	// i2 and j2 are iterators.
	// for the current position cast a ray from the current position
	// to a position on the perimeter. 
	for (i2 = 0; i2 < width; i2++)
	{
		for (j2 = 0; j2 < height; j2++)
		{
			if(!tile_map[i2][j2].visible)
				tile_map[i2][j2].visible = point_can_be_seen(i,j,i2,j2);

			if (tile_map[i2][j2].visible && discover)
				tile_map[i2][j2].discovered = true;
		}
	}

}

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
			return false;

		vision_cast.next();
	}

	return true;
}