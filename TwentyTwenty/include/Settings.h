#pragma once

// Forward declare to avoid the header include
class nlohmann::json;

struct Settings
{
public:
	int width, height;
	int fullscreen;

	void toggleFullScreen()
	{
		fullscreen = fullscreen ? 0 : 1;
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
