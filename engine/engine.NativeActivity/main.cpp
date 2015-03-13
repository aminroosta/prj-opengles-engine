using namespace std;
struct engine engine;
/**
* Initialize an EGL context for the current display.
*/
static int engine_init_display(struct engine* engine) {
	// initialize OpenGL ES and EGL

	/*
	* Here specify the attributes of the desired configuration.
	* Below, we select an EGLConfig with at least 8 bits per color
	* component compatible with on-screen windows
	*/
	const EGLint attribs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_NONE
	};
	EGLint w, h, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(display, 0, 0);

	/* Here, the application chooses the configuration it desires. In this
	* sample, we have a very simplified selection process, where we pick
	* the first EGLConfig that matches our criteria */
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);

	/* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
	* guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
	* As soon as we picked a EGLConfig, we can safely reconfigure the
	* ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

	surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
	EGLint AttribList[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	context = eglCreateContext(display, config, NULL, AttribList);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		LOGW("Unable to eglMakeCurrent");
		return -1;
	}

	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	engine->display = display;
	engine->context = context;
	engine->surface = surface;
	engine->width = w;
	engine->height = h;

	glViewport(0, 0, w, h);

	static bool first_time_called = true;
	// call Game::start first time display is initialized
	if (first_time_called) {
		Game::start();
		first_time_called = false;
	}
	Game::resume();
	return 0;
}

/**
* Just the current frame in the display.
*/
static void engine_draw_frame(struct engine* engine) {
	if (engine->display == NULL) {
		// No display.
		return;
	}

	Game::draw();

	eglSwapBuffers(engine->display, engine->surface);
}

/**
* Tear down the EGL context currently associated with the display.
*/
static void engine_term_display(struct engine* engine) {
	Game::pause();

	if (engine->display != EGL_NO_DISPLAY) {
		eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (engine->context != EGL_NO_CONTEXT) {
			eglDestroyContext(engine->display, engine->context);
		}
		if (engine->surface != EGL_NO_SURFACE) {
			eglDestroySurface(engine->display, engine->surface);
		}
		eglTerminate(engine->display);
	}
	engine->animating = 0;
	engine->display = EGL_NO_DISPLAY;
	engine->context = EGL_NO_CONTEXT;
	engine->surface = EGL_NO_SURFACE;
}

/**
* Process the next input event.
*/
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
	int32_t type = AInputEvent_getType(event);
	if (AINPUT_EVENT_TYPE_MOTION == type) {

		vector<touch_pointer_t> touch_pointers;
		size_t pointer_count = AMotionEvent_getPointerCount(event);

		// extract touch X,Y,id for all of the pointers
		rep(i, 0, pointer_count) {
			touch_pointer_t pointer;

			pointer.id = AMotionEvent_getPointerId(event, i);
			pointer.x = AMotionEvent_getX(event, i);
			pointer.y = AMotionEvent_getY(event, i);

			touch_pointers.push_back(pointer);
		}

		// combined action and index of the pointer thant changed the state
		int action = AMotionEvent_getAction(event);

		// move the pointer that changed state to the beginning of vector
		if (action >> 8)
			swap(touch_pointers[0], touch_pointers[action >> 8]);

		switch (action & AMOTION_EVENT_ACTION_MASK) {
			case AMOTION_EVENT_ACTION_DOWN:
			case AMOTION_EVENT_ACTION_POINTER_DOWN:
				touch_pointers[0].type = touch_type::down;
				break;
			case AMOTION_EVENT_ACTION_UP:
			case AMOTION_EVENT_ACTION_POINTER_UP:
				touch_pointers[0].type = touch_type::up;
				break;
			case AMOTION_EVENT_ACTION_MOVE:
				touch_pointers[0].type = touch_type::move;
				break;
			case AMOTION_EVENT_ACTION_CANCEL:
				touch_pointers[0].type = touch_type::cancel;
				break;
			default:
				touch_pointers[0].type = touch_type::notchanged;
				break;
		}

		Game::onTouch(std::move(touch_pointers));

		return 1;
	}
	return 0;
}

/**
* Process the next main command.
*/
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
	struct engine* engine = (struct engine*)app->userData;
	switch (cmd) {
		case APP_CMD_SAVE_STATE:
			// The system has asked us to save our current state.  Do so.
			engine->app->savedState = malloc(sizeof(struct game_state));
			*((struct game_state*)engine->app->savedState) = Game::state;
			engine->app->savedStateSize = sizeof(struct game_state);
			break;
		case APP_CMD_INIT_WINDOW:
			// The window is being shown, get it ready.
			if (engine->app->window != NULL) {
				engine_init_display(engine);
				engine->animating = 1;
				engine_draw_frame(engine);
			}
			break;
		case APP_CMD_TERM_WINDOW:
			// The window is being hidden or closed, clean it up.
			engine_term_display(engine);
			break;
		case APP_CMD_GAINED_FOCUS:
			// When our app gains focus, we start monitoring the accelerometer.
			if (engine->accelerometerSensor != NULL) {
				ASensorEventQueue_enableSensor(engine->sensorEventQueue,
					engine->accelerometerSensor);
				// We'd like to get 60 events per second (in us).
				ASensorEventQueue_setEventRate(engine->sensorEventQueue,
					engine->accelerometerSensor, (1000L / 60) * 1000);
			}
			break;
		case APP_CMD_LOST_FOCUS:
			// When our app loses focus, we stop monitoring the accelerometer.
			// This is to avoid consuming battery while not being used.
			if (engine->accelerometerSensor != NULL) {
				ASensorEventQueue_disableSensor(engine->sensorEventQueue,
					engine->accelerometerSensor);
			}
			// Also stop animating.
			engine->animating = 0;
			engine_draw_frame(engine);
			break;
	}
}

/**
* This is the main entry point of a native application that is using
* android_native_app_glue.  It runs in its own thread, with its own
* event loop for receiving input events and doing other things.
*/
void android_main(struct android_app* state) {
	memset(&engine, 0, sizeof(engine));
	state->userData = &engine;
	state->onAppCmd = engine_handle_cmd;
	state->onInputEvent = engine_handle_input;
	engine.app = state;

	// Prepare to monitor accelerometer
	engine.sensorManager = ASensorManager_getInstance();
	engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,
		ASENSOR_TYPE_ACCELEROMETER);
	engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager,
		state->looper, LOOPER_ID_USER, NULL, NULL);

	if (state->savedState != NULL) {
		// We are starting with a previous saved state; restore from it.
		Game::state = *(struct game_state*)state->savedState;
	}

	engine.animating = 1;

	// loop waiting for stuff to do.
	while (1) {
		// Read all pending events.
		int ident;
		int events;
		struct android_poll_source* source;

		// If not animating, we will block forever waiting for events.
		// If animating, we loop until all events are read, then continue
		// to draw the next frame of animation.
		while ((ident = ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events, (void**)&source)) >= 0) {

			// Process this event.
			if (source != NULL) {
				source->process(state, source);
			}

			// If a sensor has data, process it now.
			if (ident == LOOPER_ID_USER) {
				if (engine.accelerometerSensor != NULL) {
					ASensorEvent event;
					while (ASensorEventQueue_getEvents(engine.sensorEventQueue, &event, 1) > 0) {
						//LOGI("accelerometer: x=%f y=%f z=%f", event.acceleration.x, event.acceleration.y, event.acceleration.z);
					}
				}
			}

			// Check if we are exiting.
			if (state->destroyRequested != 0) {
				engine_term_display(&engine);
				return;
			}
		}

		if (engine.animating) {
			engine_draw_frame(&engine);
		}
	}
}
