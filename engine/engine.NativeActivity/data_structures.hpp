#pragma once

enum touch_type {
	down,
	up,
	move,
	cancel,
	notchanged
};

struct touch_pointer_t {
	int id = 0;
	float x = 0;
	float y = 0;

	touch_type type = touch_type::notchanged;
};