#pragma once


#include <SDL.h>
#include <SDL_opengl.h>
#include "SOIL.h"
#include <string>
#include <map>
#include <gl/GLU.h>
#include <gl/gl.h>     // The GL Header File

// shader stuff
#define uglGetProcAddress(x) wglGetProcAddress(x)
#define WIN32_OR_X11

typedef struct
{
	GLuint vertex_buffer;
	GLuint texcoord_buffer;
	GLuint color_buffer;

	// If you can and the API lets you, std::unique_ptr or std::shared_ptr
	// will cut down on your cleanup code
	float* verticies;
	float* texcoords;
	float* colors;

	int num_faces;
	GLuint texture;

}t_VBO;

typedef enum
{
	TEXTURE_CLAMP,
	TEXTURE_REPEAT
}e_texture_clampmode;

class PaintBrush
{
public:

	static std::map<std::string, GLuint> texture_db;

	t_VBO test;
	static void generate_vbo(t_VBO& the_vbo);
	static void bind_vbo(t_VBO& the_vbo);
	static void draw_vbo(t_VBO the_vbo);
	static void setup_extensions();
	static GLuint Soil_Load_Texture(const std::string& filename);
	static GLuint Soil_Load_Texture(const std::string& filename, const e_texture_clampmode& mode);
	static GLuint get_texture(const std::string& texture_id);
	static GLuint get_texture(const std::string& texture_id, const e_texture_clampmode& mode);
};
