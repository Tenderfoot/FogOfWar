#pragma once

#include "json.hpp"

struct Settings
{
public:
	int width, height;
	int fullscreen;
	int use_sound;
	int use_scroll;
	std::string host_name;

	void toggleFullScreen()
	{
		fullscreen = fullscreen ? 0 : 1;
		dirty = true;
	}

	void toggleSound()
	{
		use_sound = use_sound ? 0 : 1;
		dirty = true;
	}

	void toggleScroll()
	{
		use_scroll = use_scroll ? 0 : 1;
		dirty = true;
	}


	void clearDirty()
	{
		dirty = false;
	}

	bool isDirty() const
	{
		return dirty;
	}

	void write_to_json(nlohmann::json& json_object) const;
private:
	bool dirty{ false };
};

constexpr const char* DEFAULT_SETTINGS_PATH("data/settings.json");

void save_settings_to_file(const Settings& settings, const std::string& file_path);
void shutdown_settings_thread();
