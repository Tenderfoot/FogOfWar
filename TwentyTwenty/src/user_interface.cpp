#include "common.h"
#include "user_interface.h"
#include "paintbrush.h"
#include "game.h"
#include "fow_player.h"
#include "Settings.h"
#include "grid_manager.h"

extern Settings user_settings;
std::vector<UIWidget*> UserInterface::widgets;

MapWidget::MapWidget()
{
	visible = true;
	position.x = user_settings.width / 100;
	position.y = (user_settings.height / 9) * 7.5;
	draw_2d = true;
	type_to_color = { std::make_pair(TILE_DIRT, t_vertex(0.72f,0.47f,0.34f)),
					std::make_pair(TILE_GRASS, t_vertex(0.5f,1.0f,0.5f)),
					std::make_pair(TILE_WATER, t_vertex(0.0f,0.0f,1.0f)),
					std::make_pair(TILE_ROCKS, t_vertex(0.5f,0.5f,0.5f)),
					std::make_pair(TILE_TREES, t_vertex(0.0f,0.5f,0.0f)) };
	PaintBrush::generate_vbo(map_vbo);
	build_map_vbo();
}

void MapWidget::take_input(SDL_Keycode input, bool keydown)
{
	// the attack move mode thing prevents minimap movement when clicking to do an attack move command
	if (input == LMOUSE && !FOWPlayer::attack_move_mode)
	{
		mouse_down = keydown;
	}
}

t_vertex MapWidget::get_click_position()
{
	t_vertex mouse_coords = Game::raw_mouse_position;
	float x_percent = (mouse_coords.x - position.x) / ((size.x * GridManager::size.x));
	float y_percent = (mouse_coords.y - position.y) / ((size.y * GridManager::size.y));
	return t_vertex((int)(x_percent * GridManager::size.x), (int)((y_percent)*GridManager::size.y), 0);
}

void MapWidget::build_map_vbo()
{
	map_vbo.num_faces = GridManager::size.x * GridManager::size.y * 4;	// two triangles I guess
	map_vbo.verticies = std::shared_ptr<float[]>(new float[map_vbo.num_faces * 3]);
	map_vbo.colors = std::shared_ptr<float[]>(new float[map_vbo.num_faces * 3]);
	map_vbo.texcoords = std::shared_ptr<float[]>(new float[map_vbo.num_faces * 2]);
	// is this ok with shared_ptr?
	float* verticies = map_vbo.verticies.get();
	float* texcoords = map_vbo.texcoords.get();
	float* colors = map_vbo.colors.get();

	// This may seem super arbitrary but it works out so that
	// the map looks the same regardless of the resolution you're using
	size.x = ((user_settings.width / 12) / (GridManager::size.x / 15)) * 0.1;
	size.y = ((user_settings.height / 10) / (GridManager::size.y / 15)) * 0.1;

	float widthItr, heightItr;
	for (widthItr = 0; widthItr < GridManager::size.x; widthItr++)
	{
		for (heightItr = 0; heightItr < GridManager::size.y; heightItr++)
		{
			t_tile map_tile = GridManager::tile_map[widthItr][heightItr];
			t_vertex color = t_vertex(1.0f, 1.0f, 1.0f);
			if (map_tile.entity_on_position != nullptr)
			{
			}
			else
			{
				color = type_to_color[map_tile.type];
			}

			int vertex_offset = (widthItr * GridManager::size.x * 12) + (heightItr * 12);
			int texcoord_offset = (widthItr * GridManager::size.x * 8) + (heightItr * 8);
			 
			verticies[vertex_offset + 0] = position.x + (widthItr * size.x) + 0.5f * size.x;
			verticies[vertex_offset + 1] = position.y + (heightItr * size.y) + 0.5f * size.y;
			verticies[vertex_offset + 2] = 0.0f;
			texcoords[texcoord_offset + 0] = 1;
			texcoords[texcoord_offset + 1] = 1;
			colors[vertex_offset + 0] = color.x;
			colors[vertex_offset + 1] = color.y;
			colors[vertex_offset + 2] = color.z;

			verticies[vertex_offset + 3] = position.x + (widthItr * size.x) - 0.5f * size.x;
			verticies[vertex_offset + 4] = position.y + (heightItr * size.y) + 0.5f * size.y;
			verticies[vertex_offset + 5] = 0.0f;
			texcoords[texcoord_offset + 2] = 0;
			texcoords[texcoord_offset + 3] = 1;
			colors[vertex_offset + 3] = color.x;
			colors[vertex_offset + 4] = color.y;
			colors[vertex_offset + 5] = color.z;

			verticies[vertex_offset + 6] = position.x + (widthItr * size.x) - 0.5f * size.x;
			verticies[vertex_offset + 7] = position.y + (heightItr * size.y) - 0.5f * size.y;
			verticies[vertex_offset + 8] = 0.0f;
			texcoords[texcoord_offset + 4] = 0;
			texcoords[texcoord_offset + 5] = 0;
			colors[vertex_offset + 6] = color.x;
			colors[vertex_offset + 7] = color.y;
			colors[vertex_offset + 8] = color.z;

			verticies[vertex_offset + 9] = position.x + (widthItr * size.x) + 0.5f * size.x;
			verticies[vertex_offset + 10] = position.y + (heightItr * size.y) - 0.5f * size.y;
			verticies[vertex_offset + 11] = 0.0f;
			texcoords[texcoord_offset + 6] = 1;
			texcoords[texcoord_offset + 7] = 0;
			colors[vertex_offset + 9] = color.x;
			colors[vertex_offset + 10] = color.y;
			colors[vertex_offset + 11] = color.z;
		}
	}

	PaintBrush::bind_vbo(map_vbo);
}

