#pragma once

class ProgramObject {
public:
	struct Uniform {
		Uniform(const std::string& _name, GLint _size, GLenum _type, GLint _location) :
			name(_name),
			size(_size),
			type(_type),
			location(_location) { }
		std::string name;
		GLint size;
		GLenum type;
		GLint location;

		void set(const float* data) {
			if (name.empty()) return; // do nothing
			switch (type)
			{
				case GL_FLOAT: glUniform1fv(location, size, data); break;
				case GL_FLOAT_VEC2: glUniform2fv(location, size, data); break;
				case GL_FLOAT_VEC3: glUniform3fv(location, size, data); break;
				case GL_FLOAT_VEC4: glUniform4fv(location, size, data); break;
				case GL_FLOAT_MAT2: glUniformMatrix2fv(location, size, GL_FALSE, data); break;
				case GL_FLOAT_MAT3: glUniformMatrix3fv(location, size, GL_FALSE, data); break;
				case GL_FLOAT_MAT4: glUniformMatrix4fv(location, size, GL_FALSE, data); break;
				default:
					LOGE("glUniform(Matrix)[1|2|3|4][fv], uniform type mismatch");
					break;
			}
		}

		void set(const int* data) {
			if (name.empty()) return; // do nothing
			switch (type)
			{
				case GL_INT:
				case GL_SAMPLER_2D:
				case GL_SAMPLER_CUBE:
					glUniform1iv(location, size, data); break;
				case GL_INT_VEC2: glUniform2iv(location, size, data); break;
				case GL_INT_VEC3: glUniform3iv(location, size, data); break;
				case GL_INT_VEC4: glUniform4iv(location, size, data); break;
				default:
					LOGE("glUniform[1|2|3][iv], uniform type mismatch");
					break;
			}
		}
		void set(int a) {
			assert(type == GL_INT);
			glUniform1i(location, a);
		}
		void set(int a, int b) {
			assert(type == GL_INT_VEC2);
			glUniform2i(location, a, b);
		}
		void set(int a, int b, int c) {
			assert(type == GL_INT_VEC3);
			glUniform3i(location, a, b, c);
		}
		void set(int a, int b, int c, int d) {
			assert(type == GL_INT_VEC4);
			glUniform4i(location, a, b, c, d);
		}

		static Uniform& no_op() {
			static Uniform no_operation_uniform(std::string(), 0, 0, 0);
			return no_operation_uniform;
		}
	};

	struct Texture {
		Texture(const std::string& _name, GLint _size, GLenum _type, GLint _location, GLbyte _active_texture_index) :
			name(_name),
			size(_size),
			type(_type),
			location(_location),
			active_texture_index(_active_texture_index),
			texture_id(0) { }

		bool image2d(const std::string& img_path) {
			bytes image = Asset::read(img_path);
			texture_id = SOIL_load_OGL_texture_from_memory(
			   (unsigned char*)image.get(),
			   image.lenght,
			   SOIL_LOAD_AUTO,
			   SOIL_CREATE_NEW_ID,
			   SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
			   );
			// ok texture is created succesfully :)
			return texture_id != 0;
		}

		void enable() {
			if (!texture_id) return; // do nothing !
			glActiveTexture(GL_TEXTURE0 + active_texture_index);
			glBindTexture(GL_TEXTURE_2D, texture_id);
			glUniform1i(location, active_texture_index);
		}

		std::string name;
		GLint size;
		GLenum type;
		GLint location;
		GLbyte active_texture_index;
		GLuint texture_id;

		void delete_() {
			if (texture_id) {
				glDeleteTextures(1, &texture_id);
				texture_id = 0;
			}
		}
		~Texture() {
			delete_();
		}

		static Texture& no_op() {
			static Texture _no_op = Texture("", 0, 0, 0, -1);
			return _no_op;
		}
	};

	struct Attribute {
		Attribute(const std::string& _name, GLint _size, GLenum _type, GLint _location) :
			name(_name),
			size(_size),
			type(_type),
			location(_location),
			buffer_id(0),
			components_per_vertex(0) { }

