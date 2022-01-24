
#include "paintbrush.h"
#include "grid_manager.h"

std::map<std::string, GLuint> PaintBrush::texture_db = {};
TTF_Font* PaintBrush::font;
std::string PaintBrush::supported_characters;
std::map<char, t_texturechar> PaintBrush::char_texture;

// binding methods from extenions
PFNGLCREATEPROGRAMOBJECTARBPROC     glCreateProgramObjectARB = NULL;
PFNGLDELETEOBJECTARBPROC            glDeleteObjectARB = NULL;
PFNGLCREATESHADEROBJECTARBPROC      glCreateShaderObjectARB = NULL;
PFNGLSHADERSOURCEARBPROC            glShaderSourceARB = NULL;
PFNGLCOMPILESHADERARBPROC           glCompileShaderARB = NULL;
PFNGLGETOBJECTPARAMETERIVARBPROC    glGetObjectParameterivARB = NULL;
PFNGLATTACHOBJECTARBPROC            glAttachObjectARB = NULL;
PFNGLGETINFOLOGARBPROC              glGetInfoLogARB = NULL;
PFNGLLINKPROGRAMARBPROC             glLinkProgramARB = NULL;
PFNGLUSEPROGRAMOBJECTARBPROC        glUseProgramObjectARB = NULL;
PFNGLGETUNIFORMLOCATIONARBPROC      glGetUniformLocationARB = NULL;
PFNGLUNIFORM1FARBPROC               glUniform1fARB = NULL;
PFNGLUNIFORM1IARBPROC               glUniform1iARB = NULL;
PFNGLGETSHADERIVPROC                glGetShaderiv = NULL;
PFNGLGETSHADERINFOLOGPROC           glGetShaderInfoLog = NULL;
// stuff for VBOs...
PFNGLGENBUFFERSARBPROC      glGenBuffers = NULL;
PFNGLBUFFERDATAARBPROC      glBufferData = NULL;
PFNGLBINDBUFFERARBPROC      glBindBuffer = NULL;

void PaintBrush::setup_extensions()
{
	char* extensionList = (char*)glGetString(GL_EXTENSIONS);

	glCreateProgramObjectARB = (PFNGLCREATEPROGRAMOBJECTARBPROC)
		uglGetProcAddress("glCreateProgramObjectARB");
	glDeleteObjectARB = (PFNGLDELETEOBJECTARBPROC)
		uglGetProcAddress("glDeleteObjectARB");
	glCreateShaderObjectARB = (PFNGLCREATESHADEROBJECTARBPROC)
		uglGetProcAddress("glCreateShaderObjectARB");
	glShaderSourceARB = (PFNGLSHADERSOURCEARBPROC)
		uglGetProcAddress("glShaderSourceARB");
	glCompileShaderARB = (PFNGLCOMPILESHADERARBPROC)
		uglGetProcAddress("glCompileShaderARB");
	glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)
		uglGetProcAddress("glGetObjectParameterivARB");
	glAttachObjectARB = (PFNGLATTACHOBJECTARBPROC)
		uglGetProcAddress("glAttachObjectARB");

	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)
		uglGetProcAddress("glGetShaderInfoLog");

	glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)
		uglGetProcAddress("glGetInfoLogARB");
	glLinkProgramARB = (PFNGLLINKPROGRAMARBPROC)
		uglGetProcAddress("glLinkProgramARB");
	glUseProgramObjectARB = (PFNGLUSEPROGRAMOBJECTARBPROC)
		uglGetProcAddress("glUseProgramObjectARB");
	glGetUniformLocationARB = (PFNGLGETUNIFORMLOCATIONARBPROC)
		uglGetProcAddress("glGetUniformLocationARB");

	glGenBuffers = (PFNGLGENBUFFERSARBPROC)
		uglGetProcAddress("glGenBuffersARB");
	glBufferData = (PFNGLBUFFERDATAARBPROC)
		uglGetProcAddress("glBufferDataARB");
	glBindBuffer = (PFNGLBINDBUFFERARBPROC)
		uglGetProcAddress("glBindBufferARB");

	glUniform1fARB = (PFNGLUNIFORM1FARBPROC)
		uglGetProcAddress("glUniform1fARB");
	glUniform1iARB = (PFNGLUNIFORM1IARBPROC)
		uglGetProcAddress("glUniform1iARB");
	glGetShaderiv = (PFNGLGETSHADERIVPROC)
		uglGetProcAddress("glGetShaderiv");

	font = TTF_OpenFont("data/fonts/Greyscale Basic Regular.ttf", 32);
	if (!font)
	{
		printf("TTF_OpenFont: %s\n", TTF_GetError());
	}

	// TTF_RenderText_Blended needs a const char * - when I iterated through the string and passed in &char, it broke
	// showed weird extra stuff
	// so I'm using a string to both grab the character and the 1 character substring
	supported_characters = "aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ0123456789:. ";
	for (int charItr=0; charItr<supported_characters.size(); ++charItr)
	{
		char_texture[supported_characters.at(charItr)] = TextToTexture(255, 255, 255, supported_characters.substr(charItr, 1).c_str());
	}
}

