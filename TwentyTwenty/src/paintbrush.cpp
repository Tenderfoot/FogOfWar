

#include <stdio.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include "paintbrush.h"
#include "grid_manager.h"

std::map<std::string, GLuint> PaintBrush::texture_db = {};
TTF_Font* PaintBrush::font;
std::string PaintBrush::supported_characters;
std::map<char, t_texturechar> PaintBrush::char_texture;
std::map<std::string, GLenum> PaintBrush::shader_db = {};
std::map<std::pair<GLenum, std::string>, GLint> PaintBrush::uniform_db = {};

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
// stuff for VAOs....
PFNGLBINDVERTEXARRAYPROC	glBindVertexArray = NULL;
PFNGLGENVERTEXARRAYSPROC	glGenVertexArrays = NULL;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = NULL;

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

	// vertex arrays
	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)
		uglGetProcAddress("glGenVertexArrays");
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)
		uglGetProcAddress("glBindVertexArray");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)
		uglGetProcAddress("glVertexAttribPointer");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)
		uglGetProcAddress("glEnableVertexAttribArray");

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
	glGenVertexArrays(1, &the_vbo.vertex_array);
	glBindVertexArray(the_vbo.vertex_array);
	glGenBuffers(1, &the_vbo.vertex_buffer);
	glGenBuffers(1, &the_vbo.texcoord_buffer);
	glGenBuffers(1, &the_vbo.color_buffer);
}

/*
void PaintBrush::generate_vao(t_VAO& the_vao)
{
	glGenVertexArrays(1, &the_vao.vertex_array);
	glBindVertexArray(the_vao.vertex_array);
}

void PaintBrush::bind_vbo_to_vao(t_VAO& the_vao, t_VBO& the_vbo)
{
	glBindVertexArray(the_vao.vertex_array);

	// 2. copy our vertices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, the_vbo.vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(the_vbo.num_faces*3), the_vbo.verticies.get(), GL_DYNAMIC_DRAW);

	// 3. then set our vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


}*/

void PaintBrush::draw_vao(t_VBO& the_vbo)
{
	PaintBrush::use_shader(PaintBrush::get_shader("spine"));
	glBindVertexArray(the_vbo.vertex_array);
	glDrawArrays(GL_TRIANGLES, 0, the_vbo.num_faces);
	glBindVertexArray(0);
	PaintBrush::stop_shader();
}