void MapWidget::draw()
{
	/*****  This should happen in update instead of draw ********/
	if (mouse_down)
	{
		t_vertex mouse_coords = Game::raw_mouse_position;
		if(coords_in_ui())
		{
			float x_percent = (mouse_coords.x - position.x) / ((size.x * GridManager::size.x));
			float y_percent = (mouse_coords.y - position.y) / ((size.y * GridManager::size.y));
			FOWPlayer::camera_pos = t_vertex(GridManager::size.x * x_percent, -(GridManager::size.y * y_percent), FOWPlayer::camera_pos.z);
		}
	}
	/*************************************************************/

	// draw the map
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	PaintBrush::draw_quad_vbo(map_vbo);

	// draw the red box
	glColor3f(1.0, 1.0f, 1.0f);
	draw_red_box();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
}

t_transform MapWidget::get_red_box()
{
	return t_transform(FOWPlayer::camera_pos.x, -FOWPlayer::camera_pos.y-5, 50, 20);
}

void MapWidget::draw_red_box()
{
	// we want to move the box so that it matches the camera over the grid
	// the ((0.28 * map_grid->width) comes from the fact at zoom z=36,
	// the box is the same size as the 128x128 minimap. it uses that as the baseline
	// so 128x = 36 so x = 36/128 = 0.28
	// and scales from there
	// works on all map sizes and reflects camera size pretty accurately
	// its not a perfect solution but it works well enough it might as well be
	float x_percent = (FOWPlayer::camera_pos.x / GridManager::size.x);
	float y_percent = ((FOWPlayer::camera_pos.y) / GridManager::size.y);
	float x = position.x + (((GridManager::size.x * size.x))*x_percent);
	float y = position.y - (((GridManager::size.y * size.y)) * y_percent);
	float width = ((((GridManager::size.x * size.x)) / ((0.28* GridManager::size.x) / FOWPlayer::camera_pos.z)))/2;
	float height = (((GridManager::size.y/2 * size.y) / ((0.28* GridManager::size.y) / FOWPlayer::camera_pos.z)))/2;

	glColor3f(1.0f, 0.0f, 0.0f);
	glLineWidth(2.0f);
	glBegin(GL_LINES);
		glVertex2f(x-width, y-height);
		glVertex2f(x+width, y - height);
		glVertex2f(x - width, y - height);
		glVertex2f(x - width, y+height);
		glVertex2f(x+width, y - height);
		glVertex2f(x+width, y + height);
		glVertex2f(x - width, y + height);
		glVertex2f(x+width, y + height);
	glEnd();
	glColor3f(1.0f, 1.0f, 1.0f);
}