void PaintBrush::draw_string(t_vertex position, t_vertex scale, std::string text)
{

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	float total_width = 0.0f;
	for (auto character_to_draw : text)
	{
		glPushMatrix();
		glEnable(GL_TEXTURE_2D);
		t_texturechar current_char = char_texture[character_to_draw];
		glBindTexture(GL_TEXTURE_2D, current_char.texture);
		glTranslatef(position.x, position.y, 0.0f);
		glBegin(GL_QUADS);
			glTexCoord2f(1.0f, 1.00f);	glVertex3f(total_width + current_char.width * scale.x, current_char.height * scale.y, 0.0f);
			glTexCoord2f(0.0f, 1.00f);	glVertex3f(total_width + 0.0f, current_char.height * scale.y, 0.0f);
			glTexCoord2f(0.0f, 0.0f);	glVertex3f(total_width + 0.0f, 0.0f, 0.0f);
			glTexCoord2f(1.0f, 0.0f);	glVertex3f(total_width + current_char.width * scale.x, 0.0f, 0.0f);
		glEnd();
		glPopMatrix();

		total_width += char_texture[character_to_draw].width*scale.x;
	}

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

}

void PaintBrush::generate_vbo(t_VBO& the_vbo)
{
	glGenBuffers(1, &the_vbo.vertex_buffer);
	glGenBuffers(1, &the_vbo.texcoord_buffer);
	glGenBuffers(1, &the_vbo.color_buffer);
}

void PaintBrush::bind_vbo(t_VBO& the_vbo)
{
	glBindBuffer(GL_ARRAY_BUFFER, the_vbo.vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * the_vbo.num_faces * 3, the_vbo.verticies.get(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, the_vbo.texcoord_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * the_vbo.num_faces * 2, the_vbo.texcoords.get(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, the_vbo.color_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * the_vbo.num_faces * 3, the_vbo.colors.get(), GL_STATIC_DRAW);
}

void PaintBrush::draw_vbo(t_VBO the_vbo)
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, the_vbo.vertex_buffer);
	glVertexPointer(3, GL_FLOAT, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, the_vbo.color_buffer);
	glColorPointer(3, GL_FLOAT, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, the_vbo.texcoord_buffer);
	glTexCoordPointer(2, GL_FLOAT, 0, 0);

	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.0f);
	glBindTexture(GL_TEXTURE_2D, the_vbo.texture);
	glDrawArrays(GL_TRIANGLES, 0, the_vbo.num_faces);
	glPopMatrix();

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

// Pass as const-reference
GLuint PaintBrush::Soil_Load_Texture(const std::string &filename)
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


// Pass as const-reference
GLuint PaintBrush::Soil_Load_Texture(const std::string& filename, const e_texture_clampmode& mode)
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

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	return loaded_texture;
}

t_texturechar PaintBrush::TextToTexture(GLubyte r, GLubyte g, GLubyte b, const char* text)
{
	t_texturechar new_character;

	SDL_Color color = { r, g, b };
	SDL_Surface* msg = TTF_RenderText_Blended(font, text, color);

	// create new texture, with default filtering state (==mipmapping on)
	GLuint font_texture;
	glGenTextures(1, &font_texture);
	glBindTexture(GL_TEXTURE_2D, font_texture);

	// disable mipmapping on the new texture
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// make sure it doesn't wrap
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// set data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, msg->w, msg->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, msg->pixels);

	new_character.width = msg->w;
	new_character.height = msg->h;
	new_character.texture = font_texture;

	SDL_FreeSurface(msg);
	return new_character;
}

GLuint PaintBrush::get_texture(const std::string& texture_id)
{
	auto texture_search = texture_db.find(texture_id);
	if (texture_search == texture_db.end())
	{
		texture_db.insert({ texture_id, Soil_Load_Texture(texture_id) });
	}
	return texture_db[texture_id];
}

// Pass as const-reference
GLuint PaintBrush::get_texture(const std::string& texture_id, const e_texture_clampmode& mode)
{
	auto texture_search = texture_db.find(texture_id);
	if (texture_search == texture_db.end())
	{
		texture_db.insert({ texture_id, Soil_Load_Texture(texture_id, mode) });
	}
	return texture_db[texture_id];
}

void PaintBrush::draw_quad()
{
	glPushMatrix();
	glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 1.00f);	glVertex3f(0.5f, 0.5f, 0.0f);
		glTexCoord2f(0.0f, 1.00f);	glVertex3f(-0.5f, 0.5f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);	glVertex3f(-0.5f, -0.5f, 0.0f);
		glTexCoord2f(1.0f, 0.0f);	glVertex3f(0.5f, -0.5f, 0.0f);
	glEnd();
	glPopMatrix();
}