#pragma once

struct  game_state {
	int x;
};
//application class
class Game {
public:
	// Android Activity.OnCreate
	static void start();
	// Android Activity.OnPause
	static void pause();
	// Android Activity.OnResume
	static void resume();
	// Android Renderer.OnDraw
	static void draw();

	// first pointer in the array is the one that changed state
	static void onTouch(std::vector<touch_pointer_t> pointers);

	static Timer timer;

	static game_state state;

	// called on every touch event with two possible pointers, if id2 == -1 we dont have a second pointer
	// static void on_touch(int type, int id, float x, float y, int id2, float x2, float y2);

	//static const int TOUCH_MOVE = AMOTION_EVENT_ACTION_MOVE;
	//static const int TOUCH_UP = AMOTION_EVENT_ACTION_UP;
	//static const int TOUCH_DOWN = AMOTION_EVENT_ACTION_DOWN;

	//static double& width() { static double v = 0; return v; }
	//static double& height() { static double v = 0; return v; }
	//static double acpect() { return width() / height(); }
};