bool MapWidget::coords_in_ui()
{
	t_vertex mouse_coords = Game::raw_mouse_position;
	t_vertex maxes = t_vertex(position.x + (size.x * GridManager::size.x), (position.y + size.y * GridManager::size.y), 0.0f);
	if (mouse_coords.x > position.x && mouse_coords.x < maxes.x &&
		mouse_coords.y>position.y && mouse_coords.y < maxes.y)
	{
		return true;
	}
	return false;
}

void UIImage::draw()
{
	glPushMatrix();

	glTranslatef(position.x * user_settings.width, position.y * user_settings.height, 0.0f);
	glScalef(size.x * user_settings.width, size.y * user_settings.height, 1.0f);

	glBindTexture(GL_TEXTURE_2D, texture);
	if (texture == NULL)
		glColor3f(0.0, 0.0f, 0.0f);
	PaintBrush::draw_quad();
	glColor3f(1.0, 1.0f, 1.0f);
	glPopMatrix();

	// this shouldn't just be sitting in UI image
	std::string gold_label = std::string("Gold: ").append(std::to_string(FOWPlayer::gold));
	PaintBrush::draw_string(t_vertex((user_settings.width)*0.725, (user_settings.height)*0.9 , 0), t_vertex(1.0, 1.0, 1.0f), gold_label);
}

void UIMenu::draw()
{
	float y_offset = 0;
	int i = 0;
	for (auto item : menu_options)
	{
		if (i == current_selection)
			glColor3f(1.0f, 0.0f, 0.0f);

		PaintBrush::draw_string(t_vertex(user_settings.width / 2, (user_settings.height / 2) + y_offset, 0.0f), t_vertex(1.0f, 1.0f, 1.0f), item);

		glColor3f(1.0f, 1.0f, 1.0f);
		y_offset += 50;
		i++;
	}
}

void UIMenu::take_input(SDL_Keycode input, bool keydown)
{
	if (keydown)
	{
		if (keymap[UP] == input)
		{
			current_selection = (current_selection - 1) % menu_options.size();
			current_selection = std::abs(current_selection);
		}

		if (keymap[DOWN] == input)
		{
			current_selection = (current_selection + 1) % menu_options.size();
		}

		if (keymap[ENTER_KEY] == input)
		{
			select_callback(menu_options.at(current_selection));
		}
	}
}

void UIProgressBar::set_current(int new_current)
{
	this->current = new_current;
}

void UIProgressBar::draw()
{
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, NULL);

	float bar_width = user_settings.width * 0.2;
	float bar_height = user_settings.height * 0.05;

	glPushMatrix();
	glTranslatef(user_settings.width / 2, user_settings.height*0.925, 0.0f);
	glScalef(bar_width, bar_height, 1.0f);
	glColor3f(0.2f, 0.2f, 0.2f);
	PaintBrush::draw_quad();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(user_settings.width / 2, user_settings.height * 0.925, 0.0f);
	glScalef(bar_width*((float)current/(float)maximum), bar_height, 1.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	PaintBrush::draw_quad();
	glPopMatrix();

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
		glDisable(GL_DEPTH_TEST);
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
		glEnable(GL_DEPTH_TEST);
		glColor3f(1.0f, 1.0f, 1.0f);
	}
}

UserInterface::UserInterface()
{
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
	glOrtho(0.0, (float)user_settings.width, (float)user_settings.height, 0.0, -1.0, 1.0);
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
bool UserInterface::mouse_focus()
{
	for (auto widget : widgets)
	{
		if (widget->coords_in_ui())
		{
			return true;
		}
	}
	return false;
}

void UserInterface::add_widget(UIWidget* new_widget)
{
	widgets.push_back(new_widget);
}