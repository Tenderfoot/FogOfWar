#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <semaphore>
#include <string>
#include <thread>

#include "json.hpp"
#include "Settings.h"

static std::thread* save_thread{ nullptr };
static std::queue < std::pair<Settings, std::string>> save_queue;
static std::binary_semaphore save_sem{0};
static bool stop_request{ false };

static void file_write_thread();

void Settings::write_to_json(nlohmann::json& json_object) const
{
	json_object["width"] = width;
	json_object["height"] = height;
	json_object["fullscreen"] = fullscreen;
	json_object["use_sound"] = use_sound;
}

void save_settings_to_file(const Settings& settings, const std::string& file_path)
{
	if (save_thread == nullptr)
	{
		save_thread = new std::thread(file_write_thread);
	}
	save_queue.push(std::make_pair(settings, file_path));
	save_sem.release();
}

static void write_to_file(const Settings& settings, const std::string& file_path)
{
	std::ofstream output_file(file_path);
	nlohmann::json settings_data;
	if (output_file.is_open() )
	{
		settings.write_to_json(settings_data);
		std::string formatted_json = settings_data.dump(4);
		output_file << formatted_json << std::endl;
	}
	else
	{
		std::cout << "Failed to write " << file_path << std::endl;
	}
}

static void file_write_thread()
{
	std::cout << "Running file write thread " << std::endl;
	while (!stop_request)
	{
		save_sem.acquire();
		while (!save_queue.empty())
		{
			auto save_item = save_queue.front();
			write_to_file(save_item.first, save_item.second);
			save_queue.pop();
		}
	}
}

void shutdown_settings_thread()
{
	stop_request = true;
	save_sem.release();
	if (save_thread != nullptr)
	{
		save_thread->join();
	}
}
