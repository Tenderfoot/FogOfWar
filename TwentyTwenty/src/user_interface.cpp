
#include "common.h"
#include "user_interface.h"
#include "paintbrush.h"
#include "game.h"
#include "fow_player.h"
#include "Settings.h"

extern Settings user_settings;
std::vector<UIWidget*> UserInterface::widgets;
GridManager* UserInterface::grid_manager;

MapWidget::MapWidget()
{
	map_grid = UserInterface::grid_manager;
	visible = true;
	position.x = user_settings.width / 100;
	position.y = (user_settings.height / 9) * 7.5;
	draw_2d = true;
	type_to_color = { std::make_pair(TILE_DIRT, t_vertex(0.72f,0.47f,0.34f)),
					std::make_pair(TILE_GRASS, t_vertex(0.5f,1.0f,0.5f)),
					std::make_pair(TILE_WATER, t_vertex(0.0f,0.0f,1.0f)),
					std::make_pair(TILE_ROCKS, t_vertex(0.5f,0.5f,0.5f)),
					std::make_pair(TILE_TREES, t_vertex(0.0f,0.5f,0.0f)) };
}

void MapWidget::take_input(SDL_Keycode input, bool keydown)
{
	if (input == LMOUSE && keydown == true)
	{
		t_vertex mouse_coords = Game::raw_mouse_position;
		t_vertex maxes = t_vertex(position.x + (size.x * map_grid->width), (position.y + size.y * map_grid->height), 0.0f);
		if (mouse_coords.x > position.x && mouse_coords.x < maxes.x &&
			mouse_coords.y>position.y && mouse_coords.y < maxes.y)
		{
			float x_percent = (mouse_coords.x - position.x) / ((size.x * map_grid->width));
			float y_percent = (mouse_coords.y - position.y) / ((size.y * map_grid->height));
			FOWPlayer::camera_pos = t_vertex(map_grid->width*x_percent, -(map_grid->height*y_percent), FOWPlayer::camera_pos.z);
		}
	}
}

void MapWidget::draw()
{
	if (map_grid != NULL)
	{
		// This may seem super arbitrary but it works out so that
		// the map looks the same regardless of the resolution you're using
		size.x = ((user_settings.width / 14) / (map_grid->width / 15))*0.1;
		size.y = ((user_settings.height / 12) / (map_grid->height / 15))*0.1;
	}

	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, NULL);

	float i, j;
	for (i = 0; i < map_grid->width; i++)
	{
		for (j = 0; j < map_grid->height; j++)
		{
			t_tile map_tile = map_grid->tile_map[i][j];

			glPushMatrix();

			glTranslatef(position.x + (i * size.x), position.y + (j * size.y), 0.0f);
			glScalef(size.x, size.y, 1.0f);

			if (map_tile.entity_on_position != nullptr)
			{
				glColor3f(1.0, 1.0f, 0.0f);
			}
			else
			{
				t_vertex color = type_to_color[map_tile.type];
				glColor3f(color.x, color.y, color.z);
			}

			PaintBrush::draw_quad();

			glPopMatrix();
		}
	}

	glColor3f(1.0, 1.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
}

GreenBox::GreenBox()
{
	visible = false;
	position.x = 0;
	position.y = 0;
	size.x = 0;
	size.y = 0;
}

void GreenBox::draw()
{
	if (visible)
	{
		glDisable(GL_TEXTURE_2D);
		glColor3f(0.5f, 1.0f, 0.5f);
		glLineWidth(2.0f);

		float x = position.x;
		float y = position.y;
		float width = size.x;
		float height = size.y;

		glBegin(GL_LINES);
			glVertex2f(x, y);
			glVertex2f(width, y);
			glVertex2f(x, y);
			glVertex2f(x, height);
			glVertex2f(width, y);
			glVertex2f(width, height);
			glVertex2f(x, height);
			glVertex2f(width, height);
		glEnd();

		glEnable(GL_TEXTURE_2D);
		glColor3f(1.0f, 1.0f, 1.0f);
	}
}

void UserInterface::take_input(SDL_Keycode input, bool keydown)
{
	for (auto widget : widgets)
	{
		if (widget->visible)
		{
			widget->take_input(input, keydown);
		}
	}
}

void UserInterface::draw()
{
	// set up orthographic projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1920, 1080, 0.0, -1.0, 1.0);
	// go back to the modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	glColor3f(1.0f, 1.0f, 1.0f);

	for (auto widget : widgets)
	{
		if (widget->visible)
		{
			widget->draw();
		}
	}

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	
	// go back to regular projection...
	glMatrixMode(GL_PROJECTION);  // Select The Projection Matrix
	glLoadIdentity();                // Reset The Projection Matrix
	gluPerspective(90, (float)user_settings.width / (float)user_settings.height, 0.1, 1000.0);
	glMatrixMode(GL_MODELVIEW);  // Select The Model View Matrix
	glLoadIdentity();    // Reset The Model View Matrix
}

// does a widget have mouse focus? hopefully two don't!
void UserInterface::mouse_focus()
{
	/*
	for (auto widget : widgets)
	{
		if (widget->coords_in_ui(mouse_coords))
		{
			return *widget;
		}
	}
	*/
}

void UserInterface::add_widget(UIWidget* new_widget)
{
	widgets.push_back(new_widget);
}