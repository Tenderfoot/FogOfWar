#pragma once

#include <SDL_opengl.h>
#include "SOIL.h"
#include <string>
#include <map>

#define uglGetProcAddress(x) wglGetProcAddress(x)
#define WIN32_OR_X11

typedef enum
{
	TEXTURE_CLAMP,
	TEXTURE_REPEAT
}e_texture_clampmode;

typedef struct
{
	GLuint vertex_buffer;
	GLuint texcoord_buffer;
	GLuint color_buffer;

	float* verticies;
	float* texcoords;
	float* colors;

	int num_faces;
	GLuint texture;

}t_VBO;

class PaintBrush
{
public:

	static std::map<std::string, GLuint> texture_db;

	static void init();
	static void setup_extensions();
	static void draw_vbo(t_VBO *the_vbo);
	static void build_vbo(t_VBO *the_vbo);
	static GLuint Soil_Load_Texture(std::string filename);
	static GLuint Soil_Load_Texture(std::string filename, e_texture_clampmode mode);
	static GLuint get_texture(std::string texture_id);
	static GLuint get_texture(std::string texture_id, e_texture_clampmode mode);

};