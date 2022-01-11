#pragma once

#include <SDL_opengl.h>
#include <gl/GLU.h>
#include <map>
#include <string>
#include "SDL_mixer.h"

class AudioController
{
public:

	// these don't need safe pointers, sdl_mixers handels it, can't double garbage collect
	static std::map<std::string, Mix_Chunk*> audio_db;

	static void init();

	static void play_sound(const std::string& filename);
	static void play_music();
	static void stop_music();
	static Mix_Chunk* get_sound(const std::string& audio_id);

	static int music_channel;

};

