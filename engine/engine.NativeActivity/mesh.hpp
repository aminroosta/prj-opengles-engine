#pragma once

class Mesh {
public:
	ProgramObject program_obj;
	tinyobj::mesh_t mesh;
	tinyobj::material_t material;



	Mesh(const char* vshader_src, const char* fshader_src)
		: program_obj(vshader_src, fshader_src) { }

	virtual void initialize(const std::string& file_name) {
		std::vector<tinyobj::shape_t> all_shapes;
		std::vector<tinyobj::material_t> all_materials;

		tinyobj::LoadObj(all_shapes, all_materials,file_name.c_str());
		if (!all_shapes.empty())
			mesh = std::move(all_shapes.front().mesh);
		if (!all_materials.empty())
			material = std::move(all_materials.front());

		program_obj.attrib("position").buffer_data(3, mesh.positions.size() / 3, &mesh.positions[0]);
		program_obj.indices.buffer_data(mesh.indices.size(), &mesh.indices[0]);
	}


	virtual void draw(const glm::mat4& mvp) {
		program_obj.uniform("mvp").set(&mvp[0][0]);
		program_obj.attrib("position").enable();
		program_obj.draw();
	}

	virtual ~Mesh() { }
};