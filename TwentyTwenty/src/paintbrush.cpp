
#include "paintbrush.h"

// stuff for VBOs...
PFNGLGENBUFFERSARBPROC      glGenBuffersARB = NULL;
PFNGLBUFFERDATAARBPROC      glBufferDataARB = NULL;
PFNGLBINDBUFFERARBPROC      glBindBufferARB = NULL;
std::map<std::string, GLuint> PaintBrush::texture_db = {};

void PaintBrush::init()
{
	setup_extensions();
}

void PaintBrush::setup_extensions()
{
	char* extensionList = (char*)glGetString(GL_EXTENSIONS);

	glGenBuffersARB = (PFNGLGENBUFFERSARBPROC)
		uglGetProcAddress("glGenBuffersARB");
	glBufferDataARB = (PFNGLBUFFERDATAARBPROC)
		uglGetProcAddress("glBufferDataARB");
	glBindBufferARB = (PFNGLBINDBUFFERARBPROC)
		uglGetProcAddress("glBindBufferARB");
}

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


// Pass in a vbo with vertex data
// output the respective buffers
void PaintBrush::build_vbo(t_VBO *the_vbo)
{
	glGenBuffersARB(1, &the_vbo->vertex_buffer);
	glGenBuffersARB(1, &the_vbo->texcoord_buffer);
	glGenBuffersARB(1, &the_vbo->color_buffer); 
}

void PaintBrush::draw_vbo(t_VBO *the_vbo)
{
	glEnableClientState(GL_VERTEX_ARRAY);
	//glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindBufferARB(GL_ARRAY_BUFFER, the_vbo->vertex_buffer);
	glBufferDataARB(GL_ARRAY_BUFFER, sizeof(float) * the_vbo->num_faces * 3, the_vbo->verticies, GL_STATIC_DRAW);

	glBindBufferARB(GL_ARRAY_BUFFER, the_vbo->color_buffer);
	glBufferDataARB(GL_ARRAY_BUFFER, sizeof(float) * the_vbo->num_faces * 3, the_vbo->colors, GL_STATIC_DRAW);

	glBindBufferARB(GL_ARRAY_BUFFER, the_vbo->texcoord_buffer);
	glBufferDataARB(GL_ARRAY_BUFFER, sizeof(float) * the_vbo->num_faces * 2, the_vbo->texcoords, GL_STATIC_DRAW);

	// bind the vbo
	//Make the new VBO active. Repeat here incase changed since initialisation
	glBindBufferARB(GL_ARRAY_BUFFER, the_vbo->vertex_buffer);
	glVertexPointer(3, GL_FLOAT, 0, 0);
	glBindBufferARB(GL_ARRAY_BUFFER, the_vbo->color_buffer);
	glColorPointer(3, GL_FLOAT, 0, 0);
	glBindBufferARB(GL_ARRAY_BUFFER, the_vbo->texcoord_buffer);
	glTexCoordPointer(2, GL_FLOAT, 0, 0);
	&the_vbo;
	glPushMatrix();
		glBindTexture(GL_TEXTURE_2D, the_vbo->texture);
		glDrawArrays(GL_TRIANGLES, 0, the_vbo->num_faces);
	glPopMatrix();

	glDisableClientState(GL_VERTEX_ARRAY);
//	glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
 	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
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

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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