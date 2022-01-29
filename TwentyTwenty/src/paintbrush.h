#pragma once

#include <SDL.h>
#include <SDL_opengl.h>
#include "SOIL.h"
#include <string>
#include <map>
#include <gl/GLU.h>
#include <gl/gl.h>     // The GL Header File
#include <sdl_ttf.h>
#include <vector>
#include <memory>

// shader stuff
#define uglGetProcAddress(x) wglGetProcAddress(x)
#define WIN32_OR_X11

class t_vertex;

typedef struct
{
	int width;
	int height;
	GLuint texture;

}t_texturechar;

typedef struct
{
	GLuint vertex_buffer;
	GLuint texcoord_buffer;
	GLuint color_buffer;

	// If you can and the API lets you, std::unique_ptr or std::shared_ptr
	// will cut down on your cleanup code
	std::shared_ptr<float[]> verticies;
	std::shared_ptr<float[]> texcoords;
	std::shared_ptr<float[]> colors;

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


	// initialization
	static void setup_extensions();

	// Vertex Buffer Objects
	static void generate_vbo(t_VBO& the_vbo);
	static void bind_vbo(t_VBO& the_vbo);
	static void draw_quad();
	static void draw_vbo(t_VBO the_vbo);
	static void draw_quad_vbo(t_VBO the_vbo);

	// Text and Font, SDL_TTF
	static t_texturechar TextToTexture(GLubyte r, GLubyte g, GLubyte b, const char* text);
	static void draw_string(t_vertex position, t_vertex scale, std::string text);

	// Texture loading
	static GLuint Soil_Load_Texture(const std::string& filename);
	static GLuint Soil_Load_Texture(const std::string& filename, const e_texture_clampmode& mode);
	static GLuint get_texture(const std::string& texture_id);
	static GLuint get_texture(const std::string& texture_id, const e_texture_clampmode& mode);


	/******** NEW SHADER STUFF *********/
	GLenum get_shader(std::string shader_id);
	GLint get_uniform(GLenum shader, std::string uniform_name);
	GLenum load_shader(std::string shadername);
	void use_shader(GLenum shader);
	void stop_shader();
	GLint get_uniform_location(GLenum shader, std::string variable_name);
	void set_uniform_location(GLenum shader, GLint uniform_location, float data);
	void set_uniform(GLenum shader, std::string uniform_name, float data);

	// variables
	static std::map<std::string, GLuint> texture_db;
	static std::string supported_characters;
	static std::map<char, t_texturechar> char_texture;
	static TTF_Font* font;
	static std::map<std::string, GLenum> shader_db;
	static std::map<std::pair<GLenum, std::string>, GLint> uniform_db;

};
