#include <jni.h>
#include <errno.h>

#include <string.h>
#include <unistd.h>
#include <sys/resource.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <SLES/OpenSLES_AndroidConfiguration.h>

#include <sys/types.h>
#include <android/sensor.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <thread>
#include <memory>
#include <string>
#include <vector>
#include <cmath>

#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "App", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "App", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "App", __VA_ARGS__))
#define GLSL(a) #a
#define GLSLF(a) "precision mediump float;\n" #a
#define rep(i,f,t) for(decltype(t) i = (f); i < (t); ++i)

#undef NDEBUG
#include "glue.h"
#include "main.h"

// external library headers
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "soil.h"

// engine headers
#include "data_structures.hpp"
#include "io.hpp"
#include "SoundManager.hpp"
#include "obj_loader.h"
#include "program_object.hpp"
#include "timer.hpp"
#include "mesh.hpp"
#include "game.h"