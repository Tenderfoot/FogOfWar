
#include "audiocontroller.h"
#include "settings.h"

std::map<std::string, Mix_Chunk*> AudioController::audio_db = {};
extern Settings user_settings;
int AudioController::music_channel;
float AudioController::music_volume;
float AudioController::effects_volume;

void AudioController::init()
{
	printf("initializing sound..\n");
	printf("sound is currently: %d\n", user_settings.use_sound);
	constexpr int audio_rate = 22050;
	Uint16 audio_format = MIX_DEFAULT_FORMAT;
	int audio_channels = 2;
	int audio_buffers = 1024;

	music_volume = 0.5;
	effects_volume = 0.1;

	if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
		printf("Unable to open audio!\n");
		exit(1);
	}

	Mix_AllocateChannels(16);

	Mix_Volume(-1, MIX_MAX_VOLUME * effects_volume);
}

void AudioController::play_sound(const std::string& filename)
{
	if (user_settings.use_sound)
	{
		Mix_PlayChannel(-1, get_sound(filename), 0);
	}
}

void AudioController::play_music()
{
	const char* music_file = "data/sounds/Fog_of_War.mp3";

	if (user_settings.use_sound)
	{
		music_channel = Mix_PlayChannel(-1, get_sound(music_file), -1);
		Mix_Volume(music_channel, MIX_MAX_VOLUME * music_volume);
	}
}

void AudioController::stop_music()
{
	Mix_HaltChannel(music_channel);
}

Mix_Chunk* AudioController::get_sound(const std::string& audio_id)
{
	auto it = audio_db.find(audio_id);
	if (it == audio_db.end())
	{
		audio_db.insert({ audio_id , Mix_LoadWAV(audio_id.c_str()) });
	}
	return audio_db[audio_id];
}