
#include "level.h"
#include "game_entity.h"
#include "game.h"

void from_json(const nlohmann::json& j, Level& l) {
	j.at("name").get_to(l.name);
	nlohmann::json entities_data = j.at("entities");
	for (nlohmann::json::iterator it = entities_data.begin(); it != entities_data.end(); ++it) {
		GameEntity* new_entity = new GameEntity();
		it.value().at("x").get_to(new_entity->transform.x);
		it.value().at("y").get_to(new_entity->transform.y);
		it.value().at("width").get_to(new_entity->transform.w);
		it.value().at("height").get_to(new_entity->transform.h);
		it.value().at("collision_enabled").get_to(new_entity->collision_enabled);
		it.value().at("RGBA").at("R").get_to(new_entity->r);
		it.value().at("RGBA").at("G").get_to(new_entity->g);
		it.value().at("RGBA").at("B").get_to(new_entity->b);
		it.value().at("RGBA").at("A").get_to(new_entity->a);
		Game::entities.push_back(new_entity);
	}
}