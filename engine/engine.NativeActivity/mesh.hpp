#pragma once

class MeshBase {
public:
	ProgramObject program_obj;
	MeshBase(const char* vshader_src, const char* fshader_src)
		: program_obj(vshader_src, fshader_src) { }

	// initalize position and indices array
	virtual void init() = 0;

	// enable attribute,uniform,sampler data in before_draw
	void draw() {
		program_obj.use_me();

		// call child class method
		before_draw();

		program_obj.draw();
	}
	virtual ~MeshBase() { }
protected:

	virtual void before_draw() = 0;
};

class Mesh : public MeshBase {
public:
	tinyobj::mesh_t mesh;
	tinyobj::material_t material;
	const std::string obj_file;



	Mesh(const std::string _obj_file,const char* vshader_src, const char* fshader_src)
		: MeshBase(vshader_src,fshader_src), obj_file(_obj_file) { }

	virtual void init() override {
		std::vector<tinyobj::shape_t> all_shapes;
		std::vector<tinyobj::material_t> all_materials;

		tinyobj::LoadObj(all_shapes, all_materials,obj_file.c_str());
		if (!all_shapes.empty())
			mesh = std::move(all_shapes.front().mesh);
		if (!all_materials.empty())
			material = std::move(all_materials.front());

		program_obj.attrib("position").buffer_data(3, mesh.positions.size() / 3, &mesh.positions[0]);
		program_obj.indices.buffer_data(mesh.indices.size(), &mesh.indices[0]);
	}

	virtual ~Mesh() { }

protected:
	virtual void before_draw() override {
		program_obj.attrib("position").enable();
	}
};