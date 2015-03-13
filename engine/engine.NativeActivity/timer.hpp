#pragma once
class Timer {
public:
	long long _last_time;
	Timer() {
		reset();
	}
	long long nano_time() {
		struct timespec now;
		/*int err = */clock_gettime(CLOCK_MONOTONIC, &now);
		return now.tv_sec * 1000000000LL + now.tv_nsec;
	}

	// reset the timer
	void reset() {
		_last_time = nano_time();
	}
	// get the time difference from last call to step in seconds
	float step() {
		long long now = nano_time();
		float delta = (now - _last_time)*0.000000001f;
		_last_time = now;
		return delta;
	}
};