
#include "level.h"
#include "game_entity.h"
#include "game.h"
#include "paintbrush.h"

void from_json(const nlohmann::json& j, Level& l) {
	j.at("name").get_to(l.name);
	nlohmann::json entities_data = j.at("entities");
	for (nlohmann::json::iterator it = entities_data.begin(); it != entities_data.end(); ++it) {
		GameEntity* new_entity = new GameEntity();
		it.value().at("x").get_to(new_entity->transform.x);
		it.value().at("y").get_to(new_entity->transform.y);
		it.value().at("width").get_to(new_entity->transform.w);
		it.value().at("height").get_to(new_entity->transform.h);
		it.value().at("tex_x").get_to(new_entity->texture_coordinates.x);
		it.value().at("tex_y").get_to(new_entity->texture_coordinates.y);
		it.value().at("tex_width").get_to(new_entity->texture_coordinates.w);
		it.value().at("tex_height").get_to(new_entity->texture_coordinates.h);
		it.value().at("collision_enabled").get_to(new_entity->collision_enabled);
		it.value().at("layer").get_to(new_entity->layer);
		it.value().at("RGBA").at("R").get_to(new_entity->r);
		it.value().at("RGBA").at("G").get_to(new_entity->g);
		it.value().at("RGBA").at("B").get_to(new_entity->b);
		it.value().at("RGBA").at("A").get_to(new_entity->a);
		std::string texture_name;
		it.value().at("texture").get_to(texture_name);
		new_entity->texture = PaintBrush::get_texture(std::string("data/").append(texture_name), TEXTURE_REPEAT);
		Game::entities.push_back(new_entity);
	}
}

bool sort_layers(Entity *i, Entity *j) { return (((GameEntity*)i)->layer < ((GameEntity*)j)->layer); }