void PaintBrush::bind_vbo(t_VBO& the_vbo)
{
	glBindBuffer(GL_ARRAY_BUFFER, the_vbo.vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * the_vbo.num_faces * 3, the_vbo.verticies.get(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, the_vbo.texcoord_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * the_vbo.num_faces * 2, the_vbo.texcoords.get(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, the_vbo.color_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * the_vbo.num_faces * 3, the_vbo.colors.get(), GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void PaintBrush::draw_vbo(t_VBO the_vbo)
{
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
}

void PaintBrush::draw_quad_vbo(t_VBO the_vbo)
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
	glBindTexture(GL_TEXTURE_2D, the_vbo.texture);
	glDrawArrays(GL_QUADS, 0, the_vbo.num_faces);
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

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
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


/******************** NEW SHADER STUFF *****************************/

GLenum PaintBrush::get_shader(std::string shader_id)
{
	std::map<std::string, GLenum>::iterator it;

	it = shader_db.find(shader_id);

	if (it == shader_db.end())
	{
		shader_db.insert({ shader_id, load_shader(shader_id) });
	}

	return shader_db[shader_id];
}

GLint PaintBrush::get_uniform(GLenum shader, std::string uniform_name)
{
	std::map<std::pair<GLenum, std::string>, GLint>::iterator it;
	std::pair<GLenum, std::string> mypair = std::make_pair(shader, uniform_name);
	GLint return_value;

	// I tried to overload the comparator so I could use map.find
	// but it wasn't cooperating with the custom operator I had
	// so I'm rewriting it to
	for (auto it = uniform_db.begin(); it != uniform_db.end(); ++it)
	{

		if (shader == it->first.first)
		{
			if (it->first.second == uniform_name)
			{
				return it->second;
			}
		}
	}

	return_value = get_uniform_location(shader, uniform_name);

	if (return_value != -1)
	{
		uniform_db.insert({ std::make_pair(shader, uniform_name), return_value });
	}

	return return_value;
}

// streak

GLenum PaintBrush::load_shader(std::string shadername)
{
	GLenum shader_program;

	shader_program = glCreateProgramObjectARB();

	GLenum my_fragment_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	GLenum my_vertex_shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);

	std::ostringstream full_path;
	full_path << "data/shaders/" << shadername << ".frag";

	std::ifstream myfile(full_path.str().c_str());
	std::stringstream ss;
	if (myfile.is_open())
	{
		ss << myfile.rdbuf();
		myfile.close();
	}
	else
	{
		printf("file not found\n");
		return 0;
	}

	std::string test(ss.str().c_str());
	const GLchar* frag_shad_src = test.c_str();
	glShaderSourceARB(my_fragment_shader, 1, &frag_shad_src, NULL);

	// LOAD IN VERTEX SHADER
	full_path.str("");
	full_path << "data/shaders/" << shadername << ".vert";

	std::ifstream myfiletwo(full_path.str().c_str());
	std::stringstream sstwo;
	if (myfiletwo.is_open())
	{
		sstwo << myfiletwo.rdbuf();
		myfiletwo.close();
	}
	std::string testtwo(sstwo.str().c_str());
	const GLchar* vertex_shad_src = testtwo.c_str();
	glShaderSourceARB(my_vertex_shader, 1, &vertex_shad_src, NULL);

	// Compile The Shaders
	int i;
	// VERTEX
	glCompileShaderARB(my_vertex_shader);

	GLint maxLength = 0;
	glGetShaderiv(my_vertex_shader, GL_INFO_LOG_LENGTH, &maxLength);
	if (maxLength > 0)
	{
		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(my_vertex_shader, maxLength, &maxLength, &errorLog[0]);
		printf("///////// SHADER COMPILER /////////////\n");
		for (i = 0; i < errorLog.size(); i++)
		{
			printf("%c", errorLog.at(i));
		}
		printf("///////// END SHADER COMPILER /////////////\n");
	}

	// FRAGMENT
	glCompileShaderARB(my_fragment_shader);

	maxLength = 0;
	glGetShaderiv(my_fragment_shader, GL_INFO_LOG_LENGTH, &maxLength);
	if (maxLength > 0)
	{
		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(my_fragment_shader, maxLength, &maxLength, &errorLog[0]);
		printf("///////// SHADER COMPILER /////////////\n");
		for (i = 0; i < errorLog.size(); i++)
		{
			printf("%c", errorLog.at(i));
		}
		printf("///////// END SHADER COMPILER /////////////\n");
	}

	// Attach The Shader Objects To The Program Object
	glAttachObjectARB(shader_program, my_vertex_shader);
	glAttachObjectARB(shader_program, my_fragment_shader);

	glLinkProgramARB(shader_program);

	return shader_program;
}

void PaintBrush::use_shader(GLenum shader)
{
	glUseProgramObjectARB(shader);
}

void PaintBrush::stop_shader()
{
	glUseProgramObjectARB(0);
}

GLint PaintBrush::get_uniform_location(GLenum shader, std::string variable_name)
{
	glUseProgramObjectARB(shader);
	GLint loc = glGetUniformLocationARB(shader, variable_name.c_str());
	glUseProgramObjectARB(0);

	return loc;
}

void PaintBrush::set_uniform_location(GLenum shader, GLint uniform_location, float data)
{
	glUseProgramObjectARB(shader);
	if (uniform_location != -1)
	{
		glUniform1fARB(uniform_location, data);
	}
	glUseProgramObjectARB(0);
}

void PaintBrush::set_uniform(GLenum shader, std::string uniform_name, float data)
{
	set_uniform_location(shader, get_uniform(shader, uniform_name), data);
}

/********************* VAO STUFF ******************************/

GLuint PaintBrush::vao, PaintBrush::vbo[2];

void PaintBrush::do_vao_setup()
{
	/* We're going to create a simple diamond made from lines */
	const GLfloat diamond[4][2] = {
	{  0.0,  1.0  }, /* Top point */
	{  1.0,  0.0  }, /* Right point */
	{  0.0, -1.0  }, /* Bottom point */
	{ -1.0,  0.0  } }; /* Left point */

	const GLfloat colors[4][3] = {
	{  1.0,  0.0,  0.0  }, /* Red */
	{  0.0,  1.0,  0.0  }, /* Green */
	{  0.0,  0.0,  1.0  }, /* Blue */
	{  1.0,  1.0,  1.0  } }; /* White */

	/* These pointers will receive the contents of our shader source code files */
	GLchar* vertexsource, * fragmentsource;

	/* These are handles used to reference the shaders */
	GLuint vertexshader, fragmentshader;

	/* This is a handle to the shader program */
	GLuint shaderprogram;

	/* Allocate and assign a Vertex Array Object to our handle */
	glGenVertexArrays(1, &vao);

	/* Bind our Vertex Array Object as the current used object */
	glBindVertexArray(vao);

	/* Allocate and assign two Vertex Buffer Objects to our handle */
	glGenBuffers(2, vbo);

	/* Bind our first VBO as being the active buffer and storing vertex attributes (coordinates) */
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);

	/* Copy the vertex data from diamond to our buffer */
	/* 8 * sizeof(GLfloat) is the size of the diamond array, since it contains 8 GLfloat values */
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), diamond, GL_STATIC_DRAW);

	/* Specify that our coordinate data is going into attribute index 0, and contains two floats per vertex */
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	/* Enable attribute index 0 as being used */
	glEnableVertexAttribArray(0);

	/* Bind our second VBO as being the active buffer and storing vertex attributes (colors) */
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);

	/* Copy the color data from colors to our buffer */
	/* 12 * sizeof(GLfloat) is the size of the colors array, since it contains 12 GLfloat values */
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), colors, GL_STATIC_DRAW);

	/* Specify that our color data is going into attribute index 1, and contains three floats per vertex */
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	/* Enable attribute index 1 as being used */
	glEnableVertexAttribArray(1);
}

void PaintBrush::draw_vao_dirty()
{
	use_shader(get_shader("spine"));
	glDrawArrays(GL_LINE_LOOP, 0, 4);
}