#pragma once

#include <SDL_opengl.h>
#include "SOIL.h"
#include <string>
#include <map>

typedef enum
{
	TEXTURE_CLAMP,
	TEXTURE_REPEAT
}e_texture_clampmode;

class PaintBrush
{
public:

	static std::map<std::string, GLuint> texture_db;

	static GLuint Soil_Load_Texture(std::string filename);
	static GLuint Soil_Load_Texture(std::string filename, e_texture_clampmode mode);
	static GLuint get_texture(std::string texture_id);
	static GLuint get_texture(std::string texture_id, e_texture_clampmode mode);
};
