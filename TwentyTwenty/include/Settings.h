#pragma once

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
private:
	bool dirty{ false };
};
