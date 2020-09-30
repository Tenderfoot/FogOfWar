
#include "paintbrush.h"

std::map<std::string, GLuint> PaintBrush::texture_db = {};

GLuint PaintBrush::Soil_Load_Texture(std::string filename)
{
	GLuint loaded_texture;
	int flags;

	flags = SOIL_FLAG_MIPMAPS;

	loaded_texture = SOIL_load_OGL_texture(filename.c_str(),
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		flags);

	// Make sure texture is set to repeat on wrap
	glBindTexture(GL_TEXTURE_2D, loaded_texture);

	// make sure it doesn't wrap
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	return loaded_texture;
}


GLuint PaintBrush::Soil_Load_Texture(std::string filename, e_texture_clampmode mode)
{
	GLuint loaded_texture;
	int flags;

	flags = SOIL_FLAG_MIPMAPS;

	loaded_texture = SOIL_load_OGL_texture(filename.c_str(),
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		flags);

	// Make sure texture is set to repeat on wrap
	glBindTexture(GL_TEXTURE_2D, loaded_texture);

	// make sure it doesn't wrap
	if (mode == TEXTURE_CLAMP)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	return loaded_texture;
}

GLuint PaintBrush::get_texture(std::string texture_id)
{
	std::map<std::string, GLuint>::iterator it;

	it = texture_db.find(texture_id);

	if (it == texture_db.end())
		texture_db.insert({ texture_id, Soil_Load_Texture(texture_id) });

	return texture_db[texture_id];
}

GLuint PaintBrush::get_texture(std::string texture_id, e_texture_clampmode mode)
{
	std::map<std::string, GLuint>::iterator it;

	it = texture_db.find(texture_id);

	if (it == texture_db.end())
		texture_db.insert({ texture_id, Soil_Load_Texture(texture_id, mode) });

	return texture_db[texture_id];
}