#pragma once

#define GAME_PLANE -50.0f
#include <map>
#include <SDL.h>

typedef enum
{
	NO_BIND,
	UP,
	DOWN,
	LEFT,
	RIGHT,
	ACTION,
	LMOUSE,
	RMOUSE,
	MIDDLEMOUSE,
	EDIT_KEY,
	PLAY_KEY,
	SAVE,
	SHIFT,
	CTRL,
	ALT,
	PAGE_UP,
	PAGE_DOWN,
	MWHEELUP,
	MWHEELDOWN,
	ESCAPE,
	FULLSCREEN
} boundinput;

extern std::map<boundinput, SDL_Keycode> keymap;

typedef enum
{
	MOVE,
	ATTACK,
	GATHER,
	BUILD_UNIT,
	BUILD_BUILDING
}t_ability_enum;

typedef enum
{
	GRID_IDLE,
	GRID_MOVING,
	GRID_ENDTURN,
	GRID_ATTACKING,
	GRID_DYING,
	GRID_DEAD,
	GRID_COLLECTING
}GridCharacterState;