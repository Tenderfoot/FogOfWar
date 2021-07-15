#pragma once

#include <SDL_opengl.h>
#include <gl/GLU.h>
#include <map>
#include <string>
#include "SDL_mixer.h"

class AudioController
{
public:

	static std::map<std::string, Mix_Chunk*> audio_db;

	static void init();
	static void play_sound(std::string filename);

	static Mix_Chunk* get_sound(std::string audio_id);

};

