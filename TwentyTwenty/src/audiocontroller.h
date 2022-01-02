#pragma once

#include <SDL_opengl.h>
#include <gl/GLU.h>
#include <map>
#include <string>
#include "SDL_mixer.h"

class AudioController
{
public:

	// When storing pointers I'd recommend std::unique_ptr if they are owned by the container.
	// They help with managing the resource automically
	static std::map<std::string, Mix_Chunk*> audio_db;

	static void init();

	// I'd make this a const std::string& to prevent extra copies that may be incurred
	// by passing by value
	static void play_sound(std::string filename);

	// Since these area stored in the audio_db map, could they be returned by reference/const reference?
	// Generally in C++ returning a raw pointer opens you up to all sorts of fun un-expected problems
	static Mix_Chunk* get_sound(std::string audio_id);

};