		std::string name;
		GLint size;
		GLenum type;
		GLint location;
		GLuint buffer_id;
		GLint components_per_vertex;

		// have a const vertex attribute (it may be better to use a uniform instead)	
		void set_enable_const(const float* data) {
			if (name.empty()) return; // do nothing
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			switch (type)
			{
				case GL_FLOAT: glVertexAttrib1fv(location, data); break;
				case GL_FLOAT_VEC2: glVertexAttrib2fv(location, data); break;
				case GL_FLOAT_VEC3: glVertexAttrib3fv(location, data); break;
				case GL_FLOAT_VEC4: glVertexAttrib4fv(location, data); break;
				default:
					LOGE("glVertexAttrib[1|2|3|4][fv], attriubte type mismatch");
					break;
			}
		}
		// sets an attriubte data and calls glEnableVertexAttribArray
		void set_enable(GLint _components_per_vertex, const float* data) {
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glVertexAttribPointer(location, _components_per_vertex, GL_FLOAT, GL_FALSE, 0, data);
			glEnableVertexAttribArray(location);
		}
		// Enable a previously bound VBO (call this method only if you've called 'buffer_data' method before)
		void enable() {
			glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
			glVertexAttribPointer(location, components_per_vertex, GL_FLOAT, GL_FALSE, 0, (const void*)0);
			glEnableVertexAttribArray(location);
		}
		// Create a VBO and transfer the data to server memory
		void buffer_data(GLint _components_per_vertex, GLint num_vertex, const float* data) {
			components_per_vertex = _components_per_vertex;
			if (!buffer_id)
				glGenBuffers(1, &buffer_id);
			glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
			glBufferData(GL_ARRAY_BUFFER, num_vertex*components_per_vertex*sizeof(float), data, GL_STATIC_DRAW);
		}

		void delete_() {
			if (buffer_id) {
				glDeleteBuffers(1, &buffer_id);
				buffer_id = 0;
			}
		}
		~Attribute() {
			delete_();
		}

		static Attribute& no_op() {
			static Attribute no_operation_attrib(std::string(), 0, 0, 0);
			return no_operation_attrib;
		}
	};

	struct Indices {
		GLuint buffer_id = 0;
		GLuint num_indices = 0;
		void buffer_data(GLuint _num_indices, const GLushort *data) {
			num_indices = _num_indices;
			if (!buffer_id)
				glGenBuffers(1, &buffer_id);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_id);

			glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices*sizeof(GLushort), data, GL_STATIC_DRAW);
		}
		~Indices() {
			if (buffer_id)
				glDeleteBuffers(1, &buffer_id);
		}
	};

	Indices indices;

	void draw() {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices.buffer_id);
		glDrawElements(GL_TRIANGLES, indices.num_indices, GL_UNSIGNED_SHORT, (void *)0);
	}

	GLuint id = 0;
	std::vector<Attribute> _attributes;
	std::vector<Uniform> _uniforms;
	std::vector<Texture> _textures;


	ProgramObject() { }
	ProgramObject(const char* vertex_shader_src, const char* frag_shader_src) {
		compile(vertex_shader_src, frag_shader_src);
	}

	void compile(const char* vertex_shader_src, const char* frag_shader_src) {
		GLuint vertex_shader_id, frag_shader_id;
		GLint is_linked;
		id = glCreateProgram();

		if (!id) {
			LOGE("glCreateProgram failed in shader::create_program()");
			return;
		}

		vertex_shader_id = compile_shader(vertex_shader_src, GL_VERTEX_SHADER);
		frag_shader_id = compile_shader(frag_shader_src, GL_FRAGMENT_SHADER);

		glAttachShader(id, vertex_shader_id);
		glAttachShader(id, frag_shader_id);

		glLinkProgram(id);
		glGetProgramiv(id, GL_LINK_STATUS, &is_linked);
		if (!is_linked) {
			GLint infoLen = 0;
			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLen);

			if (infoLen > 1) {
				std::vector<char> infoLog(sizeof(char)* infoLen);
				glGetProgramInfoLog(id, infoLen, NULL, &infoLog[0]);
				LOGE("glLinkProgram failed.\nERROR => %s", &infoLog[0]);
			}
			else LOGE("glLinkProgram failed.");

			glDeleteProgram(id);
			return;
		}

		// reduce one reference to shaders
		glDeleteShader(vertex_shader_id);
		glDeleteShader(frag_shader_id);

		// extract active uniforms & textures
		{
			GLint max_unifrom_len;
			GLint num_uniforms;

			glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &num_uniforms);
			glGetProgramiv(id, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_unifrom_len);

			std::vector<char> uniform_name(max_unifrom_len + 1);
			_uniforms.clear();

			for (int inx_of_uniform = 0; inx_of_uniform < num_uniforms; ++inx_of_uniform) {
				GLint size;
				GLenum type;
				GLint location;

				glGetActiveUniform(id, inx_of_uniform, max_unifrom_len + 1, NULL, &size, &type, &uniform_name[0]);
				location = glGetUniformLocation(id, &uniform_name[0]);
				
				if (type != GL_SAMPLER_2D && type != GL_SAMPLER_CUBE) // it's a regular uniform
					_uniforms.push_back(Uniform(&uniform_name[0], size, type, location));
				else { // it is a texture
					GLbyte active_texture_index = _textures.size();
					_textures.push_back(Texture(&uniform_name[0], size, type, location, active_texture_index));
				}
			}
		}

		// extract active attributes
		{
			GLint max_attriubte_len;
			GLint num_attributes;
			glGetProgramiv(id, GL_ACTIVE_ATTRIBUTES, &num_attributes);
			glGetProgramiv(id, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_attriubte_len);

			std::vector<char> attriubte_name(max_attriubte_len + 1);
			_attributes.clear();

			for (int inx_of_attriubte = 0; inx_of_attriubte < num_attributes; ++inx_of_attriubte) {
				GLint size;
				GLenum type;
				GLint location;
				glGetActiveAttrib(id, inx_of_attriubte, max_attriubte_len + 1, NULL, &size, &type, &attriubte_name[0]);
				location = glGetAttribLocation(id, &attriubte_name[0]);
				_attributes.push_back(Attribute(&attriubte_name[0], size, type, location));
			}
		}
	}
	// compiles a shader
	static GLint compile_shader(const char *shaderSrc, GLenum type) {
		GLuint shader;
		GLint compiled;
		// Create the shader object
		shader = glCreateShader(type);
		if (shader == 0)
			return 0;
		// Load the shader source
		glShaderSource(shader, 1, &shaderSrc, NULL);
		// Compile the shader
		glCompileShader(shader);
		// Check the compile status
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
			GLint infoLen = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
			if (infoLen > 1) {
				//vector<char> infoLog(sizeof(char)* infoLen);
				//glGetShaderInfoLog(shader, infoLen, NULL, &infoLog[0]);
				//LOGE("glCompileShader failed.\nERROR => %s", &infoLog[0]);
			}
			else LOGE("glCompileShader failed");

			glDeleteShader(shader);
			return 0;
		}
		return shader;
	}

	Uniform& uniform(const std::string& name) {
		for (auto& uniform : _uniforms)
			if (uniform.name == name)
				return uniform;
		LOGE("UNIFORM %s NOT FOUND", name.c_str());
		// do nothing uniform
		return Uniform::no_op();
	}
	Attribute& attrib(const std::string& name) {
		for (auto& attrib : _attributes)
			if (attrib.name == name)
				return attrib;
		LOGE("ATTRIBUTE %s NOT FOUND", name.c_str());
		// do nothing uniform
		return Attribute::no_op();
	}
	Texture& texture(const std::string& name) {
		for (auto& tex : _textures)
			if (tex.name == name)
				return tex;
		LOGE("TEXTURE %s NOT FOUND", name.c_str());
		return Texture::no_op();
	}
	// delete the program with glDeleteProgram to free the resources
	void delete_() {
		if (id) {
			_attributes.clear();
			_textures.clear();
			glDeleteProgram(id);
			id = 0;

		}
	}
	~ProgramObject() {
		delete_();
	}
};
