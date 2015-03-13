using namespace glm;
using namespace std;

SoundManager sound_manager;
void Game::start() {
	sound_manager.registerSound("bad_death.wav");
	sound_manager.start();
}

vector<Mesh*> meshes;
void Game::pause() {
	LOGI("Game::pause();");

	for (auto& mesh : meshes)
		delete mesh;
	meshes.clear();
}

class CubeMesh : public Mesh {
public:
	CubeMesh()
		: Mesh(
		// ================= vertex shader  ====================
		GLSL(
	uniform mat4 mvp;
	attribute vec4 position;
	attribute vec2 uv;
	varying vec2 _uv;
	void main() {
		gl_Position = mvp * position;
		_uv = uv;
	}),
		// ================== fragment shader ==================
		GLSLF(
	uniform sampler2D texture;
	varying vec2 _uv;
	varying vec4 _color;
	void main() {
		gl_FragColor = texture2D(texture, _uv);
	})
		) { }
	virtual void initialize(const string& name) override {
		Mesh::initialize(name);

		program_obj.attrib("uv").buffer_data(2, mesh.texcoords.size() / 2, &mesh.texcoords[0]);
		program_obj.texture("texture").image2d("texture.png");
	}
	virtual void draw(const mat4& mvp) override {
		glUseProgram(program_obj.id);

		program_obj.attrib("uv").enable();
		program_obj.texture("texture").enable();

		Mesh::draw(mvp);
	}
};

void Game::onTouch(std::vector<touch_pointer_t> pointers) {
	static bool playing = false;

	if (pointers[0].type == touch_type::down) {
		sound_manager.playSound("bad_death.wav");

		if (!playing)
			sound_manager.playBackground("background.mp3");
		else
			sound_manager.stopBackground();
		playing = !playing;
	}
}

void Game::resume() {
	LOGI("Game::resume();");
	meshes.push_back(new CubeMesh());
	meshes.front()->initialize("tex_cube.obj");
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearDepthf(1.0);
}

void Game::draw() {
	static double angle = 0;
	auto delta = timer.step();
	angle += delta*0.3;

	glm::vec4 color(cos(angle)*.5 + .5, sin(angle)*.5 + .5, 0, 1);
	glClearColor(color[0], color[1], color[2], color[3]);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	mat4 mvp = glm::lookAt(vec3(0, 1, 5), vec3(0, 0, 0), vec3(0, 1, 0));
	mat4 pers = glm::infinitePerspective(80.0f, float(engine.width) / engine.height, 1.0f);
	//mat4 pers = perspective(80.0f, float(engine.width) / engine.height, 0.0f, 100.0f);
	mvp = pers * mvp;

	mvp = mvp * glm::rotate((float)angle, vec3(1 - 0.3, 1 - delta, -1 + 5 * delta));

	for (auto& mesh : meshes)
		mesh->draw(mvp);
}


Timer Game::timer;
game_state Game::state;

